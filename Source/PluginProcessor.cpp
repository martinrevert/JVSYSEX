#include "PluginProcessor.h"
#include "PluginEditor.h"

JV1010EditorAudioProcessor::JV1010EditorAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, nullptr, "JV1010Params", createParameterLayout()) {

    midiManager_.setSysExReceivedCallback([this](const uint8_t* data, int size) {
        handleIncomingSysEx(data, size);
    });

    // Register parameter listeners for DAW automation -> JV-1010 SysEx/MIDI
    for (auto* param : getParameters()) {
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(param)) {
            apvts_.addParameterListener(ranged->getParameterID(), this);
        }
    }
}

JV1010EditorAudioProcessor::~JV1010EditorAudioProcessor() {
    for (auto* param : getParameters()) {
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(param)) {
            apvts_.removeParameterListener(ranged->getParameterID(), this);
        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout JV1010EditorAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "soundMode", "Sound Mode", juce::StringArray{ "Performance", "Patch", "GM" }, 1));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "patchBank", "Patch Bank",
        juce::StringArray{ "USER", "PR-A", "PR-B", "PR-C", "PR-D (GM)", "PR-E", "XP-A (Session)", "XP-B (EXP)" }, 1));
    params.push_back(std::make_unique<juce::AudioParameterInt>("patchNumber", "Patch Number", 1, 256, 1));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "perfBank", "Performance Bank", juce::StringArray{ "USER", "PR-A", "PR-B" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterInt>("perfNumber", "Performance Number", 1, 32, 1));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "rhythmBank", "Rhythm Bank",
        juce::StringArray{ "USER", "PR-A", "PR-B", "PR-C", "PR-D (GM)", "PR-E", "XP-A (Session)", "XP-B (EXP)" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterInt>("rhythmNumber", "Rhythm Number", 1, 256, 1));

    params.push_back(std::make_unique<juce::AudioParameterInt>("efxType", "EFX Type", 0, 39, 0));
    for (int i = 1; i <= 12; ++i) {
        params.push_back(std::make_unique<juce::AudioParameterInt>(
            "efxParam" + juce::String(i), "EFX Param " + juce::String(i), 0, 127, 0));
    }
    params.push_back(std::make_unique<juce::AudioParameterInt>("efxMixSend", "EFX Mix Out Send", 0, 127, 127));
    params.push_back(std::make_unique<juce::AudioParameterInt>("efxChorusSend", "EFX Chorus Send", 0, 127, 0));
    params.push_back(std::make_unique<juce::AudioParameterInt>("efxReverbSend", "EFX Reverb Send", 0, 127, 0));

    params.push_back(std::make_unique<juce::AudioParameterInt>("chorusLevel", "Chorus Level", 0, 127, 64));
    params.push_back(std::make_unique<juce::AudioParameterInt>("chorusRate", "Chorus Rate", 0, 127, 30));
    params.push_back(std::make_unique<juce::AudioParameterInt>("chorusDepth", "Chorus Depth", 0, 127, 30));
    params.push_back(std::make_unique<juce::AudioParameterInt>("chorusPreDelay", "Chorus Pre-Delay", 0, 127, 0));
    params.push_back(std::make_unique<juce::AudioParameterInt>("chorusFeedback", "Chorus Feedback", 0, 127, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "chorusOutput", "Chorus Output", juce::StringArray{ "MIX", "REV", "MIX+REV" }, 0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "reverbType", "Reverb Type",
        juce::StringArray{ "ROOM1", "ROOM2", "STAGE1", "STAGE2", "HALL1", "HALL2", "DELAY", "PAN-DLY" }, 4));
    params.push_back(std::make_unique<juce::AudioParameterInt>("reverbLevel", "Reverb Level", 0, 127, 64));
    params.push_back(std::make_unique<juce::AudioParameterInt>("reverbTime", "Reverb Time", 0, 127, 64));
    params.push_back(std::make_unique<juce::AudioParameterInt>("reverbHFDamp", "Reverb HF Damp", 0, 17, 16));
    params.push_back(std::make_unique<juce::AudioParameterInt>("delayFeedback", "Delay Feedback", 0, 127, 0));

    return { params.begin(), params.end() };
}

void JV1010EditorAudioProcessor::prepareToPlay(double, int) {}
void JV1010EditorAudioProcessor::releaseResources() {}

void JV1010EditorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    buffer.clear();

    for (const auto metadata : midiMessages) {
        midiManager_.processIncomingDawMidi(metadata.getMessage());
    }
    midiMessages.clear();
    midiManager_.popDawOutgoingMessages(midiMessages);
}

