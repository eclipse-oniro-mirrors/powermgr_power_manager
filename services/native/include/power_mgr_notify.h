/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWER_MGR_NOTIFY_H
#define POWERMGR_POWER_MGR_NOTIFY_H

#include <mutex>

#include <common_event_publish_info.h>

#include "errors.h"
#include "want.h"
#include "ffrt.h"

namespace OHOS {
namespace PowerMgr {
class PowerMgrNotify {
public:
    void RegisterPublishEvents();
    void PublishScreenOffEvents(int64_t eventTime, const std::string& reason);
    void PublishScreenOnEvents(int64_t eventTime, const std::string& reason);
#ifdef POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION
    // Cockpit multi-screen multi-user: publish the screen on/off event only to a specific user space
    // (the OS-account bound to a given display), instead of the whole-machine broadcast above.
    void PublishScreenOffEventsToUser(int64_t eventTime, const std::string& reason, int32_t userId);
    void PublishScreenOnEventsToUser(int64_t eventTime, const std::string& reason, int32_t userId);
    // Cockpit multi-user: whole-machine screen on/off broadcast targeted to the U0 (system) user space
    // via PublishCommonEventAsUser(userId=0). Distinct from PublishScreenOnEvents (gated by
    // POWER_MANAGER_ENABLE_SCREEN_DECOUPLING), this always emits so the multi-screen callback path can
    // announce the whole-machine state (initial baseline at register + off<->on transitions) regardless
    // of the decoupling flag.
    void PublishScreenOffEventsToU0(int64_t eventTime, const std::string& reason);
    void PublishScreenOnEventsToU0(int64_t eventTime, const std::string& reason);
#endif
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    void PublishEnterForceSleepEvents(int64_t eventTime);
    void PublishExitForceSleepEvents(int64_t eventTime);
#endif
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    void PublishEnterHibernateEvent(int64_t eventTime, bool clearMemory);
    void PublishExitHibernateEvent(int64_t eventTime, bool clearMemory);
#endif
    static void PublishCustomizedEvent(const AAFwk::Want& want);

private:
    using IntentWant = OHOS::AAFwk::Want;

    void PublishEvents(int64_t eventTime, sptr<IntentWant> intent);

    sptr<IntentWant> screenOffWant_ {nullptr};
    sptr<IntentWant> screenOnWant_ {nullptr};
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    sptr<IntentWant> enterForceSleepWant_ {nullptr};
    sptr<IntentWant> exitForceSleepWant_ {nullptr};
#endif
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    sptr<IntentWant> enterHibernateWant_ {nullptr};
    sptr<IntentWant> exitHibernateWant_ {nullptr};
#endif
    sptr<OHOS::EventFwk::CommonEventPublishInfo> publishInfo_ {nullptr};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MGR_NOTIFY_H
