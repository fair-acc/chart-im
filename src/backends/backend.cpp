#include "backend.h"

#include "glfw/glfwbackend.h"
#include "sdl/sdlbackend.h"

namespace ImChart::Backend {

static Backend *g_backend = nullptr;

bool            create() {
               if (g_backend) {
                   return false;
    }
#ifdef OPENGL_ENABLED
    g_backend = SDLBackend::create();
#endif
    return g_backend;
}

Backend &instance() {
    return *g_backend;
}

} // namespace ImChart::Backend
