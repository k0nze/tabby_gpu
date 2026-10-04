#include "input_buffer.h"

#include <stdexcept>
#include <vector>

InputBuffer::InputBuffer(size_t size) : data_(size) {}

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
