#ifndef SIGNAL_SPY_UI_I_WIDGET_HPP
#define SIGNAL_SPY_UI_I_WIDGET_HPP

#include <string_view>

namespace signal_spy::ui {

/**
 * @brief Abstract interface for all UI widgets and dockable panels.
 * 
 * Public headers MUST NOT include third-party GUI headers like imgui.h.
 */
class IWidget {
public:
    virtual ~IWidget() = default;

    /// Renders the widget content during the active frame loop.
    virtual void render() = 0;

    /// Returns the human-readable display title of the widget panel.
    [[nodiscard]] virtual std::string_view title() const noexcept = 0;

    /// Returns whether the widget window is currently open/visible.
    [[nodiscard]] virtual bool is_visible() const noexcept = 0;

    /// Sets the visibility state of the widget window.
    virtual void set_visible(bool visible) noexcept = 0;
};

} // namespace signal_spy::ui

#endif // SIGNAL_SPY_UI_I_WIDGET_HPP
