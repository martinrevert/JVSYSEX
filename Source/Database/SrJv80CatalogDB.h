#pragma once

#include <string>
#include <vector>

namespace jv1010 {

struct SrJv80BoardInfo {
    int groupId;          // 1..19 (matches SysEx Patch/Wave Group ID when Group Type == 2)
    const char* model;    // e.g. "SR-JV80-04"
    const char* title;    // e.g. "Vintage Synth"
    int patchCount;       // up to 256
    int rhythmCount;      // up to 16
    int waveCount;        // up to 255
    std::vector<const char*> samplePatchNames; // First patches + key patches for identification & browsing
};

const std::vector<SrJv80BoardInfo>& getSrJv80Catalog();
const SrJv80BoardInfo* findSrJv80ByGroupId(int groupId);
const SrJv80BoardInfo* findSrJv80ByFirstPatchName(const std::string& patch001Name);

} // namespace jv1010
