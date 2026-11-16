#include "PatchEditor.h"
#include "../ImGuiWidgets.h"
#include "../../Database/FactoryPresetsDB.h"
#include "../../Database/WaveformsDB.h"
#include "../../Librarian/PatchMorphRandom.h"
#include <imgui.h>
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace jv1010 {

PatchEditor::PatchEditor(JV1010EditorAudioProcessor& processor)
    : processor_(processor) {}

void PatchEditor::render() {
    uint32_t patchBase = (targetPart_ < 0) ? addr::kPatchModeTempPatchBase : addr::getPerfTempPatchBase(targetPart_);
    auto patch = (targetPart_ < 0) ? processor_.getSynthState().getPatchModeTempPatch()
                                   : processor_.getSynthState().getPerfTempPatch(targetPart_);

    auto sendToneByte = [&](size_t linearOffset, uint8_t value) {
        if (multiToneLink_) {
            for (int t = 0; t < 4; ++t) {
                if (patch.tones[static_cast<size_t>(t)][addr::patch_tone::ToneSwitch] != 0 || t == activeTone_) {
                    uint32_t addr32 = sysex::addAddressOffset(patchBase | addr::getPatchToneOffset(t), static_cast<uint32_t>(linearOffset));
                    processor_.sendParameterDT1(addr32, value);
                }
            }
        } else {
            uint32_t addr32 = sysex::addAddressOffset(patchBase | addr::getPatchToneOffset(activeTone_), static_cast<uint32_t>(linearOffset));
            processor_.sendParameterDT1(addr32, value);
        }
    };

    // ========================================================================
    // TOP HEADER BAR: Target, Name, Category, Tempo, Clock, Tone 1..4 Selector
    // ========================================================================
    ImGui::SetNextItemWidth(130.0f);
    std::string tgtLbl = (targetPart_ < 0) ? "Patch Mode Temp" : ("Perf Part " + std::to_string(targetPart_ + 1));
    if (ImGui::BeginCombo("##PatchTarget", tgtLbl.c_str())) {
        if (ImGui::Selectable("Patch Mode Temp", targetPart_ < 0)) targetPart_ = -1;
        for (int p = 0; p < 16; ++p) {
            if (p == 9) continue;
            std::string l = "Perf Part " + std::to_string(p + 1);
            if (ImGui::Selectable(l.c_str(), targetPart_ == p)) targetPart_ = p;
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    char nameBuf[16]{};
    std::strncpy(nameBuf, patch.getName().c_str(), 12);
    ImGui::SetNextItemWidth(110.0f);
    if (ImGui::InputText("##PName", nameBuf, 13)) {
        auto enc = sysex::encodeAsciiName12(nameBuf);
        processor_.getSynthState().applyDT1(patchBase, enc);
        processor_.getMidiManager().sendSysEx(sysex::buildDT1(processor_.getDeviceId(), patchBase, enc));
    }

    ImGui::SameLine();
    int catIdx = std::clamp<int>(patch.common[addr::patch_common::PatchCategory], 0, 38);
    const auto& cats = getPatchCategories();
    ImGui::SetNextItemWidth(130.0f);
    if (ImGui::BeginCombo("##PCat", (std::string(cats[static_cast<size_t>(catIdx)].shortCode) + " " + cats[static_cast<size_t>(catIdx)].name).c_str())) {
        for (const auto& c : cats) {
            std::string l = std::string(c.shortCode) + " " + c.name;
            if (ImGui::Selectable(l.c_str(), catIdx == c.id)) {
                processor_.sendParameterDT1(patchBase | addr::patch_common::PatchCategory, static_cast<uint8_t>(c.id));
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    int pTempo = sysex::decodeNibbled2(patch.common[addr::patch_common::TempoNibbled],
                                       patch.common[addr::patch_common::TempoNibbled + 1]);
    ImGui::SetNextItemWidth(72.0f);
    if (ImGui::DragInt("##PTempo", &pTempo, 0.5f, 20, 250, "%d BPM")) {
        processor_.sendParameterNibbled2DT1(patchBase | addr::patch_common::TempoNibbled, pTempo);
    }

    ImGui::SameLine();
    int clkSrc = patch.common[addr::patch_common::ClockSource];
    const char* clkNames[] = { "Clk:PATCH", "Clk:SYS" };
    ImGui::SetNextItemWidth(82.0f);
    if (ImGui::Combo("##ClkSrc", &clkSrc, clkNames, 2)) {
        processor_.sendParameterDT1(patchBase | addr::patch_common::ClockSource, static_cast<uint8_t>(clkSrc));
    }

    ImGui::SameLine(0.0f, 12.0f);
    for (int t = 0; t < 4; ++t) {
        if (t > 0) ImGui::SameLine();
        ImGui::PushID(t);
        bool on = patch.tones[static_cast<size_t>(t)][addr::patch_tone::ToneSwitch] != 0;
        if (ui::ToggleSwitch("##On", &on)) {
            processor_.sendParameterDT1(patchBase | addr::getPatchToneOffset(t) | addr::patch_tone::ToneSwitch, on ? 1 : 0);
        }
        ImGui::SameLine();
        char tLbl[16];
        std::snprintf(tLbl, sizeof(tLbl), "T%d", t + 1);
        if (ui::RolandButton(tLbl, activeTone_ == t, ImVec2(36.0f, 26.0f), 0)) activeTone_ = t;
        ImGui::PopID();
    }

    ImGui::SameLine();
    ui::ToggleSwitch("Link", &multiToneLink_);
    ImGui::SameLine();
    char cpLbl[24];
    std::snprintf(cpLbl, sizeof(cpLbl), "Cp T%d->T%d", activeTone_ + 1, ((activeTone_ + 1) % 4) + 1);
    if (ui::RolandButton(cpLbl, false, ImVec2(0.0f, 26.0f), 1)) {
        int dstT = (activeTone_ + 1) % 4;
        PatchTools::copyTone(patch, dstT, patch, activeTone_);
        if (targetPart_ < 0) processor_.getSynthState().setPatchModeTempPatch(patch);
        else processor_.getSynthState().setPerfTempPatch(targetPart_, patch);
        processor_.getMidiManager().sendSysExQueue(
            processor_.getSynthState().serializePatchToDT1(processor_.getDeviceId(), patchBase, patch));
    }

    ImGui::Separator();

    // ========================================================================
    // MAIN 2-COLUMN SPLIT: LEFT (Tone + Matrix + 3 Envelopes) | RIGHT (Common + LFO)
    // ========================================================================
    float totalW = ImGui::GetContentRegionAvail().x;
    float rightW = std::clamp(totalW * 0.30f, 295.0f, 390.0f);
    float leftW = std::max(420.0f, totalW - rightW - 8.0f);

    const auto& tn = patch.tones[static_cast<size_t>(activeTone_)];

    // ------------------------------------------------------------------------
    // LEFT COLUMN: ACTIVE TONE (Wave, FXM, Delay, Ranges, Matrix, TVA, TVF, Pitch)
    // ------------------------------------------------------------------------
    ImGui::BeginChild("##PatchLeftPane", ImVec2(leftW, 0.0f), true);
    {
        char toneHdr[48];
        std::snprintf(toneHdr, sizeof(toneHdr), "TONE %d  |  WAVE, RANGE, OUTPUT & FXM", activeTone_ + 1);
        ui::SectionHeader(toneHdr, IM_COL32(36, 98, 178, 255));

        int wgType = tn[addr::patch_tone::WaveGroupType];
        int wgId = tn[addr::patch_tone::WaveGroupID];
        int wgSel = (wgType == 0 && wgId == 2) ? 1 : (wgType == 2 && wgId == 9 ? 2 : (wgType == 2 ? 3 : 0));
        const char* wgNames[] = { "INT-A", "INT-B", "XP-A (Session)", "XP-B (EXP)" };
        ImGui::SetNextItemWidth(115.0f);
        if (ImGui::Combo("WaveGrp", &wgSel, wgNames, 4)) {
            sendToneByte(addr::patch_tone::WaveGroupType, wgSel <= 1 ? 0 : 2);
            sendToneByte(addr::patch_tone::WaveGroupID, wgSel == 0 ? 1 : (wgSel == 1 ? 2 : (wgSel == 2 ? 9 : 1)));
        }
        ImGui::SameLine();
        int wNum = sysex::decodeNibbled2(tn[addr::patch_tone::WaveNumberNibbled], tn[addr::patch_tone::WaveNumberNibbled + 1]) + 1;
        int maxW = getWaveformCount(static_cast<WaveGroup>(wgSel));
        std::string curWaveStr = std::to_string(wNum) + " " + getWaveformName(static_cast<WaveGroup>(wgSel), wNum);
        ImGui::SetNextItemWidth(185.0f);
        if (ImGui::BeginCombo("Wave", curWaveStr.c_str())) {
            for (int i = 1; i <= maxW; ++i) {
                std::string wl = std::to_string(i) + " " + getWaveformName(static_cast<WaveGroup>(wgSel), i);
                if (ImGui::Selectable(wl.c_str(), wNum == i)) {
                    auto nib = sysex::encodeNibbled2(i - 1);
                    sendToneByte(addr::patch_tone::WaveNumberNibbled, nib.first);
                    sendToneByte(addr::patch_tone::WaveNumberNibbled + 1, nib.second);
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        int wGain = tn[addr::patch_tone::WaveGain];
        const char* gains[] = { "-6 dB", "0 dB", "+6 dB", "+12 dB" };
        ImGui::SetNextItemWidth(72.0f);
        if (ImGui::Combo("Gain", &wGain, gains, 4)) sendToneByte(addr::patch_tone::WaveGain, static_cast<uint8_t>(wGain));

        ImGui::SameLine();
        int outA = tn[addr::patch_tone::OutputAssign];
        const char* outs[] = { "MIX", "EFX", "DIR1", "DIR2" };
        ImGui::SetNextItemWidth(68.0f);
        if (ImGui::Combo("Out", &outA, outs, 4)) sendToneByte(addr::patch_tone::OutputAssign, static_cast<uint8_t>(outA));

        // Row 2: Switches + Ranges (with musical note formatting!) + Delay + FXM
        bool red = tn[addr::patch_tone::RedamperSwitch] != 0, vol = tn[addr::patch_tone::VolumeSwitch] != 0;
        bool hld = tn[addr::patch_tone::Hold1Switch] != 0, bnd = tn[addr::patch_tone::PitchBendSwitch] != 0;
        bool fxmSw = tn[addr::patch_tone::FXMSwitch] != 0;
        if (ui::ToggleSwitch("Vol", &vol)) sendToneByte(addr::patch_tone::VolumeSwitch, vol ? 1 : 0); ImGui::SameLine();
        if (ui::ToggleSwitch("Bend", &bnd)) sendToneByte(addr::patch_tone::PitchBendSwitch, bnd ? 1 : 0); ImGui::SameLine();
        if (ui::ToggleSwitch("Hold1", &hld)) sendToneByte(addr::patch_tone::Hold1Switch, hld ? 1 : 0); ImGui::SameLine();
        if (ui::ToggleSwitch("ReDmp", &red)) sendToneByte(addr::patch_tone::RedamperSwitch, red ? 1 : 0); ImGui::SameLine();
        if (ui::ToggleSwitch("FXM", &fxmSw)) sendToneByte(addr::patch_tone::FXMSwitch, fxmSw ? 1 : 0); ImGui::SameLine();

        int fxmCol = tn[addr::patch_tone::FXMColor] + 1;
        if (ui::FaderH("Col", &fxmCol, 1, 4, 56.0f, "%d", 1)) sendToneByte(addr::patch_tone::FXMColor, static_cast<uint8_t>(fxmCol - 1));
        ImGui::SameLine();
        int fxmDep = tn[addr::patch_tone::FXMDepth] + 1;
        if (ui::FaderH("Dep", &fxmDep, 1, 16, 62.0f, "%d", 1)) sendToneByte(addr::patch_tone::FXMDepth, static_cast<uint8_t>(fxmDep - 1));
        ImGui::SameLine();
        int dMode = tn[addr::patch_tone::ToneDelayMode];
        const char* dModes[] = { "NORMAL", "HOLD", "PLAYMATE", "CLK-SYNC", "TAP-SYNC", "KEY-OFF-N", "KEY-OFF-D", "TMP-SYNC" };
        ImGui::SetNextItemWidth(95.0f);
        if (ImGui::Combo("##DlyM", &dMode, dModes, 8)) sendToneByte(addr::patch_tone::ToneDelayMode, static_cast<uint8_t>(dMode));
        ImGui::SameLine();
        int dlyT = tn[addr::patch_tone::ToneDelayTime];
        if (ui::FaderH("DlyT", &dlyT, 0, 127, 68.0f, "%d", 0)) sendToneByte(addr::patch_tone::ToneDelayTime, static_cast<uint8_t>(dlyT));

        // Row 3: Key Range (Notes), Vel Range, Pan & Sends
        int kLo = tn[addr::patch_tone::KeyboardRangeLower], kHi = tn[addr::patch_tone::KeyboardRangeUpper];
        char kLoBuf[16], kHiBuf[16];
        ui::formatMidiNote(kLo, kLoBuf, sizeof(kLoBuf));
        ui::formatMidiNote(kHi, kHiBuf, sizeof(kHiBuf));
        if (ui::FaderH("KeyLo", &kLo, 0, 127, 72.0f, kLoBuf, 0)) sendToneByte(addr::patch_tone::KeyboardRangeLower, static_cast<uint8_t>(kLo));
        ImGui::SameLine();
        if (ui::FaderH("KeyHi", &kHi, 0, 127, 72.0f, kHiBuf, 127)) sendToneByte(addr::patch_tone::KeyboardRangeUpper, static_cast<uint8_t>(kHi));
        ImGui::SameLine();
        int vLo = tn[addr::patch_tone::VelocityRangeLower], vHi = tn[addr::patch_tone::VelocityRangeUpper], xFade = tn[addr::patch_tone::VelocityCrossFade];
        if (ui::FaderH("VelLo", &vLo, 1, 127, 64.0f, "%d", 1)) sendToneByte(addr::patch_tone::VelocityRangeLower, static_cast<uint8_t>(vLo));
        ImGui::SameLine();
        if (ui::FaderH("VelHi", &vHi, 1, 127, 64.0f, "%d", 127)) sendToneByte(addr::patch_tone::VelocityRangeUpper, static_cast<uint8_t>(vHi));
        ImGui::SameLine();
        if (ui::FaderH("XFade", &xFade, 0, 127, 64.0f, "%d", 0)) sendToneByte(addr::patch_tone::VelocityCrossFade, static_cast<uint8_t>(xFade));
        ImGui::SameLine();
        int pCtrl = tn[addr::patch_tone::PanControlSwitch];
        const char* pCtrls[] = { "Pan:OFF", "Pan:CONT", "Pan:KEY" };
        ImGui::SetNextItemWidth(82.0f);
        if (ImGui::Combo("##PanCtrl", &pCtrl, pCtrls, 3)) sendToneByte(addr::patch_tone::PanControlSwitch, static_cast<uint8_t>(pCtrl));

        // Knobs: Pan, PanKF, RndPan, AltPan, MixEFX, Chorus, Reverb
        int pan = tn[addr::patch_tone::TonePan], pkf = tn[addr::patch_tone::PanKeyfollow];
        int rndP = tn[addr::patch_tone::RandomPanDepth], altP = tn[addr::patch_tone::AlternatePanDepth];
        int mix = tn[addr::patch_tone::MixEFXSendLevel], cho = tn[addr::patch_tone::ChorusSendLevel], rev = tn[addr::patch_tone::ReverbSendLevel];
        if (ui::KnobInt("Pan", &pan, 0, 127, "%d", 14.0f, 64)) sendToneByte(addr::patch_tone::TonePan, static_cast<uint8_t>(pan));
        ImGui::SameLine(); if (ui::KnobInt("PanKF", &pkf, 0, 14, "%d", 14.0f, 7)) sendToneByte(addr::patch_tone::PanKeyfollow, static_cast<uint8_t>(pkf));
        ImGui::SameLine(); if (ui::KnobInt("RndPan", &rndP, 0, 63, "%d", 14.0f, 0)) sendToneByte(addr::patch_tone::RandomPanDepth, static_cast<uint8_t>(rndP));
        ImGui::SameLine(); if (ui::KnobInt("AltPan", &altP, 1, 127, "%d", 14.0f, 64)) sendToneByte(addr::patch_tone::AlternatePanDepth, static_cast<uint8_t>(altP));
        ImGui::SameLine(); if (ui::KnobInt("MixEFX", &mix, 0, 127, "%d", 14.0f, 127)) sendToneByte(addr::patch_tone::MixEFXSendLevel, static_cast<uint8_t>(mix));
        ImGui::SameLine(); if (ui::KnobInt("ChoSnd", &cho, 0, 127, "%d", 14.0f, 0)) sendToneByte(addr::patch_tone::ChorusSendLevel, static_cast<uint8_t>(cho));
        ImGui::SameLine(); if (ui::KnobInt("RevSnd", &rev, 0, 127, "%d", 14.0f, 40)) sendToneByte(addr::patch_tone::ReverbSendLevel, static_cast<uint8_t>(rev));

        // --------------------------------------------------------------------
        // CONTROL MATRIX (12-column horizontal grid: C1-1..C3-4)
        // --------------------------------------------------------------------
        ImGui::Spacing();
        ui::SectionHeader("CONTROL MATRIX (C1 / C2 / C3)", IM_COL32(195, 118, 24, 255));
        const char* ctrlSources[] = {
            "OFF", "SYS-CTRL1", "SYS-CTRL2", "MODULATION", "BREATH", "FOOT", "VOLUME",
            "PAN", "EXPRESSION", "PITCH BEND", "AFTERTOUCH", "LFO1", "LFO2", "VELOCITY", "KEYFOLLOW", "PLAYMATE"
        };
        int cs2 = patch.common[addr::patch_common::PatchControlSource2];
        int cs3 = patch.common[addr::patch_common::PatchControlSource3];
        ImGui::SetNextItemWidth(115.0f);
        if (ImGui::Combo("Ctrl 2 Src", &cs2, ctrlSources, 16)) processor_.sendParameterDT1(patchBase | addr::patch_common::PatchControlSource2, static_cast<uint8_t>(cs2));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(115.0f);
        if (ImGui::Combo("Ctrl 3 Src", &cs3, ctrlSources, 16)) processor_.sendParameterDT1(patchBase | addr::patch_common::PatchControlSource3, static_cast<uint8_t>(cs3));

        const char* dests[] = { "OFF", "PCH", "CUT", "RES", "LEV", "PAN", "MIX", "CHO", "REV", "PL1", "PL2", "FL1", "FL2", "AL1", "AL2", "pL1", "pL2", "L1R", "L2R" };
        if (ImGui::BeginTable("##CtrlMatrixGrid", 12, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchSame)) {
            for (int i = 0; i < 12; ++i) {
                char hdr[8];
                std::snprintf(hdr, sizeof(hdr), "C%d-%d", (i / 4) + 1, (i % 4) + 1);
                ImGui::TableSetupColumn(hdr);
            }
            ImGui::TableHeadersRow();
            ImGui::TableNextRow();
            for (int i = 0; i < 12; ++i) {
                ImGui::TableSetColumnIndex(i);
                ImGui::PushID(500 + i);
                size_t off = addr::patch_tone::Ctrl1Dest1 + static_cast<size_t>(i * 2);
                int dst = tn[off], dep = static_cast<int>(tn[off + 1]) - 63;
                ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::Combo("##D", &dst, dests, 19)) sendToneByte(off, static_cast<uint8_t>(dst));
                ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::DragInt("##V", &dep, 0.5f, -63, 63, "%+d")) sendToneByte(off + 1, static_cast<uint8_t>(dep + 63));
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        // --------------------------------------------------------------------
        // MODULE 1: AMPLIFIER (TVA & BIAS)
        // --------------------------------------------------------------------
        ImGui::Spacing();
        ui::SectionHeader("AMPLIFIER (TVA & BIAS)", IM_COL32(198, 42, 42, 255));
        ImGui::PushID("TVA");
        {
            float envW = std::max(180.0f, ImGui::GetContentRegionAvail().x - 375.0f);
            ImGui::BeginGroup();
            int lvl = tn[addr::patch_tone::ToneLevel];
            int bPos = tn[addr::patch_tone::BiasPosition], bLvl = tn[addr::patch_tone::BiasLevel], bDir = tn[addr::patch_tone::BiasDirection];
            int vs = tn[addr::patch_tone::LevelEnvVelSens], vt1 = tn[addr::patch_tone::LevelEnvVelTime1], vt4 = tn[addr::patch_tone::LevelEnvVelTime4], tkf = tn[addr::patch_tone::LevelEnvTimeKeyfollow];
            const char* bDirs[] = { "LOWER", "UPPER", "L&U", "ALL" };
            char bPosBuf[16];
            ui::formatMidiNote(bPos, bPosBuf, sizeof(bPosBuf));

            if (ui::FaderH("Level", &lvl, 0, 127, 86.0f, "%d", 127)) sendToneByte(addr::patch_tone::ToneLevel, static_cast<uint8_t>(lvl));
            if (ui::FaderH("Bias Pos", &bPos, 0, 127, 86.0f, bPosBuf, 60)) sendToneByte(addr::patch_tone::BiasPosition, static_cast<uint8_t>(bPos));
            if (ui::FaderH("Bias Lvl", &bLvl, 0, 14, 86.0f, "%d", 7)) sendToneByte(addr::patch_tone::BiasLevel, static_cast<uint8_t>(bLvl));
            ImGui::SetNextItemWidth(122.0f); if (ImGui::Combo("Bias Dir", &bDir, bDirs, 4)) sendToneByte(addr::patch_tone::BiasDirection, static_cast<uint8_t>(bDir));
            if (ui::FaderH("Vel Sens", &vs, 0, 125, 86.0f, "%d", 50)) sendToneByte(addr::patch_tone::LevelEnvVelSens, static_cast<uint8_t>(vs));
            if (ui::FaderH("V-On Time", &vt1, 0, 125, 86.0f, "%d", 50)) sendToneByte(addr::patch_tone::LevelEnvVelTime1, static_cast<uint8_t>(vt1));
            if (ui::FaderH("V-Off Time", &vt4, 0, 125, 86.0f, "%d", 50)) sendToneByte(addr::patch_tone::LevelEnvVelTime4, static_cast<uint8_t>(vt4));
            if (ui::FaderH("Time KF", &tkf, 0, 14, 86.0f, "%d", 7)) sendToneByte(addr::patch_tone::LevelEnvTimeKeyfollow, static_cast<uint8_t>(tkf));
            ImGui::EndGroup();

            ImGui::SameLine();
            int tArr[4] = { tn[addr::patch_tone::LevelEnvTime1], tn[addr::patch_tone::LevelEnvTime2], tn[addr::patch_tone::LevelEnvTime3], tn[addr::patch_tone::LevelEnvTime4] };
            int lArr[4] = { tn[addr::patch_tone::LevelEnvLevel1], tn[addr::patch_tone::LevelEnvLevel2], tn[addr::patch_tone::LevelEnvLevel3], 0 };
            if (ui::EnvelopeGraph("TvaEnv", "TVA Level Envelope", tArr, lArr, false, false, ImVec2(envW, 175.0f))) {
                for (int i = 0; i < 4; ++i) {
                    sendToneByte(addr::patch_tone::LevelEnvTime1 + static_cast<size_t>(i), static_cast<uint8_t>(tArr[i]));
                    if (i < 3) sendToneByte(addr::patch_tone::LevelEnvLevel1 + static_cast<size_t>(i), static_cast<uint8_t>(lArr[i]));
                }
            }

            ImGui::SameLine();
            ImGui::BeginGroup();
            int vc = tn[addr::patch_tone::LevelEnvVelCurve] + 1;
            if (ui::VelocityCurvePreview("TvaVC", &vc, ImVec2(145.0f, 52.0f))) {
                sendToneByte(addr::patch_tone::LevelEnvVelCurve, static_cast<uint8_t>(vc - 1));
            }
            if (ui::KnobInt("T1", &tArr[0], 0, 127, "%d", 13.0f, 0)) sendToneByte(addr::patch_tone::LevelEnvTime1, static_cast<uint8_t>(tArr[0]));
            ImGui::SameLine(); if (ui::KnobInt("T2", &tArr[1], 0, 127, "%d", 13.0f, 0)) sendToneByte(addr::patch_tone::LevelEnvTime2, static_cast<uint8_t>(tArr[1]));
            ImGui::SameLine(); if (ui::KnobInt("T3", &tArr[2], 0, 127, "%d", 13.0f, 0)) sendToneByte(addr::patch_tone::LevelEnvTime3, static_cast<uint8_t>(tArr[2]));
            ImGui::SameLine(); if (ui::KnobInt("T4", &tArr[3], 0, 127, "%d", 13.0f, 20)) sendToneByte(addr::patch_tone::LevelEnvTime4, static_cast<uint8_t>(tArr[3]));
            if (ui::KnobInt("L1", &lArr[0], 0, 127, "%d", 13.0f, 127)) sendToneByte(addr::patch_tone::LevelEnvLevel1, static_cast<uint8_t>(lArr[0]));
            ImGui::SameLine(); if (ui::KnobInt("L2", &lArr[1], 0, 127, "%d", 13.0f, 127)) sendToneByte(addr::patch_tone::LevelEnvLevel2, static_cast<uint8_t>(lArr[1]));
            ImGui::SameLine(); if (ui::KnobInt("L3", &lArr[2], 0, 127, "%d", 13.0f, 127)) sendToneByte(addr::patch_tone::LevelEnvLevel3, static_cast<uint8_t>(lArr[2]));
            ImGui::EndGroup();
        }
        ImGui::PopID();

        // --------------------------------------------------------------------
        // MODULE 2: FILTER (TVF)
        // --------------------------------------------------------------------
        ImGui::Spacing();
        ui::SectionHeader("FILTER (TVF)", IM_COL32(36, 98, 178, 255));
        ImGui::PushID("TVF");
        {
            float envW = std::max(180.0f, ImGui::GetContentRegionAvail().x - 375.0f);
            ImGui::BeginGroup();
            int fType = tn[addr::patch_tone::FilterType];
            const char* fTypes[] = { "OFF", "LPF", "BPF", "HPF", "PKG" };
            int cut = tn[addr::patch_tone::CutoffFrequency], res = tn[addr::patch_tone::Resonance], rVs = tn[addr::patch_tone::ResonanceVelSens];
            int kf = tn[addr::patch_tone::CutoffKeyfollow], dep = static_cast<int>(tn[addr::patch_tone::FilterEnvDepth]) - 63;
            int vel = tn[addr::patch_tone::FilterEnvVelSens], vt1 = tn[addr::patch_tone::FilterEnvVelTime1], vt4 = tn[addr::patch_tone::FilterEnvVelTime4], tkf = tn[addr::patch_tone::FilterEnvTimeKeyfollow];

            ImGui::SetNextItemWidth(122.0f); if (ImGui::Combo("Filter", &fType, fTypes, 5)) sendToneByte(addr::patch_tone::FilterType, static_cast<uint8_t>(fType));
            if (ui::FaderH("Cutoff", &cut, 0, 127, 86.0f, "%d", 127)) sendToneByte(addr::patch_tone::CutoffFrequency, static_cast<uint8_t>(cut));
            if (ui::FaderH("Resonance", &res, 0, 127, 86.0f, "%d", 0)) sendToneByte(addr::patch_tone::Resonance, static_cast<uint8_t>(res));
            if (ui::FaderH("Reso Vel", &rVs, 0, 125, 86.0f, "%d", 50)) sendToneByte(addr::patch_tone::ResonanceVelSens, static_cast<uint8_t>(rVs));
            if (ui::FaderH("Cutoff KF", &kf, 0, 15, 86.0f, "%d", 5)) sendToneByte(addr::patch_tone::CutoffKeyfollow, static_cast<uint8_t>(kf));
            if (ui::FaderH("Env Depth", &dep, -63, 63, 86.0f, "%+d", 0)) sendToneByte(addr::patch_tone::FilterEnvDepth, static_cast<uint8_t>(dep + 63));
            if (ui::FaderH("Vel Sens", &vel, 0, 125, 86.0f, "%d", 50)) sendToneByte(addr::patch_tone::FilterEnvVelSens, static_cast<uint8_t>(vel));
            if (ui::FaderH("V-On Time", &vt1, 0, 125, 86.0f, "%d", 50)) sendToneByte(addr::patch_tone::FilterEnvVelTime1, static_cast<uint8_t>(vt1));
            if (ui::FaderH("V-Off Time", &vt4, 0, 125, 86.0f, "%d", 50)) sendToneByte(addr::patch_tone::FilterEnvVelTime4, static_cast<uint8_t>(vt4));
            if (ui::FaderH("Time KF##TvfKF", &tkf, 0, 14, 86.0f, "%d", 7)) sendToneByte(addr::patch_tone::FilterEnvTimeKeyfollow, static_cast<uint8_t>(tkf));
            ImGui::EndGroup();

            ImGui::SameLine();
            int tArr[4] = { tn[addr::patch_tone::FilterEnvTime1], tn[addr::patch_tone::FilterEnvTime2], tn[addr::patch_tone::FilterEnvTime3], tn[addr::patch_tone::FilterEnvTime4] };
            int lArr[4] = { tn[addr::patch_tone::FilterEnvLevel1], tn[addr::patch_tone::FilterEnvLevel2], tn[addr::patch_tone::FilterEnvLevel3], tn[addr::patch_tone::FilterEnvLevel4] };
            if (ui::EnvelopeGraph("TvfEnv", "TVF Filter Envelope", tArr, lArr, false, true, ImVec2(envW, 195.0f))) {
                for (int i = 0; i < 4; ++i) {
                    sendToneByte(addr::patch_tone::FilterEnvTime1 + static_cast<size_t>(i), static_cast<uint8_t>(tArr[i]));
                    sendToneByte(addr::patch_tone::FilterEnvLevel1 + static_cast<size_t>(i), static_cast<uint8_t>(lArr[i]));
                }
            }

            ImGui::SameLine();
            ImGui::BeginGroup();
            int vc = tn[addr::patch_tone::FilterEnvVelCurve] + 1;
            if (ui::VelocityCurvePreview("TvfVC", &vc, ImVec2(145.0f, 52.0f))) {
                sendToneByte(addr::patch_tone::FilterEnvVelCurve, static_cast<uint8_t>(vc - 1));
            }
            if (ui::KnobInt("T1", &tArr[0], 0, 127, "%d", 13.0f, 0)) sendToneByte(addr::patch_tone::FilterEnvTime1, static_cast<uint8_t>(tArr[0]));
            ImGui::SameLine(); if (ui::KnobInt("T2", &tArr[1], 0, 127, "%d", 13.0f, 0)) sendToneByte(addr::patch_tone::FilterEnvTime2, static_cast<uint8_t>(tArr[1]));
            ImGui::SameLine(); if (ui::KnobInt("T3", &tArr[2], 0, 127, "%d", 13.0f, 0)) sendToneByte(addr::patch_tone::FilterEnvTime3, static_cast<uint8_t>(tArr[2]));
            ImGui::SameLine(); if (ui::KnobInt("T4", &tArr[3], 0, 127, "%d", 13.0f, 0)) sendToneByte(addr::patch_tone::FilterEnvTime4, static_cast<uint8_t>(tArr[3]));
            if (ui::KnobInt("L1", &lArr[0], 0, 127, "%d", 13.0f, 0)) sendToneByte(addr::patch_tone::FilterEnvLevel1, static_cast<uint8_t>(lArr[0]));
            ImGui::SameLine(); if (ui::KnobInt("L2", &lArr[1], 0, 127, "%d", 13.0f, 0)) sendToneByte(addr::patch_tone::FilterEnvLevel2, static_cast<uint8_t>(lArr[1]));
            ImGui::SameLine(); if (ui::KnobInt("L3", &lArr[2], 0, 127, "%d", 13.0f, 0)) sendToneByte(addr::patch_tone::FilterEnvLevel3, static_cast<uint8_t>(lArr[2]));
            ImGui::SameLine(); if (ui::KnobInt("L4", &lArr[3], 0, 127, "%d", 13.0f, 0)) sendToneByte(addr::patch_tone::FilterEnvLevel4, static_cast<uint8_t>(lArr[3]));
            ImGui::EndGroup();
        }
        ImGui::PopID();

        // --------------------------------------------------------------------
        // MODULE 3: PITCH & ENVELOPE
        // --------------------------------------------------------------------
        ImGui::Spacing();
        ui::SectionHeader("PITCH & ENVELOPE", IM_COL32(32, 138, 86, 255));
        ImGui::PushID("PITCH");
        {
            float envW = std::max(180.0f, ImGui::GetContentRegionAvail().x - 375.0f);
            ImGui::BeginGroup();
            int coarse = static_cast<int>(tn[addr::patch_tone::CoarseTune]) - 48;
            int fine = static_cast<int>(tn[addr::patch_tone::FineTune]) - 50;
            int rnd = tn[addr::patch_tone::RandomPitchDepth], kf = tn[addr::patch_tone::PitchKeyfollow];
            int dep = static_cast<int>(tn[addr::patch_tone::PitchEnvDepth]) - 12, vs = tn[addr::patch_tone::PitchEnvVelSens];
            int vt1 = tn[addr::patch_tone::PitchEnvVelTime1], vt4 = tn[addr::patch_tone::PitchEnvVelTime4], tkf = tn[addr::patch_tone::PitchEnvTimeKeyfollow];

            if (ui::FaderH("Coarse", &coarse, -48, 48, 86.0f, "%+d", 0)) sendToneByte(addr::patch_tone::CoarseTune, static_cast<uint8_t>(coarse + 48));
            if (ui::FaderH("Fine", &fine, -50, 50, 86.0f, "%+d", 0)) sendToneByte(addr::patch_tone::FineTune, static_cast<uint8_t>(fine + 50));
            if (ui::FaderH("Rnd Pitch", &rnd, 0, 30, 86.0f, "%d", 0)) sendToneByte(addr::patch_tone::RandomPitchDepth, static_cast<uint8_t>(rnd));
            if (ui::FaderH("Pitch KF", &kf, 0, 15, 86.0f, "%d", 10)) sendToneByte(addr::patch_tone::PitchKeyfollow, static_cast<uint8_t>(kf));
            if (ui::FaderH("Env Depth", &dep, -12, 12, 86.0f, "%+d", 0)) sendToneByte(addr::patch_tone::PitchEnvDepth, static_cast<uint8_t>(dep + 12));
            if (ui::FaderH("Vel Sens", &vs, 0, 125, 86.0f, "%d", 50)) sendToneByte(addr::patch_tone::PitchEnvVelSens, static_cast<uint8_t>(vs));
            if (ui::FaderH("V-On Time", &vt1, 0, 125, 86.0f, "%d", 50)) sendToneByte(addr::patch_tone::PitchEnvVelTime1, static_cast<uint8_t>(vt1));
            if (ui::FaderH("V-Off Time", &vt4, 0, 125, 86.0f, "%d", 50)) sendToneByte(addr::patch_tone::PitchEnvVelTime4, static_cast<uint8_t>(vt4));
            if (ui::FaderH("Time KF##PchKF", &tkf, 0, 14, 86.0f, "%d", 7)) sendToneByte(addr::patch_tone::PitchEnvTimeKeyfollow, static_cast<uint8_t>(tkf));
            ImGui::EndGroup();

            ImGui::SameLine();
            int tArr[4] = { tn[addr::patch_tone::PitchEnvTime1], tn[addr::patch_tone::PitchEnvTime2], tn[addr::patch_tone::PitchEnvTime3], tn[addr::patch_tone::PitchEnvTime4] };
            int lArr[4] = { tn[addr::patch_tone::PitchEnvLevel1], tn[addr::patch_tone::PitchEnvLevel2], tn[addr::patch_tone::PitchEnvLevel3], tn[addr::patch_tone::PitchEnvLevel4] };
            if (ui::EnvelopeGraph("PitchEnv", "Pitch Envelope (Bipolar)", tArr, lArr, true, true, ImVec2(envW, 180.0f))) {
                for (int i = 0; i < 4; ++i) {
                    sendToneByte(addr::patch_tone::PitchEnvTime1 + static_cast<size_t>(i), static_cast<uint8_t>(tArr[i]));
                    sendToneByte(addr::patch_tone::PitchEnvLevel1 + static_cast<size_t>(i), static_cast<uint8_t>(lArr[i]));
                }
            }

            ImGui::SameLine();
            ImGui::BeginGroup();
            if (ui::KnobInt("T1", &tArr[0], 0, 127, "%d", 13.0f, 0)) sendToneByte(addr::patch_tone::PitchEnvTime1, static_cast<uint8_t>(tArr[0]));
            ImGui::SameLine(); if (ui::KnobInt("T2", &tArr[1], 0, 127, "%d", 13.0f, 0)) sendToneByte(addr::patch_tone::PitchEnvTime2, static_cast<uint8_t>(tArr[1]));
            ImGui::SameLine(); if (ui::KnobInt("T3", &tArr[2], 0, 127, "%d", 13.0f, 0)) sendToneByte(addr::patch_tone::PitchEnvTime3, static_cast<uint8_t>(tArr[2]));
            ImGui::SameLine(); if (ui::KnobInt("T4", &tArr[3], 0, 127, "%d", 13.0f, 0)) sendToneByte(addr::patch_tone::PitchEnvTime4, static_cast<uint8_t>(tArr[3]));
            if (ui::KnobInt("L1", &lArr[0], 0, 126, "%d", 13.0f, 63)) sendToneByte(addr::patch_tone::PitchEnvLevel1, static_cast<uint8_t>(lArr[0]));
            ImGui::SameLine(); if (ui::KnobInt("L2", &lArr[1], 0, 126, "%d", 13.0f, 63)) sendToneByte(addr::patch_tone::PitchEnvLevel2, static_cast<uint8_t>(lArr[1]));
            ImGui::SameLine(); if (ui::KnobInt("L3", &lArr[2], 0, 126, "%d", 13.0f, 63)) sendToneByte(addr::patch_tone::PitchEnvLevel3, static_cast<uint8_t>(lArr[2]));
            ImGui::SameLine(); if (ui::KnobInt("L4", &lArr[3], 0, 126, "%d", 13.0f, 63)) sendToneByte(addr::patch_tone::PitchEnvLevel4, static_cast<uint8_t>(lArr[3]));
            ImGui::EndGroup();
        }
        ImGui::PopID();
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // ------------------------------------------------------------------------
    // RIGHT COLUMN: PATCH COMMON, PEAK+HOLD, STRUCTURES 1+2 & 3+4, LFO 1 & 2
    // ------------------------------------------------------------------------
    ImGui::BeginChild("##PatchRightPane", ImVec2(0.0f, 0.0f), true);
    {
        ui::SectionHeader("PATCH COMMON", IM_COL32(36, 98, 178, 255));

        int pLvl = patch.common[addr::patch_common::PatchLevel];
        int pPan = patch.common[addr::patch_common::PatchPan];
        if (ui::FaderV("Level", &pLvl, 0, 127, 102.0f, "%d", 127)) {
            processor_.sendParameterDT1(patchBase | addr::patch_common::PatchLevel, static_cast<uint8_t>(pLvl));
        }
        ImGui::SameLine();
        ImGui::BeginGroup();
        if (ui::KnobInt("Pan", &pPan, 0, 127, "%d", 16.0f, 64)) processor_.sendParameterDT1(patchBase | addr::patch_common::PatchPan, static_cast<uint8_t>(pPan));
        int bUp = patch.common[addr::patch_common::BendRangeUp], bDn = patch.common[addr::patch_common::BendRangeDown];
        if (ui::KnobInt("Bnd+", &bUp, 0, 12, "%d", 15.0f, 2)) processor_.sendParameterDT1(patchBase | addr::patch_common::BendRangeUp, static_cast<uint8_t>(bUp));
        ImGui::SameLine();
        if (ui::KnobInt("Bnd-", &bDn, 0, 48, "%d", 15.0f, 2)) processor_.sendParameterDT1(patchBase | addr::patch_common::BendRangeDown, static_cast<uint8_t>(bDn));
        int aFeel = patch.common[addr::patch_common::AnalogFeel], oct = static_cast<int>(patch.common[addr::patch_common::OctaveShift]) - 3;
        ImGui::SameLine();
        if (ui::KnobInt("Anlg", &aFeel, 0, 127, "%d", 15.0f, 0)) processor_.sendParameterDT1(patchBase | addr::patch_common::AnalogFeel, static_cast<uint8_t>(aFeel));
        ImGui::SameLine();
        if (ui::KnobInt("Oct", &oct, -3, 3, "%+d", 15.0f, 0)) processor_.sendParameterDT1(patchBase | addr::patch_common::OctaveShift, static_cast<uint8_t>(oct + 3));
        ImGui::EndGroup();

        // Assign / Voice Priority / Stretch / Switches
        int keyAssign = patch.common[addr::patch_common::KeyAssignMode];
        const char* ka[] = { "POLY", "SOLO" };
        ImGui::SetNextItemWidth(72.0f);
        if (ImGui::Combo("KeyAsgn", &keyAssign, ka, 2)) processor_.sendParameterDT1(patchBase | addr::patch_common::KeyAssignMode, static_cast<uint8_t>(keyAssign));
        ImGui::SameLine();
        int vPri = patch.common[addr::patch_common::VoicePriority];
        const char* vp[] = { "LAST", "LOUDEST" };
        ImGui::SetNextItemWidth(78.0f);
        if (ImGui::Combo("Priority", &vPri, vp, 2)) processor_.sendParameterDT1(patchBase | addr::patch_common::VoicePriority, static_cast<uint8_t>(vPri));

        int strT = patch.common[addr::patch_common::StretchTuneDepth];
        const char* stNames[] = { "OFF", "1", "2", "3" };
        ImGui::SetNextItemWidth(72.0f);
        if (ImGui::Combo("Stretch", &strT, stNames, 4)) processor_.sendParameterDT1(patchBase | addr::patch_common::StretchTuneDepth, static_cast<uint8_t>(strT));
        ImGui::SameLine();
        bool leg = patch.common[addr::patch_common::SoloLegato] != 0;
        if (ui::ToggleSwitch("Legato", &leg)) processor_.sendParameterDT1(patchBase | addr::patch_common::SoloLegato, leg ? 1 : 0);
        ImGui::SameLine();
        bool velSw = patch.common[addr::patch_common::VelocityRangeSwitch] != 0;
        if (ui::ToggleSwitch("VelSw", &velSw)) processor_.sendParameterDT1(patchBase | addr::patch_common::VelocityRangeSwitch, velSw ? 1 : 0);

        // Portamento (Complete)
        ui::SectionHeader("PORTAMENTO", IM_COL32(195, 118, 24, 255));
        bool pSw = patch.common[addr::patch_common::PortamentoSwitch] != 0;
        if (ui::ToggleSwitch("Porta", &pSw)) processor_.sendParameterDT1(patchBase | addr::patch_common::PortamentoSwitch, pSw ? 1 : 0);
        ImGui::SameLine();
        int pTim = patch.common[addr::patch_common::PortamentoTime];
        if (ui::FaderH("Time##Porta", &pTim, 0, 127, 92.0f, "%d", 0)) processor_.sendParameterDT1(patchBase | addr::patch_common::PortamentoTime, static_cast<uint8_t>(pTim));

        int pMode = patch.common[addr::patch_common::PortamentoMode];
        int pType = patch.common[addr::patch_common::PortamentoType];
        int pStart = patch.common[addr::patch_common::PortamentoStart];
        const char* pmNames[] = { "NORMAL", "LEGATO" };
        const char* ptNames[] = { "RATE", "TIME" };
        const char* psNames[] = { "PITCH", "NOTE" };
        ImGui::SetNextItemWidth(78.0f); if (ImGui::Combo("Mode", &pMode, pmNames, 2)) processor_.sendParameterDT1(patchBase | addr::patch_common::PortamentoMode, static_cast<uint8_t>(pMode));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(65.0f); if (ImGui::Combo("Type", &pType, ptNames, 2)) processor_.sendParameterDT1(patchBase | addr::patch_common::PortamentoType, static_cast<uint8_t>(pType));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(68.0f); if (ImGui::Combo("Start", &pStart, psNames, 2)) processor_.sendParameterDT1(patchBase | addr::patch_common::PortamentoStart, static_cast<uint8_t>(pStart));

        // Peak + Hold
        ui::SectionHeader("PEAK + HOLD", IM_COL32(198, 42, 42, 255));
        const char* hpNames[] = { "OFF", "HOLD", "PEAK", "PEAK+HOLD" };
        int hpE = patch.common[addr::patch_common::EFXControlHoldPeak];
        int hp1 = patch.common[addr::patch_common::Control1HoldPeak];
        int hp2 = patch.common[addr::patch_common::Control2HoldPeak];
        int hp3 = patch.common[addr::patch_common::Control3HoldPeak];
        ImGui::SetNextItemWidth(92.0f); if (ImGui::Combo("EFX Ctrl", &hpE, hpNames, 4)) processor_.sendParameterDT1(patchBase | addr::patch_common::EFXControlHoldPeak, static_cast<uint8_t>(hpE));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(92.0f); if (ImGui::Combo("Ctrl 1", &hp1, hpNames, 4)) processor_.sendParameterDT1(patchBase | addr::patch_common::Control1HoldPeak, static_cast<uint8_t>(hp1));
        ImGui::SetNextItemWidth(92.0f); if (ImGui::Combo("Ctrl 2", &hp2, hpNames, 4)) processor_.sendParameterDT1(patchBase | addr::patch_common::Control2HoldPeak, static_cast<uint8_t>(hp2));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(92.0f); if (ImGui::Combo("Ctrl 3", &hp3, hpNames, 4)) processor_.sendParameterDT1(patchBase | addr::patch_common::Control3HoldPeak, static_cast<uint8_t>(hp3));

        // Structures 1+2 & 3+4
        ui::SectionHeader("STRUCTURES 1+2 & 3+4", IM_COL32(36, 98, 178, 255));
        int s12 = patch.common[addr::patch_common::StructureType1_2], b12 = patch.common[addr::patch_common::Booster1_2];
        int s34 = patch.common[addr::patch_common::StructureType3_4], b34 = patch.common[addr::patch_common::Booster3_4];
        const char* boostNames[] = { "0dB", "+6dB", "+12dB", "+18dB" };

        int s12Disp = s12 + 1;
        if (ui::FaderH("S1+2", &s12Disp, 1, 10, 96.0f, "%d", 1)) processor_.sendParameterDT1(patchBase | addr::patch_common::StructureType1_2, static_cast<uint8_t>(s12Disp - 1));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(72.0f);
        if (ImGui::Combo("##B12", &b12, boostNames, 4)) processor_.sendParameterDT1(patchBase | addr::patch_common::Booster1_2, static_cast<uint8_t>(b12));
        ui::StructureDiagram("Tones 1 & 2", s12, b12, ImVec2(-1.0f, 76.0f));

        int s34Disp = s34 + 1;
        if (ui::FaderH("S3+4", &s34Disp, 1, 10, 96.0f, "%d", 1)) processor_.sendParameterDT1(patchBase | addr::patch_common::StructureType3_4, static_cast<uint8_t>(s34Disp - 1));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(72.0f);
        if (ImGui::Combo("##B34", &b34, boostNames, 4)) processor_.sendParameterDT1(patchBase | addr::patch_common::Booster3_4, static_cast<uint8_t>(b34));
        ui::StructureDiagram("Tones 3 & 4", s34, b34, ImVec2(-1.0f, 76.0f));

        // LFO 1 & LFO 2 Side-by-Side Comparison Table (MidiQuest style!)
        ImGui::Spacing();
        ui::SectionHeader("LFO 1 & LFO 2 (ACTIVE TONE)", IM_COL32(32, 138, 86, 255));
        const char* lfoWaves[] = { "TRI", "SIN", "SAW", "SQR", "TRP", "S&H", "RND", "CHS" };
        const char* lfoOffsets[] = { "-100", "-50", "0", "+50", "+100" };
        const char* lfoFades[] = { "ON-IN", "ON-OUT", "OFF-IN", "OFF-OUT" };
        const char* lfoSyncs[] = { "OFF", "CLOCK", "TAP" };

        if (ImGui::BeginTable("##LfoTable", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("Param", ImGuiTableColumnFlags_WidthFixed, 76.0f);
            ImGui::TableSetupColumn("LFO 1");
            ImGui::TableSetupColumn("LFO 2");
            ImGui::TableHeadersRow();

            auto lfoRowCombo = [&](const char* lbl, size_t off1, size_t off2, const char* const items[], int count) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(lbl);
                int v1 = tn[off1], v2 = tn[off2];
                ImGui::TableSetColumnIndex(1); ImGui::PushID(static_cast<int>(off1)); ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::Combo("##1", &v1, items, count)) sendToneByte(off1, static_cast<uint8_t>(v1));
                ImGui::PopID();
                ImGui::TableSetColumnIndex(2); ImGui::PushID(static_cast<int>(off2)); ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::Combo("##2", &v2, items, count)) sendToneByte(off2, static_cast<uint8_t>(v2));
                ImGui::PopID();
            };

            auto lfoRowInt = [&](const char* lbl, size_t off1, size_t off2, int minV, int maxV, int bias) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(lbl);
                int v1 = static_cast<int>(tn[off1]) - bias;
                int v2 = static_cast<int>(tn[off2]) - bias;
                const char* fmt = (bias > 0) ? "%+d" : "%d";
                ImGui::TableSetColumnIndex(1); ImGui::PushID(static_cast<int>(off1)); ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::DragInt("##1", &v1, 0.5f, minV, maxV, fmt)) sendToneByte(off1, static_cast<uint8_t>(v1 + bias));
                ImGui::PopID();
                ImGui::TableSetColumnIndex(2); ImGui::PushID(static_cast<int>(off2)); ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::DragInt("##2", &v2, 0.5f, minV, maxV, fmt)) sendToneByte(off2, static_cast<uint8_t>(v2 + bias));
                ImGui::PopID();
            };

            lfoRowCombo("Form", addr::patch_tone::LFO1Waveform, addr::patch_tone::LFO2Waveform, lfoWaves, 8);
            lfoRowCombo("Offset", addr::patch_tone::LFO1Offset, addr::patch_tone::LFO2Offset, lfoOffsets, 5);

            // Key Sync row
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Key Trig");
            bool ks1 = tn[addr::patch_tone::LFO1KeySync] != 0, ks2 = tn[addr::patch_tone::LFO2KeySync] != 0;
            ImGui::TableSetColumnIndex(1); if (ui::ToggleSwitch("##KS1", &ks1)) sendToneByte(addr::patch_tone::LFO1KeySync, ks1 ? 1 : 0);
            ImGui::TableSetColumnIndex(2); if (ui::ToggleSwitch("##KS2", &ks2)) sendToneByte(addr::patch_tone::LFO2KeySync, ks2 ? 1 : 0);

            lfoRowInt("Rate", addr::patch_tone::LFO1Rate, addr::patch_tone::LFO2Rate, 0, 127, 0);
            lfoRowInt("Delay", addr::patch_tone::LFO1DelayTime, addr::patch_tone::LFO2DelayTime, 0, 127, 0);
            lfoRowInt("Fade Time", addr::patch_tone::LFO1FadeTime, addr::patch_tone::LFO2FadeTime, 0, 127, 0);
            lfoRowCombo("Fade Mode", addr::patch_tone::LFO1FadeMode, addr::patch_tone::LFO2FadeMode, lfoFades, 4);
            lfoRowCombo("Extn Sync", addr::patch_tone::LFO1ExternalSync, addr::patch_tone::LFO2ExternalSync, lfoSyncs, 3);
            lfoRowInt("P-Depth", addr::patch_tone::PitchLFO1Depth, addr::patch_tone::PitchLFO2Depth, -63, 63, 63);
            lfoRowInt("TVF Depth", addr::patch_tone::FilterLFO1Depth, addr::patch_tone::FilterLFO2Depth, -63, 63, 63);
            lfoRowInt("Pan Depth", addr::patch_tone::PanLFO1Depth, addr::patch_tone::PanLFO2Depth, -63, 63, 63);
            lfoRowInt("TVA Depth", addr::patch_tone::LevelLFO1Depth, addr::patch_tone::LevelLFO2Depth, -63, 63, 63);

            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
}

} // namespace jv1010
