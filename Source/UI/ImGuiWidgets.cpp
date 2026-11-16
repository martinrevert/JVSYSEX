#include "ImGuiWidgets.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace jv1010 {
namespace ui {

void SectionHeader(const char* title, ImU32 barColor) {
    ImGui::PushID(title);
    float availW = std::max(80.0f, ImGui::GetContentRegionAvail().x);
    float h = ImGui::GetTextLineHeight() + 6.0f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(availW, h + 3.0f));

    ImDrawList* dl = ImGui::GetWindowDrawList();
    // Classic Roland JUNO-60 colored strip with subtle top highlight and bottom shadow
    dl->AddRectFilled(pos, ImVec2(pos.x + availW, pos.y + h), barColor, 2.0f);
    dl->AddLine(ImVec2(pos.x + 1.0f, pos.y + 1.0f), ImVec2(pos.x + availW - 1.0f, pos.y + 1.0f),
                IM_COL32(255, 255, 255, 55), 1.0f);
    dl->AddLine(ImVec2(pos.x, pos.y + h), ImVec2(pos.x + availW, pos.y + h),
                IM_COL32(10, 11, 14, 220), 1.5f);

    ImVec2 txtSz = ImGui::CalcTextSize(title);
    dl->AddText(ImVec2(pos.x + 8.0f, pos.y + (h - txtSz.y) * 0.5f),
                IM_COL32(246, 244, 238, 255), title);
    ImGui::PopID();
}

bool ToggleSwitch(const char* label, bool* v) {
    ImGui::PushID(label);
    char cleanLbl[128] = {};
    if (label) {
        size_t len = 0;
        while (label[len] && !(label[len] == '#' && label[len + 1] == '#') && len + 1 < sizeof(cleanLbl)) {
            cleanLbl[len] = label[len];
            ++len;
        }
        cleanLbl[len] = '\0';
    }

    const char* btnText = cleanLbl[0] ? cleanLbl : (*v ? "ON" : "OFF");
    ImVec2 txtSz = ImGui::CalcTextSize(btnText);
    ImVec2 btnSz(std::max(32.0f, txtSz.x + 14.0f), std::max(24.0f, txtSz.y + 10.0f));

    bool clicked = RolandButton(btnText, *v, btnSz, *v ? 1 : 3);
    if (clicked) {
        *v = !(*v);
    }

    ImGui::PopID();
    return clicked;
}

bool FaderV(const char* label, int* v, int vMin, int vMax,
            float height, const char* displayFmt, int defaultValue) {
    ImGui::PushID(label);
    ImGui::BeginGroup();

    char cleanLbl[128] = {};
    if (label) {
        size_t len = 0;
        while (label[len] && !(label[len] == '#' && label[len + 1] == '#') && len + 1 < sizeof(cleanLbl)) {
            cleanLbl[len] = label[len];
            ++len;
        }
        cleanLbl[len] = '\0';
    }

    char valPreview[32];
    std::snprintf(valPreview, sizeof(valPreview), displayFmt, *v);
    ImVec2 lblSz = cleanLbl[0] ? ImGui::CalcTextSize(cleanLbl) : ImVec2(0.0f, 0.0f);
    ImVec2 valPreSz = ImGui::CalcTextSize(valPreview);

    float lblH = ImGui::GetTextLineHeight();
    float w = std::max({ 42.0f, lblSz.x + 8.0f, valPreSz.x + 8.0f });
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##fader", ImVec2(w, height + lblH * 2.0f + 8.0f));

    bool changed = false;
    bool active = ImGui::IsItemActive();
    bool hovered = ImGui::IsItemHovered();

    float trackTop = pos.y + 8.0f;
    float trackBot = pos.y + height - 8.0f;
    float trackH = std::max(20.0f, trackBot - trackTop);

    if (active && (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f) || ImGui::IsMouseClicked(ImGuiMouseButton_Left))) {
        float mpY = ImGui::GetIO().MousePos.y;
        float norm = 1.0f - std::clamp((mpY - trackTop) / trackH, 0.0f, 1.0f);
        int nv = std::clamp(vMin + static_cast<int>(std::round(norm * static_cast<float>(vMax - vMin))), vMin, vMax);
        if (nv != *v) {
            *v = nv;
            changed = true;
        }
    }
    if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        int def = std::clamp(defaultValue, vMin, vMax);
        if (*v != def) {
            *v = def;
            changed = true;
        }
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float cx = pos.x + w * 0.5f;

    // Graduation tick marks on both sides (Roland JUNO-60 style)
    for (int i = 0; i <= 10; ++i) {
        float ty = trackTop + (static_cast<float>(i) / 10.0f) * trackH;
        float tickLen = (i == 0 || i == 5 || i == 10) ? 7.0f : 4.0f;
        ImU32 tickCol = (i == 0 || i == 5 || i == 10) ? IM_COL32(215, 212, 204, 210) : IM_COL32(130, 136, 146, 160);
        dl->AddLine(ImVec2(cx - 5.0f - tickLen, ty), ImVec2(cx - 5.0f, ty), tickCol, 1.0f);
        dl->AddLine(ImVec2(cx + 5.0f, ty), ImVec2(cx + 5.0f + tickLen, ty), tickCol, 1.0f);
    }

    // Recessed dark fader slot
    dl->AddRectFilled(ImVec2(cx - 2.5f, trackTop - 3.0f), ImVec2(cx + 2.5f, trackBot + 3.0f),
                      IM_COL32(10, 11, 14, 255), 1.5f);
    dl->AddRect(ImVec2(cx - 2.5f, trackTop - 3.0f), ImVec2(cx + 2.5f, trackBot + 3.0f),
                IM_COL32(55, 60, 68, 255), 1.5f);

    // 3D beveled metallic/charcoal fader cap
    float norm = (vMax > vMin) ? static_cast<float>(*v - vMin) / static_cast<float>(vMax - vMin) : 0.0f;
    float capY = trackBot - norm * trackH;
    float capW = 22.0f;
    float capH = 12.0f;
    ImVec2 cMin(cx - capW * 0.5f, capY - capH * 0.5f);
    ImVec2 cMax(cx + capW * 0.5f, capY + capH * 0.5f);

    // Cap drop shadow
    dl->AddRectFilled(ImVec2(cMin.x + 1.5f, cMin.y + 2.5f), ImVec2(cMax.x + 1.5f, cMax.y + 2.5f),
                      IM_COL32(6, 7, 10, 175), 2.0f);
    // Cap body
    ImU32 capBody = active ? IM_COL32(82, 88, 98, 255) : (hovered ? IM_COL32(72, 77, 86, 255) : IM_COL32(58, 62, 70, 255));
    dl->AddRectFilled(cMin, cMax, capBody, 2.0f);
    // Top bevel highlight & bottom shadow
    dl->AddLine(ImVec2(cMin.x + 1.0f, cMin.y + 1.0f), ImVec2(cMax.x - 1.0f, cMin.y + 1.0f),
                IM_COL32(150, 158, 172, 220), 1.2f);
    dl->AddLine(ImVec2(cMin.x + 1.0f, cMax.y - 1.0f), ImVec2(cMax.x - 1.0f, cMax.y - 1.0f),
                IM_COL32(20, 22, 26, 230), 1.2f);
    dl->AddRect(cMin, cMax, IM_COL32(24, 26, 30, 255), 2.0f);
    // Crisp white center indicator line
    dl->AddLine(ImVec2(cMin.x + 2.0f, capY), ImVec2(cMax.x - 2.0f, capY),
                IM_COL32(248, 246, 240, 255), 1.8f);

    // Value & label
    char valBuf[32];
    std::snprintf(valBuf, sizeof(valBuf), displayFmt, *v);
    ImVec2 valSz = ImGui::CalcTextSize(valBuf);
    dl->AddText(ImVec2(cx - valSz.x * 0.5f, pos.y + height + 2.0f), IM_COL32(236, 233, 224, 255), valBuf);

    if (cleanLbl[0]) {
        ImVec2 lblSz = ImGui::CalcTextSize(cleanLbl);
        dl->AddText(ImVec2(cx - lblSz.x * 0.5f, pos.y + height + lblH + 3.0f), IM_COL32(168, 172, 182, 255), cleanLbl);
    }

    ImGui::EndGroup();
    ImGui::PopID();
    return changed;
}

