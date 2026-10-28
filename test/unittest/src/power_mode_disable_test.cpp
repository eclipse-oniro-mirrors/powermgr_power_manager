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

#include "power_mode_disable_test.h"

#include <parameters.h>

#include "permission.h"
#include "power_common.h"
#include "power_mgr_service.h"
#include "power_mode_module.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
constexpr uint32_t MODE_BELOW_MIN = 599;
constexpr uint32_t MODE_ABOVE_MAX = 651;
const std::string DISALLOWED_BATTERY_SAVER_KEY = "persist.edm.disallowed_battery_saver";
bool g_isSystem = true;
bool g_isPermissionGranted = true;
bool g_disallowedBatterySaver = false;
sptr<PowerMgrService> g_pms;
} // namespace

bool Permission::IsSystem()
{
    return g_isSystem;
}

bool Permission::IsPermissionGranted(const std::string& perm)
{
    return g_isPermissionGranted;
}

namespace OHOS {
namespace system {
bool GetBoolParameter(const std::string& key, bool def)
{
    if (key == DISALLOWED_BATTERY_SAVER_KEY) {
        return g_disallowedBatterySaver;
    }
    return def;
}
} // namespace system
} // namespace OHOS

void PowerModeDisableTest::SetUpTestCase()
{
    g_pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(g_pms != nullptr) << "PowerModeDisableTest fail to get PowerMgrService";
    g_pms->OnStart();
}

void PowerModeDisableTest::TearDownTestCase()
{
    g_pms->OnStop();
}

void PowerModeDisableTest::SetUp()
{
    g_isSystem = true;
    g_isPermissionGranted = true;
    g_disallowedBatterySaver = false;
    g_pms->SetDeviceMode(PowerMode::NORMAL_MODE);
}

void PowerModeDisableTest::TearDown()
{
    g_isSystem = true;
    g_isPermissionGranted = true;
    g_disallowedBatterySaver = false;
}

/**
 * @tc.name: SetPowerSaveModeDisabled001
 * @tc.desc: test set the disabled power save modes is intercepted when the policy is enabled
 * @tc.type: FUNC
 */
HWTEST_F(PowerModeDisableTest, SetPowerSaveModeDisabled001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SetPowerSaveModeDisabled001 function start!");
    g_disallowedBatterySaver = true;
    PowerErrors ret = g_pms->SetDeviceMode(PowerMode::POWER_SAVE_MODE);
    EXPECT_EQ(ret, PowerErrors::ERR_ENTERPRISE_POLICIES_DENIED);
    EXPECT_EQ(g_pms->GetDeviceMode(), PowerMode::NORMAL_MODE);

    ret = g_pms->SetDeviceMode(PowerMode::EXTREME_POWER_SAVE_MODE);
    EXPECT_EQ(ret, PowerErrors::ERR_ENTERPRISE_POLICIES_DENIED);
    EXPECT_EQ(g_pms->GetDeviceMode(), PowerMode::NORMAL_MODE);

    ret = g_pms->SetDeviceMode(PowerMode::CUSTOM_POWER_SAVE_MODE);
    EXPECT_EQ(ret, PowerErrors::ERR_ENTERPRISE_POLICIES_DENIED);
    EXPECT_EQ(g_pms->GetDeviceMode(), PowerMode::NORMAL_MODE);
    POWER_HILOGI(LABEL_TEST, "SetPowerSaveModeDisabled001 function end!");
}

/**
 * @tc.name: SetPowerSaveModeDisabled002
 * @tc.desc: test the modes outside the disabled set are not intercepted when the policy is enabled
 * @tc.type: FUNC
 */
HWTEST_F(PowerModeDisableTest, SetPowerSaveModeDisabled002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SetPowerSaveModeDisabled002 function start!");
    g_disallowedBatterySaver = true;
    EXPECT_EQ(g_pms->SetDeviceMode(PowerMode::NORMAL_MODE), PowerErrors::ERR_OK);
    EXPECT_EQ(g_pms->SetDeviceMode(PowerMode::PERFORMANCE_MODE), PowerErrors::ERR_OK);
    EXPECT_EQ(g_pms->GetDeviceMode(), PowerMode::PERFORMANCE_MODE);
    POWER_HILOGI(LABEL_TEST, "SetPowerSaveModeDisabled002 function end!");
}

