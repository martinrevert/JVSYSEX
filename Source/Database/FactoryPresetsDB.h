#pragma once

#include "../Model/JV1010Types.h"
#include <array>
#include <string>
#include <vector>

namespace jv1010 {

struct PatchCatalogEntry {
    PatchBank bank;
    int indexZeroBased; // 0..127 or 0..254
    const char* name;
    int voices;
    bool solo; // false=POLY, true=SOLO
    int categoryIndex; // 0..38 (from Patch Common 00 49, p. 78)
};

struct PerfCatalogEntry {
    PerfBank bank;
    int indexZeroBased; // 0..31
    const char* name;
    bool isLayer; // false=SINGLE, true=LAYER
};

struct RhythmCatalogEntry {
    RhythmBank bank;
    int indexZeroBased;
    const char* name;
};

struct CategoryInfo {
    int id; // 0..38
    const char* shortCode;
    const char* name;
    const char* macroGroup; // PIANO, KEY&ORGAN, GUITAR/BASS, ORCH/BRASS, SYNTH/PAD, ETHNIC, RHYTHM&SFX
};

const std::array<CategoryInfo, 39>& getPatchCategories();
const std::vector<PatchCatalogEntry>& getFactoryPatches();
const std::vector<PerfCatalogEntry>& getFactoryPerformances();
const std::vector<RhythmCatalogEntry>& getFactoryRhythmSets();

std::string getFactoryPatchName(PatchBank bank, int indexZeroBased);
std::string getFactoryPerfName(PerfBank bank, int indexZeroBased);
std::string getFactoryRhythmName(RhythmBank bank, int indexZeroBased);

} // namespace jv1010