bool FaderH(const char* label, int* v, int vMin, int vMax,
            float width, const char* displayFmt, int defaultValue) {
    ImGui::PushID(label);
    ImGui::BeginGroup();

    char cleanLbl[128] = {};
    if (label) {
        size_t len = 0;
        while (label[len] && !(label[len] == '#' && label[len + 1] == '#') && len + 1 < sizeof(cleanLbl)) {
            cleanLbl[len] = label[len];
            ++len;
        }
        cleanLbl[len] = '\0';
    }

    char valBuf[32];
    std::snprintf(valBuf, sizeof(valBuf), displayFmt, *v);
    ImVec2 valSz = ImGui::CalcTextSize(valBuf);
    ImVec2 lblSz = cleanLbl[0] ? ImGui::CalcTextSize(cleanLbl) : ImVec2(0.0f, 0.0f);

    float h = 22.0f;
    float totalW = width + 38.0f + (lblSz.x > 0.0f ? (6.0f + lblSz.x) : 0.0f);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##faderH", ImVec2(totalW, h));

    bool changed = false;
    bool active = ImGui::IsItemActive();
    bool hovered = ImGui::IsItemHovered();

    float trackLeft = pos.x + 6.0f;
    float trackRight = pos.x + width - 6.0f;
    float trackW = std::max(20.0f, trackRight - trackLeft);
    float cy = pos.y + h * 0.5f;

    if (active && (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f) || ImGui::IsMouseClicked(ImGuiMouseButton_Left))) {
        float mpX = ImGui::GetIO().MousePos.x;
        float norm = std::clamp((mpX - trackLeft) / trackW, 0.0f, 1.0f);
        int nv = std::clamp(vMin + static_cast<int>(std::round(norm * static_cast<float>(vMax - vMin))), vMin, vMax);
        if (nv != *v) {
            *v = nv;
            changed = true;
        }
    }
    if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        int def = std::clamp(defaultValue, vMin, vMax);
        if (*v != def) {
            *v = def;
            changed = true;
        }
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Graduation ticks above & below
    for (int i = 0; i <= 8; ++i) {
        float tx = trackLeft + (static_cast<float>(i) / 8.0f) * trackW;
        float tickH = (i == 0 || i == 4 || i == 8) ? 4.5f : 2.5f;
        ImU32 tickCol = (i == 0 || i == 4 || i == 8) ? IM_COL32(215, 212, 204, 200) : IM_COL32(130, 136, 146, 150);
        dl->AddLine(ImVec2(tx, cy - 3.5f - tickH), ImVec2(tx, cy - 3.5f), tickCol, 1.0f);
        dl->AddLine(ImVec2(tx, cy + 3.5f), ImVec2(tx, cy + 3.5f + tickH), tickCol, 1.0f);
    }

    // Recessed dark horizontal slot
    dl->AddRectFilled(ImVec2(trackLeft - 3.0f, cy - 2.5f), ImVec2(trackRight + 3.0f, cy + 2.5f),
                      IM_COL32(10, 11, 14, 255), 1.5f);
    dl->AddRect(ImVec2(trackLeft - 3.0f, cy - 2.5f), ImVec2(trackRight + 3.0f, cy + 2.5f),
                IM_COL32(55, 60, 68, 255), 1.5f);

    // 3D beveled metallic/charcoal slider cap
    float norm = (vMax > vMin) ? static_cast<float>(*v - vMin) / static_cast<float>(vMax - vMin) : 0.0f;
    float capX = trackLeft + norm * trackW;
    float capW = 11.0f;
    float capH = 18.0f;
    ImVec2 cMin(capX - capW * 0.5f, cy - capH * 0.5f);
    ImVec2 cMax(capX + capW * 0.5f, cy + capH * 0.5f);

    dl->AddRectFilled(ImVec2(cMin.x + 1.5f, cMin.y + 2.0f), ImVec2(cMax.x + 1.5f, cMax.y + 2.0f),
                      IM_COL32(6, 7, 10, 175), 2.0f);
    ImU32 capBody = active ? IM_COL32(82, 88, 98, 255) : (hovered ? IM_COL32(72, 77, 86, 255) : IM_COL32(58, 62, 70, 255));
    dl->AddRectFilled(cMin, cMax, capBody, 2.0f);
    dl->AddLine(ImVec2(cMin.x + 1.0f, cMin.y + 1.0f), ImVec2(cMax.x - 1.0f, cMin.y + 1.0f),
                IM_COL32(150, 158, 172, 220), 1.2f);
    dl->AddLine(ImVec2(cMin.x + 1.0f, cMax.y - 1.0f), ImVec2(cMax.x - 1.0f, cMax.y - 1.0f),
                IM_COL32(20, 22, 26, 230), 1.2f);
    dl->AddRect(cMin, cMax, IM_COL32(24, 26, 30, 255), 2.0f);
    // Crisp white vertical center line
    dl->AddLine(ImVec2(capX, cMin.y + 2.0f), ImVec2(capX, cMax.y - 2.0f),
                IM_COL32(248, 246, 240, 255), 1.8f);

    // Value readout box + label
    ImVec2 vBoxMin(pos.x + width + 2.0f, pos.y + 2.0f);
    ImVec2 vBoxMax(pos.x + width + 36.0f, pos.y + h - 2.0f);
    dl->AddRectFilled(vBoxMin, vBoxMax, IM_COL32(14, 16, 20, 255), 2.0f);
    dl->AddRect(vBoxMin, vBoxMax, IM_COL32(46, 50, 58, 255), 2.0f);
    dl->AddText(ImVec2(vBoxMin.x + (34.0f - valSz.x) * 0.5f, cy - valSz.y * 0.5f),
                IM_COL32(236, 233, 224, 255), valBuf);

    if (cleanLbl[0]) {
        dl->AddText(ImVec2(pos.x + width + 42.0f, cy - lblSz.y * 0.5f),
                    IM_COL32(190, 194, 204, 255), cleanLbl);
    }

    ImGui::EndGroup();
    ImGui::PopID();
    return changed;
}

