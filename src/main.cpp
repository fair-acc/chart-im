#include <fmt/format.h>

#include <imgui.h>
#include <imgui_node_editor.h>
#include <implot.h>

#include "backends/backend.h"
#include "flowgraph.h"
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

void ImGuiEx_BeginColumn() {
    ImGui::BeginGroup();
}

void ImGuiEx_NextColumn() {
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
}

void ImGuiEx_EndColumn() {
    ImGui::EndGroup();
}

struct LinkInfo {
    ax::NodeEditor::LinkId Id;
    ax::NodeEditor::PinId  InputId;
    ax::NodeEditor::PinId  OutputId;
};

int main(int argc, char **argv) {
    if (!init()) {
        return 1;
    }

    FlowGraph flowGraph;
    flowGraph.loadBlockDefinitions(BLOCKS_DIR);
    flowGraph.parse(SRC_DIR "/test.grc");

    SinDataSet dataset;

    Window     win(1000, 1000);

    dataset.onDataChanged = [&](int, int) {
        win.scheduleRender();
    };

    auto ed = ax::NodeEditor::CreateEditor();
    ax::NodeEditor::SetCurrentEditor(ed);

    std::vector<LinkInfo> links;
    int                   id = 10000;
    for (const auto &b : flowGraph.blocks()) {
        for (const auto &port : b->outputs()) {
            for (auto &conn : port.connections) {
                auto inPort = conn.block->inputs()[conn.portNumber];
                links.push_back({ ax::NodeEditor::LinkId(id++), port.id, inPort.id });
            }
        }
    }

    win.onRender = [&]() {
        ImGui::SetNextWindowPos({ 0, 0 });
        const auto size = win.pixelSize();
        ImGui::SetNextWindowSize({ float(size.width), float(size.height) });
        ImGui::Begin("Main Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);
        if (ImPlot::BeginPlot("My Plot")) {
            // ImPlot::SetupAxis(ImAxis_X1, "My X-Axis", ImPlotAxisFlags_LogScale);
            ImPlot::PlotLine("My Line Plot", dataset.getValues(0).data(), dataset.getValues(1).data(), dataset.getDataCount());

            // ImPlot::PlotHeatmap("Heightmap", dataset.getValues(2).data() ,2000,2000, -1, 1, nullptr);

            ImPlot::EndPlot();
        }
        ImGui::End();

        // ImGui::ShowDemoWindow();

        // ImPlot::ShowDemoWindow();

        ax::NodeEditor::Begin("My Editor", ImVec2(0.0, 0.0f));

        for (const auto &b : flowGraph.blocks()) {
            ax::NodeEditor::BeginNode(b->id);
            ImGui::TextUnformatted(b->name.c_str());

            if (!b->type) {
                ImGui::TextUnformatted("Unkown type");
            } else {
                ImGuiEx_BeginColumn();
                const auto &inputs = b->type->inputs;
                for (std::size_t i = 0; i < inputs.size(); ++i) {
                    ax::NodeEditor::BeginPin(b->inputs()[i].id, ax::NodeEditor::PinKind::Input);
                    ImGui::Text("-> In1 (%s)", inputs[i].type.c_str());
                    ax::NodeEditor::EndPin();
                }
                ImGuiEx_NextColumn();

                int         x       = ImGui::GetCursorPosX();
                const auto &outputs = b->type->outputs;
                for (std::size_t i = 0; i < outputs.size(); ++i) {
                    ImGui::SetCursorPosX(x + 100);
                    ax::NodeEditor::BeginPin(b->outputs()[i].id, ax::NodeEditor::PinKind::Output);
                    ImGui::Text("Out (%s) ->", outputs[i].type.c_str());
                    ax::NodeEditor::EndPin();
                }
                ImGuiEx_EndColumn();
            }

            ImGui::Dummy(ImVec2(80.0f, 45.0f));

            ax::NodeEditor::EndNode();
        }

        for (auto &linkInfo : links) {
            ax::NodeEditor::Link(linkInfo.Id, linkInfo.InputId, linkInfo.OutputId);
        }

        // Handle creation action, returns true if editor want to create new object (node or link)
        if (ax::NodeEditor::BeginCreate()) {
            ax::NodeEditor::PinId inputPinId, outputPinId;
            if (ax::NodeEditor::QueryNewLink(&inputPinId, &outputPinId)) {
                // QueryNewLink returns true if editor want to create new link between pins.
                //
                // Link can be created only for two valid pins, it is up to you to
                // validate if connection make sense. Editor is happy to make any.
                //
                // Link always goes from input to output. User may choose to drag
                // link from output pin or input pin. This determine which pin ids
                // are valid and which are not:
                //   * input valid, output invalid - user started to drag new ling from input pin
                //   * input invalid, output valid - user started to drag new ling from output pin
                //   * input valid, output valid   - user dragged link over other pin, can be validated

                if (inputPinId && outputPinId) // both are valid, let's accept link
                {
                    // ed::AcceptNewItem() return true when user release mouse button.
                    if (ax::NodeEditor::AcceptNewItem()) {
                        // Since we accepted new link, lets add one to our list of links.
                        links.push_back({ ax::NodeEditor::LinkId(id++), inputPinId, outputPinId });

                        // Draw new link.
                        ax::NodeEditor::Link(links.back().Id, links.back().InputId, links.back().OutputId);
                    }

                    // You may choose to reject connection between these nodes
                    // by calling ed::RejectNewItem(). This will allow editor to give
                    // visual feedback by changing link thickness and color.
                }
            }
        }
        ax::NodeEditor::EndCreate(); // Wraps up object creation action handling.

        ax::NodeEditor::End();
    };
    win.show();

    Backend::instance().run();
}
