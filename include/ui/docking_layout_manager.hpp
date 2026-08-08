#pragma once

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

    void Render() override;
    [[nodiscard]] std::string_view get_Title() const noexcept override;
    [[nodiscard]] bool is_Visible() const noexcept override;
    void set_Visible(bool visible) noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace spy::ui
