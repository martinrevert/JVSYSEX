#include "MidiManager.h"

namespace jv1010 {

MidiManager::MidiManager() : juce::Thread("JV1010_SysEx_Tx") {
    startThread();
}

MidiManager::~MidiManager() {
    signalThreadShouldExit();
    notify();
    stopThread(2000);
    closeInputDevice();
    closeThruInputDevice();
    closeOutputDevice();
}

juce::StringArray MidiManager::getAvailableInputNames() const {
    juce::StringArray names;
    for (const auto& info : juce::MidiInput::getAvailableDevices()) {
        names.add(info.name);
    }
    return names;
}

juce::StringArray MidiManager::getAvailableOutputNames() const {
    juce::StringArray names;
    for (const auto& info : juce::MidiOutput::getAvailableDevices()) {
        names.add(info.name);
    }
    return names;
}

bool MidiManager::openInputDevice(const juce::String& deviceIdentifierOrName) {
    closeInputDevice();
    auto devices = juce::MidiInput::getAvailableDevices();
    for (const auto& dev : devices) {
        if (dev.identifier == deviceIdentifierOrName || dev.name == deviceIdentifierOrName) {
            auto opened = juce::MidiInput::openDevice(dev.identifier, this);
            if (opened) {
                std::lock_guard<std::mutex> lock(portMutex_);
                midiInput_ = std::move(opened);
                midiInput_->start();
                currentInputName_ = dev.name;
                return true;
            }
        }
    }
    return false;
}

bool MidiManager::openOutputDevice(const juce::String& deviceIdentifierOrName) {
    closeOutputDevice();
    auto devices = juce::MidiOutput::getAvailableDevices();
    for (const auto& dev : devices) {
        if (dev.identifier == deviceIdentifierOrName || dev.name == deviceIdentifierOrName) {
            auto opened = juce::MidiOutput::openDevice(dev.identifier);
            if (opened) {
                std::lock_guard<std::mutex> lock(portMutex_);
                midiOutput_ = std::move(opened);
                currentOutputName_ = dev.name;
                return true;
            }
        }
    }
    return false;
}

bool MidiManager::openThruInputDevice(const juce::String& deviceIdentifierOrName) {
    closeThruInputDevice();
    auto devices = juce::MidiInput::getAvailableDevices();
    for (const auto& dev : devices) {
        if (dev.identifier == deviceIdentifierOrName || dev.name == deviceIdentifierOrName) {
            auto opened = juce::MidiInput::openDevice(dev.identifier, this);
            if (opened) {
                std::lock_guard<std::mutex> lock(portMutex_);
                thruInput_ = std::move(opened);
                thruInput_->start();
                currentThruInputName_ = dev.name;
                return true;
            }
        }
    }
    return false;
}

void MidiManager::closeInputDevice() {
    std::lock_guard<std::mutex> lock(portMutex_);
    if (midiInput_) {
        midiInput_->stop();
        midiInput_.reset();
    }
    currentInputName_.clear();
}

void MidiManager::closeOutputDevice() {
    std::lock_guard<std::mutex> lock(portMutex_);
    midiOutput_.reset();
    currentOutputName_.clear();
}

void MidiManager::closeThruInputDevice() {
    std::lock_guard<std::mutex> lock(portMutex_);
    if (thruInput_) {
        thruInput_->stop();
        thruInput_.reset();
    }
    currentThruInputName_.clear();
}

juce::String MidiManager::getCurrentInputName() const {
    std::lock_guard<std::mutex> lock(portMutex_);
    return currentInputName_;
}

juce::String MidiManager::getCurrentOutputName() const {
    std::lock_guard<std::mutex> lock(portMutex_);
    return currentOutputName_;
}

juce::String MidiManager::getCurrentThruInputName() const {
    std::lock_guard<std::mutex> lock(portMutex_);
    return currentThruInputName_;
}

bool MidiManager::isInputOpen() const {
    std::lock_guard<std::mutex> lock(portMutex_);
    return midiInput_ != nullptr;
}

bool MidiManager::isOutputOpen() const {
    std::lock_guard<std::mutex> lock(portMutex_);
    return midiOutput_ != nullptr;
}

void MidiManager::sendSysEx(const std::vector<uint8_t>& fullSysExWithF0F7) {
    if (fullSysExWithF0F7.size() < 4) return;
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        sysexQueue_.push_back(fullSysExWithF0F7);
    }
    notify();
}

void MidiManager::sendSysExQueue(const std::vector<std::vector<uint8_t>>& packets) {
    if (packets.empty()) return;
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        for (const auto& pkt : packets) {
            if (pkt.size() >= 4) {
                sysexQueue_.push_back(pkt);
            }
        }
    }
    notify();
}

void MidiManager::clearSysExQueue() {
    std::lock_guard<std::mutex> lock(queueMutex_);
    sysexQueue_.clear();
}

size_t MidiManager::getPendingSysExCount() {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return sysexQueue_.size();
}

void MidiManager::sendChannelMessage(const juce::MidiMessage& msg) {
    dispatchNow(msg);
}

void MidiManager::sendBankAndProgramChange(int channelZeroBased, uint8_t msb, uint8_t lsb, uint8_t program) {
    int ch = juce::jlimit(1, 16, channelZeroBased + 1);
    dispatchNow(juce::MidiMessage::controllerEvent(ch, 0, msb & 0x7F));
    dispatchNow(juce::MidiMessage::controllerEvent(ch, 32, lsb & 0x7F));
    dispatchNow(juce::MidiMessage::programChange(ch, program & 0x7F));
}

