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

#ifndef POWERMGR_WAKEUP_SOURCES_PARSER_H
#define POWERMGR_WAKEUP_SOURCES_PARSER_H

#include "wakeup_sources.h"

#include <memory>
#include <string>

#include <cJSON.h>

namespace OHOS {
namespace PowerMgr {
class WakeupSourceParser {
public:
    static std::shared_ptr<WakeupSources> ParseSources();
    static std::shared_ptr<WakeupSources> ParseSources(const std::string& config);
    static bool ParseSourcesProc(
        std::shared_ptr<WakeupSources>& parseSources, cJSON* valueObj, std::string& key);
    static bool GetTargetPath(std::string& targetPath);
    static void SetSettingsToDatabase(WakeupDeviceType type, bool enable);
    static const std::string GetWakeupSourcesByConfig();
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_SUSPEND_SOURCES_PARSER_H