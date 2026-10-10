#pragma once

#include "buffer_interface.h"

#include <cstddef>
#include <cstdint>
#include <systemc>
#include <tlm>
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>
#include <tlm_utils/simple_target_socket.h>
#include <vector>

struct FrameBufferConfig {
    size_t width;
    size_t height;
    uint64_t clock_freq_hz;
    uint64_t write_setup_cycles;
    uint64_t write_cycles_per_word;
    size_t bytes_per_word;
};

class FrameBuffer : public sc_core::sc_module, public BufferInterface {
   public:
    tlm_utils::simple_target_socket<FrameBuffer> socket;

    FrameBuffer(sc_core::sc_module_name name, const FrameBufferConfig& config);

    void clear();
    void write(uint64_t address, uint8_t value) override;
    uint8_t read(uint64_t address) override;

    size_t get_width() const;
    size_t get_height() const;

    std::vector<uint8_t> read_whole_buffer() const;

   private:
    std::vector<uint8_t> data_;
    void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);

    const FrameBufferConfig config_;

    sc_core::sc_time clock_period_;
    sc_core::sc_time write_setup_latency_;
    sc_core::sc_time write_word_latency_;
};
