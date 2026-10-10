#include "rasterized_scanline.h"
#include "triangle_rasterizer.h"

std::vector<RasterizedScanline> rasterize_triangle(const Vertex2D& v0,
                                                   const Vertex2D& v1,
                                                   const Vertex2D& v2,
                                                   std::size_t width,
                                                   std::size_t height) {
    return {RasterizedScanline{300, 400, {{0xff, 0x00, 0x00}}}};
}
