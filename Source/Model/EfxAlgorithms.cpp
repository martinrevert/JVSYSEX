#include "EfxAlgorithms.h"
#include <algorithm>

namespace jv1010 {

namespace {
constexpr EfxParamDef P(const char* n, int mn, int mx) { return { n, mn, mx, true }; }
constexpr EfxParamDef U() { return { "(not used)", 0, 127, false }; }
} // namespace

const std::array<EfxAlgorithmDef, 40>& getEfxAlgorithms() {
    static const std::array<EfxAlgorithmDef, 40> kAlgorithms = {{
        { 0, "01: STEREO-EQ", {{
            P("Low Frequency", 0, 1), P("Low Gain", 0, 30), P("High Frequency", 0, 1), P("Hi Gain", 0, 30),
            P("Peaking1 Freq", 0, 16), P("Peaking1 Q", 0, 4), P("Peaking1 Gain", 0, 30),
            P("Peaking2 Freq", 0, 16), P("Peaking2 Q", 0, 4), P("Peaking2 Gain", 0, 30),
            P("Level", 0, 127), U()
        }}},
        { 1, "02: OVERDRIVE", {{
            P("Drive", 0, 127), P("Output Pan", 0, 127), P("Amp Sim Type", 0, 3), P("Low Gain", 0, 30),
            P("High Gain", 0, 30), P("Output Level", 0, 127), U(), U(), U(), U(), U(), U()
        }}},
        { 2, "03: DISTORTION", {{
            P("Drive", 0, 127), P("Output Pan", 0, 127), P("Amp Sim Type", 0, 3), P("Low Gain", 0, 30),
            P("High Gain", 0, 30), P("Output Level", 0, 127), U(), U(), U(), U(), U(), U()
        }}},
        { 3, "04: PHASER", {{
            P("Manual", 0, 125), P("Rate", 0, 125), P("Depth", 0, 127), P("Resonance", 0, 127),
            P("Mix Level", 0, 127), P("Output Pan", 0, 127), P("Output Level", 0, 127), U(), U(), U(), U(), U()
        }}},
        { 4, "05: SPECTRUM", {{
            P("Band1 Gain", 0, 30), P("Band2 Gain", 0, 30), P("Band3 Gain", 0, 30), P("Band4 Gain", 0, 30),
            P("Band5 Gain", 0, 30), P("Band6 Gain", 0, 30), P("Band7 Gain", 0, 30), P("Band8 Gain", 0, 30),
            P("Q", 0, 4), P("Output Pan", 0, 127), P("Output Level", 0, 127), U()
        }}},
        { 5, "06: ENHANCER", {{
            P("Sens", 0, 127), P("Mix Level", 0, 127), P("Low Gain", 0, 30), P("High Gain", 0, 30),
            P("Output Level", 0, 127), U(), U(), U(), U(), U(), U(), U()
        }}},
        { 6, "07: AUTO-WAH", {{
            P("Filter Type", 0, 1), P("Rate", 0, 125), P("Depth", 0, 127), P("Sens", 0, 127),
            P("Manual", 0, 127), P("Peak", 0, 127), P("Output Level", 0, 127), U(), U(), U(), U(), U()
        }}},
        { 7, "08: ROTARY", {{
            P("HF Slow Rate", 0, 125), P("LF Slow Rate", 0, 125), P("HF Fast Rate", 0, 125), P("LF Fast Rate", 0, 125),
            P("Speed", 0, 1), P("HF Accel", 0, 15), P("LF Accel", 0, 15), P("HF Level", 0, 127),
            P("LF Level", 0, 127), P("Separation", 0, 127), P("Output Level", 0, 127), U()
        }}},
        { 8, "09: COMPRESSOR", {{
            P("Sustain", 0, 127), P("Attack", 0, 127), P("Output Pan", 0, 127), P("Post Gain", 0, 3),
            P("Low Gain", 0, 30), P("High Gain", 0, 30), P("Output Level", 0, 127), U(), U(), U(), U(), U()
        }}},
        { 9, "10: LIMITER", {{
            P("Threshold", 0, 127), P("Release Time", 0, 127), P("Comp Ratio", 0, 3), P("Output Pan", 0, 127),
            P("Post Gain", 0, 3), P("Low Gain", 0, 30), P("High Gain", 0, 30), P("Output Level", 0, 127),
            U(), U(), U(), U()
        }}},
        { 10, "11: HEXA-CHORUS", {{
            P("Pre Delay", 0, 125), P("Rate", 0, 125), P("Depth", 0, 127), P("PreDly Dev", 0, 20),
            P("Depth Dev", 0, 40), P("Pan Dev", 0, 20), P("Effect Balance", 0, 100), P("Output Level", 0, 127),
            U(), U(), U(), U()
        }}},
        { 11, "12: TREMOLO-CHORUS", {{
            P("Pre Delay", 0, 125), P("Chorus Rate", 0, 125), P("Chorus Depth", 0, 127), P("Tremolo Rate", 0, 125),
            P("Tremolo Sep", 0, 127), P("Tremolo Phase", 0, 90), P("Effect Balance", 0, 100), P("Output Level", 0, 127),
            U(), U(), U(), U()
        }}},
        { 12, "13: SPACE-D", {{
            P("Pre Delay", 0, 125), P("Rate", 0, 125), P("Depth", 0, 127), P("Phase", 0, 90),
            P("Low Gain", 0, 30), P("High Gain", 0, 30), P("Effect Balance", 0, 100), P("Output Level", 0, 127),
            U(), U(), U(), U()
        }}},
        { 13, "14: STEREO-CHORUS", {{
            P("Filter Type", 0, 2), P("Cutoff Freq", 0, 16), P("Pre Delay", 0, 125), P("Rate", 0, 125),
            P("Depth", 0, 127), P("Phase", 0, 90), U(), P("Low Gain", 0, 30),
            P("High Gain", 0, 30), P("Effect Balance", 0, 100), P("Output Level", 0, 127), U()
        }}},
        { 14, "15: STEREO-FLANGER", {{
            P("Filter Type", 0, 2), P("Cutoff Freq", 0, 16), P("Pre Delay", 0, 125), P("Rate", 0, 125),
            P("Depth", 0, 127), P("Phase", 0, 90), P("Feedback", 0, 98), P("Low Gain", 0, 30),
            P("High Gain", 0, 30), P("Effect Balance", 0, 100), P("Output Level", 0, 127), U()
        }}},
        { 15, "16: STEP-FLANGER", {{
            P("Pre Delay", 0, 125), P("Rate", 0, 125), P("Depth", 0, 127), P("Feedback", 0, 98),
            P("Step Rate", 0, 125), P("Phase", 0, 90), P("Low Gain", 0, 30), P("High Gain", 0, 30),
            P("Effect Balance", 0, 100), P("Output Level", 0, 127), U(), U()
        }}},
        { 16, "17: STEREO-DELAY", {{
            P("Feedback Mode", 0, 1), P("Delay Time L", 0, 126), P("Delay Time R", 0, 126), P("Fbk Phase L", 0, 1),
            P("Fbk Phase R", 0, 1), P("Feedback", 0, 98), P("HF Damp", 0, 17), P("Low Gain", 0, 30),
            P("High Gain", 0, 30), P("Effect Balance", 0, 100), P("Output Level", 0, 127), U()
        }}},
        { 17, "18: MODULATION-DELAY", {{
            P("Feedback Mode", 0, 1), P("Delay Time L", 0, 126), P("Delay Time R", 0, 126), P("Feedback", 0, 98),
            P("HF Damp", 0, 17), P("Rate", 0, 125), P("Depth", 0, 127), P("Phase", 0, 90),
            P("Low Gain", 0, 30), P("High Gain", 0, 30), P("Effect Balance", 0, 100), P("Output Level", 0, 127)
        }}},
        { 18, "19: TRIPLE-TAP-DELAY", {{
            P("Delay Time L", 0, 125), P("Delay Time R", 0, 125), P("Delay Time C", 0, 125), P("Feedback", 0, 98),
            P("HF Damp", 0, 17), P("Left Level", 0, 127), P("Right Level", 0, 127), P("Center Level", 0, 127),
            P("Low Gain", 0, 30), P("High Gain", 0, 30), P("Effect Balance", 0, 100), P("Output Level", 0, 127)
        }}},
        { 19, "20: QUADRUPLE-TAP-DELAY", {{
            P("Delay Time 1", 0, 125), P("Delay Time 2", 0, 125), P("Delay Time 3", 0, 125), P("Delay Time 4", 0, 125),
            P("Level 1", 0, 127), P("Level 2", 0, 127), P("Level 3", 0, 127), P("Level 4", 0, 127),
            P("Feedback", 0, 98), P("HF Damp", 0, 17), P("Effect Balance", 0, 100), P("Output Level", 0, 127)
        }}},
        { 20, "21: TIME-CONTROL-DELAY", {{
            P("Delay Time", 0, 120), P("Feedback", 0, 98), P("Acceleration", 0, 15), P("HF Damp", 0, 17),
            P("Output Pan", 0, 127), P("Low Gain", 0, 30), P("High Gain", 0, 30), P("Effect Balance", 0, 100),
            P("Output Level", 0, 127), U(), U(), U()
        }}},
        { 21, "22: 2VOICE-PITCH-SHIFTER", {{
            P("Mode", 0, 4), P("Coarse Pitch A", 0, 36), P("Coarse Pitch B", 0, 36), P("Fine Pitch A", 0, 100),
            P("Fine Pitch B", 0, 100), P("Pre Delay A", 0, 126), P("Pre Delay B", 0, 126), P("Output Pan A", 0, 127),
            P("Output Pan B", 0, 127), P("Level Balance", 0, 100), P("Effect Balance", 0, 100), P("Output Level", 0, 127)
        }}},
        { 22, "23: FBK-PITCH-SHIFTER", {{
            P("Mode", 0, 4), P("Coarse Pitch", 0, 36), P("Fine Pitch", 0, 100), P("Pre Delay", 0, 126),
            P("Feedback", 0, 98), P("Output Pan", 0, 127), P("Low Gain", 0, 30), P("High Gain", 0, 30),
            P("Effect Balance", 0, 100), P("Output Level", 0, 127), U(), U()
        }}},
        { 23, "24: REVERB", {{
            P("Reverb Type", 0, 5), P("Pre Delay", 0, 125), P("Gate Time", 0, 127), P("HF Damp", 0, 17),
            P("Low Gain", 0, 30), P("High Gain", 0, 30), P("Effect Balance", 0, 100), P("Output Level", 0, 127),
            U(), U(), U(), U()
        }}},
        { 24, "25: GATE-REVERB", {{
            P("Gate-Rev Type", 0, 3), P("Pre Delay", 0, 125), P("Gate Time", 0, 99), P("Low Gain", 0, 30),
            P("High Gain", 0, 30), P("Effect Balance", 0, 100), P("Output Level", 0, 127), U(), U(), U(), U(), U()
        }}},
        { 25, "26: OVERDRIVE->CHORUS", {{
            P("Drive", 0, 127), P("Over Drive Pan", 0, 127), P("Cho PreDelay", 0, 125), P("Cho Rate", 0, 125),
            P("Cho Depth", 0, 127), U(), P("Cho Balance", 0, 100), P("Output Level", 0, 127), U(), U(), U(), U()
        }}},
        { 26, "27: OVERDRIVE->FLANGER", {{
            P("Drive", 0, 127), P("Over Drive Pan", 0, 127), P("Flg PreDelay", 0, 125), P("Flg Rate", 0, 125),
            P("Flg Depth", 0, 127), P("Flg Feedback", 0, 98), P("Flg Balance", 0, 100), P("Output Level", 0, 127),
            U(), U(), U(), U()
        }}},
        { 27, "28: OVERDRIVE->DELAY", {{
            P("Drive", 0, 127), P("Over Drive Pan", 0, 127), P("Delay Time", 0, 126), P("Delay Feedback", 0, 98),
            P("Delay HF Damp", 0, 17), P("Delay Balance", 0, 100), P("Output Level", 0, 127), U(), U(), U(), U(), U()
        }}},
        { 28, "29: DISTORTION->CHORUS", {{
            P("Dist Drive", 0, 127), P("Dist Pan", 0, 127), P("Cho PreDelay", 0, 125), P("Cho Rate", 0, 125),
            P("Cho Depth", 0, 127), U(), P("Cho Balance", 0, 100), P("Output Level", 0, 127), U(), U(), U(), U()
        }}},
        { 29, "30: DISTORTION->FLANGER", {{
            P("Dist Drive", 0, 127), P("Dist Pan", 0, 127), P("Flg PreDelay", 0, 125), P("Flg Rate", 0, 125),
            P("Flg Depth", 0, 127), P("Flg Feedback", 0, 98), P("Flg Balance", 0, 100), P("Output Level", 0, 127),
            U(), U(), U(), U()
        }}},
        { 30, "31: DISTORTION->DELAY", {{
            P("Dist Drive", 0, 127), P("Dist Pan", 0, 127), P("Delay Time", 0, 126), P("Delay Feedback", 0, 98),
            P("Delay HF Damp", 0, 17), P("Delay Balance", 0, 100), P("Output Level", 0, 127), U(), U(), U(), U(), U()
        }}},
        { 31, "32: ENHANCER->CHORUS", {{
            P("Enhancer Sens", 0, 127), P("Enh Mix Level", 0, 127), P("Cho PreDelay", 0, 125), P("Cho Rate", 0, 125),
            P("Cho Depth", 0, 127), U(), P("Cho Balance", 0, 100), P("Output Level", 0, 127), U(), U(), U(), U()
        }}},
        { 32, "33: ENHANCER->FLANGER", {{
            P("Enhancer Sens", 0, 127), P("Enh Mix Level", 0, 127), P("Flg PreDelay", 0, 125), P("Flg Rate", 0, 125),
            P("Flg Depth", 0, 127), P("Flg Feedback", 0, 98), P("Flg Balance", 0, 100), P("Output Level", 0, 127),
            U(), U(), U(), U()
        }}},
        { 33, "34: ENHANCER->DELAY", {{
            P("Enhancer Sens", 0, 127), P("Enh Mix Level", 0, 127), P("Delay Time", 0, 126), P("Delay Feedback", 0, 98),
            P("Delay HF Damp", 0, 17), U(), P("Delay Balance", 0, 100), P("Output Level", 0, 127), U(), U(), U(), U()
        }}},
        { 34, "35: CHORUS->DELAY", {{
            P("Cho PreDelay", 0, 125), P("Cho Rate", 0, 125), P("Cho Depth", 0, 127), U(),
            P("Cho Balance", 0, 100), P("Delay Time", 0, 126), P("Delay Feedback", 0, 98), P("Delay HF Damp", 0, 17),
            P("Delay Balance", 0, 100), P("Output Level", 0, 127), U(), U()
        }}},
        { 35, "36: FLANGER->DELAY", {{
            P("Flg PreDelay", 0, 125), P("Flg Rate", 0, 125), P("Flg Depth", 0, 127), P("Flg Feedback", 0, 98),
            P("Flg Balance", 0, 100), P("Delay Time", 0, 126), P("Delay Feedback", 0, 98), P("Delay HF Damp", 0, 17),
            P("Delay Balance", 0, 100), P("Output Level", 0, 127), U(), U()
        }}},
        { 36, "37: CHORUS->FLANGER", {{
            P("Cho PreDelay", 0, 125), P("Cho Rate", 0, 125), P("Cho Depth", 0, 127), P("Cho Balance", 0, 100),
            P("Flg PreDelay", 0, 125), P("Flg Rate", 0, 125), P("Flg Depth", 0, 127), P("Flg Feedback", 0, 98),
            P("Flg Balance", 0, 100), P("Output Level", 0, 127), U(), U()
        }}},
        { 37, "38: CHORUS/DELAY (parallel)", {{
            P("Cho PreDelay", 0, 125), P("Cho Rate", 0, 125), P("Cho Depth", 0, 127), U(),
            P("Cho Balance", 0, 100), P("Delay Time", 0, 126), P("Delay Feedback", 0, 98), P("Delay HF Damp", 0, 17),
            P("Delay Balance", 0, 100), P("Output Level", 0, 127), U(), U()
        }}},
        { 38, "39: FLANGER/DELAY (parallel)", {{
            P("Flg PreDelay", 0, 125), P("Flg Rate", 0, 125), P("Flg Depth", 0, 127), P("Flg Feedback", 0, 98),
            P("Flg Balance", 0, 100), P("Delay Time", 0, 126), P("Delay Feedback", 0, 98), P("Delay HF Damp", 0, 17),
            P("Delay Balance", 0, 100), P("Output Level", 0, 127), U(), U()
        }}},
        { 39, "40: CHORUS/FLANGER (parallel)", {{
            P("Cho PreDelay", 0, 125), P("Cho Rate", 0, 125), P("Cho Depth", 0, 127), P("Cho Balance", 0, 100),
            P("Flg PreDelay", 0, 125), P("Flg Rate", 0, 125), P("Flg Depth", 0, 127), P("Flg Feedback", 0, 98),
            P("Flg Balance", 0, 100), P("Output Level", 0, 127), U(), U()
        }}}
    }};
    return kAlgorithms;
}

const EfxAlgorithmDef& getEfxAlgorithm(int typeIndex) {
    const auto& all = getEfxAlgorithms();
    int clamped = std::clamp(typeIndex, 0, 39);
    return all[static_cast<size_t>(clamped)];
}

} // namespace jv1010
