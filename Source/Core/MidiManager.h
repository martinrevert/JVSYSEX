#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_core/juce_core.h>
#include <atomic>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace jv1010 {

enum class MidiRoutingMode {
    DirectHardwareOnly = 0,
    DawBusOnly,
    BothHardwareAndDaw
};

class MidiManager : public juce::MidiInputCallback,
                    private juce::Thread {
public:
    MidiManager();
    ~MidiManager() override;

    // Device enumeration
    juce::StringArray getAvailableInputNames() const;
    juce::StringArray getAvailableOutputNames() const;

    // Port selection (opens physical ports directly, works in both Standalone and VST3)
    bool openInputDevice(const juce::String& deviceIdentifierOrName);
    bool openOutputDevice(const juce::String& deviceIdentifierOrName);
    bool openThruInputDevice(const juce::String& deviceIdentifierOrName);
    void closeInputDevice();
    void closeOutputDevice();
    void closeThruInputDevice();

    juce::String getCurrentInputName() const;
    juce::String getCurrentOutputName() const;
    juce::String getCurrentThruInputName() const;

    bool isInputOpen() const;
    bool isOutputOpen() const;

    // Routing & Thru configuration
    void setRoutingMode(MidiRoutingMode mode) { routingMode_ = mode; }
    MidiRoutingMode getRoutingMode() const { return routingMode_.load(); }

    void setMidiThruEnabled(bool enabled) { midiThruEnabled_ = enabled; }
    bool isMidiThruEnabled() const { return midiThruEnabled_.load(); }

    void setSysExIntervalMs(int ms) { sysexIntervalMs_ = juce::jlimit(5, 100, ms); }
    int getSysExIntervalMs() const { return sysexIntervalMs_.load(); }

    // Sending methods
    void sendSysEx(const std::vector<uint8_t>& fullSysExWithF0F7);
    void sendSysExQueue(const std::vector<std::vector<uint8_t>>& packets);
    void clearSysExQueue();
    size_t getPendingSysExCount();

    void sendChannelMessage(const juce::MidiMessage& msg);
    void sendBankAndProgramChange(int channelZeroBased, uint8_t msb, uint8_t lsb, uint8_t program);
    void sendNoteOn(int channelZeroBased, int noteNumber, uint8_t velocity);
    void sendNoteOff(int channelZeroBased, int noteNumber);
    void sendControlChange(int channelZeroBased, int ccNumber, uint8_t value);
    void sendAllNotesOff(int channelZeroBased);

    // For VST3 processBlock to drain messages destined for the DAW MIDI Output bus
    void popDawOutgoingMessages(juce::MidiBuffer& destBuffer);

    // Process incoming MIDI from DAW bus (in VST3 processBlock)
    void processIncomingDawMidi(const juce::MidiMessage& msg);

    // Callbacks for received SysEx / MIDI
    using SysExReceivedCallback = std::function<void(const uint8_t* data, int size)>;
    void setSysExReceivedCallback(SysExReceivedCallback cb);

    using MidiActivityCallback = std::function<void(bool isTx)>;
    void setMidiActivityCallback(MidiActivityCallback cb);

private:
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;
    void run() override;
    void dispatchNow(const juce::MidiMessage& msg);

    std::unique_ptr<juce::MidiInput> midiInput_;
    std::unique_ptr<juce::MidiInput> thruInput_;
    std::unique_ptr<juce::MidiOutput> midiOutput_;

    juce::String currentInputName_;
    juce::String currentOutputName_;
    juce::String currentThruInputName_;

    mutable std::mutex portMutex_;
    std::mutex queueMutex_;
    std::deque<std::vector<uint8_t>> sysexQueue_;

    std::mutex dawBufferMutex_;
    std::vector<juce::MidiMessage> dawOutgoingQueue_;

    std::atomic<MidiRoutingMode> routingMode_{ MidiRoutingMode::BothHardwareAndDaw };
    std::atomic<bool> midiThruEnabled_{ false };
    std::atomic<int> sysexIntervalMs_{ 20 };

    std::mutex callbackMutex_;
    SysExReceivedCallback onSysExReceived_;
    MidiActivityCallback onMidiActivity_;
};

} // namespace jv1010