/**
 * @tc.name: SetPowerSaveModeDisabled003
 * @tc.desc: test the power save modes are allowed when the policy is disabled
 * @tc.type: FUNC
 */
HWTEST_F(PowerModeDisableTest, SetPowerSaveModeDisabled003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SetPowerSaveModeDisabled003 function start!");
    g_disallowedBatterySaver = false;
    EXPECT_EQ(g_pms->SetDeviceMode(PowerMode::POWER_SAVE_MODE), PowerErrors::ERR_OK);
    EXPECT_EQ(g_pms->GetDeviceMode(), PowerMode::POWER_SAVE_MODE);

    EXPECT_EQ(g_pms->SetDeviceMode(PowerMode::NORMAL_MODE), PowerErrors::ERR_OK);
    EXPECT_EQ(g_pms->SetDeviceMode(PowerMode::EXTREME_POWER_SAVE_MODE), PowerErrors::ERR_OK);
    EXPECT_EQ(g_pms->GetDeviceMode(), PowerMode::EXTREME_POWER_SAVE_MODE);

    EXPECT_EQ(g_pms->SetDeviceMode(PowerMode::NORMAL_MODE), PowerErrors::ERR_OK);
    EXPECT_EQ(g_pms->SetDeviceMode(PowerMode::CUSTOM_POWER_SAVE_MODE), PowerErrors::ERR_OK);
    EXPECT_EQ(g_pms->GetDeviceMode(), PowerMode::CUSTOM_POWER_SAVE_MODE);
    POWER_HILOGI(LABEL_TEST, "SetPowerSaveModeDisabled003 function end!");
}

/**
 * @tc.name: SetPowerSaveModeDisabled004
 * @tc.desc: test the existing permission checks take precedence over the disabled check
 * @tc.type: FUNC
 */
HWTEST_F(PowerModeDisableTest, SetPowerSaveModeDisabled004, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SetPowerSaveModeDisabled004 function start!");
    g_disallowedBatterySaver = true;
    g_isSystem = false;
    EXPECT_EQ(g_pms->SetDeviceMode(PowerMode::POWER_SAVE_MODE), PowerErrors::ERR_SYSTEM_API_DENIED);

    g_isSystem = true;
    g_isPermissionGranted = false;
    EXPECT_EQ(g_pms->SetDeviceMode(PowerMode::POWER_SAVE_MODE), PowerErrors::ERR_PERMISSION_DENIED);

    g_isPermissionGranted = true;
    EXPECT_EQ(g_pms->SetDeviceMode(PowerMode::POWER_SAVE_MODE),
        PowerErrors::ERR_ENTERPRISE_POLICIES_DENIED);
    EXPECT_EQ(g_pms->GetDeviceMode(), PowerMode::NORMAL_MODE);
    POWER_HILOGI(LABEL_TEST, "SetPowerSaveModeDisabled004 function end!");
}

/**
 * @tc.name: SetPowerSaveModeDisabled005
 * @tc.desc: test the policy value change takes effect on the next call
 * @tc.type: FUNC
 */
HWTEST_F(PowerModeDisableTest, SetPowerSaveModeDisabled005, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SetPowerSaveModeDisabled005 function start!");
    g_disallowedBatterySaver = true;
    EXPECT_EQ(g_pms->SetDeviceMode(PowerMode::POWER_SAVE_MODE),
        PowerErrors::ERR_ENTERPRISE_POLICIES_DENIED);

    g_disallowedBatterySaver = false;
    EXPECT_EQ(g_pms->SetDeviceMode(PowerMode::POWER_SAVE_MODE), PowerErrors::ERR_OK);
    EXPECT_EQ(g_pms->GetDeviceMode(), PowerMode::POWER_SAVE_MODE);

    g_disallowedBatterySaver = true;
    EXPECT_EQ(g_pms->SetDeviceMode(PowerMode::NORMAL_MODE), PowerErrors::ERR_OK);
    EXPECT_EQ(g_pms->SetDeviceMode(PowerMode::POWER_SAVE_MODE),
        PowerErrors::ERR_ENTERPRISE_POLICIES_DENIED);
    POWER_HILOGI(LABEL_TEST, "SetPowerSaveModeDisabled005 function end!");
}

