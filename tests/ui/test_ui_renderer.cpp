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
        : title_(std::move(name)) {}

    void render() override {
        render_count_++;
    }

    [[nodiscard]] std::string_view title() const noexcept override {
        return title_;
    }

    [[nodiscard]] bool is_visible() const noexcept override {
        return visible_;
    }

    void set_visible(bool visible) noexcept override {
        visible_ = visible;
    }

    [[nodiscard]] size_t render_count() const noexcept {
        return render_count_;
    }

private:
    std::string title_;
    bool visible_{true};
    size_t render_count_{0};
};

TEST(UIRendererTest, MockWidgetLifecycle) {
    auto widget = std::make_shared<MockTestWidget>("Test Panel");
    EXPECT_EQ(widget->title(), "Test Panel");
    EXPECT_TRUE(widget->is_visible());
    EXPECT_EQ(widget->render_count(), 0);

    widget->render();
    EXPECT_EQ(widget->render_count(), 1);

    widget->set_visible(false);
    EXPECT_FALSE(widget->is_visible());
}

TEST(UIRendererTest, DockingLayoutManagerLifecycle) {
    DockingLayoutManager manager;
    EXPECT_EQ(manager.title(), "Dockspace Manager");
    EXPECT_TRUE(manager.is_visible());

    manager.set_visible(false);
    EXPECT_FALSE(manager.is_visible());
}

TEST(UIRendererTest, HeadlessRendererInitializationAndLoop) {
    auto renderer = create_ui_renderer();
    ASSERT_NE(renderer, nullptr);

    UIRendererConfig config;
    config.window_width = 800;
    config.window_height = 600;
    config.window_title = "Signal Spy Test Window";
    config.headless = true;

    bool init_ok = renderer->initialize(config);
    ASSERT_TRUE(init_ok);

    auto widget = std::make_shared<MockTestWidget>("Mock Widget");
    renderer->register_widget(widget);

    renderer->begin_frame();
    renderer->render_widgets();
    renderer->end_frame();

    EXPECT_EQ(widget->render_count(), 1);

    renderer->shutdown();
}

} // namespace spy::ui::testing
