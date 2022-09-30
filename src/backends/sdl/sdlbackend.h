#pragma once

#include <mutex>
#include <vector>

#include "../backend.h"

struct SDL_Window;
struct ImGuiContext;
struct ImPlotContext;

namespace ImChart {

namespace Backend {

class SDLBackend : public Backend {
public:
    static SDLBackend      *create();

    void                    run() final;

    void                   *nativeDisplay() final;

    std::unique_ptr<Window> createWindow(ImChart::Window *window, int w, int h) final;

    void                    scheduleRender(ImChart::Window *window) final;

    void                    startTimer(Timer *t) final;

private:
    bool                           iterate();
    std::vector<ImChart::Window *> m_windowsToRender;
};

class SDLWindow : public Window {
public:
    ~SDLWindow();

    void          *nativeWindow() final;

    void           setSize(int width, int height) override;
    void           show() override;

    Size           pixelSize() const override;

    SDL_Window    *m_win;
    ImGuiContext  *m_imgui;
    ImPlotContext *m_implot;
};

}

} // namespace ImChart::Backend
