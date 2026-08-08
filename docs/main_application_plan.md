# Signal Spy Main Application Architecture & Implementation Plan

## 1. Executive Summary & Objectives

This document defines the architecture, lifecycle state machine, inter-thread communication infrastructure, and phased implementation roadmap specifically for the **Signal Spy Main Application (`signal-spy` executable)**.

The main application acts as the top-level system orchestrator. It manages application startup, command-line configuration, lifecycle state transitions, thread synchronization between the high-throughput DSP pipeline and the main OS UI render loop, signal handling (`SIGINT`/`SIGTERM`), and clean shutdown.

### Core Architectural Pillars
1. **Application Controller (`Application`)**: Central orchestrator encapsulating system subsystems. It owns the main execution loop, manages subsystem initialization, and controls thread lifecycles.
2. **Lock-Free Inter-Thread Pipeline**:
   - **DSP -> UI Spectral Streaming**: High-speed, zero-allocation lock-free Single-Producer Single-Consumer (`SPSCRingBuffer`) ring buffer transferring `SpectrumFrameDTO` payloads from DSP worker threads to UI visualization widgets.
   - **UI -> DSP Command Bus**: Non-blocking Multi-Producer Single-Consumer (`CommandQueue`) queue carrying parameter mutation requests (e.g. center frequency, sample rate, gain, FFT size) from UI widgets to the DSP worker thread.
3. **Strict Subsystem Decoupling**: Application logic in `include/app/` depends only on clean abstract interfaces (`IUIRenderer`, `ISignalSource`), remaining independent of specific third-party graphics (GLFW, ImGui, ImPlot) or driver backends.
4. **Target Structure**: Compiled as executable target `signal-spy` in `source/CMakeLists.txt`, linking against the static library `signal-spy-lib` and required submodules.
5. **Code Formatting Standard**: Automated pre-commit formatting across all C++ files via:
   ```bash
   git ls-files '*.c' '*.cpp' '*.h' '*.hpp' | xargs clang-format-22 -i
   ```

---

## 2. Application Lifecycle & State Machine

The main application operates as a deterministic state machine managing the main thread and asynchronous worker threads.

```
 +------------------+     Initialize()     +------------------+
 |  Uninitialized   | -------------------> |   Initializing   |
 +------------------+                      +--------+---------+
                                                    | Success
                                                    v
 +------------------+        Stop()        +------------------+
 |     Stopping     | <------------------- |     Running      |
 +--------+---------+                      +------------------+
          |
          v
 +------------------+
 |     Shutdown     |
 +------------------+
```

### 2.1 State Transitions & Execution Phases

#### Phase 1: Startup & Initialization (`Initializing`)
1. **Signal Trap Registration**: Registers handlers for `SIGINT` and `SIGTERM` to enable graceful shutdown on system interrupts.
2. **CLI & Config Parsing**: Reads command-line arguments and loads configuration defaults (`AppConfig`).
3. **Subsystem Instantiation**:
   - Instantiates `IUIRenderer` via factory (`CreateUIRenderer()`).
   - Instantiates `ISignalSource` via factory (`SignalSourceFactory`).
   - Allocates the lock-free `SPSCRingBuffer` (spectrum payload transfer) and `CommandQueue` (control bus).
4. **UI Setup**: Registers core widgets (`DockingLayoutManager`, `MainMenuBarWidget`, `StatusBarWidget`) with `IUIRenderer`.

#### Phase 2: Frame Loop Execution (`Running`)
- **Main OS / UI Thread (Thread 0)**:
  - Polls OS window events (`glfwPollEvents`).
  - Drains new spectral frames from `SPSCRingBuffer` and passes data to active visualization widgets.
  - Submits UI control actions to `CommandQueue`.
  - Renders UI frame via `IUIRenderer` at target monitor refresh rate.
- **DSP Worker Thread (Thread 1..N)**:
  - Ingests raw signal samples from `ISignalSource`.
  - Checks `CommandQueue` for pending UI control commands and applies configuration mutations.
  - Computes FFT spectrum magnitudes and pushes completed `SpectrumFrameDTO` frames into `SPSCRingBuffer`.

#### Phase 3: Shutdown (`Stopping` -> `Shutdown`)
1. `ShouldClose()` flag set via OS window close button or `SIGINT`/`SIGTERM` signal.
2. Signal worker threads to stop and join worker threads.
3. Release graphics resources and call `IUIRenderer::Shutdown()`.
4. Return exit code 0.

---

## 3. Inter-Thread Communication Infrastructure

```
 +-------------------------------------------------------------------+
 |                         Main UI Thread                            |
 |  +-------------------+   +------------------+   +--------------+  |
 |  |  IUIRenderer      |-->| Docking / Shell  |-->| UI Widgets   |  |
 |  +-------------------+   +------------------+   +-------+------+  |
 +---------------------------^-----------------------------|----------+
                             |                             |
      SPSCRingBuffer<SpectrumFrameDTO>          CommandQueue (MPSC)
             (DSP -> UI Spectrum)               (UI -> DSP Commands)
                             |                             |
 +---------------------------|-----------------------------v----------+
 |                           |    DSP Processing Thread               |
 |  +-------------------+   +------------------+   +--------------+  |
 |  |  ISignalSource    |-->| Windowing / FFT  |-->| Command Bus  |  |
 |  +-------------------+   +------------------+   +--------------+  |
 +-------------------------------------------------------------------+
```

