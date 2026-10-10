#pragma once

#include "command_payload.h"
#include "commands.h"
#include "frame_buffer_payload.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <sysc/kernel/sc_module_name.h>
#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <vector>

class CommandProcessor : public sc_core::sc_module {
   public:
    tlm_utils::simple_initiator_socket<CommandProcessor> input_buffer_socket;
    tlm_utils::simple_initiator_socket<CommandProcessor, 32, FrameBufferProtocolTypes>
        frame_buffer_socket;
    tlm_utils::simple_initiator_socket<CommandProcessor, 32, CommandProtocolTypes>
        command_queue_socket;

    CommandProcessor(sc_core::sc_module_name name, size_t frame_buffer_size_bytes);

    std::vector<Command> read_commands(size_t request_count);
    std::vector<uint8_t> read_input_buffer_bytes(uint64_t address, size_t length);
    void clear_frame_buffer();
    void write_frame_buffer_bytes(uint64_t address, std::span<const uint8_t> data);

    void process_command(const Command& command);

   private:
    size_t frame_buffer_size_bytes_;
};