bool RolandButton(const char* label, bool isActive, ImVec2 size, int style) {
    ImGui::PushID(label);
    char cleanLbl[128] = {};
    if (label) {
        size_t len = 0;
        while (label[len] && !(label[len] == '#' && label[len + 1] == '#') && len + 1 < sizeof(cleanLbl)) {
            cleanLbl[len] = label[len];
            ++len;
        }
        cleanLbl[len] = '\0';
    }
    ImVec2 txtSz = ImGui::CalcTextSize(cleanLbl);
    ImVec2 btnSz(size.x > 0.0f ? size.x : (txtSz.x + 20.0f),
                 size.y > 0.0f ? size.y : (txtSz.y + 12.0f));

    ImVec2 pos = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton("##rbtn", btnSz);
    bool hovered = ImGui::IsItemHovered();
    bool held = ImGui::IsItemActive();

    ImU32 baseCol, hiCol, loCol, txtCol;
    if (style == 1) { // Amber
        baseCol = isActive ? IM_COL32(238, 168, 42, 255) : IM_COL32(214, 146, 36, 255);
        hiCol   = IM_COL32(252, 208, 108, 255);
        loCol   = IM_COL32(142, 86, 16, 255);
        txtCol  = IM_COL32(24, 22, 18, 255);
    } else if (style == 2) { // Orange-Red
        baseCol = isActive ? IM_COL32(232, 92, 42, 255) : IM_COL32(204, 74, 34, 255);
        hiCol   = IM_COL32(250, 144, 96, 255);
        loCol   = IM_COL32(132, 42, 16, 255);
        txtCol  = IM_COL32(248, 244, 238, 255);
    } else if (style == 3) { // Dark Hardware
        baseCol = isActive ? IM_COL32(68, 74, 84, 255) : IM_COL32(46, 50, 58, 255);
        hiCol   = IM_COL32(104, 112, 126, 255);
        loCol   = IM_COL32(20, 22, 26, 255);
        txtCol  = IM_COL32(236, 234, 228, 255);
    } else { // 0 = Cream / Ivory JUNO-60 keycap
        baseCol = isActive ? IM_COL32(242, 236, 214, 255) : IM_COL32(224, 218, 196, 255);
        hiCol   = IM_COL32(252, 248, 236, 255);
        loCol   = IM_COL32(156, 148, 128, 255);
        txtCol  = IM_COL32(28, 28, 30, 255);
    }

    if (hovered && !held) {
        // Subtle brighten on hover
        baseCol = (baseCol & 0xFF000000) |
                  (std::min(255, static_cast<int>((baseCol >> 0) & 0xFF) + 12) << 0) |
                  (std::min(255, static_cast<int>((baseCol >> 8) & 0xFF) + 12) << 8) |
                  (std::min(255, static_cast<int>((baseCol >> 16) & 0xFF) + 12) << 16);
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float pressOff = held ? 1.0f : 0.0f;
    ImVec2 bMin(pos.x, pos.y + pressOff);
    ImVec2 bMax(pos.x + btnSz.x, pos.y + btnSz.y + pressOff);

    // Outer recessed slot
    dl->AddRectFilled(ImVec2(pos.x - 1.0f, pos.y - 1.0f), ImVec2(pos.x + btnSz.x + 1.0f, pos.y + btnSz.y + 2.0f),
                      IM_COL32(10, 11, 14, 255), 3.0f);
    // Keycap face
    dl->AddRectFilled(bMin, bMax, baseCol, 2.5f);
    // 3D bevels
    dl->AddLine(ImVec2(bMin.x + 2.0f, bMin.y + 1.0f), ImVec2(bMax.x - 2.0f, bMin.y + 1.0f), hiCol, 1.4f);
    dl->AddLine(ImVec2(bMin.x + 2.0f, bMax.y - 1.0f), ImVec2(bMax.x - 2.0f, bMax.y - 1.0f), loCol, 1.8f);
    dl->AddRect(bMin, bMax, IM_COL32(18, 20, 24, 255), 2.5f);

    // Optional glowing red LED bar on top when active
    if (isActive) {
        float ledW = std::min(16.0f, btnSz.x * 0.4f);
        float lx = (bMin.x + bMax.x) * 0.5f - ledW * 0.5f;
        dl->AddRectFilled(ImVec2(lx, bMin.y + 2.5f), ImVec2(lx + ledW, bMin.y + 5.0f),
                          IM_COL32(255, 64, 52, 255), 1.0f);
    }

    dl->AddText(ImVec2((bMin.x + bMax.x - txtSz.x) * 0.5f, (bMin.y + bMax.y - txtSz.y) * 0.5f + (isActive ? 1.5f : 0.0f)),
                txtCol, cleanLbl);

    ImGui::PopID();
    return clicked;
}

bool KnobInt(const char* label, int* v, int vMin, int vMax,
             const char* displayFmt, float radius, int defaultValue) {
    ImGui::PushID(label);
    ImGui::BeginGroup();

    char cleanLbl[128] = {};
    if (label) {
        size_t len = 0;
        while (label[len] && !(label[len] == '#' && label[len + 1] == '#') && len + 1 < sizeof(cleanLbl)) {
            cleanLbl[len] = label[len];
            ++len;
        }
        cleanLbl[len] = '\0';
    }

    ImVec2 pos = ImGui::GetCursorScreenPos();
    float diam = radius * 2.0f;
    float labelH = ImGui::GetTextLineHeight();
    ImVec2 lblSz = cleanLbl[0] ? ImGui::CalcTextSize(cleanLbl) : ImVec2(0.0f, 0.0f);
    float minW = (radius >= 15.0f) ? 54.0f : 34.0f;
    float itemW = std::max({ diam + 8.0f, minW, (radius >= 15.0f ? (lblSz.x + 10.0f) : minW) });
    ImGui::InvisibleButton("##knob", ImVec2(itemW, diam + labelH * 2.0f + 6.0f));

    bool changed = false;
    bool active = ImGui::IsItemActive();
    bool hovered = ImGui::IsItemHovered();

    if (active && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
        float dy = -ImGui::GetIO().MouseDelta.y;
        float range = static_cast<float>(std::max(1, vMax - vMin));
        static ImGuiID lastKnobId = 0;
        static float accum = 0.0f;
        ImGuiID myId = ImGui::GetItemID();
        if (lastKnobId != myId) {
            lastKnobId = myId;
            accum = 0.0f;
        }
        accum += dy * (range / 120.0f);
        if (std::abs(accum) >= 1.0f) {
            int step = static_cast<int>(accum);
            accum -= static_cast<float>(step);
            int nv = std::clamp(*v + step, vMin, vMax);
            if (nv != *v) {
                *v = nv;
                changed = true;
            }
        }
    }
    if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        int def = std::clamp(defaultValue, vMin, vMax);
        if (*v != def) {
            *v = def;
            changed = true;
        }
    }

    ImVec2 center(pos.x + itemW * 0.5f, pos.y + radius + 3.0f);
    float angleMin = 3.1415926f * 0.75f;
    float angleMax = 3.1415926f * 2.25f;
    float norm = (vMax > vMin) ? static_cast<float>(*v - vMin) / static_cast<float>(vMax - vMin) : 0.0f;
    float angle = angleMin + norm * (angleMax - angleMin);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Outer dot scale around the arc
    const int numDots = 11;
    for (int i = 0; i < numDots; ++i) {
        float da = angleMin + (static_cast<float>(i) / static_cast<float>(numDots - 1)) * (angleMax - angleMin);
        ImVec2 dp(center.x + std::cos(da) * (radius + 1.5f),
                  center.y + std::sin(da) * (radius + 1.5f));
        bool lit = (static_cast<float>(i) / static_cast<float>(numDots - 1)) <= norm + 0.01f;
        dl->AddCircleFilled(dp, (i == 0 || i == 5 || i == 10) ? 1.5f : 1.0f,
                            lit ? IM_COL32(236, 232, 220, 230) : IM_COL32(100, 106, 118, 170), 6);
    }

    float bodyR = radius * 0.80f;
    // Drop shadow
    dl->AddCircleFilled(ImVec2(center.x + 2.0f, center.y + 3.0f), bodyR, IM_COL32(6, 7, 10, 185), 24);
    // Metallic silver-grey skirt
    dl->AddCircleFilled(center, bodyR, hovered || active ? IM_COL32(118, 124, 134, 255) : IM_COL32(92, 98, 108, 255), 24);
    // Black fluted inner cap
    float capR = bodyR * 0.76f;
    dl->AddCircleFilled(center, capR, IM_COL32(24, 26, 30, 255), 24);
    dl->AddCircle(center, capR, active ? IM_COL32(245, 158, 11, 255) : IM_COL32(56, 60, 68, 255), 24, 1.2f);

    // Crisp bright white Roland pointer line
    ImVec2 p1(center.x + std::cos(angle) * (capR * 0.25f),
              center.y + std::sin(angle) * (capR * 0.25f));
    ImVec2 p2(center.x + std::cos(angle) * (bodyR - 1.0f),
              center.y + std::sin(angle) * (bodyR - 1.0f));
    dl->AddLine(p1, p2, IM_COL32(248, 246, 240, 255), 2.2f);

    char valBuf[32];
    std::snprintf(valBuf, sizeof(valBuf), displayFmt, *v);
    ImVec2 valSz = ImGui::CalcTextSize(valBuf);
    dl->AddText(ImVec2(center.x - valSz.x * 0.5f, pos.y + diam + 3.0f), IM_COL32(236, 234, 226, 255), valBuf);

    if (cleanLbl[0]) {
        dl->AddText(ImVec2(center.x - lblSz.x * 0.5f, pos.y + diam + labelH + 4.0f), IM_COL32(168, 174, 184, 255), cleanLbl);
    }

    ImGui::EndGroup();
    ImGui::PopID();
    return changed;
}

