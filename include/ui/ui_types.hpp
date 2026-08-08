#pragma once

#include <cstdint>
#include <string>
#include <vector>

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

struct UIRendererConfig {
    int window_width{1600};
    int window_height{900};
    std::string window_title{"Signal Spy"};
    bool vsync{true};
    bool headless{false};
};

} // namespace spy::ui
