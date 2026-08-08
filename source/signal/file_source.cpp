#include "signal/file_source.hpp"

namespace spy
{

auto FileSource::name() const -> std::string_view
{
    return "FileSource";
}

auto FileSource::type() const -> SignalSourceType
{
    return SignalSourceType::File;
}

} // namespace spy