### 3.1 Lock-Free Spectrum Ring Buffer (`SPSCRingBuffer`)
- Header-only implementation in `include/signal/spsc_ring_buffer.hpp`.
- Single-Producer Single-Consumer lock-free ring buffer using `std::atomic<size_t>` read/write indices with cache line padding to prevent false sharing.
- Guarantees zero heap allocation during frame streaming.

### 3.2 Thread-Safe Command Queue (`CommandQueue`)
- Header-only implementation in `include/signal/command_queue.hpp`.
- Encapsulates parameter mutation commands (e.g. `SetFrequencyCommand`, `SetSampleRateCommand`, `SetGainCommand`, `SetFFTSizeCommand`) using `std::variant`.
- Drained asynchronously by the DSP processing worker thread at frame boundaries.

---

## 4. Main Application Class Architecture

### 4.1 Header Definition (`include/app/application.hpp`)

```cpp
#pragma once

#include "ui/i_ui_renderer.hpp"
#include "signal/signal_source.hpp"
#include <memory>
#include <atomic>
#include <string_view>

namespace spy::app {

struct AppConfig {
    int window_width{1600};
    int window_height{900};
    std::string_view window_title{"Signal Spy"};
    bool vsync{true};
    size_t spectrum_queue_capacity{64};
};

enum class AppState {
    Uninitialized,
    Initializing,
    Running,
    Stopping,
    Shutdown
};

class Application {
public:
    explicit Application(AppConfig config = {});
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) noexcept;
    Application& operator=(Application&&) noexcept;

    /// Initializes all application subsystems (Renderer, DSP, Queues).
    bool Initialize();

    /// Executes the main application loop. Returns exit code when finished.
    int Run();

    /// Requests application shutdown.
    void Stop() noexcept;

    /// Returns current state of the application.
    [[nodiscard]] AppState get_State() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace spy::app
```

---

## 5. Phased Implementation Roadmap & Milestones (Main Application)

The implementation of the Main Application is divided into 5 sequential single-PR milestones targeting branch `dev`.

### Milestone 1: Application Controller & Target Structure (PR 1)
- [ ] Create `include/app/application.hpp` and `source/app/application.cpp` implementing core `Application` state machine and signal handlers.
- [ ] Create `source/main.cpp` entry point initializing `Application app; app.Initialize(); return app.Run();`.
- [ ] Update `source/CMakeLists.txt` to define `add_executable(signal-spy source/main.cpp ...)` linking `signal-spy-lib`.
- [ ] Write integration test in `tests/app/test_application.cpp` verifying `Initialize()`, `Run()` (single-frame mock loop), and `Stop()`.

### Milestone 2: Inter-Thread SPSC Ring Buffer & Command Queue (PR 2)
- [ ] Implement lock-free `SPSCRingBuffer` in `include/signal/spsc_ring_buffer.hpp` with cache-line alignment (`std::hardware_destructive_interference_size`).
- [ ] Implement thread-safe MPSC `CommandQueue` in `include/signal/command_queue.hpp` supporting control command variants.
- [ ] Write unit tests in `tests/signal/test_spsc_ring_buffer.cpp` validating lock-free concurrency and zero allocations.

### Milestone 3: Shell Widgets & Subsystem Wiring (PR 3)
- [ ] Create `MainMenuBarWidget` (`include/ui/main_menu_bar_widget.hpp` / `source/ui/main_menu_bar_widget.cpp`).
- [ ] Create `StatusBarWidget` (`include/ui/status_bar_widget.hpp` / `source/ui/status_bar_widget.cpp`) for displaying UI FPS, render latency (ms), active signal source, and queue telemetry.
- [ ] Register shell widgets with `Application`'s `IUIRenderer` instance.
- [ ] Write widget tests in `tests/ui/test_shell_widgets.cpp`.

### Milestone 4: DSP Worker Thread Integration & Command Dispatch (PR 4)
- [ ] Integrate DSP processing worker loop inside `Application` running asynchronously.
- [ ] Drain `CommandQueue` in DSP thread to mutate `ISignalSource` state dynamically.
- [ ] Push calculated spectrum frames into `SPSCRingBuffer` for UI consumption.
- [ ] Integration test verifying end-to-end DSP-to-UI frame delivery.

### Milestone 5: Application Profiling, Benchmarking & Packaging (PR 5)
- [ ] Add Tracy profiling instrumentation markers (`FrameMark`, `ZoneScoped`) across main application loop.
- [ ] Run automated code formatting check:
  ```bash
  git ls-files '*.c' '*.cpp' '*.h' '*.hpp' | xargs clang-format-22 -i
  ```
- [ ] Run full test suite (`ctest --test-dir build`) and verify zero compiler warnings.

---

## 6. Verification & Quality Assurance Plan

### Automated Verification
1. **Compilation**: Clean build with zero warnings (`cmake --build build`).
2. **Unit & Integration Tests**: Executing `ctest` targeting `test_application`, `test_spsc_ring_buffer`, and `test_shell_widgets`.
3. **Formatting Check**: Verify all C++ headers and source files adhere to `.clang-format`.

### Manual Verification
1. Launch `build/source/signal-spy` executable.
2. Confirm window initialization, smooth frame rendering, menu bar navigation, and status bar telemetry counters.
3. Test terminal interrupt (`Ctrl+C`) to verify clean graceful exit.
