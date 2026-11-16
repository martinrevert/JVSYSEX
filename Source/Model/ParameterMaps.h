#pragma once

#include <cstdint>
#include <cstddef>

namespace jv1010 {
namespace addr {

// Base Start Addresses (p. 74 & p. 81)
constexpr uint32_t kSystemBase             = 0x00000000;
constexpr uint32_t kTempPerformanceBase    = 0x01000000;
constexpr uint32_t kPerfTempPatchPart1Base = 0x02000000; // Parts 1..9: 02 00..08 00 00
constexpr uint32_t kTempRhythmBase         = 0x02090000; // Part 10
constexpr uint32_t kPerfTempPatchPart11Base= 0x020A0000; // Parts 11..16: 02 0A..0F 00 00
constexpr uint32_t kPatchModeTempPatchBase = 0x03000000;

constexpr uint32_t kUserPerformanceBase    = 0x10000000; // USER:01..32 -> 10 00..1F 00 00
constexpr uint32_t kUserRhythmBase         = 0x10400000; // USER:1..2   -> 10 40..41 00 00
constexpr uint32_t kUserPatchBase          = 0x11000000; // USER:001..128 -> 11 00..7F 00 00

// System Sub-blocks (p. 74-75)
constexpr uint32_t kSystemCommonOffset     = 0x00000000;
constexpr uint32_t kSystemScaleTunePart1   = 0x00001000; // + (part * 0x100) for Part 1..16
constexpr uint32_t kSystemScaleTunePatch   = 0x00002000;

// Performance Sub-blocks (p. 76-77)
constexpr uint32_t kPerfCommonOffset       = 0x00000000;
constexpr uint32_t kPerfPart1Offset        = 0x00001000; // + (part * 0x100) for Part 1..16

// Patch Sub-blocks (p. 77-79)
constexpr uint32_t kPatchCommonOffset      = 0x00000000;
constexpr uint32_t kPatchTone1Offset       = 0x00001000;
constexpr uint32_t kPatchTone2Offset       = 0x00001200;
constexpr uint32_t kPatchTone3Offset       = 0x00001400;
constexpr uint32_t kPatchTone4Offset       = 0x00001600;

// Rhythm Sub-blocks (p. 80)
constexpr uint32_t kRhythmCommonOffset     = 0x00000000;
constexpr uint32_t kRhythmNoteFirstKey     = 35;
constexpr uint32_t kRhythmNoteLastKey      = 98;
constexpr size_t   kRhythmNoteCount        = 64;         // Keys 35..98 -> offsets 23 00..62 00

// Exact Linear Block Sizes & 7-bit RQ1 Sizes
constexpr size_t   kSystemCommonSize       = 0x66;       // 102 bytes
constexpr uint32_t kSystemCommonSize7Bit   = 0x00000066;

constexpr size_t   kScaleTuneSize          = 0x0C;       // 12 bytes
constexpr uint32_t kScaleTuneSize7Bit      = 0x0000000C;

constexpr size_t   kPerfCommonSize         = 0x42;       // 66 bytes
constexpr uint32_t kPerfCommonSize7Bit     = 0x00000042;

constexpr size_t   kPerfPartSize           = 0x19;       // 25 bytes
constexpr uint32_t kPerfPartSize7Bit       = 0x00000019;

constexpr size_t   kPatchCommonSize        = 0x4A;       // 74 bytes
constexpr uint32_t kPatchCommonSize7Bit    = 0x0000004A;

constexpr size_t   kPatchToneSize          = 129;        // 00 00 01 01 in 7-bit = 128 + 1 = 129 bytes
constexpr uint32_t kPatchToneSize7Bit      = 0x00000101;

constexpr size_t   kRhythmCommonSize       = 0x0C;       // 12 bytes
constexpr uint32_t kRhythmCommonSize7Bit   = 0x0000000C;

constexpr size_t   kRhythmNoteSize         = 0x3A;       // 58 bytes
constexpr uint32_t kRhythmNoteSize7Bit     = 0x0000003A;

// Counts
constexpr size_t kNumPerfParts             = 16;
constexpr size_t kNumPatchTones            = 4;
constexpr size_t kNumUserPerformances      = 32;
constexpr size_t kNumUserRhythmSets        = 2;
constexpr size_t kNumUserPatches           = 128;

// Helper to get base address for a Performance Part's Temporary Patch (0..15, skipping Part 10 / idx 9)
inline uint32_t getPerfTempPatchBase(int partIndexZeroBased) {
    if (partIndexZeroBased < 0 || partIndexZeroBased >= 16 || partIndexZeroBased == 9) {
        return kPatchModeTempPatchBase;
    }
    return 0x02000000u | (static_cast<uint32_t>(partIndexZeroBased) << 16);
}

inline uint32_t getUserPerfBase(int indexZeroBased) {
    return kUserPerformanceBase | (static_cast<uint32_t>(indexZeroBased & 0x1F) << 16);
}

inline uint32_t getUserRhythmBase(int indexZeroBased) {
    return kUserRhythmBase | (static_cast<uint32_t>(indexZeroBased & 0x01) << 16);
}

inline uint32_t getUserPatchBase(int indexZeroBased) {
    return kUserPatchBase | (static_cast<uint32_t>(indexZeroBased & 0x7F) << 16);
}

inline uint32_t getPatchToneOffset(int toneIndexZeroBased) {
    return kPatchTone1Offset + (static_cast<uint32_t>(toneIndexZeroBased & 0x03) * 0x0200u);
}

inline uint32_t getRhythmNoteOffset(int midiKey) {
    return static_cast<uint32_t>(midiKey & 0x7F) << 8;
}

// ============================================================================
// Offsets within System Common (1-1-1, p. 74-75)
// ============================================================================
namespace sys {
    constexpr size_t SoundMode             = 0x00; // 0:PERFORM, 1:PATCH, 2:GM
    constexpr size_t PerformanceNumber     = 0x01; // 0..127
    constexpr size_t PatchGroupType        = 0x02; // 0:USER&PRESET, 1:PCM, 2:EXP
    constexpr size_t PatchGroupID          = 0x03; // 0..127
    constexpr size_t PatchNumberNibbled    = 0x04; // # 2 bytes (0x04, 0x05)
    constexpr size_t MasterTune            = 0x06; // 0..126 (427.4 - 452.6 Hz)
    constexpr size_t ScaleTuneSwitch       = 0x07; // 0..1
    constexpr size_t EFXSwitch             = 0x08; // 0..1
    constexpr size_t ChorusSwitch          = 0x09; // 0..1
    constexpr size_t ReverbSwitch          = 0x0A; // 0..1
    constexpr size_t PatchRemain           = 0x0B; // 0..1
    constexpr size_t ClockSource           = 0x0C; // 0:INT, 1:MIDI
    constexpr size_t TAPControlSource      = 0x0D; // 0..4
    constexpr size_t HoldControlSource     = 0x0E; // 0..4
    constexpr size_t PeakControlSource     = 0x0F; // 0..4
    constexpr size_t VolumeControlSource   = 0x10; // 0..1
    constexpr size_t AftertouchSource      = 0x11; // 0..2
    constexpr size_t SystemControlSource1  = 0x12; // 1..97
    constexpr size_t SystemControlSource2  = 0x13; // 1..97
    constexpr size_t ReceiveProgramChange  = 0x14; // 0..1
    constexpr size_t ReceiveBankSelect     = 0x15; // 0..1
    constexpr size_t ControlChannel        = 0x1C; // 0..16 (1..16, OFF)
    constexpr size_t PatchReceiveChannel   = 0x1D; // 0..15 (1..16)
    constexpr size_t PreviewSoundMode      = 0x1F; // 0..2 (SINGLE, CHORD, PHRASE)
    constexpr size_t PreviewNoteSet1       = 0x20;
    constexpr size_t PreviewVelocitySet1   = 0x21;
    constexpr size_t PreviewNoteSet2       = 0x22;
    constexpr size_t PreviewVelocitySet2   = 0x23;
    constexpr size_t PreviewNoteSet3       = 0x24;
    constexpr size_t PreviewVelocitySet3   = 0x25;
    constexpr size_t PreviewNoteSet4       = 0x26;
    constexpr size_t PreviewVelocitySet4   = 0x27;
    constexpr size_t SystemTempoNibbled    = 0x60; // # 2 bytes (0x60, 0x61), 20..250
}

// ============================================================================
// Offsets within Performance Common (1-2-1, p. 76)
// ============================================================================
namespace perf_common {
    constexpr size_t Name                  = 0x00; // 12 bytes (0x00..0x0B)
    constexpr size_t EFXSource             = 0x0C; // 0..15 (PERFORM, 1-9, 11-16)
    constexpr size_t EFXType               = 0x0D; // 0..39 (1..40)
    constexpr size_t EFXParam1             = 0x0E; // 0x0E..0x19 (12 params)
    constexpr size_t EFXOutputAssign       = 0x1A; // 0..2
    constexpr size_t EFXMixOutSendLevel    = 0x1B;
    constexpr size_t EFXChorusSendLevel    = 0x1C;
    constexpr size_t EFXReverbSendLevel    = 0x1D;
    constexpr size_t EFXControlSource1     = 0x1E; // 0..10
    constexpr size_t EFXControlDepth1      = 0x1F; // 0..126 (-63..+63)
    constexpr size_t EFXControlSource2     = 0x20; // 0..10
    constexpr size_t EFXControlDepth2      = 0x21; // 0..126 (-63..+63)
    constexpr size_t ChorusLevel           = 0x22;
    constexpr size_t ChorusRate            = 0x23;
    constexpr size_t ChorusDepth           = 0x24;
    constexpr size_t ChorusPreDelay        = 0x25;
    constexpr size_t ChorusFeedback        = 0x26;
    constexpr size_t ChorusOutput          = 0x27; // 0..2
    constexpr size_t ReverbType            = 0x28; // 0..7
    constexpr size_t ReverbLevel           = 0x29;
    constexpr size_t ReverbTime            = 0x2A;
    constexpr size_t ReverbHFDamp          = 0x2B; // 0..17
    constexpr size_t DelayFeedback         = 0x2C;
    constexpr size_t TempoNibbled          = 0x2D; // # 2 bytes (0x2D, 0x2E)
    constexpr size_t KeyboardRangeSwitch   = 0x2F; // 0..1
    constexpr size_t VoiceReserve1         = 0x30; // 0x30..0x3F (16 parts, 0..64)
    constexpr size_t KeyboardMode          = 0x40; // 0:LAYER, 1:SINGLE
    constexpr size_t ClockSource           = 0x41; // 0:PERFORM, 1:SYSTEM
}

// ============================================================================
// Offsets within Performance Part (1-2-2, p. 77)
// ============================================================================
namespace perf_part {
    constexpr size_t ReceiveSwitch         = 0x00;
    constexpr size_t MidiChannel           = 0x01; // 0..15
    constexpr size_t PatchGroupType        = 0x02;
    constexpr size_t PatchGroupID          = 0x03;
    constexpr size_t PatchNumberNibbled    = 0x04; // # 2 bytes (0x04, 0x05)
    constexpr size_t PartLevel             = 0x06;
    constexpr size_t PartPan               = 0x07;
    constexpr size_t PartCoarseTune        = 0x08; // 0..96 (-48..+48)
    constexpr size_t PartFineTune          = 0x09; // 0..100 (-50..+50)
    constexpr size_t OutputAssign          = 0x0A; // 0..4
    constexpr size_t MixEFXSendLevel       = 0x0B;
    constexpr size_t ChorusSendLevel       = 0x0C;
    constexpr size_t ReverbSendLevel       = 0x0D;
    constexpr size_t ReceiveProgramChange  = 0x0E;
    constexpr size_t ReceiveVolume         = 0x0F;
    constexpr size_t ReceiveHold1          = 0x10;
    constexpr size_t KeyboardRangeLower    = 0x11;
    constexpr size_t KeyboardRangeUpper    = 0x12;
    constexpr size_t OctaveShift           = 0x13; // 0..6 (-3..+3)
}

// ============================================================================
// Offsets within Patch Common (1-3-1, p. 77-78)
// ============================================================================
namespace patch_common {
    constexpr size_t Name                  = 0x00; // 12 bytes (0x00..0x0B)
    constexpr size_t EFXType               = 0x0C; // 0..39
    constexpr size_t EFXParam1             = 0x0D; // 0x0D..0x18 (12 params)
    constexpr size_t EFXOutputAssign       = 0x19; // 0..2
    constexpr size_t EFXMixOutSendLevel    = 0x1A;
    constexpr size_t EFXChorusSendLevel    = 0x1B;
    constexpr size_t EFXReverbSendLevel    = 0x1C;
    constexpr size_t EFXControlSource1     = 0x1D;
    constexpr size_t EFXControlDepth1      = 0x1E;
    constexpr size_t EFXControlSource2     = 0x1F;
    constexpr size_t EFXControlDepth2      = 0x20;
    constexpr size_t ChorusLevel           = 0x21;
    constexpr size_t ChorusRate            = 0x22;
    constexpr size_t ChorusDepth           = 0x23;
    constexpr size_t ChorusPreDelay        = 0x24;
    constexpr size_t ChorusFeedback        = 0x25;
    constexpr size_t ChorusOutput          = 0x26;
    constexpr size_t ReverbType            = 0x27;
    constexpr size_t ReverbLevel           = 0x28;
    constexpr size_t ReverbTime            = 0x29;
    constexpr size_t ReverbHFDamp          = 0x2A;
    constexpr size_t DelayFeedback         = 0x2B;
    constexpr size_t TempoNibbled          = 0x2C; // # 2 bytes (0x2C, 0x2D)
    constexpr size_t PatchLevel            = 0x2E;
    constexpr size_t PatchPan              = 0x2F;
    constexpr size_t AnalogFeel            = 0x30;
    constexpr size_t BendRangeUp           = 0x31; // 0..12
    constexpr size_t BendRangeDown         = 0x32; // 0..48
    constexpr size_t KeyAssignMode         = 0x33; // 0:POLY, 1:SOLO
    constexpr size_t SoloLegato            = 0x34;
    constexpr size_t PortamentoSwitch      = 0x35;
    constexpr size_t PortamentoMode        = 0x36;
    constexpr size_t PortamentoType        = 0x37;
    constexpr size_t PortamentoStart       = 0x38;
    constexpr size_t PortamentoTime        = 0x39;
    constexpr size_t PatchControlSource2   = 0x3A;
    constexpr size_t PatchControlSource3   = 0x3B;
    constexpr size_t EFXControlHoldPeak    = 0x3C;
    constexpr size_t Control1HoldPeak      = 0x3D;
    constexpr size_t Control2HoldPeak      = 0x3E;
    constexpr size_t Control3HoldPeak      = 0x3F;
    constexpr size_t VelocityRangeSwitch   = 0x40;
    constexpr size_t OctaveShift           = 0x41; // 0..6 (-3..+3)
    constexpr size_t StretchTuneDepth      = 0x42; // 0..3
    constexpr size_t VoicePriority         = 0x43; // 0:LAST, 1:LOUDEST
    constexpr size_t StructureType1_2      = 0x44; // 0..9 (1..10)
    constexpr size_t Booster1_2            = 0x45; // 0..3 (0,+6,+12,+18)
    constexpr size_t StructureType3_4      = 0x46; // 0..9 (1..10)
    constexpr size_t Booster3_4            = 0x47; // 0..3
    constexpr size_t ClockSource           = 0x48; // 0:PATCH, 1:SYSTEM
    constexpr size_t PatchCategory         = 0x49; // 0..38
}

// ============================================================================
// Linear Offsets within Patch Tone (1-3-2, p. 78-79, 129 bytes: 0..128)
// Note: Linear index 128 corresponds to MIDI address offset 01 00!
// ============================================================================
namespace patch_tone {
    constexpr size_t ToneSwitch            = 0x00;
    constexpr size_t WaveGroupType         = 0x01; // 0:INT, 1:PCM, 2:EXP
    constexpr size_t WaveGroupID           = 0x02;
    constexpr size_t WaveNumberNibbled     = 0x03; // # 2 bytes (0x03, 0x04)
    constexpr size_t WaveGain              = 0x05; // 0..3 (-6,0,+6,+12)
    constexpr size_t FXMSwitch             = 0x06;
    constexpr size_t FXMColor              = 0x07;
    constexpr size_t FXMDepth              = 0x08;
    constexpr size_t ToneDelayMode         = 0x09;
    constexpr size_t ToneDelayTime         = 0x0A;
    constexpr size_t VelocityCrossFade     = 0x0B;
    constexpr size_t VelocityRangeLower    = 0x0C;
    constexpr size_t VelocityRangeUpper    = 0x0D;
    constexpr size_t KeyboardRangeLower    = 0x0E;
    constexpr size_t KeyboardRangeUpper    = 0x0F;
    constexpr size_t RedamperSwitch        = 0x10;
    constexpr size_t VolumeSwitch          = 0x11;
    constexpr size_t Hold1Switch           = 0x12;
    constexpr size_t PitchBendSwitch       = 0x13;
    constexpr size_t PanControlSwitch      = 0x14;
    constexpr size_t Ctrl1Dest1            = 0x15; // 0x15..0x2C (3 ctrls * 4 dest/depth pairs)
    constexpr size_t LFO1Waveform          = 0x2D;
    constexpr size_t LFO1KeySync           = 0x2E;
    constexpr size_t LFO1Rate              = 0x2F;
    constexpr size_t LFO1Offset            = 0x30;
    constexpr size_t LFO1DelayTime         = 0x31;
    constexpr size_t LFO1FadeMode          = 0x32;
    constexpr size_t LFO1FadeTime          = 0x33;
    constexpr size_t LFO1ExternalSync      = 0x34;
    constexpr size_t LFO2Waveform          = 0x35;
    constexpr size_t LFO2KeySync           = 0x36;
    constexpr size_t LFO2Rate              = 0x37;
    constexpr size_t LFO2Offset            = 0x38;
    constexpr size_t LFO2DelayTime         = 0x39;
    constexpr size_t LFO2FadeMode          = 0x3A;
    constexpr size_t LFO2FadeTime          = 0x3B;
    constexpr size_t LFO2ExternalSync      = 0x3C;
    constexpr size_t CoarseTune            = 0x3D; // 0..96 (-48..+48)
    constexpr size_t FineTune              = 0x3E; // 0..100 (-50..+50)
    constexpr size_t RandomPitchDepth      = 0x3F;
    constexpr size_t PitchKeyfollow        = 0x40;
    constexpr size_t PitchEnvDepth         = 0x41; // 0..24 (-12..+12)
    constexpr size_t PitchEnvVelSens       = 0x42;
    constexpr size_t PitchEnvVelTime1      = 0x43;
    constexpr size_t PitchEnvVelTime4      = 0x44;
    constexpr size_t PitchEnvTimeKeyfollow = 0x45;
    constexpr size_t PitchEnvTime1         = 0x46;
    constexpr size_t PitchEnvTime2         = 0x47;
    constexpr size_t PitchEnvTime3         = 0x48;
    constexpr size_t PitchEnvTime4         = 0x49;
    constexpr size_t PitchEnvLevel1        = 0x4A;
    constexpr size_t PitchEnvLevel2        = 0x4B;
    constexpr size_t PitchEnvLevel3        = 0x4C;
    constexpr size_t PitchEnvLevel4        = 0x4D;
    constexpr size_t PitchLFO1Depth        = 0x4E;
    constexpr size_t PitchLFO2Depth        = 0x4F;
    constexpr size_t FilterType            = 0x50; // 0..4 (OFF,LPF,BPF,HPF,PKG)
    constexpr size_t CutoffFrequency       = 0x51;
    constexpr size_t CutoffKeyfollow       = 0x52;
    constexpr size_t Resonance             = 0x53;
    constexpr size_t ResonanceVelSens      = 0x54;
    constexpr size_t FilterEnvDepth        = 0x55;
    constexpr size_t FilterEnvVelCurve     = 0x56;
    constexpr size_t FilterEnvVelSens      = 0x57;
    constexpr size_t FilterEnvVelTime1     = 0x58;
    constexpr size_t FilterEnvVelTime4     = 0x59;
    constexpr size_t FilterEnvTimeKeyfollow= 0x5A;
    constexpr size_t FilterEnvTime1        = 0x5B;
    constexpr size_t FilterEnvTime2        = 0x5C;
    constexpr size_t FilterEnvTime3        = 0x5D;
    constexpr size_t FilterEnvTime4        = 0x5E;
    constexpr size_t FilterEnvLevel1       = 0x5F;
    constexpr size_t FilterEnvLevel2       = 0x60;
    constexpr size_t FilterEnvLevel3       = 0x61;
    constexpr size_t FilterEnvLevel4       = 0x62;
    constexpr size_t FilterLFO1Depth       = 0x63;
    constexpr size_t FilterLFO2Depth       = 0x64;
    constexpr size_t ToneLevel             = 0x65;
    constexpr size_t BiasDirection         = 0x66;
    constexpr size_t BiasPosition          = 0x67;
    constexpr size_t BiasLevel             = 0x68;
    constexpr size_t LevelEnvVelCurve      = 0x69;
    constexpr size_t LevelEnvVelSens       = 0x6A;
    constexpr size_t LevelEnvVelTime1      = 0x6B;
    constexpr size_t LevelEnvVelTime4      = 0x6C;
    constexpr size_t LevelEnvTimeKeyfollow = 0x6D;
    constexpr size_t LevelEnvTime1         = 0x6E;
    constexpr size_t LevelEnvTime2         = 0x6F;
    constexpr size_t LevelEnvTime3         = 0x70;
    constexpr size_t LevelEnvTime4         = 0x71;
    constexpr size_t LevelEnvLevel1        = 0x72;
    constexpr size_t LevelEnvLevel2        = 0x73;
    constexpr size_t LevelEnvLevel3        = 0x74;
    constexpr size_t LevelLFO1Depth        = 0x75;
    constexpr size_t LevelLFO2Depth        = 0x76;
    constexpr size_t TonePan               = 0x77;
    constexpr size_t PanKeyfollow          = 0x78;
    constexpr size_t RandomPanDepth        = 0x79;
    constexpr size_t AlternatePanDepth     = 0x7A;
    constexpr size_t PanLFO1Depth          = 0x7B;
    constexpr size_t PanLFO2Depth          = 0x7C;
    constexpr size_t OutputAssign          = 0x7D;
    constexpr size_t MixEFXSendLevel       = 0x7E;
    constexpr size_t ChorusSendLevel       = 0x7F;
    constexpr size_t ReverbSendLevel       = 0x80; // Offset 01 00 in 7-bit MIDI address!
}

// ============================================================================
// Offsets within Rhythm Note (1-4-2, p. 80, 58 bytes)
// ============================================================================
namespace rhythm_note {
    constexpr size_t ToneSwitch            = 0x00;
    constexpr size_t WaveGroupType         = 0x01;
    constexpr size_t WaveGroupID           = 0x02;
    constexpr size_t WaveNumberNibbled     = 0x03; // # 2 bytes
    constexpr size_t WaveGain              = 0x05;
    constexpr size_t BendRange             = 0x06;
    constexpr size_t MuteGroup             = 0x07;
    constexpr size_t EnvelopeMode          = 0x08;
    constexpr size_t VolumeSwitch          = 0x09;
    constexpr size_t Hold1Switch           = 0x0A;
    constexpr size_t PanControlSwitch      = 0x0B;
    constexpr size_t CoarseTune            = 0x0C; // 0..127 (C-1..G9)
    constexpr size_t FineTune              = 0x0D; // 0..100 (-50..+50)
    constexpr size_t RandomPitchDepth      = 0x0E;
    constexpr size_t PitchEnvDepth         = 0x0F;
    constexpr size_t PitchEnvVelSens       = 0x10;
    constexpr size_t PitchEnvVelTime       = 0x11;
    constexpr size_t PitchEnvTime1         = 0x12; // T1..T4: 0x12..0x15
    constexpr size_t PitchEnvTime2         = 0x13;
    constexpr size_t PitchEnvTime3         = 0x14;
    constexpr size_t PitchEnvTime4         = 0x15;
    constexpr size_t PitchEnvLevel1        = 0x16; // L1..L4: 0x16..0x19
    constexpr size_t PitchEnvLevel2        = 0x17;
    constexpr size_t PitchEnvLevel3        = 0x18;
    constexpr size_t PitchEnvLevel4        = 0x19;
    constexpr size_t FilterType            = 0x1A;
    constexpr size_t CutoffFrequency       = 0x1B;
    constexpr size_t Resonance             = 0x1C;
    constexpr size_t ResonanceVelSens      = 0x1D;
    constexpr size_t FilterEnvDepth        = 0x1E;
    constexpr size_t FilterEnvVelSens      = 0x1F;
    constexpr size_t FilterEnvVelTime      = 0x20;
    constexpr size_t FilterEnvTime1        = 0x21; // T1..T4: 0x21..0x24
    constexpr size_t FilterEnvTime2        = 0x22;
    constexpr size_t FilterEnvTime3        = 0x23;
    constexpr size_t FilterEnvTime4        = 0x24;
    constexpr size_t FilterEnvLevel1       = 0x25; // L1..L4: 0x25..0x28
    constexpr size_t FilterEnvLevel2       = 0x26;
    constexpr size_t FilterEnvLevel3       = 0x27;
    constexpr size_t FilterEnvLevel4       = 0x28;
    constexpr size_t ToneLevel             = 0x29;
    constexpr size_t LevelEnvVelSens       = 0x2A;
    constexpr size_t LevelEnvVelTime       = 0x2B;
    constexpr size_t LevelEnvTime1         = 0x2C; // T1..T4: 0x2C..0x2F
    constexpr size_t LevelEnvTime2         = 0x2D;
    constexpr size_t LevelEnvTime3         = 0x2E;
    constexpr size_t LevelEnvTime4         = 0x2F;
    constexpr size_t LevelEnvLevel1        = 0x30; // L1..L3: 0x30..0x32
    constexpr size_t LevelEnvLevel2        = 0x31;
    constexpr size_t LevelEnvLevel3        = 0x32;
    constexpr size_t TonePan               = 0x33;
    constexpr size_t RandomPanDepth        = 0x34;
    constexpr size_t AlternatePanDepth     = 0x35;
    constexpr size_t OutputAssign          = 0x36;
    constexpr size_t MixEFXSendLevel       = 0x37;
    constexpr size_t ChorusSendLevel       = 0x38;
    constexpr size_t ReverbSendLevel       = 0x39;
}

} // namespace addr
} // namespace jv1010