void JV1010EditorAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue) {
    int val = static_cast<int>(newValue);
    bool isPerfMode = (static_cast<int>(apvts_.getRawParameterValue("soundMode")->load()) == 0);
    uint32_t commonBase = isPerfMode ? jv1010::addr::kTempPerformanceBase : jv1010::addr::kPatchModeTempPatchBase;

    if (parameterID == "soundMode") {
        if (val == 2) {
            midiManager_.sendSysEx(jv1010::sysex::buildGMSystemOn());
        } else {
            sendParameterDT1(jv1010::addr::kSystemBase | jv1010::addr::sys::SoundMode, static_cast<uint8_t>(val));
        }
        return;
    }

    if (parameterID == "patchBank" || parameterID == "patchNumber") {
        auto bank = static_cast<jv1010::PatchBank>(static_cast<int>(apvts_.getRawParameterValue("patchBank")->load()));
        int num = static_cast<int>(apvts_.getRawParameterValue("patchNumber")->load()) - 1;
        int ch = synthState_.getByte(jv1010::addr::kSystemBase | jv1010::addr::sys::PatchReceiveChannel);
        selectPatch(bank, num, ch);
        return;
    }

    if (parameterID == "perfBank" || parameterID == "perfNumber") {
        auto bank = static_cast<jv1010::PerfBank>(static_cast<int>(apvts_.getRawParameterValue("perfBank")->load()));
        int num = static_cast<int>(apvts_.getRawParameterValue("perfNumber")->load()) - 1;
        selectPerformance(bank, num);
        return;
    }

    if (parameterID == "rhythmBank" || parameterID == "rhythmNumber") {
        auto bank = static_cast<jv1010::RhythmBank>(static_cast<int>(apvts_.getRawParameterValue("rhythmBank")->load()));
        int num = static_cast<int>(apvts_.getRawParameterValue("rhythmNumber")->load()) - 1;
        selectRhythmSet(bank, num, 9); // Part 10
        return;
    }

    // Effects parameters (map cleanly to both Performance Common and Patch Common)
    if (parameterID == "efxType") {
        uint32_t offset = isPerfMode ? jv1010::addr::perf_common::EFXType : jv1010::addr::patch_common::EFXType;
        sendParameterDT1(commonBase | offset, static_cast<uint8_t>(val));
        return;
    }
    if (parameterID.startsWith("efxParam")) {
        int idx = parameterID.substring(8).getIntValue() - 1;
        if (idx >= 0 && idx < 12) {
            uint32_t baseOff = isPerfMode ? jv1010::addr::perf_common::EFXParam1 : jv1010::addr::patch_common::EFXParam1;
            sendParameterDT1(commonBase | (baseOff + static_cast<uint32_t>(idx)), static_cast<uint8_t>(val));
        }
        return;
    }
    if (parameterID == "efxMixSend") {
        sendParameterDT1(commonBase | (isPerfMode ? jv1010::addr::perf_common::EFXMixOutSendLevel : jv1010::addr::patch_common::EFXMixOutSendLevel), static_cast<uint8_t>(val));
    } else if (parameterID == "efxChorusSend") {
        sendParameterDT1(commonBase | (isPerfMode ? jv1010::addr::perf_common::EFXChorusSendLevel : jv1010::addr::patch_common::EFXChorusSendLevel), static_cast<uint8_t>(val));
    } else if (parameterID == "efxReverbSend") {
        sendParameterDT1(commonBase | (isPerfMode ? jv1010::addr::perf_common::EFXReverbSendLevel : jv1010::addr::patch_common::EFXReverbSendLevel), static_cast<uint8_t>(val));
    } else if (parameterID == "chorusLevel") {
        sendParameterDT1(commonBase | (isPerfMode ? jv1010::addr::perf_common::ChorusLevel : jv1010::addr::patch_common::ChorusLevel), static_cast<uint8_t>(val));
    } else if (parameterID == "chorusRate") {
        sendParameterDT1(commonBase | (isPerfMode ? jv1010::addr::perf_common::ChorusRate : jv1010::addr::patch_common::ChorusRate), static_cast<uint8_t>(val));
    } else if (parameterID == "chorusDepth") {
        sendParameterDT1(commonBase | (isPerfMode ? jv1010::addr::perf_common::ChorusDepth : jv1010::addr::patch_common::ChorusDepth), static_cast<uint8_t>(val));
    } else if (parameterID == "chorusPreDelay") {
        sendParameterDT1(commonBase | (isPerfMode ? jv1010::addr::perf_common::ChorusPreDelay : jv1010::addr::patch_common::ChorusPreDelay), static_cast<uint8_t>(val));
    } else if (parameterID == "chorusFeedback") {
        sendParameterDT1(commonBase | (isPerfMode ? jv1010::addr::perf_common::ChorusFeedback : jv1010::addr::patch_common::ChorusFeedback), static_cast<uint8_t>(val));
    } else if (parameterID == "chorusOutput") {
        sendParameterDT1(commonBase | (isPerfMode ? jv1010::addr::perf_common::ChorusOutput : jv1010::addr::patch_common::ChorusOutput), static_cast<uint8_t>(val));
    } else if (parameterID == "reverbType") {
        sendParameterDT1(commonBase | (isPerfMode ? jv1010::addr::perf_common::ReverbType : jv1010::addr::patch_common::ReverbType), static_cast<uint8_t>(val));
    } else if (parameterID == "reverbLevel") {
        sendParameterDT1(commonBase | (isPerfMode ? jv1010::addr::perf_common::ReverbLevel : jv1010::addr::patch_common::ReverbLevel), static_cast<uint8_t>(val));
    } else if (parameterID == "reverbTime") {
        sendParameterDT1(commonBase | (isPerfMode ? jv1010::addr::perf_common::ReverbTime : jv1010::addr::patch_common::ReverbTime), static_cast<uint8_t>(val));
    } else if (parameterID == "reverbHFDamp") {
        sendParameterDT1(commonBase | (isPerfMode ? jv1010::addr::perf_common::ReverbHFDamp : jv1010::addr::patch_common::ReverbHFDamp), static_cast<uint8_t>(val));
    } else if (parameterID == "delayFeedback") {
        sendParameterDT1(commonBase | (isPerfMode ? jv1010::addr::perf_common::DelayFeedback : jv1010::addr::patch_common::DelayFeedback), static_cast<uint8_t>(val));
    }
}

