#include "EfxAlgorithms.h"
#include <algorithm>

namespace jv1010 {

namespace {
const char* const kEqLowFreq[]    = { "200Hz", "400Hz" };
const char* const kEqHighFreq[]   = { "4kHz", "8kHz" };
const char* const kEqQ[]          = { "0.5", "1.0", "2.0", "4.0", "8.0" };
const char* const kEq17Freq[]     = {
    "200Hz", "250Hz", "315Hz", "400Hz", "500Hz", "630Hz", "800Hz", "1.0k",
    "1.25k", "1.6k", "2.0k", "2.5k", "3.15k", "4.0k", "5.0k", "6.3k", "8.0k"
};
const char* const kHFDamp18[]     = {
    "200Hz", "250Hz", "315Hz", "400Hz", "500Hz", "630Hz", "800Hz", "1.0k",
    "1.25k", "1.6k", "2.0k", "2.5k", "3.15k", "4.0k", "5.0k", "6.3k", "8.0k", "BYPASS"
};
const char* const kAmpSim[]       = { "SMALL", "BLT-IN", "2-STK", "3-STK" };
const char* const kWahFilter[]    = { "LPF", "BPF" };
const char* const kRotarySpeed[]  = { "SLOW", "FAST" };
const char* const kPostGain[]     = { "0dB", "+6dB", "+12dB", "+18dB" };
const char* const kCompRatio[]    = { "1.5:1", "2:1", "4:1", "100:1" };
const char* const kChoFilter[]    = { "OFF", "LPF", "HPF" };
const char* const kDlyMode[]      = { "NORMAL", "CROSS" };
const char* const kPhase[]        = { "NORM", "INV" };
const char* const kPitchMode[]    = { "1", "2", "3", "4", "5" };
const char* const kRevType6[]     = { "ROOM1", "ROOM2", "STAGE1", "STAGE2", "HALL1", "HALL2" };
const char* const kGateRevType4[] = { "NORM", "REV", "SWP1", "SWP2" };

constexpr EfxParamDef K(const char* n, int mn, int mx, int off = 0, const char* fmt = "%d") {
    return { n, mn, mx, true, EfxControlStyle::Knob, off, fmt, nullptr, 0 };
}
constexpr EfxParamDef VS(const char* n, int mn, int mx, int off = 0, const char* fmt = "%d") {
    return { n, mn, mx, true, EfxControlStyle::VSlider, off, fmt, nullptr, 0 };
}
constexpr EfxParamDef HS(const char* n, int mn, int mx, int off = 0, const char* fmt = "%d") {
    return { n, mn, mx, true, EfxControlStyle::HSlider, off, fmt, nullptr, 0 };
}
constexpr EfxParamDef EQG(const char* n) {
    return { n, 0, 30, true, EfxControlStyle::VSlider, -15, "%+ddB", nullptr, 0 };
}
constexpr EfxParamDef PAN(const char* n) {
    return { n, 0, 127, true, EfxControlStyle::HSlider, -64, "%+d", nullptr, 0 };
}
constexpr EfxParamDef BAL(const char* n) {
    return { n, 0, 100, true, EfxControlStyle::HSlider, 0, "%d%%", nullptr, 0 };
}
constexpr EfxParamDef TOG(const char* n, const char* const* lbls) {
    return { n, 0, 1, true, EfxControlStyle::Toggle, 0, "%d", lbls, 2 };
}
template <int N>
constexpr EfxParamDef SEG(const char* n, const char* const (&lbls)[N]) {
    return { n, 0, N - 1, true, EfxControlStyle::Segmented, 0, "%d", lbls, N };
}
template <int N>
constexpr EfxParamDef CMB(const char* n, const char* const (&lbls)[N]) {
    return { n, 0, N - 1, true, EfxControlStyle::Choice, 0, "%d", lbls, N };
}
constexpr EfxParamDef U() {
    return { "(not used)", 0, 127, false, EfxControlStyle::Knob, 0, "%d", nullptr, 0 };
}
} // namespace

const std::array<EfxAlgorithmDef, 40>& getEfxAlgorithms() {
    static const std::array<EfxAlgorithmDef, 40> kAlgorithms = {{
        { 0, "01: STEREO-EQ", {{
            SEG("Low Freq", kEqLowFreq), EQG("Low Gain"), SEG("High Freq", kEqHighFreq), EQG("Hi Gain"),
            CMB("Peak1 Frq", kEq17Freq), SEG("Peak1 Q", kEqQ), EQG("Peak1 Gn"),
            CMB("Peak2 Frq", kEq17Freq), SEG("Peak2 Q", kEqQ), EQG("Peak2 Gn"),
            VS("Level", 0, 127), U()
        }}},
        { 1, "02: OVERDRIVE", {{
            K("Drive", 0, 127), PAN("Out Pan"), SEG("Amp Sim", kAmpSim), EQG("Low Gain"),
            EQG("High Gain"), VS("Out Lvl", 0, 127), U(), U(), U(), U(), U(), U()
        }}},
        { 2, "03: DISTORTION", {{
            K("Drive", 0, 127), PAN("Out Pan"), SEG("Amp Sim", kAmpSim), EQG("Low Gain"),
            EQG("High Gain"), VS("Out Lvl", 0, 127), U(), U(), U(), U(), U(), U()
        }}},
        { 3, "04: PHASER", {{
            K("Manual", 0, 125), K("Rate", 0, 125), K("Depth", 0, 127), K("Reso", 0, 127),
            VS("Mix Lvl", 0, 127), PAN("Out Pan"), VS("Out Lvl", 0, 127), U(), U(), U(), U(), U()
        }}},
        { 4, "05: SPECTRUM", {{
            EQG("250Hz"), EQG("500Hz"), EQG("1kHz"), EQG("1.25k"),
            EQG("2kHz"), EQG("3.15k"), EQG("4kHz"), EQG("8kHz"),
            SEG("Band Q", kEqQ), PAN("Out Pan"), VS("Out Lvl", 0, 127), U()
        }}},
        { 5, "06: ENHANCER", {{
            K("Sens", 0, 127), VS("Mix Lvl", 0, 127), EQG("Low Gain"), EQG("High Gain"),
            VS("Out Lvl", 0, 127), U(), U(), U(), U(), U(), U(), U()
        }}},
        { 6, "07: AUTO-WAH", {{
            SEG("Filter", kWahFilter), K("Rate", 0, 125), K("Depth", 0, 127), K("Sens", 0, 127),
            K("Manual", 0, 127), K("Peak", 0, 127), VS("Out Lvl", 0, 127), U(), U(), U(), U(), U()
        }}},
        { 7, "08: ROTARY", {{
            K("HF Slow", 0, 125), K("LF Slow", 0, 125), K("HF Fast", 0, 125), K("LF Fast", 0, 125),
            TOG("Speed", kRotarySpeed), K("HF Accel", 0, 15), K("LF Accel", 0, 15), VS("HF Lvl", 0, 127),
            VS("LF Lvl", 0, 127), K("Separate", 0, 127), VS("Out Lvl", 0, 127), U()
        }}},
        { 8, "09: COMPRESSOR", {{
            K("Sustain", 0, 127), K("Attack", 0, 127), PAN("Out Pan"), SEG("Post Gain", kPostGain),
            EQG("Low Gain"), EQG("High Gain"), VS("Out Lvl", 0, 127), U(), U(), U(), U(), U()
        }}},
        { 9, "10: LIMITER", {{
            K("Threshold", 0, 127), K("Release", 0, 127), SEG("Ratio", kCompRatio), PAN("Out Pan"),
            SEG("Post Gain", kPostGain), EQG("Low Gain"), EQG("High Gain"), VS("Out Lvl", 0, 127),
            U(), U(), U(), U()
        }}},
        { 10, "11: HEXA-CHORUS", {{
            K("Pre Dly", 0, 125), K("Rate", 0, 125), K("Depth", 0, 127), K("Dly Dev", 0, 20),
            K("Dep Dev", 0, 40), K("Pan Dev", 0, 20), BAL("Balance"), VS("Out Lvl", 0, 127),
            U(), U(), U(), U()
        }}},
        { 11, "12: TREMOLO-CHORUS", {{
            K("Pre Dly", 0, 125), K("Cho Rate", 0, 125), K("Cho Dep", 0, 127), K("Trm Rate", 0, 125),
            K("Trm Sep", 0, 127), K("Trm Phs", 0, 90), BAL("Balance"), VS("Out Lvl", 0, 127),
            U(), U(), U(), U()
        }}},
        { 12, "13: SPACE-D", {{
            K("Pre Dly", 0, 125), K("Rate", 0, 125), K("Depth", 0, 127), K("Phase", 0, 90),
            EQG("Low Gain"), EQG("High Gain"), BAL("Balance"), VS("Out Lvl", 0, 127),
            U(), U(), U(), U()
        }}},
        { 13, "14: STEREO-CHORUS", {{
            SEG("Filter", kChoFilter), CMB("Cutoff", kEq17Freq), K("Pre Dly", 0, 125), K("Rate", 0, 125),
            K("Depth", 0, 127), K("Phase", 0, 90), U(), EQG("Low Gain"),
            EQG("High Gain"), BAL("Balance"), VS("Out Lvl", 0, 127), U()
        }}},
        { 14, "15: STEREO-FLANGER", {{
            SEG("Filter", kChoFilter), CMB("Cutoff", kEq17Freq), K("Pre Dly", 0, 125), K("Rate", 0, 125),
            K("Depth", 0, 127), K("Phase", 0, 90), K("Feedback", 0, 98), EQG("Low Gain"),
            EQG("High Gain"), BAL("Balance"), VS("Out Lvl", 0, 127), U()
        }}},
        { 15, "16: STEP-FLANGER", {{
            K("Pre Dly", 0, 125), K("Rate", 0, 125), K("Depth", 0, 127), K("Feedback", 0, 98),
            K("StepRate", 0, 125), K("Phase", 0, 90), EQG("Low Gain"), EQG("High Gain"),
            BAL("Balance"), VS("Out Lvl", 0, 127), U(), U()
        }}},
        { 16, "17: STEREO-DELAY", {{
            SEG("Fbk Mode", kDlyMode), HS("Time L", 0, 126), HS("Time R", 0, 126), TOG("Phase L", kPhase),
            TOG("Phase R", kPhase), K("Feedback", 0, 98), CMB("HF Damp", kHFDamp18), EQG("Low Gain"),
            EQG("High Gain"), BAL("Balance"), VS("Out Lvl", 0, 127), U()
        }}},
        { 17, "18: MODULATION-DELAY", {{
            SEG("Fbk Mode", kDlyMode), HS("Time L", 0, 126), HS("Time R", 0, 126), K("Feedback", 0, 98),
            CMB("HF Damp", kHFDamp18), K("Rate", 0, 125), K("Depth", 0, 127), K("Phase", 0, 90),
            EQG("Low Gain"), EQG("High Gain"), BAL("Balance"), VS("Out Lvl", 0, 127)
        }}},
        { 18, "19: TRIPLE-TAP-DELAY", {{
            HS("Time L", 0, 125), HS("Time R", 0, 125), HS("Time C", 0, 125), K("Feedback", 0, 98),
            CMB("HF Damp", kHFDamp18), VS("L Lvl", 0, 127), VS("R Lvl", 0, 127), VS("C Lvl", 0, 127),
            EQG("Low Gain"), EQG("High Gain"), BAL("Balance"), VS("Out Lvl", 0, 127)
        }}},
        { 19, "20: QUADRUPLE-TAP-DELAY", {{
            HS("Time 1", 0, 125), HS("Time 2", 0, 125), HS("Time 3", 0, 125), HS("Time 4", 0, 125),
            VS("Lvl 1", 0, 127), VS("Lvl 2", 0, 127), VS("Lvl 3", 0, 127), VS("Lvl 4", 0, 127),
            K("Feedback", 0, 98), CMB("HF Damp", kHFDamp18), BAL("Balance"), VS("Out Lvl", 0, 127)
        }}},
        { 20, "21: TIME-CONTROL-DELAY", {{
            HS("Dly Time", 0, 120), K("Feedback", 0, 98), K("Accel", 0, 15), CMB("HF Damp", kHFDamp18),
            PAN("Out Pan"), EQG("Low Gain"), EQG("High Gain"), BAL("Balance"),
            VS("Out Lvl", 0, 127), U(), U(), U()
        }}},
        { 21, "22: 2VOICE-PITCH-SHIFTER", {{
            SEG("Mode", kPitchMode), K("Coarse A", 0, 36, -24, "%+dst"), K("Coarse B", 0, 36, -24, "%+dst"), K("Fine A", 0, 100, -50, "%+d"),
            K("Fine B", 0, 100, -50, "%+d"), K("PreDly A", 0, 126), K("PreDly B", 0, 126), PAN("Pan A"),
            PAN("Pan B"), BAL("Lvl Bal"), BAL("Efx Bal"), VS("Out Lvl", 0, 127)
        }}},
        { 22, "23: FBK-PITCH-SHIFTER", {{
            SEG("Mode", kPitchMode), K("Coarse", 0, 36, -24, "%+dst"), K("Fine", 0, 100, -50, "%+d"), K("Pre Dly", 0, 126),
            K("Feedback", 0, 98), PAN("Out Pan"), EQG("Low Gain"), EQG("High Gain"),
            BAL("Balance"), VS("Out Lvl", 0, 127), U(), U()
        }}},
        { 23, "24: REVERB", {{
            SEG("Rev Type", kRevType6), K("Pre Dly", 0, 125), K("Rev Time", 0, 127), CMB("HF Damp", kHFDamp18),
            EQG("Low Gain"), EQG("High Gain"), BAL("Balance"), VS("Out Lvl", 0, 127),
            U(), U(), U(), U()
        }}},
        { 24, "25: GATE-REVERB", {{
            SEG("Gate Type", kGateRevType4), K("Pre Dly", 0, 125), K("GateTime", 0, 99), EQG("Low Gain"),
            EQG("High Gain"), BAL("Balance"), VS("Out Lvl", 0, 127), U(), U(), U(), U(), U()
        }}},
        { 25, "26: OVERDRIVE->CHORUS", {{
            K("Drive", 0, 127), PAN("OD Pan"), K("Cho Dly", 0, 125), K("Cho Rate", 0, 125),
            K("Cho Dep", 0, 127), U(), BAL("Cho Bal"), VS("Out Lvl", 0, 127), U(), U(), U(), U()
        }}},
        { 26, "27: OVERDRIVE->FLANGER", {{
            K("Drive", 0, 127), PAN("OD Pan"), K("Flg Dly", 0, 125), K("Flg Rate", 0, 125),
            K("Flg Dep", 0, 127), K("Flg Fbk", 0, 98), BAL("Flg Bal"), VS("Out Lvl", 0, 127),
            U(), U(), U(), U()
        }}},
        { 27, "28: OVERDRIVE->DELAY", {{
            K("Drive", 0, 127), PAN("OD Pan"), HS("Dly Time", 0, 126), K("Dly Fbk", 0, 98),
            CMB("HF Damp", kHFDamp18), BAL("Dly Bal"), VS("Out Lvl", 0, 127), U(), U(), U(), U(), U()
        }}},
        { 28, "29: DISTORTION->CHORUS", {{
            K("Dist Drv", 0, 127), PAN("Dst Pan"), K("Cho Dly", 0, 125), K("Cho Rate", 0, 125),
            K("Cho Dep", 0, 127), U(), BAL("Cho Bal"), VS("Out Lvl", 0, 127), U(), U(), U(), U()
        }}},
        { 29, "30: DISTORTION->FLANGER", {{
            K("Dist Drv", 0, 127), PAN("Dst Pan"), K("Flg Dly", 0, 125), K("Flg Rate", 0, 125),
            K("Flg Dep", 0, 127), K("Flg Fbk", 0, 98), BAL("Flg Bal"), VS("Out Lvl", 0, 127),
            U(), U(), U(), U()
        }}},
        { 30, "31: DISTORTION->DELAY", {{
            K("Dist Drv", 0, 127), PAN("Dst Pan"), HS("Dly Time", 0, 126), K("Dly Fbk", 0, 98),
            CMB("HF Damp", kHFDamp18), BAL("Dly Bal"), VS("Out Lvl", 0, 127), U(), U(), U(), U(), U()
        }}},
        { 31, "32: ENHANCER->CHORUS", {{
            K("Enh Sens", 0, 127), VS("Enh Mix", 0, 127), K("Cho Dly", 0, 125), K("Cho Rate", 0, 125),
            K("Cho Dep", 0, 127), U(), BAL("Cho Bal"), VS("Out Lvl", 0, 127), U(), U(), U(), U()
        }}},
        { 32, "33: ENHANCER->FLANGER", {{
            K("Enh Sens", 0, 127), VS("Enh Mix", 0, 127), K("Flg Dly", 0, 125), K("Flg Rate", 0, 125),
            K("Flg Dep", 0, 127), K("Flg Fbk", 0, 98), BAL("Flg Bal"), VS("Out Lvl", 0, 127),
            U(), U(), U(), U()
        }}},
        { 33, "34: ENHANCER->DELAY", {{
            K("Enh Sens", 0, 127), VS("Enh Mix", 0, 127), HS("Dly Time", 0, 126), K("Dly Fbk", 0, 98),
            CMB("HF Damp", kHFDamp18), U(), BAL("Dly Bal"), VS("Out Lvl", 0, 127), U(), U(), U(), U()
        }}},
        { 34, "35: CHORUS->DELAY", {{
            K("Cho Dly", 0, 125), K("Cho Rate", 0, 125), K("Cho Dep", 0, 127), U(),
            BAL("Cho Bal"), HS("Dly Time", 0, 126), K("Dly Fbk", 0, 98), CMB("HF Damp", kHFDamp18),
            BAL("Dly Bal"), VS("Out Lvl", 0, 127), U(), U()
        }}},
        { 35, "36: FLANGER->DELAY", {{
            K("Flg Dly", 0, 125), K("Flg Rate", 0, 125), K("Flg Dep", 0, 127), K("Flg Fbk", 0, 98),
            BAL("Flg Bal"), HS("Dly Time", 0, 126), K("Dly Fbk", 0, 98), CMB("HF Damp", kHFDamp18),
            BAL("Dly Bal"), VS("Out Lvl", 0, 127), U(), U()
        }}},
        { 36, "37: CHORUS->FLANGER", {{
            K("Cho Dly", 0, 125), K("Cho Rate", 0, 125), K("Cho Dep", 0, 127), BAL("Cho Bal"),
            K("Flg Dly", 0, 125), K("Flg Rate", 0, 125), K("Flg Dep", 0, 127), K("Flg Fbk", 0, 98),
            BAL("Flg Bal"), VS("Out Lvl", 0, 127), U(), U()
        }}},
        { 37, "38: CHORUS/DELAY (parallel)", {{
            K("Cho Dly", 0, 125), K("Cho Rate", 0, 125), K("Cho Dep", 0, 127), U(),
            BAL("Cho Bal"), HS("Dly Time", 0, 126), K("Dly Fbk", 0, 98), CMB("HF Damp", kHFDamp18),
            BAL("Dly Bal"), VS("Out Lvl", 0, 127), U(), U()
        }}},
        { 38, "39: FLANGER/DELAY (parallel)", {{
            K("Flg Dly", 0, 125), K("Flg Rate", 0, 125), K("Flg Dep", 0, 127), K("Flg Fbk", 0, 98),
            BAL("Flg Bal"), HS("Dly Time", 0, 126), K("Dly Fbk", 0, 98), CMB("HF Damp", kHFDamp18),
            BAL("Dly Bal"), VS("Out Lvl", 0, 127), U(), U()
        }}},
        { 39, "40: CHORUS/FLANGER (parallel)", {{
            K("Cho Dly", 0, 125), K("Cho Rate", 0, 125), K("Cho Dep", 0, 127), BAL("Cho Bal"),
            K("Flg Dly", 0, 125), K("Flg Rate", 0, 125), K("Flg Dep", 0, 127), K("Flg Fbk", 0, 98),
            BAL("Flg Bal"), VS("Out Lvl", 0, 127), U(), U()
        }}}
    }};
    return kAlgorithms;
}

const EfxAlgorithmDef& getEfxAlgorithm(int typeIndex) {
    const auto& all = getEfxAlgorithms();
    int clamped = std::clamp(typeIndex, 0, static_cast<int>(all.size()) - 1);
    return all[static_cast<size_t>(clamped)];
}

} // namespace jv1010
