#include "Core/SysExProtocol.h"
#include "Model/JV1010State.h"
#include "Model/EfxAlgorithms.h"
#include "Database/FactoryPresetsDB.h"
#include "Database/SrJv80CatalogDB.h"
#include "Librarian/SyxMidFileIO.h"
#include "Librarian/PatchMorphRandom.h"
#include <cassert>
#include <iostream>
#include <vector>

using namespace jv1010;

static void testOfficialManualExamples() {
    // Page 86 Example 1: Performance Common REVERB TYPE -> DELAY (06H)
    auto dt1 = sysex::buildDT1SingleByte(0x10, 0x01000028, 0x06);
    std::vector<uint8_t> expected1 = { 0xF0, 0x41, 0x10, 0x6A, 0x12, 0x01, 0x00, 0x00, 0x28, 0x06, 0x51, 0xF7 };
    assert(dt1 == expected1);

    // Page 86 Example 2: RQ1 USER:03 Performance Part 3
    auto rq1_2 = sysex::buildRQ1(0x10, 0x10021200, 0x00000019);
    std::vector<uint8_t> expected2 = { 0xF0, 0x41, 0x10, 0x6A, 0x11, 0x10, 0x02, 0x12, 0x00, 0x00, 0x00, 0x00, 0x19, 0x43, 0xF7 };
    assert(rq1_2 == expected2);

    // Page 87 Example 3: RQ1 Temporary Performance
    auto rq1_3 = sysex::buildRQ1(0x10, 0x01000000, 0x00001F19);
    std::vector<uint8_t> expected3 = { 0xF0, 0x41, 0x10, 0x6A, 0x11, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x19, 0x47, 0xF7 };
    assert(rq1_3 == expected3);

    // Page 87 Example 4: RQ1 Temporary Performance + All Temp Patches
    auto rq1_4 = sysex::buildRQ1(0x10, 0x01000000, 0x010F1701);
    std::vector<uint8_t> expected4 = { 0xF0, 0x41, 0x10, 0x6A, 0x11, 0x01, 0x00, 0x00, 0x00, 0x01, 0x0F, 0x17, 0x01, 0x57, 0xF7 };
    assert(rq1_4 == expected4);

    // Page 87 Scale Tune GS (42H) Arabian-type Scale on Part 1
    std::vector<uint8_t> arabian = { 0x3A, 0x6D, 0x3E, 0x34, 0x0D, 0x38, 0x6B, 0x3C, 0x6F, 0x40, 0x36, 0x0F };
    auto gs = sysex::buildGS_DT1(0x10, 0x401140, arabian);
    std::vector<uint8_t> expectedGS = {
        0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x11, 0x40,
        0x3A, 0x6D, 0x3E, 0x34, 0x0D, 0x38, 0x6B, 0x3C, 0x6F, 0x40, 0x36, 0x0F,
        0x76, 0xF7
    };
    assert(gs == expectedGS);

    // Parse back DT1 and GS
    auto parsed1 = sysex::parseSysEx(dt1.data(), dt1.size());
    assert(parsed1.type == sysex::MessageType::JV_DT1);
    assert(parsed1.checksumValid);
    assert(parsed1.address == 0x01000028);
    assert(parsed1.payload.size() == 1 && parsed1.payload[0] == 0x06);

    auto parsedGS = sysex::parseSysEx(gs.data(), gs.size());
    assert(parsedGS.type == sysex::MessageType::GS_DT1);
    assert(parsedGS.checksumValid);
    assert(parsedGS.address == 0x401140);
    assert(parsedGS.payload == arabian);
}

