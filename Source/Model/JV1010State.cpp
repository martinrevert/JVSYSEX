#include "JV1010State.h"
#include "../Core/SysExProtocol.h"
#include <algorithm>
#include <cstring>

namespace jv1010 {

// ============================================================================
// PatchData
// ============================================================================
void PatchData::initDefault(const std::string& name) {
    common.fill(0);
    setName(name);

    common[addr::patch_common::EFXOutputAssign] = 0; // MIX
    common[addr::patch_common::EFXMixOutSendLevel] = 127;
    common[addr::patch_common::EFXControlDepth1] = 63; // 0
    common[addr::patch_common::EFXControlDepth2] = 63; // 0
    common[addr::patch_common::ChorusLevel] = 64;
    common[addr::patch_common::ChorusRate] = 30;
    common[addr::patch_common::ChorusDepth] = 30;
    common[addr::patch_common::ReverbType] = 4; // HALL1
    common[addr::patch_common::ReverbLevel] = 64;
    common[addr::patch_common::ReverbTime] = 64;
    common[addr::patch_common::ReverbHFDamp] = 16; // 8000 Hz
    auto tempo = sysex::encodeNibbled2(120);
    common[addr::patch_common::TempoNibbled] = tempo.first;
    common[addr::patch_common::TempoNibbled + 1] = tempo.second;
    common[addr::patch_common::PatchLevel] = 127;
    common[addr::patch_common::PatchPan] = 64; // Center
    common[addr::patch_common::BendRangeUp] = 2;
    common[addr::patch_common::BendRangeDown] = 2;
    common[addr::patch_common::OctaveShift] = 3; // 0

    for (size_t t = 0; t < addr::kNumPatchTones; ++t) {
        auto& tone = tones[t];
        tone.fill(0);
        tone[addr::patch_tone::ToneSwitch] = (t == 0) ? 1 : 0;
        tone[addr::patch_tone::WaveGroupType] = 0; // INT
        tone[addr::patch_tone::WaveGroupID] = 1;   // INT-A
        tone[addr::patch_tone::WaveGain] = 1;      // 0 dB
        tone[addr::patch_tone::VelocityRangeLower] = 1;
        tone[addr::patch_tone::VelocityRangeUpper] = 127;
        tone[addr::patch_tone::KeyboardRangeLower] = 0;
        tone[addr::patch_tone::KeyboardRangeUpper] = 127;
        tone[addr::patch_tone::RedamperSwitch] = 1;
        tone[addr::patch_tone::VolumeSwitch] = 1;
        tone[addr::patch_tone::Hold1Switch] = 1;
        tone[addr::patch_tone::PitchBendSwitch] = 1;
        tone[addr::patch_tone::PanControlSwitch] = 1; // CONTINUOUS
        tone[addr::patch_tone::CoarseTune] = 48; // 0
        tone[addr::patch_tone::FineTune] = 50;   // 0
        tone[addr::patch_tone::PitchKeyfollow] = 10; // +100
        tone[addr::patch_tone::PitchEnvDepth] = 12; // 0
        tone[addr::patch_tone::PitchEnvVelSens] = 50; // 0 (-100..+150 -> 0 is around 50? Actually 0..125 -> -100..+150, 0 is 50)
        tone[addr::patch_tone::PitchEnvLevel1] = 63;
        tone[addr::patch_tone::PitchEnvLevel2] = 63;
        tone[addr::patch_tone::PitchEnvLevel3] = 63;
        tone[addr::patch_tone::PitchEnvLevel4] = 63;
        tone[addr::patch_tone::PitchLFO1Depth] = 63;
        tone[addr::patch_tone::PitchLFO2Depth] = 63;

        tone[addr::patch_tone::FilterType] = 1; // LPF
        tone[addr::patch_tone::CutoffFrequency] = 127;
        tone[addr::patch_tone::CutoffKeyfollow] = 5; // 0
        tone[addr::patch_tone::FilterEnvDepth] = 63; // 0
        tone[addr::patch_tone::FilterLFO1Depth] = 63;
        tone[addr::patch_tone::FilterLFO2Depth] = 63;

        tone[addr::patch_tone::ToneLevel] = 127;
        tone[addr::patch_tone::BiasPosition] = 60; // C4
        tone[addr::patch_tone::BiasLevel] = 7;     // 0
        tone[addr::patch_tone::LevelEnvLevel1] = 127;
        tone[addr::patch_tone::LevelEnvLevel2] = 127;
        tone[addr::patch_tone::LevelEnvLevel3] = 127;
        tone[addr::patch_tone::LevelEnvTime4] = 20;
        tone[addr::patch_tone::LevelLFO1Depth] = 63;
        tone[addr::patch_tone::LevelLFO2Depth] = 63;
        tone[addr::patch_tone::TonePan] = 64;
        tone[addr::patch_tone::PanKeyfollow] = 7; // 0
        tone[addr::patch_tone::AlternatePanDepth] = 64; // 0
        tone[addr::patch_tone::PanLFO1Depth] = 63;
        tone[addr::patch_tone::PanLFO2Depth] = 63;
        tone[addr::patch_tone::MixEFXSendLevel] = 127;
        tone[addr::patch_tone::ReverbSendLevel] = 40;
    }
}

std::string PatchData::getName() const {
    return sysex::decodeAsciiName12(common.data(), 12);
}

void PatchData::setName(const std::string& name) {
    auto encoded = sysex::encodeAsciiName12(name);
    std::copy(encoded.begin(), encoded.end(), common.begin());
}

// ============================================================================
// PerformanceData
// ============================================================================
void PerformanceData::initDefault(const std::string& name) {
    common.fill(0);
    setName(name);
    common[addr::perf_common::EFXMixOutSendLevel] = 127;
    common[addr::perf_common::EFXControlDepth1] = 63;
    common[addr::perf_common::EFXControlDepth2] = 63;
    common[addr::perf_common::ChorusLevel] = 64;
    common[addr::perf_common::ChorusRate] = 30;
    common[addr::perf_common::ChorusDepth] = 30;
    common[addr::perf_common::ReverbType] = 4; // HALL1
    common[addr::perf_common::ReverbLevel] = 64;
    common[addr::perf_common::ReverbTime] = 64;
    common[addr::perf_common::ReverbHFDamp] = 16;
    auto tempo = sysex::encodeNibbled2(120);
    common[addr::perf_common::TempoNibbled] = tempo.first;
    common[addr::perf_common::TempoNibbled + 1] = tempo.second;
    for (size_t i = 0; i < 16; ++i) {
        common[addr::perf_common::VoiceReserve1 + i] = 4;
    }

    for (size_t p = 0; p < addr::kNumPerfParts; ++p) {
        auto& part = parts[p];
        part.fill(0);
        part[addr::perf_part::ReceiveSwitch] = 1;
        part[addr::perf_part::MidiChannel] = static_cast<uint8_t>(p);
        part[addr::perf_part::PatchGroupType] = 0; // USER&PRESET
        part[addr::perf_part::PatchGroupID] = 3;   // PR-A
        part[addr::perf_part::PartLevel] = 127;
        part[addr::perf_part::PartPan] = 64;
        part[addr::perf_part::PartCoarseTune] = 48; // 0
        part[addr::perf_part::PartFineTune] = 50;   // 0
        part[addr::perf_part::OutputAssign] = 4;   // PATCH
        part[addr::perf_part::MixEFXSendLevel] = 127;
        part[addr::perf_part::ChorusSendLevel] = 0;
        part[addr::perf_part::ReverbSendLevel] = 40;
        part[addr::perf_part::ReceiveProgramChange] = 1;
        part[addr::perf_part::ReceiveVolume] = 1;
        part[addr::perf_part::ReceiveHold1] = 1;
        part[addr::perf_part::KeyboardRangeLower] = 0;
        part[addr::perf_part::KeyboardRangeUpper] = 127;
        part[addr::perf_part::OctaveShift] = 3;
    }
}

std::string PerformanceData::getName() const {
    return sysex::decodeAsciiName12(common.data(), 12);
}

void PerformanceData::setName(const std::string& name) {
    auto encoded = sysex::encodeAsciiName12(name);
    std::copy(encoded.begin(), encoded.end(), common.begin());
}

// ============================================================================
// RhythmSetData
// ============================================================================
void RhythmSetData::initDefault(const std::string& name) {
    common.fill(0);
    setName(name);
    for (size_t k = 0; k < addr::kRhythmNoteCount; ++k) {
        auto& note = notes[k];
        note.fill(0);
        note[addr::rhythm_note::ToneSwitch] = 1;
        note[addr::rhythm_note::WaveGroupType] = 0;
        note[addr::rhythm_note::WaveGroupID] = 1;
        note[addr::rhythm_note::WaveGain] = 1; // 0 dB
        note[addr::rhythm_note::BendRange] = 2;
        note[addr::rhythm_note::VolumeSwitch] = 1;
        note[addr::rhythm_note::Hold1Switch] = 1;
        note[addr::rhythm_note::PanControlSwitch] = 1;
        note[addr::rhythm_note::CoarseTune] = 60; // C4
        note[addr::rhythm_note::FineTune] = 50;
        note[addr::rhythm_note::PitchEnvDepth] = 12;
        note[addr::rhythm_note::PitchEnvLevel1] = 63;
        note[addr::rhythm_note::PitchEnvLevel2] = 63;
        note[addr::rhythm_note::PitchEnvLevel3] = 63;
        note[addr::rhythm_note::PitchEnvLevel4] = 63;
        note[addr::rhythm_note::FilterType] = 1; // LPF
        note[addr::rhythm_note::CutoffFrequency] = 127;
        note[addr::rhythm_note::FilterEnvDepth] = 63;
        note[addr::rhythm_note::ToneLevel] = 127;
        note[addr::rhythm_note::LevelEnvLevel1] = 127;
        note[addr::rhythm_note::LevelEnvLevel2] = 127;
        note[addr::rhythm_note::LevelEnvLevel3] = 127;
        note[addr::rhythm_note::LevelEnvTime4] = 30;
        note[addr::rhythm_note::TonePan] = 64;
        note[addr::rhythm_note::AlternatePanDepth] = 64;
        note[addr::rhythm_note::MixEFXSendLevel] = 127;
        note[addr::rhythm_note::ReverbSendLevel] = 40;
    }
}

std::string RhythmSetData::getName() const {
    return sysex::decodeAsciiName12(common.data(), 12);
}

void RhythmSetData::setName(const std::string& name) {
    auto encoded = sysex::encodeAsciiName12(name);
    std::copy(encoded.begin(), encoded.end(), common.begin());
}

// ============================================================================
// SystemData
// ============================================================================
void SystemData::initDefault() {
    common.fill(0);
    common[addr::sys::SoundMode] = 1; // PATCH
    common[addr::sys::PatchGroupType] = 0;
    common[addr::sys::PatchGroupID] = 3; // PR-A
    common[addr::sys::MasterTune] = 63;  // 440.0 Hz (427.4 + 0.2*63 = 440.0)
    common[addr::sys::EFXSwitch] = 1;
    common[addr::sys::ChorusSwitch] = 1;
    common[addr::sys::ReverbSwitch] = 1;
    common[addr::sys::ReceiveProgramChange] = 1;
    common[addr::sys::ReceiveBankSelect] = 1;
    common[addr::sys::ControlChannel] = 16; // OFF
    common[addr::sys::PatchReceiveChannel] = 0; // Ch 1
    common[addr::sys::PreviewSoundMode] = 2; // PHRASE
    common[addr::sys::PreviewNoteSet1] = 60;
    common[addr::sys::PreviewVelocitySet1] = 100;
    auto tempo = sysex::encodeNibbled2(120);
    common[addr::sys::SystemTempoNibbled] = tempo.first;
    common[addr::sys::SystemTempoNibbled + 1] = tempo.second;

    for (auto& st : scaleTunes) {
        st.fill(64); // 0 cents (-64..+63 -> 64 is 0)
    }
}

// ============================================================================
// JV1010State
// ============================================================================
JV1010State::JV1010State() {
    resetToDefaults();
}

void JV1010State::resetToDefaults() {
    std::lock_guard<std::mutex> lock(mutex_);
    system_.initDefault();
    tempPerformance_.initDefault("Temp Perform");
    for (size_t i = 0; i < 16; ++i) {
        perfTempPatches_[i].initDefault("Part Patch  ");
    }
    tempRhythm_.initDefault("Temp Rhythm ");
    patchModeTempPatch_.initDefault("Temp Patch  ");

    for (size_t i = 0; i < addr::kNumUserPerformances; ++i) {
        userPerformances_[i].initDefault("UserPerf " + std::to_string(i + 1));
    }
    for (size_t i = 0; i < addr::kNumUserRhythmSets; ++i) {
        userRhythms_[i].initDefault("UserDrm " + std::to_string(i + 1));
    }
    for (size_t i = 0; i < addr::kNumUserPatches; ++i) {
        userPatches_[i].initDefault("UserPat " + std::to_string(i + 1));
    }
}

bool JV1010State::resolvePointer(uint32_t address, uint8_t*& outPtr, size_t& outRemainingLinear) {
    uint8_t b0 = static_cast<uint8_t>((address >> 24) & 0x7F);
    uint8_t b1 = static_cast<uint8_t>((address >> 16) & 0x7F);
    uint8_t b2 = static_cast<uint8_t>((address >> 8) & 0x7F);
    uint8_t b3 = static_cast<uint8_t>(address & 0x7F);

    auto resolvePatchPtr = [&](PatchData& p) -> bool {
        if (b2 == 0x00 && b3 < addr::kPatchCommonSize) {
            outPtr = p.common.data() + b3;
            outRemainingLinear = addr::kPatchCommonSize - b3;
            return true;
        }
        for (int t = 0; t < 4; ++t) {
            uint8_t toneStart = static_cast<uint8_t>(0x10 + t * 2);
            if (b2 == toneStart || b2 == toneStart + 1) {
                size_t linearOffset = static_cast<size_t>((b2 - toneStart) * 128 + b3);
                if (linearOffset < addr::kPatchToneSize) {
                    outPtr = p.tones[static_cast<size_t>(t)].data() + linearOffset;
                    outRemainingLinear = addr::kPatchToneSize - linearOffset;
                    return true;
                }
            }
        }
        return false;
    };

    auto resolvePerfPtr = [&](PerformanceData& perf) -> bool {
        if (b2 == 0x00 && b3 < addr::kPerfCommonSize) {
            outPtr = perf.common.data() + b3;
            outRemainingLinear = addr::kPerfCommonSize - b3;
            return true;
        }
        if (b2 >= 0x10 && b2 <= 0x1F && b3 < addr::kPerfPartSize) {
            size_t partIdx = static_cast<size_t>(b2 - 0x10);
            outPtr = perf.parts[partIdx].data() + b3;
            outRemainingLinear = addr::kPerfPartSize - b3;
            return true;
        }
        return false;
    };

    auto resolveRhythmPtr = [&](RhythmSetData& r) -> bool {
        if (b2 == 0x00 && b3 < addr::kRhythmCommonSize) {
            outPtr = r.common.data() + b3;
            outRemainingLinear = addr::kRhythmCommonSize - b3;
            return true;
        }
        if (b2 >= addr::kRhythmNoteFirstKey && b2 <= addr::kRhythmNoteLastKey && b3 < addr::kRhythmNoteSize) {
            size_t noteIdx = static_cast<size_t>(b2 - addr::kRhythmNoteFirstKey);
            outPtr = r.notes[noteIdx].data() + b3;
            outRemainingLinear = addr::kRhythmNoteSize - b3;
            return true;
        }
        return false;
    };

    // System (00 00 xx xx)
    if (b0 == 0x00 && b1 == 0x00) {
        if (b2 == 0x00 && b3 < addr::kSystemCommonSize) {
            outPtr = system_.common.data() + b3;
            outRemainingLinear = addr::kSystemCommonSize - b3;
            return true;
        }
        if (b2 >= 0x10 && b2 <= 0x20 && b3 < addr::kScaleTuneSize) {
            size_t stIdx = static_cast<size_t>(b2 - 0x10);
            outPtr = system_.scaleTunes[stIdx].data() + b3;
            outRemainingLinear = addr::kScaleTuneSize - b3;
            return true;
        }
        return false;
    }

    // Temporary Performance (01 00 xx xx)
    if (b0 == 0x01 && b1 == 0x00) {
        return resolvePerfPtr(tempPerformance_);
    }

    // Performance Mode Temporary Patches (02 00..08, 02 0A..0F) & Temp Rhythm (02 09)
    if (b0 == 0x02 && b1 < 16) {
        if (b1 == 0x09) {
            return resolveRhythmPtr(tempRhythm_);
        }
        return resolvePatchPtr(perfTempPatches_[b1]);
    }

    // Patch Mode Temporary Patch (03 00 xx xx)
    if (b0 == 0x03 && b1 == 0x00) {
        return resolvePatchPtr(patchModeTempPatch_);
    }

    // User Performances (10 00..1F) & User Rhythm Sets (10 40..41)
    if (b0 == 0x10) {
        if (b1 < addr::kNumUserPerformances) {
            return resolvePerfPtr(userPerformances_[b1]);
        }
        if (b1 == 0x40 || b1 == 0x41) {
            return resolveRhythmPtr(userRhythms_[b1 - 0x40]);
        }
        return false;
    }

    // User Patches (11 00..7F)
    if (b0 == 0x11 && b1 < addr::kNumUserPatches) {
        return resolvePatchPtr(userPatches_[b1]);
    }

    return false;
}

bool JV1010State::applyDT1(uint32_t address, const std::vector<uint8_t>& data) {
    if (data.empty()) return false;
    bool updated = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        uint8_t* ptr = nullptr;
        size_t remaining = 0;
        if (resolvePointer(address, ptr, remaining)) {
            size_t toCopy = std::min(remaining, data.size());
            for (size_t i = 0; i < toCopy; ++i) {
                ptr[i] = data[i] & 0x7F;
            }
            updated = true;
        }
    }
    if (updated) {
        notifyListeners(address);
    }
    return updated;
}

