#pragma once

#include "../../PluginProcessor.h"

namespace jv1010 {

class DrumsEditor {
public:
    explicit DrumsEditor(JV1010EditorAudioProcessor& processor);

    void render();

private:
    JV1010EditorAudioProcessor& processor_;
    int selectedKeyIdx_{ 1 }; // 0..63 -> Key# 35..98 (36 = Kick)
};

} // namespace jv1010
