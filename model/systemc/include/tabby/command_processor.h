#pragma once

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>

class CommandProcessor : public sc_core::sc_module {
   public:
    tlm_utils::simple_initiator_socket<CommandProcessor> socket;
    SC_CTOR(CommandProcessor);

    std::vector<uint8_t> read_bytes(uint64_t address, size_t length);
};