uint8_t JV1010State::getByte(uint32_t address) const {
    std::lock_guard<std::mutex> lock(mutex_);
    uint8_t* ptr = nullptr;
    size_t remaining = 0;
    if (const_cast<JV1010State*>(this)->resolvePointer(address, ptr, remaining) && remaining > 0) {
        return *ptr;
    }
    return 0;
}

void JV1010State::setByte(uint32_t address, uint8_t value) {
    bool updated = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        uint8_t* ptr = nullptr;
        size_t remaining = 0;
        if (resolvePointer(address, ptr, remaining) && remaining > 0) {
            *ptr = value & 0x7F;
            updated = true;
        }
    }
    if (updated) {
        notifyListeners(address);
    }
}

int JV1010State::getNibbled2(uint32_t address) const {
    std::lock_guard<std::mutex> lock(mutex_);
    uint8_t* ptr = nullptr;
    size_t remaining = 0;
    if (const_cast<JV1010State*>(this)->resolvePointer(address, ptr, remaining) && remaining >= 2) {
        return sysex::decodeNibbled2(ptr[0], ptr[1]);
    }
    return 0;
}

void JV1010State::setNibbled2(uint32_t address, int value) {
    bool updated = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        uint8_t* ptr = nullptr;
        size_t remaining = 0;
        if (resolvePointer(address, ptr, remaining) && remaining >= 2) {
            auto nib = sysex::encodeNibbled2(value);
            ptr[0] = nib.first;
            ptr[1] = nib.second;
            updated = true;
        }
    }
    if (updated) {
        notifyListeners(address);
    }
}

