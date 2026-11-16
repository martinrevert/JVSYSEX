#include "TopControlBar.h"
#include "../ImGuiWidgets.h"
#include "../../Database/SrJv80CatalogDB.h"
#include <imgui.h>

namespace jv1010 {

TopControlBar::TopControlBar(JV1010EditorAudioProcessor& processor)
    : processor_(processor) {
    midiThru_ = processor_.getMidiManager().isMidiThruEnabled();
}

void TopControlBar::render() {
    auto& midi = processor_.getMidiManager();

    // Row 1: MIDI In, MIDI Out, Routing, SysEx Queue / Status
    ImGui::SetNextItemWidth(165.0f);
    auto curIn = midi.getCurrentInputName();
    if (ImGui::BeginCombo("##MidiIn", curIn.isEmpty() ? "MIDI In: (None)" : curIn.toRawUTF8())) {
        if (ImGui::Selectable("(None)", curIn.isEmpty())) {
            juce::MessageManager::callAsync([&midi]() { midi.closeInputDevice(); });
        }
        for (const auto& name : midi.getAvailableInputNames()) {
            if (ImGui::Selectable(name.toRawUTF8(), name == curIn)) {
                juce::MessageManager::callAsync([this, &midi, name]() {
                    midi.openInputDevice(name);
                    if (midi.isInputOpen() && midi.isOutputOpen()) {
                        processor_.requestTemporaryStateFromHardware();
                    }
                });
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(165.0f);
    auto curOut = midi.getCurrentOutputName();
    if (ImGui::BeginCombo("##MidiOut", curOut.isEmpty() ? "MIDI Out: (None)" : curOut.toRawUTF8())) {
        if (ImGui::Selectable("(None)", curOut.isEmpty())) {
            juce::MessageManager::callAsync([&midi]() { midi.closeOutputDevice(); });
        }
        for (const auto& name : midi.getAvailableOutputNames()) {
            if (ImGui::Selectable(name.toRawUTF8(), name == curOut)) {
                juce::MessageManager::callAsync([this, &midi, name]() {
                    midi.openOutputDevice(name);
                    if (midi.isInputOpen() && midi.isOutputOpen()) {
                        processor_.requestTemporaryStateFromHardware();
                    }
                });
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(125.0f);
    const char* routeNames[] = { "Route: Direct HW", "Route: DAW Bus", "Route: HW + DAW" };
    int routeIdx = static_cast<int>(midi.getRoutingMode());
    if (ImGui::Combo("##Routing", &routeIdx, routeNames, 3)) {
        midi.setRoutingMode(static_cast<MidiRoutingMode>(routeIdx));
    }

    size_t pending = midi.getPendingSysExCount();
    ImGui::SameLine();
    if (pending > 0) {
        ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "SysEx Queue: %zu...", pending);
    } else {
        ImGui::TextColored(ImVec4(0.06f, 0.73f, 0.51f, 1.0f), "%s", statusText_.c_str());
    }

    // Row 2: Import, Export, Compare A/B, Configurable Toggles
    if (ImGui::Button("Import .syx/.mid")) {
        triggerImportAsync();
    }
    ImGui::SameLine();
    if (ImGui::Button("Export .syx/.mid")) {
        ImGui::OpenPopup("##ExportPopup");
    }
    if (ImGui::BeginPopup("##ExportPopup")) {
        if (ImGui::MenuItem("Temp Patch (.syx)")) triggerExportAsync(ExportScope::TempPatch, false);
        if (ImGui::MenuItem("Temp Patch (.mid)")) triggerExportAsync(ExportScope::TempPatch, true);
        if (ImGui::MenuItem("Temp Performance (.syx)")) triggerExportAsync(ExportScope::TempPerformance, false);
        if (ImGui::MenuItem("Temp Performance (.mid)")) triggerExportAsync(ExportScope::TempPerformance, true);
        ImGui::Separator();
        if (ImGui::MenuItem("User Patches 1-128 (.syx)")) triggerExportAsync(ExportScope::UserPatches128, false);
        if (ImGui::MenuItem("User Performances 1-32 (.syx)")) triggerExportAsync(ExportScope::UserPerformances32, false);
        if (ImGui::MenuItem("Full User Memory Backup (.syx)")) triggerExportAsync(ExportScope::FullUserMemory, false);
        if (ImGui::MenuItem("Full User Memory Backup (.mid)")) triggerExportAsync(ExportScope::FullUserMemory, true);
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ui::RolandButton("Compare A/B", false, ImVec2(0.0f, 26.0f), 0)) {
        if (onCompareToggled) onCompareToggled();
    }

    ImGui::SameLine();
    ui::ToggleSwitch("Keyboard", &showKeyboard_);
    ImGui::SameLine();
    if (ui::ToggleSwitch("MIDI Thru", &midiThru_)) {
        midi.setMidiThruEnabled(midiThru_);
        statusText_ = midiThru_ ? "MIDI Thru ACTIVADO" : "MIDI Thru DESACTIVADO";
    }
    ImGui::SameLine();
    ui::ToggleSwitch("Morph/Rnd", &showTools_);
}

void TopControlBar::triggerImportAsync() {
    juce::MessageManager::callAsync([this]() {
        fileChooser_ = std::make_unique<juce::FileChooser>(
            "Importar SysEx / MIDI (JV-1010 / JV-1080 / JV-2080 / XP)", juce::File{}, "*.syx;*.mid");
        fileChooser_->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this](const juce::FileChooser& fc) {
            auto f = fc.getResult();
            if (!f.existsAsFile()) return;
            auto res = SyxMidFileIO::importFile(f, processor_.getSynthState(), ImportTargetMode::PreserveAddresses, 0, processor_.getDeviceId());
            if (res.success && !res.hardwarePackets.empty()) {
                processor_.getMidiManager().sendSysExQueue(res.hardwarePackets);
                statusText_ = res.summary + " -> Enviando al JV-1010...";
            } else {
                statusText_ = res.summary;
            }
        });
    });
}

void TopControlBar::triggerExportAsync(ExportScope scope, bool isMid) {
    juce::MessageManager::callAsync([this, scope, isMid]() {
        fileChooser_ = std::make_unique<juce::FileChooser>(
            "Exportar Memoria JV-1010", juce::File{}, isMid ? "*.mid" : "*.syx");
        fileChooser_->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this, scope, isMid](const juce::FileChooser& fc) {
            auto f = fc.getResult();
            if (f == juce::File{}) return;
            bool ok = isMid ? SyxMidFileIO::exportToMid(f, processor_.getSynthState(), scope, processor_.getDeviceId())
                            : SyxMidFileIO::exportToSyx(f, processor_.getSynthState(), scope, processor_.getDeviceId());
            statusText_ = ok ? ("Exportado: " + f.getFileName().toStdString()) : "Error al exportar.";
        });
    });
}

} // namespace jv1010
