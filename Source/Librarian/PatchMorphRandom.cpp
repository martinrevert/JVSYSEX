#include "PatchMorphRandom.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace jv1010 {

void PatchTools::copyTone(PatchData& dst, int dstToneIdx, const PatchData& src, int srcToneIdx) {
    if (dstToneIdx < 0 || dstToneIdx >= 4 || srcToneIdx < 0 || srcToneIdx >= 4) return;
    dst.tones[static_cast<size_t>(dstToneIdx)] = src.tones[static_cast<size_t>(srcToneIdx)];
}

void PatchTools::randomizePatch(PatchData& patch, const RandomizeMask& mask) {
    static std::mt19937 rng{ std::random_device{}() };
    auto randRange = [&](int current, int mn, int mx) -> uint8_t {
        std::uniform_int_distribution<int> dist(mn, mx);
        int target = dist(rng);
        int blended = static_cast<int>(std::round(current + (target - current) * std::clamp(mask.amount, 0.0f, 1.0f)));
        return static_cast<uint8_t>(std::clamp(blended, mn, mx));
    };

    if (mask.randomizeEffects) {
        patch.common[addr::patch_common::ChorusLevel] = randRange(patch.common[addr::patch_common::ChorusLevel], 0, 127);
        patch.common[addr::patch_common::ChorusRate]  = randRange(patch.common[addr::patch_common::ChorusRate], 5, 110);
        patch.common[addr::patch_common::ChorusDepth] = randRange(patch.common[addr::patch_common::ChorusDepth], 0, 100);
        patch.common[addr::patch_common::ReverbLevel] = randRange(patch.common[addr::patch_common::ReverbLevel], 10, 110);
        patch.common[addr::patch_common::ReverbTime]  = randRange(patch.common[addr::patch_common::ReverbTime], 20, 115);
    }

    for (size_t t = 0; t < addr::kNumPatchTones; ++t) {
        auto& tone = patch.tones[t];
        if (tone[addr::patch_tone::ToneSwitch] == 0) continue;

        if (mask.randomizeFilter) {
            tone[addr::patch_tone::CutoffFrequency] = randRange(tone[addr::patch_tone::CutoffFrequency], 25, 127);
            tone[addr::patch_tone::Resonance]       = randRange(tone[addr::patch_tone::Resonance], 0, 95);
            tone[addr::patch_tone::FilterEnvDepth]  = randRange(tone[addr::patch_tone::FilterEnvDepth], 20, 106);
        }
        if (mask.randomizeEnvelopes) {
            for (size_t i = 0; i < 4; ++i) {
                tone[addr::patch_tone::FilterEnvTime1 + i] = randRange(tone[addr::patch_tone::FilterEnvTime1 + i], 0, 100);
                tone[addr::patch_tone::LevelEnvTime1 + i]  = randRange(tone[addr::patch_tone::LevelEnvTime1 + i], 0, 100);
            }
            tone[addr::patch_tone::LevelEnvLevel1] = randRange(tone[addr::patch_tone::LevelEnvLevel1], 60, 127);
            tone[addr::patch_tone::LevelEnvLevel2] = randRange(tone[addr::patch_tone::LevelEnvLevel2], 40, 127);
            tone[addr::patch_tone::LevelEnvLevel3] = randRange(tone[addr::patch_tone::LevelEnvLevel3], 20, 127);
        }
        if (mask.randomizeLFOs) {
            tone[addr::patch_tone::LFO1Waveform]   = randRange(tone[addr::patch_tone::LFO1Waveform], 0, 5);
            tone[addr::patch_tone::LFO1Rate]       = randRange(tone[addr::patch_tone::LFO1Rate], 10, 100);
            tone[addr::patch_tone::FilterLFO1Depth]= randRange(tone[addr::patch_tone::FilterLFO1Depth], 40, 86);
            tone[addr::patch_tone::PanLFO1Depth]   = randRange(tone[addr::patch_tone::PanLFO1Depth], 40, 86);
        }
        if (mask.randomizePitch) {
            tone[addr::patch_tone::FineTune] = randRange(tone[addr::patch_tone::FineTune], 35, 65);
            tone[addr::patch_tone::RandomPitchDepth] = randRange(tone[addr::patch_tone::RandomPitchDepth], 0, 12);
        }
    }
}

PatchData PatchTools::morphPatches(const PatchData& a, const PatchData& b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    PatchData out = (t < 0.5f) ? a : b;

    auto lerpByte = [t](uint8_t va, uint8_t vb) -> uint8_t {
        return static_cast<uint8_t>(std::clamp<int>(
            static_cast<int>(std::round(va + (vb - va) * t)), 0, 127));
    };

    // Morph continuous common parameters (avoiding discrete name & EFX type bytes)
    out.common[addr::patch_common::PatchLevel] = lerpByte(a.common[addr::patch_common::PatchLevel], b.common[addr::patch_common::PatchLevel]);
    out.common[addr::patch_common::PatchPan]   = lerpByte(a.common[addr::patch_common::PatchPan],   b.common[addr::patch_common::PatchPan]);
    out.common[addr::patch_common::AnalogFeel] = lerpByte(a.common[addr::patch_common::AnalogFeel], b.common[addr::patch_common::AnalogFeel]);
    out.common[addr::patch_common::ChorusLevel]= lerpByte(a.common[addr::patch_common::ChorusLevel],b.common[addr::patch_common::ChorusLevel]);
    out.common[addr::patch_common::ChorusRate] = lerpByte(a.common[addr::patch_common::ChorusRate], b.common[addr::patch_common::ChorusRate]);
    out.common[addr::patch_common::ChorusDepth]= lerpByte(a.common[addr::patch_common::ChorusDepth],b.common[addr::patch_common::ChorusDepth]);
    out.common[addr::patch_common::ReverbLevel]= lerpByte(a.common[addr::patch_common::ReverbLevel],b.common[addr::patch_common::ReverbLevel]);
    out.common[addr::patch_common::ReverbTime] = lerpByte(a.common[addr::patch_common::ReverbTime], b.common[addr::patch_common::ReverbTime]);

    for (size_t i = 0; i < addr::kNumPatchTones; ++i) {
        const auto& ta = a.tones[i];
        const auto& tb = b.tones[i];
        auto& to = out.tones[i];
        // Morph continuous tone parameters: LFO rates, Pitch/Filter/Level envelopes, Cutoff, Resonance, Level, Pan, Sends
        for (size_t off = addr::patch_tone::CoarseTune; off <= addr::patch_tone::ReverbSendLevel; ++off) {
            if (off == addr::patch_tone::FilterType || off == addr::patch_tone::BiasDirection || off == addr::patch_tone::OutputAssign)
                continue;
            to[off] = lerpByte(ta[off], tb[off]);
        }
        to[addr::patch_tone::LFO1Rate] = lerpByte(ta[addr::patch_tone::LFO1Rate], tb[addr::patch_tone::LFO1Rate]);
        to[addr::patch_tone::LFO2Rate] = lerpByte(ta[addr::patch_tone::LFO2Rate], tb[addr::patch_tone::LFO2Rate]);
    }
    return out;
}

} // namespace jv1010
