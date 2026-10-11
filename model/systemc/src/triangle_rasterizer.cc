#include "rasterized_scanline.h"
#include "rgb_color.h"
#include "timing.h"
#include "triangle_2d.h"
#include "triangle_rasterizer.h"
#include "vertex_2d.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <sysc/kernel/sc_module.h>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_time.h>
#include <sysc/kernel/sc_wait.h>
#include <utility>

std::optional<std::array<int64_t, 4>> compute_triangle_bounds(
    const Triangle2D& triangle, std::size_t frame_buffer_width,
    std::size_t frame_buffer_height) {
    if (frame_buffer_width == 0 || frame_buffer_height == 0) {
        return std::nullopt;
    }

    // clamp unsigned dimensions before converting to signed coordinates
    const auto last_x = static_cast<int64_t>(std::min<std::size_t>(
        frame_buffer_width - 1, std::numeric_limits<int32_t>::max()));
    const auto last_y = static_cast<int64_t>(std::min<std::size_t>(
        frame_buffer_height - 1, std::numeric_limits<int32_t>::max()));

    const auto& a = triangle.v0;
    const auto& b = triangle.v1;
    const auto& c = triangle.v2;

    const std::array<int64_t, 4> bounds{
        std::max<int64_t>(0, std::min({a.x, b.x, c.x})),
        std::max<int64_t>(0, std::min({a.y, b.y, c.y})),
        std::min<int64_t>(last_x, std::max({a.x, b.x, c.x})),
        std::min<int64_t>(last_y, std::max({a.y, b.y, c.y})),
    };

    const auto [min_x, min_y, max_x, max_y] = bounds;

    // check triangle lies outside of the frame buffer
    if (min_x > max_x || min_y > max_y) {
        return std::nullopt;
    }
    return bounds;
}

std::vector<RasterizedScanline> rasterize_triangle(const Triangle2D& triangle,
                                                   std::size_t frame_buffer_width,
                                                   std::size_t frame_buffer_height) {
    // local copy of vertices
    Vertex2D a = triangle.v0;
    Vertex2D b = triangle.v1;
    Vertex2D c = triangle.v2;

    // compute signed area
    // area2 == 0 vertices are collinear
    // area2 > 0 clockwise winding
    // area2 < 0 counter clockwise winding
    const int64_t dx1 = static_cast<int64_t>(b.x) - a.x;
    const int64_t dy1 = static_cast<int64_t>(b.y) - a.y;

    const int64_t dx2 = static_cast<int64_t>(c.x) - a.x;
    const int64_t dy2 = static_cast<int64_t>(c.y) - a.y;

    // compute the cross-product, which returns twice the triangles area because it
    // computes the parallelo gram area
    const int64_t area2 = dx1 * dy2 - dy1 * dx2;

    if (area2 == 0) {
        return {};
    }

    // normalize winding such that the area becomes positive
    if (area2 < 0) {
        std::swap(b, c);
    }

    const float pos_area2 = std::abs(static_cast<float>(area2));

    const auto bounds =
        compute_triangle_bounds(triangle, frame_buffer_width, frame_buffer_height);
    if (!bounds) {
        return {};
    }
    const auto [min_x, min_y, max_x, max_y] = *bounds;

    /*
                     a
                    / \
                   /   \
             c->a /  P  \ a->b
                 /       \
                c---------b
                    b->c

       P is a pixel center (x + 0.5, y + 0.5).
       Test P against each directed edge: a->b, b->c, and c->a.
       With positive winding and screen y increasing downward, all three
       edge values positive means inside. Any negative value means outside.
       A zero value requires the edge-inclusion rule.
    */
    const auto edge = [](const Vertex2D& from, const Vertex2D& to, float x, float y) {
        const float dx = static_cast<float>(to.x) - from.x;
        const float dy = static_cast<float>(to.y) - from.y;

        return dx * (y - from.y) - dy * (x - from.x);
    };

    // top-left rule:
    // a pixel is inside the triangle if it above and left of and edge otherwise it is
    // excluded
    const auto is_top_left = [](const Vertex2D& from, const Vertex2D& to) {
        return to.y < from.y || (to.y == from.y && to.x > from.x);
    };

    // using the top-left rule, for each edge it is decided if pixesl exactly on that
    // edge should be included in the triangle drawing or not
    const bool include_ab = is_top_left(a, b);
    const bool include_bc = is_top_left(b, c);
    const bool include_ca = is_top_left(c, a);

    std::vector<RasterizedScanline> scanlines;

    // loop over all positions in the bounding rectangle
    for (int64_t y = min_y; y <= max_y; y++) {
        RasterizedScanline scanline{static_cast<std::size_t>(y), 0, {}};
        for (int64_t x = min_x; x <= max_x; x++) {
            // move the test pixel from the corner to the center
            // e.g. (10,20) occupies a square from (10, 20) to (11, 21)
            // -> center at (10.5, 20.5)
            const float px = static_cast<float>(x) + 0.5f;
            const float py = static_cast<float>(y) + 0.5f;

            // edge *->* and the pixel center form a triangle
            // sign e_*: determines inclusion
            // magnitude e_*: is two times the area of e_* and the pixel center because
            // the function computes the parallelo gram area through the cross-product
            const float e_ab = edge(a, b, px, py);
            const float e_bc = edge(b, c, px, py);
            const float e_ca = edge(c, a, px, py);

            // check for each pixel if is either "above" edge (e_* > 0)
            // or if it is on the edge (e_* == 0) if it is top-left
            const bool inside = (e_ab > 0 || (e_ab == 0 && include_ab)) &&
                                (e_bc > 0 || (e_bc == 0 && include_bc)) &&
                                (e_ca > 0 || (e_ca == 0 && include_ca));

            if (inside) {
                // check if it is the first pixel to be added to the scanline
                if (scanline.pixels.empty()) {
                    scanline.x_start = static_cast<std::size_t>(x);
                }

                // barycentric color interpolation
                // compute the weights for each vertex
                const float weight_a = e_bc / pos_area2;
                const float weight_b = e_ca / pos_area2;
                const float weight_c = e_ab / pos_area2;

                // red = weight_a * a.color.red + weight_b * b.color.red + weight_c *
                // c.color.red green = weight_a * a.color.green + weight_b *
                // b.color.green + weight_c * c.color.green blue = weight_a *
                // a.color.blue + weight_b * b.color.blue + weight_c * c.color.blue
                const auto interpolate = [&](uint8_t color_a, uint8_t color_b,
                                             uint8_t color_c) {
                    const float value =
                        weight_a * color_a + weight_b * color_b + weight_c * color_c;
                    return static_cast<uint8_t>(
                        std::clamp(std::lround(value), 0L, 255L));
                };

                scanline.pixels.push_back(RGBColor{
                    interpolate(a.color.red, b.color.red, c.color.red),
                    interpolate(a.color.green, b.color.green, c.color.green),
                    interpolate(a.color.blue, b.color.blue, c.color.blue),
                });
            }
        }

        // if the whole scanline is not empty add it to the scanlines vector
        if (!scanline.pixels.empty()) {
            scanlines.push_back(std::move(scanline));
        }
    }

    return scanlines;
}