void JV1010EditorAudioProcessor::selectPatch(jv1010::PatchBank bank, int patchIndexZeroBased, int midiChannelZeroBased) {
    int ch = juce::jlimit(0, 15, midiChannelZeroBased);
    bool needMode = (synthState_.getByte(jv1010::addr::kSystemBase | jv1010::addr::sys::SoundMode) != 1);
    bool needCh   = (synthState_.getByte(jv1010::addr::kSystemBase | jv1010::addr::sys::PatchReceiveChannel) != static_cast<uint8_t>(ch));

    if (needMode) {
        auto modeMsg = jv1010::sysex::buildDT1SingleByte(deviceId_, jv1010::addr::kSystemBase | jv1010::addr::sys::SoundMode, 1);
        synthState_.setByte(jv1010::addr::kSystemBase | jv1010::addr::sys::SoundMode, 1);
        midiManager_.sendChannelMessage(juce::MidiMessage::createSysExMessage(modeMsg.data() + 1, static_cast<int>(modeMsg.size() - 2)));
    }
    if (needCh) {
        auto chMsg = jv1010::sysex::buildDT1SingleByte(deviceId_, jv1010::addr::kSystemBase | jv1010::addr::sys::PatchReceiveChannel, static_cast<uint8_t>(ch));
        synthState_.setByte(jv1010::addr::kSystemBase | jv1010::addr::sys::PatchReceiveChannel, static_cast<uint8_t>(ch));
        midiManager_.sendChannelMessage(juce::MidiMessage::createSysExMessage(chMsg.data() + 1, static_cast<int>(chMsg.size() - 2)));
    }

    auto info = jv1010::getPatchBankSelect(bank, patchIndexZeroBased);
    uint32_t tok = ++selectionRequestToken_;
    juce::WeakReference<JV1010EditorAudioProcessor> safeThis(this);

    auto sendPCAndScheduleRQ1 = [safeThis, tok, ch, info]() {
        if (safeThis == nullptr || safeThis->selectionRequestToken_.load() != tok) return;
        safeThis->midiManager_.sendBankAndProgramChange(ch, info.msb, info.lsb, info.program);

        std::vector<std::vector<uint8_t>> reqs;
        reqs.reserve(5);
        reqs.push_back(jv1010::sysex::buildRQ1(safeThis->deviceId_, jv1010::addr::kPatchModeTempPatchBase, jv1010::addr::kPatchCommonSize7Bit));
        for (int t = 0; t < 4; ++t) {
            reqs.push_back(jv1010::sysex::buildRQ1(safeThis->deviceId_, jv1010::addr::kPatchModeTempPatchBase | jv1010::addr::getPatchToneOffset(t), jv1010::addr::kPatchToneSize7Bit));
        }
        juce::Timer::callAfterDelay(140, [safeThis, tok, reqs = std::move(reqs)]() {
            if (safeThis != nullptr && safeThis->selectionRequestToken_.load() == tok) {
                safeThis->midiManager_.sendSysExQueue(reqs);
            }
        });
    };

    if (needMode) {
        // Give JV-1010 60ms to finish switching into PATCH mode before sending Bank Select + PC
        juce::Timer::callAfterDelay(60, sendPCAndScheduleRQ1);
    } else {
        sendPCAndScheduleRQ1();
    }
}

