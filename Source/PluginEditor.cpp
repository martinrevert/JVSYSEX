#include "PluginEditor.h"

JV1010EditorAudioProcessorEditor::JV1010EditorAudioProcessorEditor(JV1010EditorAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor_(p),
      appView_(p) {

    imguiHost_.setRenderCallback([this]() {
        appView_.render();
    });
    addAndMakeVisible(imguiHost_);

    setResizable(true, true);
    setResizeLimits(1040, 680, 16384, 16384);
    setSize(1360, 860);
}

void JV1010EditorAudioProcessorEditor::parentHierarchyChanged() {
    if (auto* dw = findParentComponentOfClass<juce::DocumentWindow>()) {
        dw->setTitleBarButtonsRequired(juce::DocumentWindow::allButtons, false);
        dw->setResizable(true, true);
    }
}

void JV1010EditorAudioProcessorEditor::resized() {
    imguiHost_.setBounds(getLocalBounds());
}
