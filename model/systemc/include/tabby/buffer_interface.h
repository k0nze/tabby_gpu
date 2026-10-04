#pragma once

#include <cstddef>
#include <cstdint>

class BufferInterface {
   public:
    virtual ~BufferInterface() = default;

    virtual void write(uint64_t address, uint8_t value) = 0;
    virtual uint8_t read(uint64_t address) = 0;
};
