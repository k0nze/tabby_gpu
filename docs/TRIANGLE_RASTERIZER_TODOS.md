# Triangle rasterizer TODOs

Goal: replace the stub with a rasterizer that returns clipped, colored scanlines.
The command processor owns RGB packing, framebuffer writes, and transport timing.

Coverage, scanline integration, and barycentric RGB interpolation are implemented.
Signed-area arithmetic is widened to `int64_t`; edge tests and interpolation still
use `float`. Compilation, precision, full-range coordinate robustness, and the
verification cases below remain to be checked or resolved.

Next priority: model pipelined rasterization with scanlines becoming available
incrementally and overlapping timed framebuffer writes. Verification is deferred
at the user's request; the unchecked verification items remain outstanding.

## Next: pipelined timing and scanline delivery

- [ ] Define rasterizer clock frequency, setup cost, pipeline latency to the first
      result, and initiation interval (cycles between accepted candidate pixels).
- [ ] Keep pipeline latency separate from throughput; account for all tested
      candidate pixels, including those rejected by coverage tests.
- [ ] Keep coverage and interpolation independent of SystemC; add a simulation
      wrapper that reports and schedules scanline availability incrementally.
- [ ] Initially release a scanline after its candidate pixels have been processed,
      including pipeline drain needed for its final result.
- [ ] Advance modeled computation through rows with no covered pixels even though
      those rows produce no framebuffer writes.
- [ ] Add a bounded FIFO carrying completed scanlines between rasterization and
      framebuffer writing.
- [ ] Run rasterization and the framebuffer writer in separate `SC_THREAD`s so
      computation overlaps annotated framebuffer write delays.
- [ ] Stall rasterizer progress when the FIFO is full; resume when the writer
      frees space, preserving the supported pipeline buffering behavior.
- [ ] Preserve scanline order and issue one timed write per contiguous covered run.
- [ ] Signal triangle completion only after the final framebuffer write completes;
      define completion for degenerate and fully offscreen triangles as well.
- [ ] Define whether command processing waits for triangle completion before
      starting the next command, preserving ordering with clear operations.
- [ ] Account for overlap rather than adding whole-triangle computation time to
      all framebuffer write times; let the slower stage limit sustained throughput.
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
- [ ] Validate framebuffer dimensions before narrowing bounding limits to `int32_t`.
- [ ] Avoid loop-counter overflow when a bounding limit reaches `INT32_MAX`.
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
- [x] Keep SystemC and framebuffer transport dependencies out of the rasterizer.
- [x] Connect real rasterizer output to the existing scanline encoder and timed
      writes in `CommandProcessor`.
- [ ] Export the demo triangle to PNG and inspect its coverage and color gradient.
- [ ] Implement computation timing separately from memory-access latency using
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