static void testNibbledAnd7BitAddressing() {
    auto n4 = sysex::encodeNibbled4(1258);
    assert((n4 == std::vector<uint8_t>{ 0x00, 0x04, 0x0E, 0x0A }));
    assert(sysex::decodeNibbled4(n4[0], n4[1], n4[2], n4[3]) == 1258);

    auto n2 = sysex::encodeNibbled2(254);
    assert((n2 == std::pair<uint8_t, uint8_t>{ 0x0F, 0x0E }));
    assert(sysex::decodeNibbled2(n2.first, n2.second) == 254);

    assert(sysex::linearTo7Bit(129) == 0x00000101);
    assert(sysex::sevenBitToLinear(0x00000101) == 129);
    assert(sysex::addAddressOffset(0x03001000, 128) == 0x03001100);
}

static void testStateSerializationAndApply() {
    JV1010State state;
    PatchData patch;
    patch.initDefault("Test Patch  ");
    patch.tones[0][addr::patch_tone::ReverbSendLevel] = 99;
    patch.tones[0][addr::patch_tone::CutoffFrequency] = 77;

    auto packets = state.serializePatchToDT1(0x10, addr::kPatchModeTempPatchBase, patch);
    // 1 Common packet + 4 Tones * 2 split packets (128 + 1) = 9 packets
    assert(packets.size() == 9);

    JV1010State target;
    for (const auto& pkt : packets) {
        auto parsed = sysex::parseSysEx(pkt.data(), pkt.size());
        assert(parsed.type == sysex::MessageType::JV_DT1);
        assert(parsed.checksumValid);
        assert(target.applyDT1(parsed.address, parsed.payload));
    }

    auto restored = target.getPatchModeTempPatch();
    assert(restored.getName() == "Test Patch");
    assert(restored.tones[0][addr::patch_tone::CutoffFrequency] == 77);
    assert(restored.tones[0][addr::patch_tone::ReverbSendLevel] == 99);
}

static void testEfxAlgorithmsCount() {
    const auto& algos = getEfxAlgorithms();
    assert(algos.size() == 40);
    assert(std::string(algos[0].name) == "01: STEREO-EQ");
    assert(std::string(algos[39].name) == "40: CHORUS/FLANGER (parallel)");
}

static void testDatabaseAndFileRoundTrip() {
    // Factory database counts from official manual
    assert(getFactoryPatches().size() == 1023);
    assert(getFactoryPerformances().size() == 96);
    assert(getFactoryRhythmSets().size() == 20);
    assert(getSrJv80Catalog().size() == 19);
    assert(findSrJv80ByGroupId(9) != nullptr);

    // Export & Import .syx and .mid round-trip
    JV1010State src;
    PatchData p;
    p.initDefault("RoundTripPch");
    p.tones[0][addr::patch_tone::CutoffFrequency] = 88;
    src.setUserPatch(5, p);

    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
    auto syxFile = tempDir.getChildFile("jv1010_test.syx");
    auto midFile = tempDir.getChildFile("jv1010_test.mid");

    assert(SyxMidFileIO::exportToSyx(syxFile, src, ExportScope::UserPatches128));
    assert(SyxMidFileIO::exportToMid(midFile, src, ExportScope::UserPatches128));

    JV1010State dstSyx;
    auto resSyx = SyxMidFileIO::importFile(syxFile, dstSyx);
    assert(resSyx.success);
    assert(dstSyx.getUserPatch(5).getName() == "RoundTripPch");
    assert(dstSyx.getUserPatch(5).tones[0][addr::patch_tone::CutoffFrequency] == 88);

    JV1010State dstMid;
    auto resMid = SyxMidFileIO::importFile(midFile, dstMid);
    assert(resMid.success);
    assert(dstMid.getUserPatch(5).getName() == "RoundTripPch");
    assert(dstMid.getUserPatch(5).tones[0][addr::patch_tone::CutoffFrequency] == 88);

    syxFile.deleteFile();
    midFile.deleteFile();
}

int main() {
    testOfficialManualExamples();
    testNibbledAnd7BitAddressing();
    testStateSerializationAndApply();
    testEfxAlgorithmsCount();
    testDatabaseAndFileRoundTrip();
    std::cout << "All JV-1010 SysEx, State, Database & File I/O tests passed!" << std::endl;
    return 0;
}
