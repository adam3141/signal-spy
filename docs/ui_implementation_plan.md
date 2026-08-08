# Signal Spy UI Architecture & Implementation Plan

## 1. Executive Summary & Objectives

This document establishes the architecture, design principles, component taxonomy, and phased implementation roadmap for the Signal Spy user interface (UI). 

The primary objective of the UI engine is to provide **real-time, zero-latency visualization** (waterfall heatmaps, power spectrum density plots, time-domain oscilloscope views) and full control over Digital Signal Processing (DSP) pipelines while maintaining a modern, highly responsive user experience.

### Core Architectural Pillars
1. **Strict ImGui Abstraction (Header Decoupling)**: Public application headers (`include/ui/...`) must remain 100% free of Dear ImGui (`imgui.h`), ImPlot (`implot.h`), and GLFW (`glfw3.h`) dependencies. All GUI third-party details are encapsulated using Pure Abstract Base Classes, Pimpl (Pointer to Implementation), and Value Data Transfer Objects (DTOs).
2. **Lock-Free Multithreaded Architecture**: The UI render loop runs on the OS main thread while DSP data ingestion and FFT execution run asynchronously on dedicated worker threads. Data transfer between DSP and UI uses lock-free Single-Producer Single-Consumer (SPSC) ring buffers.
3. **High-Performance GPU-Accelerated Rendering**: Double/triple-buffered OpenGL/Vulkan texture streaming for 2D waterfall displays and hardware-accelerated vector plotting for oscilloscope sweeps.
4. **Dockable & Customizable Layouts**: Full support for docking spaces, pop-out viewports, custom dark-mode aesthetics, and persistent JSON layout presets.

---

## 2. UI Threading Strategy & Latency Evaluation

A key requirement for Signal Spy is zero perceived latency and smooth 60-144 FPS UI interaction regardless of signal processing throughput (e.g. streaming 20 MSps raw I/Q data and computing 65,536-point FFTs).

### 2.1 Exploration of Threading Models

| Criteria | Option A: Mono-Threaded (GUI + DSP on 1 Thread) | Option B: Decoupled DSP Workers + Main UI Thread (Recommended) | Option C: Dedicated UI Thread off Main Thread |
| :--- | :--- | :--- | :--- |
| **Responsiveness** | Poor (FFT stalls cause UI dropped frames/lag) | **Optimal** (Guaranteed 60-144 FPS render loop) | High |
| **OS Compatibility** | High | **High** (Standard GLFW/Cocoa/X11 model) | Low/Complex (GLFW/macOS event pump restrictions) |
| **Threading Overhead** | Zero | **Minimal** (Lock-free atomic queues) | High (Context switching & GL context migrations) |
| **Implementation Risk**| Low | **Low** (Industry-standard SDR/Audio architecture) | High (Thread safety bugs across OS windowing backends) |

#### Analysis of Option C (Dedicated UI Thread off Main Thread)
Running ImGui and OpenGL rendering on a secondary thread while keeping window event polling (`glfwPollEvents`) on the OS main thread was evaluated. While theoretically appealing, OS windowing frameworks (macOS Cocoa, Wayland, Windows Win32) strictly enforce that window event dispatching and graphics context creation occur on Thread 0 (the main OS thread). Transferring OpenGL contexts (`glfwMakeContextCurrent`) between threads introduces platform-dependent locks and severe driver latency.

#### Selected Architecture: Option B (Decoupled DSP Workers + Main UI/Render Thread)
- **Main OS / UI Thread (Thread 0)**:
  - Executes `glfwPollEvents()`.
  - Runs `IUIRenderer::render_frame()` at monitor refresh rate (60Hz - 144Hz).
  - Handles ImGui input processing, layout calculation, and OpenGL draw call submissions.
  - Uploads new FFT line data to waterfall OpenGL textures via double-buffered PBOs (Pixel Buffer Objects).
- **DSP Worker Threads (Thread 1..N)**:
  - Ingest raw I/Q samples from SDR drivers or file streams.
  - Execute windowing and FFT transformations (FFTW3 / hardware acceleration).
  - Push calculated power spectrum magnitude vectors into a lock-free SPSC ring buffer.
- **Asynchronous Command Bus (UI -> DSP)**:
  - UI control widgets send non-blocking parameter mutation requests (e.g., `SetFFTSizeCommand`, `SetCenterFrequencyCommand`) into a thread-safe lock-free MPSC command queue. The DSP loop drains commands at frame boundaries without blocking the UI thread.

