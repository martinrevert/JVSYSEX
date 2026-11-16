#pragma once

#include "../../PluginProcessor.h"
#include "../../Librarian/SyxMidFileIO.h"
#include <functional>
#include <memory>
#include <string>

namespace jv1010 {

class TopControlBar {
public:
    explicit TopControlBar(JV1010EditorAudioProcessor& processor);

    void render();

    bool isKeyboardVisible() const { return showKeyboard_; }
    bool isToolsVisible() const { return showTools_; }
    void setStatusText(const std::string& text) { statusText_ = text; }
    const std::string& getStatusText() const { return statusText_; }

    std::function<void()> onCompareToggled;

private:
    void triggerImportAsync();
    void triggerExportAsync(ExportScope scope, bool isMid);

    JV1010EditorAudioProcessor& processor_;
    bool showKeyboard_{ true };
    bool showTools_{ true };
    bool midiThru_{ false };
    std::string statusText_{ "Listo (Roland JV-1010 Model ID 6AH)" };
    std::unique_ptr<juce::FileChooser> fileChooser_;
};

} // namespace jv1010
