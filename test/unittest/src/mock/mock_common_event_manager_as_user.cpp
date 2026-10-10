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

#ifdef POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION
#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_publish_info.h>
#include <common_event_subscriber.h>
#include <common_event_support.h>

#include <memory>

namespace {
// Link-time mock for the per-user broadcast API. PowerMgrNotify::PublishScreenOnEventsToUser /
// PublishScreenOffEventsToUser call this overload; defining it here lets the test assert which user
// space received the event without a live common event service.
struct MockCommonEvent {
    bool ret {true};
    int callCount {0};
    int32_t lastUserId {-1};
    int onCallCount {0};
    int offCallCount {0};
    int wholeMachineCallCount {0};
    int wholeMachineOnCount {0};
    int wholeMachineOffCount {0};
};
MockCommonEvent g_mockCommonEvent;
} // namespace

namespace OHOS::PowerMgr::TestMock {
void ResetCommonEventMock()
{
    g_mockCommonEvent = MockCommonEvent {};
}
bool& MockPublishAsUserRet()
{
    return g_mockCommonEvent.ret;
}
int& MockPublishAsUserCallCount()
{
    return g_mockCommonEvent.callCount;
}
int32_t& MockPublishAsUserLastUserId()
{
    return g_mockCommonEvent.lastUserId;
}
int& MockPublishAsUserOnCount()
{
    return g_mockCommonEvent.onCallCount;
}
int& MockPublishAsUserOffCount()
{
    return g_mockCommonEvent.offCallCount;
}
int& MockWholeMachineCallCount()
{
    return g_mockCommonEvent.wholeMachineCallCount;
}
int& MockWholeMachineOnCount()
{
    return g_mockCommonEvent.wholeMachineOnCount;
}
int& MockWholeMachineOffCount()
{
    return g_mockCommonEvent.wholeMachineOffCount;
}
} // namespace OHOS::PowerMgr::TestMock

namespace OHOS {
namespace EventFwk {
bool CommonEventManager::PublishCommonEventAsUser(
    const CommonEventData& data, const CommonEventPublishInfo& publishInfo, const int32_t& userId)
{
    (void)publishInfo;
    g_mockCommonEvent.callCount++;
    g_mockCommonEvent.lastUserId = userId;
    const auto& action = data.GetWant().GetAction();
    if (action == CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        g_mockCommonEvent.onCallCount++;
    } else if (action == CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
        g_mockCommonEvent.offCallCount++;
    }
    return g_mockCommonEvent.ret;
}

bool CommonEventManager::PublishCommonEvent(const CommonEventData& data,
    const CommonEventPublishInfo& publishInfo, const std::shared_ptr<CommonEventSubscriber>& subscriber)
{
    (void)publishInfo;
    (void)subscriber;
    g_mockCommonEvent.wholeMachineCallCount++;
    const auto& action = data.GetWant().GetAction();
    if (action == CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        g_mockCommonEvent.wholeMachineOnCount++;
    } else if (action == CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
        g_mockCommonEvent.wholeMachineOffCount++;
    }
    return g_mockCommonEvent.ret;
}
} // namespace EventFwk
} // namespace OHOS
#endif // POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION
