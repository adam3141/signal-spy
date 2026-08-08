# Signal Spy Requirements

## 1. Overview
Signal Spy is a high-performance C++ application designed to visualize and analyze signals in real time and from recorded data. It provides dual primary representations:
1. **Waterfall Display (Frequency vs. Time)** for spectral history analysis.
2. **Oscilloscope Display (Amplitude vs. Time)** for time-domain waveform analysis.

---

## 2. Input Data & Processing Requirements

### 2.1 Input Sources
- **Live Streams**: Support for raw I/Q (In-phase / Quadrature) sample streams from SDR (Software Defined Radio) devices and audio input interfaces.
- **File Playback**: Support reading recorded signal formats, including SigMF (Signal Metadata Format), raw complex float32/int16 binary files, and standard WAV audio files.

### 2.2 Digital Signal Processing (DSP) Pipeline
- **FFT Engine**: Configurable FFT sizes (e.g., 512 to 65,536 points) with hardware acceleration support.
- **Windowing Functions**: Selectable windowing algorithms (e.g., Hann, Hamming, Blackman-Harris, Rectangular, Flat-Top).
- **Averaging & Smoothing**: Adjustable spectrum averaging (moving average, exponential decay, peak hold).
- **Sample Rate & Decimation**: Configurable sample rates and decimation/filtering stages to zoom into specific frequency bands.

---

## 3. Visualization Requirements

### 3.1 Waterfall Display (Frequency vs. Time)
- **Time History Heatmap**: Continuous scrolling 2D heatmap showing frequency spectrum power over time.
- **Color Palettes**: Multiple configurable colormaps (e.g., Viridis, Inferno, Turbo, Grayscale) with custom color mapping support.
- **Dynamic Range Controls**: Interactive gain, minimum/maximum dB threshold adjustments, and contrast control.
- **Scroll Speed**: Variable scroll speed and line history buffer depth.

### 3.2 Time-Domain Display (Amplitude vs. Time)
- **Waveform Viewer**: Real-time oscilloscope-style display of raw amplitude (I/Q channels or envelope power) over time.
- **Constellation Diagram**: 2D I/Q plot (In-phase vs. Quadrature) for modulation analysis.
- **Trigger Modes**: Auto, Normal, and Single-shot trigger modes with adjustable edge threshold and pre-trigger buffers.

### 3.3 Frequency Spectrum Display (Power Spectrum Density)
- **2D Spectrum Plot**: Instantaneous power vs. frequency curve aligned vertically/horizontally with the waterfall display.
- **Peak & Noise Floor Detection**: Automatic peak markers and noise floor estimation overlays.
- **Interactive Cursors**: Multi-cursor measurement system for delta-frequency ($\Delta f$), bandwidth, and signal-to-noise ratio (SNR) measurements.

---

## 4. User Interface & Interactivity

- **Layout Management**: Customizable and dockable UI panes allowing users to toggle, stack, or detach waterfall, time-domain, and spectrum views.
- **Axis Navigation**: Pan and zoom capability across frequency (Hz/kHz/MHz) and time (ms/s) axes.
- **Presets & Profiles**: Ability to save and restore UI layouts and DSP processing configurations.

---

## 5. Data Export & Storage

- **Data Recording**: Real-time recording of input streams to SigMF format with metadata tags.
- **Image & Data Capture**: Export current view snapshots (PNG/SVG) and raw spectrum frame data (CSV/JSON).

---

## 6. Non-Functional & Technical Requirements

- **Performance**: High frame rate rendering (minimum 60 FPS for display updates) with minimal CPU usage on main UI thread.
- **Latency**: End-to-end processing latency under 50ms from input sample ingestion to display frame render.
- **Architecture & Standards**:
  - Built using C++26 standard features.
  - Profiled with Tracy (`TRACY_ENABLE` in `RelWithDebInfo` builds).
  - Cross-platform modular CMake build architecture (`CMakePresets.json`).
