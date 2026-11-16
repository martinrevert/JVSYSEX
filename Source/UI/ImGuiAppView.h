#pragma once

#include "../PluginProcessor.h"
#include "Header/TopControlBar.h"
#include "Browser/SetLibrarianView.h"
#include "Editors/PerformanceEditor.h"
#include "Editors/PatchEditor.h"
#include "Editors/PatchEffectsEditor.h"
#include "Editors/DrumsEditor.h"
#include "Editors/SystemEditor.h"
#include "Footer/VirtualKeyboardDock.h"

namespace jv1010 {

class ImGuiAppView {
public:
    explicit ImGuiAppView(JV1010EditorAudioProcessor& processor);
    void render();

private:
    JV1010EditorAudioProcessor& processor_;
    TopControlBar topBar_;
    SetLibrarianView browser_;
    PerformanceEditor perfEditor_;
    PatchEditor patchEditor_;
    PatchEffectsEditor effectsEditor_;
    DrumsEditor drumsEditor_;
    SystemEditor systemEditor_;
    VirtualKeyboardDock footerDock_;

    int requestedEditorTab_{ -1 };
    PatchData compareBackup_;
    bool showingCompareBackup_{ false };
};

} // namespace jv1010
