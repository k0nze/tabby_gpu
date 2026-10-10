#pragma once

#include "buffer_interface.h"
#include "vertex_2d.h"

#include <array>
#include <cstdint>
#include <span>

std::array<uint8_t, VERTEX_2D_SIZE> encode_vertex_2d(const Vertex2D& vertex_2d);
Vertex2D decode_vertex_2d(std::span<const uint8_t> data);

void write_vertex_2d(BufferInterface& buffer, uint64_t address,
                     const Vertex2D& vertex_2d);
Vertex2D read_vertex_2d(BufferInterface& buffer, uint64_t address);
