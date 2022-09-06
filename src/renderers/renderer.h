#pragma once

#include <memory>

namespace ImChart {

namespace Backend {
class Window;
}

namespace Renderer {

class Surface;

class Renderer {
public:
    virtual ~Renderer()                                                     = default;

    virtual std::unique_ptr<Surface> createSurface(Backend::Window *window) = 0;

    virtual void                     begin()                                = 0;
    virtual void                     end()                                  = 0;
};

class Surface {
public:
    virtual ~Surface()      = default;

    virtual bool newFrame() = 0;
    virtual void present()  = 0;
};

bool      create();
Renderer &instance();

} // namespace Renderer

} // namespace ImChart
