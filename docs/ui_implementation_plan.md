# Signal Spy UI Architecture & Implementation Plan

## 1. Executive Summary & Objectives

This document establishes the architecture, design principles, component taxonomy, and phased implementation roadmap for the Signal Spy user interface (UI) and main application entry point (`main.cpp`).

The primary objective of the UI engine is to provide **real-time, zero-latency visualization** (waterfall heatmaps, power spectrum density plots, time-domain oscilloscope views) and full control over Digital Signal Processing (DSP) pipelines while maintaining a modern, highly responsive user experience.

### Core Architectural Pillars
1. **Strict ImGui & ImPlot Abstraction (Header Decoupling)**: Public application headers (`include/ui/...`) must remain 100% free of Dear ImGui (`imgui.h`), ImPlot (`implot.h`), and GLFW (`glfw3.h`) dependencies. All GUI third-party details are encapsulated using Pure Abstract Base Classes, Pimpl (Pointer to Implementation), and Value Data Transfer Objects (DTOs).
2. **Lock-Free Multithreaded Architecture**: The UI render loop runs on the OS main thread while DSP data ingestion and FFT execution run asynchronously on dedicated worker threads. Data transfer between DSP and UI uses a lightweight header-only Single-Producer Single-Consumer (`SPSCRingBuffer`) in `include/signal/spsc_ring_buffer.hpp`.
3. **High-Performance GPU-Accelerated Rendering**: Double/triple-buffered OpenGL texture streaming for 2D waterfall displays and hardware-accelerated vector plotting for oscilloscope sweeps.
4. **Dockable & Customizable Layouts**: Full support for docking spaces, pop-out viewports, custom dark-mode aesthetics, and persistent Glaze JSON layout presets.
5. **Executable & Library Structure**: The project compiles a core static library (`signal-spy-lib`) containing all DSP and UI components, linked to the main executable target (`signal-spy`) defined in `source/CMakeLists.txt`.
6. **Code Formatting & Verification Standard**: All C++ source files undergo automated formatting prior to completed tasks and PR submissions via:
   ```bash
   git ls-files '*.c' '*.cpp' '*.h' '*.hpp' | xargs clang-format-22 -i
   ```

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
  - High-speed 2D line plot rendered via ImPlot abstraction (Pimpl pattern hiding `implot.h`).
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

### Class Abstraction Hierarchy

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

---

## 5. Phased Implementation Roadmap & Milestones

The main application and UI implementation is structured into 5 single-PR milestones. Each milestone will be developed in a dedicated Git worktree branch targeting `dev`.

### Milestone 1: Main Application Entry Point & Shell Integration (PR 1)
- [x] Core abstraction (`IWidget`, `IUIRenderer`, `DockingLayoutManager`) and dark mode theme.
- [ ] Add `source/main.cpp` entry point initializing `IUIRenderer`, `DockingLayoutManager`, and event loop.
- [ ] Implement `MainMenuBarWidget` (top navigation menu) and `StatusBarWidget` (live FPS, render latency, telemetry).
- [ ] Update `source/CMakeLists.txt` to build `signal-spy` executable linking with `signal-spy-lib`.
- [ ] Add automated tests for menu/status bar widget lifecycles.

### Milestone 2: Signal Visualization Widgets (PR 2)
- [ ] Implement `SpectrumWidget` with ImPlot line plotting, dB scaling, and multi-trace overlays (Live, Max-Hold, Min-Hold).
- [ ] Implement peak detection marker overlays and interactive multi-cursors ($\Delta f$, bandwidth, SNR).
- [ ] Implement `WaterfallWidget` using double-buffered OpenGL PBO texture streaming.
- [ ] Add colormap lookup tables (Viridis, Inferno, Turbo, Plasma, Grayscale) and dB dynamic range adjustments.
- [ ] Implement `TimeDomainWidget` oscilloscope amplitude sweeps and I/Q constellation 2D scatter plots.

### Milestone 3: Control Panels & Command Bus Integration (PR 3)
- [ ] Implement `SignalSourceControlWidget` (SDR device selection, frequency tuning, RF gain, WAV/SigMF file playback).
- [ ] Implement `DSPConfigWidget` (FFT size selector 512..65536, windowing functions, averaging mode).
- [ ] Implement `DataExportWidget` (SigMF recording toggle, viewport PNG snapshot, JSON spectrum export).

### Milestone 4: Inter-Thread SPSC Spectrum Pipeline & Mock Ingestion (PR 4)
- [ ] Implement lightweight header-only lock-free queue (`SPSCRingBuffer`) in `include/signal/spsc_ring_buffer.hpp`.
- [ ] Implement non-blocking MPSC command queue for UI-to-DSP control instructions.
- [ ] Integrate mock synthetic spectrum generator thread pushing live frames to UI visualization widgets.

### Milestone 5: Layout Presets, Glaze JSON Persistence & Final Polish (PR 5)
- [ ] Implement layout preset manager (`LayoutPresetManager`) with Default, Spectrum Focused, Oscilloscope Focused, and Minimal presets.
- [ ] Integrate Glaze JSON serialization for saving/loading layout presets and UI settings to disk.
- [ ] Benchmark high-throughput waterfall streaming (60 FPS rendering at 65,536 FFT points) with Tracy profiling markers.
- [ ] Verify clean code formatting (`clang-format-22`), zero compiler warnings, and 100% passing unit tests.

---

## 6. Verification & Quality Assurance Plan

### Automated Verification
- **Code Formatting**: Ensure all source files pass `git ls-files '*.c' '*.cpp' '*.h' '*.hpp' | xargs clang-format-22 -i`.
- **Build Verification**: Clean compilation (`cmake --build build`).
- **Unit Testing**: Execution of test suite (`ctest --test-dir build --output-on-failure`).
- **Header Hygiene**: `grep -r "imgui.h" include/` and `grep -r "implot.h" include/` return zero matches.

### Manual Verification
- Execute `signal-spy` binary, verify window layout docking, widget interactions, menu navigation, status bar counters, and smooth rendering.
