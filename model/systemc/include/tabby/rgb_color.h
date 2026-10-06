#pragma once

#include <cstddef>
#include <cstdint>

inline constexpr std::size_t RGB_COLOR_SIZE = 3;

struct RGBColor {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};
