#include "SrJv80CatalogDB.h"
#include <algorithm>

namespace jv1010 {

const std::vector<SrJv80BoardInfo>& getSrJv80Catalog() {
    static const std::vector<SrJv80BoardInfo> kCatalog = {
        {  1, "SR-JV80-01", "Pop",             255, 8, 224, { "St.Concert", "Pop Piano", "Acoustic Bass", "Brass Sect" } },
        {  2, "SR-JV80-02", "Orchestral",      255, 0, 255, { "Warm Violins", "Violin Sect", "Cello Sect", "Harp" } },
        {  3, "SR-JV80-03", "Piano",           200, 0, 148, { "9ft.Grand 1", "9ft.Grand 2", "Bright Grand", "Euro Piano" } },
        {  4, "SR-JV80-04", "Vintage Synth",   255, 0, 255, { "Analog Brass", "JP-8 Strings", "Juno-60 Pad", "OB-8 Pad" } },
        {  5, "SR-JV80-05", "World",           255, 6, 255, { "Sitar", "Tabla", "Koto", "Shakuhachi" } },
        {  6, "SR-JV80-06", "Dance",           200, 8, 240, { "Dance Piano", "House Organ", "909 Kit", "808 Bass" } },
        {  7, "SR-JV80-07", "Super Sound Set", 255, 8, 255, { "Super Grand", "Jazz Guitar", "Tenor Sax", "Pop Brass" } },
        {  8, "SR-JV80-08", "Keyboards of the 60s & 70s", 255, 0, 255, { "Mellotron Str", "B3 Organ", "Clavinet D6", "Wurly" } },
        {  9, "SR-JV80-09", "Session (Onboard)", 255, 8, 206, { "St.Concert", "9ft.Grand 1", "9ft.Grand 2", "Euro Classic" } },
        { 10, "SR-JV80-10", "Bass & Drums",    204, 16, 232, { "Marcus Slap", "Finger Jazz", "Acoustic Kit", "Brush Kit" } },
        { 11, "SR-JV80-11", "Techno",          255, 8, 255, { "TB-303 Acid", "Rave Lead", "Techno Kit", "TR-909" } },
        { 12, "SR-JV80-12", "Hip-Hop",         255, 8, 255, { "HipHop Kit", "Sub Bass", "Vinyl Scratch", "Rhodes Chd" } },
        { 13, "SR-JV80-13", "Vocal",           255, 0, 224, { "Gospel Choir", "Jazz Scat", "Classical Aah", "Doo Wah" } },
        { 14, "SR-JV80-14", "Asia",            224, 4, 255, { "Erhu", "Pipa", "Gamelan", "Taiko" } },
        { 15, "SR-JV80-15", "Special FX",      200, 8, 255, { "Sci-Fi Amb", "Thunder", "Explosion", "Foley FX" } },
        { 16, "SR-JV80-16", "Orchestral II",   255, 0, 255, { "Celtic Harp", "Uilleann Pipe", "Solo Violin", "Choir" } },
        { 17, "SR-JV80-17", "Country",         255, 4, 255, { "Pedal Steel", "Telecaster", "Fiddle", "Banjo" } },
        { 18, "SR-JV80-18", "Latin",           255, 8, 255, { "Salsa Brass", "Conga Kit", "Timbales", "Tres" } },
        { 19, "SR-JV80-19", "House",           255, 8, 255, { "House Piano", "Deep Bass", "Garage Organ", "909 House" } }
    };
    return kCatalog;
}

const SrJv80BoardInfo* findSrJv80ByGroupId(int groupId) {
    for (const auto& board : getSrJv80Catalog()) {
        if (board.groupId == groupId) {
            return &board;
        }
    }
    return nullptr;
}

const SrJv80BoardInfo* findSrJv80ByFirstPatchName(const std::string& patch001Name) {
    if (patch001Name.empty()) return nullptr;
    for (const auto& board : getSrJv80Catalog()) {
        if (!board.samplePatchNames.empty() && patch001Name == board.samplePatchNames[0]) {
            return &board;
        }
    }
    return nullptr;
}

} // namespace jv1010
