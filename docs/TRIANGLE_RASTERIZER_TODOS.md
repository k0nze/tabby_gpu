# Triangle rasterizer TODOs

Goal: replace the stub with a rasterizer that returns clipped, colored scanlines.
The command processor owns RGB packing, framebuffer writes, and transport timing.

Coverage, scanline integration, and barycentric RGB interpolation are implemented.
Signed-area arithmetic is widened to `int64_t`; edge tests and interpolation still
use `float`. The SystemC module schedules scanline delivery through a byte-budget
FIFO while the command processor performs timed writes. Geometry is computed
upfront, and FIFO backpressure is modeled as row-level stalls.

Precision and full-range coordinate robustness remain outstanding. Verification
is deferred at the user's request; unchecked verification items remain pending.

## Pipelined timing and scanline delivery

- [x] Define rasterizer clock frequency, setup cost, pipeline latency to the first
      result, and `pixel_input_interval_cycles` for input spacing.
- [x] Keep pipeline latency separate from throughput; account for bounding-box
      pixels, including rejected pixels, when output is nonempty.
- [x] Keep geometry computation untimed and schedule incremental delivery in the
      `TriangleRasterizer` SystemC module.
- [x] Initially release a scanline after its bounding-box row has been processed,
      including pipeline drain needed for its final result.
- [x] Include intermediate and trailing empty rows in pipeline timing when output
      is nonempty; treat entirely empty output as setup-only by design.
- [x] Add a FIFO bounded by queued RGB bytes, releasing capacity on dequeue and
      rejecting a scanline that cannot fit in the configured capacity.
- [x] Run the producer and command-processor consumer in separate `SC_THREAD`s so
      computation overlaps annotated framebuffer write delays.
- [x] Accumulate FIFO stall duration and shift subsequent row availability times.
- [x] Preserve scanline order and issue one timed write per contiguous covered run.
- [x] Complete command processing after producer completion, FIFO drain, and the
      last timed write; empty output completes after setup.
- [x] Process commands sequentially, preserving ordering with clear operations.
- [x] Overlap modeled rasterization and framebuffer writes rather than adding
      whole-triangle computation and write times sequentially.
- [ ] Expose progress and command completion for future interactive stepping and
      the shared SystemC/RTL backend interface.

## Coverage and coordinate conventions

- [x] Define screen coordinates: origin at the top-left, x increasing rightward,
      and y increasing downward.
- [x] Sample coverage at pixel centers `(x + 0.5, y + 0.5)`.
- [ ] Choose arithmetic that avoids overflow across the supported `int32_t`
      coordinate range; widen operands before subtraction and multiplication.
- [x] Widen coordinate differences before subtraction and compute signed area in
      `int64_t` for ordinary screen-sized coordinates.
- [x] Use `float` consistently for project floating-point calculations, including
      edge values, pixel centers, area conversion, interpolation, and timing inputs.
- [ ] Define and verify the supported coordinate range for `float` coverage tests;
      preserve the project-wide `float` convention when addressing precision limits.
- [x] Clamp framebuffer bounds before converting to signed coordinates in the
      reusable `compute_triangle_bounds()` helper.
- [x] Use `int64_t` loop counters to avoid overflow at `INT32_MAX` bounds.
- [x] Compute signed triangle area and return no scanlines for zero-area triangles.
- [x] Normalize clockwise and counterclockwise input winding for coverage tests.
- [x] Compute the bounding rectangle and clip it to framebuffer dimensions before
      iterating; return no scanlines for fully offscreen triangles.
- [x] Require nonzero framebuffer dimensions, validated by module construction.
- [x] Implement edge-function inside tests.
- [x] Apply the top-left edge-inclusion rule; verify shared-edge behavior below.
- [x] Keep geometric coverage rules separate from color interpolation.

## Color interpolation

- [x] Compute barycentric weights from edge values and positive triangle area.
- [x] Interpolate red, green, and blue from the three vertex colors.
- [x] Round channels with `std::lround` and clamp results to 0 through 255.
- [x] Include `<cmath>` explicitly for `std::abs` and `std::lround`, then verify a
      fresh compilation of the rasterizer.
- [ ] Preserve constant colors when all vertices have the same color.

## Scanline output and integration

- [x] Return scanlines in increasing y order with pixels ordered left to right.
- [x] Emit only nonempty, contiguous runs; set `x_start` to the first covered pixel.
- [x] Clip runs to framebuffer rows, with additional clipping in the command processor.
- [x] Keep framebuffer transport and timed waits out of the geometry function;
      the module and untimed function share the current header and source file.
- [x] Connect real rasterizer output to the existing scanline encoder and timed
      writes in `CommandProcessor`.
- [x] Seed four demo triangles with varied colors, overlap, winding, and offscreen
      vertices; drain command batches so every triangle is processed.
- [x] Export the final demo framebuffer to PNG.
- [ ] Inspect the demo PNG for coverage, clipping, overlap, and color gradients.
- [x] Implement computation timing separately from memory-access latency using
      the pipelined timing plan above.

## Verification

- [ ] Check known small triangles against explicit expected pixel coverage.
- [ ] Check flat-top, flat-bottom, thin, and single-pixel coverage cases.
- [ ] Check degenerate triangles and repeated vertices.
- [ ] Check negative coordinates, partial clipping at every border, and triangles
      completely outside the framebuffer.
- [ ] Check equivalent output when vertex winding is reversed with colors preserved.
- [ ] Check adjacent triangles forming a rectangle for gaps or double coverage.
- [ ] Check constant colors, interpolated gradients, and channel rounding.
- [ ] Check extreme supported coordinates for arithmetic overflow.
- [ ] Verify scanline addresses and RGB writes preserve pixels outside the triangle.

## Later improvements

- [ ] Update edge functions incrementally across rows and columns.
- [ ] Refine incremental scanline production to smaller pixel batches if needed
      after the scanline pipeline is implemented.
- [ ] Compare coverage and interpolation with the RTL rasterizer using shared
      reference cases.
