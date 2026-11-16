#pragma once

#include "MidiManager.h"
#include "SysExProtocol.h"
#include "../Database/SrJv80CatalogDB.h"
#include "../Model/JV1010State.h"
#include <atomic>
#include <functional>
#include <string>

namespace jv1010 {

class ExpansionDetector {
public:
    ExpansionDetector(MidiManager& midi, JV1010State& state);

    // Starts auto-detection probe:
    // 1. Sends Universal Identity Request (F0 7E dev 06 01 F7)
    // 2. Switches temporarily to WAVE-EXP Patch 001 (MSB=84, LSB=2, PC=0) on Patch Rx Ch
    // 3. Requests System Common (00 00 00 00) & Temp Patch Common (03 00 00 00)
    // 4. Inspects Patch Group ID + Patch Name against SrJv80CatalogDB
    void startAutoDetect(uint8_t deviceId, std::function<void(const SrJv80BoardInfo* detected, const std::string& statusMsg)> onComplete);

    // Process incoming SysEx messages during detection
    void handleSysEx(const sysex::ParsedSysEx& msg);

    // Manual override / current board
    void setInstalledBoard(int groupId); // 0 = None, 1..19 = SR-JV80-01..19
    int getInstalledBoardGroupId() const { return installedGroupId_.load(); }
    const SrJv80BoardInfo* getInstalledBoardInfo() const;

    bool isDetecting() const { return state_ != ProbeState::Idle; }

private:
    enum class ProbeState {
        Idle,
        WaitingIdentity,
        WaitingSystemAndPatch
    };

    MidiManager& midi_;
    JV1010State& synthState_;
    std::atomic<int> installedGroupId_{ 0 }; // 0 = None
    ProbeState state_{ ProbeState::Idle };
    uint8_t activeDevId_{ sysex::kDefaultDeviceId };

    // Saved previous patch selection to restore after probe
    uint8_t prevGroupType_{ 0 };
    uint8_t prevGroupId_{ 3 };
    int prevPatchNum_{ 0 };

    std::function<void(const SrJv80BoardInfo*, const std::string&)> callback_;
};

} // namespace jv1010
