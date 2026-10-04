#pragma once

#include <cstddef>
#include <cstdint>

class BufferInterface {
   public:
    virtual ~BufferInterface() = default;

    virtual void write(size_t address, uint8_t value) = 0;
    virtual uint8_t read(size_t address) = 0;
};
