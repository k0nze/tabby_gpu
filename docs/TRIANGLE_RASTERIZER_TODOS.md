# Triangle rasterizer TODOs

Goal: replace the stub with a rasterizer that returns clipped, colored scanlines.
The command processor owns RGB packing, framebuffer writes, and transport timing.

Current output uses solid `RGB_WHITE`. Coverage and scanline integration are
implemented for ordinary framebuffer coordinates; color interpolation and the
verification cases below remain pending. Signed area still uses 32-bit arithmetic,
and edge tests use `float`, so full-range coordinate robustness is not complete.

## Coverage and coordinate conventions

- [x] Define screen coordinates: origin at the top-left, x increasing rightward,
      and y increasing downward.
- [x] Sample coverage at pixel centers `(x + 0.5, y + 0.5)`.
- [ ] Choose arithmetic that avoids overflow across the supported `int32_t`
      coordinate range; widen operands before subtraction and multiplication.
- [x] Compute signed triangle area and return no scanlines for zero-area triangles.
- [x] Normalize clockwise and counterclockwise input winding for coverage tests.
- [x] Compute the bounding rectangle and clip it to framebuffer dimensions before
      iterating; return no scanlines for fully offscreen triangles.
- [x] Require nonzero framebuffer dimensions, validated by module construction.
- [x] Implement edge-function inside tests.
- [x] Apply the top-left edge-inclusion rule; verify shared-edge behavior below.
- [ ] Keep geometric coverage rules separate from color interpolation.

## Color interpolation

- [ ] Compute barycentric weights from edge values and triangle area.
- [ ] Interpolate red, green, and blue from the three vertex colors.
- [ ] Define channel rounding and clamp results to the range 0 through 255.
- [ ] Preserve constant colors when all vertices have the same color.

## Scanline output and integration

- [x] Return scanlines in increasing y order with pixels ordered left to right.
- [x] Emit only nonempty, contiguous runs; set `x_start` to the first covered pixel.
- [x] Clip runs to framebuffer rows, with additional clipping in the command processor.
- [x] Keep SystemC and framebuffer transport dependencies out of the rasterizer.
- [x] Connect real rasterizer output to the existing scanline encoder and timed
      writes in `CommandProcessor`.
- [ ] Export the demo triangle to PNG and inspect its coverage and color gradient.
- [ ] Define rasterizer computation latency separately from memory-access latency
      when extending the timing model.

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
- [ ] Consider incremental scanline production for reduced temporary storage and
      future pipeline timing.
- [ ] Compare coverage and interpolation with the RTL rasterizer using shared
      reference cases.
