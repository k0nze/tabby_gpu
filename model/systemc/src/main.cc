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
#include "triangle_rasterizer.h"

#include <array>
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
        TriangleRasterizer triangle_rasterizer(
            "triangle_rasterizer", config.triangle_rasterizer, frame_buffer.get_width(),
            frame_buffer.get_height());
        CommandQueue cmd_queue("cmd_queue", config.command_queue);
        CommandProcessor cmd_proc("cmd_proc", config.command_processor,
                                  triangle_rasterizer, frame_buffer.get_width(),
                                  frame_buffer.get_height());
        TabbyTLMModel tabby_tlm_model("tabby_tlm_model", cmd_proc);

        cmd_proc.input_buffer_socket.bind(input_buffer.socket);
        cmd_proc.frame_buffer_socket.bind(frame_buffer.socket);
        cmd_proc.command_queue_socket.bind(cmd_queue.socket);

        constexpr uint64_t triangle_address = 0x100;

        const std::array<Triangle2D, 4> triangles{
            Triangle2D{{600, 80, RGB_RED}, {160, 360, RGB_GREEN}, {480, 360, RGB_BLUE}},
            Triangle2D{
                {40, 40, RGB_YELLOW}, {220, 60, RGB_CYAN}, {80, 220, RGB_MAGENTA}},
            Triangle2D{
                {360, 180, RGB_WHITE}, {520, 320, RGB_RED}, {580, 200, RGB_BLUE}},
            Triangle2D{
                {-40, 380, RGB_CYAN}, {180, 420, RGB_GREEN}, {60, 520, RGB_YELLOW}},
        };
        cmd_queue.push(CommandClearFrameBuffer{});

        for (std::size_t index = 0; index < triangles.size(); ++index) {
            const auto triangle_bytes = encode_triangle_2d(triangles[index]);
            const auto address = triangle_address + index * triangle_bytes.size();
            for (std::size_t i = 0; i < triangle_bytes.size(); ++i) {
                input_buffer.write(address + i, triangle_bytes[i]);
            }
            cmd_queue.push(CommandDrawTriangle{address});
        }

        sc_core::sc_start();

        frame_buffer.export_png("frame.png");

    } catch (const std::exception& error) {
        std::cerr << "Model initialization or simulation failed: " << error.what()
                  << '\n';
        return 1;
    }

    return 0;
}
