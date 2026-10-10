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
#include <vector>

#ifdef HAS_DISPLAY_MANAGER_PART
#include "display_mgr_errors.h"
#include "display_power_info.h"
#include "display_power_mgr_client.h"
#endif

#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
namespace {
// Link-time mock state, set by the multi-user test to drive the real MultiUserScreenAdapter entry
// functions with controlled display_manager return values (instead of live display SA IPC).
struct MockDisplayState {
    OHOS::DisplayPowerMgr::DisplayErrors getStateRet {OHOS::DisplayPowerMgr::DisplayErrors::ERR_OK};
    OHOS::DisplayPowerMgr::DisplayState state {OHOS::DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN};
    OHOS::DisplayPowerMgr::DisplayErrors registerRet {OHOS::DisplayPowerMgr::DisplayErrors::ERR_OK};
    OHOS::DisplayPowerMgr::DisplayErrors unregisterRet {OHOS::DisplayPowerMgr::DisplayErrors::ERR_OK};
    std::vector<uint32_t> displayIds {};
    OHOS::DisplayPowerMgr::DisplayState mainDisplayState {OHOS::DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN};
    int getStateCallCount {0};
    int registerCallCount {0};
    int unregisterCallCount {0};
    int getDisplayIdsCallCount {0};
    int getDisplayStateCallCount {0};
    uint64_t lastGetStateDisplayId {0};
};
MockDisplayState g_mockDisplay;
} // namespace

// Accessors used by the test to reset/inspect mock state across cases.
namespace OHOS::PowerMgr::TestMock {
void ResetDisplayMock()
{
    g_mockDisplay = MockDisplayState {};
}
OHOS::DisplayPowerMgr::DisplayState& MockDisplayStateValue()
{
    return g_mockDisplay.state;
}
OHOS::DisplayPowerMgr::DisplayErrors& MockDisplayGetStateRet()
{
    return g_mockDisplay.getStateRet;
}
OHOS::DisplayPowerMgr::DisplayErrors& MockDisplayRegisterRet()
{
    return g_mockDisplay.registerRet;
}
std::vector<uint32_t>& MockDisplayIds()
{
    return g_mockDisplay.displayIds;
}
OHOS::DisplayPowerMgr::DisplayState& MockMainDisplayState()
{
    return g_mockDisplay.mainDisplayState;
}
int& MockGetStateCallCount()
{
    return g_mockDisplay.getStateCallCount;
}
int& MockRegisterCallCount()
{
    return g_mockDisplay.registerCallCount;
}
int& MockUnregisterCallCount()
{
    return g_mockDisplay.unregisterCallCount;
}
} // namespace OHOS::PowerMgr::TestMock

namespace OHOS::DisplayPowerMgr {
DisplayErrors DisplayPowerMgrClient::GetMultiScreenDisplayState(uint64_t displayId, DisplayState& state)
{
    g_mockDisplay.getStateCallCount++;
    g_mockDisplay.lastGetStateDisplayId = displayId;
    state = g_mockDisplay.state;
    return g_mockDisplay.getStateRet;
}

DisplayErrors DisplayPowerMgrClient::RegisterMultiScreenDisplayStateCallback(
    sptr<IMultiScreenDisplayStateCallback> callback, uint64_t displayId)
{
    g_mockDisplay.registerCallCount++;
    (void)callback;
    (void)displayId;
    return g_mockDisplay.registerRet;
}

DisplayErrors DisplayPowerMgrClient::UnregisterMultiScreenDisplayStateCallback(
    sptr<IMultiScreenDisplayStateCallback> callback, uint64_t displayId)
{
    g_mockDisplay.unregisterCallCount++;
    (void)callback;
    (void)displayId;
    return g_mockDisplay.unregisterRet;
}

std::vector<uint32_t> DisplayPowerMgrClient::GetDisplayIds()
{
    g_mockDisplay.getDisplayIdsCallCount++;
    return g_mockDisplay.displayIds;
}

DisplayState DisplayPowerMgrClient::GetDisplayState(uint32_t id)
{
    g_mockDisplay.getDisplayStateCallCount++;
    (void)id;
    return g_mockDisplay.mainDisplayState;
}
} // namespace OHOS::DisplayPowerMgr
#endif // DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