```
 +-------------------------------------------------------+
 |                  Main UI / Render Thread              |
 |  +--------------------+   +------------------------+  |
 |  |  GLFW Event Loop   |-->| ImGui / ImPlot Render  |  |
 |  +--------------------+   +------------------------+  |
 +---------------------------^---------------------------+
                             |  Read latest spectrum (Lock-Free SPSC)
                             |  Write user control cmds (Lock-Free MPSC)
 +---------------------------v---------------------------+
 |                     DSP Pipeline Thread               |
 |  +--------------------+   +------------------------+  |
 |  | SDR Ingestion / IQ |-->| FFTW3 / Windowing Engine|  |
 |  +--------------------+   +------------------------+  |
 +-------------------------------------------------------+
```

---

## 3. UI Component & Widget Taxonomy

Signal Spy requires a rich set of specialized widgets categorized into **Signal Visualization**, **DSP & System Controls**, and **Application Shell**.

```
                           +------------------------+
                           |     UIManager / Shell   |
                           +-----------+------------+
                                       |
      +--------------------------------+--------------------------------+
      |                                |                                |
+-----v--------------+      +----------v---------+           +----------v---------+
| Visualization      |      | Control Panels     |           | Status & Shell     |
| - WaterfallWidget  |      | - SignalSourceCtrl |           | - MainMenuBar      |
| - SpectrumWidget   |      | - DSPConfigCtrl    |           | - StatusBar        |
| - TimeDomainWidget |      | - DataExportCtrl   |           | - DockspaceManager |
+--------------------+      +--------------------+           +--------------------+
```

### 3.1 Signal Visualization Widgets

#### 1. `WaterfallWidget` (Frequency vs. Time 2D Scrolling Heatmap)
- **Purpose**: Displays real-time spectral power density history as a vertically scrolling 2D heatmap.
- **Key Features**:
  - GPU texture streaming using ring-buffered OpenGL 2D textures.
  - Interactive colormap selection (Viridis, Inferno, Turbo, Plasma, Grayscale).
  - Dynamic range tuning (Min/Max dB threshold sliders, contrast adjustment).
  - Variable scroll speed and line history depth (up to 2,048 spectral lines).
  - Synchronized frequency cursor crosshair linked with the Spectrum Display.

#### 2. `SpectrumWidget` (Power Spectral Density / PSD Plot)
- **Purpose**: Displays instantaneous power (dBm or relative dB) versus frequency (Hz/kHz/MHz).
- **Key Features**:
  - High-speed 2D line plot rendered via ImPlot abstraction.
  - Multi-trace support: Live trace, Max-Hold, Min-Hold, and Average spectrum overlays.
  - Automatic peak detection markers (frequency peak labeling).
  - Multi-cursor delta measurement system ($\Delta f$, bandwidth, SNR calculation).
  - Noise floor estimation baseline rendering.

#### 3. `TimeDomainWidget` (Oscilloscope & Constellation View)
- **Purpose**: Time-series amplitude analysis and digital modulation inspection.
- **Key Features**:
  - Real-time oscilloscope sweep display for I (In-phase) and Q (Quadrature) channels.
  - Constellation 2D scatter plot (I vs. Q) for signal modulation phase analysis.
  - Digital triggering modes: Auto, Normal, Single-Shot with edge threshold and pre-trigger buffer adjustments.

### 3.2 Control & Input Widgets

#### 4. `SignalSourceControlWidget`
- **Purpose**: Hardware and file input configuration.
- **Key Features**:
  - Input selector: SDR Device (RTL-SDR, HackRF, USRP), WAV Audio, SigMF file playback, or Synthetic/Null Source.
  - Sample rate selector (e.g., 1 MSps to 50 MSps), center frequency tuning, hardware RF gain controls.
  - File playback controls: Play, Pause, Seek bar, Loop, and playback speed multiplier.

#### 5. `DSPConfigWidget`
- **Purpose**: Signal processing pipeline parameters tuning.
- **Key Features**:
  - FFT size selector: 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 points.
  - Windowing algorithm selection: Hann, Hamming, Blackman-Harris, Rectangular, Flat-Top.
  - Spectrum averaging mode: Moving Average, Exponential Decay, Peak Hold.
  - Decimation factor and digital filter bandwidth selection.

