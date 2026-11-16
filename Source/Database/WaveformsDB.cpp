#include "WaveformsDB.h"
#include <array>

namespace jv1010 {

namespace {
const std::array<const char*, 64> kIntAWaveHighlights = {{
    "St.Grand A L", "St.Grand A R", "St.Grand B L", "St.Grand B R",
    "St.Grand C L", "St.Grand C R", "Brite Pno A", "Brite Pno B",
    "E.Grand 1", "E.Grand 2", "Honky-Tonk", "Rhodes A",
    "Rhodes B", "Rhodes C", "Wurlitzer", "Clav 1",
    "Clav 2", "Harpsichord", "Celesta", "Glockenspiel",
    "Vibraphone", "Marimba", "Xylophone", "Tubular Bell",
    "Pipe Organ 1", "Pipe Organ 2", "Jazz Organ 1", "Jazz Organ 2",
    "Rock Organ", "Accordion", "Harmonica", "Nylon Gtr 1",
    "Nylon Gtr 2", "Steel Gtr 1", "Steel Gtr 2", "Jazz Gtr",
    "Clean Gtr", "Muted Gtr", "Overdrive Gtr", "Distortion Gtr",
    "Acoustic Bass", "Finger Bass", "Pick Bass", "Fretless Bass",
    "Slap Bass 1", "Slap Bass 2", "Violin", "Viola",
    "Cello", "Contrabass", "Strings L", "Strings R",
    "Pizzicato", "Harp", "Timpani", "Trumpet",
    "Trombone", "French Horn", "Tuba", "Brass Sect",
    "Soprano Sax", "Alto Sax", "Tenor Sax", "Flute"
}};

const std::array<const char*, 40> kIntBWaveHighlights = {{
    "JP-8 Saw", "JP-8 Pulse", "Juno Saw", "Juno Square",
    "MG Saw", "MG Square", "MG Triangle", "Sine Wave",
    "P5 Saw", "P5 Sync", "OB Saw", "D-50 Fantasia",
    "D-50 Choir", "D-50 Bell", "PWM Wave 1", "PWM Wave 2",
    "Super Saw", "Noise White", "Noise Pink", "Sub Sine",
    "Sub Triangle", "Sub Saw", "Digital Wave 1", "Digital Wave 2",
    "Vocal Aah", "Vocal Ooh", "Breath Noise", "Fret Noise",
    "Pick Scrape", "Hammer Hit", "Wind Chimes", "Spectrum 1",
    "Spectrum 2", "Bell Transient", "Clav Attack", "Organ Click",
    "CrossMod 1", "CrossMod 2", "Metallic", "Industrial FX"
}};
} // namespace

int getWaveformCount(WaveGroup group, int installedBoardMaxWaves) {
    switch (group) {
        case WaveGroup::IntA:    return 255;
        case WaveGroup::IntB:    return 193;
        case WaveGroup::Session: return 206;
        case WaveGroup::WaveExp: return installedBoardMaxWaves;
    }
    return 255;
}

std::string getWaveformName(WaveGroup group, int waveNumberOneBased) {
    int idx = waveNumberOneBased - 1;
    if (idx < 0) return "---";

    if (group == WaveGroup::IntA) {
        if (idx < static_cast<int>(kIntAWaveHighlights.size())) {
            return std::string(kIntAWaveHighlights[static_cast<size_t>(idx)]);
        }
        return "INT-A Wave " + std::to_string(waveNumberOneBased);
    }
    if (group == WaveGroup::IntB) {
        if (idx < static_cast<int>(kIntBWaveHighlights.size())) {
            return std::string(kIntBWaveHighlights[static_cast<size_t>(idx)]);
        }
        return "INT-B Wave " + std::to_string(waveNumberOneBased);
    }
    if (group == WaveGroup::Session) {
        return "XP-A Session " + std::to_string(waveNumberOneBased);
    }
    return "XP-B Exp " + std::to_string(waveNumberOneBased);
}

} // namespace jv1010
