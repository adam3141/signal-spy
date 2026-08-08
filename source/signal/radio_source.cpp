#include "signal/radio_source.hpp"

namespace spy
{

auto RadioSource::name() const -> std::string_view
{
    return "RadioSource";
}

auto RadioSource::type() const -> SignalSourceType
{
    return SignalSourceType::Radio;
}

} // namespace spy
