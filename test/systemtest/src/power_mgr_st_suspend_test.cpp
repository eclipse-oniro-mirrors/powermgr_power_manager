/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "power_mgr_st_suspend_test.h"
#include "power_state_machine.h"
#include "suspend_controller.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

static sptr<PowerMgrService> g_service;
static constexpr int SLEEP_AFTER_LOCK_TIME_US = 1000 * 1000;

void PowerMgrSTSuspendTest::SetUpTestCase(void)
{
    // create singleton service object at the beginning
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
    g_service->GetPowerStateMachine()->SetSleepTime(SLEEP_WAIT_TIME_S);
}

void PowerMgrSTSuspendTest::TearDownTestCase(void)
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

void SuspendController::HandleAutoSleep(SuspendDeviceType reason)
{
    POWER_HILOGI(FEATURE_INPUT, "auto suspend by reason=%{public}d", reason);

    bool ret = stateMachine_->SetState(
        PowerState::SLEEP, stateMachine_->GetReasonBySuspendType(reason));
    if (ret) {
        POWER_HILOGI(FEATURE_INPUT, "State changed, Mock suspend intreface");
    } else {
        POWER_HILOGI(FEATURE_INPUT, "auto suspend: State change failed");
    }
}

namespace {
/**
 * @tc.name: PowerMgrMockSuspend001
 * @tc.desc: test suspend by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTSuspendTest, PowerMgrMockSuspend001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMockSuspend001: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMockSuspend001 function start!");
    int64_t PARM_ZERO = 0;
    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMockSuspend001: Failed to get PowerMgrService";
    }
    pms->WakeupDevice(PARM_ZERO, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("test"));
    pms->SuspendControllerInit();
    auto suspendController = pms->GetSuspendController();
    suspendController->ExecSuspendMonitorByReason(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY);
    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());
    POWER_HILOGI(LABEL_TEST, "PowerMgrMockSuspend001 function end!");
    GTEST_LOG_(INFO) << "PowerMgrMockSuspend001: end";
}

/**
 * @tc.name: PowerMgrMockSuspend002
 * @tc.desc: test proximity screen control RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTSuspendTest, PowerMgrMockSuspend002, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMockSuspend002: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMockSuspend002 function start!");
#ifdef HAS_SENSORS_SENSOR_PART
    int64_t PARM_ZERO = 0;
    int PARM_THREE = 3;
    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMockSuspend002: Failed to get PowerMgrService";
    }

    pms->WakeupDevice(PARM_ZERO, WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, std::string("test"));
    auto powerStateMachine = pms->GetPowerStateMachine();
    powerStateMachine->SetDisplayOffTime(SET_DISPLAY_OFF_TIME, false);

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    pms->CreateRunningLock(token, info);
    pms->Lock(token);
    EXPECT_EQ(pms->IsUsed(token), true);
    sleep(SLEEP_WAIT_TIME_S);

    pms->UnLock(token);
    EXPECT_EQ(pms->IsUsed(token), false);
    sleep(SLEEP_WAIT_TIME_S + PARM_THREE * ONE_SECOND);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    powerStateMachine->SetDisplayOffTime(PowerStateMachine::DEFAULT_DISPLAY_OFF_TIME_MS, false);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrMockSuspend002 function end!");
    GTEST_LOG_(INFO) << "PowerMgrMockSuspend002: end";
}

/**
 * @tc.name: PowerMgrMock003
 * @tc.desc: test Screen RunningLock by mock
 * @tc.type: FUNC
 * @tc.require: issueI5MJZJ
 */
HWTEST_F(PowerMgrSTSuspendTest, PowerMgrMock003, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "PowerMgrMock003: start";
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock003 function start!");
    sptr<PowerMgrService> pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        GTEST_LOG_(INFO) << "PowerMgrMock003: Failed to get PowerMgrService";
    }

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    pms->CreateRunningLock(token, info);

    pms->Lock(token);
    usleep(SLEEP_AFTER_LOCK_TIME_US);
    EXPECT_EQ(pms->IsUsed(token), true);

    pms->SuspendControllerInit();
    auto suspendController = pms->GetSuspendController();
    suspendController->ExecSuspendMonitorByReason(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY);
    sleep(SLEEP_WAIT_TIME_S + ONE_SECOND);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());

    pms->UnLock(token);

    EXPECT_EQ(pms->IsUsed(token), false);
    EXPECT_EQ(PowerState::SLEEP, pms->GetState());
    POWER_HILOGI(LABEL_TEST, "PowerMgrMock003 function end!");
    GTEST_LOG_(INFO) << "PowerMgrMock003: end";
}
} // namespace