void JV1010EditorAudioProcessor::selectPerformance(jv1010::PerfBank bank, int perfIndexZeroBased) {
    bool needMode = (synthState_.getByte(jv1010::addr::kSystemBase | jv1010::addr::sys::SoundMode) != 0);
    if (needMode) {
        auto modeMsg = jv1010::sysex::buildDT1SingleByte(deviceId_, jv1010::addr::kSystemBase | jv1010::addr::sys::SoundMode, 0);
        synthState_.setByte(jv1010::addr::kSystemBase | jv1010::addr::sys::SoundMode, 0);
        midiManager_.sendChannelMessage(juce::MidiMessage::createSysExMessage(modeMsg.data() + 1, static_cast<int>(modeMsg.size() - 2)));
    }

    auto info = jv1010::getPerfBankSelect(bank, perfIndexZeroBased);
    int ctrlCh = synthState_.getByte(jv1010::addr::kSystemBase | jv1010::addr::sys::ControlChannel);
    uint8_t perfSysVal = static_cast<uint8_t>(perfIndexZeroBased & 0x1F);
    if (bank == jv1010::PerfBank::PresetA) perfSysVal += 64;
    else if (bank == jv1010::PerfBank::PresetB) perfSysVal += 96;

    uint32_t tok = ++selectionRequestToken_;
    juce::WeakReference<JV1010EditorAudioProcessor> safeThis(this);

    auto sendPerfAndScheduleRQ1 = [safeThis, tok, ctrlCh, info, perfSysVal]() {
        if (safeThis == nullptr || safeThis->selectionRequestToken_.load() != tok) return;
        if (ctrlCh < 16) {
            safeThis->midiManager_.sendBankAndProgramChange(ctrlCh, info.msb, info.lsb, info.program);
        }
        safeThis->sendParameterDT1(jv1010::addr::kSystemBase | jv1010::addr::sys::PerformanceNumber, perfSysVal);

        std::vector<std::vector<uint8_t>> reqs;
        reqs.reserve(17);
        reqs.push_back(jv1010::sysex::buildRQ1(safeThis->deviceId_, jv1010::addr::kTempPerformanceBase, jv1010::addr::kPerfCommonSize7Bit));
        for (int p = 0; p < 16; ++p) {
            reqs.push_back(jv1010::sysex::buildRQ1(safeThis->deviceId_, jv1010::addr::kTempPerformanceBase | (jv1010::addr::kPerfPart1Offset + (static_cast<uint32_t>(p) << 8)), jv1010::addr::kPerfPartSize7Bit));
        }
        juce::Timer::callAfterDelay(140, [safeThis, tok, reqs = std::move(reqs)]() {
            if (safeThis != nullptr && safeThis->selectionRequestToken_.load() == tok) {
                safeThis->midiManager_.sendSysExQueue(reqs);
            }
        });
    };

    if (needMode) {
        juce::Timer::callAfterDelay(60, sendPerfAndScheduleRQ1);
    } else {
        sendPerfAndScheduleRQ1();
    }
}

