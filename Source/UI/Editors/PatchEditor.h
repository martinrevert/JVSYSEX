#pragma once

#include "../../PluginProcessor.h"

namespace jv1010 {

class PatchEditor {
public:
    explicit PatchEditor(JV1010EditorAudioProcessor& processor);

    void setTargetPart(int partIndexMinus1ForPatchMode) { targetPart_ = partIndexMinus1ForPatchMode; }
    int getTargetPart() const { return targetPart_; }

    void render();

private:
    JV1010EditorAudioProcessor& processor_;
    int targetPart_{ -1 }; // -1 = Patch Mode Temp, 0..15 = Perf Part
    int activeTone_{ 0 };  // 0..3
    bool multiToneLink_{ false };
};

} // namespace jv1010
