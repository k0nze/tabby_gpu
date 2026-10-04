#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "buffer_interface.h"

class InputBuffer : public BufferInterface {
   public:
    explicit InputBuffer(size_t size);

    void write(size_t address, uint8_t value);
    uint8_t read(size_t address);

   private:
    std::vector<uint8_t> data_;
};
