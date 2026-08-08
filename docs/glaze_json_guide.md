# Glaze JSON Usage & Reflection Guide

## 1. Overview

**Glaze** is a high-performance, header-only C++20/C++23/C++26 JSON serialization and deserialization library. Unlike traditional C++ JSON libraries (e.g., `nlohmann::json`), Glaze leverages compile-time structural reflection to automatically map C++ data types to/from JSON without macros or runtime overhead.

### Key Benefits for Signal Spy:
- **Zero Boilerplate**: C++ aggregate structs work automatically with zero serialization code.
- **Maximum Performance**: Direct memory layout mapping with compile-time reflection yields near native-binary parsing speeds.
- **Type Safety**: Enforces strict typing at compile time without intermediate JSON object/DOM allocations.
- **Ideal for Signal Spy Metadata**: Perfectly suited for parsing SigMF metadata, saving/loading DSP presets, and exporting application configurations.

---

## 2. Basic Usage & Automatic Reflection

In C++20/23/26, any standard aggregate `struct` is automatically reflective in Glaze.

### 2.1 Simple Struct Example

```cpp
#include <glaze/glaze.hpp>
#include <iostream>
#include <string>
#include <vector>

struct SignalSettings {
    std::string signal_name = "CW Beacon";
    double center_frequency_hz = 144.200e6;
    double sample_rate_hz = 2.4e6;
    uint32_t fft_size = 2048;
    std::vector<std::string> active_filters = {"hann", "decimate_by_4"};
};

void example() {
    SignalSettings settings;

    // 1. Serialize C++ Struct to JSON string
    std::string json_buffer;
    auto write_err = glz::write_json(settings, json_buffer);
    if (!write_err) {
        std::cout << "Serialized JSON:\n" << json_buffer << "\n";
    }

    // 2. Deserialize JSON string into C++ Struct
    std::string input_json = R"({
        "signal_name": "NFM Telemetry",
        "center_frequency_hz": 433920000.0,
        "sample_rate_hz": 1000000.0,
        "fft_size": 4096,
        "active_filters": ["blackman_harris"]
    })";

    SignalSettings loaded_settings;
    auto read_err = glz::read_json(loaded_settings, input_json);
    if (!read_err) {
        std::cout << "Loaded signal: " << loaded_settings.signal_name << "\n";
    }
}
```

---

## 3. Advanced & Custom Reflection (`glz::meta`)

When default member names or visibility do not match your desired JSON schema, define a `glz::meta` specialization or nested `glz` struct.

### 3.1 Custom Member Names & Field Exclusion

```cpp
#include <glaze/glaze.hpp>

struct WaterfallConfig {
    double min_db = -100.0;
    double max_db = -10.0;
    std::string colormap = "viridis";
    bool internal_render_flag = false; // Exclude from JSON

    // Define custom metadata reflection
    struct glz {
        using T = WaterfallConfig;
        static constexpr auto value = glz::object(
            "min_power_db", &T::min_db,   // Rename field in JSON
            "max_power_db", &T::max_db,   // Rename field in JSON
            "palette",      &T::colormap  // Rename field in JSON
            // internal_render_flag is omitted and will not be serialized
        );
    };
};
```

### 3.2 Enum Serialization

Glaze supports serializing C++ `enum class` types directly as readable JSON strings:

```cpp
enum class WindowFunction {
    Hann,
    Hamming,
    BlackmanHarris,
    Rectangular
};

// Map enum values to JSON string representations
template <>
struct glz::meta<WindowFunction> {
    using enum WindowFunction;
    static constexpr auto value = glz::enumerate(
        "hann", Hann,
        "hamming", Hamming,
        "blackman_harris", BlackmanHarris,
        "rectangular", Rectangular
    );
};
```

---

## 4. Pretty Printing & Formatting

To format JSON with human-readable indentation (e.g. when saving preset files to disk):

```cpp
SignalSettings settings;
std::string pretty_json;

// Write indented JSON
glz::write<glz::opts{.indented = true}>(settings, pretty_json);
```

---

## 5. Error Handling

Glaze returns structured `glz::error_code` objects containing detailed diagnostic information:

```cpp
SignalSettings settings;
std::string bad_json = R"({"center_frequency_hz": "invalid_number_string"})";

glz::parse_error err = glz::read_json(settings, bad_json);

if (err) {
    std::cerr << "JSON Parse Error Code: " << static_cast<int>(err.ec) << "\n";
    std::cerr << "Error location offset: " << err.location << "\n";
    // Format human-readable error description
    std::cerr << glz::format_error(err, bad_json) << "\n";
}
```

---

## 6. CMake Integration in Signal Spy

Glaze is imported as a submodule via `FetchContent` in [`submodules/glaze.cmake`](../submodules/glaze.cmake).

To use Glaze in any target within `CMakeLists.txt`:

```cmake
target_link_libraries(your_target_name PRIVATE glaze::glaze)
```
