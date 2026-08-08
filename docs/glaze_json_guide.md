# Glaze JSON Usage & Reflection Guide (v8.0.0)

## 1. Overview

**Glaze** is a state-of-the-art, high-performance C++20/C++23/C++26 serialization and reflection library. Unlike traditional C++ JSON libraries (e.g., `nlohmann::json`), Glaze leverages compile-time structural reflection to automatically map C++ data types to/from JSON and binary formats without macros, code generators, or runtime DOM overhead.

### Key Benefits for Signal Spy:
- **Zero Boilerplate**: C++ aggregate structs work automatically out of the box with zero custom serialization code.
- **Maximum Performance**: Direct memory layout mapping with compile-time reflection yields near native-binary parsing throughput.
- **Type Safety**: Enforces strict compile-time typing without intermediate JSON object allocations.
- **Multi-Format Versatility**: Use the exact same C++ types for JSON, binary (BEVE), TOML, YAML, and CSV.
- **Ideal for Signal Spy**: Perfectly suited for parsing SigMF metadata, saving/loading DSP presets, and exporting signal processing configurations.

---

## 2. Basic Usage & Automatic Reflection

In C++20/C++23/C++26, any standard aggregate `struct` is automatically reflective in Glaze without writing any metadata.

### 2.1 Simple Struct Example

```cpp
#include <glaze/glaze.hpp>
#include <iostream>
#include <string>
#include <vector>

// Standard C++ aggregate struct - no macros or glz::meta needed!
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
    glz::parse_error read_err = glz::read_json(loaded_settings, input_json);
    if (!read_err) {
        std::cout << "Loaded signal: " << loaded_settings.signal_name << "\n";
    }
}
```

---

## 3. Custom Reflection Mechanics (`glz::meta`)

When member names in C++ do not match desired JSON key names, or when dealing with private members / non-aggregate classes, Glaze provides two equivalent mechanisms to specify custom metadata.

### 3.1 Method A: External `glz::meta<T>` Specialization (Recommended for Third-Party or Clean Headers)

Specializing `glz::meta<T>` outside the struct keeps your domain models completely decoupled from the Glaze library header inclusions.

```cpp
#include <glaze/glaze.hpp>

// Third-party or domain struct with non-matching JSON keys
struct DSPParams {
    double freq_hz = 144.2e6;
    double gain_db = 12.5;
    bool enable_agc = true;
};

// Specialize glz::meta<DSPParams> in namespace glz
template <>
struct glz::meta<DSPParams> {
    using T = DSPParams;
    
    // glz::object maps JSON key strings to member pointers
    static constexpr auto value = glz::object(
        "frequency_hz", &T::freq_hz,    // Maps JSON "frequency_hz" -> C++ freq_hz
        "receiver_gain", &T::gain_db,   // Maps JSON "receiver_gain" -> C++ gain_db
        "agc_enabled",   &T::enable_agc // Maps JSON "agc_enabled"   -> C++ enable_agc
    );
};
```

### 3.2 Method B: Internal `struct glz` (Nested Inside Struct)

If you own the struct definition and prefer keeping reflection metadata adjacent to the member declarations, define a nested `struct glz`:

```cpp
#include <glaze/glaze.hpp>

struct WaterfallConfig {
    double min_power_db = -100.0;
    double max_power_db = -10.0;
    std::string colormap = "viridis";
    bool internal_gpu_flag = false; // Intentionally omitted from reflection

    // Nested reflection definition
    struct glz {
        using T = WaterfallConfig;
        static constexpr auto value = glz::object(
            "min_db",  &T::min_power_db, // Custom JSON key
            "max_db",  &T::max_power_db, // Custom JSON key
            "palette", &T::colormap     // Custom JSON key
            // internal_gpu_flag is omitted and will NOT be serialized/deserialized
        );
    };
};
```

### 3.3 Enum Class Reflection

Glaze supports serializing C++ `enum class` types as readable string literals in JSON:

```cpp
enum class WindowType {
    Hann,
    Hamming,
    BlackmanHarris,
    Rectangular
};

template <>
struct glz::meta<WindowType> {
    using enum WindowType;
    static constexpr auto value = glz::enumerate(
        "hann",            Hann,
        "hamming",         Hamming,
        "blackman_harris", BlackmanHarris,
        "rectangular",     Rectangular
    );
};
```

