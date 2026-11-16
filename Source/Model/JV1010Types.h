#pragma once

#include <cstdint>
#include <string>

namespace jv1010 {

enum class SoundMode : uint8_t {
    Performance = 0,
    Patch = 1,
    GM = 2
};

enum class PatchBank : uint8_t {
    User = 0,
    PresetA,
    PresetB,
    PresetC,
    PresetD, // GM
    PresetE,
    Session, // XP-A (SR-JV80-09 onboard)
    WaveExp  // XP-B (User-installed SR-JV80 board)
};

enum class PerfBank : uint8_t {
    User = 0,
    PresetA,
    PresetB
};

enum class RhythmBank : uint8_t {
    User = 0,
    PresetA,
    PresetB,
    PresetC,
    PresetD, // GM
    PresetE,
    Session, // XP-A
    WaveExp  // XP-B
};

enum class WaveGroup : uint8_t {
    IntA = 0,
    IntB,
    Session, // XP-A
    WaveExp  // XP-B
};

struct BankSelectInfo {
    uint8_t msb = 80;
    uint8_t lsb = 0;
    uint8_t program = 0; // 0..127
};

inline BankSelectInfo getPatchBankSelect(PatchBank bank, int patchIndexZeroBased) {
    BankSelectInfo info;
    switch (bank) {
        case PatchBank::User:    info.msb = 80; info.lsb = 0; info.program = static_cast<uint8_t>(patchIndexZeroBased & 0x7F); break;
        case PatchBank::PresetA: info.msb = 81; info.lsb = 0; info.program = static_cast<uint8_t>(patchIndexZeroBased & 0x7F); break;
        case PatchBank::PresetB: info.msb = 81; info.lsb = 1; info.program = static_cast<uint8_t>(patchIndexZeroBased & 0x7F); break;
        case PatchBank::PresetC: info.msb = 81; info.lsb = 2; info.program = static_cast<uint8_t>(patchIndexZeroBased & 0x7F); break;
        case PatchBank::PresetD: info.msb = 81; info.lsb = 3; info.program = static_cast<uint8_t>(patchIndexZeroBased & 0x7F); break;
        case PatchBank::PresetE: info.msb = 81; info.lsb = 4; info.program = static_cast<uint8_t>(patchIndexZeroBased & 0x7F); break;
        case PatchBank::Session:
            info.msb = 84;
            if (patchIndexZeroBased < 128) {
                info.lsb = 0;
                info.program = static_cast<uint8_t>(patchIndexZeroBased);
            } else {
                info.lsb = 1;
                info.program = static_cast<uint8_t>((patchIndexZeroBased - 128) & 0x7F);
            }
            break;
        case PatchBank::WaveExp:
            info.msb = 84;
            if (patchIndexZeroBased < 128) {
                info.lsb = 2;
                info.program = static_cast<uint8_t>(patchIndexZeroBased);
            } else {
                info.lsb = 3;
                info.program = static_cast<uint8_t>((patchIndexZeroBased - 128) & 0x7F);
            }
            break;
    }
    return info;
}

inline BankSelectInfo getPerfBankSelect(PerfBank bank, int perfIndexZeroBased) {
    BankSelectInfo info;
    info.program = static_cast<uint8_t>(perfIndexZeroBased & 0x1F);
    switch (bank) {
        case PerfBank::User:    info.msb = 80; info.lsb = 0; break;
        case PerfBank::PresetA: info.msb = 81; info.lsb = 0; break;
        case PerfBank::PresetB: info.msb = 81; info.lsb = 1; break;
    }
    return info;
}

inline BankSelectInfo getRhythmBankSelect(RhythmBank bank, int kitIndexZeroBased) {
    BankSelectInfo info;
    switch (bank) {
        case RhythmBank::User:    info.msb = 80; info.lsb = 0; info.program = static_cast<uint8_t>(kitIndexZeroBased & 0x01); break;
        case RhythmBank::PresetA: info.msb = 81; info.lsb = 0; info.program = static_cast<uint8_t>(kitIndexZeroBased & 0x01); break;
        case RhythmBank::PresetB: info.msb = 81; info.lsb = 1; info.program = static_cast<uint8_t>(kitIndexZeroBased & 0x01); break;
        case RhythmBank::PresetC: info.msb = 81; info.lsb = 2; info.program = static_cast<uint8_t>(kitIndexZeroBased & 0x01); break;
        case RhythmBank::PresetD: info.msb = 81; info.lsb = 3; info.program = static_cast<uint8_t>(kitIndexZeroBased & 0x01); break;
        case RhythmBank::PresetE: info.msb = 81; info.lsb = 4; info.program = static_cast<uint8_t>(kitIndexZeroBased & 0x01); break;
        case RhythmBank::Session: info.msb = 84; info.lsb = 0; info.program = static_cast<uint8_t>(kitIndexZeroBased & 0x07); break;
        case RhythmBank::WaveExp:
            info.msb = 84;
            if (kitIndexZeroBased < 128) {
                info.lsb = 2;
                info.program = static_cast<uint8_t>(kitIndexZeroBased);
            } else {
                info.lsb = 3;
                info.program = static_cast<uint8_t>((kitIndexZeroBased - 128) & 0x7F);
            }
            break;
    }
    return info;
}

} // namespace jv1010
