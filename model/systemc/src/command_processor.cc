#include "command_processor.h"
#include "commands.h"
#include "frame_buffer_payload.h"
#include "rasterized_scanline.h"
#include "rgb_color.h"
#include "timing.h"
#include "triangle_rasterizer.h"
#include "vertex_2d.h"
#include "vertex_2d_io.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <sysc/kernel/sc_module.h>
#include <sysc/kernel/sc_module_name.h>
#include <sysc/kernel/sc_time.h>
#include <sysc/kernel/sc_wait.h>
#include <sysc/kernel/sc_wait_cthread.h>
#include <sysc/utils/sc_report.h>
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>
#include <type_traits>
#include <variant>
#include <vector>

CommandProcessor::CommandProcessor(sc_core::sc_module_name name,
                                   const CommandProcessorConfig& config,
                                   size_t frame_buffer_width,
                                   size_t frame_buffer_height)
    : sc_core::sc_module(name),
      input_buffer_socket("input_buffer_socket"),
      frame_buffer_socket("frame_buffer_socket"),
      command_queue_socket("command_queue_socket"),
      config_(config),
      frame_buffer_width_(frame_buffer_width),
      frame_buffer_height_(frame_buffer_height) {
    if (frame_buffer_width_ == 0 || frame_buffer_height_ == 0) {
        throw std::invalid_argument("Framebuffer dimensions must be nonzero");
    }
    // compute clock period
    clock_period_ = clock_period_from_hz(config_.clock_freq_hz);

    // compute latency
    decode_latency_ = clock_period_ * static_cast<double>(config_.decode_cycles);
}

std::vector<Command> CommandProcessor::read_commands(size_t request_count) {
    // silently terminate request if 0 commands are requested
    if (request_count == 0) {
        return {};
    }

    CommandPayload trans;
    trans.operation = CommandPayload::Operation::Pop;
    trans.request_count = request_count;

    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

    command_queue_socket->b_transport(trans, delay);

    if (trans.response == CommandPayload::Response::Empty) {
        return {};
    }

    if (trans.response != CommandPayload::Response::Ok) {
        throw std::runtime_error("CommandQueue fetch failed");
    }

    sc_core::wait(delay);
    return std::move(trans.commands);
}

std::vector<uint8_t> CommandProcessor::read_input_buffer_bytes(uint64_t address,
                                                               size_t length) {
    // since tlm length is an unsigned int (32bit) and the length parameter is size_t
    // (64bit) a length check is needed
    if (length > std::numeric_limits<unsigned int>::max()) {
        throw std::length_error("TLM transfer length exceeds unsigned int");
    }

    tlm::tlm_generic_payload trans;
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

    std::vector<uint8_t> data(length);

    trans.set_command(tlm::TLM_READ_COMMAND);
    trans.set_address(address);
    trans.set_data_ptr(data.data());
    trans.set_data_length(length);
    trans.set_streaming_width(length);
    trans.set_byte_enable_ptr(nullptr);
    trans.set_dmi_allowed(false);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    input_buffer_socket->b_transport(trans, delay);

    if (trans.is_response_error()) {
        SC_REPORT_ERROR("CommandProcessor", "TLM read failed");
    }

    sc_core::wait(delay);

    return data;
}

void CommandProcessor::clear_frame_buffer() {
    FrameBufferPayload trans;
    trans.operation = FrameBufferPayload::Operation::Clear;
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
    frame_buffer_socket->b_transport(trans, delay);

    if (trans.is_response_error()) {
        throw std::runtime_error("FrameBuffer clear failed");
    }

    sc_core::wait(delay);
}

void CommandProcessor::write_frame_buffer_bytes(uint64_t address,
                                                std::span<const uint8_t> data) {
    // silently terminate an empty write
    if (data.empty()) {
        return;
    }

    // since tlm length is an unsigned int (32bit) and the length parameter is size_t
    // (64bit) a length check is needed
    if (data.size() > std::numeric_limits<unsigned int>::max()) {
        throw std::length_error("TLM transfer length exceeds unsigned int");
    }

    FrameBufferPayload trans;
    trans.operation = FrameBufferPayload::Operation::Write;
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
    const auto length = static_cast<unsigned int>(data.size());
    std::vector<uint8_t> payload_data(data.begin(), data.end());

    trans.set_command(tlm::TLM_WRITE_COMMAND);
    trans.set_address(address);
    trans.set_data_ptr(payload_data.data());
    trans.set_data_length(length);
    trans.set_streaming_width(length);
    trans.set_byte_enable_ptr(nullptr);
    trans.set_dmi_allowed(false);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    frame_buffer_socket->b_transport(trans, delay);

    if (trans.is_response_error()) {
        SC_REPORT_ERROR("CommandProcessor", "TLM write failed");
    }

    sc_core::wait(delay);
}

void CommandProcessor::process_command(const Command& command) {
    sc_core::wait(decode_latency_);

    std::visit(
        [this](const auto& cmd) {
            // get command type without const and reference
            // decltype: determine cmd type at compile time
            // decay_t: removes const and reference
            // using T: creates type alias such that T can be used as type
            using T = std::decay_t<decltype(cmd)>;

            if constexpr (std::is_same_v<T, CommandClearFrameBuffer>) {
                clear_frame_buffer();
            } else if constexpr (std::is_same_v<T, CommandDrawTriangle>) {
                const auto bytes = read_input_buffer_bytes(
                    cmd.vertices_2d_start_address, 3 * VERTEX_2D_SIZE);
                const std::span<const uint8_t> data{bytes};

                const auto v0 = decode_vertex_2d(data.subspan(0, VERTEX_2D_SIZE));
                const auto v1 =
                    decode_vertex_2d(data.subspan(VERTEX_2D_SIZE, VERTEX_2D_SIZE));
                const auto v2 =
                    decode_vertex_2d(data.subspan(2 * VERTEX_2D_SIZE, VERTEX_2D_SIZE));

                auto rasterized_scanlines = rasterize_triangle(
                    v0, v1, v2, frame_buffer_width_, frame_buffer_height_);

                for (auto& rasterized_scanline : rasterized_scanlines) {
                    // compute frame buffer address
                    const auto y = rasterized_scanline.y;
                    const auto x_start = rasterized_scanline.x_start;

                    // check if scanline is out of bound of frame buffer
                    if (y >= frame_buffer_height_ || x_start >= frame_buffer_width_ ||
                        rasterized_scanline.pixels.empty()) {
                        continue;
                    }

                    // clip the run to the remaining pixels in this row.
                    const auto available_pixels = frame_buffer_width_ - x_start;
                    if (rasterized_scanline.pixels.size() > available_pixels) {
                        rasterized_scanline.pixels.resize(available_pixels);
                    }

                    const uint64_t address = y * frame_buffer_width_ * RGB_COLOR_SIZE +
                                             x_start * RGB_COLOR_SIZE;

                    // encode rasterized scanline
                    const auto scanline_bytes =
                        encode_rasterized_scanline(rasterized_scanline);
                    write_frame_buffer_bytes(address, scanline_bytes);
                }
            }
        },
        command);
}