void formatMidiNote(int note0to127, char* buf, size_t bufSize) {
    static const char* kNotes[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int n = std::clamp(note0to127, 0, 127);
    int oct = (n / 12) - 1;
    std::snprintf(buf, bufSize, "%s%d", kNotes[n % 12], oct);
}

bool VelocityCurvePreview(const char* id, int* curve1to7, ImVec2 size) {
    ImGui::PushID(id);
    ImGui::BeginGroup();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton("##vcurve", size);
    bool hovered = ImGui::IsItemHovered();
    if (clicked) {
        *curve1to7 = (*curve1to7 % 7) + 1;
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(14, 16, 20, 255), 3.0f);
    dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y),
                hovered ? IM_COL32(236, 152, 36, 255) : IM_COL32(52, 56, 64, 255), 3.0f);

    float pad = 6.0f;
    float w = size.x - pad * 2.0f;
    float h = size.y - pad * 2.0f - 12.0f;
    ImVec2 gMin(pos.x + pad, pos.y + pad);
    ImVec2 gMax(pos.x + pad + w, pos.y + pad + h);

    // Subtle 2x2 grid
    dl->AddLine(ImVec2(gMin.x + w * 0.5f, gMin.y), ImVec2(gMin.x + w * 0.5f, gMax.y), IM_COL32(42, 46, 54, 140));
    dl->AddLine(ImVec2(gMin.x, gMin.y + h * 0.5f), ImVec2(gMax.x, gMin.y + h * 0.5f), IM_COL32(42, 46, 54, 140));

    int c = std::clamp(*curve1to7, 1, 7);
    const int steps = 16;
    ImVec2 prev;
    for (int i = 0; i <= steps; ++i) {
        float x = static_cast<float>(i) / static_cast<float>(steps);
        float y = x;
        switch (c) {
            case 1: y = x; break;
            case 2: y = std::pow(x, 0.55f); break;
            case 3: y = std::pow(x, 1.85f); break;
            case 4: y = 0.5f + 0.5f * std::sin((x - 0.5f) * 3.1415926f); break;
            case 5: y = (x < 0.5f) ? (0.4f * x * 2.0f) : (0.4f + 0.6f * (x - 0.5f) * 2.0f); y = 1.0f - std::pow(1.0f - x, 2.2f); break;
            case 6: y = (x < 0.25f) ? (x * 2.0f) : (x > 0.75f ? (0.5f + (x - 0.75f) * 2.0f) : 0.5f); break;
            case 7: y = 0.65f + 0.35f * x; break;
        }
        ImVec2 pt(gMin.x + x * w, gMax.y - std::clamp(y, 0.0f, 1.0f) * h);
        if (i > 0) {
            dl->AddLine(prev, pt, IM_COL32(236, 232, 222, 255), 1.6f);
        }
        prev = pt;
    }

    char lbl[24];
    std::snprintf(lbl, sizeof(lbl), "VelCrv %d", c);
    ImVec2 txtSz = ImGui::CalcTextSize(lbl);
    dl->AddText(ImVec2(pos.x + (size.x - txtSz.x) * 0.5f, pos.y + size.y - txtSz.y - 2.0f),
                IM_COL32(180, 185, 194, 255), lbl);

    ImGui::EndGroup();
    ImGui::PopID();
    return clicked;
}

