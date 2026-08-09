#include "app/application.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    spy::app::AppConfig config;
    config.window_width = 1600;
    config.window_height = 900;
    config.window_title = "Signal Spy";
    config.vsync = true;

    spy::app::Application app(config);

    if (!app.Initialize())
    {
        std::cerr << "Failed to initialize Signal Spy application." << std::endl;
        return 1;
    }

    return app.Run();
}
