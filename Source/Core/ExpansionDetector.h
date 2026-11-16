#pragma once

#include "MidiManager.h"
#include "../Database/SrJv80CatalogDB.h"
#include "../Model/JV1010State.h"
#include <atomic>

namespace jv1010 {

class ExpansionDetector {
public:
    ExpansionDetector(MidiManager& midi, JV1010State& state);

    // User-selected expansion board (0 = None, 1..19 = SR-JV80-01..19), persisted across sessions
    void setInstalledBoard(int groupId, bool persist = true);
    int getInstalledBoardGroupId() const { return installedGroupId_.load(); }
    const SrJv80BoardInfo* getInstalledBoardInfo() const;

    void loadPersistedBoard();

private:
    MidiManager& midi_;
    JV1010State& synthState_;
    std::atomic<int> installedGroupId_{ 0 }; // 0 = None
};

} // namespace jv1010
