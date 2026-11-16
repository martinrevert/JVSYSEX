#pragma once

#include "../Model/JV1010Types.h"
#include <string>

namespace jv1010 {

int getWaveformCount(WaveGroup group, int installedBoardMaxWaves = 255);
std::string getWaveformName(WaveGroup group, int waveNumberOneBased);

} // namespace jv1010
