#pragma once

#include "ui/i_widget.hpp"
#include "ui/ui_types.hpp"

#include <memory>

namespace spy::ui
{

/**
 * @brief Pure abstract base class for the UI Renderer.
 *
 * Manages the application window, graphics context, ImGui frame loop,
 * and widget execution without exposing third-party graphics headers in public APIs.
 */
class IUIRenderer
{
  public:
    virtual ~IUIRenderer() = default;

    /// Initializes the windowing backend, graphics context, and ImGui subsystems.
    virtual bool Initialize(const UIRendererConfig& config) = 0;

    /// Registers a widget to be rendered in the main render loop.
    virtual void RegisterWidget(std::shared_ptr<IWidget> widget) = 0;

    /// Returns true if the user requested the application window to close.
    [[nodiscard]] virtual bool ShouldClose() const noexcept = 0;

    /// Starts a new frame (polls OS events, begins ImGui frame).
    virtual void BeginFrame() = 0;

    /// Executes Render() on all registered widgets.
    virtual void RenderWidgets() = 0;

    /// Completes the frame (renders ImGui draw data, swaps OpenGL buffers).
    virtual void EndFrame() = 0;

    /// Releases graphics resources and cleans up GLFW / ImGui contexts.
    virtual void Shutdown() = 0;
};

/// Factory function creating an instance of the UI renderer.
[[nodiscard]] std::unique_ptr<IUIRenderer> CreateUIRenderer();

} // namespace spy::ui