SystemData JV1010State::getSystem() const { std::lock_guard<std::mutex> l(mutex_); return system_; }
void JV1010State::setSystem(const SystemData& sys) {
    { std::lock_guard<std::mutex> l(mutex_); system_ = sys; }
    notifyListeners(addr::kSystemBase);
}

PerformanceData JV1010State::getTempPerformance() const { std::lock_guard<std::mutex> l(mutex_); return tempPerformance_; }
void JV1010State::setTempPerformance(const PerformanceData& perf) {
    { std::lock_guard<std::mutex> l(mutex_); tempPerformance_ = perf; }
    notifyListeners(addr::kTempPerformanceBase);
}

PatchData JV1010State::getPatchModeTempPatch() const { std::lock_guard<std::mutex> l(mutex_); return patchModeTempPatch_; }
void JV1010State::setPatchModeTempPatch(const PatchData& patch) {
    { std::lock_guard<std::mutex> l(mutex_); patchModeTempPatch_ = patch; }
    notifyListeners(addr::kPatchModeTempPatchBase);
}

PatchData JV1010State::getPerfTempPatch(int partIndexZeroBased) const {
    std::lock_guard<std::mutex> l(mutex_);
    if (partIndexZeroBased < 0 || partIndexZeroBased >= 16) return patchModeTempPatch_;
    return perfTempPatches_[static_cast<size_t>(partIndexZeroBased)];
}

