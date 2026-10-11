#pragma once

#include "rasterized_scanline.h"
#include "triangle_2d.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <sysc/kernel/sc_module.h>
#include <sysc/kernel/sc_module_name.h>
#include <systemc>
#include <vector>

struct TriangleRasterizerConfig {
    size_t scanline_fifo_capacity_bytes;
    uint64_t clock_freq_hz;
    uint64_t setup_cycles;
    uint64_t pipeline_latency_cycles;
    uint64_t pixel_input_interval_cycles;
};

// Inclusive {min_x, min_y, max_x, max_y}; nullopt means no framebuffer overlap.
std::optional<std::array<int64_t, 4>> compute_triangle_bounds(
    const Triangle2D& triangle, std::size_t frame_buffer_width,
    std::size_t frame_buffer_height);

// Return covered scanlines in increasing y order, clipped to the framebuffer.
// The command processor handles packing pixels and timed framebuffer writes.
std::vector<RasterizedScanline> rasterize_triangle(const Triangle2D& triangle,
                                                   std::size_t frame_buffer_width,
                                                   std::size_t frame_buffer_height);

/*
 CommandProcessor                       TriangleRasterizer::run()
 (consumer thread)                      (producer thread)
        |                                      |
        | submit(): wait(idle_) while busy_     |
        | pending_triangle_ = triangle         |
        |---- notify(work_available_) -------->|
        |                               setup wait; compute geometry
        |                               wait for row ready_time
        |                                      |
        |                     +----------------v----------------+
        |                     | scanlines_output_fifo_          |
        |                     | usage: queued_pixel_bytes_      |
        |                     | limit: config_.                 |
        |                     |   scanline_fifo_capacity_bytes  |
        |                     +----------------+----------------+
        |<---- notify(output_available_) ------|
        | next_scanline(): dequeue             |
        | decrease queued_pixel_bytes_         |
        |---- notify(space_available_) ------->| wait if FIFO full
        |                                      |
        |---- timed write ----> FrameBuffer    |
        | wait(delay)                          | release next row
        |          ...                         |
        |                               wait for finish_time
        |                               output_complete_ = true
        |<---- notify(output_available_) ------|
        | next_scanline(): consume remaining rows
        | FIFO empty && output_complete_       |
        | busy_ = false; notify(idle_)          |
        | return nullopt                       |
        |                                      |
   triangle command complete              ready for submit()
*/
class TriangleRasterizer : public sc_core::sc_module {
   public:
    SC_HAS_PROCESS(TriangleRasterizer);

    TriangleRasterizer(sc_core::sc_module_name name,
                       const TriangleRasterizerConfig& config,
                       size_t frame_buffer_width, size_t frame_buffer_height);

    void submit(const Triangle2D& triangle);
    std::optional<RasterizedScanline> next_scanline();

   private:
    void run();

    const TriangleRasterizerConfig config_;
    const std::size_t frame_buffer_width_;
    const std::size_t frame_buffer_height_;
    sc_core::sc_time clock_period_;
    sc_core::sc_time pipeline_latency_;
    sc_core::sc_time pixel_input_interval_;

    std::optional<Triangle2D> pending_triangle_;
    bool busy_ = false;
    bool output_complete_ = false;

    std::deque<RasterizedScanline> scanlines_output_fifo_;
    std::size_t queued_pixel_bytes_ = 0;

    sc_core::sc_event work_available_;
    sc_core::sc_event output_available_;
    sc_core::sc_event space_available_;
    sc_core::sc_event idle_;
};
