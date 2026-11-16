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

    int initW = 1520;
    int initH = 900;
    if (const auto* disp = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()) {
        auto area = disp->userArea;
        initW = juce::jmin(initW, juce::jmax(1040, area.getWidth() - 32));
        initH = juce::jmin(initH, juce::jmax(680, area.getHeight() - 48));
    }
    setSize(initW, initH);
}

void JV1010EditorAudioProcessorEditor::parentHierarchyChanged() {
    if (auto* dw = findParentComponentOfClass<juce::DocumentWindow>()) {
        dw->setTitleBarButtonsRequired(juce::DocumentWindow::allButtons, false);
        dw->setResizable(true, true);

        // Remove StandaloneFilterWindow's redundant "Options" button
        for (int i = dw->getNumChildComponents() - 1; i >= 0; --i) {
            if (auto* btn = dynamic_cast<juce::TextButton*>(dw->getChildComponent(i))) {
                if (btn->getButtonText() == "Options") {
                    btn->setVisible(false);
                    btn->setEnabled(false);
                    dw->removeChildComponent(btn);
                }
            }
        }

        if (!initialBoundsApplied_) {
            initialBoundsApplied_ = true;
            int targetW = 1520;
            int targetH = 900;
            if (const auto* disp = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()) {
                auto area = disp->userArea;
                targetW = juce::jmin(targetW, juce::jmax(1040, area.getWidth() - 32));
                targetH = juce::jmin(targetH, juce::jmax(680, area.getHeight() - 48));
            }
            if (dw->getWidth() < targetW || dw->getHeight() < targetH) {
                dw->centreWithSize(targetW, targetH);
            }
        }
    }
}

void JV1010EditorAudioProcessorEditor::resized() {
    imguiHost_.setBounds(getLocalBounds());
}
