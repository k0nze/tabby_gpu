#pragma once

#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>
#include <tlm_utils/simple_target_socket.h>

#include <cstddef>
#include <cstdint>
#include <systemc>
#include <tlm>
#include <vector>

#include "buffer_interface.h"

struct InputBufferConfig {
    uint64_t clock_freq_hz;
    uint64_t read_setup_cycles;
    uint64_t read_cycles_per_word;
    size_t bytes_per_word;
};

class InputBuffer : public sc_core::sc_module, public BufferInterface {
   public:
    tlm_utils::simple_target_socket<InputBuffer> socket;

    InputBuffer(sc_core::sc_module_name name, size_t size);

    void write(uint64_t address, uint8_t value) override;
    uint8_t read(uint64_t address) override;

   private:
    std::vector<uint8_t> data_;
    void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);

    const sc_core::sc_time transaction_latency_{10, sc_core::SC_NS};
    const sc_core::sc_time per_word_latency_{10, sc_core::SC_NS};

    static constexpr size_t bytes_per_word_ = 4;
};
