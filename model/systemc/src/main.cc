#include <cstdint>
#include <systemc>
#include <vector>

#include "command_processor.h"
#include "input_buffer.h"

int sc_main(int argc, char* argv[]) {
    InputBuffer input_buffer("input_buffer", 1024);
    CommandProcessor cmd_proc("cmd_proc");

    cmd_proc.socket.bind(input_buffer.socket);

    input_buffer.write(0x100, 42);
    input_buffer.write(0x101, 23);
    input_buffer.write(0x102, 67);

    std::vector<uint8_t> data = cmd_proc.read_bytes(0x100, 3);

    for (auto byte : data) {
        std::cout << static_cast<int>(byte) << std::endl;
    }

    sc_core::sc_start();

    return 0;
}
