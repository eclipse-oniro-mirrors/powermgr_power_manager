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

#include <fstream>
#include <unistd.h>

#include <cJSON.h>
#include "config_policy_utils.h"
#include "power_log.h"
#include "setting_helper.h"
#include "wakeup_source_parser.h"

namespace OHOS {
namespace PowerMgr {

namespace {
static const std::string POWER_WAKEUP_CONFIG_FILE = "etc/power_config/power_wakeup.json";
static const std::string VENDOR_POWER_WAKEUP_CONFIG_FILE = "/vendor/etc/power_config/power_wakeup.json";
static const std::string SYSTEM_POWER_WAKEUP_CONFIG_FILE = "/system/etc/power_config/power_wakeup.json";
static const uint32_t SINGLE_CLICK = static_cast<uint32_t>(WakeUpAction::CLICK_SINGLE);
static const uint32_t DOUBLE_CLICK = static_cast<uint32_t>(WakeUpAction::CLICK_DOUBLE);
} // namespace
bool g_isFirstSettingUpdated = true;

std::shared_ptr<WakeupSources> WakeupSourceParser::ParseSources()
{
    bool isWakeupSourcesSettingValid = SettingHelper::IsWakeupSourcesSettingValid();
    POWER_HILOGI(FEATURE_WAKEUP, "ParseSources setting=%{public}d", isWakeupSourcesSettingValid);
    std::string configJsonStr;
    if (isWakeupSourcesSettingValid) {
        std::string sourcesSettingStr = SettingHelper::GetSettingWakeupSources();
        configJsonStr = sourcesSettingStr;
#ifdef POWER_MANAGER_ENABLE_WATCH_UPDATE_ADAPT
        // this branch means use config file for update scene in watch
        if (sourcesSettingStr.find(WakeupSources::TP_TOUCH_KEY) == std::string::npos) {
            configJsonStr = GetWakeupSourcesByConfig();
            POWER_HILOGW(FEATURE_WAKEUP, "update scene need use (config file)");
        }
#endif
    } else {
        configJsonStr = GetWakeupSourcesByConfig();
    }
    g_isFirstSettingUpdated = true;
    std::shared_ptr<WakeupSources> parseSources = ParseSources(configJsonStr);
    if (parseSources->GetParseErrorFlag()) {
        POWER_HILOGI(FEATURE_WAKEUP, "call GetWakeupSourcesByConfig again");
        configJsonStr = GetWakeupSourcesByConfig();
        parseSources = ParseSources(configJsonStr);
    }
    if (parseSources != nullptr) {
        SettingHelper::SetSettingWakeupSources(configJsonStr);
    }
    g_isFirstSettingUpdated = false;
    return parseSources;
}

const std::string WakeupSourceParser::GetWakeupSourcesByConfig()
{
    std::string targetPath;
    bool ret = GetTargetPath(targetPath);
    if (ret == false) {
        return "";
    }

    POWER_HILOGI(FEATURE_WAKEUP, "use targetPath=%{public}s", targetPath.c_str());
    std::ifstream inputStream(targetPath.c_str(), std::ios::in | std::ios::binary);
    return std::string(std::istreambuf_iterator<char> {inputStream}, std::istreambuf_iterator<char> {});
}

bool WakeupSourceParser::GetTargetPath(std::string& targetPath)
{
    targetPath.clear();
    char buf[MAX_PATH_LEN];
    char* path = GetOneCfgFile(POWER_WAKEUP_CONFIG_FILE.c_str(), buf, MAX_PATH_LEN);
    if (path != nullptr && *path != '\0') {
        POWER_HILOGI(FEATURE_WAKEUP, "use policy path=%{public}s", path);
        targetPath = path;
        return true;
    }

    if (access(VENDOR_POWER_WAKEUP_CONFIG_FILE.c_str(), F_OK | R_OK) == -1) {
        POWER_HILOGE(FEATURE_WAKEUP, "vendor wakeup config is not exist or permission denied");
        if (access(SYSTEM_POWER_WAKEUP_CONFIG_FILE.c_str(), F_OK | R_OK) == -1) {
            POWER_HILOGE(FEATURE_WAKEUP, "system wakeup config is not exist or permission denied");
            return false;
        } else {
            targetPath = SYSTEM_POWER_WAKEUP_CONFIG_FILE;
        }
    } else {
        targetPath = VENDOR_POWER_WAKEUP_CONFIG_FILE;
    }

    return true;
}

std::shared_ptr<WakeupSources> WakeupSourceParser::ParseSources(const std::string& jsonStr)
{
    std::shared_ptr<WakeupSources> parseSources = std::make_shared<WakeupSources>();
    cJSON* root = cJSON_Parse(jsonStr.c_str());
    if (!root) {
        POWER_HILOGE(FEATURE_WAKEUP, "json parse error");
        parseSources->SetParseErrorFlag(true);
        return parseSources;
    }

    if (!cJSON_IsObject(root)) {
        POWER_HILOGE(FEATURE_WAKEUP, "json root invalid[%{public}s]", jsonStr.c_str());
        parseSources->SetParseErrorFlag(true);
        cJSON_Delete(root);
        return parseSources;
    }

    cJSON* item = nullptr;
    cJSON_ArrayForEach(item, root) {
        const char* key = item->string;
        if (!key) {
            POWER_HILOGI(FEATURE_WAKEUP, "invalid key in json object");
            continue;
        }
        std::string keyStr = std::string(key);
        bool ret = ParseSourcesProc(parseSources, item, keyStr);
        if (ret == false) {
            POWER_HILOGI(FEATURE_WAKEUP, "lost map config key");
            continue;
        }
    }

    cJSON_Delete(root);
    return parseSources;
}

bool WakeupSourceParser::ParseSourcesProc(
    std::shared_ptr<WakeupSources>& parseSources, cJSON* valueObj, std::string& key)
{
    bool enable = true;
    uint32_t click = DOUBLE_CLICK;
    WakeupDeviceType wakeupDeviceType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    if (valueObj && cJSON_IsObject(valueObj)) {
        cJSON* enableValue = cJSON_GetObjectItemCaseSensitive(valueObj, WakeupSource::ENABLE_KEY);
        if (enableValue && cJSON_IsBool(enableValue)) {
            enable = cJSON_IsTrue(enableValue);
        }
        cJSON* clickValue = cJSON_GetObjectItemCaseSensitive(valueObj, WakeupSource::KEYS_KEY);
        if (clickValue && cJSON_IsNumber(clickValue)) {
            uint32_t clickInt = static_cast<uint32_t>(clickValue->valueint);
            click = (clickInt == SINGLE_CLICK || clickInt == DOUBLE_CLICK) ? clickInt : DOUBLE_CLICK;
        }
    }

    wakeupDeviceType = WakeupSources::mapWakeupDeviceType(key, click);
    POWER_HILOGI(FEATURE_WAKEUP, "key=%{public}s, type=%{public}u, click=%{public}u, enable=%{public}d",
        key.c_str(), wakeupDeviceType, click, enable);

    if (wakeupDeviceType == WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN) {
        return false;
    }

    if (!enable && g_isFirstSettingUpdated) {
        if (wakeupDeviceType == WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK
            && (!SettingHelper::IsWakeupDoubleSettingValid())) {
            SettingHelper::SetSettingWakeupDouble(enable);
            POWER_HILOGI(FEATURE_WAKEUP, "the setting wakeupDoubleClick enable=%{public}d", enable);
        }

        if (wakeupDeviceType == WakeupDeviceType::WAKEUP_DEVICE_PICKUP
            && (!SettingHelper::IsWakeupPickupSettingValid())) {
            SettingHelper::SetSettingWakeupPickup(enable);
            POWER_HILOGI(FEATURE_WAKEUP, "the setting pickup enable=%{public}d", enable);
        }
    }

    if (enable == true) {
        WakeupSource wakeupSource = WakeupSource(wakeupDeviceType, enable, click);
        parseSources->PutSource(wakeupSource);
    }

    SetSettingsToDatabase(wakeupDeviceType, enable);
    return true;
}

void WakeupSourceParser::SetSettingsToDatabase(WakeupDeviceType type, bool enable)
{
    if (type == WakeupDeviceType::WAKEUP_DEVICE_LID) {
        if (!SettingHelper::IsWakeupLidSettingValid()) {
            SettingHelper::SetSettingWakeupLid(enable);
            POWER_HILOGI(FEATURE_WAKEUP, "the setting lidwakeup enable=%{public}d", enable);
        }
    }
}
} // namespace PowerMgr
} // namespace OHOS