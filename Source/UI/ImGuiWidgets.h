#pragma once

#include <imgui.h>
#include <functional>

namespace jv1010 {
namespace ui {

void SectionHeader(const char* title, ImU32 barColor = IM_COL32(36, 98, 178, 255));

bool ToggleSwitch(const char* label, bool* v);

bool FaderV(const char* label, int* v, int vMin, int vMax,
            float height = 115.0f, const char* displayFmt = "%d", int defaultValue = 0);

bool FaderH(const char* label, int* v, int vMin, int vMax,
            float width = 110.0f, const char* displayFmt = "%d", int defaultValue = 0);

// style: 0 = Cream, 1 = Amber, 2 = Orange, 3 = Dark
bool RolandButton(const char* label, bool isActive = false,
                  ImVec2 size = ImVec2(0.0f, 0.0f), int style = 0);

bool KnobInt(const char* label, int* v, int vMin, int vMax,
             const char* displayFmt = "%d", float radius = 19.0f, int defaultValue = 0);

void formatMidiNote(int note0to127, char* buf, size_t bufSize);

bool VelocityCurvePreview(const char* id, int* curve1to7, ImVec2 size = ImVec2(66.0f, 52.0f));

bool EnvelopeGraph(const char* id, const char* title,
                   int times[4], int levels[4],
                   bool bipolarPitch, bool hasLevel4, ImVec2 size);

void StructureDiagram(const char* title, int structType0to9, int booster0to3, ImVec2 size);

void VirtualKeyboard(const char* id, int startNote, int endNote,
                     int& activeNoteOut,
                     const std::function<void(int note, bool isDown)>& onNote,
                     ImVec2 size);

} // namespace ui
} // namespace jv1010
