#include "renderer.h"

#include "opengl/openglrenderer.h"

namespace ImChart::Renderer {

static Renderer *g_renderer = nullptr;

bool             create() {
                if (g_renderer) {
                    return false;
    }
                g_renderer = OpenGLRenderer::create();
                return g_renderer;
}

Renderer &instance() {
    return *g_renderer;
}

} // namespace ImChart::Renderer
