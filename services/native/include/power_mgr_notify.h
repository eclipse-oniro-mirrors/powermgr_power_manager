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

namespace OHOS {
namespace PowerMgr {
class PowerMgrNotify {
public:
    void RegisterPublishEvents();
    void PublishScreenOffEvents(int64_t eventTime);
    void PublishScreenOnEvents(int64_t eventTime);

private:
    using IntentWant = OHOS::AAFwk::Want;

    void PublishEvents(int64_t eventTime, sptr<IntentWant> intent);

    sptr<IntentWant> screenOffWant_ {nullptr};
    sptr<IntentWant> screenOnWant_ {nullptr};
    sptr<OHOS::EventFwk::CommonEventPublishInfo> publishInfo_ {nullptr};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_POWER_MGR_NOTIFY_H
