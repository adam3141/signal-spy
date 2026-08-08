#include "imgui_ui_renderer.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <iostream>

namespace spy::ui {

ImGuiUIRenderer::ImGuiUIRenderer() = default;

ImGuiUIRenderer::~ImGuiUIRenderer() {
    Shutdown();
}

bool ImGuiUIRenderer::Initialize(const UIRendererConfig& config) {
    if (_initialized) {
        return true;
    }

    _config = config;

    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "[GLFW Error " << error << "]: " << description << std::endl;
    });

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    if (_config.headless) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }

    _window = glfwCreateWindow(_config.window_width, _config.window_height, _config.window_title.c_str(), nullptr, nullptr);
    if (!_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(_window);
    glfwSwapInterval(_config.vsync ? 1 : 0);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    setup_theme();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    _initialized = true;
    return true;
}

void ImGuiUIRenderer::RegisterWidget(std::shared_ptr<IWidget> widget) {
    if (widget) {
        _widgets.push_back(std::move(widget));
    }
}

bool ImGuiUIRenderer::ShouldClose() const noexcept {
    if (!_window) {
        return true;
    }
    return glfwWindowShouldClose(_window) != 0;
}

void ImGuiUIRenderer::BeginFrame() {
    if (!_initialized) return;

    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiUIRenderer::RenderWidgets() {
    if (!_initialized) return;

    for (auto& widget : _widgets) {
        if (widget && widget->is_Visible()) {
            widget->Render();
        }
    }
}

void ImGuiUIRenderer::EndFrame() {
    if (!_initialized) return;

    ImGui::Render();
    int display_w = 0;
    int display_h = 0;
    glfwGetFramebufferSize(_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.10f, 0.11f, 0.13f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(_window);
}

void ImGuiUIRenderer::Shutdown() {
    if (!_initialized) return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (_window) {
        glfwDestroyWindow(_window);
        _window = nullptr;
    }

    glfwTerminate();
    _initialized = false;
}


void ImGuiUIRenderer::setup_theme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Modern Dark Palette
    colors[ImGuiCol_Text]                  = ImVec4(0.92f, 0.93f, 0.94f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.53f, 0.56f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.14f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_Border]                = ImVec4(0.22f, 0.24f, 0.28f, 1.00f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.18f, 0.20f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.24f, 0.27f, 0.32f, 1.00f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.28f, 0.32f, 0.38f, 1.00f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.14f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.14f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.24f, 0.27f, 0.32f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.30f, 0.34f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.36f, 0.40f, 0.48f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(0.38f, 0.65f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.38f, 0.65f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.48f, 0.73f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]                = ImVec4(0.20f, 0.23f, 0.28f, 1.00f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.28f, 0.33f, 0.40f, 1.00f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.35f, 0.42f, 0.52f, 1.00f);
    colors[ImGuiCol_Header]                = ImVec4(0.20f, 0.23f, 0.28f, 1.00f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.28f, 0.33f, 0.40f, 1.00f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.35f, 0.42f, 0.52f, 1.00f);
    colors[ImGuiCol_Separator]             = ImVec4(0.22f, 0.24f, 0.28f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.38f, 0.65f, 0.98f, 1.00f);
    colors[ImGuiCol_SeparatorActive]       = ImVec4(0.48f, 0.73f, 1.00f, 1.00f);
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.20f, 0.23f, 0.28f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.38f, 0.65f, 0.98f, 1.00f);
    colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.48f, 0.73f, 1.00f, 1.00f);
    colors[ImGuiCol_Tab]                   = ImVec4(0.14f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered]            = ImVec4(0.24f, 0.27f, 0.32f, 1.00f);
    colors[ImGuiCol_TabActive]             = ImVec4(0.20f, 0.23f, 0.28f, 1.00f);
    colors[ImGuiCol_TabUnfocused]          = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.16f, 0.18f, 0.21f, 1.00f);
    colors[ImGuiCol_DockingPreview]        = ImVec4(0.38f, 0.65f, 0.98f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);

    style.WindowRounding    = 6.0f;
    style.ChildRounding     = 4.0f;
    style.FrameRounding     = 4.0f;
    style.PopupRounding     = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding      = 4.0f;
    style.TabRounding       = 4.0f;
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.PopupBorderSize   = 1.0f;
}

std::unique_ptr<IUIRenderer> CreateUIRenderer() {
    return std::make_unique<ImGuiUIRenderer>();
}


} // namespace spy::ui
