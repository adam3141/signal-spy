#include "signal/null_source.hpp"

namespace spy
{

auto NullSource::name() const -> std::string_view
{
    return "NullSource";
}

auto NullSource::type() const -> SignalSourceType
{
    return SignalSourceType::Null;
}

} // namespace spy
