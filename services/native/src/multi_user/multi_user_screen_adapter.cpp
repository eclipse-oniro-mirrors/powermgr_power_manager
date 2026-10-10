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

#include <display_mgr_errors.h>
#include <display_power_mgr_client.h>
#include <errors.h>
#include <os_account_manager.h>

#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
namespace {
using DisplayErrors = OHOS::DisplayPowerMgr::DisplayErrors;
using MultiScreenStateChangeReason = OHOS::DisplayPowerMgr::MultiScreenStateChangeReason;
using OHOS::DisplayPowerMgr::DISPLAY_ID_ALL;

inline bool IsDisplayOnState(OHOS::DisplayPowerMgr::DisplayState state)
{
    return state == OHOS::DisplayPowerMgr::DisplayState::DISPLAY_ON ||
        state == OHOS::DisplayPowerMgr::DisplayState::DISPLAY_DIM;
}
} // namespace

sptr<MultiUserScreenAdapter::MultiScreenCallback> MultiUserScreenAdapter::multiScreenCallback_;
MultiUserScreenAdapter::ScreenChangeHandler MultiUserScreenAdapter::screenChangeHandler_;

int32_t MultiUserScreenAdapter::GetUserIdFromUid(int32_t uid)
{
    if (uid < 0) {
        return INVALID_USER_ID;
    }
    return uid / UID_TRANSFORM_DIVISOR;
}

int32_t MultiUserScreenAdapter::GetUserIdByDisplayId(uint64_t displayId)
{
    int32_t localId = INVALID_USER_ID;
    ErrCode ret = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(displayId, localId);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_SVC, "GetForegroundOsAccountLocalId failed, displayId=%{public}" PRIu64 " ret=%{public}d",
            displayId, ret);
        return INVALID_USER_ID;
    }
    return localId;
}

uint64_t MultiUserScreenAdapter::GetDisplayIdByUserId(int32_t userId)
{
    uint64_t displayId = INVALID_DISPLAY_ID;
    ErrCode ret = AccountSA::OsAccountManager::GetForegroundOsAccountDisplayId(userId, displayId);
    if (ret != ERR_OK) {
        POWER_HILOGW(COMP_SVC, "GetForegroundOsAccountDisplayId failed, userId=%{public}d ret=%{public}d", userId, ret);
        return INVALID_DISPLAY_ID;
    }
    return displayId;
}

bool MultiUserScreenAdapter::IsDisplayOn(uint64_t displayId)
{
    OHOS::DisplayPowerMgr::DisplayState state = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN;
    DisplayErrors ret = OHOS::DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().GetMultiScreenDisplayState(
        displayId, state);
    if (ret != DisplayErrors::ERR_OK) {
        POWER_HILOGW(COMP_SVC, "GetMultiScreenDisplayState failed, displayId=%{public}" PRIu64 " ret=%{public}d",
            displayId, static_cast<int32_t>(ret));
        return false;
    }
    return IsDisplayOnState(state);
}

bool MultiUserScreenAdapter::IsAnyScreenOn(uint64_t excludeDisplayId)
{
    auto& client = OHOS::DisplayPowerMgr::DisplayPowerMgrClient::GetInstance();
    std::vector<uint32_t> displayIds = client.GetDisplayIds();
    bool queried = false;
    for (uint32_t id : displayIds) {
        uint64_t displayId = static_cast<uint64_t>(id);
        if (displayId == excludeDisplayId) {
            continue;
        }
        queried = true;
        if (IsDisplayOn(displayId)) {
            return true;
        }
    }
    if (queried) {
        return false;
    }
    // No usable multi-screen info (single-screen device or display SA not ready): fall back to the
    // main display state so the whole-machine query stays correct on non-cockpit products.
    OHOS::DisplayPowerMgr::DisplayState mainState = client.GetDisplayState();
    return IsDisplayOnState(mainState);
}

bool MultiUserScreenAdapter::IsScreenOnForUser(int32_t userId)
{
    uint64_t displayId = GetDisplayIdByUserId(userId);
    if (displayId == INVALID_DISPLAY_ID) {
        POWER_HILOGI(COMP_SVC, "no display for userId=%{public}d, report screen off", userId);
        return false;
    }
    return IsDisplayOn(displayId);
}

void MultiUserScreenAdapter::SetScreenChangeHandler(ScreenChangeHandler handler)
{
    screenChangeHandler_ = std::move(handler);
}

bool MultiUserScreenAdapter::RegisterScreenStateCallback()
{
    if (multiScreenCallback_ == nullptr) {
        multiScreenCallback_ = new (std::nothrow) MultiScreenCallback();
    }
    if (multiScreenCallback_ == nullptr) {
        POWER_HILOGE(COMP_SVC, "create MultiScreenCallback failed");
        return false;
    }
    DisplayErrors ret =
        OHOS::DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().RegisterMultiScreenDisplayStateCallback(
            multiScreenCallback_, DISPLAY_ID_ALL);
    POWER_HILOGI(COMP_SVC, "RegisterMultiScreenDisplayStateCallback ret=%{public}d", static_cast<int32_t>(ret));
    return ret == DisplayErrors::ERR_OK;
}

bool MultiUserScreenAdapter::UnregisterScreenStateCallback()
{
    if (multiScreenCallback_ == nullptr) {
        return false;
    }
    DisplayErrors ret =
        OHOS::DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().UnregisterMultiScreenDisplayStateCallback(
            multiScreenCallback_, DISPLAY_ID_ALL);
    POWER_HILOGI(COMP_SVC, "UnregisterMultiScreenDisplayStateCallback ret=%{public}d", static_cast<int32_t>(ret));
    if (ret == DisplayErrors::ERR_OK) {
        multiScreenCallback_ = nullptr;
    }
    return ret == DisplayErrors::ERR_OK;
}

void MultiUserScreenAdapter::MultiScreenCallback::OnMultiScreenDisplayStateChanged(uint64_t displayId,
    const std::string& screenName, OHOS::DisplayPowerMgr::DisplayState state, MultiScreenStateChangeReason reason)
{
    bool isScreenOn = IsDisplayOnState(state);
    uint32_t reasonValue = static_cast<uint32_t>(reason);
    POWER_HILOGI(COMP_SVC,
        "OnMultiScreenDisplayStateChanged displayId=%{public}" PRIu64 " screen=%{public}s state=%{public}u "
        "on=%{public}d reason=%{public}u",
        displayId, screenName.c_str(), static_cast<uint32_t>(state), isScreenOn, reasonValue);
    if (screenChangeHandler_) {
        screenChangeHandler_(displayId, isScreenOn, reasonValue);
    }
}
} // namespace PowerMgr
} // namespace OHOS
#endif // POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION
