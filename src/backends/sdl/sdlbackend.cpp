#include "sdlbackend.h"

#include <thread>

#include <SDL.h>

#ifndef EMSCRIPTEN

#include <SDL_syswm.h>

#else
#include <emscripten.h>
#endif

#include <fmt/format.h>

#include "backends/imgui_impl_sdl.h"
#include <implot.h>

#include "renderers/renderer.h"
#include "timer.h"
#include "window.h"

namespace ImChart::Backend {

SDLBackend *SDLBackend::create() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fmt::print(stderr, "Failed to initialize SDL.\n");
        return nullptr;
    }

    // glfwWindowHint(GLFW_VISIBLE, false);
    // glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    return new SDLBackend;
}

void *SDLBackend::nativeDisplay() {
#ifndef EMSCRIPTEN
#ifdef X11_ENABLED

    auto          win = SDL_CreateWindow("", 0, 0, 1, 1, SDL_WINDOW_HIDDEN);

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(win, &wmInfo);
    return wmInfo.info.x11.display;

#endif
#endif
    return nullptr;
}

std::unique_ptr<Window> SDLBackend::createWindow(ImChart::Window *window, int width, int height) {
    auto win = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_HIDDEN);
    if (!win) {
        return nullptr;
    }

    auto w   = std::make_unique<SDLWindow>();
    w->m_win = win;
    SDL_SetWindowData(win, "window", window);

    w->m_imgui  = ImGui::CreateContext();
    w->m_implot = ImPlot::CreateContext();
    ImGui::SetCurrentContext(w->m_imgui);
    ImPlot::SetCurrentContext(w->m_implot);

    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    ImGui_ImplSDL2_InitForSDLRenderer(win, nullptr);

    return w;
}

static constexpr int TIMER_EVENT = SDL_USEREVENT + 1;

void                 SDLBackend::scheduleRender(ImChart::Window *window) {
                    m_windowsToRender.push_back(window);
                    SDL_Event e;
                    e.type = SDL_USEREVENT;
                    SDL_PushEvent(&e);
}

#if EMSCRIPTEN
static void timeout(void *p)
#else
static unsigned int timeout(unsigned int, void *p)
#endif
{
    auto t = static_cast<Timer *>(p);
    SDL_Event e;
    e.type       = TIMER_EVENT;
    e.user.data1 = p;
    SDL_PushEvent(&e);

#if EMSCRIPTEN
    emscripten_async_call(timeout, t, t->interval.count());
#else
    return t->interval.count();
#endif
}

void SDLBackend::startTimer(Timer *timer) {
#if EMSCRIPTEN
    // SDL_AddTimer does not works correctly on emscripten:
    // https://github.com/emscripten-core/emscripten/issues/17941
    emscripten_async_call(timeout, timer, timer->interval.count());
#else
    int id = SDL_AddTimer(timer->interval.count(), timeout, timer);
#endif
}

bool SDLBackend::iterate() {
    auto processEvent = [this](const SDL_Event &event) {
        if (event.type == SDL_QUIT) {
            return false;
        } else if (event.type == TIMER_EVENT) {
            auto t = static_cast<Timer *>(event.user.data1);
            t->onTimeout();
            return true;
        }

        if (ImGui_ImplSDL2_ProcessEvent(&event)) {
            const uint32_t wid = [&]() -> uint32_t {
                switch (event.type) {
                case SDL_MOUSEMOTION: return event.motion.windowID;
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEBUTTONDOWN: return event.button.windowID;
                case SDL_MOUSEWHEEL: return event.wheel.windowID;
                case SDL_KEYUP:
                case SDL_KEYDOWN: return event.key.windowID;
                default:
                    break;
                }
                return -1;
            }();
            if (wid != -1) {
                static_cast<ImChart::Window *>(SDL_GetWindowData(SDL_GetWindowFromID(wid), "window"))->scheduleRender();
            }
        }
        return true;
    };

#ifndef EMSCRIPTEN
    {
        SDL_Event event;
        if (SDL_WaitEvent(&event)) {
            if (!processEvent(event)) {
                return false;
            }
        }
    }
#endif

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (!processEvent(event)) {
            return false;
        }
    }

    if (!m_windowsToRender.empty()) {
        auto wins = std::move(m_windowsToRender);
        Renderer::instance().begin();
        for (auto *w : wins) {
            auto gw = static_cast<SDLWindow *>(&w->backendWindow());
            ImGui::SetCurrentContext(gw->m_imgui);
            ImPlot::SetCurrentContext(gw->m_implot);
            if (w->surface().newFrame()) {
                ImGui_ImplSDL2_NewFrame();
                ImGui::NewFrame();

                w->render();

                ImGui::Render();
                w->surface().present();
            }
        }
        Renderer::instance().end();
    }
    return true;
}
void SDLBackend::run() {
#ifdef EMSCRIPTEN
    emscripten_set_main_loop_arg([](void *a) {
        static_cast<SDLBackend *>(a)->iterate();
    },
            this, 0, true);
#else
    bool quit = false;
    while (!quit) {
        quit = !iterate();
    }
#endif

    SDL_Quit();
}

SDLWindow::~SDLWindow() {
    SDL_DestroyWindow(m_win);
}

void *SDLWindow::nativeWindow() {
#ifndef EMSCRIPTEN
#ifdef X11_ENABLED

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(m_win, &wmInfo);
    return (void *) wmInfo.info.x11.window;

#endif
#endif
    return nullptr;
}

void SDLWindow::setSize(int width, int height) {
    SDL_SetWindowSize(m_win, width, height);
}

void SDLWindow::show() {
    SDL_ShowWindow(m_win);
}

Size SDLWindow::pixelSize() const {
    int w, h;
    SDL_GetWindowSize(m_win, &w, &h);
    return { w, h };
}

} // namespace ImChart::Backend
