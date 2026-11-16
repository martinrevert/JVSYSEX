#pragma once

#include "../../PluginProcessor.h"
#include <functional>

namespace jv1010 {

class PerformanceEditor {
public:
    explicit PerformanceEditor(JV1010EditorAudioProcessor& processor);

    void render(const std::function<void(int partIndexZeroBased)>& onEditPartPatch);

private:
    JV1010EditorAudioProcessor& processor_;
};

} // namespace jv1010
