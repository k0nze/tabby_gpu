# Interactive framebuffer viewer TODOs

Goal: inspect the framebuffer while stepping through commands, then support
continuous execution and refresh at a configurable frame rate. Use MVC and support
both SystemC and RTL simulation backends through the same viewer.

## MVC architecture and simulation backends

- [ ] Use Model-View-Controller (MVC): the model owns simulation state and
      framebuffer snapshots, the view renders state and exposes user controls,
      and the controller translates user actions into simulation operations.
- [ ] Define a shared simulation-backend interface for command submission,
      command stepping, bounded advancement, pause/resume, simulation timestamps,
      framebuffer snapshots, status, and errors.
- [ ] Keep SystemC and RTL simulator types out of the view and controller; use
      backend-independent data types at the interface boundary.
- [ ] Implement a SystemC adapter using its process scheduling and pause behavior.
- [ ] Implement an RTL simulation adapter using clock/reset driving, command
      handshakes, and observation of command completion and framebuffer state.
- [ ] Define command completion consistently across backends so stepping executes
      exactly one command even when the RTL requires many clock cycles.
- [ ] Normalize timestamps to a shared time unit and describe backend capabilities
      when queue occupancy or other internal state cannot be observed.
- [ ] Select the backend through configuration or command-line options without
      changing the viewer or controller.
- [ ] Keep simulator lifecycle, thread ownership, and reset behavior inside each
      adapter; support the same controller in interactive and headless modes.

## Simulation control

- [ ] Replace the demo's unconditional `sc_stop()` with resumable execution.
- [ ] Add a persistent command-processing `SC_THREAD` that waits for execution
      permission, executes a command, and signals completion.
- [ ] Implement stepping exactly one command, including command-fetch and memory
      latency. Pause only after the command and its timed operations complete.
- [ ] Handle an empty command queue without busy polling or blocking the UI.
- [ ] Add run and pause controls; pause at the next command boundary.
- [ ] Keep SystemC calls on the thread owning the simulation.
- [ ] Advance simulation in bounded intervals so the host event loop remains
      responsive, including while a long command is waiting for modeled latency.
- [ ] Reserve `sc_stop()` for final shutdown; use resumable pauses for inspection.
- [ ] Report transaction and configuration errors in the viewer.

## Framebuffer inspection

- [ ] Take framebuffer snapshots while simulation is paused.
- [ ] Include width, height, pixel format, and simulation timestamp in snapshots.
- [ ] Verify RGB channel order, row stride, and top-left image orientation.
- [ ] Check framebuffer capacity calculations for multiplication overflow.
- [ ] Keep PPM export available for debugging and reproducible comparisons.

## Interactive viewer

- [ ] Select a desktop window/rendering library and add it to CMake.
- [ ] Display the RGB framebuffer through a texture.
- [ ] Add Step Command, Run, and Pause controls.
- [ ] Display simulation time, queue occupancy, execution state, and the last
      executed command.
- [ ] Refresh the texture after a stepped command completes.
- [ ] Add a configurable host refresh rate for continuous execution.
- [ ] Handle closing the window and shutting down the simulation cleanly.
- [ ] Add zoom and pixel-coordinate/color inspection.
- [ ] Preserve a headless execution mode for CI and automated verification.

## Simulated display refresh

- [ ] Add a display-controller module with a configurable simulated frame rate.
- [ ] Schedule frame capture using simulation time: 60 Hz corresponds to roughly
      16.67 ms between frames.
- [ ] Define whether initial scanout is an instantaneous snapshot or a timed
      framebuffer read, and document that model.
- [ ] Separate simulated scanout rate from host window refresh rate.
- [ ] Publish the latest completed frame to the viewer; avoid accumulating an
      unbounded backlog when simulation runs faster than the host display.
- [ ] Keep command-step mode able to show framebuffer changes immediately,
      independently of simulated scanout.
- [ ] Decide whether run mode follows simulated scanout or shows live snapshots.
- [ ] Add optional real-time pacing separately from modeled transaction latency.
- [ ] Define tearing behavior or add front/back buffers if scanout and rendering
      need independent ownership.

## Verification milestones

- [ ] Seed nonblack pixels, step a clear command, verify a black framebuffer and
      the expected simulated latency, then step another command successfully.
- [ ] Verify that one step consumes exactly one command and preserves FIFO order.
- [ ] Verify that pausing and resuming preserves simulation state and time.
- [ ] Verify empty-queue handling, long commands, and window-close responsiveness.
- [ ] Verify continuous execution with host refresh throttling.
- [ ] Verify simulated frame timestamps against the configured frame rate.
- [ ] Verify that headless execution produces the same final framebuffer as the
      interactive path for the same command sequence.
- [ ] Run shared command sequences against SystemC and RTL backends and compare
      framebuffer results; compare timing where both models promise equivalence.
- [ ] Verify step, run, pause, refresh, and shutdown through the shared controller
      for both simulation backends.
