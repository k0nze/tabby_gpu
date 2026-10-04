#pragma once

#include <cstdint>

#include "rgb_color.h"

struct Vertex2D {
    uint32_t x;
    uint32_t y;
    RGBColor color;
};
