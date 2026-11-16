#include "SetLibrarianView.h"
#include "../../Database/FactoryPresetsDB.h"
#include "../../Database/SrJv80CatalogDB.h"
#include <imgui.h>
#include <algorithm>
#include <cstdio>
#include <vector>

namespace jv1010 {

SetLibrarianView::SetLibrarianView(JV1010EditorAudioProcessor& processor)
    : processor_(processor) {}

void SetLibrarianView::render(float width, float height, int activeKbdChannelZeroBased,
                              const std::function<void(const std::string&)>& setStatus,
                              const std::function<void(int tabIndex)>& switchEditorTab) {
    ImGui::BeginChild("##BrowserPane", ImVec2(width, height), true);

    auto activateModeForAsset = [&](int type) {
        assetType_ = type;
        uint8_t targetMode = (type == 0) ? 1 : 0; // 1 = PATCH, 0 = PERFORM (for Perfs & Drums)
        if (processor_.getSynthState().getByte(addr::kSystemBase | addr::sys::SoundMode) != targetMode) {
            processor_.sendParameterDT1(addr::kSystemBase | addr::sys::SoundMode, targetMode);
        }
        if (switchEditorTab) {
            switchEditorTab((type == 0) ? 0 : (type == 1 ? 3 : 2));
        }
    };

    if (ImGui::RadioButton("Patches", assetType_ == 0)) activateModeForAsset(0);
    ImGui::SameLine();
    if (ImGui::RadioButton("Perfs", assetType_ == 1)) activateModeForAsset(1);
    ImGui::SameLine();
    if (ImGui::RadioButton("Drums", assetType_ == 2)) activateModeForAsset(2);

    ImGui::SetNextItemWidth(-1.0f);
    const char* bankOpts[] = { "All Banks", "USER", "PR-A", "PR-B", "PR-C", "PR-D (GM)", "PR-E", "XP-A (Session)", "XP-B (EXP)" };
    ImGui::Combo("##BankFilter", &bankFilter_, bankOpts, 9);

    if (assetType_ == 0) {
        ImGui::SetNextItemWidth(-1.0f);
        const auto& cats = getPatchCategories();
        std::string catPreview = (catFilter_ == 0) ? "All Categories"
                                                   : (std::string(cats[static_cast<size_t>(catFilter_ - 1)].shortCode) + " " + cats[static_cast<size_t>(catFilter_ - 1)].name);
        if (ImGui::BeginCombo("##CatFilter", catPreview.c_str())) {
            if (ImGui::Selectable("All Categories", catFilter_ == 0)) catFilter_ = 0;
            for (const auto& c : cats) {
                std::string lbl = std::string(c.shortCode) + " - " + c.name;
                if (ImGui::Selectable(lbl.c_str(), catFilter_ == c.id + 1)) catFilter_ = c.id + 1;
            }
            ImGui::EndCombo();
        }
    }

    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputTextWithHint("##Search", "Search patch / kit / perf...", searchBuf_, sizeof(searchBuf_));

    struct RowItem { int bankIdx; int itemIdx; std::string label; std::string name; std::string meta; };
    std::vector<RowItem> rows;
    std::string q(searchBuf_);
    std::transform(q.begin(), q.end(), q.begin(), [](unsigned char c) { return static_cast<char>(::tolower(c)); });

    auto bankTag = [](int b) -> const char* {
        static const char* tags[] = { "USER", "PR-A", "PR-B", "PR-C", "PR-D", "PR-E", "XP-A", "XP-B" };
        return tags[std::clamp(b, 0, 7)];
    };

    if (assetType_ == 0) {
        for (const auto& p : getFactoryPatches()) {
            int bi = static_cast<int>(p.bank);
            if (bankFilter_ > 0 && (bi + 1) != bankFilter_) continue;
            if (catFilter_ > 0 && p.categoryIndex != (catFilter_ - 1)) continue;
            std::string nm = p.name;
            if (p.bank == PatchBank::User) {
                auto ln = processor_.getSynthState().getUserPatch(p.indexZeroBased).getName();
                if (!ln.empty() && ln.rfind("UserPat ", 0) != 0) nm = ln;
            }
            if (!q.empty()) {
                std::string lnm = nm;
                std::transform(lnm.begin(), lnm.end(), lnm.begin(), [](unsigned char c) { return static_cast<char>(::tolower(c)); });
                if (lnm.find(q) == std::string::npos) continue;
            }
            const auto& cat = getPatchCategories()[static_cast<size_t>(std::clamp(p.categoryIndex, 0, 38))];
            char tagBuf[24];
            std::snprintf(tagBuf, sizeof(tagBuf), "%s:%03d", bankTag(bi), p.indexZeroBased + 1);
            rows.push_back({ bi, p.indexZeroBased, tagBuf, nm, cat.shortCode });
        }
        if (const auto* exp = processor_.getExpansionDetector().getInstalledBoardInfo()) {
            if (bankFilter_ == 0 || bankFilter_ == 8) {
                for (int i = 0; i < exp->patchCount; ++i) {
                    std::string nm = std::string(exp->model) + " #" + std::to_string(i + 1);
                    if (!q.empty()) {
                        std::string lnm = nm;
                        std::transform(lnm.begin(), lnm.end(), lnm.begin(), [](unsigned char c) { return static_cast<char>(::tolower(c)); });
                        if (lnm.find(q) == std::string::npos) continue;
                    }
                    char tagBuf[24];
                    std::snprintf(tagBuf, sizeof(tagBuf), "XP-B:%03d", i + 1);
                    rows.push_back({ static_cast<int>(PatchBank::WaveExp), i, tagBuf, nm, exp->model });
                }
            }
        }
    } else if (assetType_ == 1) {
        for (const auto& pf : getFactoryPerformances()) {
            int bi = static_cast<int>(pf.bank);
            if (bankFilter_ > 0 && (bi + 1) != bankFilter_) continue;
            std::string nm = pf.name;
            if (pf.bank == PerfBank::User) {
                auto ln = processor_.getSynthState().getUserPerformance(pf.indexZeroBased).getName();
                if (!ln.empty() && ln.rfind("UserPerf ", 0) != 0) nm = ln;
            }
            if (!q.empty()) {
                std::string lnm = nm;
                std::transform(lnm.begin(), lnm.end(), lnm.begin(), [](unsigned char c) { return static_cast<char>(::tolower(c)); });
                if (lnm.find(q) == std::string::npos) continue;
            }
            char tagBuf[24];
            std::snprintf(tagBuf, sizeof(tagBuf), "%s:%02d", bankTag(bi), pf.indexZeroBased + 1);
            rows.push_back({ bi, pf.indexZeroBased, tagBuf, nm, pf.isLayer ? "LAYER" : "SINGLE" });
        }
    } else {
        for (const auto& rk : getFactoryRhythmSets()) {
            int bi = static_cast<int>(rk.bank);
            if (bankFilter_ > 0 && (bi + 1) != bankFilter_) continue;
            std::string nm = rk.name;
            if (rk.bank == RhythmBank::User) {
                auto ln = processor_.getSynthState().getUserRhythm(rk.indexZeroBased).getName();
                if (!ln.empty() && ln.rfind("UserDrm ", 0) != 0) nm = ln;
            }
            if (!q.empty()) {
                std::string lnm = nm;
                std::transform(lnm.begin(), lnm.end(), lnm.begin(), [](unsigned char c) { return static_cast<char>(::tolower(c)); });
                if (lnm.find(q) == std::string::npos) continue;
            }
            char tagBuf[24];
            std::snprintf(tagBuf, sizeof(tagBuf), "%s:%02d", bankTag(bi), rk.indexZeroBased + 1);
            rows.push_back({ bi, rk.indexZeroBased, tagBuf, nm, "KIT" });
        }
        if (const auto* exp = processor_.getExpansionDetector().getInstalledBoardInfo()) {
            if ((bankFilter_ == 0 || bankFilter_ == 8) && exp->rhythmCount > 0) {
                for (int i = 0; i < exp->rhythmCount; ++i) {
                    std::string nm = std::string(exp->model) + " Kit #" + std::to_string(i + 1);
                    if (!q.empty()) {
                        std::string lnm = nm;
                        std::transform(lnm.begin(), lnm.end(), lnm.begin(), [](unsigned char c) { return static_cast<char>(::tolower(c)); });
                        if (lnm.find(q) == std::string::npos) continue;
                    }
                    char tagBuf[24];
                    std::snprintf(tagBuf, sizeof(tagBuf), "XP-B:%02d", i + 1);
                    rows.push_back({ static_cast<int>(RhythmBank::WaveExp), i, tagBuf, nm, exp->model });
                }
            }
        }
    }

    auto sendPCForRow = [&](const RowItem& it) {
        if (assetType_ == 0) {
            processor_.selectPatch(static_cast<PatchBank>(it.bankIdx), it.itemIdx, activeKbdChannelZeroBased);
            if (setStatus) setStatus("Patch -> " + it.label + " " + it.name + " (Ch " + std::to_string(activeKbdChannelZeroBased + 1) + ")");
        } else if (assetType_ == 1) {
            processor_.selectPerformance(static_cast<PerfBank>(it.bankIdx), it.itemIdx);
            if (setStatus) setStatus("Perf -> " + it.label + " " + it.name);
        } else {
            processor_.selectRhythmSet(static_cast<RhythmBank>(it.bankIdx), it.itemIdx, 9);
            if (setStatus) setStatus("Rhythm -> " + it.label + " " + it.name + " (Ch 10)");
        }
    };

    bool isUserBank = (bankFilter_ == 1);
    float footerExtraH = isUserBank ? 88.0f : 60.0f;
    float listH = std::max(80.0f, ImGui::GetContentRegionAvail().y - footerExtraH);
    ImGui::BeginChild("##BrowserList", ImVec2(0.0f, listH), true);
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(rows.size()));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
            const auto& it = rows[static_cast<size_t>(i)];
            char rowText[128];
            std::snprintf(rowText, sizeof(rowText), "%s  %-14s [%s]", it.label.c_str(), it.name.c_str(), it.meta.c_str());
            if (ImGui::Selectable(rowText, selectedIdx_ == i, ImGuiSelectableFlags_AllowDoubleClick)) {
                selectedIdx_ = i;
                sendPCForRow(it);
                if (switchEditorTab) {
                    switchEditorTab((assetType_ == 0) ? 0 : (assetType_ == 1 ? 3 : 2));
                }
            }
        }
    }
    ImGui::EndChild();

    if (ImGui::Button("Send PC") && selectedIdx_ >= 0 && selectedIdx_ < static_cast<int>(rows.size())) {
        sendPCForRow(rows[static_cast<size_t>(selectedIdx_)]);
    }
    ImGui::SameLine();
    if (ImGui::Button("Send SysEx") && selectedIdx_ >= 0 && selectedIdx_ < static_cast<int>(rows.size())) {
        const auto& it = rows[static_cast<size_t>(selectedIdx_)];
        auto& st = processor_.getSynthState();
        if (assetType_ == 0 && it.bankIdx == 0) {
            auto p = st.getUserPatch(it.itemIdx);
            st.setPatchModeTempPatch(p);
            processor_.getMidiManager().sendSysExQueue(st.serializePatchToDT1(processor_.getDeviceId(), addr::kPatchModeTempPatchBase, p));
        } else if (assetType_ == 1 && it.bankIdx == 0) {
            auto pf = st.getUserPerformance(it.itemIdx);
            st.setTempPerformance(pf);
            processor_.getMidiManager().sendSysExQueue(st.serializePerformanceToDT1(processor_.getDeviceId(), addr::kTempPerformanceBase, pf));
        } else {
            sendPCForRow(it);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Write User")) {
        int slot = 0;
        if (selectedIdx_ >= 0 && selectedIdx_ < static_cast<int>(rows.size()) && rows[static_cast<size_t>(selectedIdx_)].bankIdx == 0) {
            slot = rows[static_cast<size_t>(selectedIdx_)].itemIdx;
        }
        auto& st = processor_.getSynthState();
        if (assetType_ == 0) {
            auto p = st.getPatchModeTempPatch();
            st.setUserPatch(slot, p);
            processor_.getMidiManager().sendSysExQueue(st.serializePatchToDT1(processor_.getDeviceId(), addr::getUserPatchBase(slot), p));
        } else if (assetType_ == 1) {
            auto pf = st.getTempPerformance();
            st.setUserPerformance(slot, pf);
            processor_.getMidiManager().sendSysExQueue(st.serializePerformanceToDT1(processor_.getDeviceId(), addr::getUserPerfBase(slot), pf));
        } else {
            auto rk = st.getTempRhythm();
            st.setUserRhythm(slot & 0x01, rk);
            processor_.getMidiManager().sendSysExQueue(st.serializeRhythmToDT1(processor_.getDeviceId(), addr::getUserRhythmBase(slot & 0x01), rk));
        }
        if (setStatus) setStatus("Guardado en USER slot " + std::to_string(slot + 1));
    }

    if (isUserBank) {
        if (ImGui::Button("Sync RQ1")) {
            processor_.requestTemporaryStateFromHardware();
            if (setStatus) setStatus("RQ1 Temp enviado al JV-1010...");
        }
        ImGui::SameLine();
        if (ImGui::Button("Pull User Bank")) {
            ImGui::OpenPopup("##PullUserPopup");
        }
        if (ImGui::BeginPopup("##PullUserPopup")) {
            if (ImGui::MenuItem("Pull User Names (Fast - 128 Patches, 32 Perfs, 2 Kits)")) {
                processor_.requestUserBankNamesFromHardware();
                if (setStatus) setStatus("Descargando nombres del banco USER...");
            }
            if (ImGui::MenuItem("Pull Full User Bank (All Parameters)")) {
                processor_.requestFullUserBankFromHardware();
                if (setStatus) setStatus("Descargando memoria USER completa...");
            }
            ImGui::EndPopup();
        }
    }

    // EXP-B Expansion Board selector (persisted across sessions)
    auto& det = processor_.getExpansionDetector();
    const auto* curBoard = det.getInstalledBoardInfo();
    std::string boardPreview = curBoard ? (std::string(curBoard->model) + " " + curBoard->title) : "EXP-B: None";
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::BeginCombo("##ExpBoard", boardPreview.c_str())) {
        if (ImGui::Selectable("EXP-B: None", curBoard == nullptr)) det.setInstalledBoard(0);
        for (const auto& b : getSrJv80Catalog()) {
            if (b.groupId == 9) continue;
            std::string lbl = std::string(b.model) + " " + b.title;
            if (ImGui::Selectable(lbl.c_str(), curBoard && curBoard->groupId == b.groupId)) {
                det.setInstalledBoard(b.groupId);
            }
        }
        ImGui::EndCombo();
    }

    ImGui::EndChild();
}

} // namespace jv1010
