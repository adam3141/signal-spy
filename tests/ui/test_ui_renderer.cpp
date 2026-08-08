#include "ui/docking_layout_manager.hpp"
#include "ui/i_ui_renderer.hpp"
#include "ui/i_widget.hpp"
#include "ui/ui_types.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <string>

namespace spy::ui::testing {

class MockTestWidget : public IWidget {
public:
    explicit MockTestWidget(std::string name)
        : _title(std::move(name)) {}

    void Render() override {
        _render_count++;
    }

    [[nodiscard]] std::string_view get_Title() const noexcept override {
        return _title;
    }

    [[nodiscard]] bool is_Visible() const noexcept override {
        return _visible;
    }

    void set_Visible(bool visible) noexcept override {
        _visible = visible;
    }

    [[nodiscard]] size_t get_Render_Count() const noexcept {
        return _render_count;
    }

private:
    std::string _title;
    bool _visible{true};
    size_t _render_count{0};
};

TEST(UIRendererTest, MockWidgetLifecycle) {
    auto widget = std::make_shared<MockTestWidget>("Test Panel");
    EXPECT_EQ(widget->get_Title(), "Test Panel");
    EXPECT_TRUE(widget->is_Visible());
    EXPECT_EQ(widget->get_Render_Count(), 0);

    widget->Render();
    EXPECT_EQ(widget->get_Render_Count(), 1);

    widget->set_Visible(false);
    EXPECT_FALSE(widget->is_Visible());
}

TEST(UIRendererTest, DockingLayoutManagerLifecycle) {
    DockingLayoutManager manager;
    EXPECT_EQ(manager.get_Title(), "Dockspace Manager");
    EXPECT_TRUE(manager.is_Visible());

    manager.set_Visible(false);
    EXPECT_FALSE(manager.is_Visible());
}

TEST(UIRendererTest, HeadlessRendererInitializationAndLoop) {
    auto renderer = create_ui_renderer();
    ASSERT_NE(renderer, nullptr);

    UIRendererConfig config;
    config.window_width = 800;
    config.window_height = 600;
    config.window_title = "Signal Spy Test Window";
    config.headless = true;

    bool init_ok = renderer->Initialize(config);
    ASSERT_TRUE(init_ok);

    auto widget = std::make_shared<MockTestWidget>("Mock Widget");
    renderer->Register_Widget(widget);

    renderer->Begin_Frame();
    renderer->Render_Widgets();
    renderer->End_Frame();

    EXPECT_EQ(widget->get_Render_Count(), 1);

    renderer->Shutdown();
}


} // namespace spy::ui::testing
