#ifndef SPY_UI_I_UI_RENDERER_HPP
#define SPY_UI_I_UI_RENDERER_HPP

#include "ui/i_widget.hpp"
#include "ui/ui_types.hpp"

#include <memory>

namespace spy::ui {

/**
 * @brief Pure abstract base class for the UI Renderer.
 * 
 * Manages the application window, graphics context, ImGui frame loop,
 * and widget execution without exposing third-party graphics headers in public APIs.
 */
class IUIRenderer {
public:
    virtual ~IUIRenderer() = default;

    /// Initializes the windowing backend, graphics context, and ImGui subsystems.
    virtual bool initialize(const UIRendererConfig& config) = 0;

    /// Registers a widget to be rendered in the main render loop.
    virtual void register_widget(std::shared_ptr<IWidget> widget) = 0;

    /// Returns true if the user requested the application window to close.
    [[nodiscard]] virtual bool should_close() const noexcept = 0;

    /// Starts a new frame (polls OS events, begins ImGui frame).
    virtual void begin_frame() = 0;

    /// Executes render() on all registered widgets.
    virtual void render_widgets() = 0;

    /// Completes the frame (renders ImGui draw data, swaps OpenGL buffers).
    virtual void end_frame() = 0;

    /// Releases graphics resources and cleans up GLFW / ImGui contexts.
    virtual void shutdown() = 0;
};

/// Factory function creating an instance of the UI renderer.
[[nodiscard]] std::unique_ptr<IUIRenderer> create_ui_renderer();

} // namespace spy::ui

#endif // SPY_UI_I_UI_RENDERER_HPP

