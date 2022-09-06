#pragma once

#include <vector>

#include "../backend.h"

struct GLFWwindow;
struct ImGuiContext;

namespace ImChart {

namespace Backend {

class GLFWBackend : public Backend {
public:
    static GLFWBackend     *create();

    void                    run() final;

    void                   *nativeDisplay() final;

    std::unique_ptr<Window> createWindow(ImChart::Window *window, int w, int h) final;

    void                    scheduleRender(ImChart::Window *window) final;

private:
    std::vector<ImChart::Window *> m_windowsToRender;
};

class GLFWWindow : public Window {
public:
    ~GLFWWindow();

    void         *nativeWindow() final;

    void          setSize(int width, int height) override;
    void          show() override;

    Size          pixelSize() const override;

    GLFWwindow   *m_win;
    ImGuiContext *m_imgui;
};

}

} // namespace ImChart::Backend
