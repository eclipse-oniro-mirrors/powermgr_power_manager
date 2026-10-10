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

#ifndef POWERMGR_MULTI_USER_TEST_MOCK_H
#define POWERMGR_MULTI_USER_TEST_MOCK_H

#include <cstdint>
#include <vector>

#ifdef POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION

#include "display_mgr_errors.h"
#include "display_power_info.h"
#include "errors.h"

namespace OHOS::PowerMgr::TestMock {
// display_manager multi-screen link-time mock (mock_display_power_mgr_client_multi_user.cpp)
void ResetDisplayMock();
OHOS::DisplayPowerMgr::DisplayState& MockDisplayStateValue();
OHOS::DisplayPowerMgr::DisplayErrors& MockDisplayGetStateRet();
OHOS::DisplayPowerMgr::DisplayErrors& MockDisplayRegisterRet();
std::vector<uint32_t>& MockDisplayIds();
OHOS::DisplayPowerMgr::DisplayState& MockMainDisplayState();
int& MockGetStateCallCount();
int& MockRegisterCallCount();
int& MockUnregisterCallCount();

// OsAccountManager link-time mock (mock_os_account_manager_multi_user.cpp)
void ResetOsAccountMock();
ErrCode& MockLocalIdRet();
int32_t& MockLocalId();
ErrCode& MockDisplayIdRet();
uint64_t& MockDisplayId();
int& MockLocalIdCallCount();
int& MockDisplayIdCallCount();

// CommonEventManager link-time mock (mock_common_event_manager_as_user.cpp)
void ResetCommonEventMock();
bool& MockPublishAsUserRet();
int& MockPublishAsUserCallCount();
int32_t& MockPublishAsUserLastUserId();
int& MockPublishAsUserOnCount();
int& MockPublishAsUserOffCount();
int& MockWholeMachineCallCount();
int& MockWholeMachineOnCount();
int& MockWholeMachineOffCount();
} // namespace OHOS::PowerMgr::TestMock
#endif // POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION
#endif // POWERMGR_MULTI_USER_TEST_MOCK_H
