#pragma once

#include "signal/signal_source.hpp"

namespace spy
{

/// Signal source for live radio-frequency input from SDR devices
/// or other RF receivers.
class RadioSource final : public ISignalSource
{
  public:
    [[nodiscard]] auto name() const -> std::string_view override;
    [[nodiscard]] auto type() const -> SignalSourceType override;
};

} // namespace spy
