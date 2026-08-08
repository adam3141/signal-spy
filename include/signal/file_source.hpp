#pragma once

#include "signal/signal_source.hpp"

namespace spy
{

/// Signal source that reads recorded data from files.
/// Supports SigMF, raw binary, and WAV formats (to be implemented).
class FileSource final : public ISignalSource
{
  public:
    [[nodiscard]] auto name() const -> std::string_view override;
    [[nodiscard]] auto type() const -> SignalSourceType override;
};

} // namespace spy
