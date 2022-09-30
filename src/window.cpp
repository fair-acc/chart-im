#include "window.h"

#include <fmt/format.h>

#include "backends/backend.h"
#include "renderers/renderer.h"

namespace ImChart {

Window::Window(int width, int height) {
    m_window = Backend::instance().createWindow(this, width, height);
}

Window::~Window() {
}

void Window::setSize(int width, int height) {
    m_window->setSize(width, height);
}

Size Window::pixelSize() const {
    return m_window->pixelSize();
}

void Window::show() {
    if (!m_surface) {
        m_surface = Renderer::instance().createSurface(m_window.get());
    }
    m_window->show();
}

void Window::scheduleRender() {
    if (!m_renderPending) {
        Backend::instance().scheduleRender(this);
        m_renderPending = true;
    }
}

void Window::render() {
    m_renderPending = false;
    onRender();
}

} // namespace ImChart
