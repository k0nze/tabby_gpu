#include <systemc>

#include "command_processor.h"
#include "input_buffer.h"
#include "tabby_tlm_model.h"

int sc_main(int argc, char* argv[]) {
    InputBuffer input_buffer("input_buffer", 1024);
    CommandProcessor cmd_proc("cmd_proc");
    TabbyTLMModel tabby_tlm_model("tabby_tlm_model", cmd_proc);

    cmd_proc.socket.bind(input_buffer.socket);

    input_buffer.write(0x100, 42);
    input_buffer.write(0x101, 23);
    input_buffer.write(0x102, 67);

    sc_core::sc_start();

    return 0;
}