#### 6. `DataExportWidget`
- **Purpose**: Real-time signal capture and screenshot export.
- **Key Features**:
  - SigMF recording start/stop with live recorded file size counter.
  - High-resolution viewport image snapshot (PNG/SVG export).
  - Raw spectrum frame data export (CSV/JSON).

### 3.3 Application Shell & Layout Widgets

#### 7. `MainMenuBarWidget` & `StatusBarWidget`
- **Purpose**: Top navigation menu and bottom telemetry bar.
- **Key Features**:
  - File menu (Open recording, Save preset, Exit).
  - View menu (Toggle individual dockable panels, layout reset).
  - Live status telemetry: UI FPS, Frame render latency (ms), DSP sample drop count, Tracy profiling status, and active signal source indicator.

#### 8. `DockingLayoutManager`
- **Purpose**: Window management, docking spaces, and customizable themes.
- **Key Features**:
  - Central dockspace supporting tabbed, stacked, or floating detached viewports.
  - Preset layout manager (Default, Spectrum Focused, Oscilloscope Focused, Minimal).
  - Modern dark-mode palette configuration (custom background, accent colors, Inter/Roboto typography).

---

## 4. ImGui Abstraction & Interface Design Architecture

To ensure clean architecture, long-term maintainability, and fast build times, **third-party headers (`imgui.h`, `implot.h`, `glfw3.h`) must never be included in public header files in `include/ui/`**.

### 4.1 Class Abstraction Hierarchy

```
       +--------------------+
       |      IWidget       |  <--- Public Interface (include/ui/i_widget.hpp)
       +---------+----------+
                 |
                 +-----------------------------------------+
                 |                                         |
       +---------v----------+                    +---------v----------+
       |   WaterfallWidget  |                    |   SpectrumWidget   |
       |  (Pimpl Pattern)   |                    |  (Pimpl Pattern)   |
       +---------+----------+                    +---------+----------+
                 | (owns unique_ptr)                       | (owns unique_ptr)
       +---------v----------+                    +---------v----------+
       |WaterfallWidgetImpl |                    | SpectrumWidgetImpl |  <--- Private Impl
       | (#include imgui.h) |                    | (#include implot.h)|       (source/ui/...)
       +--------------------+                    +--------------------+
```

### 4.2 Core Header Definitions

#### 1. Public Base Widget Interface (`include/ui/i_widget.hpp`)
```cpp
#pragma once

#include <string_view>

namespace spy::ui {

class IWidget {
public:
    virtual ~IWidget() = default;

    /// Renders the widget content during the active frame loop.
    virtual void Render() = 0;

    /// Returns the human-readable display title of the widget panel.
    [[nodiscard]] virtual std::string_view get_Title() const noexcept = 0;

    /// Returns whether the widget window is currently open/visible.
    [[nodiscard]] virtual bool is_Visible() const noexcept = 0;

    /// Sets the visibility state of the widget window.
    virtual void set_Visible(bool visible) noexcept = 0;
};

} // namespace spy::ui
```

#### 2. Data Transfer Objects (DTOs) (`include/ui/ui_types.hpp`)
```cpp
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <span>

namespace spy::ui {

enum class WindowFunction : uint8_t {
    Rectangular,
    Hann,
    Hamming,
    BlackmanHarris,
    FlatTop
};

enum class ColourMap : uint8_t {
    Viridis,
    Inferno,
    Turbo,
    Plasma,
    Grayscale
};

struct SpectrumFrameDTO {
    uint64_t timestamp_ns{0};
    double center_frequency_hz{0.0};
    double sample_rate_hz{0.0};
    std::vector<float> magnitudes_db;
};

struct WaterfallParamsDTO {
    ColourMap colour_scheme{ColourMap::Viridis};
    float min_db{-120.0f};
    float max_db{0.0f};
    float contrast{1.0f};
    uint32_t history_depth{512};
};

} // namespace spy::ui
```

