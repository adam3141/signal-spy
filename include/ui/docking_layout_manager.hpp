#ifndef SPY_UI_DOCKING_LAYOUT_MANAGER_HPP
#define SPY_UI_DOCKING_LAYOUT_MANAGER_HPP

#include "ui/i_widget.hpp"
#include <memory>

namespace spy::ui {

/**
 * @brief Docking Layout Manager widget handling main dockspace and window docking.
 */
class DockingLayoutManager : public IWidget {
public:
    DockingLayoutManager();
    ~DockingLayoutManager() override;

    DockingLayoutManager(const DockingLayoutManager&) = delete;
    DockingLayoutManager& operator=(const DockingLayoutManager&) = delete;
    DockingLayoutManager(DockingLayoutManager&&) noexcept;
    DockingLayoutManager& operator=(DockingLayoutManager&&) noexcept;

    void render() override;
    [[nodiscard]] std::string_view title() const noexcept override;
    [[nodiscard]] bool is_visible() const noexcept override;
    void set_visible(bool visible) noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace spy::ui

#endif // SPY_UI_DOCKING_LAYOUT_MANAGER_HPP