TriangleRasterizer::TriangleRasterizer(sc_core::sc_module_name name,
                                       const TriangleRasterizerConfig& config,
                                       size_t frame_buffer_width,
                                       size_t frame_buffer_height)
    : sc_core::sc_module(name),
      config_(config),
      frame_buffer_width_(frame_buffer_width),
      frame_buffer_height_(frame_buffer_height) {
    // compute clock period
    clock_period_ = clock_period_from_hz(config_.clock_freq_hz);

    // Convert fixed pipeline timing from cycles once during construction.
    pipeline_latency_ =
        clock_period_ * static_cast<float>(config_.pipeline_latency_cycles);
    pixel_input_interval_ =
        clock_period_ * static_cast<float>(config_.pixel_input_interval_cycles);

    SC_THREAD(run);
}

void TriangleRasterizer::submit(const Triangle2D& triangle) {
    // wait until the previous triangle's output has been consumed.
    while (busy_) {
        sc_core::wait(idle_);
    }

    // store the new work and mark its output as unfinished.
    pending_triangle_ = triangle;
    busy_ = true;
    output_complete_ = false;
    // wake the producer thread in the next delta cycle (simulation time is not
    // advanced)
    work_available_.notify(sc_core::SC_ZERO_TIME);
}

void TriangleRasterizer::run() {
    while (true) {
        // wait for a triangle to be submitted
        while (!pending_triangle_) {
            // suspend thread until work_available_ is notified through submit
            sc_core::wait(work_available_);
        }

        // a triangle was submitted
        const auto triangle = *pending_triangle_;
        // clear optional, such that it does not hold a triangle
        pending_triangle_.reset();

        // accumulate setup cycles
        sc_core::wait(clock_period_ * static_cast<float>(config_.setup_cycles));

        // record the time after the first pixel enter the pipeline
        const auto pipeline_start = sc_core::sc_time_stamp();

        // compute the triangle bounds to compute the number of pixels which need to
        // be pushed through the pipeline
        const auto bounds = compute_triangle_bounds(triangle, frame_buffer_width_,
                                                    frame_buffer_height_);

        // if bound is std::nullopt the triangle is out of bounds of the framebuffer
        if (!bounds) {
            output_complete_ = true;
            output_available_.notify(sc_core::SC_ZERO_TIME);
            continue;
        }

        // unpack bounds
        const auto [min_x, min_y, max_x, max_y] = *bounds;

        // compute number of rows in a bounding box
        const auto bounding_box_rows = static_cast<uint64_t>(max_y - min_y + 1);

        // compute the pixels which pass through the pipeline to be tested if they are
        // inside or outside of the triangle
        const auto bounding_box_pixels_per_row =
            static_cast<uint64_t>(max_x - min_x + 1);

        // compute the index of the last pixel in the last row of the bounding box
        const auto last_bounding_box_pixel_index =
            bounding_box_rows * bounding_box_pixels_per_row - 1;

        // compute all scanlines
        const auto scanlines =
            rasterize_triangle(triangle, frame_buffer_width_, frame_buffer_height_);

        // Treat degenerate triangles and triangles with no covered pixel centers
        // alike: empty output incurs setup latency only, with no pipeline delay.
        if (scanlines.empty()) {
            output_complete_ = true;
            output_available_.notify(sc_core::SC_ZERO_TIME);
            continue;
        }

        sc_core::sc_time stalled_time = sc_core::SC_ZERO_TIME;

        for (const auto& scanline : scanlines) {
            const auto pixel_count = scanline.pixels.size();

            // reject a row that can never fit
            if (pixel_count > config_.scanline_fifo_capacity_bytes / RGB_COLOR_SIZE) {
                throw std::length_error(
                    "Scanline exceeds triangle rasterizer FIFO capacity");
            }

            const auto row_index =
                static_cast<uint64_t>(scanline.y - static_cast<size_t>(min_y));
            const auto last_bounding_box_pixel_index =
                (row_index + 1) * bounding_box_pixels_per_row - 1;

            // time when the last scanline pixel exists the pipeline
            const auto ready_time =
                pipeline_start + pipeline_latency_ +
                pixel_input_interval_ *
                    static_cast<float>(last_bounding_box_pixel_index) +
                stalled_time;

            // wait until the scanlines last pixel exists the pipeline
            if (ready_time > sc_core::sc_time_stamp()) {
                sc_core::wait(ready_time - sc_core::sc_time_stamp());
            }

            const auto stall_start = sc_core::sc_time_stamp();

            // number of bytes in the current scanline
            const auto bytes = pixel_count * RGB_COLOR_SIZE;

            // wait until the consumer of the scanlines frees FIFO space
            while (bytes > config_.scanline_fifo_capacity_bytes - queued_pixel_bytes_) {
                // suspend thread until space becomes available
                sc_core::wait(space_available_);
            }

            stalled_time += sc_core::sc_time_stamp() - stall_start;

            // push row into FIFO and wake the consumer
            scanlines_output_fifo_.push_back(scanline);
            queued_pixel_bytes_ += bytes;
            output_available_.notify(sc_core::SC_ZERO_TIME);
        }

        const auto finish_time =
            pipeline_start + pipeline_latency_ +
            pixel_input_interval_ * static_cast<float>(last_bounding_box_pixel_index) +
            stalled_time;

        if (finish_time > sc_core::sc_time_stamp()) {
            sc_core::wait(finish_time - sc_core::sc_time_stamp());
        }

        output_complete_ = true;
        output_available_.notify(sc_core::SC_ZERO_TIME);
    }
}

std::optional<RasterizedScanline> TriangleRasterizer::next_scanline() {
    // wait for a scanline or the end of the triangle output
    while (scanlines_output_fifo_.empty() && !output_complete_) {
        // suspend thread until a scanline is in the FIFO
        sc_core::wait(output_available_);
    }

    // scanline is available
    if (!scanlines_output_fifo_.empty()) {
        // transfer the scanline to the caller and release the FIFO capacity
        auto scanline = std::move(scanlines_output_fifo_.front());
        scanlines_output_fifo_.pop_front();

        queued_pixel_bytes_ -= scanline.pixels.size() * RGB_COLOR_SIZE;
        space_available_.notify(sc_core::SC_ZERO_TIME);
        return scanline;
    }

    // all scanlines were consumed, allow to submit the next triangle
    busy_ = false;
    idle_.notify(sc_core::SC_ZERO_TIME);
    return std::nullopt;
}