bool EnvelopeGraph(const char* id, const char* title,
                   int times[4], int levels[4],
                   bool bipolarPitch, bool hasLevel4, ImVec2 size) {
    ImGui::PushID(id);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##env", size);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(16, 18, 22, 255), 4.0f);
    dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(52, 56, 64, 255), 4.0f);
    dl->AddText(ImVec2(pos.x + 8.0f, pos.y + 4.0f), IM_COL32(180, 184, 192, 255), title);

    ImVec2 areaMin(pos.x + 12.0f, pos.y + 22.0f);
    ImVec2 areaMax(pos.x + size.x - 12.0f, pos.y + size.y - 10.0f);
    float areaW = areaMax.x - areaMin.x;
    float areaH = areaMax.y - areaMin.y;
    float segW = areaW / 4.2f;
    int maxLvl = bipolarPitch ? 126 : 127;

    auto lvlToY = [&](int lvl) -> float {
        float n = static_cast<float>(std::clamp(lvl, 0, maxLvl)) / static_cast<float>(maxLvl);
        return areaMax.y - n * areaH;
    };

    if (bipolarPitch) {
        float cy = lvlToY(63);
        dl->AddLine(ImVec2(areaMin.x, cy), ImVec2(areaMax.x, cy), IM_COL32(75, 85, 99, 200));
    }
    // Subtle MidiQuest-style background grid
    for (int gx = 1; gx <= 7; ++gx) {
        float x = areaMin.x + (areaW * static_cast<float>(gx) / 8.0f);
        dl->AddLine(ImVec2(x, areaMin.y), ImVec2(x, areaMax.y), IM_COL32(42, 46, 54, 110));
    }
    for (int gy = 1; gy <= 3; ++gy) {
        float y = areaMin.y + (areaH * static_cast<float>(gy) / 4.0f);
        dl->AddLine(ImVec2(areaMin.x, y), ImVec2(areaMax.x, y), IM_COL32(42, 46, 54, 110));
    }

    ImVec2 pts[5];
    pts[0] = ImVec2(areaMin.x, bipolarPitch ? lvlToY(63) : areaMax.y);
    float curX = areaMin.x;
    for (int i = 0; i < 4; ++i) {
        float dx = (static_cast<float>(std::clamp(times[i], 0, 127)) / 127.0f) * segW;
        curX += std::max(10.0f, dx);
        int l = (i == 3 && !hasLevel4) ? 0 : levels[i];
        pts[i + 1] = ImVec2(std::min(areaMax.x, curX), lvlToY(l));
    }

    static ImGuiID activeGraphId = 0;
    static int draggedNode = -1;
    ImGuiID myId = ImGui::GetItemID();
    bool changed = false;

    if (ImGui::IsItemActive()) {
        ImVec2 mp = ImGui::GetIO().MousePos;
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || activeGraphId != myId) {
            activeGraphId = myId;
            draggedNode = -1;
            float best = 14.0f;
            for (int i = 1; i <= 4; ++i) {
                float d = std::hypot(mp.x - pts[i].x, mp.y - pts[i].y);
                if (d < best) { best = d; draggedNode = i; }
            }
        }
        if (draggedNode >= 1 && draggedNode <= 4 && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
            int st = draggedNode - 1;
            float dx = std::max(0.0f, mp.x - pts[st].x);
            int nt = std::clamp(static_cast<int>((dx / segW) * 127.0f), 0, 127);
            float ny = 1.0f - std::clamp((mp.y - areaMin.y) / std::max(1.0f, areaH), 0.0f, 1.0f);
            int nl = (st == 3 && !hasLevel4) ? 0 : std::clamp(static_cast<int>(ny * maxLvl), 0, maxLvl);
            if (times[st] != nt || levels[st] != nl) {
                times[st] = nt;
                levels[st] = nl;
                changed = true;
            }
        }
    } else if (activeGraphId == myId) {
        draggedNode = -1;
    }

    float baseY = bipolarPitch ? lvlToY(63) : areaMax.y;
    for (int i = 0; i < 4; ++i) {
        dl->AddQuadFilled(pts[i], pts[i + 1],
                          ImVec2(pts[i + 1].x, baseY), ImVec2(pts[i].x, baseY),
                          IM_COL32(66, 132, 214, 48));
    }
    for (int i = 0; i < 4; ++i) {
        dl->AddLine(pts[i], pts[i + 1], IM_COL32(130, 188, 255, 255), 2.2f);
    }
    for (int i = 1; i <= 4; ++i) {
        bool isDrag = (activeGraphId == myId && draggedNode == i);
        dl->AddCircleFilled(pts[i], 4.8f, isDrag ? IM_COL32(255, 84, 68, 255) : IM_COL32(243, 244, 246, 255));
        dl->AddCircle(pts[i], 4.8f, IM_COL32(59, 130, 246, 255), 12, 1.5f);
    }

    ImGui::PopID();
    return changed;
}

