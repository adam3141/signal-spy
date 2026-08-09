#pragma once

#include "ui/i_ui_renderer.hpp"
#include <atomic>
#include <memory>
#include <string>
#include <string_view>

namespace spy::app
{

/**
 * @brief Application configuration settings.
 */
struct AppConfig
{
    int window_width{1600};
    int window_height{900};
    std::string window_title{"Signal Spy"};
    bool vsync{true};
};

/**
 * @brief State machine enum representing the overall application lifecycle.
 */
enum class AppState
{
    Uninitialized,
    Initializing,
    Running,
    Stopping,
    Shutdown
};

/**
 * @brief Core Application controller class.
 *
 * Manages the top-level application lifecycle, window renderer, thread orchestration,
 * signal handling, and main event loop execution.
 */
class Application
{
  public:
    explicit Application(AppConfig config = {});
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) noexcept;
    Application& operator=(Application&&) noexcept;

    /**
     * @brief Initializes application subsystems, window, and graphics context.
     * @return True if initialization succeeded, false otherwise.
     */
    bool Initialize();

    /**
     * @brief Executes the main application loop until exit is requested.
     * @return Exit status code (0 for clean exit, non-zero on error).
     */
    int Run();

    /**
     * @brief Requests application stop/shutdown.
     */
    void Stop() noexcept;

    /**
     * @brief Returns the current state of the application lifecycle.
     */
    [[nodiscard]] AppState get_State() const noexcept;

    /**
     * @brief Registers OS signal handlers (SIGINT, SIGTERM) for graceful shutdown.
     */
    static void RegisterSignalHandlers();

  private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace spy::app
