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

#include "multi_user_screen_adapter.h"

#ifdef POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION

#include <cinttypes>
#include <map>

#include <display_power_info.h>
#include <display_power_mgr_client.h>
#include <errors.h>
#include <os_account_manager.h>
#include <os_account_constants.h>
#include <ipc_skeleton.h>

#include "power_log.h"

namespace OHOS {
namespace PowerMgr {

bool MultiUserScreenAdapter::IsSystemAccount(int32_t userId)
{
    return userId >= AccountSA::Constants::START_USER_ID && userId <= AccountSA::Constants::MAX_USER_ID;
}

bool MultiUserScreenAdapter::IsValidUserId(int32_t userId)
{
    constexpr int32_t SYSTEM_USER_ID = 0;
    return userId == SYSTEM_USER_ID
        || (userId >= AccountSA::Constants::START_USER_ID && userId <= AccountSA::Constants::MAX_USER_ID);
}

int32_t MultiUserScreenAdapter::GetUserIdFromUid(int32_t uid)
{
    int32_t userId = 0;
    AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId);
    return userId;
}

uint64_t MultiUserScreenAdapter::GetDisplayIdByUserId(int32_t userId)
{
    uint64_t displayId = INVALID_DISPLAY_ID;
    ErrCode ret = AccountSA::OsAccountManager::GetForegroundOsAccountDisplayId(userId, displayId);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_SVC, "GetForegroundOsAccountDisplayId failed, userId=%{public}d ret=%{public}d",
            userId, ret);
        return INVALID_DISPLAY_ID;
    }
    return displayId;
}

int32_t MultiUserScreenAdapter::GetUserIdByDisplayId(uint64_t displayId)
{
    int32_t userId = INVALID_USER_ID;
    ErrCode err = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(
        static_cast<uint32_t>(displayId), userId);
    if (err != ERR_OK) {
        POWER_HILOGW(COMP_SVC,
            "GetForegroundOsAccountLocalId failed, displayId=%{public}" PRIu64 ", ret=%{public}d",
            displayId, err);
        return INVALID_USER_ID;
    }
    return userId;
}

bool MultiUserScreenAdapter::IsOnState(uint32_t state)
{
    return state == static_cast<uint32_t>(OHOS::DisplayPowerMgr::DisplayState::DISPLAY_ON) ||
        state == static_cast<uint32_t>(OHOS::DisplayPowerMgr::DisplayState::DISPLAY_DIM);
}

bool MultiUserScreenAdapter::IsScreenOnForUser(int32_t userId)
{
    uint64_t displayId = GetDisplayIdByUserId(userId);
    if (displayId == INVALID_DISPLAY_ID) {
        POWER_HILOGI(COMP_SVC, "no display for userId=%{public}d, report screen off", userId);
        return false;
    }
    // Single-screen query: one IPC for the user's bound display state.
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    OHOS::DisplayPowerMgr::DisplayState state = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN;
    auto ret = OHOS::DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().GetMultiScreenDisplayState(displayId, state);
    IPCSkeleton::SetCallingIdentity(identity);
    if (ret != OHOS::DisplayPowerMgr::DisplayErrors::ERR_OK) {
        POWER_HILOGW(COMP_SVC, "GetMultiScreenDisplayState failed displayId=%{public}" PRIu64 " ret=%{public}d",
            displayId, static_cast<int32_t>(ret));
        return false;
    }
    return IsOnState(static_cast<uint32_t>(state));
}

bool MultiUserScreenAdapter::IsUserMainScreen(uint64_t displayId)
{
    int32_t userId = GetUserIdByDisplayId(displayId);
    if (userId == INVALID_USER_ID) {
        return false;
    }
    return GetDisplayIdByUserId(userId) == displayId;
}


bool MultiUserScreenAdapter::IsAnyScreenOn()
{
    // Whole-machine query: any screen ON/DIM in display_manager's one-call vector of all screens.
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto states = OHOS::DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().GetAllMultiScreenDisplayState();
    IPCSkeleton::SetCallingIdentity(identity);
    for (const auto& [id, state] : states) {
        if (IsUserMainScreen(id) && IsOnState(static_cast<uint32_t>(state))) {
            return true;
        }
    }
    return false;
}

} // namespace PowerMgr
} // namespace OHOS
#endif // POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION
