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
    void RegisterWidget(std::shared_ptr<IWidget> widget) override;
    [[nodiscard]] bool ShouldClose() const noexcept override;
    void BeginFrame() override;
    void RenderWidgets() override;
    void EndFrame() override;
    void Shutdown() override;

private:
    void setup_theme();

    GLFWwindow* _window{nullptr};
    UIRendererConfig _config{};
    std::vector<std::shared_ptr<IWidget>> _widgets{};
    bool _initialized{false};
};

} // namespace spy::ui
