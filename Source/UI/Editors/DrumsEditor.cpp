#include "DrumsEditor.h"
#include "../ImGuiWidgets.h"
#include "../../Database/WaveformsDB.h"
#include <imgui.h>
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace jv1010 {

DrumsEditor::DrumsEditor(JV1010EditorAudioProcessor& processor)
    : processor_(processor) {}

void DrumsEditor::render() {
    auto kit = processor_.getSynthState().getTempRhythm();
    char nameBuf[16]{};
    std::strncpy(nameBuf, kit.getName().c_str(), 12);
    ImGui::SetNextItemWidth(130.0f);
    if (ImGui::InputText("Kit Name", nameBuf, 13)) {
        auto enc = sysex::encodeAsciiName12(nameBuf);
        processor_.getSynthState().applyDT1(addr::kTempRhythmBase, enc);
        processor_.getMidiManager().sendSysEx(sysex::buildDT1(processor_.getDeviceId(), addr::kTempRhythmBase, enc));
    }
    ImGui::SameLine();
    int midiKey = static_cast<int>(addr::kRhythmNoteFirstKey) + selectedKeyIdx_;
    if (ImGui::Button("Audition Key (Ch 10)")) {
        processor_.getMidiManager().sendNoteOn(9, midiKey, 110);
    }

    ImGui::Separator();
    ImGui::BeginChild("##DrumKeys", ImVec2(195.0f, 0.0f), true);
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(addr::kRhythmNoteCount));
    while (clipper.Step()) {
        for (int k = clipper.DisplayStart; k < clipper.DisplayEnd; ++k) {
            int mk = static_cast<int>(addr::kRhythmNoteFirstKey) + k;
            auto nn = juce::MidiMessage::getMidiNoteName(mk, true, true, 4);
            int wNum = sysex::decodeNibbled2(kit.notes[static_cast<size_t>(k)][addr::rhythm_note::WaveNumberNibbled],
                                             kit.notes[static_cast<size_t>(k)][addr::rhythm_note::WaveNumberNibbled + 1]) + 1;
            char lbl[48];
            std::snprintf(lbl, sizeof(lbl), "%d (%-3s) W:%03d", mk, nn.toRawUTF8(), wNum);
            if (ImGui::Selectable(lbl, selectedKeyIdx_ == k)) {
                selectedKeyIdx_ = k;
                processor_.getMidiManager().sendNoteOn(9, mk, 105);
            }
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("##DrumInspector", ImVec2(0.0f, 0.0f), true);
    ui::SectionHeader("RHYTHM KIT NOTE INSPECTOR", IM_COL32(198, 42, 42, 255));
    const auto& n = kit.notes[static_cast<size_t>(selectedKeyIdx_)];
    uint32_t nAddr = addr::kTempRhythmBase | addr::getRhythmNoteOffset(midiKey);
    auto sendNB = [&](size_t off, uint8_t v) { processor_.sendParameterDT1(nAddr | static_cast<uint32_t>(off), v); };

    // Row 1: Tone Switch, Wave Group, Wave #, Wave Gain, Env Mode
    bool tOn = n[addr::rhythm_note::ToneSwitch] != 0;
    if (ui::ToggleSwitch("Tone On", &tOn)) sendNB(addr::rhythm_note::ToneSwitch, tOn ? 1 : 0);
    ImGui::SameLine();

    int wgType = n[addr::rhythm_note::WaveGroupType];
    int wgId = n[addr::rhythm_note::WaveGroupID];
    int wgSel = (wgType == 0 && wgId == 2) ? 1 : (wgType == 2 && wgId == 9 ? 2 : (wgType == 2 ? 3 : 0));
    const char* wgNames[] = { "INT-A", "INT-B", "XP-A (Session)", "XP-B (EXP)" };
    ImGui::SetNextItemWidth(120.0f);
    if (ImGui::Combo("WaveGrp", &wgSel, wgNames, 4)) {
        sendNB(addr::rhythm_note::WaveGroupType, wgSel <= 1 ? 0 : 2);
        sendNB(addr::rhythm_note::WaveGroupID, wgSel == 0 ? 1 : (wgSel == 1 ? 2 : (wgSel == 2 ? 9 : 1)));
    }
    ImGui::SameLine();

    int wNum = sysex::decodeNibbled2(n[addr::rhythm_note::WaveNumberNibbled], n[addr::rhythm_note::WaveNumberNibbled + 1]) + 1;
    int maxW = getWaveformCount(static_cast<WaveGroup>(wgSel));
    ImGui::SetNextItemWidth(185.0f);
    std::string curWaveStr = std::to_string(wNum) + " " + getWaveformName(static_cast<WaveGroup>(wgSel), wNum);
    if (ImGui::BeginCombo("Wave", curWaveStr.c_str())) {
        for (int i = 1; i <= maxW; ++i) {
            std::string wl = std::to_string(i) + " " + getWaveformName(static_cast<WaveGroup>(wgSel), i);
            if (ImGui::Selectable(wl.c_str(), wNum == i)) {
                auto nib = sysex::encodeNibbled2(i - 1);
                sendNB(addr::rhythm_note::WaveNumberNibbled, nib.first);
                sendNB(addr::rhythm_note::WaveNumberNibbled + 1, nib.second);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();

    int wGain = n[addr::rhythm_note::WaveGain];
    const char* gains[] = { "-6dB", "0dB", "+6dB", "+12dB" };
    ImGui::SetNextItemWidth(70.0f);
    if (ImGui::Combo("Gain", &wGain, gains, 4)) sendNB(addr::rhythm_note::WaveGain, static_cast<uint8_t>(wGain));

    // Row 2: EnvMode, FilterType, Output, PanCtrl, Vol/Hold
    int envMode = n[addr::rhythm_note::EnvelopeMode];
    const char* eModes[] = { "NO-SUS", "SUSTAIN" };
    ImGui::SetNextItemWidth(90.0f);
    if (ImGui::Combo("EnvMode", &envMode, eModes, 2)) sendNB(addr::rhythm_note::EnvelopeMode, static_cast<uint8_t>(envMode));
    ImGui::SameLine();

    int fltT = n[addr::rhythm_note::FilterType];
    const char* flts[] = { "OFF", "LPF", "BPF", "HPF", "PKG" };
    ImGui::SetNextItemWidth(75.0f);
    if (ImGui::Combo("Filter", &fltT, flts, 5)) sendNB(addr::rhythm_note::FilterType, static_cast<uint8_t>(fltT));
    ImGui::SameLine();

    int outA = n[addr::rhythm_note::OutputAssign];
    const char* outs[] = { "MIX", "EFX", "DIR1", "DIR2" };
    ImGui::SetNextItemWidth(80.0f);
    if (ImGui::Combo("Out", &outA, outs, 4)) sendNB(addr::rhythm_note::OutputAssign, static_cast<uint8_t>(outA));
    ImGui::SameLine();

    int pCtrl = n[addr::rhythm_note::PanControlSwitch];
    const char* pCtrls[] = { "Pan:OFF", "Pan:CONT", "Pan:KEY" };
    ImGui::SetNextItemWidth(90.0f);
    if (ImGui::Combo("##PanC", &pCtrl, pCtrls, 3)) sendNB(addr::rhythm_note::PanControlSwitch, static_cast<uint8_t>(pCtrl));
    ImGui::SameLine();

    bool vSw = n[addr::rhythm_note::VolumeSwitch] != 0, hSw = n[addr::rhythm_note::Hold1Switch] != 0;
    if (ui::ToggleSwitch("Vol", &vSw)) sendNB(addr::rhythm_note::VolumeSwitch, vSw ? 1 : 0);
    ImGui::SameLine();
    if (ui::ToggleSwitch("Hold1", &hSw)) sendNB(addr::rhythm_note::Hold1Switch, hSw ? 1 : 0);

    // Knobs Row 1
    int bnd = n[addr::rhythm_note::BendRange], mute = n[addr::rhythm_note::MuteGroup];
    int coarse = n[addr::rhythm_note::CoarseTune], fine = static_cast<int>(n[addr::rhythm_note::FineTune]) - 50;
    int rndP = n[addr::rhythm_note::RandomPitchDepth], pDep = static_cast<int>(n[addr::rhythm_note::PitchEnvDepth]) - 12;
    int cut = n[addr::rhythm_note::CutoffFrequency], res = n[addr::rhythm_note::Resonance], fDep = static_cast<int>(n[addr::rhythm_note::FilterEnvDepth]) - 63;

    if (ui::KnobInt("Bend", &bnd, 0, 12)) sendNB(addr::rhythm_note::BendRange, static_cast<uint8_t>(bnd));
    ImGui::SameLine(); if (ui::KnobInt("MuteGrp", &mute, 0, 31)) sendNB(addr::rhythm_note::MuteGroup, static_cast<uint8_t>(mute));
    ImGui::SameLine(); if (ui::KnobInt("Coarse", &coarse, 0, 127)) sendNB(addr::rhythm_note::CoarseTune, static_cast<uint8_t>(coarse));
    ImGui::SameLine(); if (ui::KnobInt("Fine", &fine, -50, 50, "%+d", 17.0f, 0)) sendNB(addr::rhythm_note::FineTune, static_cast<uint8_t>(fine + 50));
    ImGui::SameLine(); if (ui::KnobInt("RndPch", &rndP, 0, 30)) sendNB(addr::rhythm_note::RandomPitchDepth, static_cast<uint8_t>(rndP));
    ImGui::SameLine(); if (ui::KnobInt("PchEnv", &pDep, -12, 12, "%+d", 17.0f, 0)) sendNB(addr::rhythm_note::PitchEnvDepth, static_cast<uint8_t>(pDep + 12));
    ImGui::SameLine(); if (ui::KnobInt("Cutoff", &cut, 0, 127)) sendNB(addr::rhythm_note::CutoffFrequency, static_cast<uint8_t>(cut));
    ImGui::SameLine(); if (ui::KnobInt("Reso", &res, 0, 127)) sendNB(addr::rhythm_note::Resonance, static_cast<uint8_t>(res));
    ImGui::SameLine(); if (ui::KnobInt("FltEnv", &fDep, -63, 63, "%+d", 17.0f, 0)) sendNB(addr::rhythm_note::FilterEnvDepth, static_cast<uint8_t>(fDep + 63));

    // Knobs Row 2
    int rVs = n[addr::rhythm_note::ResonanceVelSens], lvl = n[addr::rhythm_note::ToneLevel], pan = n[addr::rhythm_note::TonePan];
    int rndPan = n[addr::rhythm_note::RandomPanDepth], altPan = n[addr::rhythm_note::AlternatePanDepth];
    int mix = n[addr::rhythm_note::MixEFXSendLevel], cho = n[addr::rhythm_note::ChorusSendLevel], rev = n[addr::rhythm_note::ReverbSendLevel];

    if (ui::KnobInt("ResVel", &rVs, 0, 125)) sendNB(addr::rhythm_note::ResonanceVelSens, static_cast<uint8_t>(rVs));
    ImGui::SameLine(); if (ui::KnobInt("Level", &lvl, 0, 127)) sendNB(addr::rhythm_note::ToneLevel, static_cast<uint8_t>(lvl));
    ImGui::SameLine(); if (ui::KnobInt("Pan", &pan, 0, 127)) sendNB(addr::rhythm_note::TonePan, static_cast<uint8_t>(pan));
    ImGui::SameLine(); if (ui::KnobInt("RndPan", &rndPan, 0, 63)) sendNB(addr::rhythm_note::RandomPanDepth, static_cast<uint8_t>(rndPan));
    ImGui::SameLine(); if (ui::KnobInt("AltPan", &altPan, 1, 127)) sendNB(addr::rhythm_note::AlternatePanDepth, static_cast<uint8_t>(altPan));
    ImGui::SameLine(); if (ui::KnobInt("MixEFX", &mix, 0, 127)) sendNB(addr::rhythm_note::MixEFXSendLevel, static_cast<uint8_t>(mix));
    ImGui::SameLine(); if (ui::KnobInt("Chorus", &cho, 0, 127)) sendNB(addr::rhythm_note::ChorusSendLevel, static_cast<uint8_t>(cho));
    ImGui::SameLine(); if (ui::KnobInt("Reverb", &rev, 0, 127)) sendNB(addr::rhythm_note::ReverbSendLevel, static_cast<uint8_t>(rev));

    // 3 Envelope Graphs side-by-side
    float ew = std::max(140.0f, (ImGui::GetContentRegionAvail().x - 16.0f) / 3.0f);
    int tPch[4] = { n[addr::rhythm_note::PitchEnvTime1], n[addr::rhythm_note::PitchEnvTime2], n[addr::rhythm_note::PitchEnvTime3], n[addr::rhythm_note::PitchEnvTime4] };
    int lPch[4] = { n[addr::rhythm_note::PitchEnvLevel1], n[addr::rhythm_note::PitchEnvLevel2], n[addr::rhythm_note::PitchEnvLevel3], n[addr::rhythm_note::PitchEnvLevel4] };
    if (ui::EnvelopeGraph("DrumPitch", "Pitch Env", tPch, lPch, true, true, ImVec2(ew, 140.0f))) {
        for (int i = 0; i < 4; ++i) {
            sendNB(addr::rhythm_note::PitchEnvTime1 + static_cast<size_t>(i), static_cast<uint8_t>(tPch[i]));
            sendNB(addr::rhythm_note::PitchEnvLevel1 + static_cast<size_t>(i), static_cast<uint8_t>(lPch[i]));
        }
    }
    ImGui::SameLine();
    int tTvf[4] = { n[addr::rhythm_note::FilterEnvTime1], n[addr::rhythm_note::FilterEnvTime2], n[addr::rhythm_note::FilterEnvTime3], n[addr::rhythm_note::FilterEnvTime4] };
    int lTvf[4] = { n[addr::rhythm_note::FilterEnvLevel1], n[addr::rhythm_note::FilterEnvLevel2], n[addr::rhythm_note::FilterEnvLevel3], n[addr::rhythm_note::FilterEnvLevel4] };
    if (ui::EnvelopeGraph("DrumTvf", "Filter Env", tTvf, lTvf, false, true, ImVec2(ew, 140.0f))) {
        for (int i = 0; i < 4; ++i) {
            sendNB(addr::rhythm_note::FilterEnvTime1 + static_cast<size_t>(i), static_cast<uint8_t>(tTvf[i]));
            sendNB(addr::rhythm_note::FilterEnvLevel1 + static_cast<size_t>(i), static_cast<uint8_t>(lTvf[i]));
        }
    }
    ImGui::SameLine();
    int tTva[4] = { n[addr::rhythm_note::LevelEnvTime1], n[addr::rhythm_note::LevelEnvTime2], n[addr::rhythm_note::LevelEnvTime3], n[addr::rhythm_note::LevelEnvTime4] };
    int lTva[4] = { n[addr::rhythm_note::LevelEnvLevel1], n[addr::rhythm_note::LevelEnvLevel2], n[addr::rhythm_note::LevelEnvLevel3], 0 };
    if (ui::EnvelopeGraph("DrumTva", "Level Env", tTva, lTva, false, false, ImVec2(ew, 140.0f))) {
        for (int i = 0; i < 4; ++i) {
            sendNB(addr::rhythm_note::LevelEnvTime1 + static_cast<size_t>(i), static_cast<uint8_t>(tTva[i]));
            if (i < 3) sendNB(addr::rhythm_note::LevelEnvLevel1 + static_cast<size_t>(i), static_cast<uint8_t>(lTva[i]));
        }
    }

    ImGui::EndChild();
}

} // namespace jv1010