void JV1010EditorAudioProcessor::selectRhythmSet(jv1010::RhythmBank bank, int kitIndexZeroBased, int midiChannelZeroBased) {
    // Rhythm sets operate in Performance mode (SoundMode = 0)
    bool needMode = (synthState_.getByte(jv1010::addr::kSystemBase | jv1010::addr::sys::SoundMode) != 0);
    if (needMode) {
        auto modeMsg = jv1010::sysex::buildDT1SingleByte(deviceId_, jv1010::addr::kSystemBase | jv1010::addr::sys::SoundMode, 0);
        synthState_.setByte(jv1010::addr::kSystemBase | jv1010::addr::sys::SoundMode, 0);
        midiManager_.sendChannelMessage(juce::MidiMessage::createSysExMessage(modeMsg.data() + 1, static_cast<int>(modeMsg.size() - 2)));
    }

    auto info = jv1010::getRhythmBankSelect(bank, kitIndexZeroBased);
    uint32_t tok = ++selectionRequestToken_;
    juce::WeakReference<JV1010EditorAudioProcessor> safeThis(this);

    auto sendKitAndScheduleRQ1 = [safeThis, tok, midiChannelZeroBased, info]() {
        if (safeThis == nullptr || safeThis->selectionRequestToken_.load() != tok) return;
        safeThis->midiManager_.sendBankAndProgramChange(midiChannelZeroBased, info.msb, info.lsb, info.program);
        juce::Timer::callAfterDelay(140, [safeThis, tok]() {
            if (safeThis != nullptr && safeThis->selectionRequestToken_.load() == tok) {
                safeThis->midiManager_.sendSysEx(
                    jv1010::sysex::buildRQ1(safeThis->deviceId_, jv1010::addr::kTempRhythmBase, jv1010::addr::kRhythmCommonSize7Bit));
            }
        });
    };

    if (needMode) {
        juce::Timer::callAfterDelay(60, sendKitAndScheduleRQ1);
    } else {
        sendKitAndScheduleRQ1();
    }
}

void JV1010EditorAudioProcessor::sendParameterDT1(uint32_t address, uint8_t value) {
    synthState_.setByte(address, value);
    midiManager_.sendSysEx(jv1010::sysex::buildDT1SingleByte(deviceId_, address, value));
}

void JV1010EditorAudioProcessor::sendParameterNibbled2DT1(uint32_t address, int value) {
    synthState_.setNibbled2(address, value);
    auto nib = jv1010::sysex::encodeNibbled2(value);
    midiManager_.sendSysEx(jv1010::sysex::buildDT1(deviceId_, address, { nib.first, nib.second }));
}

void JV1010EditorAudioProcessor::requestTemporaryStateFromHardware() {
    std::vector<std::vector<uint8_t>> reqs;
    reqs.push_back(jv1010::sysex::buildRQ1(deviceId_, jv1010::addr::kSystemBase, jv1010::addr::kSystemCommonSize7Bit));
    reqs.push_back(jv1010::sysex::buildRQ1(deviceId_, jv1010::addr::kPatchModeTempPatchBase, jv1010::addr::kPatchCommonSize7Bit));
    for (int t = 0; t < 4; ++t) {
        reqs.push_back(jv1010::sysex::buildRQ1(deviceId_, jv1010::addr::kPatchModeTempPatchBase | jv1010::addr::getPatchToneOffset(t), jv1010::addr::kPatchToneSize7Bit));
    }
    reqs.push_back(jv1010::sysex::buildRQ1(deviceId_, jv1010::addr::kTempPerformanceBase, jv1010::addr::kPerfCommonSize7Bit));
    midiManager_.sendSysExQueue(reqs);
}

void JV1010EditorAudioProcessor::requestUserBankNamesFromHardware() {
    std::vector<std::vector<uint8_t>> reqs;
    reqs.reserve(162);
    // Request only the 12-byte ASCII name (0x0C) at offset 0x0000 for fast, reliable transfer
    constexpr uint32_t kNameSize7Bit = 0x0000000C;
    // 128 User Patch Names
    for (int i = 0; i < static_cast<int>(jv1010::addr::kNumUserPatches); ++i) {
        reqs.push_back(jv1010::sysex::buildRQ1(deviceId_, jv1010::addr::getUserPatchBase(i), kNameSize7Bit));
    }
    // 32 User Performance Names
    for (int i = 0; i < static_cast<int>(jv1010::addr::kNumUserPerformances); ++i) {
        reqs.push_back(jv1010::sysex::buildRQ1(deviceId_, jv1010::addr::getUserPerfBase(i), kNameSize7Bit));
    }
    // 2 User Rhythm Names
    for (int i = 0; i < static_cast<int>(jv1010::addr::kNumUserRhythmSets); ++i) {
        reqs.push_back(jv1010::sysex::buildRQ1(deviceId_, jv1010::addr::getUserRhythmBase(i), kNameSize7Bit));
    }
    midiManager_.sendSysExQueue(reqs);
}

