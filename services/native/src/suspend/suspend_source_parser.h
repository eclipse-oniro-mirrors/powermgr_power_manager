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

#ifndef POWERMGR_SUSPEND_SOURCES_PARSER_H
#define POWERMGR_SUSPEND_SOURCES_PARSER_H

#include <memory>
#include <string>

#include <cJSON.h>
#include "suspend_sources.h"

namespace OHOS {
namespace PowerMgr {
class SuspendSourceParser {
public:
    static std::shared_ptr<SuspendSources> ParseSources();
    static std::shared_ptr<SuspendSources> ParseSources(const std::string& config);
    static bool GetTargetPath(std::string& targetPath);
    static bool ParseSourcesProc(
        std::shared_ptr<SuspendSources> &parseSources,  cJSON* valueObj, std::string& key);
    static const std::string GetSuspendSourcesByConfig();
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_SUSPEND_SOURCES_PARSER_H