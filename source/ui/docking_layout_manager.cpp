#include "ui/docking_layout_manager.hpp"

#include <imgui.h>
#include <string>

namespace spy::ui {

struct DockingLayoutManager::Impl {
    std::string title{"Dockspace Manager"};
    bool visible{true};
};

DockingLayoutManager::DockingLayoutManager()
    : impl_(std::make_unique<Impl>()) {}

DockingLayoutManager::~DockingLayoutManager() = default;

DockingLayoutManager::DockingLayoutManager(DockingLayoutManager&&) noexcept = default;
DockingLayoutManager& DockingLayoutManager::operator=(DockingLayoutManager&&) noexcept = default;

void DockingLayoutManager::render() {
    if (!impl_->visible) {
        return;
    }

    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("MainDockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("SignalSpyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    ImGui::End();
}

std::string_view DockingLayoutManager::title() const noexcept {
    return impl_->title;
}

bool DockingLayoutManager::is_visible() const noexcept {
    return impl_->visible;
}

void DockingLayoutManager::set_visible(bool visible) noexcept {
    impl_->visible = visible;
}

} // namespace spy::ui
