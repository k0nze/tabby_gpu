#include "tabby_tlm_model.h"

#include "command_processor.h"

#include <sysc/kernel/sc_simcontext.h>
#include <systemc>

TabbyTLMModel::TabbyTLMModel(sc_core::sc_module_name name, CommandProcessor& cmd_proc)
    : sc_core::sc_module(name), cmd_proc_(cmd_proc) {
    SC_THREAD(run);
}

void TabbyTLMModel::run() {
    auto start_time = sc_core::sc_time_stamp();

    auto data = cmd_proc_.read_bytes(0x100, 3);

    for (auto byte : data) {
        std::cout << static_cast<int>(byte) << std::endl;
    }

    sc_core::sc_stop();

    std::cout << "latency: " << sc_core::sc_time_stamp() - start_time << std::endl;
}
