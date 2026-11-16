#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/ImGuiHost.h"
#include "UI/ImGuiAppView.h"

class JV1010EditorAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit JV1010EditorAudioProcessorEditor(JV1010EditorAudioProcessor&);
    ~JV1010EditorAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override {}
    void resized() override;
    void parentHierarchyChanged() override;

private:
    JV1010EditorAudioProcessor& audioProcessor_;
    jv1010::ImGuiHost imguiHost_;
    jv1010::ImGuiAppView appView_;
    bool initialBoundsApplied_{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JV1010EditorAudioProcessorEditor)
};
