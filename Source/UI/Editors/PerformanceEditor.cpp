#include "PerformanceEditor.h"
#include "../ImGuiWidgets.h"
#include <imgui.h>
#include <algorithm>
#include <cstring>

namespace jv1010 {

PerformanceEditor::PerformanceEditor(JV1010EditorAudioProcessor& processor)
    : processor_(processor) {}

void PerformanceEditor::render(const std::function<void(int partIndexZeroBased)>& onEditPartPatch) {
    auto perf = processor_.getSynthState().getTempPerformance();

    char nameBuf[16]{};
    std::strncpy(nameBuf, perf.getName().c_str(), 12);
    ImGui::SetNextItemWidth(120.0f);
    if (ImGui::InputText("Perf Name", nameBuf, 13)) {
        auto enc = sysex::encodeAsciiName12(nameBuf);
        processor_.getSynthState().applyDT1(addr::kTempPerformanceBase, enc);
        processor_.getMidiManager().sendSysEx(sysex::buildDT1(processor_.getDeviceId(), addr::kTempPerformanceBase, enc));
    }

    ImGui::SameLine();
    int tempo = sysex::decodeNibbled2(perf.common[addr::perf_common::TempoNibbled], perf.common[addr::perf_common::TempoNibbled + 1]);
    ImGui::SetNextItemWidth(85.0f);
    if (ImGui::DragInt("Tempo", &tempo, 0.5f, 20, 250)) {
        processor_.sendParameterNibbled2DT1(addr::kTempPerformanceBase | addr::perf_common::TempoNibbled, tempo);
    }

    ImGui::SameLine();
    int clkSrc = perf.common[addr::perf_common::ClockSource];
    const char* clkNames[] = { "PERFORM", "SYSTEM" };
    ImGui::SetNextItemWidth(90.0f);
    if (ImGui::Combo("Clock", &clkSrc, clkNames, 2)) {
        processor_.sendParameterDT1(addr::kTempPerformanceBase | addr::perf_common::ClockSource, static_cast<uint8_t>(clkSrc));
    }

    ImGui::SameLine();
    int kMode = perf.common[addr::perf_common::KeyboardMode];
    const char* kModes[] = { "LAYER", "SINGLE" };
    ImGui::SetNextItemWidth(80.0f);
    if (ImGui::Combo("Mode", &kMode, kModes, 2)) {
        processor_.sendParameterDT1(addr::kTempPerformanceBase | addr::perf_common::KeyboardMode, static_cast<uint8_t>(kMode));
    }

    ImGui::SameLine();
    bool krSw = perf.common[addr::perf_common::KeyboardRangeSwitch] != 0;
    if (ui::ToggleSwitch("Key Range", &krSw)) {
        processor_.sendParameterDT1(addr::kTempPerformanceBase | addr::perf_common::KeyboardRangeSwitch, krSw ? 1 : 0);
    }

    ImGui::SameLine();
    int efxSrc = perf.common[addr::perf_common::EFXSource];
    const char* efxSrcNames[] = { "PERFORM", "Part 1", "Part 2", "Part 3", "Part 4", "Part 5", "Part 6", "Part 7", "Part 8", "Part 9", "Part 11", "Part 12", "Part 13", "Part 14", "Part 15", "Part 16" };
    ImGui::SetNextItemWidth(95.0f);
    if (ImGui::Combo("EFX Src", &efxSrc, efxSrcNames, 16)) {
        processor_.sendParameterDT1(addr::kTempPerformanceBase | addr::perf_common::EFXSource, static_cast<uint8_t>(efxSrc));
    }

    ImGui::SameLine();
    int totalRes = 0;
    for (int i = 0; i < 16; ++i) totalRes += perf.common[addr::perf_common::VoiceReserve1 + static_cast<size_t>(i)];
    ImGui::TextColored(totalRes > 64 ? ImVec4(0.94f, 0.27f, 0.27f, 1.0f) : ImVec4(0.06f, 0.73f, 0.51f, 1.0f),
                       "Voices: %d/64", totalRes);

    ImGui::Spacing();
    ui::SectionHeader("PERFORMANCE MIXER (16 PARTS)", IM_COL32(36, 98, 178, 255));
    ImGui::BeginChild("##PerfMixer", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_HorizontalScrollbar);
    for (int p = 0; p < 16; ++p) {
        if (p > 0) ImGui::SameLine();
        ImGui::PushID(p);
        ImGui::BeginChild("##Strip", ImVec2(142.0f, 0.0f), true);
        uint32_t pBase = addr::kTempPerformanceBase | (addr::kPerfPart1Offset + (static_cast<uint32_t>(p) << 8));
        const auto& pt = perf.parts[static_cast<size_t>(p)];

        ImGui::TextColored(p == 9 ? ImVec4(0.96f, 0.62f, 0.04f, 1.0f) : ImVec4(0.23f, 0.51f, 0.96f, 1.0f),
                           p == 9 ? "P10 DRUMS" : "PART %d", p + 1);
        bool rx = pt[addr::perf_part::ReceiveSwitch] != 0;
        if (ui::ToggleSwitch("On", &rx)) processor_.sendParameterDT1(pBase | addr::perf_part::ReceiveSwitch, rx ? 1 : 0);
        ImGui::SameLine();
        if (p != 9 && ui::RolandButton("Edit", false, ImVec2(48.0f, 24.0f), 1)) {
            if (onEditPartPatch) onEditPartPatch(p);
        }

        int ch = pt[addr::perf_part::MidiChannel] + 1;
        if (ui::FaderH("Ch", &ch, 1, 16, 76.0f, "%d", p + 1)) {
            processor_.sendParameterDT1(pBase | addr::perf_part::MidiChannel, static_cast<uint8_t>(ch - 1));
        }

        uint8_t gType = pt[addr::perf_part::PatchGroupType];
        uint8_t gId = pt[addr::perf_part::PatchGroupID];
        int gSel = (gType == 0 && gId == 1) ? 0 : (gType == 0 && gId >= 3 && gId <= 7 ? (gId - 2) : (gType == 2 && gId == 9 ? 6 : (gType == 2 ? 7 : 1)));
        const char* pGroups[] = { "USER", "PR-A", "PR-B", "PR-C", "PR-D", "PR-E", "XP-A", "XP-B" };
        ImGui::SetNextItemWidth(60.0f);
        if (ImGui::Combo("##Grp", &gSel, pGroups, 8)) {
            uint8_t ngType = (gSel >= 6) ? 2 : 0;
            uint8_t ngId = (gSel == 0) ? 1 : (gSel <= 5 ? static_cast<uint8_t>(gSel + 2) : (gSel == 6 ? 9 : 1));
            processor_.sendParameterDT1(pBase | addr::perf_part::PatchGroupType, ngType);
            processor_.sendParameterDT1(pBase | addr::perf_part::PatchGroupID, ngId);
        }
        ImGui::SameLine();
        int pNum = sysex::decodeNibbled2(pt[addr::perf_part::PatchNumberNibbled], pt[addr::perf_part::PatchNumberNibbled + 1]) + 1;
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::DragInt("##PNum", &pNum, 0.5f, 1, 256, "#%03d")) {
            processor_.sendParameterNibbled2DT1(pBase | addr::perf_part::PatchNumberNibbled, pNum - 1);
        }

        int outA = pt[addr::perf_part::OutputAssign];
        const char* outNames[] = { "MIX", "EFX", "DIR1", "DIR2", "PATCH" };
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::Combo("##Out", &outA, outNames, 5)) {
            processor_.sendParameterDT1(pBase | addr::perf_part::OutputAssign, static_cast<uint8_t>(outA));
        }

