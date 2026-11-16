#pragma once

#include "../../PluginProcessor.h"

namespace jv1010 {

class PatchEffectsEditor {
public:
    explicit PatchEffectsEditor(JV1010EditorAudioProcessor& processor);

    void setTargetPerformanceMode(bool isPerf) { perfMode_ = isPerf; }
    void render();

private:
    JV1010EditorAudioProcessor& processor_;
    bool perfMode_{ false };
};

} // namespace jv1010
