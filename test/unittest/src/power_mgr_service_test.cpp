/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "power_mgr_service_test.h"

#include <csignal>
#include <iostream>

#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <string_ex.h>
#include <system_ability_definition.h>

#include "power_common.h"
#include "power_mgr_client.h"
#include "power_mgr_service.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void PowerMgrServiceTest::SetUpTestCase(void)
{
}

void PowerMgrServiceTest::TearDownTestCase(void)
{
}

void PowerMgrServiceTest::SetUp(void)
{
}

void PowerMgrServiceTest::TearDown(void)
{
}

namespace {
/**
 * @tc.name: PowerMgrService01
 * @tc.desc: Test PowerMgrService service ready.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService001, TestSize.Level0)
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(sam != nullptr) << "PowerMgrService01 fail to get GetSystemAbilityManager";
    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(POWER_MANAGER_SERVICE_ID);
    ASSERT_TRUE(remoteObject_ != nullptr) << "GetSystemAbility failed.";
}

/**
 * @tc.name: PowerMgrService002
 * @tc.desc: Test PowerMgrService Start and stop.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService002, TestSize.Level0)
{
    if (false) {
        auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
        ASSERT_TRUE(pmsTest_ != nullptr) << "PowerMgrService02 fail to get PowerMgrService";
        ASSERT_TRUE(!pmsTest_->IsServiceReady()) << "SetUpTestCase pmsTest_ is ready";
        pmsTest_->OnStart();
        ASSERT_TRUE(pmsTest_->IsServiceReady()) << "SetUpTestCase pmsTest_ is not ready";
        pmsTest_->OnStop();
        ASSERT_TRUE(!pmsTest_->IsServiceReady()) << "SetUpTestCase pmsTest_ stop fail";
        pmsTest_->OnStart();
        ASSERT_TRUE(pmsTest_->IsServiceReady()) << "SetUpTestCase pmsTest_ is not ready";
        pmsTest_->OnStop();
        ASSERT_TRUE(!pmsTest_->IsServiceReady()) << "SetUpTestCase pmsTest_ stop fail";
    }
}

/**
 * @tc.name: PowerMgrService003
 * @tc.desc: Test overrideScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService003, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_TRUE(powerMgrClient.OverrideScreenOffTime(1000));
    sleep(2);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrService003: Prepare Fail, Screen is ON.";
    EXPECT_TRUE(powerMgrClient.RestoreScreenOffTime());
    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService003 end.");
}

/**
 * @tc.name: PowerMgrService004
 * @tc.desc: Test overrideScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService004, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(0));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService004: Prepare Fail, Screen is ON.";

    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService004 end.");
}

/**
 * @tc.name: PowerMgrService005
 * @tc.desc: Test overrideScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService005, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(-1));
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService005: Prepare Fail, Screen is OFF.";
    
    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService005 end.");
}

/**
 * @tc.name: PowerMgrService006
 * @tc.desc: Test overrideScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService006, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_TRUE(powerMgrClient.OverrideScreenOffTime(1000));
    powerMgrClient.WakeupDevice();
    sleep(2);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false) << "PowerMgrService006: Prepare Fail, Screen is ON.";
    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService006 end.");
}

/**
 * @tc.name: PowerMgrService007
 * @tc.desc: Test overrideScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService007, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(0));
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService007: Prepare Fail, Screen is ON.";

    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService007 end.");
}

/**
 * @tc.name: PowerMgrService008
 * @tc.desc: Test overrideScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService008, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(-1));
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService008: Prepare Fail, Screen is OFF.";
    
    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService008 end.");
}

/**
 * @tc.name: PowerMgrService009
 * @tc.desc: Test restoreScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService009, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_TRUE(powerMgrClient.OverrideScreenOffTime(1000));
    EXPECT_TRUE(powerMgrClient.RestoreScreenOffTime());
    sleep(2);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService009: Prepare Fail, Screen is OFF.";
    
    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService009 end.");
}

/**
 * @tc.name: PowerMgrService010
 * @tc.desc: Test restoreScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService010, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(0));
    EXPECT_FALSE(powerMgrClient.RestoreScreenOffTime());
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService010: Prepare Fail, Screen is OFF.";
    
    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService010 end.");
}

/**
 * @tc.name: PowerMgrService011
 * @tc.desc: Test restoreScreenOffTime in screenon
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService011, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.WakeupDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(-1));;
    EXPECT_FALSE(powerMgrClient.RestoreScreenOffTime());
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService011: Prepare Fail, Screen is OFF.";
    
    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService011 end.");
}

/**
 * @tc.name: PowerMgrService012
 * @tc.desc: Test restoreScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService012, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_TRUE(powerMgrClient.OverrideScreenOffTime(1000));
    EXPECT_TRUE(powerMgrClient.RestoreScreenOffTime());
    powerMgrClient.WakeupDevice();
    sleep(2);
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService012: Prepare Fail, Screen is OFF.";
    
    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService012 end.");
}

/**
 * @tc.name: PowerMgrService013
 * @tc.desc: Test restoreScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService013, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(0));
    EXPECT_FALSE(powerMgrClient.RestoreScreenOffTime());
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService013: Prepare Fail, Screen is OFF.";
    
    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService013 end.");
}

/**
 * @tc.name: PowerMgrService014
 * @tc.desc: Test restoreScreenOffTime in screenoff
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService014, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_FALSE(powerMgrClient.OverrideScreenOffTime(-1));
    EXPECT_FALSE(powerMgrClient.RestoreScreenOffTime());
    powerMgrClient.WakeupDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true) << "PowerMgrService014: Prepare Fail, Screen is OFF.";
    
    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService014 end.");
}

/**
 * @tc.name: PowerMgrService015
 * @tc.desc: Test Dump
 * @tc.type: FUNC
 * @tc.require: issueI650CX
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService015, TestSize.Level2)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::vector<std::string> dumpArgs {};
    std::string expectedDebugInfo = "Power manager dump options";
    std::string actualDebugInfo = powerMgrClient.Dump(dumpArgs);
    auto index = actualDebugInfo.find(expectedDebugInfo);
    EXPECT_TRUE(index != string::npos);
}

/**
 * @tc.name: PowerMgrService016
 * @tc.desc: Test IsStandby
 * @tc.type: FUNC
 * @tc.require: issueI7QHBE
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService016, TestSize.Level2)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    bool standby = false;
    EXPECT_EQ(powerMgrClient.IsStandby(standby), PowerErrors::ERR_OK);
    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService016 end.");
}

/**
 * @tc.name: PowerMgrService017
 * @tc.desc: Test QueryRunningLockLists
 * @tc.type: FUNC
 * @tc.require: issueI8FCZA
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService017, TestSize.Level2)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::map<std::string, RunningLockInfo> runningLockLists;
    bool ret = powerMgrClient.QueryRunningLockLists(runningLockLists);
    EXPECT_EQ(ret, true);
    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService017 end.");
}

/**
 * @tc.name: PowerMgrService018
 * @tc.desc: Test RunningLock Deconstructor unlock
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService018, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock1 = powerMgrClient.CreateRunningLock("runninglock1", RunningLockType::RUNNINGLOCK_SCREEN);
    ASSERT_TRUE(runningLock1 != nullptr);
    runningLock1->Lock();
    ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
    runningLock1->UnLock();
    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService018 end.");
}

/**
 * @tc.name: PowerMgrService019
 * @tc.desc: Test Pre-light the screen.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService019, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);
    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright_auth_success");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);

    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService019 end.");
}

/**
 * @tc.name: PowerMgrService020
 * @tc.desc: Test if the authentication fails, the screen is on.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService020, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);
    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright_auth_fail_screen_on");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), true);

    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService020 end.");
}

/**
 * @tc.name: PowerMgrService021
 * @tc.desc: Test if the authentication fails, the screen is off.
 * @tc.type: FUNC
 */
HWTEST_F (PowerMgrServiceTest, PowerMgrService021, TestSize.Level0)
{
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    powerMgrClient.SuspendDevice();
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);

    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);
    powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, "pre_bright_auth_fail_screen_off");
    EXPECT_EQ(powerMgrClient.IsScreenOn(), false);

    POWER_HILOGD(LABEL_TEST, "PowerMgrServiceTest::PowerMgrService021 end.");
}

