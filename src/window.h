#pragma once

#include <functional>
#include <memory>

#include "utils.h"

namespace ImChart {

namespace Backend {
class Window;
}

namespace Renderer {
class Surface;
}

class Window {
public:
    explicit Window(int width = 100, int height = 100);
    ~Window();

    void                      setSize(int width, int height);
    void                      show();

    Size                      pixelSize() const;

    void                      scheduleRender();
    void                      render();
    std::function<void()>     onRender;

    inline Backend::Window   &backendWindow() const { return *m_window; }
    inline Renderer::Surface &surface() const { return *m_surface; }

private:
    std::unique_ptr<Backend::Window>   m_window;
    std::unique_ptr<Renderer::Surface> m_surface;

    std::function<void()>              m_updateCallback;
    bool                               m_renderPending = false;
};

} // namespace ImChart