void JV1010State::setPerfTempPatch(int partIndexZeroBased, const PatchData& patch) {
    if (partIndexZeroBased < 0 || partIndexZeroBased >= 16) return;
    { std::lock_guard<std::mutex> l(mutex_); perfTempPatches_[static_cast<size_t>(partIndexZeroBased)] = patch; }
    notifyListeners(addr::getPerfTempPatchBase(partIndexZeroBased));
}

RhythmSetData JV1010State::getTempRhythm() const { std::lock_guard<std::mutex> l(mutex_); return tempRhythm_; }
void JV1010State::setTempRhythm(const RhythmSetData& rhythm) {
    { std::lock_guard<std::mutex> l(mutex_); tempRhythm_ = rhythm; }
    notifyListeners(addr::kTempRhythmBase);
}

PerformanceData JV1010State::getUserPerformance(int indexZeroBased) const {
    std::lock_guard<std::mutex> l(mutex_);
    return userPerformances_[static_cast<size_t>(std::clamp(indexZeroBased, 0, 31))];
}
void JV1010State::setUserPerformance(int indexZeroBased, const PerformanceData& perf) {
    int idx = std::clamp(indexZeroBased, 0, 31);
    { std::lock_guard<std::mutex> l(mutex_); userPerformances_[static_cast<size_t>(idx)] = perf; }
    notifyListeners(addr::getUserPerfBase(idx));
}

