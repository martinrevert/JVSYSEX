#include "ExpansionDetector.h"
#include <juce_data_structures/juce_data_structures.h>

namespace jv1010 {

namespace {
std::unique_ptr<juce::PropertiesFile> openAppSettings() {
    juce::PropertiesFile::Options opts;
    opts.applicationName = "JV-1010 Editor Pro";
    opts.filenameSuffix = ".settings";
    opts.osxLibrarySubFolder = "Application Support";
    return std::make_unique<juce::PropertiesFile>(opts);
}
} // namespace

ExpansionDetector::ExpansionDetector(MidiManager& midi, JV1010State& state)
    : midi_(midi), synthState_(state) {
    loadPersistedBoard();
}

void ExpansionDetector::loadPersistedBoard() {
    if (auto props = openAppSettings()) {
        int saved = props->getIntValue("expBoardId", 0);
        if (saved >= 0 && saved <= 19) {
            installedGroupId_.store(saved);
        }
    }
}

void ExpansionDetector::setInstalledBoard(int groupId, bool persist) {
    if (groupId < 0 || groupId > 19) groupId = 0;
    installedGroupId_.store(groupId);
    if (persist) {
        if (auto props = openAppSettings()) {
            props->setValue("expBoardId", groupId);
            props->saveIfNeeded();
        }
    }
}

const SrJv80BoardInfo* ExpansionDetector::getInstalledBoardInfo() const {
    int gid = installedGroupId_.load();
    if (gid <= 0) return nullptr;
    return findSrJv80ByGroupId(gid);
}

} // namespace jv1010