/**
 * @tc.name: PowerMgrService022
 * @tc.desc: Test PowerMgrService LockScreenAfterTimingOut.
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceTest, PowerMgrService022, TestSize.Level0)
{
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest_ != nullptr) << "PowerMgrService022 fail to get PowerMgrService";
    pmsTest_->OnStart();
    auto stateMaschine_ = pmsTest_->GetPowerStateMachine();
    auto runningLockMgr = pmsTest_->GetRunningLockMgr();

    sptr<IPowerMgr> ptr;
    ptr.ForceSetRefPtr(static_cast<IPowerMgr*>(pmsTest_.GetRefPtr()));
    pmsTest_.GetRefPtr()->IncStrongRef(pmsTest_.GetRefPtr());
    RunningLock runninglock1(ptr, "runninglock_screen_on", RunningLockType::RUNNINGLOCK_SCREEN);
    runninglock1.Init();
    runninglock1.Lock();

    EXPECT_EQ(stateMaschine_->GetReasionBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    stateMaschine_->LockScreenAfterTimingOut(true, false);
    EXPECT_EQ(stateMaschine_->GetReasionBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    stateMaschine_->LockScreenAfterTimingOut(false, false);
    EXPECT_EQ(stateMaschine_->GetReasionBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK);
    EXPECT_EQ(stateMaschine_->GetReasionBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_POWER_KEY),
        StateChangeReason::STATE_CHANGE_REASON_HARD_KEY);
    stateMaschine_->LockScreenAfterTimingOut(true, true);
    EXPECT_EQ(stateMaschine_->GetReasionBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT_NO_SCREEN_LOCK);
    runninglock1.UnLock();
    EXPECT_EQ(stateMaschine_->GetReasionBySuspendType(SuspendDeviceType::SUSPEND_DEVICE_REASON_TIMEOUT),
        StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
}
}
