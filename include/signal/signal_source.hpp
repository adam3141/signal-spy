#pragma once

#include <memory>
#include <string_view>

namespace spy
{

/// Identifies the category of a signal source.
/// Used as a factory key and for config-file mapping.
enum class SignalSourceType
{
    Null,
    File,
    Radio
};

/// Abstract interface for all signal sources.
///
/// Concrete implementations provide identity information and will later
/// be extended with data-flow methods (read, configure, etc.) once the
/// DSP pipeline design is finalized.
class ISignalSource
{
  public:
    virtual ~ISignalSource() = default;

    /// Human-readable name of this source (e.g. "RadioSource").
    [[nodiscard]] virtual auto name() const -> std::string_view = 0;

    /// Returns the enumerated type of this source.
    [[nodiscard]] virtual auto type() const -> SignalSourceType = 0;
};

} // namespace spy