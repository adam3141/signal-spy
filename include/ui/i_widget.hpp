#pragma once

#include <string_view>

namespace spy::ui
{

/**
 * @brief Abstract interface for all UI widgets and dockable panels.
 *
 * Public headers MUST NOT include third-party GUI headers like imgui.h.
 */
class IWidget
{
  public:
    virtual ~IWidget() = default;

    /// Renders the widget content during the active frame loop.
    virtual void Render() = 0;

    /// Returns the human-readable display title of the widget panel.
    [[nodiscard]] virtual std::string_view get_Title() const noexcept = 0;

    /// Returns whether the widget window is currently open/visible.
    [[nodiscard]] virtual bool is_Visible() const noexcept = 0;

    /// Sets the visibility state of the widget window.
    virtual void set_Visible(bool visible) noexcept = 0;
};

} // namespace spy::ui
