#pragma once

#include "signal/signal_source.hpp"

namespace spy
{

/// Placeholder signal source that produces no data.
/// Useful as a default or sentinel value when no real source is configured.
class NullSource final : public ISignalSource
{
  public:
    [[nodiscard]] auto name() const -> std::string_view override;
    [[nodiscard]] auto type() const -> SignalSourceType override;
};

} // namespace spy
