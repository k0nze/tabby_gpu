#include "frame_buffer.h"
#include "frame_buffer_payload.h"
#include "rgb_color.h"
#include "timing.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>
#include <vector>

FrameBuffer::FrameBuffer(sc_core::sc_module_name name, const FrameBufferConfig& config)
    : sc_core::sc_module(name), socket("socket"), config_(config) {
    // check that config values are valid (greater than 0)
    if (config_.width < 1 || config_.height < 1 || config_.bytes_per_word < 1) {
        throw std::invalid_argument(
            "FrameBuffer width, height, and word size must be greater than zero");
    }

    // compute clock period
    clock_period_ = clock_period_from_hz(config_.clock_freq_hz);

    // compute latencies
    write_setup_latency_ =
        clock_period_ * static_cast<double>(config_.write_setup_cycles);
    write_word_latency_ =
        clock_period_ * static_cast<double>(config_.write_cycles_per_word);
    data_.resize(config_.width * config_.height * RGB_COLOR_SIZE);

    socket.register_b_transport(this, &FrameBuffer::b_transport);
}

void FrameBuffer::clear() { std::fill(data_.begin(), data_.end(), uint8_t{0}); }

void FrameBuffer::write(uint64_t address, uint8_t value) {
    // check if address is in range
    if (address >= data_.size()) {
        throw std::out_of_range("FrameBuffer write");
    }

    data_[address] = value;
}

uint8_t FrameBuffer::read(uint64_t address) {
    // check if address is in range
    if (address >= data_.size()) {
        throw std::out_of_range("FrameBuffer read");
    }

    return data_[address];
}

size_t FrameBuffer::get_width() const { return config_.width; }

size_t FrameBuffer::get_height() const { return config_.height; }

std::vector<uint8_t> FrameBuffer::read_whole_buffer() const {
    // returns copy
    return data_;
}

void FrameBuffer::b_transport(FrameBufferPayload& trans, sc_core::sc_time& delay) {
    if (trans.operation == FrameBufferPayload::Operation::Clear) {
        clear();

        delay += clock_period_ * static_cast<double>(config_.clear_cycles);
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
        return;
    }

    if (trans.operation != FrameBufferPayload::Operation::Write) {
        trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
        return;
    }

    uint64_t address = trans.get_address();
    size_t length = trans.get_data_length();
    uint8_t* data = trans.get_data_ptr();

    // check if the data container points to a valid reference and that the transfer
    // length is non-zero
    if (data == nullptr || length == 0) {
        trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return;
    }

    // only consecutive accesses are allowed
    if (trans.get_byte_enable_ptr() != nullptr) {
        trans.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
        return;
    }

    // only non-wrapping accesses are allowed
    if (trans.get_streaming_width() < length) {
        trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
        return;
    }

    // check if access is out of bounds
    if (address >= data_.size() || length > data_.size() - address) {
        trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        return;
    }

    for (size_t i = 0; i < length; i++) {
        write(address + i, data[i]);
    }

    const size_t words =
        length / config_.bytes_per_word + (length % config_.bytes_per_word != 0);
    delay += write_setup_latency_ + write_word_latency_ * static_cast<double>(words);

    trans.set_response_status(tlm::TLM_OK_RESPONSE);
}
