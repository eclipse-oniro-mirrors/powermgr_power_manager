/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef POWER_MODE_INFO_H
#define POWER_MODE_INFO_H

#include <string>
#include <parcel.h>

namespace OHOS {
namespace PowerMgr {
enum class PowerMode : uint32_t {
    POWER_MODE_MIN = 600,
    NORMAL_MODE = POWER_MODE_MIN,
    POWER_SAVE_MODE,
    PERFORMANCE_MODE,
    EXTREME_POWER_SAVE_MODE,
    CUSTOM_POWER_SAVE_MODE = 650,
    POWER_MODE_MAX = CUSTOM_POWER_SAVE_MODE
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWER_MODE_INFO_H
