#pragma once

#include "../../PluginProcessor.h"

namespace jv1010 {

class SystemEditor {
public:
    explicit SystemEditor(JV1010EditorAudioProcessor& processor);

    void render();

private:
    JV1010EditorAudioProcessor& processor_;
    int scaleTargetIdx_{ 0 }; // 0=Patch Mode, 1..16=Part 1..16
    bool sendGS42H_{ false };
};

} // namespace jv1010
