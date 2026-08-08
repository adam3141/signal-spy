#include "signal/signal_source_factory.hpp"

#include "signal/file_source.hpp"
#include "signal/null_source.hpp"
#include "signal/radio_source.hpp"

namespace spy
{

auto SignalSourceFactory::create(SignalSourceType type) -> std::unique_ptr<ISignalSource>
{
    auto& reg = registry();
    auto it = reg.byType.find(type);
    if (it == reg.byType.end())
    {
        return nullptr;
    }
    return it->second();
}

auto SignalSourceFactory::create(std::string_view key) -> std::unique_ptr<ISignalSource>
{
    auto& reg = registry();
    auto it = reg.byKey.find(std::string{key});
    if (it == reg.byKey.end())
    {
        return nullptr;
    }
    return it->second();
}

void SignalSourceFactory::registerSource(SignalSourceType type, std::string key, CreatorFn creator)
{
    auto& reg = registry();
    reg.byType[type] = creator;
    reg.byKey[std::move(key)] = std::move(creator);
}

auto SignalSourceFactory::registry() -> Registry&
{
    static Registry reg = []() {
        Registry r;

        r.byType[SignalSourceType::Null] = []() { return std::make_unique<NullSource>(); };
        r.byType[SignalSourceType::File] = []() { return std::make_unique<FileSource>(); };
        r.byType[SignalSourceType::Radio] = []() { return std::make_unique<RadioSource>(); };

        r.byKey["null"] = []() { return std::make_unique<NullSource>(); };
        r.byKey["file"] = []() { return std::make_unique<FileSource>(); };
        r.byKey["radio"] = []() { return std::make_unique<RadioSource>(); };

        return r;
    }();

    return reg;
}

} // namespace spy
