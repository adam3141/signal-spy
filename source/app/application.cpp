#include "app/application.hpp"
#include "ui/docking_layout_manager.hpp"

#include <csignal>
#include <iostream>
#include <utility>

namespace spy::app
{

namespace
{
std::atomic<bool> s_stop_requested{false};

void SignalHandler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        s_stop_requested.store(true, std::memory_order_relaxed);
    }
}
} // namespace

struct Application::Impl
{
    AppConfig _config;
    std::atomic<AppState> _state{AppState::Uninitialized};
    std::unique_ptr<spy::ui::IUIRenderer> _renderer;
    std::shared_ptr<spy::ui::DockingLayoutManager> _docking_manager;

    explicit Impl(AppConfig config)
        : _config(std::move(config))
    {
    }
};

Application::Application(AppConfig config)
    : _impl(std::make_unique<Impl>(std::move(config)))
{
}

Application::~Application() = default;

Application::Application(Application&&) noexcept = default;
Application& Application::operator=(Application&&) noexcept = default;

void Application::RegisterSignalHandlers()
{
    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);
}

bool Application::Initialize()
{
    if (_impl->_state.load() != AppState::Uninitialized)
    {
        return false;
    }

    _impl->_state.store(AppState::Initializing);

    RegisterSignalHandlers();

    _impl->_renderer = spy::ui::CreateUIRenderer();
    if (!_impl->_renderer)
    {
        _impl->_state.store(AppState::Shutdown);
        return false;
    }

    spy::ui::UIRendererConfig render_config;
    render_config.window_width = _impl->_config.window_width;
    render_config.window_height = _impl->_config.window_height;
    render_config.window_title = _impl->_config.window_title.c_str();
    render_config.vsync = _impl->_config.vsync;

    if (!_impl->_renderer->Initialize(render_config))
    {
        _impl->_state.store(AppState::Shutdown);
        return false;
    }

    _impl->_docking_manager = std::make_shared<spy::ui::DockingLayoutManager>();
    _impl->_renderer->RegisterWidget(_impl->_docking_manager);

    _impl->_state.store(AppState::Running);
    return true;
}

int Application::Run()
{
    if (_impl->_state.load() != AppState::Running)
    {
        return 1;
    }

    while (_impl->_state.load() == AppState::Running && !s_stop_requested.load(std::memory_order_relaxed))
    {
        if (_impl->_renderer->ShouldClose())
        {
            break;
        }

        _impl->_renderer->BeginFrame();
        _impl->_renderer->RenderWidgets();
        _impl->_renderer->EndFrame();
    }

    _impl->_state.store(AppState::Stopping);

    if (_impl->_renderer)
    {
        _impl->_renderer->Shutdown();
    }

    _impl->_state.store(AppState::Shutdown);
    return 0;
}

void Application::Stop() noexcept
{
    if (_impl->_state.load() == AppState::Running)
    {
        _impl->_state.store(AppState::Stopping);
        s_stop_requested.store(true, std::memory_order_relaxed);
    }
}

AppState Application::get_State() const noexcept
{
    return _impl->_state.load();
}

} // namespace spy::app
