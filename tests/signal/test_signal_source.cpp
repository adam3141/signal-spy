#include <gtest/gtest.h>

#include "signal/file_source.hpp"
#include "signal/null_source.hpp"
#include "signal/radio_source.hpp"
#include "signal/signal_source.hpp"
#include "signal/signal_source_factory.hpp"

// ---------------------------------------------------------------------------
// NullSource
// ---------------------------------------------------------------------------

TEST(NullSourceTest, Name)
{
    spy::NullSource source;
    EXPECT_EQ(source.name(), "NullSource");
}

TEST(NullSourceTest, Type)
{
    spy::NullSource source;
    EXPECT_EQ(source.type(), spy::SignalSourceType::Null);
}

// ---------------------------------------------------------------------------
// FileSource
// ---------------------------------------------------------------------------

TEST(FileSourceTest, Name)
{
    spy::FileSource source;
    EXPECT_EQ(source.name(), "FileSource");
}

TEST(FileSourceTest, Type)
{
    spy::FileSource source;
    EXPECT_EQ(source.type(), spy::SignalSourceType::File);
}

// ---------------------------------------------------------------------------
// RadioSource
// ---------------------------------------------------------------------------

TEST(RadioSourceTest, Name)
{
    spy::RadioSource source;
    EXPECT_EQ(source.name(), "RadioSource");
}

TEST(RadioSourceTest, Type)
{
    spy::RadioSource source;
    EXPECT_EQ(source.type(), spy::SignalSourceType::Radio);
}

// ---------------------------------------------------------------------------
// SignalSourceFactory — create by enum
// ---------------------------------------------------------------------------

TEST(SignalSourceFactoryTest, CreateNullByEnum)
{
    auto source = spy::SignalSourceFactory::create(spy::SignalSourceType::Null);
    ASSERT_NE(source, nullptr);
    EXPECT_EQ(source->type(), spy::SignalSourceType::Null);
    EXPECT_EQ(source->name(), "NullSource");
}

TEST(SignalSourceFactoryTest, CreateFileByEnum)
{
    auto source = spy::SignalSourceFactory::create(spy::SignalSourceType::File);
    ASSERT_NE(source, nullptr);
    EXPECT_EQ(source->type(), spy::SignalSourceType::File);
    EXPECT_EQ(source->name(), "FileSource");
}

TEST(SignalSourceFactoryTest, CreateRadioByEnum)
{
    auto source = spy::SignalSourceFactory::create(spy::SignalSourceType::Radio);
    ASSERT_NE(source, nullptr);
    EXPECT_EQ(source->type(), spy::SignalSourceType::Radio);
    EXPECT_EQ(source->name(), "RadioSource");
}

// ---------------------------------------------------------------------------
// SignalSourceFactory — create by string key
// ---------------------------------------------------------------------------

TEST(SignalSourceFactoryTest, CreateNullByString)
{
    auto source = spy::SignalSourceFactory::create("null");
    ASSERT_NE(source, nullptr);
    EXPECT_EQ(source->type(), spy::SignalSourceType::Null);
}

TEST(SignalSourceFactoryTest, CreateFileByString)
{
    auto source = spy::SignalSourceFactory::create("file");
    ASSERT_NE(source, nullptr);
    EXPECT_EQ(source->type(), spy::SignalSourceType::File);
}

TEST(SignalSourceFactoryTest, CreateRadioByString)
{
    auto source = spy::SignalSourceFactory::create("radio");
    ASSERT_NE(source, nullptr);
    EXPECT_EQ(source->type(), spy::SignalSourceType::Radio);
}

// ---------------------------------------------------------------------------
// SignalSourceFactory — unknown types
// ---------------------------------------------------------------------------

TEST(SignalSourceFactoryTest, UnknownStringReturnsNull)
{
    auto source = spy::SignalSourceFactory::create("unknown_source");
    EXPECT_EQ(source, nullptr);
}

// ---------------------------------------------------------------------------
// SignalSourceFactory — custom registration
// ---------------------------------------------------------------------------

namespace
{

class TestCustomSource final : public spy::ISignalSource
{
  public:
    [[nodiscard]] auto name() const -> std::string_view override
    {
        return "TestCustomSource";
    }
    [[nodiscard]] auto type() const -> spy::SignalSourceType override
    {
        return static_cast<spy::SignalSourceType>(99);
    }
};

} // namespace

TEST(SignalSourceFactoryTest, RegisterCustomSource)
{
    auto customType = static_cast<spy::SignalSourceType>(99);
    spy::SignalSourceFactory::registerSource(customType, "custom",
                                             []() { return std::make_unique<TestCustomSource>(); });

    auto byEnum = spy::SignalSourceFactory::create(customType);
    ASSERT_NE(byEnum, nullptr);
    EXPECT_EQ(byEnum->name(), "TestCustomSource");

    auto byKey = spy::SignalSourceFactory::create("custom");
    ASSERT_NE(byKey, nullptr);
    EXPECT_EQ(byKey->name(), "TestCustomSource");
}
