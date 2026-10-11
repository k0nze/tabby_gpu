#pragma once

#include "rasterized_scanline.h"
#include "vertex_2d.h"

#include <cstddef>
#include <optional>
#include <sysc/kernel/sc_module.h>
#include <sysc/kernel/sc_module_name.h>
#include <vector>

struct TriangleRasterizerConfig {
    size_t scanline_fifo_capacity_bytes;
    uint64_t clock_freq_hz;
    uint64_t setup_cycles;
    uint64_t pipeline_latency_cycles;
    uint64_t pixel_input_interval_cycles;
};

// Return covered scanlines in increasing y order, clipped to the framebuffer.
// The command processor handles packing pixels and timed framebuffer writes.
std::vector<RasterizedScanline> rasterize_triangle(const Vertex2D& v0,
                                                   const Vertex2D& v1,
                                                   const Vertex2D& v2,
                                                   std::size_t frame_buffer_width,
                                                   std::size_t frame_buffer_height);

class TriangleRasterizer : public sc_core::sc_module {
   public:
    SC_HAS_PROCESS(TriangleRasterizer);

    TriangleRasterizer(sc_core::sc_module_name name,
                       const TriangleRasterizerConfig& config,
                       size_t frame_buffer_width, size_t frame_buffer_height);

    void submit(const Vertex2D& a, const Vertex2D& b, const Vertex2D& c);
    std::optional<RasterizedScanline> next_scanline();

   private:
    void run();
};