/**
 * @tc.name: SetPowerSaveModeDisabled006
 * @tc.desc: test the query is not intercepted and the current mode is not switched forcibly
 * @tc.type: FUNC
 */
HWTEST_F(PowerModeDisableTest, SetPowerSaveModeDisabled006, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SetPowerSaveModeDisabled006 function start!");
    EXPECT_EQ(g_pms->SetDeviceMode(PowerMode::POWER_SAVE_MODE), PowerErrors::ERR_OK);
    g_disallowedBatterySaver = true;
    EXPECT_EQ(g_pms->GetDeviceMode(), PowerMode::POWER_SAVE_MODE);
    EXPECT_EQ(g_pms->SetDeviceMode(PowerMode::NORMAL_MODE), PowerErrors::ERR_OK);
    EXPECT_EQ(g_pms->GetDeviceMode(), PowerMode::NORMAL_MODE);
    POWER_HILOGI(LABEL_TEST, "SetPowerSaveModeDisabled006 function end!");
}

/**
 * @tc.name: SetPowerSaveModeDisabled007
 * @tc.desc: test the invalid mode parameter is rejected before the disabled check
 * @tc.type: FUNC
 */
HWTEST_F(PowerModeDisableTest, SetPowerSaveModeDisabled007, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "SetPowerSaveModeDisabled007 function start!");
    g_disallowedBatterySaver = true;
    EXPECT_EQ(g_pms->SetDeviceMode(static_cast<PowerMode>(MODE_BELOW_MIN)), PowerErrors::ERR_PARAM_INVALID);
    EXPECT_EQ(g_pms->SetDeviceMode(static_cast<PowerMode>(MODE_ABOVE_MAX)), PowerErrors::ERR_PARAM_INVALID);
    EXPECT_EQ(g_pms->GetDeviceMode(), PowerMode::NORMAL_MODE);
    POWER_HILOGI(LABEL_TEST, "SetPowerSaveModeDisabled007 function end!");
}

/**
 * @tc.name: IsPowerSaveModeDisabled001
 * @tc.desc: test the judgment function reads the policy in real time
 * @tc.type: FUNC
 */
HWTEST_F(PowerModeDisableTest, IsPowerSaveModeDisabled001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "IsPowerSaveModeDisabled001 function start!");
    g_disallowedBatterySaver = true;
    EXPECT_TRUE(PowerModeModule::IsPowerSaveModeDisabled(PowerMode::POWER_SAVE_MODE));
    EXPECT_TRUE(PowerModeModule::IsPowerSaveModeDisabled(PowerMode::EXTREME_POWER_SAVE_MODE));
    EXPECT_TRUE(PowerModeModule::IsPowerSaveModeDisabled(PowerMode::CUSTOM_POWER_SAVE_MODE));
    EXPECT_FALSE(PowerModeModule::IsPowerSaveModeDisabled(PowerMode::NORMAL_MODE));
    EXPECT_FALSE(PowerModeModule::IsPowerSaveModeDisabled(PowerMode::PERFORMANCE_MODE));

    g_disallowedBatterySaver = false;
    EXPECT_FALSE(PowerModeModule::IsPowerSaveModeDisabled(PowerMode::POWER_SAVE_MODE));
    EXPECT_FALSE(PowerModeModule::IsPowerSaveModeDisabled(PowerMode::EXTREME_POWER_SAVE_MODE));
    EXPECT_FALSE(PowerModeModule::IsPowerSaveModeDisabled(PowerMode::CUSTOM_POWER_SAVE_MODE));
    POWER_HILOGI(LABEL_TEST, "IsPowerSaveModeDisabled001 function end!");
}