#### 3. Main UI Renderer Interface (`include/ui/i_ui_renderer.hpp`)
```cpp
#pragma once

#include "ui/i_widget.hpp"
#include <memory>

namespace spy::ui {

struct UIRendererConfig {
    int window_width{1600};
    int window_height{900};
    const char* window_title{"Signal Spy"};
    bool vsync{true};
};

class IUIRenderer {
public:
    virtual ~IUIRenderer() = default;

    virtual bool Initialize(const UIRendererConfig& config) = 0;
    virtual void RegisterWidget(std::shared_ptr<IWidget> widget) = 0;
    [[nodiscard]] virtual bool ShouldClose() const noexcept = 0;
    virtual void BeginFrame() = 0;
    virtual void RenderWidgets() = 0;
    virtual void EndFrame() = 0;
    virtual void Shutdown() = 0;
};

/// Factory function creating the GLFW+ImGui concrete implementation.
[[nodiscard]] std::unique_ptr<IUIRenderer> CreateUIRenderer();

} // namespace spy::ui
```



---

## 5. Phased Implementation Roadmap & Milestones

The implementation is broken down into 5 executable milestones. Each milestone contains verifiable deliverables and checkboxes.

### Milestone 1: UI Core Abstraction & Renderer Framework
- [x] Implement `IWidget` interface and `IUIRenderer` factory in `include/ui/`.
- [x] Implement `ImGuiUIRenderer` concrete backend in `source/ui/imgui_ui_renderer.cpp` using GLFW + OpenGL3.
- [x] Configure custom modern dark-mode theme palette and load crisp typography (e.g. Inter font).
- [x] Implement `DockingLayoutManager` initializing main viewport dockspace.
- [x] Write unit & integration tests for UI window initialization and renderer lifecycle.


### Milestone 2: Signal Visualization Widgets
- [ ] Implement `SpectrumWidget` with ImPlot line plotting, dB scaling, and multi-trace overlays (Live, Max-Hold, Min-Hold).
- [ ] Implement peak detection marker overlays and interactive multi-cursors ($\Delta f$, bandwidth, SNR).
- [ ] Implement `WaterfallWidget` using double-buffered OpenGL PBO texture streaming.
- [ ] Add colormap shaders/lookup tables (Viridis, Inferno, Turbo, Grayscale) and dB dynamic range adjustments.
- [ ] Implement `TimeDomainWidget` oscilloscope amplitude sweeps and I/Q constellation 2D scatter plots.

### Milestone 3: Control Panels & Command Bus Integration
- [ ] Implement thread-safe, lock-free SPSC queue for spectrum frame transfer from DSP to UI.
- [ ] Implement thread-safe MPSC command queue for UI -> DSP configuration requests.
- [ ] Implement `SignalSourceControlWidget` (source selection, frequency tuning, SDR gains, WAV/SigMF file playback).
- [ ] Implement `DSPConfigWidget` (FFT size selection, windowing function selector, averaging factor).
- [ ] Implement `DataExportWidget` (SigMF recording toggle, image snapshot, JSON spectrum frame export).

### Milestone 4: Docking Layouts, Presets & Persistence
- [ ] Implement layout preset manager (Default, Spectrum Focused, Oscilloscope Focused, Minimal).
- [ ] Integrate Glaze JSON serialization for saving/loading layout presets and UI settings to disk.
- [ ] Add `MainMenuBarWidget` and `StatusBarWidget` displaying live FPS, render latency, queue drops, and sample rates.

### Milestone 5: Benchmarking, Profiling & Performance Verification
- [ ] Instrument UI frame loops, texture uploads, and widget draw calls with Tracy markers (`FrameMark`, `ZoneScoped`).
- [ ] Benchmark high throughput waterfall streaming (60 FPS rendering at 65,536 FFT points).
- [ ] Verify zero memory allocations during steady-state widget rendering.
- [ ] Ensure end-to-end processing-to-display latency stays strictly below 50ms.

---

## 6. Verification & Quality Assurance Plan

### Automated Verification
- **Build Verification**: Compile clean with zero warnings on GCC/Clang (`-Wall -Wextra -Werror`).
- **Unit Testing**: Run `ctest` targeting `signal-spy-tests` to verify widget state registration and lock-free queue concurrency.
- **Header Hygiene Check**: Verify that `grep -r "imgui.h" include/` returns zero matches.

### Performance & Latency Benchmarks
- Render loop frame budget: $< 16.6\text{ ms}$ (60 FPS) and $< 8.3\text{ ms}$ (120 FPS).
- DSP-to-UI transfer latency: $< 5\text{ ms}$ using lock-free SPSC buffer.