void MidiManager::sendNoteOn(int channelZeroBased, int noteNumber, uint8_t velocity) {
    int ch = juce::jlimit(1, 16, channelZeroBased + 1);
    dispatchNow(juce::MidiMessage::noteOn(ch, juce::jlimit(0, 127, noteNumber), static_cast<uint8_t>(velocity & 0x7F)));
}

void MidiManager::sendNoteOff(int channelZeroBased, int noteNumber) {
    int ch = juce::jlimit(1, 16, channelZeroBased + 1);
    dispatchNow(juce::MidiMessage::noteOff(ch, juce::jlimit(0, 127, noteNumber)));
}

void MidiManager::sendControlChange(int channelZeroBased, int ccNumber, uint8_t value) {
    int ch = juce::jlimit(1, 16, channelZeroBased + 1);
    dispatchNow(juce::MidiMessage::controllerEvent(ch, juce::jlimit(0, 127, ccNumber), value & 0x7F));
}

void MidiManager::sendAllNotesOff(int channelZeroBased) {
    int ch = juce::jlimit(1, 16, channelZeroBased + 1);
    dispatchNow(juce::MidiMessage::allNotesOff(ch));
}

void MidiManager::dispatchNow(const juce::MidiMessage& msg) {
    auto mode = routingMode_.load();
    if (mode == MidiRoutingMode::DirectHardwareOnly || mode == MidiRoutingMode::BothHardwareAndDaw) {
        std::lock_guard<std::mutex> lock(portMutex_);
        if (midiOutput_) {
            midiOutput_->sendMessageNow(msg);
        }
    }
    if (mode == MidiRoutingMode::DawBusOnly || mode == MidiRoutingMode::BothHardwareAndDaw) {
        std::lock_guard<std::mutex> lock(dawBufferMutex_);
        dawOutgoingQueue_.push_back(msg);
    }

    std::lock_guard<std::mutex> cbLock(callbackMutex_);
    if (onMidiActivity_) onMidiActivity_(true);
}

void MidiManager::popDawOutgoingMessages(juce::MidiBuffer& destBuffer) {
    std::lock_guard<std::mutex> lock(dawBufferMutex_);
    for (const auto& msg : dawOutgoingQueue_) {
        destBuffer.addEvent(msg, 0);
    }
    dawOutgoingQueue_.clear();
}

void MidiManager::processIncomingDawMidi(const juce::MidiMessage& msg) {
    if (msg.isSysEx()) {
        std::lock_guard<std::mutex> cbLock(callbackMutex_);
        if (onSysExReceived_) {
            onSysExReceived_(msg.getSysExData(), msg.getSysExDataSize());
        }
    } else if (midiThruEnabled_.load()) {
        // Forward DAW MIDI channel messages directly to hardware port if DirectHardwareOnly / Both
        auto mode = routingMode_.load();
        if (mode == MidiRoutingMode::DirectHardwareOnly || mode == MidiRoutingMode::BothHardwareAndDaw) {
            std::lock_guard<std::mutex> lock(portMutex_);
            if (midiOutput_) {
                midiOutput_->sendMessageNow(msg);
            }
        }
    }
}

void MidiManager::setSysExReceivedCallback(SysExReceivedCallback cb) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    onSysExReceived_ = std::move(cb);
}

void MidiManager::setMidiActivityCallback(MidiActivityCallback cb) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    onMidiActivity_ = std::move(cb);
}

void MidiManager::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) {
    {
        std::lock_guard<std::mutex> cbLock(callbackMutex_);
        if (onMidiActivity_) onMidiActivity_(false);
    }

    if (source == thruInput_.get()) {
        if (midiThruEnabled_.load() && !message.isSysEx()) {
            dispatchNow(message);
        }
        return;
    }

    if (message.isSysEx()) {
        std::lock_guard<std::mutex> cbLock(callbackMutex_);
        if (onSysExReceived_) {
            onSysExReceived_(message.getSysExData(), message.getSysExDataSize());
        }
    } else if (midiThruEnabled_.load()) {
        dispatchNow(message);
    }
}

void MidiManager::run() {
    while (!threadShouldExit()) {
        std::vector<uint8_t> nextPacket;
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            if (!sysexQueue_.empty()) {
                nextPacket = std::move(sysexQueue_.front());
                sysexQueue_.pop_front();
            }
        }

        if (!nextPacket.empty()) {
            auto msg = juce::MidiMessage::createSysExMessage(nextPacket.data() + 1,
                                                             static_cast<int>(nextPacket.size() - 2));
            dispatchNow(msg);

            int waitMs = sysexIntervalMs_.load();
            // Pace RQ1 (0x11) requests according to expected DT1 reply size so the JV-1010 never drops replies
            if (nextPacket.size() >= 14 && nextPacket[4] == 0x11) {
                uint32_t reqSize = (static_cast<uint32_t>(nextPacket[9] & 0x7F) << 21)
                                 | (static_cast<uint32_t>(nextPacket[10] & 0x7F) << 14)
                                 | (static_cast<uint32_t>(nextPacket[11] & 0x7F) << 7)
                                 | static_cast<uint32_t>(nextPacket[12] & 0x7F);
                if (reqSize <= 0x10) {
                    waitMs = std::max(waitMs, 35);
                } else if (reqSize <= 0x50) {
                    waitMs = std::max(waitMs, 50);
                } else {
                    waitMs = std::max(waitMs, 70);
                }
            } else if (nextPacket.size() > 64) {
                waitMs = std::max(waitMs, 35);
            }
            wait(waitMs);
        } else {
            wait(50);
        }
    }
}

} // namespace jv1010