void StructureDiagram(const char* title, int structType0to9, int booster0to3, ImVec2 size) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::Dummy(size);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(16, 18, 22, 255), 4.0f);
    dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(52, 56, 64, 255), 4.0f);

    const char* boostLabels[] = { "0dB", "+6dB", "+12dB", "+18dB" };
    char header[64];
    std::snprintf(header, sizeof(header), "%s | Type %d | Boost %s", title, structType0to9 + 1, boostLabels[std::clamp(booster0to3, 0, 3)]);
    dl->AddRectFilled(ImVec2(pos.x + 1.0f, pos.y + 1.0f), ImVec2(pos.x + size.x - 1.0f, pos.y + 20.0f), IM_COL32(28, 32, 38, 255), 3.0f);
    dl->AddLine(ImVec2(pos.x + 1.0f, pos.y + 20.0f), ImVec2(pos.x + size.x - 1.0f, pos.y + 20.0f), IM_COL32(52, 56, 64, 255), 1.0f);
    dl->AddText(ImVec2(pos.x + 8.0f, pos.y + 3.0f), IM_COL32(215, 220, 230, 255), header);

    auto drawBlock = [&](float x, float y, const char* txt, ImU32 col) {
        ImVec2 bMin(x, y - 9.0f);
        ImVec2 bMax(x + 38.0f, y + 9.0f);
        dl->AddRectFilled(bMin, bMax, (col & 0x00FFFFFF) | 0x45000000, 2.5f);
        dl->AddRect(bMin, bMax, col, 2.5f);
        ImVec2 tsz = ImGui::CalcTextSize(txt);
        dl->AddText(ImVec2(x + (38.0f - tsz.x) * 0.5f, y - tsz.y * 0.5f), IM_COL32(243, 244, 246, 255), txt);
    };
    auto drawLink = [&](float xA, float yA, float xB, float yB) {
        dl->AddLine(ImVec2(xA, yA), ImVec2(xB, yB), IM_COL32(170, 178, 192, 220), 1.4f);
    };

    float bodyTop = pos.y + 23.0f;
    float bodyBot = pos.y + size.y - 4.0f;
    float bodyH = std::max(36.0f, bodyBot - bodyTop);
    float x1 = pos.x + 10.0f;
    float stepX = std::max(46.0f, (size.x - 60.0f) / 3.2f);
    float x2 = x1 + stepX;
    float x3 = x2 + stepX;
    float x4 = x3 + stepX;
    float y1 = bodyTop + bodyH * 0.26f;
    float y2 = bodyTop + bodyH * 0.74f;
    float ym = (y1 + y2) * 0.5f;

    drawBlock(x1, y1, "WG1", IM_COL32(59, 130, 246, 255));
    drawBlock(x1, y2, "WG2", IM_COL32(59, 130, 246, 255));

    int st = std::clamp(structType0to9, 0, 9);
    if (st == 0) {
        drawLink(x1 + 38, y1, x2, y1); drawLink(x2 + 38, y1, x3, y1);
        drawLink(x1 + 38, y2, x2, y2); drawLink(x2 + 38, y2, x3, y2);
        drawBlock(x2, y1, "TVF", IM_COL32(16, 185, 129, 255));
        drawBlock(x3, y1, "TVA", IM_COL32(245, 158, 11, 255));
        drawBlock(x2, y2, "TVF", IM_COL32(16, 185, 129, 255));
        drawBlock(x3, y2, "TVA", IM_COL32(245, 158, 11, 255));
    } else if (st == 1 || st == 2 || st == 6 || st == 7) {
        const char* mod = (st == 1 || st == 6) ? "BST" : "RNG";
        drawLink(x1 + 38, y1, x2, ym); drawLink(x1 + 38, y2, x2, ym);
        drawLink(x2 + 38, ym, x3, ym); drawLink(x3 + 38, ym, x4, ym);
        drawBlock(x2, ym, mod, IM_COL32(239, 68, 68, 255));
        drawBlock(x3, ym, "TVF", IM_COL32(16, 185, 129, 255));
        drawBlock(x4, ym, "TVA", IM_COL32(245, 158, 11, 255));
        if (st == 6 || st == 7) {
            drawLink(x1 + 38, y1, x3, y1);
            drawBlock(x3, y1, "TVA", IM_COL32(245, 158, 11, 255));
        }
    } else {
        const char* mod = (st == 3 || st == 4 || st == 8) ? "BST" : "RNG";
        drawLink(x1 + 38, y1, x2, y1);
        drawLink(x1 + 38, y2, x2, y2);
        drawLink(x2 + 38, y1, x3, ym);
        drawLink(x2 + 38, y2, x3, ym);
        drawLink(x3 + 38, ym, x4, ym);
        drawBlock(x2, y1, "TVF", IM_COL32(16, 185, 129, 255));
        drawBlock(x2, y2, mod, IM_COL32(239, 68, 68, 255));
        drawBlock(x3, ym, "TVF", IM_COL32(16, 185, 129, 255));
        drawBlock(x4, ym, "TVA", IM_COL32(245, 158, 11, 255));
    }
}

