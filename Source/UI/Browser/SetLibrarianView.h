#pragma once

#include "../../PluginProcessor.h"
#include <functional>
#include <string>

namespace jv1010 {

class SetLibrarianView {
public:
    explicit SetLibrarianView(JV1010EditorAudioProcessor& processor);

    void render(float width, float height, int activeKbdChannelZeroBased,
                const std::function<void(const std::string&)>& setStatus,
                const std::function<void(int tabIndex)>& switchEditorTab);

private:
    JV1010EditorAudioProcessor& processor_;
    int assetType_{ 0 };    // 0=Patches, 1=Performances, 2=Drum Kits
    int bankFilter_{ 1 };   // 0=All, 1=USER..
    int catFilter_{ 0 };    // 0=All, 1..39
    char searchBuf_[64]{};
    int selectedIdx_{ 0 };
};

} // namespace jv1010
