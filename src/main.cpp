#include <fmt/format.h>

#include <imgui.h>

#include "backends/backend.h"
#include "renderers/renderer.h"
#include "window.h"

using namespace ImChart;

bool init() {
    IMGUI_CHECKVERSION();

    if (!Backend::create()) {
        return false;
    }
    if (!Renderer::create()) {
        return false;
    }
    return true;
}

int main(int argc, char **argv) {
    if (!init()) {
        return 1;
    }

    Window win;
    win.onRender = []() {
        ImGui::ShowDemoWindow();
    };
    win.show();

    Backend::instance().run();
}
