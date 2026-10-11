#include "rasterized_scanline.h"
#include "triangle_rasterizer.h"
#include "vertex_2d.h"

#include <algorithm>
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
    const auto dx1 = b.x - a.x;
    const auto dy1 = b.y - a.y;

    const auto dx2 = c.x - a.x;
    const auto dy2 = c.y - a.y;

    const auto area2 = dx1 * dy2 - dy1 * dx2;

    if (area2 == 0) {
        return {};
    }

    // normalize winding such that the area becomes positive
    if (area2 < 0) {
        std::swap(b, c);
    }

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
                scanline.pixels.push_back(RGB_WHITE);
            }
        }

        // if the whole scanline is not empty add it to the scanlines vector
        if (!scanline.pixels.empty()) {
            scanlines.push_back(std::move(scanline));
        }
    }

    return scanlines;
}
