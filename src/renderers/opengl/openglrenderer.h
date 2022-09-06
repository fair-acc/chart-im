#pragma once

#include <EGL/egl.h>

#include "../renderer.h"

namespace ImChart {

namespace Renderer {

class OpenGLRenderer : public Renderer {
public:
    static OpenGLRenderer   *create();

    std::unique_ptr<Surface> createSurface(Backend::Window *window) override;

    void                     begin() override;
    void                     end() override;

    EGLDisplay               m_display;
    EGLConfig                m_config;
    EGLContext               m_context;
};

class OpenGLSurface : public Surface {
public:
    bool             newFrame() override;
    void             present() override;

    Backend::Window *m_window;
    EGLSurface       m_surface;
};

}

} // namespace ImChart::Renderer
