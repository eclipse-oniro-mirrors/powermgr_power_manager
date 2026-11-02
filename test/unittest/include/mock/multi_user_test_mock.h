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
#include <map>

#ifdef POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION

#include "display_power_info.h"
#include "errors.h"

namespace OHOS::PowerMgr::TestMock {
// display_manager multi-screen link-time mock (mock_display_power_mgr_client_multi_user.cpp).
// The slimmed adapter consumes only the one-call map<displayId, state> query, so the mock exposes
// the map directly for the test to populate.
void ResetDisplayMock();
std::map<uint64_t, OHOS::DisplayPowerMgr::DisplayState>& MockMultiScreenStates();

// OsAccountManager link-time mock (mock_os_account_manager_multi_user.cpp).
// The adapter calls GetForegroundOsAccountDisplayId (userId -> displayId) on the IsScreenOnForUser
// path; GetForegroundOsAccountLocalId is also mocked for completeness.
void ResetOsAccountMock();
ErrCode& MockLocalIdRet();
int32_t& MockLocalId();
ErrCode& MockDisplayIdRet();
uint64_t& MockDisplayId();
int& MockLocalIdCallCount();
int& MockDisplayIdCallCount();
} // namespace OHOS::PowerMgr::TestMock
#endif // POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION
#endif // POWERMGR_MULTI_USER_TEST_MOCK_H
