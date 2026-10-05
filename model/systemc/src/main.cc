#include <iostream>
#include <string>
#include <systemc>

#include "CLI/CLI.hpp"
#include "command_processor.h"
#include "config_loader.h"
#include "input_buffer.h"
#include "tabby_tlm_model.h"

int sc_main(int argc, char* argv[]) {
    CLI::App app{"Tabby TLM Model"};

    std::string config_path;
    app.add_option("-c,--config", config_path, "Configuration YAML file")
        ->required()
        ->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);

    try {
        const auto config = load_input_buffer_config(config_path);
        InputBuffer input_buffer("input_buffer", config);
        CommandProcessor cmd_proc("cmd_proc");
        TabbyTLMModel tabby_tlm_model("tabby_tlm_model", cmd_proc);

        cmd_proc.socket.bind(input_buffer.socket);

        input_buffer.write(0x100, 42);
        input_buffer.write(0x101, 23);
        input_buffer.write(0x102, 67);

        sc_core::sc_start();
    } catch (const std::exception& error) {
        std::cerr << "Model initialization or simulation failed: " << error.what()
                  << '\n';
        return 1;
    }

    return 0;
}
