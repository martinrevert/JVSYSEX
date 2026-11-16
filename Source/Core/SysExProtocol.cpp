#include "SysExProtocol.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace jv1010 {
namespace sysex {

uint8_t calculateChecksum(const uint8_t* data, size_t length) {
    uint32_t sum = 0;
    for (size_t i = 0; i < length; ++i) {
        sum += (data[i] & 0x7F);
    }
    uint8_t remainder = static_cast<uint8_t>(sum % 128);
    return static_cast<uint8_t>((128 - remainder) & 0x7F);
}

uint32_t linearTo7Bit(uint32_t linearValue) {
    uint32_t b3 = linearValue & 0x7F;
    uint32_t b2 = (linearValue >> 7) & 0x7F;
    uint32_t b1 = (linearValue >> 14) & 0x7F;
    uint32_t b0 = (linearValue >> 21) & 0x7F;
    return (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
}

uint32_t sevenBitToLinear(uint32_t packed7Bit) {
    uint32_t b0 = (packed7Bit >> 24) & 0x7F;
    uint32_t b1 = (packed7Bit >> 16) & 0x7F;
    uint32_t b2 = (packed7Bit >> 8) & 0x7F;
    uint32_t b3 = packed7Bit & 0x7F;
    return (b0 << 21) | (b1 << 14) | (b2 << 7) | b3;
}

uint32_t addAddressOffset(uint32_t baseAddress, uint32_t linearOffset) {
    uint32_t baseLinear = sevenBitToLinear(baseAddress);
    return linearTo7Bit(baseLinear + linearOffset);
}

std::pair<uint8_t, uint8_t> encodeNibbled2(int value) {
    int clamped = std::clamp(value, 0, 255);
    uint8_t msb = static_cast<uint8_t>((clamped >> 4) & 0x0F);
    uint8_t lsb = static_cast<uint8_t>(clamped & 0x0F);
    return { msb, lsb };
}

int decodeNibbled2(uint8_t msbNibble, uint8_t lsbNibble) {
    return ((msbNibble & 0x0F) * 16) + (lsbNibble & 0x0F);
}

std::vector<uint8_t> encodeNibbled4(int value) {
    uint32_t v = static_cast<uint32_t>(std::max(0, value));
    return {
        static_cast<uint8_t>((v >> 12) & 0x0F),
        static_cast<uint8_t>((v >> 8) & 0x0F),
        static_cast<uint8_t>((v >> 4) & 0x0F),
        static_cast<uint8_t>(v & 0x0F)
    };
}

int decodeNibbled4(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
    return (((b0 & 0x0F) * 16 + (b1 & 0x0F)) * 16 + (b2 & 0x0F)) * 16 + (b3 & 0x0F);
}

std::vector<uint8_t> encodeAsciiName12(const std::string& name) {
    std::vector<uint8_t> result(12, 0x20); // Fill with ASCII space
    size_t count = std::min<size_t>(12, name.size());
    for (size_t i = 0; i < count; ++i) {
        unsigned char ch = static_cast<unsigned char>(name[i]);
        if (ch < 32 || ch > 127) {
            ch = 0x20;
        }
        result[i] = static_cast<uint8_t>(ch);
    }
    return result;
}

std::string decodeAsciiName12(const uint8_t* data, size_t length) {
    if (!data || length == 0) return "";
    size_t count = std::min<size_t>(12, length);
    std::string out;
    out.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        uint8_t ch = data[i] & 0x7F;
        if (ch < 32 || ch > 126) {
            out.push_back(' ');
        } else {
            out.push_back(static_cast<char>(ch));
        }
    }
    // Trim trailing spaces
    while (!out.empty() && out.back() == ' ') {
        out.pop_back();
    }
    return out;
}

std::vector<uint8_t> buildDT1(uint8_t deviceId, uint32_t address, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> msg;
    msg.reserve(11 + data.size());

    msg.push_back(kSysExStart);
    msg.push_back(kRolandId);
    msg.push_back(deviceId & 0x7F);
    msg.push_back(kModelIdJV1010);
    msg.push_back(kCmdDT1);

    uint8_t addrBytes[4] = {
        static_cast<uint8_t>((address >> 24) & 0x7F),
        static_cast<uint8_t>((address >> 16) & 0x7F),
        static_cast<uint8_t>((address >> 8) & 0x7F),
        static_cast<uint8_t>(address & 0x7F)
    };
    msg.insert(msg.end(), addrBytes, addrBytes + 4);

    for (uint8_t b : data) {
        msg.push_back(b & 0x7F);
    }

    // Checksum over address + data (starting at index 5)
    uint8_t checksum = calculateChecksum(msg.data() + 5, 4 + data.size());
    msg.push_back(checksum);
    msg.push_back(kSysExEnd);

    return msg;
}

std::vector<uint8_t> buildDT1SingleByte(uint8_t deviceId, uint32_t address, uint8_t value) {
    return buildDT1(deviceId, address, { static_cast<uint8_t>(value & 0x7F) });
}

std::vector<std::vector<uint8_t>> buildSplitDT1(uint8_t deviceId, uint32_t startAddress, const std::vector<uint8_t>& data) {
    std::vector<std::vector<uint8_t>> packets;
    if (data.empty()) return packets;

    size_t offset = 0;
    while (offset < data.size()) {
        size_t chunkSize = std::min(kMaxDT1PayloadBytes, data.size() - offset);
        uint32_t packetAddr = addAddressOffset(startAddress, static_cast<uint32_t>(offset));
        std::vector<uint8_t> chunk(data.begin() + static_cast<std::ptrdiff_t>(offset),
                                   data.begin() + static_cast<std::ptrdiff_t>(offset + chunkSize));
        packets.push_back(buildDT1(deviceId, packetAddr, chunk));
        offset += chunkSize;
    }
    return packets;
}

std::vector<uint8_t> buildRQ1(uint8_t deviceId, uint32_t address, uint32_t size7Bit) {
    std::vector<uint8_t> msg;
    msg.reserve(15);

    msg.push_back(kSysExStart);
    msg.push_back(kRolandId);
    msg.push_back(deviceId & 0x7F);
    msg.push_back(kModelIdJV1010);
    msg.push_back(kCmdRQ1);

    msg.push_back(static_cast<uint8_t>((address >> 24) & 0x7F));
    msg.push_back(static_cast<uint8_t>((address >> 16) & 0x7F));
    msg.push_back(static_cast<uint8_t>((address >> 8) & 0x7F));
    msg.push_back(static_cast<uint8_t>(address & 0x7F));

    msg.push_back(static_cast<uint8_t>((size7Bit >> 24) & 0x7F));
    msg.push_back(static_cast<uint8_t>((size7Bit >> 16) & 0x7F));
    msg.push_back(static_cast<uint8_t>((size7Bit >> 8) & 0x7F));
    msg.push_back(static_cast<uint8_t>(size7Bit & 0x7F));

    uint8_t checksum = calculateChecksum(msg.data() + 5, 8);
    msg.push_back(checksum);
    msg.push_back(kSysExEnd);

    return msg;
}

std::vector<uint8_t> buildGS_DT1(uint8_t deviceId, uint32_t address24, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> msg;
    msg.reserve(10 + data.size());

    msg.push_back(kSysExStart);
    msg.push_back(kRolandId);
    msg.push_back(deviceId & 0x7F);
    msg.push_back(kModelIdGS);
    msg.push_back(kCmdDT1);

    msg.push_back(static_cast<uint8_t>((address24 >> 16) & 0x7F));
    msg.push_back(static_cast<uint8_t>((address24 >> 8) & 0x7F));
    msg.push_back(static_cast<uint8_t>(address24 & 0x7F));

    for (uint8_t b : data) {
        msg.push_back(b & 0x7F);
    }

    uint8_t checksum = calculateChecksum(msg.data() + 5, 3 + data.size());
    msg.push_back(checksum);
    msg.push_back(kSysExEnd);

    return msg;
}

std::vector<uint8_t> buildIdentityRequest(uint8_t deviceId) {
    return { kSysExStart, kUniversalNonRealtimeId, static_cast<uint8_t>(deviceId & 0x7F), 0x06, 0x01, kSysExEnd };
}

std::vector<uint8_t> buildGMSystemOn() {
    return { kSysExStart, kUniversalNonRealtimeId, kBroadcastDeviceId, 0x09, 0x01, kSysExEnd };
}

std::vector<uint8_t> buildGMSystemOff() {
    return { kSysExStart, kUniversalNonRealtimeId, kBroadcastDeviceId, 0x09, 0x02, kSysExEnd };
}

ParsedSysEx parseSysEx(const uint8_t* data, size_t length) {
    ParsedSysEx result;
    if (!data || length < 4) return result;

    // Strip optional F0 and F7 framing
    size_t start = 0;
    size_t end = length;
    if (data[0] == kSysExStart) start = 1;
    if (end > start && data[end - 1] == kSysExEnd) end -= 1;

    const uint8_t* body = data + start;
    size_t bodyLen = end - start;
    if (bodyLen < 4) return result;

    // Universal Non-Realtime (7E)
    if (body[0] == kUniversalNonRealtimeId) {
        result.deviceId = body[1];
        if (bodyLen >= 4 && body[2] == 0x09 && body[3] == 0x01) {
            result.type = MessageType::GMSystemOn;
            result.checksumValid = true;
            return result;
        }
        if (bodyLen >= 4 && body[2] == 0x09 && body[3] == 0x02) {
            result.type = MessageType::GMSystemOff;
            result.checksumValid = true;
            return result;
        }
        if (bodyLen >= 13 && body[2] == 0x06 && body[3] == 0x02 && body[4] == kRolandId) {
            result.type = MessageType::IdentityReply;
            result.familyCode = static_cast<uint16_t>((body[5] << 8) | body[6]);
            result.familyNumberCode = static_cast<uint16_t>((body[7] << 8) | body[8]);
            result.softwareRevision = (static_cast<uint32_t>(body[9]) << 24) |
                                      (static_cast<uint32_t>(body[10]) << 16) |
                                      (static_cast<uint32_t>(body[11]) << 8) |
                                      static_cast<uint32_t>(body[12]);
            result.checksumValid = true;
            return result;
        }
        return result;
    }

    // Roland Messages (41H)
    if (body[0] == kRolandId && bodyLen >= 8) {
        result.deviceId = body[1];
        result.modelId = body[2];
        uint8_t cmd = body[3];

        if (result.modelId == kModelIdJV1010) {
            if (cmd == kCmdDT1 && bodyLen >= 10) {
                result.type = MessageType::JV_DT1;
                result.address = (static_cast<uint32_t>(body[4]) << 24) |
                                 (static_cast<uint32_t>(body[5]) << 16) |
                                 (static_cast<uint32_t>(body[6]) << 8) |
                                 static_cast<uint32_t>(body[7]);

                size_t payloadLen = bodyLen - 9; // minus 4 header + 4 addr + 1 checksum
                result.payload.assign(body + 8, body + 8 + payloadLen);

                uint8_t expectedSum = calculateChecksum(body + 4, 4 + payloadLen);
                result.checksumValid = (body[bodyLen - 1] == expectedSum);
                return result;
            }
            if (cmd == kCmdRQ1 && bodyLen == 13) {
                result.type = MessageType::JV_RQ1;
                result.address = (static_cast<uint32_t>(body[4]) << 24) |
                                 (static_cast<uint32_t>(body[5]) << 16) |
                                 (static_cast<uint32_t>(body[6]) << 8) |
                                 static_cast<uint32_t>(body[7]);
                result.size = (static_cast<uint32_t>(body[8]) << 24) |
                              (static_cast<uint32_t>(body[9]) << 16) |
                              (static_cast<uint32_t>(body[10]) << 8) |
                              static_cast<uint32_t>(body[11]);

                uint8_t expectedSum = calculateChecksum(body + 4, 8);
                result.checksumValid = (body[12] == expectedSum);
                return result;
            }
        } else if (result.modelId == kModelIdGS) {
            if (cmd == kCmdDT1 && bodyLen >= 9) {
                result.type = MessageType::GS_DT1;
                result.address = (static_cast<uint32_t>(body[4]) << 16) |
                                 (static_cast<uint32_t>(body[5]) << 8) |
                                 static_cast<uint32_t>(body[6]);

                size_t payloadLen = bodyLen - 8; // minus 4 header + 3 addr + 1 checksum
                result.payload.assign(body + 7, body + 7 + payloadLen);

                uint8_t expectedSum = calculateChecksum(body + 4, 3 + payloadLen);
                result.checksumValid = (body[bodyLen - 1] == expectedSum);
                return result;
            }
        }
    }

    return result;
}

std::vector<std::vector<uint8_t>> extractSysExMessages(const uint8_t* rawBytes, size_t length) {
    std::vector<std::vector<uint8_t>> messages;
    if (!rawBytes || length == 0) return messages;

    size_t i = 0;
    while (i < length) {
        if (rawBytes[i] == kSysExStart) {
            size_t start = i;
            ++i;
            while (i < length && rawBytes[i] != kSysExEnd) {
                // If another F0 starts unexpectedly before F7, restart from there
                if (rawBytes[i] == kSysExStart) {
                    start = i;
                }
                ++i;
            }
            if (i < length && rawBytes[i] == kSysExEnd) {
                messages.emplace_back(rawBytes + start, rawBytes + i + 1);
                ++i;
            }
        } else {
            ++i;
        }
    }
    return messages;
}

std::string toHexString(const std::vector<uint8_t>& bytes) {
    std::ostringstream oss;
    oss << std::uppercase << std::hex << std::setfill('0');
    for (size_t i = 0; i < bytes.size(); ++i) {
        if (i > 0) oss << ' ';
        oss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return oss.str();
}

} // namespace sysex
} // namespace jv1010
