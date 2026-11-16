#include "ExpansionDetector.h"
#include "../Model/ParameterMaps.h"

namespace jv1010 {

ExpansionDetector::ExpansionDetector(MidiManager& midi, JV1010State& state)
    : midi_(midi), synthState_(state) {}

void ExpansionDetector::startAutoDetect(uint8_t deviceId,
                                        std::function<void(const SrJv80BoardInfo*, const std::string&)> onComplete) {
    activeDevId_ = deviceId;
    callback_ = std::move(onComplete);
    state_ = ProbeState::WaitingIdentity;

    // Save current patch selection from state
    prevGroupType_ = synthState_.getByte(addr::kSystemBase | addr::sys::PatchGroupType);
    prevGroupId_   = synthState_.getByte(addr::kSystemBase | addr::sys::PatchGroupID);
    prevPatchNum_  = synthState_.getNibbled2(addr::kSystemBase | addr::sys::PatchNumberNibbled);

    // Step 1: Identity Request + switch to WAVE-EXP 001 + RQ1 System Common & Temp Patch Common
    midi_.sendSysEx(sysex::buildIdentityRequest(activeDevId_));

    int patchCh = synthState_.getByte(addr::kSystemBase | addr::sys::PatchReceiveChannel);
    midi_.sendBankAndProgramChange(patchCh, 84, 2, 0);

    std::vector<std::vector<uint8_t>> reqs;
    reqs.push_back(sysex::buildRQ1(activeDevId_, addr::kSystemBase, addr::kSystemCommonSize7Bit));
    reqs.push_back(sysex::buildRQ1(activeDevId_, addr::kPatchModeTempPatchBase, addr::kPatchCommonSize7Bit));
    midi_.sendSysExQueue(reqs);

    state_ = ProbeState::WaitingSystemAndPatch;
}

void ExpansionDetector::handleSysEx(const sysex::ParsedSysEx& msg) {
    if (state_ == ProbeState::Idle) return;

    if (msg.type == sysex::MessageType::IdentityReply) {
        // Confirmed JV-1010 (familyCode == 0x6A00, familyNumberCode == 0x0500)
        return;
    }

    if (msg.type == sysex::MessageType::JV_DT1 && msg.checksumValid) {
        if (msg.address == addr::kPatchModeTempPatchBase && msg.payload.size() >= 12) {
            uint8_t groupType = synthState_.getByte(addr::kSystemBase | addr::sys::PatchGroupType);
            uint8_t groupId   = synthState_.getByte(addr::kSystemBase | addr::sys::PatchGroupID);
            std::string patchName = sysex::decodeAsciiName12(msg.payload.data(), 12);

            const SrJv80BoardInfo* board = nullptr;
            if (groupType == 2 && groupId > 0) {
                board = findSrJv80ByGroupId(static_cast<int>(groupId));
            }
            if (!board) {
                board = findSrJv80ByFirstPatchName(patchName);
            }

            if (board) {
                installedGroupId_ = board->groupId;
                state_ = ProbeState::Idle;
                if (callback_) {
                    callback_(board, std::string("Detectada: ") + board->model + " " + board->title);
                }
            } else {
                installedGroupId_ = 0;
                state_ = ProbeState::Idle;
                if (callback_) {
                    callback_(nullptr, "Slot EXP-B vacio o tarjeta no reconocida.");
                }
            }
        }
    }
}

void ExpansionDetector::setInstalledBoard(int groupId) {
    installedGroupId_ = groupId;
}

const SrJv80BoardInfo* ExpansionDetector::getInstalledBoardInfo() const {
    int id = installedGroupId_.load();
    if (id <= 0) return nullptr;
    return findSrJv80ByGroupId(id);
}

} // namespace jv1010
