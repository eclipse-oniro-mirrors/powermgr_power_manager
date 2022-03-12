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

#include "power_mgr_monitor.h"

#include <common_event_manager.h>
#include <common_event_support.h>
#include <unistd.h>

#include "power_common.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"

using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;
using namespace std;

namespace OHOS {
namespace PowerMgr {
const map<string, PowerMgrMonitor::HandleEventFunc> PowerMgrMonitor::EVENT_HANDLES {
    {CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED, &PowerMgrMonitor::HandleStartUpCompleted},
};

PowerMgrMonitor::~PowerMgrMonitor()
{
    if (subscriber_ != nullptr) {
        CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    }
}

bool PowerMgrMonitor::Start()
{
    InitEventHandles();
    return RegisterSubscriber(GetSubscribeInfo());
}

void PowerMgrMonitor::InitEventHandles()
{
    if (!eventHandles_.empty()) {
        return;
    }
    for (auto &eh : EVENT_HANDLES) {
        POWER_HILOGI(MODULE_SERVICE, "Add event: %{public}s", eh.first.c_str());
        eventHandles_.emplace(eh.first, bind(eh.second, this, placeholders::_1));
    }
}

sptr<CesInfo> PowerMgrMonitor::GetSubscribeInfo() const
{
    MatchingSkills skill;
    for (auto &eh : eventHandles_) {
        skill.AddEvent(eh.first);
    }
    sptr<CesInfo> info = new CesInfo(skill);
    return info;
}

bool PowerMgrMonitor::RegisterSubscriber(const sptr<CesInfo>& info)
{
    static const int32_t MAX_RETRY_TIMES = 2;

    auto succeed = false;
    shared_ptr<Ces> s = make_shared<EventSubscriber>(info, eventHandles_);
    // Retry to register subscriber due to timming issues between system abilities
    for (int32_t tryTimes = 0; tryTimes < MAX_RETRY_TIMES; tryTimes++) {
        succeed = CommonEventManager::SubscribeCommonEvent(s);
        if (succeed) {
            break;
        }
        POWER_HILOGE(MODULE_SERVICE, "Sleep for a while and retry to register subscriber");
        usleep(50000); // sleep 50ms
    }
    if (!succeed) {
        POWER_HILOGE(MODULE_SERVICE, "Failed to register subscriber");
        return false;
    }
    subscriber_ = s;
    POWER_HILOGI(MODULE_SERVICE, "Succeed to register subscriber");
    return true;
}

void PowerMgrMonitor::HandleScreenStateChanged(const IntentWant& want) const
{
    bool isScreenOn = want.GetAction() == CommonEventSupport::COMMON_EVENT_SCREEN_ON;
    POWER_HILOGD(MODULE_SERVICE, "Screen is %{public}s", isScreenOn ? "on" : "off");
    DelayedSpSingleton<PowerMgrService>::GetInstance()->GetPowerStateMachine()->ReceiveScreenEvent(isScreenOn);
}

void PowerMgrMonitor::HandleStartUpCompleted(const IntentWant& want __attribute__((__unused__))) const
{
    POWER_HILOGD(MODULE_SERVICE, "Start up completed");
    DelayedSpSingleton<PowerMgrService>::GetInstance()->GetPowerStateMachine()->ResetInactiveTimer();
}

void PowerMgrMonitor::EventSubscriber::HandleEvent(const IntentWant& want)
{
    auto action = want.GetAction();
    auto it = eventHandles_.find(action);
    if (it == eventHandles_.end()) {
        POWER_HILOGI(MODULE_SERVICE, "Ignore event: %{public}s", action.c_str());
        return;
    }
    POWER_HILOGI(MODULE_SERVICE, "Handle event: %{public}s", action.c_str());
    it->second(want);
}
} // namespace PowerMgr
} // namespace OHOS
