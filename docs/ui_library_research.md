# User Interface Library Evaluation for Signal Spy

## 1. Overview & Goal

Signal Spy requires a user interface (UI) framework capable of delivering a modern, clean visual appearance while handling demanding real-time data visualization requirements:
- **High-Performance Rendering**: Smooth 60+ FPS display updates for fast-scrolling waterfall heatmaps, time-domain oscilloscope sweeps, and power spectrum density plots.
- **Dockable & Customizable Layouts**: Flexible windowing allowing users to stack, tile, or pop out waterfall, spectrum, and control panels.
- **Modern Aesthetics**: Sleek dark mode styling, clean typography, anti-aliased vector controls, and responsive feedback.
- **CMake & FetchContent Compatibility**: In alignment with [`submodules/README.md`](../submodules/README.md), the chosen UI framework should ideally be light, self-contained, and easily integrated using CMake's `FetchContent` mechanism without heavy external system dependencies.

---

## 2. Comparison Matrix

| Framework | Architecture | Real-Time Plotting | Docking / Layout | Aesthetics | FetchContent Build Overhead | Licensing | Overall Rank |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Dear ImGui + ImPlot** | Immediate Mode | Excellent (Native / ImPlot) | Excellent (`docking` branch) | High (Custom Themes) | Extremely Low (~seconds) | MIT | **1 (Primary)** |
| **Qt 6 (QML / Qt Quick)** | Retained / Declarative | Good (QCustomPlot / QtCharts) | Excellent (Native Dock) | Premium / Modern | High (System / External install) | LGPLv3 / Commercial | **2 (Alternative)** |
| **Slint (SixtyFPS)** | Retained / Declarative | Moderate (Custom OpenGL) | Basic | Modern | Moderate (Rust toolchain opt) | GPLv3 / Royalty-Free | **3** |
| **RmlUi** | HTML/CSS Retained | Moderate (Custom Canvas) | Manual / CSS | Customizable | Low | MIT | **4** |

---

## 3. Detailed Library Analysis

### 3.1 Candidate A: Dear ImGui + ImPlot (Recommended)

**Dear ImGui** is the industry standard immediate-mode GUI library for C++ real-time tools, game engines, and signal processing software (e.g., SDR++, GNU Radio tools, ImHex). When paired with **ImPlot**, it provides a complete framework for scientific plotting.

#### Key Advantages:
- **Native Real-Time Plotting**: ImPlot provides high-performance GPU-accelerated 2D line plots, scatter plots, shaded plots, and heatmaps capable of rendering tens of thousands of points smoothly.
- **Built-in Docking & Multi-Viewports**: The official `docking` branch of ImGui allows users to drag, dock, tab, and detach windows into separate native OS windows seamlessly.
- **Waterfall Texture Integration**: Direct access to underlying OpenGL/Vulkan/Metal textures makes uploading dynamic FFT waterfall lines trivial and performant.
- **Zero Heavy Dependencies**: Pure C++ code with minimal external build footprint.
- **FetchContent Integration**: Integrates cleanly using `FetchContent` (e.g., fetching GLFW/SDL3 backend + ImGui + ImPlot).

#### Considerations:
- Requires custom theme configuration (e.g., modern dark theme palette, custom font loading like Inter/Roboto) to achieve a modern visual style instead of the default classic theme.

---

### 3.2 Candidate B: Qt 6 (QML / Qt Quick)

**Qt 6** is a comprehensive, production-grade C++ framework offering state-of-the-art UI capabilities via QML (Qt Meta-object Language) and Qt Quick.

#### Key Advantages:
- **Flawless Visual Polish**: Support for smooth animations, fluid layout transitions, dynamic glassmorphism/shadow effects, and high-DPI scaling out of the box.
- **Rich Ecosystem**: Built-in signal/slot architecture, rich control library, and mature windowing system.

#### Considerations:
- **Heavy Dependency**: Qt cannot be easily built on-the-fly via `FetchContent` inside a lightweight project build due to its size and build system complexity. Users/developers must pre-install Qt 6 on their system or use package managers like `vcpkg` or `conan`.
- **Licensing**: LGPLv3 requires careful dynamic linking consideration.

---

### 3.3 Candidate C: Slint (formerly SixtyFPS)

**Slint** is a modern declarative UI toolkit designed for Rust and C++ with a custom `.slint` markup language and GPU-accelerated rendering backends.

#### Key Advantages:
- Sleek, modern design language out of the box.
- Declarative syntax for fast UI layout prototyping.

#### Considerations:
- Lack of native high-speed scientific plotting libraries (like ImPlot), requiring custom OpenGL surface bridging for waterfall and spectrum plots.
- Requires C++ bindings generation and optional Rust toolchain integration.

---

### 3.4 Candidate D: RmlUi

**RmlUi** is a C++ user interface library based on the HTML and CSS standards.

#### Key Advantages:
- Layout and styling using familiar web standards (RHTML/RCSS).

#### Considerations:
- High-frequency real-time signal plotting and heatmaps require manual custom canvas integration.
- Higher CPU overhead compared to immediate-mode rendering for rapidly changing real-time data displays.

---

## 4. Integration Blueprint (CMake & FetchContent)

In accordance with [`submodules/README.md`](../submodules/README.md), dependencies should be managed cleanly via CMake's `FetchContent` module in the `submodules/` directory.

### Proposed Submodule Structure for Dear ImGui + ImPlot:

```
submodules/
├── CMakeLists.txt
├── README.md
├── googletest.cmake
├── glfw.cmake
├── imgui.cmake
└── implot.cmake
```

#### Example `submodules/imgui.cmake`:
```cmake
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        docking
  GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(imgui)
```

#### Example `submodules/implot.cmake`:
```cmake
FetchContent_Declare(
  implot
  GIT_REPOSITORY https://github.com/epezent/implot.git
  GIT_TAG        v0.16
  GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(implot)
```

---

## 5. Recommendation & Conclusion

We recommend selecting **Dear ImGui (docking branch) + ImPlot + GLFW/SDL3** as the primary UI framework for Signal Spy.

### Reasons:
1. Best-in-class performance for real-time DSP, FFT waterfalls, and oscilloscope rendering.
2. Native dockable window architecture.
3. Clean compliance with the project's `FetchContent` dependency strategy defined in [`submodules/README.md`](../submodules/README.md).
4. Highly customizable styling engine to achieve a modern, dark-themed UI.
