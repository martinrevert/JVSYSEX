#include "SystemEditor.h"
#include "../ImGuiWidgets.h"
#include <imgui.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

namespace jv1010 {

SystemEditor::SystemEditor(JV1010EditorAudioProcessor& processor)
    : processor_(processor) {}

void SystemEditor::render() {
    auto sys = processor_.getSynthState().getSystem();
    auto sendSysB = [&](size_t off, int v) {
        processor_.sendParameterDT1(addr::kSystemBase | static_cast<uint32_t>(off), static_cast<uint8_t>(v));
    };

    // Row 1: Sound Mode, Clock Source, Perf Ctrl Ch, Patch Rx Ch, Vol Ctrl Src, Aftertouch Src
    int sMode = sys.common[addr::sys::SoundMode];
    const char* sModes[] = { "PERFORMANCE", "PATCH", "GM" };
    ImGui::SetNextItemWidth(120.0f);
    if (ImGui::Combo("Mode", &sMode, sModes, 3)) {
        if (sMode == 2) processor_.getMidiManager().sendSysEx(sysex::buildGMSystemOn());
        else sendSysB(addr::sys::SoundMode, sMode);
    }

    ImGui::SameLine();
    int clkSrc = sys.common[addr::sys::ClockSource];
    const char* clkNames[] = { "INT", "MIDI" };
    ImGui::SetNextItemWidth(75.0f);
    if (ImGui::Combo("Clock", &clkSrc, clkNames, 2)) sendSysB(addr::sys::ClockSource, clkSrc);

    ImGui::SameLine();
    int ctrlCh = sys.common[addr::sys::ControlChannel];
    const char* ctrlChNames[] = { "1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","OFF" };
    ImGui::SetNextItemWidth(70.0f);
    if (ImGui::Combo("PerfCtrlCh", &ctrlCh, ctrlChNames, 17)) sendSysB(addr::sys::ControlChannel, ctrlCh);

    ImGui::SameLine();
    int pRxCh = sys.common[addr::sys::PatchReceiveChannel] + 1;
    if (ui::FaderH("PatchRxCh", &pRxCh, 1, 16, 76.0f, "%d", 1)) sendSysB(addr::sys::PatchReceiveChannel, pRxCh - 1);

    ImGui::SameLine();
    int volSrc = sys.common[addr::sys::VolumeControlSource];
    const char* volSrcs[] = { "VOLUME", "VOL&EXP" };
    ImGui::SetNextItemWidth(95.0f);
    if (ImGui::Combo("VolSrc", &volSrc, volSrcs, 2)) sendSysB(addr::sys::VolumeControlSource, volSrc);

    ImGui::SameLine();
    int atSrc = sys.common[addr::sys::AftertouchSource];
    const char* atSrcs[] = { "CHANNEL", "POLY", "CH&POLY" };
    ImGui::SetNextItemWidth(95.0f);
    if (ImGui::Combo("AT Src", &atSrc, atSrcs, 3)) sendSysB(addr::sys::AftertouchSource, atSrc);

    // Row 2: SysCtrl1/2, Master Tune, System Tempo
    int sc1 = std::clamp<int>(sys.common[addr::sys::SystemControlSource1], 1, 97);
    int sc2 = std::clamp<int>(sys.common[addr::sys::SystemControlSource2], 1, 97);
    ImGui::SetNextItemWidth(95.0f);
    if (ImGui::DragInt("SysCtrl1", &sc1, 0.5f, 1, 97)) sendSysB(addr::sys::SystemControlSource1, sc1);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(95.0f);
    if (ImGui::DragInt("SysCtrl2", &sc2, 0.5f, 1, 97)) sendSysB(addr::sys::SystemControlSource2, sc2);
    ImGui::SameLine();

    float mTuneHz = 427.4f + sys.common[addr::sys::MasterTune] * 0.2f;
    ImGui::SetNextItemWidth(120.0f);
    if (ImGui::SliderFloat("MasterTune", &mTuneHz, 427.4f, 452.6f, "%.1f Hz")) {
        int v = std::clamp(static_cast<int>(std::round((mTuneHz - 427.4f) / 0.2f)), 0, 126);
        sendSysB(addr::sys::MasterTune, v);
    }
    ImGui::SameLine();

    int sysTempo = sysex::decodeNibbled2(sys.common[addr::sys::SystemTempoNibbled], sys.common[addr::sys::SystemTempoNibbled + 1]);
    ImGui::SetNextItemWidth(95.0f);
    if (ImGui::DragInt("SysTempo", &sysTempo, 0.5f, 20, 250, "%d BPM")) {
        processor_.sendParameterNibbled2DT1(addr::kSystemBase | addr::sys::SystemTempoNibbled, sysTempo);
    }

    // Row 3: Switches
    auto sysBool = [&](const char* lbl, size_t off) {
        bool b = sys.common[off] != 0;
        if (ui::ToggleSwitch(lbl, &b)) sendSysB(off, b ? 1 : 0);
    };
    sysBool("ScaleTune", addr::sys::ScaleTuneSwitch); ImGui::SameLine();
    sysBool("EFX", addr::sys::EFXSwitch); ImGui::SameLine();
    sysBool("Chorus", addr::sys::ChorusSwitch); ImGui::SameLine();
    sysBool("Reverb", addr::sys::ReverbSwitch); ImGui::SameLine();
    sysBool("PatchRemain", addr::sys::PatchRemain); ImGui::SameLine();
    sysBool("Rx ProgChg", addr::sys::ReceiveProgramChange); ImGui::SameLine();
    sysBool("Rx BankSel", addr::sys::ReceiveBankSelect);

    ImGui::Spacing();
    ui::SectionHeader("PHRASE PREVIEW CONFIG", IM_COL32(195, 118, 24, 255));
    int prevMode = sys.common[addr::sys::PreviewSoundMode];
    const char* pModes[] = { "SINGLE", "CHORD", "PHRASE" };
    ImGui::SetNextItemWidth(110.0f);
    if (ImGui::Combo("PreviewMode", &prevMode, pModes, 3)) sendSysB(addr::sys::PreviewSoundMode, prevMode);

    for (int i = 0; i < 4; ++i) {
        ImGui::SameLine();
        ImGui::PushID(i);
        int pn = sys.common[addr::sys::PreviewNoteSet1 + static_cast<size_t>(i * 2)];
        int pv = sys.common[addr::sys::PreviewVelocitySet1 + static_cast<size_t>(i * 2)];
        ImGui::BeginGroup();
        ImGui::SetNextItemWidth(75.0f);
        if (ImGui::DragInt("##N", &pn, 0.5f, 0, 127, "N%d")) sendSysB(addr::sys::PreviewNoteSet1 + static_cast<size_t>(i * 2), pn);
        ImGui::SetNextItemWidth(75.0f);
        if (ImGui::DragInt("##V", &pv, 0.5f, 0, 127, "V%d")) sendSysB(addr::sys::PreviewVelocitySet1 + static_cast<size_t>(i * 2), pv);
        ImGui::EndGroup();
        ImGui::PopID();
    }

    ImGui::Separator();
    // Scale Tune
    ImGui::TextColored(ImVec4(0.96f, 0.62f, 0.04f, 1.0f), "SCALE TUNE (-64..+63 cents)");
    ImGui::SetNextItemWidth(140.0f);
    std::string stTgtLbl = (scaleTargetIdx_ == 0) ? "Patch Mode" : ("Part " + std::to_string(scaleTargetIdx_));
    if (ImGui::BeginCombo("Scale Target", stTgtLbl.c_str())) {
        if (ImGui::Selectable("Patch Mode", scaleTargetIdx_ == 0)) scaleTargetIdx_ = 0;
        for (int p = 1; p <= 16; ++p) {
            std::string l = "Part " + std::to_string(p);
            if (ImGui::Selectable(l.c_str(), scaleTargetIdx_ == p)) scaleTargetIdx_ = p;
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    ui::ToggleSwitch("Send GS (42H)", &sendGS42H_);

    auto applyPreset = [&](const std::array<int, 12>& cents) {
        uint32_t stAddr = (scaleTargetIdx_ == 0) ? addr::kSystemScaleTunePatch
                                                 : (addr::kSystemScaleTunePart1 + (static_cast<uint32_t>(scaleTargetIdx_ - 1) << 8));
        std::vector<uint8_t> data(12);
        for (int i = 0; i < 12; ++i) data[static_cast<size_t>(i)] = static_cast<uint8_t>(cents[static_cast<size_t>(i)] + 64);
        processor_.getSynthState().applyDT1(stAddr, data);
        processor_.getMidiManager().sendSysEx(sysex::buildDT1(processor_.getDeviceId(), stAddr, data));
        if (sendGS42H_) {
            uint32_t gsAddr = (scaleTargetIdx_ == 0) ? 0x401140 : (0x401040 + (static_cast<uint32_t>(scaleTargetIdx_ == 10 ? 0 : scaleTargetIdx_) << 8));
            processor_.getMidiManager().sendSysEx(sysex::buildGS_DT1(processor_.getDeviceId(), gsAddr, data));
        }
    };
    ImGui::SameLine(); if (ImGui::Button("Equal Temp")) applyPreset({ 0,0,0,0,0,0,0,0,0,0,0,0 });
    ImGui::SameLine(); if (ImGui::Button("Just (C)")) applyPreset({ 0,-8,4,16,-14,-2,-10,2,14,-16,14,-12 });
    ImGui::SameLine(); if (ImGui::Button("Arabian")) applyPreset({ -6,45,-2,-12,-51,-8,43,-4,47,0,-10,-49 });

    const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int stSlot = (scaleTargetIdx_ == 0) ? 16 : (scaleTargetIdx_ - 1);
    uint32_t stAddr = (scaleTargetIdx_ == 0) ? addr::kSystemScaleTunePatch
                                             : (addr::kSystemScaleTunePart1 + (static_cast<uint32_t>(scaleTargetIdx_ - 1) << 8));
    for (int n = 0; n < 12; ++n) {
        if (n > 0) ImGui::SameLine();
        ImGui::PushID(n);
        int val = static_cast<int>(sys.scaleTunes[static_cast<size_t>(stSlot)][static_cast<size_t>(n)]) - 64;
        if (ui::FaderV(noteNames[n], &val, -64, 63, 110.0f, "%+d", 0)) {
            processor_.sendParameterDT1(stAddr + static_cast<uint32_t>(n), static_cast<uint8_t>(val + 64));
        }
        ImGui::PopID();
    }
}

} // namespace jv1010
