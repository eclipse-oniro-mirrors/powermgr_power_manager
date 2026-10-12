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
#include <functional>
#include <string>

#ifdef POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION

#include <display_power_info.h>
#include <multi_screen_display_state_callback_stub.h>

namespace OHOS {
namespace PowerMgr {
// Cockpit multi-screen multi-user adaptation.
//
// In a cockpit multi-display multi-user topology, each display is bound to an OS-account user space
// (U10X). Screen on/off common events must be isolated per user space, and IsScreenOn queries must be
// adapted by caller identity: a U10X caller gets its own display state, while a system/admin (U0/SA)
// caller gets the whole-machine state ("any screen on").
//
// This adapter centralizes all interaction with display_manager multi-screen APIs and OsAccountManager,
// so PowerMgrService only schedules broadcast publishing.
class MultiUserScreenAdapter {
public:
    // uid -> os-account local id is a pure local arithmetic (uid / UID_TRANSFORM_DIVISOR), no IPC, so
    // the hot IsScreenOn query path never blocks the powermgr process on an account IPC.
    static constexpr int32_t UID_TRANSFORM_DIVISOR = 200000; // local account id = uid / 200000
    static constexpr int32_t START_MULTIPLE_USER_ID = 100;   // first real multi-user os-account id
    static constexpr int32_t INVALID_USER_ID = -1;
    static constexpr uint64_t INVALID_DISPLAY_ID = static_cast<uint64_t>(-1);

    using ScreenChangeHandler = std::function<void(uint64_t displayId, bool isScreenOn, uint32_t reason)>;

    // Register the multi-screen display-state callback against display_manager. The provided handler is
    // invoked on every per-display state change with a normalized (displayId, isScreenOn, reason) event.
    static bool RegisterScreenStateCallback();
    static bool UnregisterScreenStateCallback();

    // Set the routing handler that converts a normalized screen change into a PowerMgrService action.
    // Split out from RegisterScreenStateCallback so the callback entry can be driven in tests without
    // requiring a live display_manager registration.
    static void SetScreenChangeHandler(ScreenChangeHandler handler);

    // Drive the registered ScreenChangeHandler once per screen with each display's CURRENT state, right
    // after registration. This "replays" a synthetic state callback per screen so the initial per-user
    // (U10X) and whole-machine (U0) broadcasts fire for screens that were already on/off before powermgr
    // registered (stable screens emit no real callback). All screen querying lives here, in the adapter;
    // PowerMgrService only schedules the broadcast + the all-off fallback.
    static void InitMultiScreenNotify(uint32_t reason);

    // Caller-identity helpers (pure local arithmetic, no IPC).
    static int32_t GetUserIdFromUid(int32_t uid);
    static inline bool IsMultiUserUid(int32_t uid)
    {
        return GetUserIdFromUid(uid) >= START_MULTIPLE_USER_ID;
    }

    // displayId <-> userId mapping (OsAccountManager IPC). On failure return the INVALID sentinel so
    // callers can skip per-user broadcast / conservatively report screen off.
    static int32_t GetUserIdByDisplayId(uint64_t displayId);
    static uint64_t GetDisplayIdByUserId(int32_t userId);

    // Per-display / whole-machine screen state queries (display_manager, local cache lookup).
    static bool IsDisplayOn(uint64_t displayId);
    static bool IsAnyScreenOn(uint64_t excludeDisplayId = INVALID_DISPLAY_ID);
    static bool IsScreenOnForUser(int32_t userId);

    // display_manager callback entry. Converts the scoped-enum state into a normalized event and routes
    // it through the registered ScreenChangeHandler. This is the real entry driven by tests.
    class MultiScreenCallback : public OHOS::DisplayPowerMgr::MultiScreenDisplayStateCallbackStub {
    public:
        void OnMultiScreenDisplayStateChanged(uint64_t displayId, const std::string& screenName,
            OHOS::DisplayPowerMgr::DisplayState state,
            OHOS::DisplayPowerMgr::MultiScreenStateChangeReason reason) override;
    };

private:
    static sptr<MultiScreenCallback> multiScreenCallback_;
    static ScreenChangeHandler screenChangeHandler_;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION
#endif // POWERMGR_MULTI_USER_SCREEN_ADAPTER_H
