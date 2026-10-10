#include "vertex_2d.h"
#include "vertex_2d_io.h"

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>

std::array<uint8_t, VERTEX_2D_SIZE> encode_vertex_2d(const Vertex2D& vertex_2d) {
    std::array<uint8_t, VERTEX_2D_SIZE> data{};

    // copy position x,y
    const auto x = std::bit_cast<uint32_t>(vertex_2d.x);
    const auto y = std::bit_cast<uint32_t>(vertex_2d.y);
    for (size_t i = 0; i < 4; i++) {
        data[i] = static_cast<uint8_t>(x >> (8 * i));
        data[4 + i] = static_cast<uint8_t>(y >> (8 * i));
    }

    // copy RGB color data
    data[8] = vertex_2d.color.red;
    data[9] = vertex_2d.color.green;
    data[10] = vertex_2d.color.blue;

    return data;
}

Vertex2D decode_vertex_2d(std::span<const uint8_t> data) {
    // check size of span
    if (data.size() < VERTEX_2D_SIZE) {
        throw std::invalid_argument("Vertex2D requires 11 bytes");
    }

    Vertex2D v{};

    // copy position x,y
    const uint32_t x =
        static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8) |
        (static_cast<uint32_t>(data[2]) << 16) | (static_cast<uint32_t>(data[3]) << 24);
    const uint32_t y =
        static_cast<uint32_t>(data[4]) | (static_cast<uint32_t>(data[5]) << 8) |
        (static_cast<uint32_t>(data[6]) << 16) | (static_cast<uint32_t>(data[7]) << 24);
    v.x = std::bit_cast<int32_t>(x);
    v.y = std::bit_cast<int32_t>(y);

    // copy RGB color data
    v.color.red = data[8];
    v.color.green = data[9];
    v.color.blue = data[10];

    return v;
}

void write_vertex_2d(BufferInterface& buffer, uint64_t address,
                     const Vertex2D& vertex_2d) {
    // check address
    if (address > std::numeric_limits<uint64_t>::max() - (VERTEX_2D_SIZE - 1)) {
        throw std::out_of_range("Vertex2D address range overflows");
    }

    const auto data = encode_vertex_2d(vertex_2d);
    for (size_t i = 0; i < data.size(); i++) {
        buffer.write(address + i, data[i]);
    }
}

Vertex2D read_vertex_2d(BufferInterface& buffer, uint64_t address) {
    // check address
    if (address > std::numeric_limits<uint64_t>::max() - (VERTEX_2D_SIZE - 1)) {
        throw std::out_of_range("Vertex2D address range overflows");
    }

    std::array<uint8_t, VERTEX_2D_SIZE> data{};
    for (size_t i = 0; i < data.size(); i++) {
        data[i] = buffer.read(address + i);
    }

    return decode_vertex_2d(data);
}
