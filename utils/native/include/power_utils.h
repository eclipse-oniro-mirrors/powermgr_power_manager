/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef POWER_UTILS_H
#define POWER_UTILS_H

#include "dm_common.h"
#include "power_state_machine_info.h"
#include "running_lock_info.h"

namespace OHOS {
namespace PowerMgr {
class PowerUtils {
public:
    static const std::string GetReasonTypeString(StateChangeReason type);
    static const std::string GetPowerStateString(PowerState state);
    static const std::string GetDisplayStateString(DisplayState state);
    static const std::string GetRunningLockTypeString(RunningLockType type);
    static Rosen::PowerStateChangeReason GetDmsReasonByPowerReason(StateChangeReason reason);
    static StateChangeReason GetReasonByUserActivity(UserActivityType type);
    static WakeupDeviceType ParseWakeupDeviceType(const std::string& details);
    static const std::string JsonToSimpleStr(const std::string& json);
    static bool IsForegroundApplication(const std::string& appName);
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWER_UTILS_H
