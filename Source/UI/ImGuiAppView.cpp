#include "ImGuiAppView.h"
#include <imgui.h>
#include <algorithm>

namespace jv1010 {

ImGuiAppView::ImGuiAppView(JV1010EditorAudioProcessor& processor)
    : processor_(processor),
      topBar_(processor),
      browser_(processor),
      perfEditor_(processor),
      patchEditor_(processor),
      effectsEditor_(processor),
      drumsEditor_(processor),
      systemEditor_(processor),
      footerDock_(processor) {

    compareBackup_ = processor_.getSynthState().getPatchModeTempPatch();

    topBar_.onCompareToggled = [this]() {
        auto cur = processor_.getSynthState().getPatchModeTempPatch();
        processor_.getSynthState().setPatchModeTempPatch(compareBackup_);
        processor_.getMidiManager().sendSysExQueue(
            processor_.getSynthState().serializePatchToDT1(
                processor_.getDeviceId(), addr::kPatchModeTempPatchBase, compareBackup_));
        compareBackup_ = cur;
        showingCompareBackup_ = !showingCompareBackup_;
        topBar_.setStatusText(showingCompareBackup_ ? "Compare A/B: Snapshot Original" : "Compare A/B: Edicion Actual");
    };
}

void ImGuiAppView::render() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 6.0f));
    if (ImGui::Begin("##MainJV1010", nullptr, flags)) {
        topBar_.render();
        ImGui::Separator();

        bool showKbd = topBar_.isKeyboardVisible();
        bool showTools = topBar_.isToolsVisible();
        float footerH = (showTools ? 34.0f : 0.0f) + (showKbd ? 106.0f : 0.0f);
        float bodyH = std::max(120.0f, ImGui::GetContentRegionAvail().y - footerH - (footerH > 0.0f ? 6.0f : 0.0f));

        browser_.render(255.0f, bodyH, footerDock_.getActiveMidiChannelZeroBased(),
                        [this](const std::string& st) { topBar_.setStatusText(st); },
                        [this](int tabIdx) {
                            if (tabIdx == 0) patchEditor_.setTargetPart(-1);
                            requestedEditorTab_ = tabIdx;
                        });
        ImGui::SameLine();

        ImGui::BeginChild("##EditorTabsPane", ImVec2(0.0f, bodyH), true);
        if (ImGui::BeginTabBar("##MainEditorTabs")) {
            auto tabFlags = [&](int idx) -> ImGuiTabItemFlags {
                if (requestedEditorTab_ == idx) {
                    requestedEditorTab_ = -1;
                    return ImGuiTabItemFlags_SetSelected;
                }
                return 0;
            };
            if (ImGui::BeginTabItem("Patch Editor", nullptr, tabFlags(0))) {
                patchEditor_.render();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Patch Effects", nullptr, tabFlags(1))) {
                effectsEditor_.render();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Drums Editor", nullptr, tabFlags(2))) {
                drumsEditor_.render();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Performance Editor", nullptr, tabFlags(3))) {
                perfEditor_.render([this](int partIdx) {
                    patchEditor_.setTargetPart(partIdx);
                    requestedEditorTab_ = 0;
                });
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("System Editor", nullptr, tabFlags(4))) {
                systemEditor_.render();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Manual", nullptr, tabFlags(5))) {
                manualView_.render();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();

        if (footerH > 0.0f) {
            footerDock_.render(ImGui::GetContentRegionAvail().x, showKbd, showTools,
                               [this](const std::string& st) { topBar_.setStatusText(st); });
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace jv1010
