#include "triangle_2d_io.h"
#include "vertex_2d_io.h"

#include <algorithm>
#include <stdexcept>

std::array<uint8_t, 3 * VERTEX_2D_SIZE> encode_triangle_2d(const Triangle2D& triangle) {
    std::array<uint8_t, 3 * VERTEX_2D_SIZE> data{};
    const auto v0 = encode_vertex_2d(triangle.v0);
    const auto v1 = encode_vertex_2d(triangle.v1);
    const auto v2 = encode_vertex_2d(triangle.v2);

    std::copy(v0.begin(), v0.end(), data.begin());
    std::copy(v1.begin(), v1.end(), data.begin() + VERTEX_2D_SIZE);
    std::copy(v2.begin(), v2.end(), data.begin() + 2 * VERTEX_2D_SIZE);
    return data;
}

Triangle2D decode_triangle_2d(std::span<const uint8_t> data) {
    if (data.size() < 3 * VERTEX_2D_SIZE) {
        throw std::invalid_argument("Triangle2D requires 33 bytes");
    }

    return {
        decode_vertex_2d(data.subspan(0, VERTEX_2D_SIZE)),
        decode_vertex_2d(data.subspan(VERTEX_2D_SIZE, VERTEX_2D_SIZE)),
        decode_vertex_2d(data.subspan(2 * VERTEX_2D_SIZE, VERTEX_2D_SIZE)),
    };
}
