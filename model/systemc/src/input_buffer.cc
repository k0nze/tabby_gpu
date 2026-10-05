#include "input_buffer.h"

#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>

#include <cstddef>
#include <limits>
#include <stdexcept>

InputBuffer::InputBuffer(sc_core::sc_module_name name, const InputBufferConfig& config)
    : sc_core::sc_module(name), socket("socket"), config_(config) {
    if (config_.capacity_bytes == 0 || config_.clock_freq_hz == 0 ||
        config_.bytes_per_word == 0) {
        throw std::invalid_argument(
            "InputBuffer capacity, frequency, and word size must be nonzero");
    }
    const sc_core::sc_time clock_period(
        1.0 / static_cast<double>(config_.clock_freq_hz), sc_core::SC_SEC);
    if (clock_period == sc_core::SC_ZERO_TIME) {
        throw std::invalid_argument(
            "InputBuffer clock period is below the simulation time resolution");
    }
    read_setup_latency_ = clock_period * static_cast<double>(config_.read_setup_cycles);
    read_word_latency_ =
        clock_period * static_cast<double>(config_.read_cycles_per_word);
    data_.resize(config_.capacity_bytes);
    socket.register_b_transport(this, &InputBuffer::b_transport);
}

void InputBuffer::write(uint64_t address, uint8_t value) {
    // check if address is in range
    if (address >= data_.size()) {
        throw std::out_of_range("InputBuffer write");
    }

    data_[address] = value;
}

uint8_t InputBuffer::read(uint64_t address) {
    // check if address is in range
    if (address >= data_.size()) {
        throw std::out_of_range("InputBuffer read");
    }

    return data_[address];
}

void InputBuffer::b_transport(tlm::tlm_generic_payload& trans,
                              sc_core::sc_time& delay) {
    uint64_t address = trans.get_address();
    size_t length = trans.get_data_length();
    uint8_t* data = trans.get_data_ptr();

    // check if the data container points to a valid reference and that the transfer
    // length is non-zero
    if (data == nullptr || length == 0) {
        trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return;
    }

    // only consecutive accesses are allowed
    if (trans.get_byte_enable_ptr() != nullptr) {
        trans.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
        return;
    }

    // only non-wrapping accesses are allowed
    if (trans.get_streaming_width() < length) {
        trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
        return;
    }

    // check if access is out of bounds
    if (address >= data_.size() || length > data_.size() - address) {
        trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        return;
    }

    if (trans.is_read()) {
        for (size_t i = 0; i < length; i++) {
            data[i] = read(address + i);
        }

        const size_t words =
            length / config_.bytes_per_word + (length % config_.bytes_per_word != 0);
        delay += read_setup_latency_ + read_word_latency_ * static_cast<double>(words);

    } else if (trans.is_write()) {
        for (size_t i = 0; i < length; i++) {
            write(address + i, data[i]);
        }

        const size_t words =
            length / config_.bytes_per_word + (length % config_.bytes_per_word != 0);
        delay += read_setup_latency_ + read_word_latency_ * static_cast<double>(words);

    } else {
        trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
        return;
    }

    trans.set_response_status(tlm::TLM_OK_RESPONSE);
}
