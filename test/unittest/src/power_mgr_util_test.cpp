/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <system_ability_definition.h>

#include "accesstoken_kit.h"
#include "mock_accesstoken_kit.h"
#include "permission.h"
#include "power_common.h"
#include "power_log.h"
#include "setting_observer.h"
#include "setting_provider.h"
#include "sysparam.h"
#include "tokenid_kit.h"

using namespace OHOS::Security::AccessToken;
using namespace OHOS::PowerMgr;
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace PowerMgr {
class PowerMgrUtilTest : public testing::Test {};
} // namespace PowerMgr
} // namespace OHOS

namespace {
/**
 * @tc.name: PermissionIsSystemNative
 * @tc.desc: The IsSystem and IsPermissionGranted functions are granted by default as TOKEN_NATIVE or TOKEN_SHELL types
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrUtilTest, PermissionIsSystemNative, TestSize.Level0)
{
    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_NATIVE);
    EXPECT_TRUE(Permission::IsSystem());
    EXPECT_TRUE(Permission::IsPermissionGranted("REBOOT"));

    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_SHELL);
    EXPECT_TRUE(Permission::IsSystem());
    EXPECT_TRUE(Permission::IsPermissionGranted("REBOOT"));
}

/**
 * @tc.name: PermissionIsSystemHap
 * @tc.desc: The function IsSystem and IsPermissionGranted in the test TOKEN_HAP
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrUtilTest, PermissionIsSystemHap, TestSize.Level0)
{
    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_HAP);
    MockAccesstokenKit::MockSetSystemApp(false);
    EXPECT_FALSE(Permission::IsSystem());

    MockAccesstokenKit::MockSetSystemApp(true);
    EXPECT_TRUE(Permission::IsSystem());
    MockAccesstokenKit::MockSetPermissionState(PermissionState::PERMISSION_GRANTED);
    EXPECT_TRUE(Permission::IsPermissionGranted("REBOOT"));
}

/**
 * @tc.name: PermissionIsSystemInvalid
 * @tc.desc: The IsSystem and IsPermissionGranted do not have permissions on TOKEN_INVALID or TOKEN_TYPE_BUTT types
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrUtilTest, PermissionIsSystemInvalid, TestSize.Level0)
{
    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_INVALID);
    EXPECT_FALSE(Permission::IsSystem());
    EXPECT_FALSE(Permission::IsPermissionGranted("REBOOT"));

    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_TYPE_BUTT);
    EXPECT_FALSE(Permission::IsSystem());
    EXPECT_FALSE(Permission::IsPermissionGranted("REBOOT"));
}

/**
 * @tc.name: PermissionIsPermissionGrantedHap
 * @tc.desc: Test Permission function IsPermissionGranted is TOKEN_HAP
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrUtilTest, PermissionIsPermissionGrantedHap, TestSize.Level0)
{
    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_HAP);
    MockAccesstokenKit::MockSetPermissionState(PermissionState::PERMISSION_DENIED);
    EXPECT_FALSE(Permission::IsPermissionGranted("REBOOT"));

    MockAccesstokenKit::MockSetPermissionState(PermissionState::PERMISSION_GRANTED);
    EXPECT_TRUE(Permission::IsPermissionGranted("REBOOT"));
}

/**
 * @tc.name: PermissionIsPermissionGrantedNative
 * @tc.desc: Test function IsPermissionGranted is TOKEN_NATIVE or TOKEN_SHELL with permissions by default
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrUtilTest, PermissionIsPermissionGrantedNative, TestSize.Level0)
{
    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_NATIVE);
    EXPECT_TRUE(Permission::IsPermissionGranted("REBOOT"));

    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_SHELL);
    EXPECT_TRUE(Permission::IsPermissionGranted("REBOOT"));
}

/**
 * @tc.name: PermissionIsPermissionGrantedInvalid
 * @tc.desc: Test Permission function IsSystem is TOKEN_INVALID or TOKEN_TYPE_BUTT without permission
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrUtilTest, PermissionIsPermissionGrantedInvalid, TestSize.Level0)
{
    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_INVALID);
    EXPECT_FALSE(Permission::IsPermissionGranted("REBOOT"));
    MockAccesstokenKit::MockSetTokenTypeFlag(ATokenTypeEnum::TOKEN_TYPE_BUTT);
    EXPECT_FALSE(Permission::IsPermissionGranted("REBOOT"));
}

/**
 * @tc.name: SettingObserverTest001
 * @tc.desc: test SetKey in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrUtilTest, SettingObserver001, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "SettingObserver001::fun is start!");
    std::shared_ptr<SettingObserver> settingObserver = std::make_shared<SettingObserver>();
    settingObserver->OnChange();
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {};
    settingObserver->SetUpdateFunc(updateFunc);
    settingObserver->SetKey("settings.power.wakeup_sources");
    std::string key = settingObserver->GetKey();
    EXPECT_EQ(key, "settings.power.wakeup_sources");
    POWER_HILOGD(LABEL_TEST, "SettingObserver001::fun is end!");
}

/**
 * @tc.name: SettingProviderTest001
 * @tc.desc: test CreateObserver in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrUtilTest, SettingProvider001, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "SettingProvider001::fun is start!");
    auto& settingProvider = SettingProvider::GetInstance(OHOS::POWER_MANAGER_SERVICE_ID);
    std::string valueStr;
    settingProvider.GetStringValue("settings.power.wakeup_sources", valueStr);
    bool valueBool;
    settingProvider.GetBoolValue("settings.power.suspend_sources", valueBool);
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {};
    auto observer = settingProvider.CreateObserver("settings.power.suspend_sources", updateFunc);
    EXPECT_TRUE(observer != nullptr);
    POWER_HILOGD(LABEL_TEST, "SettingProvider001::fun is end!");
}

/**
 * @tc.name: SettingProviderTest002
 * @tc.desc: test RegisterObserver in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrUtilTest, SettingProvider002, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "SettingProvider002::fun is start!");
    auto& settingProvider = SettingProvider::GetInstance(OHOS::POWER_MANAGER_SERVICE_ID);
    int32_t putValue = 10;
    int32_t getValue;
    settingProvider.PutIntValue("settings.power.suspend_sources", putValue);
    settingProvider.GetIntValue("settings.power.suspend_sources", getValue);
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {};
    auto observer = settingProvider.CreateObserver("settings.power.suspend_sources", updateFunc);
    EXPECT_EQ(OHOS::ERR_OK, settingProvider.RegisterObserver(observer));
    POWER_HILOGD(LABEL_TEST, "SettingProvider002::fun is end!");
}

/**
 * @tc.name: SettingProviderTest003
 * @tc.desc: test UnregisterObserver in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrUtilTest, SettingProvider003, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "SettingProvider003::fun is start!");
    auto& settingProvider = SettingProvider::GetInstance(OHOS::POWER_MANAGER_SERVICE_ID);
    int64_t value;
    settingProvider.GetLongValue("settings.display.screen_off_timeout", value);
    settingProvider.IsValidKey("settings.power.suspend_sources");
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {};
    auto observer = settingProvider.CreateObserver("settings.power.suspend_sources", updateFunc);
    OHOS::ErrCode ret = settingProvider.RegisterObserver(observer);
    ret = settingProvider.UnregisterObserver(observer);
    EXPECT_EQ(OHOS::ERR_OK, ret);
    POWER_HILOGD(LABEL_TEST, "SettingProvider003::fun is end!");
}

/**
 * @tc.name: SysparamTest001
 * @tc.desc: test GetIntValue in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrUtilTest, Sysparam001, TestSize.Level0)
{
    POWER_HILOGD(LABEL_TEST, "Sysparam001::fun is start!");
    std::shared_ptr<SysParam> sysParam = std::make_shared<SysParam>();
    int32_t def = 0;
    EXPECT_EQ(OHOS::ERR_OK, sysParam->GetIntValue("settings.power.suspend_sources", def));
    POWER_HILOGD(LABEL_TEST, "Sysparam001::fun is end!");
}
} // namespace