#include "command_processor.h"

#include <iostream>
#include <type_traits>
#include <variant>

#include "vertex_2d_io.h"

CommandProcessor::CommandProcessor(CommandQueue& cmd_queue, InputBuffer& input_buffer)
    : cmd_queue_(cmd_queue), input_buffer_(input_buffer) {}

void CommandProcessor::process_next() {
    // check if command queue is empty
    if (cmd_queue_.empty()) {
        return;
    }

    auto command = cmd_queue_.pop();

    std::visit(
        [this](const auto& cmd) {
            using T = std::decay_t<decltype(cmd)>;

            if constexpr (std::is_same_v<T, CommandDrawTriangle>) {
                constexpr std::size_t vertex_size = 11;

                auto v0 = read_vertex_2d(input_buffer_, cmd.vertices_2d_start_address);
                auto v1 = read_vertex_2d(input_buffer_,
                                         cmd.vertices_2d_start_address + vertex_size);
                auto v2 = read_vertex_2d(
                    input_buffer_, cmd.vertices_2d_start_address + 2 * vertex_size);

                std::cout << "draw triangle" << std::endl;
            }
        },
        command);
}