PatchData JV1010State::getUserPatch(int indexZeroBased) const {
    std::lock_guard<std::mutex> l(mutex_);
    return userPatches_[static_cast<size_t>(std::clamp(indexZeroBased, 0, 127))];
}
void JV1010State::setUserPatch(int indexZeroBased, const PatchData& patch) {
    int idx = std::clamp(indexZeroBased, 0, 127);
    { std::lock_guard<std::mutex> l(mutex_); userPatches_[static_cast<size_t>(idx)] = patch; }
    notifyListeners(addr::getUserPatchBase(idx));
}

RhythmSetData JV1010State::getUserRhythm(int indexZeroBased) const {
    std::lock_guard<std::mutex> l(mutex_);
    return userRhythms_[static_cast<size_t>(std::clamp(indexZeroBased, 0, 1))];
}
void JV1010State::setUserRhythm(int indexZeroBased, const RhythmSetData& rhythm) {
    int idx = std::clamp(indexZeroBased, 0, 1);
    { std::lock_guard<std::mutex> l(mutex_); userRhythms_[static_cast<size_t>(idx)] = rhythm; }
    notifyListeners(addr::getUserRhythmBase(idx));
}

std::vector<std::vector<uint8_t>> JV1010State::serializePatchToDT1(uint8_t deviceId, uint32_t baseAddress, const PatchData& patch) const {
    std::vector<std::vector<uint8_t>> packets;
    std::vector<uint8_t> commonVec(patch.common.begin(), patch.common.end());
    packets.push_back(sysex::buildDT1(deviceId, baseAddress | addr::kPatchCommonOffset, commonVec));
    for (int t = 0; t < 4; ++t) {
        std::vector<uint8_t> toneVec(patch.tones[static_cast<size_t>(t)].begin(),
                                     patch.tones[static_cast<size_t>(t)].end());
        auto split = sysex::buildSplitDT1(deviceId, baseAddress | addr::getPatchToneOffset(t), toneVec);
        packets.insert(packets.end(), split.begin(), split.end());
    }
    return packets;
}

