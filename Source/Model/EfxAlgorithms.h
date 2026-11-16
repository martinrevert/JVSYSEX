#pragma once

#include <array>
#include <string>

namespace jv1010 {

struct EfxParamDef {
    const char* name = "(not used)";
    int minVal = 0;
    int maxVal = 127;
    bool used = false;
};

struct EfxAlgorithmDef {
    int typeIndex = 0; // 0..39 (Type01..Type40)
    const char* name = "";
    std::array<EfxParamDef, 12> params;
};

const std::array<EfxAlgorithmDef, 40>& getEfxAlgorithms();
const EfxAlgorithmDef& getEfxAlgorithm(int typeIndex);

} // namespace jv1010
