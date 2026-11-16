#include "VirtualKeyboardDock.h"
#include "../ImGuiWidgets.h"
#include "../../Librarian/PatchMorphRandom.h"
#include <imgui.h>
#include <algorithm>
#include <cmath>

namespace jv1010 {

VirtualKeyboardDock::VirtualKeyboardDock(JV1010EditorAudioProcessor& processor)
    : processor_(processor) {}

void VirtualKeyboardDock::render(float width, bool showKeyboard, bool showTools,
                                 const std::function<void(const std::string&)>& setStatus) {
    handleArpTick();

    ImGui::BeginChild("##FooterDock", ImVec2(width, 0.0f), true);

    if (showTools) {
        if (ImGui::Button("Set A")) { snapA_ = processor_.getSynthState().getPatchModeTempPatch(); hasSnapA_ = true; }
        ImGui::SameLine();
        if (ImGui::Button("Set B")) { snapB_ = processor_.getSynthState().getPatchModeTempPatch(); hasSnapB_ = true; }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(220.0f);
        if (ImGui::SliderFloat("Morph A<->B", &morphAmount_, 0.0f, 1.0f, "%.2f") && hasSnapA_ && hasSnapB_) {
            auto m = PatchTools::morphPatches(snapA_, snapB_, morphAmount_);
            processor_.getSynthState().setPatchModeTempPatch(m);
            processor_.getMidiManager().sendSysExQueue(processor_.getSynthState().serializePatchToDT1(
                processor_.getDeviceId(), addr::kPatchModeTempPatchBase, m));
        }
        ImGui::SameLine();
        if (ImGui::Button("Randomize Patch")) {
            auto p = processor_.getSynthState().getPatchModeTempPatch();
            RandomizeMask mask;
            PatchTools::randomizePatch(p, mask);
            processor_.getSynthState().setPatchModeTempPatch(p);
            processor_.getMidiManager().sendSysExQueue(processor_.getSynthState().serializePatchToDT1(
                processor_.getDeviceId(), addr::kPatchModeTempPatchBase, p));
            if (setStatus) setStatus("Patch aleatorizado!");
        }
    }

    if (showKeyboard) {
        ImGui::BeginGroup();
        int chDisp = kbdChannelZeroBased_ + 1;
        if (ui::FaderH("KbdCh", &chDisp, 1, 16, 74.0f, "%d", 1)) {
            kbdChannelZeroBased_ = chDisp - 1;
            if (processor_.getSynthState().getByte(addr::kSystemBase | addr::sys::SoundMode) == 1) {
                processor_.sendParameterDT1(addr::kSystemBase | addr::sys::PatchReceiveChannel, static_cast<uint8_t>(kbdChannelZeroBased_));
            }
        }
        ui::ToggleSwitch("Chord", &chordMode_);
        ImGui::SameLine();
        ui::ToggleSwitch("Arp", &arpMode_);
        if (ui::RolandButton("Phrase Preview", false, ImVec2(0.0f, 24.0f), 1)) triggerPhrasePreview();
        ImGui::SameLine();
        if (ui::RolandButton("Panic", false, ImVec2(0.0f, 24.0f), 2)) {
            for (int c = 0; c < 16; ++c) processor_.getMidiManager().sendAllNotesOff(c);
        }
        ImGui::EndGroup();

        ImGui::SameLine();
        int patchVol = processor_.getSynthState().getByte(addr::kPatchModeTempPatchBase | addr::patch_common::PatchLevel);
        if (ui::FaderV("Vol", &patchVol, 0, 127, 58.0f, "%d", 127)) {
            processor_.sendParameterDT1(addr::kPatchModeTempPatchBase | addr::patch_common::PatchLevel, static_cast<uint8_t>(patchVol));
            processor_.getMidiManager().sendControlChange(kbdChannelZeroBased_, 7, static_cast<uint8_t>(patchVol));
        }

        ImGui::SameLine();
        if (ui::FaderV("Mod", &modWheelVal_, 0, 127, 58.0f, "%d", 0)) {
            processor_.getMidiManager().sendControlChange(kbdChannelZeroBased_, 1, static_cast<uint8_t>(modWheelVal_));
        }

        ImGui::SameLine();
        float availW = std::max(140.0f, ImGui::GetContentRegionAvail().x);
        float kbdH = 88.0f;
        float targetWhiteW = kbdH * 0.245f; // ~21.5px per white key
        int octaves = std::clamp(static_cast<int>(std::round((availW / targetWhiteW - 1.0f) / 7.0f)), 2, 10);
        int startNote = 36; // C2
        if (octaves >= 6 && octaves <= 7) startNote = 24;      // C1
        else if (octaves >= 8 && octaves <= 9) startNote = 12; // C0
        else if (octaves >= 10) startNote = 0;                 // C-1
        int endNote = (octaves >= 10) ? 127 : std::min(127, startNote + octaves * 12);
        ui::VirtualKeyboard("##VKeyboard", startNote, endNote, activeVirtualNote_, [this](int note, bool isDown) {
            if (isDown) {
                processor_.getMidiManager().sendNoteOn(kbdChannelZeroBased_, note, 105);
                if (chordMode_) {
                    processor_.getMidiManager().sendNoteOn(kbdChannelZeroBased_, note + 4, 96);
                    processor_.getMidiManager().sendNoteOn(kbdChannelZeroBased_, note + 7, 96);
                }
            } else {
                processor_.getMidiManager().sendNoteOff(kbdChannelZeroBased_, note);
                if (chordMode_) {
                    processor_.getMidiManager().sendNoteOff(kbdChannelZeroBased_, note + 4);
                    processor_.getMidiManager().sendNoteOff(kbdChannelZeroBased_, note + 7);
                }
            }
        }, ImVec2(availW, kbdH));
    }

    ImGui::EndChild();
}

void VirtualKeyboardDock::triggerPhrasePreview() {
    auto sys = processor_.getSynthState().getSystem();
    for (int i = 0; i < 4; ++i) {
        uint8_t n = sys.common[addr::sys::PreviewNoteSet1 + static_cast<size_t>(i * 2)];
        uint8_t v = sys.common[addr::sys::PreviewVelocitySet1 + static_cast<size_t>(i * 2)];
        if (v > 0 && n > 0) processor_.getMidiManager().sendNoteOn(kbdChannelZeroBased_, n, v);
    }
}

void VirtualKeyboardDock::handleArpTick() {
    if (!arpMode_) return;
    double now = juce::Time::getMillisecondCounterHiRes();
    if (now - lastArpTimeMs_ < 180.0) return;
    lastArpTimeMs_ = now;
    static const int notes[] = { 48, 52, 55, 60, 64, 67, 60, 55 };
    processor_.getMidiManager().sendNoteOff(kbdChannelZeroBased_, notes[(arpStep_ + 7) % 8]);
    processor_.getMidiManager().sendNoteOn(kbdChannelZeroBased_, notes[arpStep_ % 8], 96);
    ++arpStep_;
}

} // namespace jv1010
