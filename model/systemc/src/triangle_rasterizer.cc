#include "rasterized_scanline.h"
#include "rgb_color.h"
#include "triangle_rasterizer.h"
#include "vertex_2d.h"

#include <algorithm>
#include <cmath>
#include <utility>

std::vector<RasterizedScanline> rasterize_triangle(const Vertex2D& v0,
                                                   const Vertex2D& v1,
                                                   const Vertex2D& v2,
                                                   std::size_t frame_buffer_width,
                                                   std::size_t frame_buffer_height) {
    // local copy of vertices
    Vertex2D a = v0;
    Vertex2D b = v1;
    Vertex2D c = v2;

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

    // compute the bounding triangle
    // clip min to 0 (frame buffer limit)
    const auto min_x = std::max<int32_t>(0, std::min({a.x, b.x, c.x}));
    const auto min_y = std::max<int32_t>(0, std::min({a.y, b.y, c.y}));

    const auto max_x =
        std::min<int32_t>(frame_buffer_width - 1, std::max({a.x, b.x, c.x}));
    const auto max_y =
        std::min<int32_t>(frame_buffer_height - 1, std::max({a.y, b.y, c.y}));

    // triangle lies completely outside of frame buffer
    if (min_x > max_x || min_y > max_y) {
        return {};
    }

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
    for (int32_t y = min_y; y <= max_y; y++) {
        RasterizedScanline scanline{static_cast<std::size_t>(y), 0, {}};
        for (int32_t x = min_x; x <= max_x; x++) {
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
