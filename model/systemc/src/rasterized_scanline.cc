#include "rasterized_scanline.h"

#include <stdexcept>

std::vector<uint8_t> encode_rasterized_scanline(const RasterizedScanline& scanline) {
    std::vector<uint8_t> data;
    if (scanline.pixels.size() > data.max_size() / RGB_COLOR_SIZE) {
        throw std::length_error("Rasterized scanline exceeds byte vector capacity");
    }
    data.reserve(scanline.pixels.size() * RGB_COLOR_SIZE);

    for (const auto& pixel : scanline.pixels) {
        data.push_back(pixel.red);
        data.push_back(pixel.green);
        data.push_back(pixel.blue);
    }

    return data;
}