void VirtualKeyboard(const char* id, int startNote, int endNote,
                     int& activeNoteOut,
                     const std::function<void(int note, bool isDown)>& onNote,
                     ImVec2 size) {
    ImGui::PushID(id);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##kbd", size);

    auto isBlack = [](int n) {
        int m = n % 12;
        return m == 1 || m == 3 || m == 6 || m == 8 || m == 10;
    };

    int whiteCount = 0;
    for (int n = startNote; n <= endNote; ++n) {
        if (!isBlack(n)) ++whiteCount;
    }
    float feltH = 4.0f;
    float keyTop = pos.y + feltH;
    float keyH = std::max(20.0f, size.y - feltH);
    float whiteW = size.x / static_cast<float>(std::max(1, whiteCount));
    float blackW = whiteW * 0.62f;
    float blackH = keyH * 0.60f;

    int hitNote = -1;
    if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImVec2 mp = ImGui::GetIO().MousePos;
        int wi = 0;
        for (int n = startNote; n <= endNote; ++n) {
            if (!isBlack(n)) {
                ++wi;
            } else {
                float bx = pos.x + wi * whiteW - blackW * 0.5f;
                if (mp.x >= bx && mp.x <= bx + blackW && mp.y >= keyTop && mp.y <= keyTop + blackH) {
                    hitNote = n;
                    break;
                }
            }
        }
        if (hitNote < 0) {
            wi = 0;
            for (int n = startNote; n <= endNote; ++n) {
                if (!isBlack(n)) {
                    float wx = pos.x + wi * whiteW;
                    if (mp.x >= wx && mp.x <= wx + whiteW) {
                        hitNote = n;
                        break;
                    }
                    ++wi;
                }
            }
        }
    }

    if (hitNote != activeNoteOut) {
        if (activeNoteOut >= 0 && onNote) onNote(activeNoteOut, false);
        activeNoteOut = hitNote;
        if (activeNoteOut >= 0 && onNote) onNote(activeNoteOut, true);
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Red felt strip above the keys (classic Roland hardware look)
    dl->AddRectFilled(pos, ImVec2(pos.x + size.x, keyTop), IM_COL32(156, 24, 28, 255));
    dl->AddLine(ImVec2(pos.x, keyTop - 1.0f), ImVec2(pos.x + size.x, keyTop - 1.0f), IM_COL32(72, 10, 12, 255), 1.0f);

    // Draw 3D shaded white/ivory keys
    int wi = 0;
    for (int n = startNote; n <= endNote; ++n) {
        if (!isBlack(n)) {
            float wx = pos.x + wi * whiteW;
            bool act = (n == activeNoteOut);
            ImVec2 kMin(wx, keyTop);
            ImVec2 kMax(wx + whiteW - 1.0f, keyTop + keyH);
            ImU32 topC = act ? IM_COL32(142, 186, 242, 255) : IM_COL32(236, 232, 222, 255);
            ImU32 botC = act ? IM_COL32(96, 146, 224, 255) : IM_COL32(212, 208, 196, 255);
            dl->AddRectFilledMultiColor(kMin, kMax, topC, topC, botC, botC);
            // Bottom key lip bevel
            dl->AddRectFilled(ImVec2(kMin.x, kMax.y - 4.0f), kMax,
                              act ? IM_COL32(72, 118, 196, 255) : IM_COL32(174, 170, 158, 255));
            dl->AddRect(kMin, kMax, IM_COL32(28, 30, 34, 255));
            if (n % 12 == 0 && whiteW >= 14.0f) {
                char cLbl[8];
                std::snprintf(cLbl, sizeof(cLbl), "C%d", (n / 12) - 1);
                ImVec2 cSz = ImGui::CalcTextSize(cLbl);
                dl->AddText(ImVec2(wx + (whiteW - cSz.x) * 0.5f, kMax.y - cSz.y - 4.0f),
                            act ? IM_COL32(255, 255, 255, 255) : IM_COL32(96, 94, 88, 220), cLbl);
            }
            ++wi;
        }
    }

    // Draw 3D beveled glossy black keys
    wi = 0;
    for (int n = startNote; n <= endNote; ++n) {
        if (!isBlack(n)) {
            ++wi;
        } else {
            float bx = pos.x + wi * whiteW - blackW * 0.5f;
            bool act = (n == activeNoteOut);
            ImVec2 bMin(bx, keyTop);
            ImVec2 bMax(bx + blackW, keyTop + blackH);
            // Side shadow
            dl->AddRectFilled(ImVec2(bMin.x + 2.0f, bMin.y), ImVec2(bMax.x + 2.0f, bMax.y + 2.0f),
                              IM_COL32(0, 0, 0, 90), 2.0f);
            // Key body
            dl->AddRectFilled(bMin, bMax,
                              act ? IM_COL32(232, 138, 32, 255) : IM_COL32(24, 26, 30, 255), 2.0f);
            // Glossy top highlight
            dl->AddRectFilled(ImVec2(bMin.x + 2.0f, bMin.y + 2.0f),
                              ImVec2(bMax.x - 2.0f, bMax.y - 8.0f),
                              act ? IM_COL32(248, 172, 64, 255) : IM_COL32(46, 50, 58, 255), 1.5f);
            dl->AddRect(bMin, bMax, IM_COL32(10, 11, 14, 255), 2.0f);
        }
    }

    ImGui::PopID();
}

} // namespace ui
} // namespace jv1010
