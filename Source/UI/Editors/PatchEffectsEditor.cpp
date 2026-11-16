#include "PatchEffectsEditor.h"
#include "../ImGuiWidgets.h"
#include "../../Model/EfxAlgorithms.h"
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace jv1010 {

PatchEffectsEditor::PatchEffectsEditor(JV1010EditorAudioProcessor& processor)
    : processor_(processor) {}

namespace {
void drawEfxDiagram(int efxType, const int vals[12], const char* algoName, ImVec2 size) {
    float w = (size.x <= 0.0f) ? std::max(160.0f, ImGui::GetContentRegionAvail().x) : size.x;
    float h = (size.y <= 0.0f) ? std::max(140.0f, ImGui::GetContentRegionAvail().y) : size.y;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(w, h));
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(14, 16, 20, 255), 4.0f);
    dl->AddRect(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(52, 56, 64, 255), 4.0f);

    // Header bar
    dl->AddRectFilled(ImVec2(pos.x + 1.0f, pos.y + 1.0f), ImVec2(pos.x + w - 1.0f, pos.y + 22.0f), IM_COL32(24, 28, 34, 255), 3.0f);
    dl->AddLine(ImVec2(pos.x + 1.0f, pos.y + 22.0f), ImVec2(pos.x + w - 1.0f, pos.y + 22.0f), IM_COL32(52, 56, 64, 255));
    dl->AddText(ImVec2(pos.x + 8.0f, pos.y + 4.0f), IM_COL32(215, 220, 230, 255), algoName);

    float pad = 12.0f;
    ImVec2 gMin(pos.x + pad, pos.y + 26.0f + 4.0f);
    ImVec2 gMax(pos.x + w - pad, pos.y + h - 16.0f);
    float gW = std::max(40.0f, gMax.x - gMin.x);
    float gH = std::max(40.0f, gMax.y - gMin.y);
    float midY = gMin.y + gH * 0.5f;

    auto drawGrid = [&](int cols, int rows) {
        for (int c = 1; c < cols; ++c) {
            float x = gMin.x + gW * (static_cast<float>(c) / static_cast<float>(cols));
            dl->AddLine(ImVec2(x, gMin.y), ImVec2(x, gMax.y), IM_COL32(42, 46, 54, 120));
        }
        for (int r = 1; r < rows; ++r) {
            float y = gMin.y + gH * (static_cast<float>(r) / static_cast<float>(rows));
            bool isMid = (rows % 2 == 0 && r == rows / 2);
            dl->AddLine(ImVec2(gMin.x, y), ImVec2(gMax.x, y), isMid ? IM_COL32(95, 102, 116, 180) : IM_COL32(42, 46, 54, 120));
        }
    };

    if (efxType == 0) {
        // 01: STEREO-EQ parametric + shelving curve
        drawGrid(6, 4);
        dl->AddText(ImVec2(gMin.x + 2, gMin.y + 2), IM_COL32(130, 136, 148, 200), "+15dB");
        dl->AddText(ImVec2(gMin.x + 2, midY - 14), IM_COL32(130, 136, 148, 200), "0dB");
        dl->AddText(ImVec2(gMin.x + 2, gMax.y - 14), IM_COL32(130, 136, 148, 200), "-15dB");
        const char* fLbls[] = { "200Hz", "630Hz", "2.0kHz", "6.3kHz" };
        for (int i = 0; i < 4; ++i) {
            float fx = gMin.x + gW * (0.12f + i * 0.25f);
            dl->AddText(ImVec2(fx, gMax.y + 1.0f), IM_COL32(150, 156, 168, 220), fLbls[i]);
        }

        float loDb = static_cast<float>(vals[1] - 15);
        float hiDb = static_cast<float>(vals[3] - 15);
        float p1NormFreq = static_cast<float>(std::clamp(vals[4], 0, 16)) / 16.0f;
        float p1Db = static_cast<float>(vals[6] - 15);
        float p1Sig = 0.16f / (1.0f + vals[5] * 0.55f);
        float p2NormFreq = static_cast<float>(std::clamp(vals[7], 0, 16)) / 16.0f;
        float p2Db = static_cast<float>(vals[9] - 15);
        float p2Sig = 0.16f / (1.0f + vals[8] * 0.55f);

        const int steps = 64;
        ImVec2 prevPt;
        for (int s = 0; s <= steps; ++s) {
            float x = static_cast<float>(s) / static_cast<float>(steps);
            float lowCut = (vals[0] == 0) ? 0.16f : 0.26f;
            float highCut = (vals[2] == 0) ? 0.72f : 0.85f;
            float lowResp = loDb / (1.0f + std::exp((x - lowCut) * 14.0f));
            float hiResp = hiDb / (1.0f + std::exp((highCut - x) * 14.0f));
            float p1Resp = p1Db * std::exp(-((x - p1NormFreq) * (x - p1NormFreq)) / (2.0f * p1Sig * p1Sig));
            float p2Resp = p2Db * std::exp(-((x - p2NormFreq) * (x - p2NormFreq)) / (2.0f * p2Sig * p2Sig));
            float totalDb = std::clamp(lowResp + hiResp + p1Resp + p2Resp, -18.0f, 18.0f);
            float py = midY - (totalDb / 18.0f) * (gH * 0.44f);
            ImVec2 pt(gMin.x + x * gW, py);
            if (s > 0) {
                dl->AddQuadFilled(prevPt, pt, ImVec2(pt.x, midY), ImVec2(prevPt.x, midY), IM_COL32(59, 130, 246, 55));
                dl->AddLine(prevPt, pt, IM_COL32(118, 178, 255, 255), 2.4f);
            }
            prevPt = pt;
        }
        // Highlight Peak 1 & Peak 2 handles
        float p1y = midY - (p1Db / 18.0f) * (gH * 0.44f);
        float p2y = midY - (p2Db / 18.0f) * (gH * 0.44f);
        dl->AddCircleFilled(ImVec2(gMin.x + p1NormFreq * gW, p1y), 4.5f, IM_COL32(236, 152, 36, 255));
        dl->AddCircleFilled(ImVec2(gMin.x + p2NormFreq * gW, p2y), 4.5f, IM_COL32(32, 200, 128, 255));
    } else if (efxType == 4) {
        // 05: SPECTRUM (8-band vertical bar display)
        drawGrid(8, 4);
        const char* bands[8] = { "250", "500", "1k", "1.25k", "2k", "3.15k", "4k", "8k" };
        float colW = gW / 8.0f;
        for (int b = 0; b < 8; ++b) {
            float db = static_cast<float>(vals[b] - 15) / 15.0f;
            float bx = gMin.x + b * colW + colW * 0.16f;
            float bw = colW * 0.68f;
            float topY = midY - db * (gH * 0.44f);
            float y0 = std::min(midY, topY);
            float y1 = std::max(midY, topY);
            if (std::abs(y1 - y0) < 2.0f) { y0 = midY - 1.5f; y1 = midY + 1.5f; }
            ImU32 barCol = (db >= 0.0f) ? IM_COL32(59, 130, 246, 230) : IM_COL32(236, 152, 36, 230);
            dl->AddRectFilled(ImVec2(bx, y0), ImVec2(bx + bw, y1), barCol, 2.0f);
            dl->AddText(ImVec2(bx, gMax.y + 1.0f), IM_COL32(160, 168, 180, 220), bands[b]);
        }
    } else if (efxType == 1 || efxType == 2) {
        // 02: OVERDRIVE / 03: DISTORTION waveshaper curve
        drawGrid(4, 4);
        float drive = 1.0f + (static_cast<float>(vals[0]) / 127.0f) * 9.0f;
        bool hard = (efxType == 2);
        const int steps = 56;
        ImVec2 prev;
        for (int s = 0; s <= steps; ++s) {
            float x = -1.0f + 2.0f * (static_cast<float>(s) / static_cast<float>(steps));
            float y = hard ? std::clamp(x * drive * 0.6f, -1.0f, 1.0f) : std::tanh(x * drive);
            ImVec2 pt(gMin.x + (x * 0.5f + 0.5f) * gW, midY - y * (gH * 0.44f));
            if (s > 0) dl->AddLine(prev, pt, IM_COL32(239, 92, 68, 255), 2.4f);
            prev = pt;
        }
        dl->AddText(ImVec2(gMin.x + 4, gMax.y + 1.0f), IM_COL32(170, 178, 190, 220), "Waveshaper Transfer Curve (In -> Out)");
    } else if (efxType == 8 || efxType == 9) {
        // 09: COMPRESSOR / 10: LIMITER dynamics curve
        drawGrid(4, 4);
        dl->AddLine(ImVec2(gMin.x, gMax.y), ImVec2(gMax.x, gMin.y), IM_COL32(95, 102, 116, 160), 1.2f);
        float threshNorm = (efxType == 8) ? (1.0f - static_cast<float>(vals[0]) / 160.0f) : (static_cast<float>(vals[0]) / 127.0f);
        threshNorm = std::clamp(threshNorm, 0.15f, 0.85f);
        float ratioInv = (efxType == 8) ? 0.35f : (vals[2] == 0 ? 0.66f : (vals[2] == 1 ? 0.5f : (vals[2] == 2 ? 0.25f : 0.02f)));
        ImVec2 p0(gMin.x, gMax.y);
        ImVec2 pKnee(gMin.x + threshNorm * gW, gMax.y - threshNorm * gH);
        ImVec2 pEnd(gMax.x, gMax.y - (threshNorm + (1.0f - threshNorm) * ratioInv) * gH);
        dl->AddLine(p0, pKnee, IM_COL32(59, 130, 246, 255), 2.4f);
        dl->AddLine(pKnee, pEnd, IM_COL32(236, 152, 36, 255), 2.6f);
        dl->AddCircleFilled(pKnee, 4.5f, IM_COL32(255, 255, 255, 255));
        dl->AddText(ImVec2(gMin.x + 4, gMax.y + 1.0f), IM_COL32(170, 178, 190, 220), "Input dB -> Output dB Compression Curve");
    } else if (efxType >= 16 && efxType <= 20) {
        // 17..21: DELAYS (tap timeline)
        drawGrid(5, 2);
        // Dry reference pulse
        dl->AddRectFilled(ImVec2(gMin.x + 4.0f, gMin.y + gH * 0.15f), ImVec2(gMin.x + 12.0f, gMax.y), IM_COL32(180, 190, 205, 180), 2.0f);
        auto drawTap = [&](float timeNorm, float lvlNorm, const char* tag, ImU32 col) {
            float tx = gMin.x + 18.0f + std::clamp(timeNorm, 0.02f, 0.96f) * (gW - 28.0f);
            float ty = gMax.y - std::clamp(lvlNorm, 0.1f, 1.0f) * (gH * 0.85f);
            dl->AddRectFilled(ImVec2(tx - 5.0f, ty), ImVec2(tx + 5.0f, gMax.y), col, 2.0f);
            dl->AddText(ImVec2(tx - 8.0f, std::max(gMin.y, ty - 14.0f)), IM_COL32(235, 238, 245, 240), tag);
        };
        if (efxType == 16 || efxType == 17) {
            drawTap(vals[1] / 126.0f, 0.85f, "L", IM_COL32(59, 130, 246, 235));
            drawTap(vals[2] / 126.0f, 0.85f, "R", IM_COL32(236, 152, 36, 235));
        } else if (efxType == 18) {
            drawTap(vals[0] / 125.0f, vals[5] / 127.0f, "L", IM_COL32(59, 130, 246, 235));
            drawTap(vals[1] / 125.0f, vals[6] / 127.0f, "R", IM_COL32(236, 152, 36, 235));
            drawTap(vals[2] / 125.0f, vals[7] / 127.0f, "C", IM_COL32(32, 200, 128, 235));
        } else if (efxType == 19) {
            const ImU32 cols[4] = { IM_COL32(59,130,246,235), IM_COL32(236,152,36,235), IM_COL32(32,200,128,235), IM_COL32(239,84,84,235) };
            for (int t = 0; t < 4; ++t) {
                char tag[4]; std::snprintf(tag, sizeof(tag), "T%d", t + 1);
                drawTap(vals[t] / 125.0f, vals[4 + t] / 127.0f, tag, cols[t]);
            }
        } else {
            float baseT = std::max(0.08f, vals[0] / 120.0f);
            float fbk = vals[1] / 110.0f;
            for (int e = 0; e < 4; ++e) {
                float tx = baseT * (0.28f + e * 0.22f);
                float amp = std::pow(std::max(0.25f, fbk), static_cast<float>(e));
                char tag[4]; std::snprintf(tag, sizeof(tag), "%d", e + 1);
                drawTap(tx, amp, tag, IM_COL32(59, 130, 246, static_cast<int>(240 - e * 45)));
            }
        }
        dl->AddText(ImVec2(gMin.x + 4, gMax.y + 1.0f), IM_COL32(170, 178, 190, 220), "Delay Tap Timeline (Time -> Level)");
    } else if (efxType >= 25 && efxType <= 39) {
        // 26..40: Series & Parallel Chain Routing Diagram
        bool isParallel = (efxType >= 37);
        const char* st1 = "STAGE 1";
        const char* st2 = "STAGE 2";
        if (efxType >= 25 && efxType <= 27) st1 = "OVERDRIVE";
        else if (efxType >= 28 && efxType <= 30) st1 = "DISTORTION";
        else if (efxType >= 31 && efxType <= 33) st1 = "ENHANCER";
        else if (efxType == 34 || efxType == 36 || efxType == 37 || efxType == 39) st1 = "CHORUS";
        else st1 = "FLANGER";

        if (efxType == 25 || efxType == 28 || efxType == 31) st2 = "CHORUS";
        else if (efxType == 26 || efxType == 29 || efxType == 32 || efxType == 36 || efxType == 39) st2 = "FLANGER";
        else st2 = "DELAY";

        auto drawNode = [&](ImVec2 c, const char* title, const char* sub, ImU32 col) {
            ImVec2 b0(c.x - 52.0f, c.y - 24.0f), b1(c.x + 52.0f, c.y + 24.0f);
            dl->AddRectFilled(b0, b1, (col & 0x00FFFFFF) | 0x38000000, 5.0f);
            dl->AddRect(b0, b1, col, 5.0f, 0, 1.8f);
            ImVec2 ts = ImGui::CalcTextSize(title);
            dl->AddText(ImVec2(c.x - ts.x * 0.5f, c.y - 14.0f), IM_COL32(246, 244, 238, 255), title);
            if (sub && sub[0]) {
                ImVec2 ss = ImGui::CalcTextSize(sub);
                dl->AddText(ImVec2(c.x - ss.x * 0.5f, c.y + 2.0f), IM_COL32(190, 196, 208, 230), sub);
            }
        };

        char s1Sub[32], s2Sub[32];
        std::snprintf(s1Sub, sizeof(s1Sub), "P1:%d  P2:%d", vals[0], vals[1]);
        std::snprintf(s2Sub, sizeof(s2Sub), "Out Lvl: %d", vals[7] > 0 ? vals[7] : vals[9]);

        if (!isParallel) {
            ImVec2 c1(gMin.x + gW * 0.28f, midY);
            ImVec2 c2(gMin.x + gW * 0.72f, midY);
            dl->AddLine(ImVec2(gMin.x + 4, midY), ImVec2(c1.x - 52.0f, midY), IM_COL32(180, 188, 200, 220), 2.0f);
            dl->AddLine(ImVec2(c1.x + 52.0f, midY), ImVec2(c2.x - 52.0f, midY), IM_COL32(236, 152, 36, 255), 2.2f);
            dl->AddLine(ImVec2(c2.x + 52.0f, midY), ImVec2(gMax.x - 4, midY), IM_COL32(180, 188, 200, 220), 2.0f);
            drawNode(c1, st1, s1Sub, IM_COL32(239, 92, 68, 255));
            drawNode(c2, st2, s2Sub, IM_COL32(59, 130, 246, 255));
            dl->AddText(ImVec2(gMin.x + 4, gMax.y + 1.0f), IM_COL32(170, 178, 190, 220), "Series Routing (Stage 1 -> Stage 2 -> Out)");
        } else {
            ImVec2 c1(gMin.x + gW * 0.50f, gMin.y + gH * 0.28f);
            ImVec2 c2(gMin.x + gW * 0.50f, gMin.y + gH * 0.72f);
            dl->AddLine(ImVec2(gMin.x + 6, midY), ImVec2(c1.x - 52.0f, c1.y), IM_COL32(180, 188, 200, 200), 1.8f);
            dl->AddLine(ImVec2(gMin.x + 6, midY), ImVec2(c2.x - 52.0f, c2.y), IM_COL32(180, 188, 200, 200), 1.8f);
            dl->AddLine(ImVec2(c1.x + 52.0f, c1.y), ImVec2(gMax.x - 6, midY), IM_COL32(59, 130, 246, 220), 1.8f);
            dl->AddLine(ImVec2(c2.x + 52.0f, c2.y), ImVec2(gMax.x - 6, midY), IM_COL32(32, 200, 128, 220), 1.8f);
            drawNode(c1, st1, s1Sub, IM_COL32(59, 130, 246, 255));
            drawNode(c2, st2, s2Sub, IM_COL32(32, 200, 128, 255));
            dl->AddText(ImVec2(gMin.x + 4, gMax.y + 1.0f), IM_COL32(170, 178, 190, 220), "Parallel Routing (Stage 1 // Stage 2 -> Mix)");
        }
    } else if (efxType == 21 || efxType == 22) {
        // 22/23: Pitch Shifter intervals (-24..+12 st)
        drawGrid(4, 4);
        int stA = vals[1] - 24;
        int stB = (efxType == 21) ? (vals[2] - 24) : 0;
        auto drawBar = [&](float xFrac, int st, const char* lbl, ImU32 col) {
            float bx = gMin.x + gW * xFrac;
            float norm = std::clamp(static_cast<float>(st) / 24.0f, -1.0f, 1.0f);
            float ty = midY - norm * (gH * 0.44f);
            dl->AddRectFilled(ImVec2(bx - 18, std::min(midY, ty)), ImVec2(bx + 18, std::max(midY, ty)), col, 3.0f);
            char txt[24]; std::snprintf(txt, sizeof(txt), "%s: %+d st", lbl, st);
            dl->AddText(ImVec2(bx - 28, gMax.y + 1.0f), IM_COL32(200, 206, 218, 230), txt);
        };
        drawBar(efxType == 21 ? 0.34f : 0.50f, stA, efxType == 21 ? "Voice A" : "Pitch", IM_COL32(59, 130, 246, 230));
        if (efxType == 21) drawBar(0.68f, stB, "Voice B", IM_COL32(236, 152, 36, 230));
    } else if (efxType == 23 || efxType == 24) {
        // 24: REVERB / 25: GATE-REVERB decay envelope
        drawGrid(4, 2);
        float preFrac = 0.05f + (vals[1] / 125.0f) * 0.25f;
        float decFrac = 0.20f + (vals[2] / 127.0f) * 0.65f;
        float x0 = gMin.x + preFrac * gW;
        float x1 = std::min(gMax.x, x0 + decFrac * gW);
        if (efxType == 23) {
            ImVec2 pts[3] = { ImVec2(x0, gMin.y + 8.0f), ImVec2(x0 + (x1 - x0) * 0.4f, midY), ImVec2(x1, gMax.y) };
            dl->AddTriangleFilled(ImVec2(x0, gMax.y), pts[0], ImVec2(x1, gMax.y), IM_COL32(59, 130, 246, 55));
            dl->AddLine(pts[0], pts[1], IM_COL32(118, 178, 255, 255), 2.4f);
            dl->AddLine(pts[1], pts[2], IM_COL32(118, 178, 255, 255), 2.4f);
        } else {
            dl->AddRectFilled(ImVec2(x0, gMin.y + 10.0f), ImVec2(x1, gMax.y), IM_COL32(236, 152, 36, 65), 2.0f);
            dl->AddLine(ImVec2(x0, gMin.y + 10.0f), ImVec2(x1, gMin.y + 10.0f), IM_COL32(236, 152, 36, 255), 2.4f);
            dl->AddLine(ImVec2(x1, gMin.y + 10.0f), ImVec2(x1, gMax.y), IM_COL32(236, 152, 36, 255), 2.4f);
        }
        dl->AddText(ImVec2(gMin.x + 4, gMax.y + 1.0f), IM_COL32(170, 178, 190, 220), "Pre-Delay & Decay Envelope");
    } else {
        // Modulation / Enhancer / Auto-Wah / Rotary LFO & Filter Contour
        drawGrid(6, 4);
        const int steps = 64;
        float rateCycles = (efxType == 5) ? 1.5f : (1.0f + (vals[1] / 127.0f) * 4.5f);
        float amp = (efxType == 5) ? (0.3f + vals[0] / 180.0f) : (0.15f + (vals[2] / 127.0f) * 0.30f);
        ImVec2 prev;
        for (int s = 0; s <= steps; ++s) {
            float x = static_cast<float>(s) / static_cast<float>(steps);
            float y = (efxType == 5) ? (-0.2f + std::pow(x, 2.2f) * amp * 2.2f) : (std::sin(x * rateCycles * 6.28318f) * amp);
            ImVec2 pt(gMin.x + x * gW, midY - std::clamp(y, -0.45f, 0.45f) * gH);
            if (s > 0) dl->AddLine(prev, pt, IM_COL32(59, 130, 246, 255), 2.2f);
            prev = pt;
        }
        dl->AddText(ImVec2(gMin.x + 4, gMax.y + 1.0f), IM_COL32(170, 178, 190, 220), "Modulation / Contour Preview");
    }
}
} // namespace

