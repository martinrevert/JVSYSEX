#include "PatchEffectsEditor.h"
#include "../ImGuiWidgets.h"
#include "../../Model/EfxAlgorithms.h"
#include <imgui.h>
#include <algorithm>

namespace jv1010 {

PatchEffectsEditor::PatchEffectsEditor(JV1010EditorAudioProcessor& processor)
    : processor_(processor) {}

void PatchEffectsEditor::render() {
    uint32_t base = perfMode_ ? addr::kTempPerformanceBase : addr::kPatchModeTempPatchBase;
    auto getB = [&](uint32_t off) -> int { return processor_.getSynthState().getByte(base | off); };
    auto sendB = [&](uint32_t off, int v) { processor_.sendParameterDT1(base | off, static_cast<uint8_t>(v)); };

    // Master Effect Switches (System Common) + Quick Route to EFX
    bool mEfx = processor_.getSynthState().getByte(addr::kSystemBase | addr::sys::EFXSwitch) != 0;
    bool mCho = processor_.getSynthState().getByte(addr::kSystemBase | addr::sys::ChorusSwitch) != 0;
    bool mRev = processor_.getSynthState().getByte(addr::kSystemBase | addr::sys::ReverbSwitch) != 0;
    if (ui::ToggleSwitch("Master EFX", &mEfx)) {
        processor_.sendParameterDT1(addr::kSystemBase | addr::sys::EFXSwitch, mEfx ? 1 : 0);
    }
    ImGui::SameLine(0.0f, 14.0f);
    if (ui::ToggleSwitch("Master Chorus", &mCho)) {
        processor_.sendParameterDT1(addr::kSystemBase | addr::sys::ChorusSwitch, mCho ? 1 : 0);
    }
    ImGui::SameLine(0.0f, 14.0f);
    if (ui::ToggleSwitch("Master Reverb", &mRev)) {
        processor_.sendParameterDT1(addr::kSystemBase | addr::sys::ReverbSwitch, mRev ? 1 : 0);
    }
    ImGui::SameLine(0.0f, 18.0f);
    if (ui::RolandButton("Route Active Tones -> EFX", false, ImVec2(0.0f, 28.0f), 1)) {
        for (int t = 0; t < 4; ++t) {
            uint32_t tBase = addr::kPatchModeTempPatchBase | addr::getPatchToneOffset(t);
            if (processor_.getSynthState().getByte(tBase | addr::patch_tone::ToneSwitch) != 0) {
                processor_.sendParameterDT1(tBase | addr::patch_tone::OutputAssign, 1); // 1 = EFX
            }
        }
        if (!mEfx) {
            processor_.sendParameterDT1(addr::kSystemBase | addr::sys::EFXSwitch, 1);
        }
    }

    ImGui::Spacing();
    ui::SectionHeader("EFX (MULTI-EFFECTS)", IM_COL32(36, 98, 178, 255));

    const char* targets[] = { "Patch Mode Effects", "Performance Effects" };
    int tgt = perfMode_ ? 1 : 0;
    ImGui::SetNextItemWidth(175.0f);
    if (ImGui::Combo("##EfxTarget", &tgt, targets, 2)) perfMode_ = (tgt == 1);

    ImGui::SameLine();
    int efxType = getB(perfMode_ ? addr::perf_common::EFXType : addr::patch_common::EFXType);
    const auto& algos = getEfxAlgorithms();
    ImGui::SetNextItemWidth(230.0f);
    if (ImGui::BeginCombo("EFX Type", algos[static_cast<size_t>(std::clamp(efxType, 0, 39))].name)) {
        for (const auto& a : algos) {
            if (ImGui::Selectable(a.name, efxType == a.typeIndex)) {
                sendB(perfMode_ ? addr::perf_common::EFXType : addr::patch_common::EFXType, a.typeIndex);
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    int efxOut = getB(perfMode_ ? addr::perf_common::EFXOutputAssign : addr::patch_common::EFXOutputAssign);
    const char* outNames[] = { "MIX", "DIRECT-1", "DIRECT-2" };
    ImGui::SetNextItemWidth(100.0f);
    if (ImGui::Combo("EFX Out", &efxOut, outNames, 3)) {
        sendB(perfMode_ ? addr::perf_common::EFXOutputAssign : addr::patch_common::EFXOutputAssign, efxOut);
    }

    const auto& curAlg = getEfxAlgorithm(efxType);
    uint32_t pBase = perfMode_ ? addr::perf_common::EFXParam1 : addr::patch_common::EFXParam1;
    for (int i = 0; i < 12; ++i) {
        if (i % 6 != 0) ImGui::SameLine();
        const auto& pd = curAlg.params[static_cast<size_t>(i)];
        int val = getB(pBase + static_cast<uint32_t>(i));
        ImGui::BeginDisabled(!pd.used);
        if (ui::KnobInt(pd.name, &val, pd.minVal, pd.maxVal, "%d", 17.0f, pd.minVal)) {
            sendB(pBase + static_cast<uint32_t>(i), val);
        }
        ImGui::EndDisabled();
    }

    // EFX Routing & Control Sources 1 & 2
    int eMix = getB(perfMode_ ? addr::perf_common::EFXMixOutSendLevel : addr::patch_common::EFXMixOutSendLevel);
    int eCho = getB(perfMode_ ? addr::perf_common::EFXChorusSendLevel : addr::patch_common::EFXChorusSendLevel);
    int eRev = getB(perfMode_ ? addr::perf_common::EFXReverbSendLevel : addr::patch_common::EFXReverbSendLevel);
    int cSrc1 = getB(perfMode_ ? addr::perf_common::EFXControlSource1 : addr::patch_common::EFXControlSource1);
    int cDep1 = getB(perfMode_ ? addr::perf_common::EFXControlDepth1 : addr::patch_common::EFXControlDepth1) - 63;
    int cSrc2 = getB(perfMode_ ? addr::perf_common::EFXControlSource2 : addr::patch_common::EFXControlSource2);
    int cDep2 = getB(perfMode_ ? addr::perf_common::EFXControlDepth2 : addr::patch_common::EFXControlDepth2) - 63;

    if (ui::KnobInt("MixOut", &eMix, 0, 127)) sendB(perfMode_ ? addr::perf_common::EFXMixOutSendLevel : addr::patch_common::EFXMixOutSendLevel, eMix);
    ImGui::SameLine();
    if (ui::KnobInt("ChoSnd", &eCho, 0, 127)) sendB(perfMode_ ? addr::perf_common::EFXChorusSendLevel : addr::patch_common::EFXChorusSendLevel, eCho);
    ImGui::SameLine();
    if (ui::KnobInt("RevSnd", &eRev, 0, 127)) sendB(perfMode_ ? addr::perf_common::EFXReverbSendLevel : addr::patch_common::EFXReverbSendLevel, eRev);
    ImGui::SameLine();
    if (ui::KnobInt("Ctrl1Dep", &cDep1, -63, 63, "%+d", 17.0f, 0)) sendB(perfMode_ ? addr::perf_common::EFXControlDepth1 : addr::patch_common::EFXControlDepth1, cDep1 + 63);
    ImGui::SameLine();
    if (ui::KnobInt("Ctrl2Dep", &cDep2, -63, 63, "%+d", 17.0f, 0)) sendB(perfMode_ ? addr::perf_common::EFXControlDepth2 : addr::patch_common::EFXControlDepth2, cDep2 + 63);

    const char* ctrlSrcNames[] = { "OFF", "SYS-CTRL1", "SYS-CTRL2", "MODULATION", "BREATH", "FOOT", "VOLUME", "PAN", "EXPRESSION", "PITCH BEND", "AFTERTOUCH" };
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::SetNextItemWidth(120.0f);
    if (ImGui::Combo("Ctrl 1 Src", &cSrc1, ctrlSrcNames, 11)) sendB(perfMode_ ? addr::perf_common::EFXControlSource1 : addr::patch_common::EFXControlSource1, cSrc1);
    ImGui::SetNextItemWidth(120.0f);
    if (ImGui::Combo("Ctrl 2 Src", &cSrc2, ctrlSrcNames, 11)) sendB(perfMode_ ? addr::perf_common::EFXControlSource2 : addr::patch_common::EFXControlSource2, cSrc2);
    ImGui::EndGroup();

    ImGui::Spacing();
    ui::SectionHeader("CHORUS", IM_COL32(198, 42, 42, 255));
    int cOut = getB(perfMode_ ? addr::perf_common::ChorusOutput : addr::patch_common::ChorusOutput);
    const char* choOuts[] = { "MIX", "REV", "MIX+REV" };
    ImGui::SetNextItemWidth(110.0f);
    if (ImGui::Combo("Chorus Out", &cOut, choOuts, 3)) sendB(perfMode_ ? addr::perf_common::ChorusOutput : addr::patch_common::ChorusOutput, cOut);

    int cLvl = getB(perfMode_ ? addr::perf_common::ChorusLevel : addr::patch_common::ChorusLevel);
    int cRat = getB(perfMode_ ? addr::perf_common::ChorusRate : addr::patch_common::ChorusRate);
    int cDep = getB(perfMode_ ? addr::perf_common::ChorusDepth : addr::patch_common::ChorusDepth);
    int cPre = getB(perfMode_ ? addr::perf_common::ChorusPreDelay : addr::patch_common::ChorusPreDelay);
    int cFbk = getB(perfMode_ ? addr::perf_common::ChorusFeedback : addr::patch_common::ChorusFeedback);
    if (ui::KnobInt("C.Level", &cLvl, 0, 127)) sendB(perfMode_ ? addr::perf_common::ChorusLevel : addr::patch_common::ChorusLevel, cLvl);
    ImGui::SameLine(); if (ui::KnobInt("C.Rate", &cRat, 0, 127)) sendB(perfMode_ ? addr::perf_common::ChorusRate : addr::patch_common::ChorusRate, cRat);
    ImGui::SameLine(); if (ui::KnobInt("C.Depth", &cDep, 0, 127)) sendB(perfMode_ ? addr::perf_common::ChorusDepth : addr::patch_common::ChorusDepth, cDep);
    ImGui::SameLine(); if (ui::KnobInt("C.PreDly", &cPre, 0, 127)) sendB(perfMode_ ? addr::perf_common::ChorusPreDelay : addr::patch_common::ChorusPreDelay, cPre);
    ImGui::SameLine(); if (ui::KnobInt("C.Fdbk", &cFbk, 0, 127)) sendB(perfMode_ ? addr::perf_common::ChorusFeedback : addr::patch_common::ChorusFeedback, cFbk);

    ImGui::Spacing();
    ui::SectionHeader("REVERB / DELAY", IM_COL32(195, 118, 24, 255));
    const char* rTypes[] = { "ROOM1", "ROOM2", "STAGE1", "STAGE2", "HALL1", "HALL2", "DELAY", "PAN-DLY" };
    int rTyp = getB(perfMode_ ? addr::perf_common::ReverbType : addr::patch_common::ReverbType);
    ImGui::SetNextItemWidth(130.0f);
    if (ImGui::Combo("Reverb Type", &rTyp, rTypes, 8)) sendB(perfMode_ ? addr::perf_common::ReverbType : addr::patch_common::ReverbType, rTyp);
    ImGui::SameLine();
    const char* hfVals[] = { "200Hz","250Hz","315Hz","400Hz","500Hz","630Hz","800Hz","1kHz","1.25k","1.6k","2kHz","2.5k","3.15k","4kHz","5kHz","6.3k","8kHz","BYPASS" };
    int rHfd = getB(perfMode_ ? addr::perf_common::ReverbHFDamp : addr::patch_common::ReverbHFDamp);
    ImGui::SetNextItemWidth(110.0f);
    if (ImGui::Combo("HF Damp", &rHfd, hfVals, 18)) sendB(perfMode_ ? addr::perf_common::ReverbHFDamp : addr::patch_common::ReverbHFDamp, rHfd);

    int rLvl = getB(perfMode_ ? addr::perf_common::ReverbLevel : addr::patch_common::ReverbLevel);
    int rTim = getB(perfMode_ ? addr::perf_common::ReverbTime : addr::patch_common::ReverbTime);
    int dFbk = getB(perfMode_ ? addr::perf_common::DelayFeedback : addr::patch_common::DelayFeedback);
    if (ui::KnobInt("R.Level", &rLvl, 0, 127)) sendB(perfMode_ ? addr::perf_common::ReverbLevel : addr::patch_common::ReverbLevel, rLvl);
    ImGui::SameLine(); if (ui::KnobInt("R.Time", &rTim, 0, 127)) sendB(perfMode_ ? addr::perf_common::ReverbTime : addr::patch_common::ReverbTime, rTim);
    ImGui::SameLine(); if (ui::KnobInt("Dly Fbk", &dFbk, 0, 127)) sendB(perfMode_ ? addr::perf_common::DelayFeedback : addr::patch_common::DelayFeedback, dFbk);
}

} // namespace jv1010
