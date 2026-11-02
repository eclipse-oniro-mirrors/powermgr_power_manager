/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdint>
#include <map>
#include <utility>
#include <vector>

#ifdef HAS_DISPLAY_MANAGER_PART
#include "display_power_info.h"
#include "display_power_mgr_client.h"
#endif

#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
namespace {
// Link-time mock state for the slimmed MultiUserScreenAdapter, which consumes two display queries:
// the 2-arg single-screen GetMultiScreenDisplayState(displayId, state&) (IsScreenOnForUser) and the
// 0-arg whole-machine GetAllMultiScreenDisplayState() (IsAnyScreenOn). The test populates
// MockMultiScreenStates() (a map<displayId, state>) to drive both without a live display SA.
struct MockDisplayState {
    std::map<uint64_t, OHOS::DisplayPowerMgr::DisplayState> states {};
    int getMultiScreenCallCount {0};
};
MockDisplayState g_mockDisplay;
} // namespace

namespace OHOS::PowerMgr::TestMock {
void ResetDisplayMock()
{
    g_mockDisplay = MockDisplayState {};
}
std::map<uint64_t, OHOS::DisplayPowerMgr::DisplayState>& MockMultiScreenStates()
{
    return g_mockDisplay.states;
}
} // namespace OHOS::PowerMgr::TestMock

namespace OHOS::DisplayPowerMgr {
// Single-screen query (IsScreenOnForUser path): look up the bound displayId in the mock map.
// Found -> state + ERR_OK; absent -> DISPLAY_UNKNOWN + a non-OK error (adapter reports off).
DisplayErrors DisplayPowerMgrClient::GetMultiScreenDisplayState(uint64_t displayId, DisplayState& state)
{
    g_mockDisplay.getMultiScreenCallCount++;
    auto it = g_mockDisplay.states.find(displayId);
    if (it == g_mockDisplay.states.end()) {
        state = DisplayState::DISPLAY_UNKNOWN;
        return DisplayErrors::ERR_CONNECTION_FAIL;
    }
    state = it->second;
    return DisplayErrors::ERR_OK;
}

// Whole-machine query (IsAnyScreenOn path): return the mock states as a vector of
// {displayId, state} pairs (the new display_manager interface contract). IsAnyScreenOn iterates
// it for any ON/DIM screen.
std::vector<std::pair<uint64_t, DisplayState>> DisplayPowerMgrClient::GetAllMultiScreenDisplayState()
{
    g_mockDisplay.getMultiScreenCallCount++;
    std::vector<std::pair<uint64_t, DisplayState>> result;
    for (const auto& [id, st] : g_mockDisplay.states) {
        result.emplace_back(id, st);
    }
    return result;
}
} // namespace OHOS::DisplayPowerMgr
#endif // DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
