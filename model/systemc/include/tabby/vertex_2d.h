#pragma once

#include <cstddef>
#include <cstdint>

#include "rgb_color.h"

inline constexpr std::size_t VERTEX_2D_SIZE = 2 * 4 + RGB_COLOR_SIZE;

struct Vertex2D {
    uint32_t x;
    uint32_t y;
    RGBColor color;
};
