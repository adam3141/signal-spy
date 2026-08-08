#pragma once

#include "ui/i_ui_renderer.hpp"

#include <memory>
#include <vector>

struct GLFWwindow;

namespace spy::ui {

class ImGuiUIRenderer : public IUIRenderer {
public:
    ImGuiUIRenderer();
    ~ImGuiUIRenderer() override;

    bool Initialize(const UIRendererConfig& config) override;
    void Register_Widget(std::shared_ptr<IWidget> widget) override;
    [[nodiscard]] bool Should_Close() const noexcept override;
    void Begin_Frame() override;
    void Render_Widgets() override;
    void End_Frame() override;
    void Shutdown() override;

private:
    void setup_theme();

    GLFWwindow* _window{nullptr};
    UIRendererConfig _config{};
    std::vector<std::shared_ptr<IWidget>> _widgets{};
    bool _initialized{false};
};

} // namespace spy::ui
