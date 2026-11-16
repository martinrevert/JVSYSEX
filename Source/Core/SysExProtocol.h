#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace jv1010 {

namespace sysex {

constexpr uint8_t kSysExStart = 0xF0;
constexpr uint8_t kSysExEnd = 0xF7;
constexpr uint8_t kRolandId = 0x41;
constexpr uint8_t kDefaultDeviceId = 0x10; // Device ID 17 (10H)
constexpr uint8_t kModelIdJV1010 = 0x6A;
constexpr uint8_t kModelIdGS = 0x42;
constexpr uint8_t kCmdRQ1 = 0x11;
constexpr uint8_t kCmdDT1 = 0x12;

constexpr uint8_t kUniversalNonRealtimeId = 0x7E;
constexpr uint8_t kUniversalRealtimeId = 0x7F;
constexpr uint8_t kBroadcastDeviceId = 0x7F;

constexpr size_t kMaxDT1PayloadBytes = 128;

enum class MessageType {
    Unknown,
    JV_DT1,
    JV_RQ1,
    GS_DT1,
    IdentityReply,
    GMSystemOn,
    GMSystemOff
};

struct ParsedSysEx {
    MessageType type = MessageType::Unknown;
    uint8_t deviceId = kDefaultDeviceId;
    uint8_t modelId = 0;
    uint32_t address = 0; // 4-byte address for JV, 3-byte (0x00AABBCC) for GS
    uint32_t size = 0;    // For RQ1
    std::vector<uint8_t> payload;
    bool checksumValid = false;

    // For IdentityReply
    uint16_t familyCode = 0;
    uint16_t familyNumberCode = 0;
    uint32_t softwareRevision = 0;
};

/**
 * Calculates Roland 7-bit checksum: (128 - (sum % 128)) % 128
 */
uint8_t calculateChecksum(const uint8_t* data, size_t length);

/**
 * Converts a linear integer (e.g. 129) to 7-bit packed 4-byte representation (e.g. 0x00000101).
 */
uint32_t linearTo7Bit(uint32_t linearValue);

/**
 * Converts a 7-bit packed 4-byte representation (e.g. 0x00000101) to linear integer (129).
 */
uint32_t sevenBitToLinear(uint32_t packed7Bit);

/**
 * Adds a linear byte offset to a 7-bit packed address (carrying on 0x80).
 */
uint32_t addAddressOffset(uint32_t baseAddress, uint32_t linearOffset);

/**
 * Encodes an 8-bit/16-bit value into 2 nibbled bytes (0x0a, 0x0b) where value = a * 16 + b.
 */
std::pair<uint8_t, uint8_t> encodeNibbled2(int value);

/**
 * Decodes 2 nibbled bytes (0x0a, 0x0b) into an integer: a * 16 + b.
 */
int decodeNibbled2(uint8_t msbNibble, uint8_t lsbNibble);

/**
 * Encodes a 16-bit value into 4 nibbled bytes (0x0a, 0x0b, 0x0c, 0x0d).
 */
std::vector<uint8_t> encodeNibbled4(int value);

/**
 * Decodes 4 nibbled bytes into an integer.
 */
int decodeNibbled4(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3);

/**
 * Encodes a 12-character ASCII name (padded with spaces, clamped to 32..127).
 */
std::vector<uint8_t> encodeAsciiName12(const std::string& name);

/**
 * Decodes a 12-byte ASCII name into a trimmed std::string.
 */
std::string decodeAsciiName12(const uint8_t* data, size_t length);

/**
 * Builds a single Roland JV-1010 DT1 (Data Set 1) message (payload <= 128 bytes).
 */
std::vector<uint8_t> buildDT1(uint8_t deviceId, uint32_t address, const std::vector<uint8_t>& data);

/**
 * Builds a single-byte Roland JV-1010 DT1 message.
 */
std::vector<uint8_t> buildDT1SingleByte(uint8_t deviceId, uint32_t address, uint8_t value);

/**
 * Builds one or more DT1 packets, automatically splitting payloads > 128 bytes
 * into <= 128-byte packets with 7-bit incremented addresses (per JV-1010 manual p. 72).
 */
std::vector<std::vector<uint8_t>> buildSplitDT1(uint8_t deviceId, uint32_t startAddress, const std::vector<uint8_t>& data);

/**
 * Builds a Roland JV-1010 RQ1 (Data Request 1) message.
 * Note: size7Bit is the 7-bit size (e.g. 0x00000101 for 129 bytes).
 */
std::vector<uint8_t> buildRQ1(uint8_t deviceId, uint32_t address, uint32_t size7Bit);

/**
 * Builds a Roland GS DT1 message (Model ID = 42H, 3-byte address, e.g. Scale Tune).
 */
std::vector<uint8_t> buildGS_DT1(uint8_t deviceId, uint32_t address24, const std::vector<uint8_t>& data);

/**
 * Builds Universal Non-Realtime Identity Request: F0 7E <dev> 06 01 F7
 */
std::vector<uint8_t> buildIdentityRequest(uint8_t deviceId = kDefaultDeviceId);

/**
 * Builds Universal Non-Realtime GM System On: F0 7E 7F 09 01 F7
 */
std::vector<uint8_t> buildGMSystemOn();

/**
 * Builds Universal Non-Realtime GM System Off: F0 7E 7F 09 02 F7
 */
std::vector<uint8_t> buildGMSystemOff();

/**
 * Builds Bank Select MSB (CC#0), LSB (CC#32) and Program Change (0..127).
 */
struct BankProgramMessages {
    uint8_t channel; // 0..15
    uint8_t msb;
    uint8_t lsb;
    uint8_t program; // 0..127
};

/**
 * Parses a raw SysEx buffer (with or without F0/F7 framing; JUCE MidiMessage::getSysExData
 * strips F0/F7, whereas raw .syx files include F0/F7).
 */
ParsedSysEx parseSysEx(const uint8_t* data, size_t length);

/**
 * Splits a raw byte stream (e.g. from a .syx file) into individual F0..F7 SysEx messages.
 */
std::vector<std::vector<uint8_t>> extractSysExMessages(const uint8_t* rawBytes, size_t length);

/**
 * Formats a byte vector as uppercase hex string for logging/debugging.
 */
std::string toHexString(const std::vector<uint8_t>& bytes);

} // namespace sysex
} // namespace jv1010
