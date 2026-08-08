#ifndef SPY_SOURCE_UI_IMGUI_UI_RENDERER_HPP
#define SPY_SOURCE_UI_IMGUI_UI_RENDERER_HPP

#include "ui/i_ui_renderer.hpp"

#include <vector>
#include <memory>

struct GLFWwindow;

namespace spy::ui {

class ImGuiUIRenderer : public IUIRenderer {
public:
    ImGuiUIRenderer();
    ~ImGuiUIRenderer() override;

    bool initialize(const UIRendererConfig& config) override;
    void register_widget(std::shared_ptr<IWidget> widget) override;
    [[nodiscard]] bool should_close() const noexcept override;
    void begin_frame() override;
    void render_widgets() override;
    void end_frame() override;
    void shutdown() override;

private:
    void setup_theme();

    GLFWwindow* window_{nullptr};
    UIRendererConfig config_{};
    std::vector<std::shared_ptr<IWidget>> widgets_{};
    bool initialized_{false};
};

} // namespace spy::ui

#endif // SPY_SOURCE_UI_IMGUI_UI_RENDERER_HPP

