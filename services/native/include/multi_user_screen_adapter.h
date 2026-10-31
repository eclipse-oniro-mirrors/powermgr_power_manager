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

#ifndef POWERMGR_MULTI_USER_SCREEN_ADAPTER_H
#define POWERMGR_MULTI_USER_SCREEN_ADAPTER_H

#include <cstdint>

#ifdef POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION

namespace OHOS {
namespace PowerMgr {
class MultiUserScreenAdapter {
public:
    static constexpr int32_t INVALID_USER_ID = -1;
    static constexpr uint64_t INVALID_DISPLAY_ID = static_cast<uint64_t>(-1);

    static int32_t GetUserIdFromUid(int32_t uid);
    static bool IsSystemAccount(int32_t userId);
    // Valid user id: SYSTEM_USER_ID (0) or real multi-user [START_USER_ID, MAX_USER_ID].
    static bool IsValidUserId(int32_t userId);

    static uint64_t GetDisplayIdByUserId(int32_t userId);
    static int32_t GetUserIdByDisplayId(uint64_t displayId);

    static bool IsScreenOnForUser(int32_t userId);
    static bool IsUserMainScreen(uint64_t displayId);
    static bool IsAnyScreenOn();

private:
    static bool IsOnState(uint32_t state); // DISPLAY_ON || DISPLAY_DIM
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION
#endif // POWERMGR_MULTI_USER_SCREEN_ADAPTER_H
