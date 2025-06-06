/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "power_get_mode_test.h"

#include <iostream>

#include <datetime_ex.h>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <string_ex.h>

#include "power_common.h"
#include "power_log.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "power_state_machine.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
/**
 * @tc.name: GetDeviceModeTest001
 * @tc.desc: test GetDeviceMode in proxy
 * @tc.type: FUNC
 */
HWTEST_F (PowerGetModeTest, GetDeviceModeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDeviceModeTest001: GetDeviceMode start.";
    POWER_HILOGI(LABEL_TEST, "GetDeviceModeTest001 function start!");
    PowerMode modeFirst;
    PowerMode modeSecond = PowerMode::NORMAL_MODE;
    sleep(SLEEP_WAIT_TIME_S);
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    modeFirst = powerMgrClient.GetDeviceMode();
    powerMgrClient.SetDeviceMode(modeSecond);
    EXPECT_EQ(modeSecond, powerMgrClient.GetDeviceMode());
    powerMgrClient.SetDeviceMode(modeFirst);
    POWER_HILOGI(LABEL_TEST, "GetDeviceModeTest001 function end!");
    GTEST_LOG_(INFO) << "GetDeviceModeTest001: GetDeviceMode end.";
}
}