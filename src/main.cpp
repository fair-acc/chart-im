#include <fmt/format.h>

#include <imgui.h>
#include <implot.h>

#include "backends/backend.h"
#include "renderers/renderer.h"
#include "sindataset.h"
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

    SinDataSet2D dataset;

    Window     win(1000, 1000);

    dataset.onDataChanged = [&](int, int) {
        win.scheduleRender();
    };

    win.onRender = [&]() {
        ImGui::SetNextWindowPos({ 0, 0 });
        const auto size = win.pixelSize();
        ImGui::SetNextWindowSize({ float(size.width), float(size.height) });
        ImGui::Begin("Main Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);
        if (ImPlot::BeginPlot("My Plot")) {
            // ImPlot::SetupAxis(ImAxis_X1, "My X-Axis", ImPlotAxisFlags_LogScale);
            // ImPlot::PlotLine("My Line Plot", dataset.getValues(0).data(), dataset.getValues(1).data(), dataset.getDataCount());

            ImPlot::PlotHeatmap("Heightmap", dataset.getValues(2).data() ,2000,2000, -1, 1, nullptr);

            ImPlot::EndPlot();
        }
        ImGui::End();

        ImGui::ShowDemoWindow();

        ImPlot::ShowDemoWindow();
    };
    win.show();

    Backend::instance().run();
}