---

## 4. Standout Features in Glaze v8.0.0

### 4.1 Automated JSON Schema Generation

Glaze v8.0.0 can automatically generate a standard Draft-07 JSON Schema directly from any C++ type at compile time:

```cpp
#include <glaze/glaze.hpp>

SignalSettings settings;
std::string schema_json;

// Generates valid JSON Schema for SignalSettings struct
glz::write_json_schema<SignalSettings>(schema_json);
```

### 4.2 Multi-Format Support (BEVE Binary, TOML, YAML, CSV)

The exact same C++ reflection definitions work across multiple serialization formats by switching the function call:

```cpp
SignalSettings config;
std::string buffer;

// 1. Binary Efficient Versatile Encoding (BEVE) - High performance binary format
auto write_beve_err = glz::write_beve(config, buffer);
SignalSettings beve_config;
auto read_beve_err = glz::read_beve(beve_config, buffer);

// 2. TOML configuration format (read & write)
std::string toml_str;
auto write_toml_err = glz::write_toml(config, toml_str);
SignalSettings toml_config;
auto read_toml_err = glz::read_toml(toml_config, toml_str);

// 3. YAML format (read & write)
std::string yaml_str;
auto write_yaml_err = glz::write_yaml(config, yaml_str);
SignalSettings yaml_config;
auto read_yaml_err = glz::read_yaml(yaml_config, yaml_str);

// 4. CSV format for vectors/lists of structs (read & write)
std::vector<SignalSettings> list = {config};
std::string csv_str;
auto write_csv_err = glz::write_csv(list, csv_str);
std::vector<SignalSettings> csv_list;
auto read_csv_err = glz::read_csv(csv_list, csv_str);
```

### 4.3 JSON Pointer & Value Queries (`glz::get`)

Extract specific nested values from a JSON string without parsing the entire structure:

```cpp
std::string json_str = R"({"dsp": {"fft_size": 4096, "window": "hann"}})";

// Extract specific value by JSON pointer path
auto fft_size = glz::get<uint32_t>("/dsp/fft_size", json_str);
if (fft_size) {
    std::cout << "Extracted FFT Size: " << *fft_size << "\n";
}
```

### 4.4 Direct File I/O Helpers

Glaze provides helper functions for reading and writing directly to disk:

```cpp
SignalSettings settings;

// Write formatted / indented JSON to file
auto write_err = glz::write_file_json<glz::opts{.indented = true}>(settings, "config.json", std::string{});

// Read JSON from file into C++ struct
auto read_err = glz::read_file_json(settings, "config.json", std::string{});
if (read_err) {
    std::cerr << "File Read Error: " << glz::format_error(read_err, "") << "\n";
}
```

---

## 5. Pretty Printing & Formatting Options

Glaze behavior can be customized via compile-time template options `glz::opts`:

```cpp
SignalSettings settings;
std::string pretty_json;

// Format with 3-space indentation and sorted keys
glz::write<glz::opts{.indented = true, .indent_char = ' ', .indentation = 3}>(settings, pretty_json);
```

---

## 6. Error Handling Diagnostics

`glz::read_json` returns a `glz::parse_error` struct containing character offset and error code details:

```cpp
SignalSettings settings;
std::string bad_json = R"({"center_frequency_hz": "invalid_string_instead_of_number"})";

glz::parse_error err = glz::read_json(settings, bad_json);

if (err) {
    std::cout << "Error Code: " << static_cast<int>(err.ec) << "\n";
    std::cout << "Character Offset: " << err.location << "\n";
    // Human-readable formatted error message highlighting line/character location
    std::cout << glz::format_error(err, bad_json) << "\n";
}
```

---

## 7. CMake Integration in Signal Spy

Glaze is imported as a submodule via `FetchContent` in [`submodules/glaze.cmake`](../submodules/glaze.cmake) (pinning tag `v8.0.0`).

To use Glaze in any CMake target:

```cmake
target_link_libraries(your_target_name PRIVATE glaze::glaze)
```
