#pragma once

#include "triangle_2d.h"

#include <array>
#include <cstdint>
#include <span>

std::array<uint8_t, 3 * VERTEX_2D_SIZE> encode_triangle_2d(const Triangle2D& triangle);

Triangle2D decode_triangle_2d(std::span<const uint8_t> data);
