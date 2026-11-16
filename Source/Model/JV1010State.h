#pragma once

#include "ParameterMaps.h"
#include "JV1010Types.h"
#include <array>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace jv1010 {

struct PatchData {
    std::array<uint8_t, addr::kPatchCommonSize> common{};
    std::array<std::array<uint8_t, addr::kPatchToneSize>, addr::kNumPatchTones> tones{};

    void initDefault(const std::string& name = "Init Patch  ");
    std::string getName() const;
    void setName(const std::string& name);
};

struct PerformanceData {
    std::array<uint8_t, addr::kPerfCommonSize> common{};
    std::array<std::array<uint8_t, addr::kPerfPartSize>, addr::kNumPerfParts> parts{};

    void initDefault(const std::string& name = "Init Perform");
    std::string getName() const;
    void setName(const std::string& name);
};

struct RhythmSetData {
    std::array<uint8_t, addr::kRhythmCommonSize> common{};
    std::array<std::array<uint8_t, addr::kRhythmNoteSize>, addr::kRhythmNoteCount> notes{}; // Key# 35..98

    void initDefault(const std::string& name = "Init Rhythm ");
    std::string getName() const;
    void setName(const std::string& name);
};

struct SystemData {
    std::array<uint8_t, addr::kSystemCommonSize> common{};
    // 16 parts + 1 patch mode scale tune
    std::array<std::array<uint8_t, addr::kScaleTuneSize>, 17> scaleTunes{};

    void initDefault();
};

class JV1010State {
public:
    JV1010State();

    void resetToDefaults();

    // Thread-safe SysEx DT1 application (updates internal state from hardware or .syx/.mid)
    bool applyDT1(uint32_t address, const std::vector<uint8_t>& data);

    // Direct parameter accessors
    uint8_t getByte(uint32_t address) const;
    void setByte(uint32_t address, uint8_t value);

    int getNibbled2(uint32_t address) const;
    void setNibbled2(uint32_t address, int value);

    // Block accessors (copies for UI / export)
    SystemData getSystem() const;
    void setSystem(const SystemData& sys);

    PerformanceData getTempPerformance() const;
    void setTempPerformance(const PerformanceData& perf);

    PatchData getPatchModeTempPatch() const;
    void setPatchModeTempPatch(const PatchData& patch);

    PatchData getPerfTempPatch(int partIndexZeroBased) const;
    void setPerfTempPatch(int partIndexZeroBased, const PatchData& patch);

    RhythmSetData getTempRhythm() const;
    void setTempRhythm(const RhythmSetData& rhythm);

    PerformanceData getUserPerformance(int indexZeroBased) const;
    void setUserPerformance(int indexZeroBased, const PerformanceData& perf);

    PatchData getUserPatch(int indexZeroBased) const;
    void setUserPatch(int indexZeroBased, const PatchData& patch);

    RhythmSetData getUserRhythm(int indexZeroBased) const;
    void setUserRhythm(int indexZeroBased, const RhythmSetData& rhythm);

    // Build DT1 packet sequences for a full Patch, Performance, RhythmSet, or System
    std::vector<std::vector<uint8_t>> serializePatchToDT1(uint8_t deviceId, uint32_t baseAddress, const PatchData& patch) const;
    std::vector<std::vector<uint8_t>> serializePerformanceToDT1(uint8_t deviceId, uint32_t baseAddress, const PerformanceData& perf) const;
    std::vector<std::vector<uint8_t>> serializeRhythmToDT1(uint8_t deviceId, uint32_t baseAddress, const RhythmSetData& rhythm) const;
    std::vector<std::vector<uint8_t>> serializeSystemToDT1(uint8_t deviceId) const;

    // Listener registration for UI refresh
    using StateChangedCallback = std::function<void(uint32_t address)>;
    void addListener(StateChangedCallback cb);

private:
    bool resolvePointer(uint32_t address, uint8_t*& outPtr, size_t& outRemainingLinear);
    void notifyListeners(uint32_t address);

    mutable std::mutex mutex_;
    SystemData system_;
    PerformanceData tempPerformance_;
    std::array<PatchData, 16> perfTempPatches_; // index 9 (Part 10) unused
    RhythmSetData tempRhythm_;
    PatchData patchModeTempPatch_;

    std::array<PerformanceData, addr::kNumUserPerformances> userPerformances_;
    std::array<RhythmSetData, addr::kNumUserRhythmSets> userRhythms_;
    std::array<PatchData, addr::kNumUserPatches> userPatches_;

    std::vector<StateChangedCallback> listeners_;
};

} // namespace jv1010
