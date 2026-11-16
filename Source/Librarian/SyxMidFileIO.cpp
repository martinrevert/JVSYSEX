#include "SyxMidFileIO.h"

namespace jv1010 {

namespace {
uint32_t remapJVFamilyAddress(uint32_t addr, ImportTargetMode mode, int targetUserSlot, bool& didRemap) {
    uint8_t b0 = static_cast<uint8_t>((addr >> 24) & 0x7F);
    uint8_t b1 = static_cast<uint8_t>((addr >> 16) & 0x7F);
    uint32_t low16 = addr & 0x0000FFFFu;

    // Detect Card / JV-1080/2080/XP addresses (e.g. 20..2F for Card Performance/Patch/Rhythm)
    if (b0 >= 0x20 && b0 <= 0x2F) {
        didRemap = true;
        if (b0 == 0x20) {
            // Card Performance -> User Performance
            b0 = 0x10;
            b1 = static_cast<uint8_t>(b1 & 0x1F);
        } else if (b0 == 0x21) {
            // Card Patch -> User Patch
            b0 = 0x11;
            b1 = static_cast<uint8_t>(b1 & 0x7F);
        } else {
            b0 = 0x11;
            b1 = static_cast<uint8_t>(b1 & 0x7F);
        }
        addr = (static_cast<uint32_t>(b0) << 24) | (static_cast<uint32_t>(b1) << 16) | low16;
    }

    if (mode == ImportTargetMode::LoadIntoTempBuffer) {
        if (b0 == 0x11 || b0 == 0x03 || (b0 == 0x02 && b1 != 0x09)) {
            return addr::kPatchModeTempPatchBase | low16;
        }
        if (b0 == 0x10 && b1 < 0x20) {
            return addr::kTempPerformanceBase | low16;
        }
        if (b0 == 0x10 && (b1 == 0x40 || b1 == 0x41)) {
            return addr::kTempRhythmBase | low16;
        }
    } else if (mode == ImportTargetMode::LoadIntoUserBank) {
        if (b0 == 0x03 || b0 == 0x02) {
            return addr::getUserPatchBase(targetUserSlot) | low16;
        }
        if (b0 == 0x01) {
            return addr::getUserPerfBase(targetUserSlot) | low16;
        }
        if (b0 == 0x11) {
            int slot = (static_cast<int>(b1) + targetUserSlot) & 0x7F;
            return addr::getUserPatchBase(slot) | low16;
        }
    }
    return addr;
}
} // namespace

ImportResult SyxMidFileIO::importFile(const juce::File& file,
                                      JV1010State& state,
                                      ImportTargetMode targetMode,
                                      int targetUserSlot,
                                      uint8_t deviceId) {
    ImportResult result;
    if (!file.existsAsFile()) {
        result.summary = "El archivo no existe.";
        return result;
    }

    std::vector<std::vector<uint8_t>> rawSysExMessages;
    auto ext = file.getFileExtension().toLowerCase();

    if (ext == ".mid" || ext == ".smf") {
        juce::FileInputStream stream(file);
        if (!stream.openedOk()) {
            result.summary = "No se pudo abrir el archivo .mid";
            return result;
        }
        juce::MidiFile midiFile;
        if (!midiFile.readFrom(stream)) {
            result.summary = "Formato SMF (.mid) invalido.";
            return result;
        }
        for (int t = 0; t < midiFile.getNumTracks(); ++t) {
            if (const auto* track = midiFile.getTrack(t)) {
                for (int i = 0; i < track->getNumEvents(); ++i) {
                    const auto& msg = track->getEventPointer(i)->message;
                    if (msg.isSysEx()) {
                        std::vector<uint8_t> framed;
                        framed.reserve(static_cast<size_t>(msg.getSysExDataSize()) + 2);
                        framed.push_back(sysex::kSysExStart);
                        framed.insert(framed.end(), msg.getSysExData(), msg.getSysExData() + msg.getSysExDataSize());
                        framed.push_back(sysex::kSysExEnd);
                        rawSysExMessages.push_back(std::move(framed));
                    }
                }
            }
        }
    } else {
        juce::MemoryBlock mb;
        if (!file.loadFileAsData(mb)) {
            result.summary = "No se pudo leer el archivo .syx";
            return result;
        }
        rawSysExMessages = sysex::extractSysExMessages(static_cast<const uint8_t*>(mb.getData()), mb.getSize());
    }

    result.totalMessages = static_cast<int>(rawSysExMessages.size());
    if (result.totalMessages == 0) {
        result.summary = "No se encontraron mensajes SysEx en el archivo.";
        return result;
    }

    for (const auto& msgBytes : rawSysExMessages) {
        auto parsed = sysex::parseSysEx(msgBytes.data(), msgBytes.size());
        if (parsed.type == sysex::MessageType::JV_DT1 && parsed.checksumValid) {
            bool remapped = false;
            uint32_t targetAddr = remapJVFamilyAddress(parsed.address, targetMode, targetUserSlot, remapped);
            if (remapped) result.remappedFromJVFamily = true;

            if (state.applyDT1(targetAddr, parsed.payload)) {
                ++result.appliedMessages;
                result.hardwarePackets.push_back(sysex::buildDT1(deviceId, targetAddr, parsed.payload));
                uint8_t b0 = static_cast<uint8_t>((targetAddr >> 24) & 0x7F);
                uint8_t b1 = static_cast<uint8_t>((targetAddr >> 16) & 0x7F);
                uint32_t low16 = targetAddr & 0xFFFFu;
                if (low16 == 0) {
                    if (b0 == 0x11 || b0 == 0x03 || (b0 == 0x02 && b1 != 0x09)) ++result.patchesLoaded;
                    else if (b0 == 0x01 || (b0 == 0x10 && b1 < 0x20)) ++result.perfsLoaded;
                    else if ((b0 == 0x02 && b1 == 0x09) || (b0 == 0x10 && b1 >= 0x40)) ++result.rhythmsLoaded;
                }
            }
        }
    }

    result.success = (result.appliedMessages > 0);
    result.summary = "Mensajes SysEx procesados: " + std::to_string(result.appliedMessages) + "/" + std::to_string(result.totalMessages)
                   + " (Patches: " + std::to_string(result.patchesLoaded)
                   + ", Perfs: " + std::to_string(result.perfsLoaded)
                   + ", Rhythm: " + std::to_string(result.rhythmsLoaded) + ")";
    return result;
}

std::vector<std::vector<uint8_t>> SyxMidFileIO::collectPacketsForScope(const JV1010State& state,
                                                                       ExportScope scope,
                                                                       uint8_t deviceId) {
    std::vector<std::vector<uint8_t>> allPackets;
    auto append = [&](const std::vector<std::vector<uint8_t>>& pkts) {
        allPackets.insert(allPackets.end(), pkts.begin(), pkts.end());
    };

    switch (scope) {
        case ExportScope::TempPatch:
            append(state.serializePatchToDT1(deviceId, addr::kPatchModeTempPatchBase, state.getPatchModeTempPatch()));
            break;
        case ExportScope::TempPerformance:
            append(state.serializePerformanceToDT1(deviceId, addr::kTempPerformanceBase, state.getTempPerformance()));
            break;
        case ExportScope::TempRhythm:
            append(state.serializeRhythmToDT1(deviceId, addr::kTempRhythmBase, state.getTempRhythm()));
            break;
        case ExportScope::UserPatches128:
            for (int i = 0; i < static_cast<int>(addr::kNumUserPatches); ++i) {
                append(state.serializePatchToDT1(deviceId, addr::getUserPatchBase(i), state.getUserPatch(i)));
            }
            break;
        case ExportScope::UserPerformances32:
            for (int i = 0; i < static_cast<int>(addr::kNumUserPerformances); ++i) {
                append(state.serializePerformanceToDT1(deviceId, addr::getUserPerfBase(i), state.getUserPerformance(i)));
            }
            break;
        case ExportScope::UserRhythmSets2:
            for (int i = 0; i < static_cast<int>(addr::kNumUserRhythmSets); ++i) {
                append(state.serializeRhythmToDT1(deviceId, addr::getUserRhythmBase(i), state.getUserRhythm(i)));
            }
            break;
        case ExportScope::FullUserMemory:
            append(state.serializeSystemToDT1(deviceId));
            for (int i = 0; i < static_cast<int>(addr::kNumUserPerformances); ++i) {
                append(state.serializePerformanceToDT1(deviceId, addr::getUserPerfBase(i), state.getUserPerformance(i)));
            }
            for (int i = 0; i < static_cast<int>(addr::kNumUserRhythmSets); ++i) {
                append(state.serializeRhythmToDT1(deviceId, addr::getUserRhythmBase(i), state.getUserRhythm(i)));
            }
            for (int i = 0; i < static_cast<int>(addr::kNumUserPatches); ++i) {
                append(state.serializePatchToDT1(deviceId, addr::getUserPatchBase(i), state.getUserPatch(i)));
            }
            break;
    }
    return allPackets;
}

bool SyxMidFileIO::exportToSyx(const juce::File& file,
                               const JV1010State& state,
                               ExportScope scope,
                               uint8_t deviceId) {
    auto packets = collectPacketsForScope(state, scope, deviceId);
    if (packets.empty()) return false;

    juce::FileOutputStream stream(file);
    if (!stream.openedOk()) return false;
    stream.setPosition(0);
    stream.truncate();

    for (const auto& pkt : packets) {
        stream.write(pkt.data(), pkt.size());
    }
    return true;
}

bool SyxMidFileIO::exportToMid(const juce::File& file,
                               const JV1010State& state,
                               ExportScope scope,
                               uint8_t deviceId) {
    auto packets = collectPacketsForScope(state, scope, deviceId);
    if (packets.empty()) return false;

    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(480);

    juce::MidiMessageSequence track;
    // Set 120 BPM (500,000 us per quarter note -> 1 tick = ~1.04 ms)
    track.addEvent(juce::MidiMessage::tempoMetaEvent(500000), 0.0);

    double currentTick = 24.0;
    for (const auto& pkt : packets) {
        if (pkt.size() >= 3) {
            auto msg = juce::MidiMessage::createSysExMessage(pkt.data() + 1, static_cast<int>(pkt.size() - 2));
            track.addEvent(msg, currentTick);
            currentTick += 24.0; // ~25 ms spacing between DT1 packets
        }
    }

    midiFile.addTrack(track);

    juce::FileOutputStream stream(file);
    if (!stream.openedOk()) return false;
    stream.setPosition(0);
    stream.truncate();
    return midiFile.writeTo(stream);
}

} // namespace jv1010
