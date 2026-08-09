#include "app/application.hpp"
#include <gtest/gtest.h>

TEST(ApplicationTest, DefaultConfigValues)
{
    spy::app::AppConfig config;
    EXPECT_EQ(config.window_width, 1600);
    EXPECT_EQ(config.window_height, 900);
    EXPECT_EQ(config.window_title, "Signal Spy");
    EXPECT_TRUE(config.vsync);
}

TEST(ApplicationTest, InitialStateUninitialized)
{
    spy::app::Application app;
    EXPECT_EQ(app.get_State(), spy::app::AppState::Uninitialized);
}

TEST(ApplicationTest, StopMutatesState)
{
    spy::app::Application app;
    app.Stop();
    EXPECT_EQ(app.get_State(), spy::app::AppState::Uninitialized);
}
