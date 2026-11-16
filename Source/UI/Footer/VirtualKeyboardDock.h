#pragma once

#include "../../PluginProcessor.h"
#include <functional>
#include <string>

namespace jv1010 {

class VirtualKeyboardDock {
public:
    explicit VirtualKeyboardDock(JV1010EditorAudioProcessor& processor);

    void render(float width, bool showKeyboard, bool showTools,
                const std::function<void(const std::string&)>& setStatus);

    int getActiveMidiChannelZeroBased() const { return kbdChannelZeroBased_; }

private:
    void triggerPhrasePreview();
    void handleArpTick();

    JV1010EditorAudioProcessor& processor_;
    int kbdChannelZeroBased_{ 0 };
    bool chordMode_{ false };
    bool arpMode_{ false };
    int modWheelVal_{ 0 };
    int activeVirtualNote_{ -1 };
    double lastArpTimeMs_{ 0.0 };
    int arpStep_{ 0 };

    PatchData snapA_, snapB_;
    bool hasSnapA_{ false }, hasSnapB_{ false };
    float morphAmount_{ 0.0f };
};

} // namespace jv1010
