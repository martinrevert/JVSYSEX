#pragma once

#include "../Model/JV1010State.h"

namespace jv1010 {

struct RandomizeMask {
    bool randomizeFilter = true;
    bool randomizeEnvelopes = true;
    bool randomizeLFOs = true;
    bool randomizePitch = false;
    bool randomizeEffects = true;
    float amount = 0.5f; // 0..1
};

class PatchTools {
public:
    static void copyTone(PatchData& dst, int dstToneIdx, const PatchData& src, int srcToneIdx);
    static void randomizePatch(PatchData& patch, const RandomizeMask& mask);
    static PatchData morphPatches(const PatchData& a, const PatchData& b, float t);
};

} // namespace jv1010