        int lvl = pt[addr::perf_part::PartLevel];
        if (ui::FaderV("Lvl", &lvl, 0, 127, 118.0f, "%d", 100)) {
            processor_.sendParameterDT1(pBase | addr::perf_part::PartLevel, static_cast<uint8_t>(lvl));
        }
        ImGui::SameLine();
        ImGui::BeginGroup();
        int pan = pt[addr::perf_part::PartPan];
        if (ui::KnobInt("Pan", &pan, 0, 127, "%d", 14.0f, 64)) processor_.sendParameterDT1(pBase | addr::perf_part::PartPan, static_cast<uint8_t>(pan));
        int mix = pt[addr::perf_part::MixEFXSendLevel];
        if (ui::KnobInt("Mix", &mix, 0, 127, "%d", 14.0f, 127)) processor_.sendParameterDT1(pBase | addr::perf_part::MixEFXSendLevel, static_cast<uint8_t>(mix));
        ImGui::EndGroup();

        int cho = pt[addr::perf_part::ChorusSendLevel];
        if (ui::KnobInt("Cho", &cho, 0, 127, "%d", 14.0f, 0)) processor_.sendParameterDT1(pBase | addr::perf_part::ChorusSendLevel, static_cast<uint8_t>(cho));
        ImGui::SameLine();
        int rev = pt[addr::perf_part::ReverbSendLevel];
        if (ui::KnobInt("Rev", &rev, 0, 127, "%d", 14.0f, 40)) processor_.sendParameterDT1(pBase | addr::perf_part::ReverbSendLevel, static_cast<uint8_t>(rev));

