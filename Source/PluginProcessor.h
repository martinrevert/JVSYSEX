#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Core/MidiManager.h"
#include "Core/SysExProtocol.h"
#include "Core/ExpansionDetector.h"
#include "Model/JV1010State.h"

class JV1010EditorAudioProcessor : public juce::AudioProcessor,
                                   public juce::AudioProcessorValueTreeState::Listener {
public:
    JV1010EditorAudioProcessor();
    ~JV1010EditorAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void parameterChanged(const juce::String& parameterID, float newValue) override;

    jv1010::MidiManager& getMidiManager() { return midiManager_; }
    jv1010::JV1010State& getSynthState() { return synthState_; }
    jv1010::ExpansionDetector& getExpansionDetector() { return expansionDetector_; }
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts_; }

    uint8_t getDeviceId() const { return deviceId_; }
    void setDeviceId(uint8_t devId) { deviceId_ = devId & 0x7F; }

    // High-level actions that update state AND transmit MIDI/SysEx
    void selectPatch(jv1010::PatchBank bank, int patchIndexZeroBased, int midiChannelZeroBased = 0);
    void selectPerformance(jv1010::PerfBank bank, int perfIndexZeroBased);
    void selectRhythmSet(jv1010::RhythmBank bank, int kitIndexZeroBased, int midiChannelZeroBased = 9);
    void sendParameterDT1(uint32_t address, uint8_t value);
    void sendParameterNibbled2DT1(uint32_t address, int value);
    void requestTemporaryStateFromHardware();
    void requestUserBankNamesFromHardware();
    void requestFullUserBankFromHardware();

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void handleIncomingSysEx(const uint8_t* data, int size);

    jv1010::MidiManager midiManager_;
    jv1010::JV1010State synthState_;
    jv1010::ExpansionDetector expansionDetector_{ midiManager_, synthState_ };
    juce::AudioProcessorValueTreeState apvts_;
    uint8_t deviceId_ = jv1010::sysex::kDefaultDeviceId;
    std::atomic<uint32_t> selectionRequestToken_{ 0 };

    JUCE_DECLARE_WEAK_REFERENCEABLE(JV1010EditorAudioProcessor)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JV1010EditorAudioProcessor)
};
