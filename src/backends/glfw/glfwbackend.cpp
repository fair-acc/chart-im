#include "glfwbackend.h"

#include <GLFW/glfw3.h>
#ifdef X11_ENABLED
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>

#include <fmt/format.h>

#include "backends/imgui_impl_glfw.h"

#include "renderers/renderer.h"
#include "window.h"

namespace ImChart::Backend {

GLFWBackend *GLFWBackend::create() {
    if (!glfwInit()) {
        fmt::print(stderr, "Failed to initialize GLFW.\n");
        return nullptr;
    }

    glfwWindowHint(GLFW_VISIBLE, false);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    return new GLFWBackend;
}

void *GLFWBackend::nativeDisplay() {
#ifdef X11_ENABLED
    glfwGetX11Display();
#endif
    return nullptr;
}

std::unique_ptr<Window> GLFWBackend::createWindow(ImChart::Window *window, int width, int height) {
    auto win = glfwCreateWindow(width, height, "", nullptr, nullptr);
    if (!win) {
        return nullptr;
    }

    glfwSetCursorPosCallback(win, [](GLFWwindow *, double x, double y) {
        fmt::print("mouse {} {}\n", x, y);
    });

    auto w     = std::make_unique<GLFWWindow>();
    w->m_win   = win;

    w->m_imgui = ImGui::CreateContext();
    ImGui::SetCurrentContext(w->m_imgui);

    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    ImGui_ImplGlfw_InitForOther(win, true);

    return w;
}

void GLFWBackend::scheduleRender(ImChart::Window *window) {
    m_windowsToRender.push_back(window);
    glfwPostEmptyEvent();
}

void GLFWBackend::run() {
    bool quit = false;
    while (!quit) {
        glfwWaitEvents();

        if (!m_windowsToRender.empty()) {
            Renderer::instance().begin();
            for (auto *w : m_windowsToRender) {
                auto gw = static_cast<GLFWWindow *>(&w->backendWindow());
                ImGui::SetCurrentContext(gw->m_imgui);
                if (w->surface().newFrame()) {
                    ImGui_ImplGlfw_NewFrame();
                    ImGui::NewFrame();

                    w->onRender();

                    ImGui::Render();
                    w->surface().present();
                }
            }
            Renderer::instance().end();
        }
    }

    glfwTerminate();
}

GLFWWindow::~GLFWWindow() {
    glfwDestroyWindow(m_win);
}

void *GLFWWindow::nativeWindow() {
#ifdef X11_ENABLED
    return (void *) glfwGetX11Window(m_win);
#endif
    return nullptr;
}

void GLFWWindow::setSize(int width, int height) {
    glfwSetWindowSize(m_win, width, height);
}

void GLFWWindow::show() {
    glfwShowWindow(m_win);
}

Size GLFWWindow::pixelSize() const {
    int w, h;
    glfwGetFramebufferSize(m_win, &w, &h);
    return { w, h };
}

} // namespace ImChart::Backend
