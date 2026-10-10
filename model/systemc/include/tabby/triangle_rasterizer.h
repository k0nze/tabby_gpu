#pragma once

#include "rasterized_scanline.h"
#include "vertex_2d.h"

#include <cstddef>
#include <vector>

// Return covered scanlines in increasing y order, clipped to the framebuffer.
// The command processor handles packing pixels and timed framebuffer writes.
std::vector<RasterizedScanline> rasterize_triangle(const Vertex2D& v0,
                                                   const Vertex2D& v1,
                                                   const Vertex2D& v2,
                                                   std::size_t frame_buffer_width,
                                                   std::size_t frame_buffer_height);