void JV1010EditorAudioProcessor::requestFullUserBankFromHardware() {
    std::vector<std::vector<uint8_t>> reqs;
    // 128 User Patches (Common + 4 Tones)
    for (int i = 0; i < static_cast<int>(jv1010::addr::kNumUserPatches); ++i) {
        uint32_t base = jv1010::addr::getUserPatchBase(i);
        reqs.push_back(jv1010::sysex::buildRQ1(deviceId_, base, jv1010::addr::kPatchCommonSize7Bit));
        for (int t = 0; t < 4; ++t) {
            reqs.push_back(jv1010::sysex::buildRQ1(deviceId_, base | jv1010::addr::getPatchToneOffset(t), jv1010::addr::kPatchToneSize7Bit));
        }
    }
    // 32 User Performances (Common + 16 Parts)
    for (int i = 0; i < static_cast<int>(jv1010::addr::kNumUserPerformances); ++i) {
        uint32_t base = jv1010::addr::getUserPerfBase(i);
        reqs.push_back(jv1010::sysex::buildRQ1(deviceId_, base, jv1010::addr::kPerfCommonSize7Bit));
        for (int p = 0; p < 16; ++p) {
            reqs.push_back(jv1010::sysex::buildRQ1(deviceId_, base | (jv1010::addr::kPerfPart1Offset + (static_cast<uint32_t>(p) << 8)), jv1010::addr::kPerfPartSize7Bit));
        }
    }
    // 2 User Rhythm Sets (Common + 64 Notes)
    for (int i = 0; i < static_cast<int>(jv1010::addr::kNumUserRhythmSets); ++i) {
        uint32_t base = jv1010::addr::getUserRhythmBase(i);
        reqs.push_back(jv1010::sysex::buildRQ1(deviceId_, base, jv1010::addr::kRhythmCommonSize7Bit));
        for (int k = 0; k < static_cast<int>(jv1010::addr::kRhythmNoteCount); ++k) {
            int midiKey = static_cast<int>(jv1010::addr::kRhythmNoteFirstKey) + k;
            reqs.push_back(jv1010::sysex::buildRQ1(deviceId_, base | jv1010::addr::getRhythmNoteOffset(midiKey), jv1010::addr::kRhythmNoteSize7Bit));
        }
    }
    midiManager_.sendSysExQueue(reqs);
}

void JV1010EditorAudioProcessor::handleIncomingSysEx(const uint8_t* data, int size) {
    auto parsed = jv1010::sysex::parseSysEx(data, static_cast<size_t>(size));
    if (parsed.type == jv1010::sysex::MessageType::JV_DT1 && parsed.checksumValid) {
        synthState_.applyDT1(parsed.address, parsed.payload);
    }
    expansionDetector_.handleSysEx(parsed);
}

void JV1010EditorAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts_.copyState();
    state.setProperty("midiInput", midiManager_.getCurrentInputName(), nullptr);
    state.setProperty("midiOutput", midiManager_.getCurrentOutputName(), nullptr);
    state.setProperty("routingMode", static_cast<int>(midiManager_.getRoutingMode()), nullptr);
    state.setProperty("midiThru", midiManager_.isMidiThruEnabled(), nullptr);
    state.setProperty("expBoardId", expansionDetector_.getInstalledBoardGroupId(), nullptr);
    if (auto xml = state.createXml()) {
        copyXmlToBinary(*xml, destData);
    }
}

void JV1010EditorAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    if (auto xmlState = getXmlFromBinary(data, sizeInBytes)) {
        if (xmlState->hasTagName(apvts_.state.getType())) {
            auto tree = juce::ValueTree::fromXml(*xmlState);
            apvts_.replaceState(tree);
            auto inDev = tree.getProperty("midiInput").toString();
            auto outDev = tree.getProperty("midiOutput").toString();
            if (inDev.isNotEmpty()) midiManager_.openInputDevice(inDev);
            if (outDev.isNotEmpty()) midiManager_.openOutputDevice(outDev);
            midiManager_.setRoutingMode(static_cast<jv1010::MidiRoutingMode>(static_cast<int>(tree.getProperty("routingMode", 2))));
            midiManager_.setMidiThruEnabled(static_cast<bool>(tree.getProperty("midiThru", false)));
            expansionDetector_.setInstalledBoard(static_cast<int>(tree.getProperty("expBoardId", 0)));
        }
    }
}

juce::AudioProcessorEditor* JV1010EditorAudioProcessor::createEditor() {
    return new JV1010EditorAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new JV1010EditorAudioProcessor();
}
