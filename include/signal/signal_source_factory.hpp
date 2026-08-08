#pragma once

#include "signal/signal_source.hpp"

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace spy
{

/// Factory for creating signal source instances.
///
/// Supports creation by enum type or string key (for config-file driven
/// instantiation). New source types can be registered at runtime via
/// registerSource() without modifying the factory itself.
class SignalSourceFactory
{
  public:
    using CreatorFn = std::function<std::unique_ptr<ISignalSource>()>;

    /// Create a signal source by its enumerated type.
    /// Returns nullptr if the type is not registered.
    [[nodiscard]] static auto create(SignalSourceType type) -> std::unique_ptr<ISignalSource>;

    /// Create a signal source from a string key (e.g. "radio", "file", "null").
    /// Returns nullptr if the key is not registered.
    [[nodiscard]] static auto create(std::string_view key) -> std::unique_ptr<ISignalSource>;

    /// Register a custom source type with both an enum and a string key.
    /// This allows extending the factory without modifying its implementation.
    static void registerSource(SignalSourceType type, std::string key, CreatorFn creator);

  private:
    struct Registry
    {
        std::unordered_map<SignalSourceType, CreatorFn> byType;
        std::unordered_map<std::string, CreatorFn> byKey;
    };

    /// Returns the singleton registry, pre-populated with built-in source types.
    static auto registry() -> Registry&;
};

} // namespace spy