void PatchEffectsEditor::render() {
    uint32_t base = perfMode_ ? addr::kTempPerformanceBase : addr::kPatchModeTempPatchBase;
    auto getB = [&](uint32_t off) -> int { return processor_.getSynthState().getByte(base | off); };
    auto sendB = [&](uint32_t off, int v) { processor_.sendParameterDT1(base | off, static_cast<uint8_t>(v)); };

    // Top Master Effect Switches (System Common) + Quick Route to EFX
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
    int efxType = std::clamp(getB(perfMode_ ? addr::perf_common::EFXType : addr::patch_common::EFXType), 0, 39);
    const auto& algos = getEfxAlgorithms();
    ImGui::SetNextItemWidth(250.0f);
    if (ImGui::BeginCombo("EFX Type", algos[static_cast<size_t>(efxType)].name)) {
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
    ImGui::SetNextItemWidth(110.0f);
    if (ImGui::Combo("EFX Out", &efxOut, outNames, 3)) {
        sendB(perfMode_ ? addr::perf_common::EFXOutputAssign : addr::patch_common::EFXOutputAssign, efxOut);
    }

    // Split EFX into Left/Main Algorithm Panel + Right Sends & Realtime Control Panel
    float totalW = ImGui::GetContentRegionAvail().x;
    float totalH = ImGui::GetContentRegionAvail().y;
    float topCardH = std::clamp(totalH * 0.56f, 260.0f, 380.0f);
    float efxRightW = std::clamp(totalW * 0.28f, 255.0f, 340.0f);
    float efxLeftW = std::max(360.0f, totalW - efxRightW - 8.0f);

    const auto& curAlg = getEfxAlgorithm(efxType);
    uint32_t pBase = perfMode_ ? addr::perf_common::EFXParam1 : addr::patch_common::EFXParam1;
    int rawVals[12]{};
    for (int i = 0; i < 12; ++i) {
        rawVals[i] = getB(pBase + static_cast<uint32_t>(i));
    }

    ImGui::BeginChild("##EfxMainParams", ImVec2(efxLeftW, topCardH), true);
    {
        float innerAvailW = ImGui::GetContentRegionAvail().x;
        bool sideBySideViz = (innerAvailW >= 620.0f);
        float ctrlPaneW = sideBySideViz ? std::max(360.0f, innerAvailW * 0.58f) : innerAvailW;

        ImGui::BeginChild("##EfxControlsCol", ImVec2(ctrlPaneW, sideBySideViz ? 0.0f : (topCardH * 0.62f)), false);
        // Group 1: Switches, Segmented buttons & Frequency/HF Choices along clean flow
        bool anyDiscrete = false;
        for (int i = 0; i < 12; ++i) {
            const auto& pd = curAlg.params[static_cast<size_t>(i)];
            if (!pd.used) continue;
            if (pd.style == EfxControlStyle::Toggle || pd.style == EfxControlStyle::Segmented || pd.style == EfxControlStyle::Choice) {
                anyDiscrete = true;
                break;
            }
        }
        if (anyDiscrete) {
            float maxX = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
            bool firstItem = true;
            for (int i = 0; i < 12; ++i) {
                const auto& pd = curAlg.params[static_cast<size_t>(i)];
                if (!pd.used) continue;
                if (pd.style != EfxControlStyle::Toggle && pd.style != EfxControlStyle::Segmented && pd.style != EfxControlStyle::Choice) continue;

                ImGui::PushID(100 + i);
                int val = rawVals[i];
                if (!firstItem) {
                    float prevEnd = ImGui::GetItemRectMax().x;
                    float estW = (pd.style == EfxControlStyle::Segmented) ? (pd.valueLabelCount * 52.0f + 70.0f) : 145.0f;
                    if (prevEnd + estW < maxX) ImGui::SameLine(0.0f, 14.0f);
                }
                firstItem = false;

                if (pd.style == EfxControlStyle::Toggle && pd.valueLabels && pd.valueLabelCount >= 2) {
                    bool isSecond = (val != 0);
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%s:", pd.name);
                    ImGui::SameLine();
                    if (ui::RolandButton(isSecond ? pd.valueLabels[1] : pd.valueLabels[0], isSecond, ImVec2(64.0f, 24.0f), isSecond ? 1 : 3)) {
                        sendB(pBase + static_cast<uint32_t>(i), isSecond ? 0 : 1);
                    }
                } else if (pd.style == EfxControlStyle::Segmented && pd.valueLabels && pd.valueLabelCount > 0) {
                    ImGui::BeginGroup();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%s:", pd.name);
                    for (int o = 0; o < pd.valueLabelCount; ++o) {
                        ImGui::SameLine(0.0f, 3.0f);
                        if (ui::RolandButton(pd.valueLabels[o], val == o, ImVec2(0.0f, 22.0f), (val == o) ? 1 : 3)) {
                            sendB(pBase + static_cast<uint32_t>(i), o);
                        }
                    }
                    ImGui::EndGroup();
                } else if (pd.style == EfxControlStyle::Choice && pd.valueLabels && pd.valueLabelCount > 0) {
                    int clamped = std::clamp(val, 0, pd.valueLabelCount - 1);
                    ImGui::SetNextItemWidth(88.0f);
                    if (ImGui::Combo(pd.name, &clamped, pd.valueLabels, pd.valueLabelCount)) {
                        sendB(pBase + static_cast<uint32_t>(i), clamped);
                    }
                }
                ImGui::PopID();
            }
            ImGui::Spacing();
        }

        // Group 2: Vertical Faders (EQ bands, Gains, Tap Levels, Output Level) + Knobs side by side
        bool hasVSliders = false;
        bool hasKnobs = false;
        bool hasHSliders = false;
        for (int i = 0; i < 12; ++i) {
            const auto& pd = curAlg.params[static_cast<size_t>(i)];
            if (!pd.used) continue;
            if (pd.style == EfxControlStyle::VSlider) hasVSliders = true;
            else if (pd.style == EfxControlStyle::Knob) hasKnobs = true;
            else if (pd.style == EfxControlStyle::HSlider) hasHSliders = true;
        }

        if (hasVSliders) {
            ImGui::BeginGroup();
            bool firstVS = true;
            for (int i = 0; i < 12; ++i) {
                const auto& pd = curAlg.params[static_cast<size_t>(i)];
                if (!pd.used || pd.style != EfxControlStyle::VSlider) continue;
                if (!firstVS) ImGui::SameLine(0.0f, 10.0f);
                firstVS = false;
                ImGui::PushID(200 + i);
                int dispVal = std::clamp(rawVals[i], pd.minVal, pd.maxVal) + pd.displayOffset;
                int dMin = pd.minVal + pd.displayOffset;
                int dMax = pd.maxVal + pd.displayOffset;
                int defVal = (pd.displayOffset < 0) ? 0 : dMax;
                if (ui::FaderV(pd.name, &dispVal, dMin, dMax, 102.0f, pd.displayFmt, defVal)) {
                    sendB(pBase + static_cast<uint32_t>(i), dispVal - pd.displayOffset);
                }
                ImGui::PopID();
            }
            ImGui::EndGroup();
        }

        if (hasKnobs) {
            if (hasVSliders) ImGui::SameLine(0.0f, 22.0f);
            ImGui::BeginGroup();
            float maxX = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
            float rowStartX = ImGui::GetCursorScreenPos().x;
            float curX = rowStartX;
            bool firstK = true;
            for (int i = 0; i < 12; ++i) {
                const auto& pd = curAlg.params[static_cast<size_t>(i)];
                if (!pd.used || pd.style != EfxControlStyle::Knob) continue;
                float estKnobW = std::max(54.0f, ImGui::CalcTextSize(pd.name).x + 12.0f);
                if (!firstK) {
                    if (curX + estKnobW + 12.0f < maxX) {
                        ImGui::SameLine(0.0f, 12.0f);
                        curX += estKnobW + 12.0f;
                    } else {
                        curX = rowStartX + estKnobW;
                    }
                } else {
                    curX += estKnobW;
                }
                firstK = false;
                ImGui::PushID(300 + i);
                int dispVal = std::clamp(rawVals[i], pd.minVal, pd.maxVal) + pd.displayOffset;
                int dMin = pd.minVal + pd.displayOffset;
                int dMax = pd.maxVal + pd.displayOffset;
                int defVal = (pd.displayOffset < 0) ? 0 : pd.minVal;
                if (ui::KnobInt(pd.name, &dispVal, dMin, dMax, pd.displayFmt, 17.0f, defVal)) {
                    sendB(pBase + static_cast<uint32_t>(i), dispVal - pd.displayOffset);
                }
                ImGui::PopID();
            }
            ImGui::EndGroup();
        }

        if (hasHSliders) {
            ImGui::Spacing();
            float maxX = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
            bool firstHS = true;
            for (int i = 0; i < 12; ++i) {
                const auto& pd = curAlg.params[static_cast<size_t>(i)];
                if (!pd.used || pd.style != EfxControlStyle::HSlider) continue;
                if (!firstHS) {
                    float prevEnd = ImGui::GetItemRectMax().x;
                    if (prevEnd + 200.0f < maxX) ImGui::SameLine(0.0f, 16.0f);
                }
                firstHS = false;
                ImGui::PushID(400 + i);
                int dispVal = std::clamp(rawVals[i], pd.minVal, pd.maxVal) + pd.displayOffset;
                int dMin = pd.minVal + pd.displayOffset;
                int dMax = pd.maxVal + pd.displayOffset;
                int defVal = (pd.displayOffset < 0) ? 0 : ((dMax + dMin) / 2);
                    if (ui::FaderH(pd.name, &dispVal, dMin, dMax, 96.0f, pd.displayFmt, defVal)) {
                        sendB(pBase + static_cast<uint32_t>(i), dispVal - pd.displayOffset);
                    }
                    ImGui::PopID();
                }
            }
        ImGui::EndChild();

        if (sideBySideViz) {
            ImGui::SameLine(0.0f, 10.0f);
            float vizW = std::max(180.0f, ImGui::GetContentRegionAvail().x);
            float vizH = std::max(140.0f, ImGui::GetContentRegionAvail().y);
            drawEfxDiagram(efxType, rawVals, curAlg.name, ImVec2(vizW, vizH));
        } else {
            float vizW = std::max(180.0f, ImGui::GetContentRegionAvail().x);
            drawEfxDiagram(efxType, rawVals, curAlg.name, ImVec2(vizW, 96.0f));
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // EFX Right Panel: Sends (FaderV bank) + Control Sources 1 & 2
    ImGui::BeginChild("##EfxCtrlPanel", ImVec2(0.0f, topCardH), true);
    {
        ui::SectionHeader("EFX SENDS & CONTROL", IM_COL32(52, 60, 74, 255));
        int eMix = getB(perfMode_ ? addr::perf_common::EFXMixOutSendLevel : addr::patch_common::EFXMixOutSendLevel);
        int eCho = getB(perfMode_ ? addr::perf_common::EFXChorusSendLevel : addr::patch_common::EFXChorusSendLevel);
        int eRev = getB(perfMode_ ? addr::perf_common::EFXReverbSendLevel : addr::patch_common::EFXReverbSendLevel);
        int cSrc1 = getB(perfMode_ ? addr::perf_common::EFXControlSource1 : addr::patch_common::EFXControlSource1);
        int cDep1 = getB(perfMode_ ? addr::perf_common::EFXControlDepth1 : addr::patch_common::EFXControlDepth1) - 63;
        int cSrc2 = getB(perfMode_ ? addr::perf_common::EFXControlSource2 : addr::patch_common::EFXControlSource2);
        int cDep2 = getB(perfMode_ ? addr::perf_common::EFXControlDepth2 : addr::patch_common::EFXControlDepth2) - 63;

        if (ui::FaderV("MixOut", &eMix, 0, 127, 92.0f, "%d", 127)) sendB(perfMode_ ? addr::perf_common::EFXMixOutSendLevel : addr::patch_common::EFXMixOutSendLevel, eMix);
        ImGui::SameLine(0.0f, 12.0f);
        if (ui::FaderV("ChoSnd", &eCho, 0, 127, 92.0f, "%d", 0)) sendB(perfMode_ ? addr::perf_common::EFXChorusSendLevel : addr::patch_common::EFXChorusSendLevel, eCho);
        ImGui::SameLine(0.0f, 12.0f);
        if (ui::FaderV("RevSnd", &eRev, 0, 127, 92.0f, "%d", 0)) sendB(perfMode_ ? addr::perf_common::EFXReverbSendLevel : addr::patch_common::EFXReverbSendLevel, eRev);

        ImGui::Spacing();
        const char* ctrlSrcNames[] = { "OFF", "SYS-CTRL1", "SYS-CTRL2", "MODULATION", "BREATH", "FOOT", "VOLUME", "PAN", "EXPRESSION", "PITCH BEND", "AFTERTOUCH" };
        ImGui::SetNextItemWidth(125.0f);
        if (ImGui::Combo("Ctrl 1", &cSrc1, ctrlSrcNames, 11)) sendB(perfMode_ ? addr::perf_common::EFXControlSource1 : addr::patch_common::EFXControlSource1, cSrc1);
        if (ui::FaderH("Depth 1", &cDep1, -63, 63, 120.0f, "%+d", 0)) sendB(perfMode_ ? addr::perf_common::EFXControlDepth1 : addr::patch_common::EFXControlDepth1, cDep1 + 63);

        ImGui::SetNextItemWidth(125.0f);
        if (ImGui::Combo("Ctrl 2", &cSrc2, ctrlSrcNames, 11)) sendB(perfMode_ ? addr::perf_common::EFXControlSource2 : addr::patch_common::EFXControlSource2, cSrc2);
        if (ui::FaderH("Depth 2", &cDep2, -63, 63, 120.0f, "%+d", 0)) sendB(perfMode_ ? addr::perf_common::EFXControlDepth2 : addr::patch_common::EFXControlDepth2, cDep2 + 63);
    }
    ImGui::EndChild();

    // ========================================================================
    // LOWER ROW: CHORUS (LEFT 50%) & REVERB / DELAY (RIGHT 50%) SIDE-BY-SIDE
    // ========================================================================
    float botHalfW = std::max(260.0f, (totalW - 8.0f) * 0.5f);

    ImGui::BeginChild("##ChorusCard", ImVec2(botHalfW, 0.0f), true);
    ImGui::PushID("CHORUS");
    {
        ui::SectionHeader("CHORUS", IM_COL32(198, 42, 42, 255));
        int cOut = getB(perfMode_ ? addr::perf_common::ChorusOutput : addr::patch_common::ChorusOutput);
        const char* choOuts[] = { "MIX", "REV", "MIX+REV" };
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Chorus Out:");
        for (int i = 0; i < 3; ++i) {
            ImGui::SameLine(0.0f, 4.0f);
            if (ui::RolandButton(choOuts[i], cOut == i, ImVec2(0.0f, 24.0f), cOut == i ? 1 : 3)) {
                sendB(perfMode_ ? addr::perf_common::ChorusOutput : addr::patch_common::ChorusOutput, i);
            }
        }

        ImGui::Spacing();
        int cLvl = getB(perfMode_ ? addr::perf_common::ChorusLevel : addr::patch_common::ChorusLevel);
        int cRat = getB(perfMode_ ? addr::perf_common::ChorusRate : addr::patch_common::ChorusRate);
        int cDep = getB(perfMode_ ? addr::perf_common::ChorusDepth : addr::patch_common::ChorusDepth);
        int cPre = getB(perfMode_ ? addr::perf_common::ChorusPreDelay : addr::patch_common::ChorusPreDelay);
        int cFbk = getB(perfMode_ ? addr::perf_common::ChorusFeedback : addr::patch_common::ChorusFeedback);

        if (ui::FaderV("C.Level", &cLvl, 0, 127, 92.0f, "%d", 64)) sendB(perfMode_ ? addr::perf_common::ChorusLevel : addr::patch_common::ChorusLevel, cLvl);
        ImGui::SameLine(0.0f, 18.0f);
        if (ui::KnobInt("Rate", &cRat, 0, 127, "%d", 18.0f, 30)) sendB(perfMode_ ? addr::perf_common::ChorusRate : addr::patch_common::ChorusRate, cRat);
        ImGui::SameLine(0.0f, 14.0f);
        if (ui::KnobInt("Depth", &cDep, 0, 127, "%d", 18.0f, 30)) sendB(perfMode_ ? addr::perf_common::ChorusDepth : addr::patch_common::ChorusDepth, cDep);
        ImGui::SameLine(0.0f, 14.0f);
        if (ui::KnobInt("PreDly", &cPre, 0, 127, "%d", 18.0f, 0)) sendB(perfMode_ ? addr::perf_common::ChorusPreDelay : addr::patch_common::ChorusPreDelay, cPre);
        ImGui::SameLine(0.0f, 14.0f);
        if (ui::KnobInt("Feedback", &cFbk, 0, 127, "%d", 18.0f, 0)) sendB(perfMode_ ? addr::perf_common::ChorusFeedback : addr::patch_common::ChorusFeedback, cFbk);
    }
    ImGui::PopID();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("##ReverbCard", ImVec2(0.0f, 0.0f), true);
    ImGui::PushID("REVERB");
    {
        ui::SectionHeader("REVERB / DELAY", IM_COL32(195, 118, 24, 255));
        const char* rTypes[] = { "ROOM1", "ROOM2", "STAGE1", "STAGE2", "HALL1", "HALL2", "DELAY", "PAN-DLY" };
        int rTyp = getB(perfMode_ ? addr::perf_common::ReverbType : addr::patch_common::ReverbType);
        ImGui::SetNextItemWidth(135.0f);
        if (ImGui::Combo("Reverb Type", &rTyp, rTypes, 8)) sendB(perfMode_ ? addr::perf_common::ReverbType : addr::patch_common::ReverbType, rTyp);
        ImGui::SameLine(0.0f, 12.0f);
        const char* hfVals[] = { "200Hz","250Hz","315Hz","400Hz","500Hz","630Hz","800Hz","1kHz","1.25k","1.6k","2kHz","2.5k","3.15k","4kHz","5kHz","6.3k","8kHz","BYPASS" };
        int rHfd = getB(perfMode_ ? addr::perf_common::ReverbHFDamp : addr::patch_common::ReverbHFDamp);
        ImGui::SetNextItemWidth(105.0f);
        if (ImGui::Combo("HF Damp", &rHfd, hfVals, 18)) sendB(perfMode_ ? addr::perf_common::ReverbHFDamp : addr::patch_common::ReverbHFDamp, rHfd);

        ImGui::Spacing();
        int rLvl = getB(perfMode_ ? addr::perf_common::ReverbLevel : addr::patch_common::ReverbLevel);
        int rTim = getB(perfMode_ ? addr::perf_common::ReverbTime : addr::patch_common::ReverbTime);
        int dFbk = getB(perfMode_ ? addr::perf_common::DelayFeedback : addr::patch_common::DelayFeedback);

        if (ui::FaderV("R.Level", &rLvl, 0, 127, 92.0f, "%d", 64)) sendB(perfMode_ ? addr::perf_common::ReverbLevel : addr::patch_common::ReverbLevel, rLvl);
        ImGui::SameLine(0.0f, 20.0f);
        if (ui::KnobInt("Reverb Time", &rTim, 0, 127, "%d", 18.0f, 64)) sendB(perfMode_ ? addr::perf_common::ReverbTime : addr::patch_common::ReverbTime, rTim);
        ImGui::SameLine(0.0f, 18.0f);
        if (ui::KnobInt("Delay Fdbk", &dFbk, 0, 127, "%d", 18.0f, 0)) sendB(perfMode_ ? addr::perf_common::DelayFeedback : addr::patch_common::DelayFeedback, dFbk);
    }
    ImGui::PopID();
    ImGui::EndChild();
}

} // namespace jv1010
