#include <cstddef>
#include <cstdint>
#include <vector>

class InputBuffer {
   public:
    explicit InputBuffer(std::size_t size);

    void write(std::size_t address, uint8_t value);
    uint8_t read(std::size_t address);

   private:
    std::vector<uint8_t> data_;
};
