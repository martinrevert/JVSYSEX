#pragma once

#include "../Core/SysExProtocol.h"
#include "../Model/JV1010State.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <string>
#include <vector>

namespace jv1010 {

enum class ExportScope {
    TempPatch = 0,
    TempPerformance,
    TempRhythm,
    UserPatches128,
    UserPerformances32,
    UserRhythmSets2,
    FullUserMemory
};

enum class ImportTargetMode {
    PreserveAddresses = 0, // Apply to exact addresses in file (remapping Card/Exp if needed)
    LoadIntoTempBuffer,    // Route first Patch/Perf/Rhythm into Temporary edit buffer
    LoadIntoUserBank       // Route Patches/Perfs/Rhythms into User memory starting at slot 0
};

struct ImportResult {
    bool success = false;
    int totalMessages = 0;
    int appliedMessages = 0;
    int patchesLoaded = 0;
    int perfsLoaded = 0;
    int rhythmsLoaded = 0;
    bool remappedFromJVFamily = false;
    std::string summary;
    std::vector<std::vector<uint8_t>> hardwarePackets;
};

class SyxMidFileIO {
public:
    static ImportResult importFile(const juce::File& file,
                                   JV1010State& state,
                                   ImportTargetMode targetMode = ImportTargetMode::PreserveAddresses,
                                   int targetUserSlot = 0,
                                   uint8_t deviceId = sysex::kDefaultDeviceId);

    static bool exportToSyx(const juce::File& file,
                            const JV1010State& state,
                            ExportScope scope,
                            uint8_t deviceId = sysex::kDefaultDeviceId);

    static bool exportToMid(const juce::File& file,
                            const JV1010State& state,
                            ExportScope scope,
                            uint8_t deviceId = sysex::kDefaultDeviceId);

    static std::vector<std::vector<uint8_t>> collectPacketsForScope(const JV1010State& state,
                                                                    ExportScope scope,
                                                                    uint8_t deviceId);
};

} // namespace jv1010
