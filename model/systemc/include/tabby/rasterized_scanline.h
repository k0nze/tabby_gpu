#pragma once

#include "rgb_color.h"

#include <cstddef>
#include <cstdint>
#include <vector>

// A contiguous run of covered pixels, ordered from left to right.
struct RasterizedScanline {
    std::size_t y;
    std::size_t x_start;
    std::vector<RGBColor> pixels;
};

// Encode pixels as consecutive RGB bytes; coordinates are not included.
std::vector<uint8_t> encode_rasterized_scanline(const RasterizedScanline& scanline);
