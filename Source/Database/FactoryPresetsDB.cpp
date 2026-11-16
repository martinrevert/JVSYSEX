#include "FactoryPresetsDB.h"
#include <unordered_map>

namespace jv1010 {

const std::array<CategoryInfo, 39>& getPatchCategories() {
    static const std::array<CategoryInfo, 39> kCategories = {{
        {  0, "---", "NO ASSIGN",    "ALL" },
        {  1, "PNO", "AC.PIANO",     "PIANO" },
        {  2, "EP ", "EL.PIANO",     "PIANO" },
        {  3, "KEY", "KEYBOARDS",    "KEY&ORGAN" },
        {  4, "BEL", "BELL",         "KEY&ORGAN" },
        {  5, "MLT", "MALLET",       "KEY&ORGAN" },
        {  6, "ORG", "ORGAN",        "KEY&ORGAN" },
        {  7, "ACD", "ACCORDION",    "KEY&ORGAN" },
        {  8, "HRM", "HARMONICA",    "KEY&ORGAN" },
        {  9, "AGT", "AC.GUITAR",    "GUITAR/BASS" },
        { 10, "EGT", "EL.GUITAR",    "GUITAR/BASS" },
        { 11, "DGT", "DIST.GUITAR",  "GUITAR/BASS" },
        { 12, "BS ", "BASS",         "GUITAR/BASS" },
        { 13, "SBS", "SYNTH BASS",   "GUITAR/BASS" },
        { 14, "STR", "STRINGS",      "ORCH/BRASS" },
        { 15, "ORC", "ORCHESTRA",    "ORCH/BRASS" },
        { 16, "HIT", "HIT&STAB",     "ORCH/BRASS" },
        { 17, "WND", "WIND",         "ORCH/BRASS" },
        { 18, "FLT", "FLUTE",        "ORCH/BRASS" },
        { 19, "BRS", "AC.BRASS",     "ORCH/BRASS" },
        { 20, "SBR", "SYNTH BRASS",  "ORCH/BRASS" },
        { 21, "SAX", "SAX",          "ORCH/BRASS" },
        { 22, "HLD", "HARD LEAD",    "SYNTH/PAD" },
        { 23, "SLD", "SOFT LEAD",    "SYNTH/PAD" },
        { 24, "TEK", "TECHNO SYNTH", "SYNTH/PAD" },
        { 25, "PLS", "PULSATING",    "SYNTH/PAD" },
        { 26, "FX ", "SYNTH FX",     "SYNTH/PAD" },
        { 27, "SYN", "OTHER SYNTH",  "SYNTH/PAD" },
        { 28, "BPD", "BRIGHT PAD",   "SYNTH/PAD" },
        { 29, "SPD", "SOFT PAD",     "SYNTH/PAD" },
        { 30, "VOX", "VOX",          "SYNTH/PAD" },
        { 31, "PLK", "PLUCKED",      "ETHNIC" },
        { 32, "ETH", "ETHNIC",       "ETHNIC" },
        { 33, "FRT", "FRETTED",      "ETHNIC" },
        { 34, "PRC", "PERCUSSION",   "RHYTHM&SFX" },
        { 35, "SFX", "SOUND FX",     "RHYTHM&SFX" },
        { 36, "BTS", "BEAT&GROOVE",  "RHYTHM&SFX" },
        { 37, "DRM", "DRUMS",        "RHYTHM&SFX" },
        { 38, "CMB", "COMBINATION",  "RHYTHM&SFX" }
    }};
    return kCategories;
}

namespace {
struct RawPatch { const char* name; int voices; bool solo; int cat; };

const std::array<RawPatch, 128> kUserPatches = {{
    {"Tremendously",4,false,0}, {"St.Concert",4,false,1}, {"CyberTrance2",4,false,24}, {"SessionNyln2",3,false,9},
    {"ViennaStrgs3",4,false,14}, {"R&R Brass",3,false,19}, {"Velo Power",4,false,11}, {"Super Tenor",3,false,21},
    {"Breathy Humz",2,false,30}, {"Ow Bass",1,true,12}, {"Dunes",4,false,26}, {"Celestial 2",4,false,4},
    {"Tone Wh.Solo",3,false,6}, {"Poly Brs",4,false,20}, {"Combing Slow",3,false,28}, {"SA Rhodes 5",3,false,2},
    {"Adrenaline",4,false,22}, {"DanceStack 1",3,false,24}, {"Flying Waltz",4,false,25}, {"East Europe",2,false,31},
    {"Silky Way",2,false,29}, {"Techno Dream",3,false,24}, {"Raverborg",4,false,24}, {"Pan Pipes",2,false,18},
    {"Pretty Bells",2,false,4}, {"Glistening",4,false,26}, {"Metal Solo",4,false,11}, {"StateXLChrd2",4,true,24},
    {"Raggatronic",4,false,25}, {"ChristmasFlt",2,false,18}, {"Acc.de Paris",2,false,7}, {"DreamVoices2",2,false,30},
    {"VintageCall2",4,false,23}, {"Trancing Pad",2,false,25}, {"OldiesOrgan2",2,false,6}, {"X..? Whistle",3,false,23},
    {"Analog Drama",3,false,29}, {"BPF Lead",1,false,22}, {"Clean Tele",4,false,10}, {"Earth Blow",2,false,29},
    {"Pulsatronic",3,false,25}, {"Funky Slap",1,true,12}, {"Echo Rhodes",4,false,2}, {"pp Harmonium",1,false,6},
    {"Blue Notes",4,false,38}, {"Aurora",4,false,29}, {"Breathy Brs",3,false,20}, {"Enlighten",4,false,30},
    {"Tube Smoke",2,false,11}, {"Perelandra",4,false,26}, {"SquareLead 2",2,false,23}, {"Shadows",4,false,26},
    {"Organizer",3,false,24}, {"Full Orchest",4,false,15}, {"B'on d'moov!",3,false,24}, {"Sugar Bell 2",4,false,4},
    {"Player's EP",2,false,2}, {"4pole Bass",2,true,13}, {"Octapad",3,false,29}, {"Tria Bells",4,false,4},
    {"Wire Pad",3,false,27}, {"Singing'Mini",1,true,22}, {"Heirborne",4,false,28}, {"Trumpet",2,false,19},
    {"Steel Away",3,false,9}, {"D-50 Stack",4,false,4}, {"Solo Strat",3,false,10}, {"Soap Opera",1,false,6},
    {"Sop.Sax mf",2,false,21}, {"Dimensional",2,false,29}, {"Blusey OD",2,false,11}, {"Stacc.Heaven",4,false,4},
    {"PhaseBlipper",2,false,27}, {"Pure Tibet",1,false,26}, {"Afterlife",3,false,25}, {"Perky B",2,false,6},
    {"JUNO Power!",4,false,28}, {"SessionBrass",4,false,19}, {"JX SqrCarpet",2,false,29}, {"Phaser MC",2,false,13},
    {"Sweep Clav 2",3,false,27}, {"BluesHarp",1,false,8}, {"Scat Flute",2,false,18}, {"5thPad 2",4,false,28},
    {"Belfry Chime",3,false,4}, {"Atmos Harp",4,false,31}, {"DesertCrystl",4,false,9}, {"December Sky",4,false,29},
    {"Upright Pno",3,false,1}, {"Darkshine",4,false,29}, {"Exotic Velo",4,false,5}, {"Atlantis 2",4,false,26},
    {"Creamy",2,false,11}, {"Morning Lite",2,false,4}, {"DanceStack 5",4,false,24}, {"D50FantaPerc",3,false,28},
    {"Resojuice",2,true,24}, {"Silicon Str",4,false,28}, {"Keep :-)",2,false,24}, {"Sm.Brass Grp",4,false,19},
    {"Echo Piano",3,false,1}, {"HeavenlyPad",2,false,29}, {"Sequalog",4,false,24}, {"Solo Steel",4,false,9},
    {"Spectrum Mod",4,false,28}, {"Delicate EP",2,false,2}, {"Triumph Brs",3,false,20}, {"Flute",1,false,18},
    {"2.2 Bell Pad",4,false,4}, {"JustLovely 2",1,false,29}, {"GR500 TmpDly",2,false,28}, {"Dawn 2 Dusk",3,false,29},
    {"Gluey Pad",3,false,29}, {"SquareKeys 3",2,false,4}, {"Arasian Morn",4,false,30}, {"D'light",2,false,29},
    {"Perky Noize",3,false,34}, {"Mod DirtyWav",3,false,28}, {"Moving Glass",1,false,29}, {"Sci-Fi Str",3,false,26},
    {"OD 5ths",3,false,11}, {"Far East",4,false,32}, {"Phobos",2,false,26}, {"Cyber Dreams",3,false,25},
    {"Cave 2",4,false,26}, {"Helium Queen",4,true,26}, {"Rain Forest",4,false,35}, {"Crowds",4,false,35}
}};

const std::array<RawPatch, 128> kPrAPatches = {{
    {"64voicePiano",1,false,1}, {"Bright Piano",1,false,1}, {"Classique",2,false,1}, {"Nice Piano",3,false,1},
    {"Piano Thang",3,false,1}, {"Power Grand",3,false,1}, {"House Piano",2,false,24}, {"E.Grand",1,false,1},
    {"MIDIed Grand",3,false,1}, {"Piano Blend",3,false,1}, {"West Coast",4,false,2}, {"PianoStrings",4,false,1},
    {"Bs/Pno+Brs",4,false,38}, {"Waterhodes",2,false,2}, {"S.A.E.P.",3,false,2}, {"SA Rhodes 1",4,false,2},
    {"SA Rhodes 2",2,false,2}, {"Stiky Rhodes",3,false,2}, {"Dig Rhodes",2,false,2}, {"Nylon EPiano",4,false,2},
    {"Nylon Rhodes",4,false,2}, {"Rhodes Mix",3,false,2}, {"PsychoRhodes",2,false,2}, {"Tremo Rhodes",4,false,2},
    {"MK-80 Rhodes",1,false,2}, {"MK-80 Phaser",1,false,2}, {"Delicate EP",2,false,2}, {"Octa Rhodes1",4,false,2},
    {"Octa Rhodes2",4,false,2}, {"JV Rhodes+",4,false,2}, {"EP+Mod Pad",4,false,2}, {"Mr.Mellow",4,false,2},
    {"Comp Clav",1,false,3}, {"Klavinet",4,false,3}, {"Winger Clav",4,false,3}, {"Phaze Clav 1",2,false,3},
    {"Phaze Clav 2",1,false,3}, {"Phuzz Clav",2,false,3}, {"Chorus Clav",1,false,3}, {"Claviduck",2,false,3},
    {"Velo-Rez Clv",1,false,3}, {"Clavicembalo",4,false,3}, {"Analog Clav1",1,false,3}, {"Analog Clav2",1,false,3},
    {"Metal Clav",3,false,3}, {"Full Stops",2,false,6}, {"Ballad B",3,false,6}, {"Mellow Bars",4,false,6},
    {"AugerMentive",3,false,6}, {"Perky B",2,false,6}, {"The Big Spin",3,false,6}, {"Gospel Spin",3,false,6},
    {"Roller Spin",3,false,6}, {"Rocker Spin",3,false,6}, {"Tone Wh.Solo",3,false,6}, {"Purple Spin",4,false,6},
    {"60's LeadORG",2,false,6}, {"Assalt Organ",3,false,6}, {"D-50 Organ",2,false,6}, {"Cathedral",4,false,6},
    {"Church Pipes",4,false,6}, {"Poly Key",3,false,27}, {"Poly Saws",4,false,27}, {"Poly Pulse",4,false,27},
    {"Dual Profs",3,false,27}, {"Saw Mass",4,false,27}, {"Poly Split",4,false,27}, {"Poly Brass",3,false,20},
    {"Stackoid",4,false,27}, {"Poly Rock",4,false,27}, {"D-50 Stack",4,false,4}, {"Fantasia JV",4,false,4},
    {"Jimmee Dee",4,false,4}, {"Heavenals",4,false,4}, {"Mallet Pad",4,false,4}, {"Huff N Stuff",3,false,4},
    {"Puff 1080",2,false,27}, {"BellVox 1080",4,false,4}, {"Fantasy Vox",4,false,4}, {"Square Keys",2,false,4},
    {"Childlike",4,false,4}, {"Music Box",3,false,4}, {"Toy Box",2,false,4}, {"Wave Bells",4,false,4},
    {"Tria Bells",4,false,4}, {"Beauty Bells",4,false,4}, {"Music Bells",2,false,4}, {"Pretty Bells",2,false,4},
    {"Pulse Key",3,false,27}, {"Wide Tubular",4,false,4}, {"AmbienceVibe",4,false,5}, {"Warm Vibes",2,false,5},
    {"Dyna Marimba",1,false,5}, {"Bass Marimba",4,false,5}, {"Nomad Perc",3,false,5}, {"Ethno Metals",4,false,4},
    {"Islands Mlt",4,false,5}, {"Steelin Keys",3,false,5}, {"Steel Drums",1,false,5}, {"Voicey Pizz",3,false,14},
    {"Sitar",2,false,31}, {"Drone Split",4,false,31}, {"Ethnopluck",4,false,31}, {"Jamisen",2,false,31},
    {"Dulcimer",2,false,31}, {"East Melody",2,false,32}, {"MandolinTrem",4,false,33}, {"Nylon Gtr",1,false,9},
    {"Gtr Strings",3,false,9}, {"Steel Away",3,false,9}, {"Heavenly Gtr",4,false,9}, {"12str Gtr 1",2,false,9},
    {"12str Gtr 2",3,false,9}, {"Jz Gtr Hall",1,false,10}, {"LetterFrmPat",4,false,10}, {"Jazz Scat",3,false,10},
    {"Lounge Gig",3,false,38}, {"JC Strat",1,false,10}, {"Twin Strats",3,false,10}, {"JV Strat",2,false,10},
    {"Syn Strat",2,false,10}, {"Rotary Gtr",2,false,10}, {"Muted Gtr",1,false,10}, {"SwitchOnMute",2,false,10},
    {"Power Trip",2,false,11}, {"Crunch Split",4,false,11}, {"Rezodrive",2,true,11}, {"RockYurSocks",4,true,11}
}};

const std::array<RawPatch, 128> kPrBPatches = {{
    {"Dist Gtr 1",3,false,11}, {"Dist Gtr 2",3,false,11}, {"R&R Chunk",4,false,11}, {"Phripphuzz",1,true,11},
    {"Grungeroni",3,false,11}, {"Black Widow",4,false,11}, {"Velo-Wah Gtr",1,false,10}, {"Mod-Wah Gtr",2,false,11},
    {"Pick Bass",1,true,12}, {"Hip Bass",2,false,12}, {"Perc.Bass",3,true,12}, {"Homey Bass",2,true,12},
    {"Finger Bass",1,true,12}, {"Nylon Bass",2,false,12}, {"Ac.Upright",1,true,12}, {"Wet Fretls",1,true,12},
    {"Fretls Dry",2,false,12}, {"Slap Bass 1",2,false,12}, {"Slap Bass 2",1,true,12}, {"Slap Bass 3",1,true,12},
    {"Slap Bass 4",2,false,12}, {"4 Pole Bass",1,true,13}, {"Tick Bass",4,true,13}, {"House Bass",3,true,13},
    {"Mondo Bass",3,true,13}, {"Clk AnalogBs",2,true,13}, {"Bass In Face",2,false,13}, {"101 Bass",2,true,13},
    {"Noiz Bass",2,true,13}, {"Super Jup Bs",2,false,13}, {"Occitan Bass",3,false,13}, {"Hugo Bass",4,true,13},
    {"Multi Bass",2,false,13}, {"Moist Bass",2,true,13}, {"BritelowBass",4,true,13}, {"Untamed Bass",3,true,13},
    {"Rubber Bass",3,true,13}, {"Stereoww Bs",3,true,13}, {"Wonder Bass",3,true,13}, {"Deep Bass",2,false,13},
    {"Super JX Bs",2,true,13}, {"W<RED>-Bass",4,false,13}, {"HI-Ring Bass",3,false,13}, {"Euro Bass",2,true,13},
    {"SinusoidRave",1,true,13}, {"Alternative",2,true,25}, {"Acid Line",1,true,25}, {"Auto TB-303",3,true,24},
    {"Hihat Tekno",2,false,24}, {"Velo Tekno 1",3,true,24}, {"Raggatronic",4,false,25}, {"Blade Racer",4,false,25},
    {"S&H Pad",1,false,25}, {"Syncrosonix",3,false,25}, {"Fooled Again",1,false,25}, {"Alive",3,false,25},
    {"Velo Tekno 2",2,false,24}, {"Rezoid",4,false,24}, {"Raverborg",4,false,24}, {"Blow Hit",4,false,16},
    {"Hammer Bell",3,false,4}, {"Seq Mallet",2,false,24}, {"Intentions",3,false,24}, {"Pick It",3,false,24},
    {"Analog Seq",2,false,24}, {"Impact Vox",4,false,24}, {"TeknoSoloVox",2,false,24}, {"X-Mod Man",2,false,25},
    {"Paz <==> Zap",1,true,25}, {"4 Hits 4 You",4,false,16}, {"Impact",4,false,16}, {"Phase Hit",3,false,16},
    {"Tekno Hit 1",2,false,16}, {"Tekno Hit 2",2,false,16}, {"Tekno Hit 3",4,false,16}, {"Reverse Hit",3,false,16},
    {"SquareLead 1",3,false,23}, {"SquareLead 2",2,false,23}, {"You and Luck",2,true,23}, {"Belly Lead",4,false,23},
    {"WhistlinAtom",2,false,23}, {"Edye Boost",2,true,22}, {"MG Solo",4,true,22}, {"FXM Saw Lead",4,true,22},
    {"Sawteeth",3,true,22}, {"Smoothe",2,true,23}, {"MG Lead",2,true,23}, {"MG Interval",4,true,22},
    {"Pulse Lead 1",3,false,22}, {"Pulse Lead 2",4,true,23}, {"Little Devil",4,true,22}, {"Loud SynLead",4,true,22},
    {"Analog Lead",2,true,22}, {"5th Lead",2,true,22}, {"Flute",2,false,18}, {"Piccolo",1,false,18},
    {"VOX Flute",4,false,18}, {"Air Lead",2,false,18}, {"Pan Pipes",2,false,18}, {"Airplaaane",4,false,24},
    {"Taj Mahal",1,false,32}, {"Raya Shaku",3,false,32}, {"Oboe mf",1,false,17}, {"Oboe Express",2,false,17},
    {"Clarinet mp",1,false,17}, {"ClariExpress",2,false,17}, {"Mitzva Split",4,false,17}, {"ChamberWinds",4,false,17},
    {"ChamberWoods",3,false,17}, {"Film Orch",4,false,15}, {"Sop.Sax mf",2,false,21}, {"Alto Sax",3,false,21},
    {"AltoLead Sax",3,false,21}, {"Tenor Sax",3,false,21}, {"Baritone Sax",3,false,21}, {"Take A Tenor",4,false,21},
    {"Sax Section",4,false,21}, {"Bigband Sax",4,false,21}, {"Harmonica",2,false,8}, {"Harmo Blues",2,false,8},
    {"BluesHarp",1,false,8}, {"Hillbillys",4,false,38}, {"French Bags",4,false,32}, {"Majestic Tpt",1,true,19},
    {"Voluntare",2,false,19}, {"2Trumpets",2,false,19}, {"Tpt Sect",4,false,19}, {"Mute TP mod",4,false,19}
}};

const std::array<RawPatch, 128> kPrCPatches = {{
    {"Harmon Mute",1,false,19}, {"Tp&Sax Sect",4,false,19}, {"Sax+Tp+Tb",3,false,19}, {"Brass Sect",4,false,19},
    {"Trombone",1,false,19}, {"Hybrid Bones",4,false,19}, {"Noble Horns",4,false,19}, {"Massed Horns",3,false,19},
    {"Horn Swell",4,false,19}, {"Brass It!",4,false,20}, {"Brass Attack",3,false,20}, {"Archimede",3,false,20},
    {"Rugby Horn",3,false,20}, {"MKS-80 Brass",2,false,20}, {"True ANALOG",2,false,20}, {"Dark Vox",2,false,30},
    {"RandomVowels",4,false,30}, {"Angels Sing",2,false,30}, {"Pvox Oooze",3,false,30}, {"Longing...",3,false,30},
    {"Arasian Morn",4,false,30}, {"Beauty Vox",3,false,30}, {"Mary-AnneVox",4,false,30}, {"Belltree Vox",4,false,30},
    {"Vox Panner",2,false,30}, {"Spaced Voxx",4,false,30}, {"Glass Voices",3,false,30}, {"Tubular Vox",4,false,30},
    {"Velo Voxx",2,false,30}, {"Wavox",3,false,30}, {"Doos",1,false,30}, {"Synvox Comps",4,false,30},
    {"Vocal Oohz",3,false,30}, {"LFO Vox",1,false,25}, {"St.Strings",2,false,14}, {"Warm Strings",4,false,14},
    {"Somber Str",4,false,14}, {"Marcato",2,false,14}, {"Bright Str",2,false,14}, {"String Ens",4,false,14},
    {"TremoloStrng",2,false,14}, {"Chambers",3,false,14}, {"ViolinCello",4,false,14}, {"Symphonique",4,false,15},
    {"Film Octaves",4,false,14}, {"Film Layers",4,false,15}, {"Bass Pizz",4,false,14}, {"Real Pizz",3,false,14},
    {"Harp On It",3,false,31}, {"Harp",2,false,31}, {"JP-8 Str 1",2,false,14}, {"JP-8 Str 2",3,false,14},
    {"E-Motion Pad",4,false,29}, {"JP-8 Str 3",4,false,14}, {"Vintage Orch",4,false,28}, {"JUNO Strings",3,false,28},
    {"Gigantalog",4,false,28}, {"PWM Strings",3,false,28}, {"Warmth",2,false,29}, {"ORBit Pad",2,false,29},
    {"Deep Strings",2,false,14}, {"Pulsify",4,false,29}, {"Pulse Pad",4,false,29}, {"Greek Power",4,false,28},
    {"Harmonicum",2,false,28}, {"D-50 Heaven",2,false,28}, {"Afro Horns",3,false,20}, {"Pop Pad",4,false,29},
    {"Dreamesque",4,false,29}, {"Square Pad",4,false,29}, {"JP-8 Hollow",4,false,29}, {"JP-8Haunting",4,false,29},
    {"Heirborne",4,false,28}, {"Hush Pad",4,false,28}, {"Jet Pad 1",2,false,29}, {"Jet Pad 2",2,false,29},
    {"Phaze Pad",3,false,29}, {"Phaze Str",4,false,29}, {"Jet Str Ens",2,false,29}, {"Pivotal Pad",4,false,28},
    {"3D Flanged",1,false,29}, {"Fantawine",4,false,28}, {"Glassy Pad",3,false,29}, {"Moving Glass",1,false,29},
    {"Glasswaves",3,false,29}, {"Shiny Pad",4,false,29}, {"ShiftedGlass",2,false,29}, {"Chime Pad",3,false,29},
    {"Spin Pad",2,false,29}, {"Rotary Pad",4,false,29}, {"Dawn 2 Dusk",3,false,29}, {"Aurora",4,false,29},
    {"Strobe Mode",4,false,25}, {"Albion",2,false,25}, {"Running Pad",4,false,25}, {"Stepped Pad",4,false,25},
    {"Random Pad",4,false,25}, {"SoundtrkDANC",4,false,25}, {"Flying Waltz",4,false,25}, {"Vanishing",1,false,26},
    {"5th Sweep",4,false,28}, {"Phazweep",4,false,25}, {"Big BPF",4,false,24}, {"MG Sweep",4,false,28},
    {"CeremonyTimp",3,false,34}, {"Dyno Toms",4,false,34}, {"Sands ofTime",4,false,26}, {"Inertia",4,false,26},
    {"Vektogram",4,false,26}, {"Crash Pad",4,false,26}, {"Feedback VOX",4,false,26}, {"Cascade",1,false,26},
    {"Shattered",2,false,26}, {"NextFrontier",2,false,26}, {"Pure Tibet",1,false,26}, {"Chime Wash",4,false,26},
    {"Night Shade",4,false,26}, {"Tortured",4,false,26}, {"Dissimilate",4,false,26}, {"Dunes",4,false,26},
    {"Ocean Floor",1,false,26}, {"Cyber Space",3,false,26}, {"Biosphere",2,false,26}, {"Variable Run",4,false,26},
    {"Ice Hall",2,false,26}, {"ComputerRoom",4,false,26}, {"Inverted",4,false,26}, {"Terminate",3,false,26}
}};

const std::array<RawPatch, 128> kPrDPatches = {{
    {"Piano 1",2,false,1}, {"Piano 2",2,false,1}, {"Piano 3",2,false,1}, {"Honky-tonk",2,false,1},
    {"E.Piano 1",2,false,2}, {"E.Piano 2",4,false,2}, {"Harpsichord",2,false,3}, {"Clav.",2,false,3},
    {"Celesta",1,false,3}, {"Glockenspiel",2,false,4}, {"Music Box",1,false,4}, {"Vibraphone",1,false,5},
    {"Marimba",2,false,5}, {"Xylophone",2,false,5}, {"Tubular-bell",2,false,4}, {"Santur",2,false,31},
    {"Organ 1",1,false,6}, {"Organ 2",1,false,6}, {"Organ 3",2,false,6}, {"Church Org.1",2,false,6},
    {"Reed Organ",1,false,6}, {"Accordion Fr",2,false,7}, {"Harmonica",1,false,8}, {"Bandoneon",2,false,7},
    {"Nylon-str.Gt",1,false,9}, {"Steel-str.Gt",1,false,9}, {"Jazz Gt.",1,false,10}, {"Clean Gt.",1,false,10},
    {"Muted Gt.",1,false,10}, {"Overdrive Gt",1,false,11}, {"DistortionGt",1,false,11}, {"Gt.Harmonics",3,false,10},
    {"Acoustic Bs.",3,false,12}, {"Fingered Bs.",1,false,12}, {"Picked Bs.",1,false,12}, {"Fretless Bs.",1,false,12},
    {"Slap Bass 1",1,false,12}, {"Slap Bass 2",2,false,12}, {"Synth Bass 1",1,false,13}, {"Synth Bass 2",1,false,13},
    {"Violin",1,false,14}, {"Viola",1,false,14}, {"Cello",1,false,14}, {"Contrabass",1,false,14},
    {"Tremolo Str",1,false,14}, {"PizzicatoStr",1,false,14}, {"Harp",2,false,31}, {"Timpani",1,false,34},
    {"Strings",2,false,14}, {"Slow Strings",1,false,14}, {"Syn.Strings1",2,false,29}, {"Syn.Strings2",2,false,29},
    {"Choir Aahs",3,false,30}, {"Voice Oohs",1,false,30}, {"SynVox",1,false,30}, {"OrchestraHit",2,false,16},
    {"Trumpet",2,false,19}, {"Trombone",1,false,19}, {"Tuba",2,false,19}, {"MutedTrumpet",1,false,19},
    {"French Horn",2,false,19}, {"Brass 1",2,false,19}, {"Synth Brass1",1,false,20}, {"Synth Brass2",2,false,20},
    {"Soprano Sax",1,false,21}, {"Alto Sax",1,false,21}, {"Tenor Sax",1,false,21}, {"Baritone Sax",2,false,21},
    {"Oboe",2,false,17}, {"English Horn",2,false,17}, {"Bassoon",2,false,17}, {"Clarinet",1,false,17},
    {"Piccolo",1,false,18}, {"Flute",1,false,18}, {"Recorder",2,false,18}, {"Pan Flute",2,false,18},
    {"Bottle Blow",2,false,18}, {"Shakuhachi",1,false,32}, {"Whistle",1,false,18}, {"Ocarina",2,false,18},
    {"Square Wave",2,false,22}, {"Saw Wave",2,false,22}, {"Syn.Calliope",2,false,23}, {"Chiffer Lead",2,false,23},
    {"Charang",3,false,22}, {"Solo Vox",2,false,23}, {"5th Saw Wave",3,false,22}, {"Bass & Lead",2,false,22},
    {"Fantasia",3,false,27}, {"Warm Pad",2,false,29}, {"Polysynth",2,false,27}, {"Space Voice",2,false,30},
    {"Bowed Glass",3,false,29}, {"Metal Pad",2,false,28}, {"Halo Pad",3,false,28}, {"Sweep Pad",2,false,29},
    {"Ice Rain",2,false,27}, {"Soundtrack",2,false,29}, {"Crystal",2,false,4}, {"Atmosphere",2,false,9},
    {"Brightness",3,false,27}, {"Goblin",2,false,25}, {"Echo Drops",2,false,28}, {"Star Theme",2,false,28},
    {"Sitar",1,false,31}, {"Banjo",1,false,33}, {"Shamisen",2,false,31}, {"Koto",1,false,31},
    {"Kalimba",1,false,31}, {"Bag Pipe",3,false,32}, {"Fiddle",1,false,14}, {"Shanai",1,false,32},
    {"Tinkle Bell",4,false,4}, {"Agogo",1,false,34}, {"Steel Drums",1,false,5}, {"Woodblock",1,false,34},
    {"Taiko",4,false,34}, {"Melo. Tom 1",2,false,34}, {"Synth Drum",2,false,34}, {"Reverse Cym.",2,false,34},
    {"Gt.FretNoise",1,false,9}, {"Breath Noise",2,false,26}, {"Seashore",3,false,35}, {"Bird",4,false,35},
    {"Telephone 1",1,false,35}, {"Helicopter",2,false,35}, {"Applause",4,false,35}, {"Gun Shot",2,false,35}
}};

const std::array<RawPatch, 128> kPrEPatches = {{
    {"Echo Piano",3,false,1}, {"Upright Pno",3,false,1}, {"RD-1000",3,false,1}, {"Player's EP",2,false,2},
    {"D-50 Rhodes",4,false,2}, {"Innocent EP",2,false,2}, {"Echo Rhodes",4,false,2}, {"See-Thru EP",3,false,2},
    {"FM BellPiano",3,false,2}, {"Ring E.Piano",4,false,2}, {"Soap Opera",1,false,6}, {"Dirty Organ",3,false,6},
    {"Surf's Up!",2,false,6}, {"Organesque",3,false,6}, {"pp Harmonium",1,false,6}, {"PieceOfCheez",1,false,3},
    {"Harpsy Clav",2,false,3}, {"Exotic Velo",4,false,5}, {"HolidayCheer",4,false,4}, {"Morning Lite",2,false,4},
    {"Prefab Chime",3,false,4}, {"Belfry Chime",3,false,4}, {"Stacc.Heaven",4,false,4}, {"2.2 Bell Pad",4,false,4},
    {"Far East",4,false,32}, {"Wire Pad",3,false,27}, {"PhaseBlipper",2,false,27}, {"Sweep Clav",3,false,27},
    {"Glider",2,false,27}, {"Solo Steel",4,false,9}, {"DesertCrystl",4,false,9}, {"Clear Guitar",3,false,10},
    {"Solo Strat",3,false,10}, {"Feed Me!",4,false,11}, {"Tube Smoke",2,false,11}, {"Creamy",2,false,11},
    {"Blusey OD",2,false,11}, {"Grindstone",2,false,11}, {"OD 5ths",3,false,11}, {"East Europe",2,false,31},
    {"Dulcitar",4,false,31}, {"Atmos Harp",4,false,31}, {"Pilgrimage",4,false,31}, {"202 Rude Bs",2,true,13},
    {"2pole Bass",2,true,13}, {"4pole Bass",2,true,13}, {"Phaser MC",2,false,13}, {"Miniphaser",2,false,13},
    {"Acid TB",1,true,13}, {"Full Orchest",4,false,15}, {"Str + Winds",4,false,15}, {"Flute 2080",2,false,18},
    {"Scat Flute",2,false,18}, {"Sax Choir",4,false,21}, {"Ballad Trump",4,false,19}, {"Sm.Brass Grp",4,false,19},
    {"Royale",4,false,19}, {"Brass Mutes",2,false,19}, {"Breathy Brs",3,false,20}, {"3 Osc Brass",3,false,20},
    {"P5 Polymod",2,false,20}, {"Triumph Brs",3,false,20}, {"Techno Dream",3,false,24}, {"Organizer",3,false,24},
    {"Civilization",3,false,24}, {"Mental Chord",4,true,24}, {"House Chord",4,true,24}, {"Sequalog",4,false,24},
    {"Booster Bips",2,false,24}, {"VintagePlunk",4,true,24}, {"Plik-Plok",2,false,24}, {"RingSequence",4,false,24},
    {"Cyber Swing",4,false,24}, {"Keep :-)",2,false,24}, {"Resojuice",2,true,24}, {"B'on d'moov!",3,false,24},
    {"Dist TB-303",2,true,24}, {"Temple of JV",4,false,25}, {"Planet Asia",4,false,25}, {"Afterlife",3,false,25},
    {"Trancing Pad",2,false,25}, {"Pulsatronic",3,false,25}, {"Cyber Dreams",3,false,25}, {"Warm Pipe",1,true,23},
    {"Pure Pipe",2,false,23}, {"SH-2000",2,true,23}, {"X..? Whistle",3,false,23}, {"Jay Vee Solo",3,false,23},
    {"Progresso Ld",4,true,22}, {"Adrenaline",4,false,22}, {"Enlighten",4,false,30}, {"Glass Blower",3,false,29},
    {"Earth Blow",2,false,29}, {"JX SqrCarpet",2,false,29}, {"Dimensional",2,false,29}, {"Jupiterings",2,false,29},
    {"Analog Drama",3,false,29}, {"Rich Dynapad",4,false,29}, {"Silky Way",2,false,29}, {"Gluey Pad",3,false,29},
    {"BandPass Mod",2,false,29}, {"Soundtraque",2,false,29}, {"Translucence",4,false,29}, {"Darkshine",4,false,29},
    {"D'light",2,false,29}, {"December Sky",4,false,29}, {"Octapad",3,false,29}, {"JUNO Power!",4,false,28},
    {"Spectrum Mod",4,false,28}, {"Stringsheen",3,false,28}, {"GR500 TmpDly",2,false,28}, {"Mod DirtyWav",3,false,28},
    {"Silicon Str",4,false,28}, {"D50FantaPerc",3,false,28}, {"Rotodreams",3,false,28}, {"Blue Notes",4,false,38},
    {"RiversOfTime",4,false,26}, {"Phobos",2,false,26}, {"2 0 8 0",4,false,26}, {"Unearthly",4,false,26},
    {"Glistening",4,false,26}, {"Sci-Fi Str",3,false,26}, {"Shadows",4,false,26}, {"Helium Queen",4,true,26},
    {"Sci-Fi FX x4",1,false,26}, {"Perky Noize",3,false,34}, {"Droplet",3,false,35}, {"Rain Forest",4,false,35}
}};

const std::array<RawPatch, 255> kXpAPatches = {{
    {"St.Concert",4,false,1}, {"9ft.Grand 1",4,false,1}, {"9ft.Grand 2",4,false,1}, {"Euro Classic",2,false,1},
    {"St.Pno & Str",4,false,1}, {"Compress Pno",1,false,1}, {"LA Session",4,false,1}, {"Water Piano",4,false,1},
    {"Vibra Rhodes",3,false,2}, {"Stack Rhodes",4,false,2}, {"ArcoEnsemble",2,false,14}, {"Vienna Strgs",3,false,14},
    {"Str Adagio",2,false,14}, {"Oct Strings",3,false,14}, {"Silky Filter",2,false,14}, {"Soft Strings",2,false,14},
    {"SlowStr.Sect",2,false,14}, {"GiantStrings",4,false,14}, {"Str+Choir 1",4,false,14}, {"Str+Choir 2",4,false,14},
    {"Str+Choir 3",4,false,30}, {"Breathy Humz",2,false,30}, {"Dream Voices",2,false,30}, {"Mmmms",2,false,30},
    {"Chorale",1,false,30}, {"Space Men",3,false,30}, {"Choir Mm+Aah",4,false,30}, {"Ivory Mist",4,false,30},
    {"PercussiVox",4,false,30}, {"Mysteriouso",3,false,30}, {"Phase Mmhs",3,false,30}, {"AmbientStory",4,false,30},
    {"Venus",3,false,30}, {"SessionBrass",4,false,19}, {"Port.Tpts",2,false,19}, {"R&R Brass",3,false,19},
    {"Echo Brass",4,false,19}, {"E.Coast Brs",4,false,19}, {"Bop Soli",2,false,19}, {"Soft Saxes",4,false,21},
    {"Orchestral",4,false,19}, {"Octalog Hrn",2,false,20}, {"Tpt Soloist",1,false,19}, {"Legato Tpt",2,false,19},
    {"Dyno Trumpet",2,false,19}, {"Ethno-Trumps",2,false,19}, {"Super Tenor",3,false,21}, {"TenorExpress",2,false,21},
    {"T.Sax f",1,false,21}, {"Legato Flute",2,false,18}, {"Touch Flute",2,false,18}, {"NewAge Flute",1,false,18},
    {"Flute inMist",2,false,18}, {"Hybrid Flute",2,false,18}, {"Flute & Cla",3,false,18}, {"ChristmasFlt",2,false,18},
    {"Fifth Flute",2,false,18}, {"Cosmic Flute",2,false,18}, {"Acc.de Paris",2,false,7}, {"Paris 50's",4,false,7},
    {"Musette Ens",4,false,7}, {"Montmartre",2,false,7}, {"Sad Akordion",3,false,7}, {"Fr.Accordion",1,false,7},
    {"Fr.Accord 2",2,false,7}, {"Troubadour",3,false,9}, {"SessionNylon",3,false,9}, {"Solo Nylon 1",3,false,9},
    {"Solo Nylon 2",2,false,9}, {"Nylon & Str",4,false,9}, {"Nylon & Flt",3,false,9}, {"Nylon Chord",3,false,9},
    {"Stratar",2,false,10}, {"Clean Tele",4,false,10}, {"Nashville",1,false,10}, {"Super Trem",4,false,10},
    {"SpaghettiGtr",2,false,10}, {"Duanne'sTone",2,false,10}, {"Big Hair Ld",2,false,11}, {"Metal Solo",4,false,11},
    {"Crunch Tone",1,false,11}, {"Overdriven",1,false,11}, {"Blues Tele",4,false,11}, {"Tube Double",2,false,11},
    {"Neil's Rust",4,false,11}, {"Short Crunch",4,false,11}, {"X-Fade Metal",4,false,11}, {"Velo Power",4,false,11},
    {"Phazy Chunk",4,false,11}, {"Reso Tele",1,false,10}, {"Wah Wah BPM",4,false,10}, {"Rock P.Bass1",1,false,12},
    {"Rock P.Bass2",2,false,12}, {"Rock P.Bass3",4,false,12}, {"Big Jazz Bs",3,false,12}, {"BriteJazz Bs",1,false,12},
    {"Ch.Jazz Bs",2,false,12}, {"Mellow Jz Bs",2,false,12}, {"Mute E.Bs",1,false,12}, {"Octabahn Bs",4,false,12},
    {"Slobbery Bs",2,false,13}, {"Phase Worm",2,false,13}, {"Euro Rave Bs",2,false,13}, {"Pumpin' Bs",3,false,13},
    {"Tech NoBase",2,false,13}, {"Bad Acid Bs",2,false,13}, {"CheepEcho Bs",3,false,13}, {"Manic Bs",4,false,13},
    {"JP6 Sqr Key",2,false,27}, {"Square drops",1,false,27}, {"Celestial",3,false,4}, {"Heavenly Eko",4,false,4},
    {"JD-800 Nomad",3,false,4}, {"Vibrolater",4,false,4}, {"MartianChime",4,false,4}, {"Big Wet Blip",4,false,4},
    {"Amazing Echo",4,false,27}, {"DelaySession",4,false,26}, {"Deletex",4,false,9}, {"Tarlia",3,false,4},
    {"Mahoroba",4,false,28}, {"Eurotek Clav",2,false,22}, {"Dope Resoclv",1,false,24}, {"Tekno Juno",2,false,24},
    {"Buzzzzzzzzzz",2,false,22}, {"Slop-a-rama",2,false,27}, {"Isn't Pretty",3,false,27}, {"Polywasp",1,false,27},
    {"Quixelate",4,false,27}, {"Trangoa Wave",2,false,22}, {"Spiked Cheez",3,false,27}, {"Glassy Cheez",3,false,27},
    {"Super 808Cow",4,false,27}, {"Arpeggiatoid",4,false,27}, {"Euro Hit 1",4,false,16}, {"Euro Hit 2",2,false,16},
    {"Rave Slice",1,false,16}, {"Str Torture",2,false,27}, {"Juno Harpsi",3,false,27}, {"Big Mess Pad",4,false,27},
    {"Harpsichoir",4,false,3}, {"Echo Juno",3,false,27}, {"Phazerave",2,false,24}, {"DanceStack 1",3,false,24},
    {"DanceStack 2",4,false,24}, {"DanceStack 3",4,false,24}, {"DanceStack 4",3,false,24}, {"DanceStack 5",4,false,24},
    {"DanceStack 6",3,false,24}, {"DanceStack 7",4,false,24}, {"DanceStack 8",4,false,24}, {"Eurotek Brs",4,false,24},
    {"Synergy Brs",4,false,20}, {"PortaSynthex",3,false,27}, {"Razor VCOs",4,false,27}, {"Big PWM",2,false,27},
    {"Flutey Stack",2,false,27}, {"Wobbly 5th",4,false,27}, {"Tekno Square",3,false,24}, {"Trance VoXxX",4,false,24},
    {"Random Rave",3,false,24}, {"Raver Circus",4,false,24}, {"Resorave",2,false,24}, {"Flangomatic",2,false,27},
    {"O-Zu-Nu",4,false,26}, {"Sub Divided",1,false,27}, {"Ancient Sqr",2,false,27}, {"Fat Flange",2,false,27},
    {"Phaze NRG",1,false,27}, {"Phase Vox",3,false,30}, {"Systekno",4,false,24}, {"On the move!",2,false,22},
    {"XP'ration",2,false,25}, {"Big Ensemble",4,false,27}, {"Lazerette",3,false,27}, {"Fazed String",4,false,29},
    {"Combing Slow",3,false,28}, {"Jet Stack",4,false,28}, {"Phazeslopad",4,false,28}, {"Rize Mass",3,false,26},
    {"Portent",2,false,26}, {"DCO Sweeper",3,false,26}, {"Sweep Rain",4,false,26}, {"Sweep Stack",3,false,26},
    {"Big Vectors",4,false,26}, {"Poly Swell",2,false,29}, {"Alchemy",3,false,26}, {"Soli-na",2,false,27},
    {"90s Str Mach",4,false,27}, {"Ultra Cheez",2,false,28}, {"Juno-60 Pad",3,false,29}, {"Progressive",2,false,27},
    {"Build-Up Syn",3,false,27}, {"Atlantis 2",4,false,26}, {"Perelandra",4,false,26}, {"Plutonium",4,false,26},
    {"Nautilus",2,false,26}, {"Metal Dreams",2,false,26}, {"Glass Clouds",4,false,26}, {"Harmonicloud",4,false,26},
    {"Shining Veil",3,false,26}, {"Ethereal JX",2,false,26}, {"Striking 5th",4,false,26}, {"Meow 5ths",2,false,26},
    {"Stepflanger",3,false,26}, {"Happy LFOs",4,false,25}, {"Aero Insect",3,false,25}, {"Beat Sweeper",2,false,25},
    {"Wormy Lead",3,false,22}, {"Atmo Lead",3,false,23}, {"Caliolead",3,false,23}, {"Tweedles",2,false,22},
    {"Raw PWM",2,false,22}, {"Voc Solo 5th",4,false,23}, {"Dirty Lead",2,false,22}, {"Boostweeper1",2,false,26},
    {"Boostweeper2",2,false,26}, {"B3 Filth",4,false,6}, {"Phazed Organ",3,false,6}, {"VSw Vibrafon",3,false,5},
    {"SA Vibe",1,false,5}, {"Rich Vibes",2,false,5}, {"SpaceGamelan",4,false,5}, {"Toy Vibe",3,false,5},
    {"Analog Bomb",2,false,35}, {"Seashore 2",4,false,35}, {"Creation",4,false,26}, {"Cyberjunkie",4,false,26},
    {"Sci-Fi Bells",3,false,26}, {"Shine on",1,false,26}, {"DEMO Piano1",4,false,1}, {"DEMO Piano2",4,false,1},
    {"DEMO PnoVox",3,false,1}, {"DEMO Str 1",2,false,14}, {"DEMO Str 2",2,false,14}, {"DEMO Str 3",2,false,14},
    {"DEMO SynPuls",2,false,25}, {"DEMO 5thPad",4,false,29}, {"DEMO Choir",2,false,30}, {"DEMO Brass",1,false,19},
    {"DEMO Tenor",3,false,21}, {"DEMO Tpt",1,false,19}, {"DEMO Flute",2,false,18}, {"DEMO Nylon",4,false,9},
    {"DEMO PhaseGt",4,false,10}, {"DEMO DistGt1",4,false,11}, {"DEMO DistGt2",3,false,11}, {"DEMO Strat",3,false,10},
    {"DEMO SlapBs",2,false,12}, {"DEMO P.Bass",1,false,12}, {"DEMO SynBs",2,false,13}, {"DEMO SynLead",4,false,23},
    {"DEMO Insect",4,false,25}, {"DEMO Buzzzzz",2,false,27}, {"DEMO Crowd",4,false,35}
}};
} // namespace

const std::vector<PatchCatalogEntry>& getFactoryPatches() {
    static const std::vector<PatchCatalogEntry> kAll = []() {
        std::vector<PatchCatalogEntry> v;
        v.reserve(1023);
        auto append = [&](PatchBank b, const auto& arr) {
            for (int i = 0; i < static_cast<int>(arr.size()); ++i) {
                v.push_back({ b, i, arr[static_cast<size_t>(i)].name, arr[static_cast<size_t>(i)].voices,
                              arr[static_cast<size_t>(i)].solo, arr[static_cast<size_t>(i)].cat });
            }
        };
        append(PatchBank::User, kUserPatches);
        append(PatchBank::PresetA, kPrAPatches);
        append(PatchBank::PresetB, kPrBPatches);
        append(PatchBank::PresetC, kPrCPatches);
        append(PatchBank::PresetD, kPrDPatches);
        append(PatchBank::PresetE, kPrEPatches);
        append(PatchBank::Session, kXpAPatches);
        return v;
    }();
    return kAll;
}

const std::vector<PerfCatalogEntry>& getFactoryPerformances() {
    static const std::vector<PerfCatalogEntry> kPerfs = {{
        // USER (01-32)
        {PerfBank::User,0,"EasternSplit",true}, {PerfBank::User,1,"Opening Orch",true}, {PerfBank::User,2,"Feedback EP",true},
        {PerfBank::User,3,"Humming Vox",true}, {PerfBank::User,4,"Tekno Loop 1",true}, {PerfBank::User,5,"Fr.Horn Sect",true},
        {PerfBank::User,6,"SpaceCarrier",true}, {PerfBank::User,7,"Orchestral",true}, {PerfBank::User,8,"Nebular Vox",true},
        {PerfBank::User,9,"Terminator",true}, {PerfBank::User,10,"Flying Jazz",true}, {PerfBank::User,11,"Sweeper",true},
        {PerfBank::User,12,"Rave Split",true}, {PerfBank::User,13,"Multi Sax",true}, {PerfBank::User,14,"Cosmic Dawn",true},
        {PerfBank::User,15,"Bass / Lead",true}, {PerfBank::User,16,"S&H / Pad",true}, {PerfBank::User,17,"AcPiano+Pad",true},
        {PerfBank::User,18,"Kicks Attack",true}, {PerfBank::User,19,"Step Brass",true}, {PerfBank::User,20,"Drone / Pipe",true},
        {PerfBank::User,21,"Chime Dreams",true}, {PerfBank::User,22,"Tekno Loop 2",true}, {PerfBank::User,23,"Big Band",true},
        {PerfBank::User,24,"Labyrinth",true}, {PerfBank::User,25,"White Hole",true}, {PerfBank::User,26,"Cyber Sweep",true},
        {PerfBank::User,27,"Tekno Asia",true}, {PerfBank::User,28,"1080 Fantasy",true}, {PerfBank::User,29,"Pop Ballad",true},
        {PerfBank::User,30,"Rhythmatic",true}, {PerfBank::User,31,"Power JV",true},
        // PR-A (01-32)
        {PerfBank::PresetA,0,"House Set",false}, {PerfBank::PresetA,1,"Analectro",false}, {PerfBank::PresetA,2,"Anatronic",false},
        {PerfBank::PresetA,3,"Tekno Pop 1",false}, {PerfBank::PresetA,4,"Tekno Pop 2",false}, {PerfBank::PresetA,5,"Hard Core",false},
        {PerfBank::PresetA,6,"Hi Energy",false}, {PerfBank::PresetA,7,"Pop Dance",false}, {PerfBank::PresetA,8,"Acid Set",false},
        {PerfBank::PresetA,9,"Ambient Set",false}, {PerfBank::PresetA,10,"Electro Pop",false}, {PerfBank::PresetA,11,"Pop Set 1",false},
        {PerfBank::PresetA,12,"Pop Set 2",false}, {PerfBank::PresetA,13,"Pop Set 3",false}, {PerfBank::PresetA,14,"Pop Set 4",false},
        {PerfBank::PresetA,15,"L.A. Ballad",false}, {PerfBank::PresetA,16,"Hip Hop Set",false}, {PerfBank::PresetA,17,"Funk Rock",false},
        {PerfBank::PresetA,18,"Funk Fusion",false}, {PerfBank::PresetA,19,"Heavy Metal",false}, {PerfBank::PresetA,20,"Heavy Kids",true},
        {PerfBank::PresetA,21,"Latin Set",false}, {PerfBank::PresetA,22,"BrazilianSet",false}, {PerfBank::PresetA,23,"New Age 1",false},
        {PerfBank::PresetA,24,"New Age 2",false}, {PerfBank::PresetA,25,"Orchestra",false}, {PerfBank::PresetA,26,"Concerto",false},
        {PerfBank::PresetA,27,"Film Score 1",false}, {PerfBank::PresetA,28,"Film Score 2",false}, {PerfBank::PresetA,29,"Symphonic",false},
        {PerfBank::PresetA,30,"Chamber Set",false}, {PerfBank::PresetA,31,"Baroque Set",false},
        // PR-B (01-32)
        {PerfBank::PresetB,0,"Africa",false}, {PerfBank::PresetB,1,"World Ethnic",false}, {PerfBank::PresetB,2,"Asian Ethnic",false},
        {PerfBank::PresetB,3,"Asian Band",false}, {PerfBank::PresetB,4,"60's Set",false}, {PerfBank::PresetB,5,"Blues Band",false},
        {PerfBank::PresetB,6,"Country Band",false}, {PerfBank::PresetB,7,"Folk Set",false}, {PerfBank::PresetB,8,"Reggae Band",false},
        {PerfBank::PresetB,9,"FunkWah Band",false}, {PerfBank::PresetB,10,"Funkin'Phaze",false}, {PerfBank::PresetB,11,"Zydeco Band",false},
        {PerfBank::PresetB,12,"New Orleans",false}, {PerfBank::PresetB,13,"Dixieland",false}, {PerfBank::PresetB,14,"Big Band Set",false},
        {PerfBank::PresetB,15,"Cont.Jazz 1",false}, {PerfBank::PresetB,16,"Cont.Jazz 2",false}, {PerfBank::PresetB,17,"Ac.Jazz Set",false},
        {PerfBank::PresetB,18,"Gospel Set",false}, {PerfBank::PresetB,19,"All Strings",false}, {PerfBank::PresetB,20,"All Brass",false},
        {PerfBank::PresetB,21,"All Piano 1",false}, {PerfBank::PresetB,22,"All Piano 2",false}, {PerfBank::PresetB,23,"All Keyboard",false},
        {PerfBank::PresetB,24,"All Organ",false}, {PerfBank::PresetB,25,"All Winds",false}, {PerfBank::PresetB,26,"All Bells",true},
        {PerfBank::PresetB,27,"Mlt & Perc",false}, {PerfBank::PresetB,28,"All Seq",false}, {PerfBank::PresetB,29,"All Bass",false},
        {PerfBank::PresetB,30,"All Pad",false}, {PerfBank::PresetB,31,"All FX",false}
    }};
    return kPerfs;
}

const std::vector<RhythmCatalogEntry>& getFactoryRhythmSets() {
    static const std::vector<RhythmCatalogEntry> kRhythms = {{
        {RhythmBank::User, 0, "HouseDrumSet 1"}, {RhythmBank::User, 1, "JazzDrumSet1"},
        {RhythmBank::PresetA, 0, "PopDrumSet 1"}, {RhythmBank::PresetA, 1, "PopDrumSet 2"},
        {RhythmBank::PresetB, 0, "PowerDrumSet"}, {RhythmBank::PresetB, 1, "RaveDrumSet"},
        {RhythmBank::PresetC, 0, "JazzDrumSet2"}, {RhythmBank::PresetC, 1, "OrchDrumSet"},
        {RhythmBank::PresetD, 0, "GM Drum Set"},  {RhythmBank::PresetD, 1, "BrushDrumSet"},
        {RhythmBank::PresetE, 0, "PowerDrmSet2"}, {RhythmBank::PresetE, 1, "PowerRaveSet"},
        {RhythmBank::Session, 0, "SessionSet 1"}, {RhythmBank::Session, 1, "SessionSet 2"},
        {RhythmBank::Session, 2, "SessionSet 3"}, {RhythmBank::Session, 3, "SessionSet 4"},
        {RhythmBank::Session, 4, "SessionSet 5"}, {RhythmBank::Session, 5, "SessionSet 6"},
        {RhythmBank::Session, 6, "SessionSet 7"}, {RhythmBank::Session, 7, "Demo Drum"}
    }};
    return kRhythms;
}

std::string getFactoryPatchName(PatchBank bank, int indexZeroBased) {
    for (const auto& entry : getFactoryPatches()) {
        if (entry.bank == bank && entry.indexZeroBased == indexZeroBased) {
            return entry.name;
        }
    }
    return "Patch " + std::to_string(indexZeroBased + 1);
}

std::string getFactoryPerfName(PerfBank bank, int indexZeroBased) {
    for (const auto& entry : getFactoryPerformances()) {
        if (entry.bank == bank && entry.indexZeroBased == indexZeroBased) {
            return entry.name;
        }
    }
    return "Perf " + std::to_string(indexZeroBased + 1);
}

std::string getFactoryRhythmName(RhythmBank bank, int indexZeroBased) {
    for (const auto& entry : getFactoryRhythmSets()) {
        if (entry.bank == bank && entry.indexZeroBased == indexZeroBased) {
            return entry.name;
        }
    }
    return "Rhythm " + std::to_string(indexZeroBased + 1);
}

} // namespace jv1010
