#pragma once

#include "command_payload.h"
#include "commands.h"

#include <cstddef>
#include <queue>
#include <sysc/kernel/sc_module.h>
#include <sysc/kernel/sc_module_name.h>
#include <sysc/kernel/sc_time.h>
#include <systemc>
#include <tlm>
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>
#include <tlm_utils/simple_target_socket.h>

struct CommandQueueConfig {
    size_t size;
    uint64_t clock_freq_hz;
    uint64_t write_setup_cycles;
    uint64_t write_cycles_per_command;
    uint64_t read_setup_cycles;
    uint64_t read_cycles_per_command;
};

class CommandQueue : public sc_core::sc_module {
   public:
    tlm_utils::simple_target_socket<CommandQueue, 32, CommandProtocolTypes> socket;

    CommandQueue(sc_core::sc_module_name name, const CommandQueueConfig& config);

    void push(const Command& command);
    Command pop();
    bool empty() const;

   private:
    std::queue<Command> queue_;
    void b_transport(CommandPayload& trans, sc_core::sc_time& delay);

    const CommandQueueConfig config_;

    sc_core::sc_time clock_period_;
    sc_core::sc_time write_setup_latency_;
    sc_core::sc_time write_command_latency_;
    sc_core::sc_time read_setup_latency_;
    sc_core::sc_time read_command_latency_;
};