std::vector<std::vector<uint8_t>> JV1010State::serializePerformanceToDT1(uint8_t deviceId, uint32_t baseAddress, const PerformanceData& perf) const {
    std::vector<std::vector<uint8_t>> packets;
    std::vector<uint8_t> commonVec(perf.common.begin(), perf.common.end());
    packets.push_back(sysex::buildDT1(deviceId, baseAddress | addr::kPerfCommonOffset, commonVec));
    for (int p = 0; p < 16; ++p) {
        std::vector<uint8_t> partVec(perf.parts[static_cast<size_t>(p)].begin(),
                                     perf.parts[static_cast<size_t>(p)].end());
        uint32_t partAddr = baseAddress | (addr::kPerfPart1Offset + (static_cast<uint32_t>(p) << 8));
        packets.push_back(sysex::buildDT1(deviceId, partAddr, partVec));
    }
    return packets;
}

std::vector<std::vector<uint8_t>> JV1010State::serializeRhythmToDT1(uint8_t deviceId, uint32_t baseAddress, const RhythmSetData& rhythm) const {
    std::vector<std::vector<uint8_t>> packets;
    std::vector<uint8_t> commonVec(rhythm.common.begin(), rhythm.common.end());
    packets.push_back(sysex::buildDT1(deviceId, baseAddress | addr::kRhythmCommonOffset, commonVec));
    for (size_t k = 0; k < addr::kRhythmNoteCount; ++k) {
        std::vector<uint8_t> noteVec(rhythm.notes[k].begin(), rhythm.notes[k].end());
        uint32_t noteAddr = baseAddress | addr::getRhythmNoteOffset(static_cast<int>(addr::kRhythmNoteFirstKey + k));
        packets.push_back(sysex::buildDT1(deviceId, noteAddr, noteVec));
    }
    return packets;
}

std::vector<std::vector<uint8_t>> JV1010State::serializeSystemToDT1(uint8_t deviceId) const {
    auto sys = getSystem();
    std::vector<std::vector<uint8_t>> packets;
    std::vector<uint8_t> commonVec(sys.common.begin(), sys.common.end());
    packets.push_back(sysex::buildDT1(deviceId, addr::kSystemBase, commonVec));
    for (int i = 0; i < 17; ++i) {
        std::vector<uint8_t> stVec(sys.scaleTunes[static_cast<size_t>(i)].begin(),
                                   sys.scaleTunes[static_cast<size_t>(i)].end());
        uint32_t stAddr = addr::kSystemScaleTunePart1 + (static_cast<uint32_t>(i) << 8);
        packets.push_back(sysex::buildDT1(deviceId, stAddr, stVec));
    }
    return packets;
}

void JV1010State::addListener(StateChangedCallback cb) {
    listeners_.push_back(std::move(cb));
}

void JV1010State::notifyListeners(uint32_t address) {
    for (auto& cb : listeners_) {
        if (cb) cb(address);
    }
}

} // namespace jv1010
