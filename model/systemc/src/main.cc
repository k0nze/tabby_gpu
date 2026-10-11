#include "CLI/CLI.hpp"
#include "command_processor.h"
#include "command_queue.h"
#include "commands.h"
#include "config_loader.h"
#include "frame_buffer.h"
#include "input_buffer.h"
#include "rgb_color.h"
#include "tabby_tlm_model.h"
#include "triangle_2d_io.h"

#include <iostream>
#include <string>
#include <systemc>

int sc_main(int argc, char* argv[]) {
    CLI::App app{"Tabby TLM Model"};

    std::string config_path;
    app.add_option("-c,--config", config_path, "Configuration YAML file")
        ->required()
        ->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);

    try {
        const auto config = load_config(config_path);
        InputBuffer input_buffer("input_buffer", config.input_buffer);
        FrameBuffer frame_buffer("frame_buffer", config.frame_buffer);
        CommandQueue cmd_queue("cmd_queue", config.command_queue);
        CommandProcessor cmd_proc("cmd_proc", config.command_processor,
                                  frame_buffer.get_width(), frame_buffer.get_height());
        TabbyTLMModel tabby_tlm_model("tabby_tlm_model", cmd_proc);

        cmd_proc.input_buffer_socket.bind(input_buffer.socket);
        cmd_proc.frame_buffer_socket.bind(frame_buffer.socket);
        cmd_proc.command_queue_socket.bind(cmd_queue.socket);

        constexpr uint64_t triangle_address = 0x100;

        const Triangle2D triangle{
            Vertex2D{600, 80, RGB_RED},
            Vertex2D{160, 360, RGB_GREEN},
            Vertex2D{480, 360, RGB_BLUE},
        };
        const auto triangle_bytes = encode_triangle_2d(triangle);
        for (std::size_t i = 0; i < triangle_bytes.size(); ++i) {
            input_buffer.write(triangle_address + i, triangle_bytes[i]);
        }

        cmd_queue.push(CommandClearFrameBuffer{});
        cmd_queue.push(CommandDrawTriangle{triangle_address});

        sc_core::sc_start();

        frame_buffer.export_png("frame.png");

    } catch (const std::exception& error) {
        std::cerr << "Model initialization or simulation failed: " << error.what()
                  << '\n';
        return 1;
    }

    return 0;
}
