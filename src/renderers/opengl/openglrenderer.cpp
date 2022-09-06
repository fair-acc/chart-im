#include "openglrenderer.h"

#include <fmt/format.h>

#include <GL/gl.h>

#include <backends/imgui_impl_opengl3.h>

#include "backends/backend.h"

namespace ImChart::Renderer {

OpenGLRenderer *OpenGLRenderer::create() {
    auto nativeDisplay = Backend::instance().nativeDisplay();

    auto eglDisplay    = eglGetDisplay((EGLNativeDisplayType) nativeDisplay);
    if (eglDisplay == EGL_NO_DISPLAY) {
        fmt::print(stderr, "Unable to get the EGL display.\n");
        return nullptr;
    }

    int major, minor;
    if (!eglInitialize(eglDisplay, &major, &minor)) {
        fmt::print(stderr, "Unable to initialize EGL\n");
        return nullptr;
    }

    fmt::print("EGL version {}.{}\n", major, minor);

    EGLint attr[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    EGLConfig config;
    EGLint    numConfigs;
    if (!eglChooseConfig(eglDisplay, attr, &config, 1, &numConfigs)) {
        fmt::print(stderr, "Failed to choose config (egl error: {}).\n", eglGetError());
        return nullptr;
    }

    if (numConfigs == 0) {
        fmt::print(stderr, "No valid EGL config found.\n");
        return nullptr;
    }

    EGLint ctxattr[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    auto context = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, ctxattr);
    if (context == EGL_NO_CONTEXT) {
        fmt::print(stderr, "Unable to create EGL context (eglError: {})\n", eglGetError());
        return nullptr;
    }

    auto renderer       = new OpenGLRenderer;
    renderer->m_display = eglDisplay;
    renderer->m_config  = config;
    renderer->m_context = context;
    return renderer;
}

std::unique_ptr<Surface> OpenGLRenderer::createSurface(Backend::Window *window) {
    auto native  = (EGLNativeWindowType) window->nativeWindow();
    auto surface = eglCreateWindowSurface(m_display, m_config, native, nullptr);
    if (surface == EGL_NO_SURFACE) {
        fmt::print(stderr, "Unable to create EGL surface (eglError: {})\n", eglGetError());
        return nullptr;
    }

    if (!eglMakeCurrent(m_display, surface, surface, m_context)) {
        fmt::print("eglMakeCurrent failed.\n");
        return nullptr;
    }

    auto s       = std::make_unique<OpenGLSurface>();
    s->m_window  = window;
    s->m_surface = surface;

    ImGui_ImplOpenGL3_Init();

    return s;
}

void OpenGLRenderer::begin() {
}

void OpenGLRenderer::end() {
}

bool OpenGLSurface::newFrame() {
    auto r   = static_cast<OpenGLRenderer *>(&instance());
    auto ret = eglMakeCurrent(r->m_display, m_surface, m_surface, r->m_context);
    if (!ret) {
        fmt::print("eglMakeCurrent failed.\n");
        return false;
    }

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    const auto s = m_window->pixelSize();
    glViewport(0, 0, s.width, s.height);

    ImGui_ImplOpenGL3_NewFrame();
    return true;
}

void OpenGLSurface::present() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    auto r = static_cast<OpenGLRenderer *>(&instance());
    eglSwapBuffers(r->m_display, m_surface);
}

} // namespace ImChart::Renderer