        int crs = static_cast<int>(pt[addr::perf_part::PartCoarseTune]) - 48;
        if (ui::KnobInt("Coarse", &crs, -48, 48, "%+d", 14.0f, 0)) processor_.sendParameterDT1(pBase | addr::perf_part::PartCoarseTune, static_cast<uint8_t>(crs + 48));
        ImGui::SameLine();
        int fin = static_cast<int>(pt[addr::perf_part::PartFineTune]) - 50;
        if (ui::KnobInt("Fine", &fin, -50, 50, "%+d", 14.0f, 0)) processor_.sendParameterDT1(pBase | addr::perf_part::PartFineTune, static_cast<uint8_t>(fin + 50));

        int res = perf.common[addr::perf_common::VoiceReserve1 + static_cast<size_t>(p)];
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::DragInt("##Res", &res, 0.25f, 0, 64, "Res %d")) {
            processor_.sendParameterDT1(addr::kTempPerformanceBase | (addr::perf_common::VoiceReserve1 + static_cast<uint32_t>(p)), static_cast<uint8_t>(res));
        }

        int kLo = pt[addr::perf_part::KeyboardRangeLower];
        int kHi = pt[addr::perf_part::KeyboardRangeUpper];
        ImGui::SetNextItemWidth(52.0f);
        if (ImGui::DragInt("##KLo", &kLo, 0.5f, 0, 127, "L:%d")) processor_.sendParameterDT1(pBase | addr::perf_part::KeyboardRangeLower, static_cast<uint8_t>(kLo));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::DragInt("##KHi", &kHi, 0.5f, 0, 127, "H:%d")) processor_.sendParameterDT1(pBase | addr::perf_part::KeyboardRangeUpper, static_cast<uint8_t>(kHi));

        bool rxPC = pt[addr::perf_part::ReceiveProgramChange] != 0;
        bool rxVol = pt[addr::perf_part::ReceiveVolume] != 0;
        bool rxHld = pt[addr::perf_part::ReceiveHold1] != 0;
        if (ui::ToggleSwitch("PC", &rxPC)) processor_.sendParameterDT1(pBase | addr::perf_part::ReceiveProgramChange, rxPC ? 1 : 0);
        ImGui::SameLine();
        if (ui::ToggleSwitch("Vl", &rxVol)) processor_.sendParameterDT1(pBase | addr::perf_part::ReceiveVolume, rxVol ? 1 : 0);
        ImGui::SameLine();
        if (ui::ToggleSwitch("Hd", &rxHld)) processor_.sendParameterDT1(pBase | addr::perf_part::ReceiveHold1, rxHld ? 1 : 0);

        ImGui::EndChild();
        ImGui::PopID();
    }
    ImGui::EndChild();
}

} // namespace jv1010
