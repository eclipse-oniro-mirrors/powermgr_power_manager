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

#include "power_mgr_service_native_test.h"

#ifdef THERMAL_GTEST
#define private   public
#define protected public
#endif

#include <csignal>
#include <iostream>

#ifdef HAS_MULTIMODALINPUT_INPUT_PART
#include "key_event.h"
#include "pointer_event.h"
#endif
#include "permission.h"
#include "power_common.h"
#include "power_mgr_service.h"
#include "sync_hibernate_callback_stub.h"
#include "ulsr_callback_stub.h"
#include "setting_helper.h"
#include "shutdown/async_shutdown_callback_stub.h"
#include "running_lock_changed_callback_stub.h"
#include "death_recipient_manager.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
constexpr int32_t TIMEOUTMS = 7;
constexpr int64_t CALLTIMEMS = 1;
constexpr int64_t SUSCALLTIMEMS = 3;
constexpr pid_t PID = 1;
constexpr pid_t UID = 1;
constexpr int32_t UNCANCELID = -1;
constexpr int32_t INVALID_CODE = -1;
constexpr int32_t TRY_TIMES = 2;
sptr<PowerMgrService> g_pmsTest;
bool g_isSystem = true;
bool g_isPermissionGranted = true;
} // namespace

bool Permission::IsSystem()
{
    return g_isSystem;
}

bool Permission::IsPermissionGranted(const std::string& perm)
{
    return g_isPermissionGranted;
}

void PowerMgrServiceNativeTest::SetUp()
{
    g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(g_pmsTest != nullptr) << "PowerMgrService02 fail to get PowerMgrService";
    g_pmsTest->OnStart();

    auto stateMachine = std::make_shared<PowerStateMachine>(g_pmsTest);
    EXPECT_TRUE(stateMachine->Init());
    UserActivityType userActivityType = UserActivityType::USER_ACTIVITY_TYPE_ACCESSIBILITY;
    stateMachine->RefreshActivityInner(PID, CALLTIMEMS, userActivityType, true);
    auto runningLockMgr = std::make_shared<RunningLockMgr>(g_pmsTest);
    EXPECT_TRUE(runningLockMgr->Init());
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    RunningLockParam runningLockParam {0,
        "runninglockNativeTest1", "", RunningLockType::RUNNINGLOCK_SCREEN, TIMEOUTMS, PID, UID};
    EXPECT_TRUE(runningLockMgr->CreateRunningLock(remoteObj, runningLockParam) != nullptr);
    runningLockMgr->Lock(remoteObj);
    runningLockMgr->UnLock(remoteObj);
    EXPECT_FALSE(runningLockMgr->ReleaseLock(remoteObj));
}

void PowerMgrServiceNativeTest::TearDown()
{
    g_pmsTest->OnStop();
    auto stateMachine = g_pmsTest->GetPowerStateMachine();
    if (stateMachine) {
        if (stateMachine->ffrtTimer_) {
            stateMachine->ffrtTimer_->CancelAllTimer();
        }
        ffrt::wait();
    }
}

void PowerStateTestCallback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGI(
        LABEL_TEST, "PowerStateTestCallback::OnPowerStateChanged state = %{public}u.", static_cast<uint32_t>(state));
}

namespace {
/**
 * @tc.name: PowerMgrServiceNative001
 * @tc.desc: test init in powerMgrService
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative001 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative001 function start!");
    g_pmsTest->HallSensorSubscriberInit();
    g_pmsTest->HallSensorSubscriberCancel();
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
    int32_t keyCode = OHOS::MMI::KeyEvent::KEYCODE_F1;
    g_pmsTest->HandleKeyEvent(keyCode);
    int32_t type = OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE;
    g_pmsTest->HandlePointEvent(type);

    EXPECT_EQ(g_pmsTest->OverrideScreenOffTime(TIMEOUTMS), PowerErrors::ERR_OK);
    EXPECT_EQ(g_pmsTest->RestoreScreenOffTime(), PowerErrors::ERR_OK);

    g_pmsTest->doubleClickId_ = UNCANCELID;
    g_pmsTest->monitorId_ = UNCANCELID;
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative001 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative001 end.";
}

/**
 * @tc.name: PowerMgrServiceNative002
 * @tc.desc: test handleKeyEvent in powerMgrService
 * @tc.type: FUNC
 */
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative002 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative002 function start!");
    int32_t keyCode = OHOS::MMI::KeyEvent::KEYCODE_F1;
    g_pmsTest->HandleKeyEvent(keyCode);
    keyCode = OHOS::MMI::KeyEvent::KEYCODE_F2;
    g_pmsTest->HandleKeyEvent(keyCode);
    keyCode = OHOS::MMI::KeyEvent::KEYCODE_VIRTUAL_MULTITASK;
    g_pmsTest->HandleKeyEvent(keyCode);
    keyCode = OHOS::MMI::KeyEvent::KEYCODE_WEAR_1;
    g_pmsTest->HandleKeyEvent(keyCode);
    int32_t type = OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE;
    g_pmsTest->HandlePointEvent(type);
    type = OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    g_pmsTest->HandlePointEvent(type);

    EXPECT_TRUE(g_pmsTest->ShutDownDevice(SHUTDOWN_FAST_REASON) == PowerErrors::ERR_OK);
    SuspendDeviceType reasonSDT = SuspendDeviceType::SUSPEND_DEVICE_REASON_DEVICE_ADMIN;
    EXPECT_TRUE(g_pmsTest->SuspendDevice(SUSCALLTIMEMS, reasonSDT, false) == PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative002 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative002 end.";
}
#endif

/**
 * @tc.name: PowerMgrServiceNative003
 * @tc.desc: test pickup register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative003 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative003 function start!");
#ifdef POWER_PICKUP_ENABLE
    g_pmsTest->RegisterSettingWakeupPickupGestureObserver();
    g_pmsTest->RegisterSettingWakeupPickupGestureObserver();
    EXPECT_TRUE(SettingHelper::pickUpObserver_ != nullptr);
    SettingHelper::UnregisterSettingWakeupPickupObserver();
    SettingHelper::UnregisterSettingWakeupPickupObserver();
    EXPECT_TRUE(SettingHelper::pickUpObserver_ == nullptr);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative003 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative003 end.";
}

/**
 * @tc.name: PowerMgrServiceNative004
 * @tc.desc: test autobrightness register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative004 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative004 function start!");
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    EXPECT_TRUE(powerModeModuleTest != nullptr);
    auto flag = SettingHelper::IsAutoAdjustBrightnessSettingValid();
    powerModeModuleTest->RegisterAutoAdjustBrightnessObserver();
    SettingHelper::SetSettingAutoAdjustBrightness(SettingHelper::SwitchStatus::INVALID);
    auto ret = SettingHelper::GetSettingAutoAdjustBrightness(INVALID_CODE);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative004 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative004 end.";
}

/**
 * @tc.name: PowerMgrServiceNative005
 * @tc.desc: test brightness register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative005 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative005 function start!");
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    EXPECT_TRUE(powerModeModuleTest != nullptr);
    powerModeModuleTest->RegisterAutoAdjustBrightnessObserver();
    auto flag = SettingHelper::IsBrightnessSettingValid();
    SettingHelper::SetSettingBrightness(INVALID_CODE);
    auto ret = SettingHelper::GetSettingAutoAdjustBrightness(INVALID_CODE);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative005 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative005 end.";
}

/**
 * @tc.name: PowerMgrServiceNative006
 * @tc.desc: test vibrateState register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative006 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative006 function start!");
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    EXPECT_TRUE(powerModeModuleTest != nullptr);
    powerModeModuleTest->RegisterVibrateStateObserver();
    auto flag = SettingHelper::IsVibrationSettingValid();
    SettingHelper::SetSettingVibration(SettingHelper::SwitchStatus::INVALID);
    auto ret = SettingHelper::GetSettingVibration(INVALID_CODE);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative006 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative006 end.";
}


/**
 * @tc.name: PowerMgrServiceNative007
 * @tc.desc: test autoWindowRotation register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative007 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative007 function start!");
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    EXPECT_TRUE(powerModeModuleTest != nullptr);
    powerModeModuleTest->RegisterAutoWindowRotationObserver();
    auto flag = SettingHelper::IsWindowRotationSettingValid();
    SettingHelper::SetSettingWindowRotation(SettingHelper::SwitchStatus::INVALID);
    auto ret = SettingHelper::GetSettingWindowRotation(INVALID_CODE);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative007 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative007 end.";
}

/**
 * @tc.name: PowerMgrServiceNative008
 * @tc.desc: test intellVoiceSetting register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative008 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative008 function start!");
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    EXPECT_TRUE(powerModeModuleTest != nullptr);
    powerModeModuleTest->RegisterIntellVoiceObserver();
    auto flag = SettingHelper::IsIntellVoiceSettingValid();
    SettingHelper::SetSettingIntellVoice(SettingHelper::SwitchStatus::INVALID);
    auto ret = SettingHelper::GetSettingIntellVoice(INVALID_CODE);
    powerModeModuleTest->UnregisterSaveModeObserver();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative008 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative008 end.";
}

/**
 * @tc.name: PowerMgrServiceNative009
 * @tc.desc: test settingPowerMode register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative009 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative009 function start!");
    g_pmsTest->RegisterSettingPowerModeObservers();
    g_pmsTest->RegisterSettingPowerModeObservers();
    EXPECT_TRUE(SettingHelper::powerModeObserver_ != nullptr);
    SettingHelper::SaveCurrentMode(INVALID_CODE);
    SettingHelper::UnRegisterSettingPowerModeObserver();
    EXPECT_TRUE(SettingHelper::powerModeObserver_ == nullptr);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative009 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative009 end.";
}

/**
 * @tc.name: PowerMgrServiceNative010
 * @tc.desc: test settingWakeUpLid register
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative010 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative010 function start!");
    g_pmsTest->RegisterSettingWakeUpLidObserver();
    SettingHelper::SetSettingWakeupLid(true);
    bool ret = SettingHelper::GetSettingWakeupLid();
    SettingHelper::UnRegisterSettingWakeupLidObserver();
    EXPECT_TRUE(SettingHelper::lidObserver_ == nullptr);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative010 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative010 end.";
}

/**
 * @tc.name: PowerMgrServiceNative011
 * @tc.desc: test RegisterSettingWakeupDoubleClickObservers
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative011 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative011 function start!");
#ifdef POWER_DOUBLECLICK_ENABLE
    g_pmsTest->RegisterSettingWakeupDoubleClickObservers();
    g_pmsTest->RegisterSettingWakeupDoubleClickObservers();
    SettingHelper::IsWakeupDoubleSettingValid();
    SettingHelper::UnregisterSettingWakeupDoubleObserver();
    SettingHelper::UnregisterSettingWakeupDoubleObserver();
    EXPECT_TRUE(SettingHelper::doubleClickObserver_ == nullptr);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative011 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative011 end.";
}

/**
 * @tc.name: PowerMgrServiceNative012
 * @tc.desc: test RegisterMovementCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative012 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative012 function start!");
#ifdef MSDP_MOVEMENT_ENABLE
    auto stateMachine = std::make_shared<PowerStateMachine>(g_pmsTest);
    g_pmsTest->RegisterMovementCallback();
    g_pmsTest->UnRegisterMovementCallback();
    g_pmsTest->ResetMovementState();
    bool ret =  stateMachine->IsMovementStateOn();
    EXPECT_TRUE(ret == false);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative012 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative012 end.";
}

/**
 * @tc.name: PowerMgrServiceNative013
 * @tc.desc: test RegisterExternalScreenListener
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative013 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative013 function start!");
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    g_pmsTest->RegisterExternalScreenListener();
    EXPECT_TRUE(g_pmsTest->externalScreenListener_ != nullptr);
    g_pmsTest->UnRegisterExternalScreenListener();
    EXPECT_TRUE(g_pmsTest->externalScreenListener_ == nullptr);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative013 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative013 end.";
}

/**
 * @tc.name: PowerMgrServiceNative014
 * @tc.desc: test ExternalScreenListener OnConnect
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative014 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative014 function start!");
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    g_pmsTest->SuspendControllerInit();
    g_pmsTest->WakeupControllerInit();
    // Register screen listener
    g_pmsTest->RegisterExternalScreenListener();
    EXPECT_TRUE(g_pmsTest->externalScreenListener_ != nullptr);

    auto powerStateMachine = g_pmsTest->GetPowerStateMachine();
    constexpr uint64_t SCREEN_A_ID = 10001;
    // Mock open switch to wakeup deivce
    // case 1: isSwitchOpen:true, isScreenOn:true
    powerStateMachine->SetSwitchState(true);
    g_pmsTest->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_SWITCH, "PowerMgrServiceNative014");
    EXPECT_TRUE(powerStateMachine->IsScreenOn());
    g_pmsTest->externalScreenListener_->OnConnect(SCREEN_A_ID);
    EXPECT_TRUE(powerStateMachine->IsScreenOn());
    powerStateMachine->SetExternalScreenNumber(0);

    // case 2: isSwitchOpen:true, isScreenOn:false
    g_pmsTest->SuspendDevice(GetTickCount(), SuspendDevice::SUSPEND_DEVICE_REASON_POWER_KEY, false);
    EXPECT_FALSE(powerStateMachine->IsScreenOn());
    g_pmsTest->externalScreenListener_->OnConnect(SCREEN_A_ID);
    EXPECT_TRUE(powerStateMachine->IsScreenOn());
    powerStateMachine->SetExternalScreenNumber(0);

    // case 3: isSwitchOpen:false, isScreenOn:false
    powerStateMachine->SetSwitchState(false);
    g_pmsTest->SuspendDevice(GetTickCount(), SuspendDevice::SUSPEND_DEVICE_REASON_SWITCH, false);
    EXPECT_FALSE(powerStateMachine->IsScreenOn());
    g_pmsTest->externalScreenListener_->OnConnect(SCREEN_A_ID);
    EXPECT_FALSE(powerStateMachine->IsScreenOn());
    powerStateMachine->SetExternalScreenNumber(0);

    // Unregister screen listener
    g_pmsTest->UnRegisterExternalScreenListener();
    EXPECT_TRUE(g_pmsTest->externalScreenListener_ == nullptr);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative014 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative014 end.";
}

/**
 * @tc.name: PowerMgrServiceNative015
 * @tc.desc: test ExternalScreenListener OnDisconnect
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative015 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative015 function start!");
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    g_pmsTest->SuspendControllerInit();
    g_pmsTest->WakeupControllerInit();
    // Register screen listener
    g_pmsTest->RegisterExternalScreenListener();
    EXPECT_TRUE(g_pmsTest->externalScreenListener_ != nullptr);

    auto powerStateMachine = g_pmsTest->GetPowerStateMachine();
    constexpr uint64_t SCREEN_A_ID = 10001;
    // Mock open switch to wakeup deivce
    // case 1: isSwitchOpen:true, isScreenOn:true
    powerStateMachine->SetSwitchState(true);
    g_pmsTest->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_SWITCH, "PowerMgrServiceNative015");
    powerStateMachine->SetExternalScreenNumber(1);
    EXPECT_TRUE(powerStateMachine->IsScreenOn());
    g_pmsTest->externalScreenListener_->OnDisconnect(SCREEN_A_ID);
    EXPECT_TRUE(powerStateMachine->IsScreenOn());

    // case 2: isSwitchOpen:true, isScreenOn:false
    g_pmsTest->SuspendDevice(GetTickCount(), SuspendDevice::SUSPEND_DEVICE_REASON_POWER_KEY, false);
    EXPECT_FALSE(powerStateMachine->IsScreenOn());
    powerStateMachine->SetExternalScreenNumber(1);
    g_pmsTest->externalScreenListener_->OnDisconnect(SCREEN_A_ID);
    EXPECT_FALSE(powerStateMachine->IsScreenOn());

    // case 3: isSwitchOpen:false, isScreenOn:true, and there's only 1 external screen
    // Mock open switch to wakeup deivce
    powerStateMachine->SetSwitchState(true);
    g_service->WakeupDevice(
        static_cast<int64_t>(time(nullptr)), WakeupDeviceType::WAKEUP_DEVICE_SWITCH, "PowerMgrServiceNative015");
    EXPECT_TRUE(powerStateMachine->IsScreenOn());
    powerStateMachine->SetExternalScreenNumber(1);
    // Mock close switch to suspend device when switch's action is configured as ACTION_NONE
    powerStateMachine->SetSwitchState(false);
    auto suspendController = g_service->GetSuspendController();
    suspendController->ControlListener(SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH, SuspendAction::ACTION_NONE, 0);
    EXPECT_TRUE(powerStateMachine->IsScreenOn());
    g_pmsTest->externalScreenListener_->OnDisconnect(SCREEN_A_ID);
    EXPECT_FALSE(powerStateMachine->IsScreenOn());

    // Unregister screen listener
    g_pmsTest->UnRegisterExternalScreenListener();
    EXPECT_TRUE(g_pmsTest->externalScreenListener_ == nullptr);
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative015 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative015 end.";
}

/**
 * @tc.name: PowerMgrServiceNative016
 * @tc.desc: test RegisterSettingDuringCallObservers
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative016, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative016 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative016 function start!");
    g_pmsTest->isDuringCallStateEnable_ = true;
    EXPECT_TRUE(g_pmsTest->IsDuringCallStateEnable());
    g_pmsTest->RegisterSettingDuringCallObservers();
    EXPECT_TRUE(SettingHelper::duringCallObserver_ != nullptr);
    g_pmsTest->RegisterSettingDuringCallObservers();
    EXPECT_TRUE(SettingHelper::duringCallObserver_ != nullptr);
    SettingHelper::UnRegisterSettingDuringCallObserver();
    EXPECT_TRUE(SettingHelper::duringCallObserver_ == nullptr);
    SettingHelper::UnRegisterSettingDuringCallObserver();
    EXPECT_TRUE(SettingHelper::duringCallObserver_ == nullptr);
    g_pmsTest->DuringCallSettingUpdateFunc(SettingHelper::SETTING_DURING_CALL_STATE_KEY);
    auto stateMachine = g_pmsTest->GetPowerStateMachine();
    EXPECT_TRUE(stateMachine != nullptr && !stateMachine->isDuringCall_);
    g_pmsTest->powerStateMachine_ = nullptr;
    EXPECT_TRUE(g_pmsTest->GetPowerStateMachine() == nullptr);
    g_pmsTest->DuringCallSettingUpdateFunc(SettingHelper::SETTING_DURING_CALL_STATE_KEY);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative016 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative016 end.";
}

/**
 * @tc.name: PowerMgrServiceNative017
 * @tc.desc: test RegisterSettingDuringCallObservers
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative017, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerMgrServiceNative017 start.";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative017 function start!");
    g_pmsTest->isDuringCallStateEnable_ = false;
    EXPECT_FALSE(g_pmsTest->IsDuringCallStateEnable());
    g_pmsTest->RegisterSettingDuringCallObservers();
    EXPECT_TRUE(SettingHelper::duringCallObserver_ == nullptr);
    SettingHelper::UnRegisterSettingDuringCallObserver();
    EXPECT_TRUE(SettingHelper::duringCallObserver_ == nullptr);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative017 function end!");
    GTEST_LOG_(INFO) << "PowerMgrServiceNative017 end.";
}

/**
 * @tc.name: PowerMgrServiceNative018
 * @tc.desc: Test GetShutdownReason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative018, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative018 function start!");
    auto g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(g_pmsTest != nullptr);
    g_pmsTest->OnStart();
    std::string setReasonFirst = "reasonfirst";
    std::string setReasonSecond =
        "reasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirst";
    g_isSystem = true;
    PowerErrors ret = g_pmsTest->GetShutdownReason(setReasonFirst);
    EXPECT_TRUE(ret == PowerErrors::ERR_OK);
    ret = g_pmsTest->GetShutdownReason(setReasonSecond);
    EXPECT_TRUE(ret == PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative018 function end!");
}

/**
 * @tc.name: PowerMgrServiceNative019
 * @tc.desc: Test GetShutdownReason
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative019, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative019 function start!");
    auto g_pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(g_pmsTest != nullptr);
    g_pmsTest->OnStart();
    std::string setReasonFirst = "reasonfirst";
    std::string setReasonSecond =
        "reasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirstreasonfirst";
    g_isSystem = false;
    PowerErrors ret = g_pmsTest->GetShutdownReason(setReasonFirst);
    EXPECT_TRUE(ret != PowerErrors::ERR_PERMISSION_DENIED);
    ret = g_pmsTest->GetShutdownReason(setReasonSecond);
    EXPECT_TRUE(ret != PowerErrors::ERR_PERMISSION_DENIED);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative019 function end!");
}

class UlsrCallbackTest : public UlsrCallbackStub {
public:
    UlsrCallbackTest() = default;
    virtual ~UlsrCallbackTest() = default;

    void OnSyncUlsr() override
    {
        POWER_HILOGI(LABEL_TEST, "UlsrCallbackTest OnSyncUlsr!");
    }
    void OnAsyncWakeup(bool ulsrResult) override
    {
        POWER_HILOGI(LABEL_TEST, "UlsrCallbackTest OnAsyncWakeup!");
    }
};

class TestAsyncShutdownCallback : public AsyncShutdownCallbackStub {
public:
    TestAsyncShutdownCallback() = default;
    virtual ~TestAsyncShutdownCallback() = default;

    void OnAsyncShutdownOrReboot(bool isReboot) override
    {
        POWER_HILOGI(LABEL_TEST, "TestAsyncShutdownCallback OnAsyncShutdownOrReboot");
    }
};

/**
 * @tc.name: PowerMgrServiceNative020
 * @tc.desc: Test RegisterUlsrCallback && UnRegisterUlsrCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative020, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative020 start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    PowerErrors ret;
    
    // Test case 1: No system permission
    g_isSystem = false;
    sptr<UlsrCallbackTest> callback = new UlsrCallbackTest();
    ret = pmsTest_->RegisterUlsrCallback(callback, UlsrPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED) << "Test case 1 failed";
    ret = pmsTest_->UnRegisterUlsrCallback(callback);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED) << "Test case 1 failed";

    // Test case 2: DEFAULT
#ifdef POWER_MANAGER_ENABLE_SUSPEND_WITH_TAG
    g_isSystem = true;
    ret = pmsTest_->RegisterUlsrCallback(callback, UlsrPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 2 failed";
    ret = pmsTest_->UnRegisterUlsrCallback(callback);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 2 failed";
#else
    g_isSystem = true;
    ret = pmsTest_->RegisterUlsrCallback(callback, UlsrPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_PARAM_INVALID) << "Test case 2 failed";
    ret = pmsTest_->UnRegisterUlsrCallback(callback);
    EXPECT_EQ(ret, PowerErrors::ERR_PARAM_INVALID) << "Test case 2 failed";
#endif
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative020 end!");
}

#ifdef POWER_MANAGER_ENABLE_SUSPEND_WITH_TAG
/**
 * @tc.name: PowerMgrServiceNative021
 * @tc.desc: Test TriggerUlsrWakeupCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative021, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative021 start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    PowerErrors ret;
    g_isSystem = true;
    ret = pmsTest_->RegisterUlsrCallback(nullptr, UlsrPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 2 failed";
    sptr<UlsrCallbackTest> callback = new UlsrCallbackTest();
    ret = pmsTest_->RegisterUlsrCallback(callback, UlsrPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 2 failed";
    EXPECT_TRUE(pmsTest_->ulsrCallbackHolder_ != nullptr);
    pmsTest_->TriggerUlsrWakeupCallback(true);
    ret = pmsTest_->UnRegisterUlsrCallback(callback);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 2 failed";
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative021 end!");
}
#endif

/**
 * @tc.name: PowerMgrServiceNative022
 * @tc.desc: Test RegisterAsyncShutdownCallback && UnRegisterAsyncShutdownCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative022, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative022 start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    PowerErrors ret;

    // Test case 0: nullptr
    ret = pmsTest_->RegisterAsyncShutdownCallback(nullptr, ShutdownPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_PARAM_INVALID) << "Test case 0 failed";
    ret = pmsTest_->UnRegisterAsyncShutdownCallback(nullptr);
    EXPECT_EQ(ret, PowerErrors::ERR_PARAM_INVALID) << "Test case 0 failed";

    // Test case 1: No system permission
    g_isSystem = false;
    g_isPermissionGranted = true;
    sptr<IAsyncShutdownCallback> callback0 = new TestAsyncShutdownCallback();
    ret = pmsTest_->RegisterAsyncShutdownCallback(callback0, ShutdownPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED) << "Test case 1 failed";
    ret = pmsTest_->UnRegisterAsyncShutdownCallback(callback0);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED) << "Test case 1 failed";

    // Test case 2: No POWER_MANAGER permission
    g_isSystem = true;
    g_isPermissionGranted = false;
    ret = pmsTest_->RegisterAsyncShutdownCallback(callback0, ShutdownPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_PERMISSION_DENIED) << "Test case 2 failed";
    ret = pmsTest_->UnRegisterAsyncShutdownCallback(callback0);
    EXPECT_EQ(ret, PowerErrors::ERR_PERMISSION_DENIED) << "Test case 2 failed";

    // Test case 3: DEFAULT
    g_isSystem = true;
    g_isPermissionGranted = true;
    ret = pmsTest_->RegisterAsyncShutdownCallback(callback0, ShutdownPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 3 failed";
    ret = pmsTest_->UnRegisterAsyncShutdownCallback(callback0);
    EXPECT_EQ(ret, PowerErrors::ERR_OK) << "Test case 3 failed";

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative022 end!");
}

/**
 * @tc.name: PowerMgrServiceNative023
 * @tc.desc: Test SetProxFilteringStrategy
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative023, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative023 start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    // Test case 0: No system permission
    g_isSystem = false;
    pmsTest->SetProxFilteringStrategy(ProxFilteringStrategy::FILTERING_CLOSE, nullptr);
    auto stateMachine = pmsTest->GetPowerStateMachine();
    bool flag = stateMachine->isProximityCloseEventFiltered_;
    EXPECT_FALSE(flag);
    // Test case 1: DEFAULT - ProxFilteringStrategy::FILTERING_CLOSE
    g_isSystem = true;
    pmsTest->SetProxFilteringStrategy(ProxFilteringStrategy::FILTERING_CLOSE, nullptr);
    flag = stateMachine->isProximityCloseEventFiltered_;
#ifdef POWER_MANAGER_SUPPORT_FILTERING_PROXIMITY_EVENT
    stateMachine->HandleProximityClose();
    EXPECT_TRUE(flag);
#else
    EXPECT_FALSE(flag);
#endif
    // Test case 2: DEFAULT - ProxFilteringStrategy::NOT_FILTERING
    pmsTest->SetProxFilteringStrategy(ProxFilteringStrategy::NOT_FILTERING, nullptr);
    flag = stateMachine->isProximityCloseEventFiltered_;
    EXPECT_FALSE(flag);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative023 end!");
}

/**
 * @tc.name: PowerMgrServiceNative024
 * @tc.desc: Test SetProxFilteringStrategy OnRemoteDied Callback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative024, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative024 start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_isSystem = true;
    sptr<IRemoteObject> obj = sptr<IPCObjectProxy>::MakeSptr(0, u"PowerMgrServiceNative024");
    pmsTest->SetProxFilteringStrategy(ProxFilteringStrategy::FILTERING_CLOSE, obj);
    auto stateMachine = pmsTest->GetPowerStateMachine();
    bool flag = stateMachine->isProximityCloseEventFiltered_;
#ifdef POWER_MANAGER_SUPPORT_FILTERING_PROXIMITY_EVENT
    stateMachine->HandleProximityClose();
    EXPECT_TRUE(flag);
#else
    EXPECT_FALSE(flag);
#endif
    wptr<IRemoteObject> objWeak = obj;
    DeathRecipientManager::GetInstance().OnRemoteDied(objWeak);
    flag = stateMachine->isProximityCloseEventFiltered_;
    EXPECT_FALSE(flag);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative024 end!");
}

/**
 * @tc.name: ScreenOffBlockService001
 * @tc.desc: Register a screen-off block for APPLICATION via the real
 *           PowerMgrService::SetInterfaceCallFilteringStrategy entry, then call the real
 *           PowerMgrService::SuspendDevice entry and assert it returns
 *           ERR_SCREEN_OFF_BLOCKED (4900701). After cancelling the block SuspendDevice resumes normally.
 * @tc.type: FUNC
 * @tc.require: issueI7ZB4
 */
HWTEST_F(PowerMgrServiceNativeTest, ScreenOffBlockService001, TestSize.Level1) {
    POWER_HILOGI(LABEL_TEST, "ScreenOffBlockService001 start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_isSystem = true;
    g_isPermissionGranted = true;
    sptr<IRemoteObject> token = sptr<IPCObjectProxy>::MakeSptr(0, u"ScreenOffBlockService001");
    auto stateMachine = pmsTest->GetPowerStateMachine();
    ASSERT_TRUE(stateMachine != nullptr);
    EXPECT_FALSE(stateMachine->IsScreenOffBlocked(PowerStateMachine::ScreenOffBlockType::SUSPEND_DEVICE_INTERFACE));
    // Real entry: SetInterfaceCallFilteringStrategy writes the shared block map
    EXPECT_EQ(pmsTest->SetInterfaceCallFilteringStrategy(
        InterfaceCallFilteringStrategy::SUSPEND_DEVICE_FILTERING, token),
        PowerErrors::ERR_OK);
    EXPECT_TRUE(stateMachine->IsScreenOffBlocked(PowerStateMachine::ScreenOffBlockType::SUSPEND_DEVICE_INTERFACE));
    // Real entry: SuspendDevice must return ERR_SCREEN_OFF_BLOCKED before HDI suspend
    PowerErrors ret = pmsTest->SuspendDevice(
        CALLTIMEMS, SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, false);
    EXPECT_EQ(ret, PowerErrors::ERR_SCREEN_OFF_BLOCKED);
    EXPECT_EQ(static_cast<int32_t>(ret), 4900701);
    // Cancel via the real entry; afterwards SuspendDevice is no longer blocked
    EXPECT_EQ(pmsTest->SetInterfaceCallFilteringStrategy(
        InterfaceCallFilteringStrategy::SUSPEND_DEVICE_NOT_FILTERING, token),
        PowerErrors::ERR_OK);
    EXPECT_FALSE(stateMachine->IsScreenOffBlocked(PowerStateMachine::ScreenOffBlockType::SUSPEND_DEVICE_INTERFACE));
    POWER_HILOGI(LABEL_TEST, "ScreenOffBlockService001 end!");
}

/**
 * @tc.name: ScreenOffBlockService002
 * @tc.desc: Test SetInterfaceCallFilteringStrategy permission and parameter validation:
 *           no system permission, no POWER_MANAGER permission, and invalid strategy are rejected.
 * @tc.type: FUNC
 * @tc.require: issueI7ZB4
 */
HWTEST_F(PowerMgrServiceNativeTest, ScreenOffBlockService002, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "ScreenOffBlockService002 start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    sptr<IRemoteObject> token = sptr<IPCObjectProxy>::MakeSptr(0, u"ScreenOffBlockService002");
    // No system permission
    g_isSystem = false;
    g_isPermissionGranted = true;
    EXPECT_EQ(pmsTest->SetInterfaceCallFilteringStrategy(
        InterfaceCallFilteringStrategy::SUSPEND_DEVICE_FILTERING, token),
        PowerErrors::ERR_SYSTEM_API_DENIED);
    // System permission but no POWER_MANAGER permission
    g_isSystem = true;
    g_isPermissionGranted = false;
    EXPECT_EQ(pmsTest->SetInterfaceCallFilteringStrategy(
        InterfaceCallFilteringStrategy::SUSPEND_DEVICE_FILTERING, token),
        PowerErrors::ERR_PERMISSION_DENIED);
    // Invalid strategy (out of range)
    g_isSystem = true;
    g_isPermissionGranted = true;
    EXPECT_EQ(pmsTest->SetInterfaceCallFilteringStrategy(
        InterfaceCallFilteringStrategy::STRATEGY_MAX, token),
        PowerErrors::ERR_PARAM_INVALID);
    g_isPermissionGranted = true;
    POWER_HILOGI(LABEL_TEST, "ScreenOffBlockService002 end!");
}

/**
 * @tc.name: ScreenOffBlockService003
 * @tc.desc: Test the real PowerMgrService::SetPowerKeyFilteringStrategy entry with the new
 *           POWER_KEY_UP_SHORT_PRESS_FILTERING / POWER_KEY_UP_SHORT_PRESS_NOT_FILTERING
 *           enum values: registration blocks the POWER_KEY reason and the death recipient is wired.
 * @tc.type: FUNC
 * @tc.require: issueI7ZB4
 */
HWTEST_F(PowerMgrServiceNativeTest, ScreenOffBlockService003, TestSize.Level1) {
    POWER_HILOGI(LABEL_TEST, "ScreenOffBlockService003 start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_isSystem = true;
    g_isPermissionGranted = true;
    sptr<IRemoteObject> token = sptr<IPCObjectProxy>::MakeSptr(0, u"ScreenOffBlockService003");
    auto stateMachine = pmsTest->GetPowerStateMachine();
    ASSERT_TRUE(stateMachine != nullptr);
    EXPECT_FALSE(stateMachine->IsScreenOffBlocked(PowerStateMachine::ScreenOffBlockType::POWER_KEY));
    // Real entry: PowerMgrService::SetPowerKeyFilteringStrategy with FILTERING
    EXPECT_EQ(pmsTest->SetPowerKeyFilteringStrategy(
        PowerKeyFilteringStrategy::POWER_KEY_UP_SHORT_PRESS_FILTERING, token),
        PowerErrors::ERR_OK);
    EXPECT_TRUE(stateMachine->IsScreenOffBlocked(PowerStateMachine::ScreenOffBlockType::POWER_KEY));
    // Real entry: cancel via NOT_FILTERING
    EXPECT_EQ(pmsTest->SetPowerKeyFilteringStrategy(
        PowerKeyFilteringStrategy::POWER_KEY_UP_SHORT_PRESS_NOT_FILTERING, token),
        PowerErrors::ERR_OK);
    EXPECT_FALSE(stateMachine->IsScreenOffBlocked(PowerStateMachine::ScreenOffBlockType::POWER_KEY));
    wptr<IRemoteObject> tokenWeak = token;
    DeathRecipientManager::GetInstance().OnRemoteDied(tokenWeak);
    EXPECT_FALSE(stateMachine->IsScreenOffBlocked(PowerStateMachine::ScreenOffBlockType::POWER_KEY));
    POWER_HILOGI(LABEL_TEST, "ScreenOffBlockService003 end!");
}

/**
 * @tc.name: ScreenOffBlockService004
 * @tc.desc: Test SA-death auto-reset: after registering a screen-off block, trigger the real
 *           DeathRecipientManager::OnRemoteDied callback and assert the block is cleared.
 * @tc.type: FUNC
 * @tc.require: issueI7ZB4
 */
HWTEST_F(PowerMgrServiceNativeTest, ScreenOffBlockService004, TestSize.Level1) {
    POWER_HILOGI(LABEL_TEST, "ScreenOffBlockService004 start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_isSystem = true;
    g_isPermissionGranted = true;
    sptr<IRemoteObject> token = sptr<IPCObjectProxy>::MakeSptr(0, u"ScreenOffBlockService004");
    auto stateMachine = pmsTest->GetPowerStateMachine();
    ASSERT_TRUE(stateMachine != nullptr);
    EXPECT_EQ(pmsTest->SetInterfaceCallFilteringStrategy(
        InterfaceCallFilteringStrategy::SUSPEND_DEVICE_FILTERING, token),
        PowerErrors::ERR_OK);
    EXPECT_EQ(pmsTest->SetLidFilteringStrategy(
        LidFilteringStrategy::LID_CLOSE_FILTERING, token),
        PowerErrors::ERR_OK);
    EXPECT_TRUE(stateMachine->IsScreenOffBlocked(PowerStateMachine::ScreenOffBlockType::SUSPEND_DEVICE_INTERFACE));
    EXPECT_TRUE(stateMachine->IsScreenOffBlocked(PowerStateMachine::ScreenOffBlockType::LID));
    // Real entry: DeathRecipientManager::OnRemoteDied fires the token-level cleanup callback
    wptr<IRemoteObject> tokenWeak = token;
    DeathRecipientManager::GetInstance().OnRemoteDied(tokenWeak);
    EXPECT_FALSE(stateMachine->IsScreenOffBlocked(PowerStateMachine::ScreenOffBlockType::SUSPEND_DEVICE_INTERFACE));
    EXPECT_FALSE(stateMachine->IsScreenOffBlocked(PowerStateMachine::ScreenOffBlockType::LID));
    POWER_HILOGI(LABEL_TEST, "ScreenOffBlockService004 end!");
}

/**
 * @tc.name: ScreenOffBlockService005
 * @tc.desc: Register a screen-off block via SetInterfaceCallFilteringStrategy,
 *           then call ForceSuspendDevice and assert it returns
 *           ERR_SCREEN_OFF_BLOCKED. After cancelling the block ForceSuspendDevice resumes.
 * @tc.type: FUNC
 * @tc.require: issueI7ZB4
 */
HWTEST_F(PowerMgrServiceNativeTest, ScreenOffBlockService005, TestSize.Level1) {
    POWER_HILOGI(LABEL_TEST, "ScreenOffBlockService005 start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_isSystem = true;
    g_isPermissionGranted = true;
    sptr<IRemoteObject> token = sptr<IPCObjectProxy>::MakeSptr(0, u"ScreenOffBlockService005");
    auto stateMachine = pmsTest->GetPowerStateMachine();
    ASSERT_TRUE(stateMachine != nullptr);
    EXPECT_FALSE(stateMachine->IsScreenOffBlocked(
        PowerStateMachine::ScreenOffBlockType::SUSPEND_DEVICE_INTERFACE));
    // Register the block
    EXPECT_EQ(pmsTest->SetInterfaceCallFilteringStrategy(
        InterfaceCallFilteringStrategy::SUSPEND_DEVICE_FILTERING, token),
        PowerErrors::ERR_OK);
    EXPECT_TRUE(stateMachine->IsScreenOffBlocked(
        PowerStateMachine::ScreenOffBlockType::SUSPEND_DEVICE_INTERFACE));
    // ForceSuspendDevice must return ERR_SCREEN_OFF_BLOCKED
    PowerErrors ret = pmsTest->ForceSuspendDevice(CALLTIMEMS);
    EXPECT_EQ(ret, PowerErrors::ERR_SCREEN_OFF_BLOCKED);
    // Cancel the block; ForceSuspendDevice is no longer blocked
    EXPECT_EQ(pmsTest->SetInterfaceCallFilteringStrategy(
        InterfaceCallFilteringStrategy::SUSPEND_DEVICE_NOT_FILTERING, token),
        PowerErrors::ERR_OK);
    EXPECT_FALSE(stateMachine->IsScreenOffBlocked(
        PowerStateMachine::ScreenOffBlockType::SUSPEND_DEVICE_INTERFACE));
    POWER_HILOGI(LABEL_TEST, "ScreenOffBlockService005 end!");
}

#ifdef POWER_MANAGER_ENABLE_MONITOR_RUNNING_LOCK_CHANGE
/**
 * @tc.name: PowerMgrServiceNative025
 * @tc.desc: Test RegisterRunningLockChangedCallback & UnRegisterRunningLockChangedCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative025, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative025 start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    class TestRunningLockChangedCallback : public RunningLockChangedCallbackStub {
    public:
        TestRunningLockChangedCallback() = default;
        virtual ~TestRunningLockChangedCallback() = default;
    };
    
    sptr<TestRunningLockChangedCallback> callback = new TestRunningLockChangedCallback();
    // Test case 0: No system permission
    g_isSystem = false;
    PowerErrors ret = pmsTest->RegisterRunningLockChangedCallback(callback);
    EXPECT_TRUE(ret == PowerErrors::ERR_SYSTEM_API_DENIED);
    
    ret = pmsTest->UnRegisterRunningLockChangedCallback(callback);
    EXPECT_TRUE(ret == PowerErrors::ERR_SYSTEM_API_DENIED);

    // Test case 1: No POWER_MANAGER permission
    g_isSystem = true;
    g_isPermissionGranted = false;
    ret = pmsTest->RegisterRunningLockChangedCallback(callback);
    EXPECT_TRUE(ret == PowerErrors::ERR_PERMISSION_DENIED);

    ret = pmsTest->UnRegisterRunningLockChangedCallback(callback);
    EXPECT_TRUE(ret == PowerErrors::ERR_PERMISSION_DENIED);
    
    // Test case 2: DEFAULT
    g_isPermissionGranted = true;
    ret = pmsTest->RegisterRunningLockChangedCallback(callback);
    EXPECT_TRUE(ret == PowerErrors::ERR_OK);

    ret = pmsTest->UnRegisterRunningLockChangedCallback(callback);
    EXPECT_TRUE(ret == PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative025 end!");
}
#endif


/**
 * @tc.name: PowerMgrServiceNative026
 * @tc.desc: test GetPowerConfig and SetPowerConfig function
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative026, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative026 function start!");
    std::string sceneName = "lid_poweron_enable";
    std::string configVal;
    int32_t powerError = 0;
    
    PowerErrors ret = g_pmsTest->GetPowerConfig(sceneName, configVal);
#ifdef POWER_MANAGER_POWER_ENABLE_S4
    EXPECT_FALSE(sceneName.empty());
#else
    EXPECT_EQ(ret, PowerErrors::ERR_READ_OPERATION_FAILED);
#endif
    ret = g_pmsTest->SetPowerConfig(sceneName, "1");
    
    ret = g_pmsTest->GetPowerConfig(sceneName, configVal);
    
    sceneName = "";
    ret = g_pmsTest->GetPowerConfig(sceneName, configVal);
    
    ret = g_pmsTest->SetPowerConfig(sceneName, "test_value");
    
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative026 function end!");
}

/**
 * @tc.name: PowerMgrServiceNative027
 * @tc.desc: test GetPowerConfig permission branches
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative027, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative027 function start!");
    std::string sceneName = "lid_poweron_enable";
    std::string configVal;
    int32_t powerError = 0;
    
    g_isSystem = false;
    PowerErrors ret = g_pmsTest->GetPowerConfig(sceneName, configVal);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED);
    POWER_HILOGI(LABEL_TEST, "GetPowerConfig ERR_SYSTEM_API_DENIED ret = %{public}d", ret);
    
    g_isSystem = true;
    g_isPermissionGranted = false;
    ret = g_pmsTest->GetPowerConfig(sceneName, configVal);
    EXPECT_EQ(ret, PowerErrors::ERR_PERMISSION_DENIED);
    POWER_HILOGI(LABEL_TEST, "GetPowerConfig ERR_PERMISSION_DENIED ret = %{public}d", ret);
    
    g_isSystem = true;
    g_isPermissionGranted = true;
    ret = g_pmsTest->GetPowerConfig(sceneName, configVal);
    
    g_isSystem = true;
    g_isPermissionGranted = true;
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative027 function end!");
}

/**
 * @tc.name: PowerMgrServiceNative028
 * @tc.desc: test SetPowerConfig permission branches
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative028, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative028 function start!");
    std::string sceneName = "lid_poweron_enable";
    std::string configVal = "0";
    
    g_isSystem = false;
    PowerErrors ret = g_pmsTest->SetPowerConfig(sceneName, configVal);
    EXPECT_EQ(ret, PowerErrors::ERR_SYSTEM_API_DENIED);
    POWER_HILOGI(LABEL_TEST, "SetPowerConfig ERR_SYSTEM_API_DENIED ret = %{public}d", ret);
    
    g_isSystem = true;
    g_isPermissionGranted = false;
    ret = g_pmsTest->SetPowerConfig(sceneName, configVal);
    EXPECT_EQ(ret, PowerErrors::ERR_PERMISSION_DENIED);
    POWER_HILOGI(LABEL_TEST, "SetPowerConfig ERR_PERMISSION_DENIED ret = %{public}d", ret);
    
    g_isSystem = true;
    g_isPermissionGranted = true;
    ret = g_pmsTest->SetPowerConfig(sceneName, configVal);
    
    g_isSystem = true;
    g_isPermissionGranted = true;
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative028 function end!");
}

/**
 * @tc.name: PowerMgrServiceNative029
 * @tc.desc: test CreateRunningLock with displayId parameter
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative029, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative029 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest != nullptr);

    g_isPermissionGranted = true;
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test_display_id", RunningLockType::RUNNINGLOCK_SCREEN);
    info.displayId = 1001;

    PowerErrors ret = pmsTest->CreateRunningLock(token, info);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);

    info.displayId = UINT64_MAX;
    ret = pmsTest->CreateRunningLock(token, info);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative029 function end!");
}

/**
 * @tc.name: PowerMgrServiceNative030
 * @tc.desc: test QueryRunningLockLists with displayId parameter
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative030, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative030 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest != nullptr);

    g_isPermissionGranted = true;
    std::map<std::string, RunningLockInfo> lists;

    bool ret = pmsTest->QueryRunningLockLists(lists, UINT64_MAX);
    EXPECT_TRUE(ret);

    lists.clear();
    ret = pmsTest->QueryRunningLockLists(lists, 1001);
    EXPECT_TRUE(ret);

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative030 function end!");
}

/**
 * @tc.name: PowerMgrServiceNative032
 * @tc.desc: test IsRunningLockEnabled with displayId parameter
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative032, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative032 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest != nullptr);

    g_isSystem = true;
    g_isPermissionGranted = true;

    bool result = false;
    PowerErrors ret = pmsTest->IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_SCREEN, result, UINT64_MAX);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_FALSE(result);

    result = false;
    ret = pmsTest->IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_SCREEN, result, 1001);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_FALSE(result);

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative032 function end!");
}

#ifdef POWER_MANAGER_ENABLE_MONITOR_RUNNING_LOCK_CHANGE
/**
 * @tc.name: PowerMgrServiceNative033
 * @tc.desc: test RegisterRunningLockChangedCallback with nullptr
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative033, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative033 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest != nullptr);

    g_isSystem = true;
    g_isPermissionGranted = true;
    PowerErrors ret = pmsTest->RegisterRunningLockChangedCallback(nullptr, 1001);
    EXPECT_NE(ret, PowerErrors::ERR_OK);

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative033 function end!");
}

/**
 * @tc.name: PowerMgrServiceNative034
 * @tc.desc: test UnRegisterRunningLockChangedCallback with nullptr
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative034, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative034 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest != nullptr);

    g_isSystem = true;
    g_isPermissionGranted = true;
    PowerErrors ret = pmsTest->UnRegisterRunningLockChangedCallback(nullptr, 1001);
    EXPECT_NE(ret, PowerErrors::ERR_OK);

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative034 function end!");
}
#endif

/**
 * @tc.name: PowerMgrServiceNative035
 * @tc.desc: test CreateRunningLock with displayId = 0
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative035, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative035 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest != nullptr);

    g_isPermissionGranted = true;
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test_display_zero", RunningLockType::RUNNINGLOCK_SCREEN);
    info.displayId = 0;

    PowerErrors ret = pmsTest->CreateRunningLock(token, info);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative035 function end!");
}

#ifdef POWER_MANAGER_LOCK_SUPPORT_MULTI_SCREEN
/**
 * @tc.name: PowerMgrServiceNative036
 * @tc.desc: test IsRunningLockEnabled with displayId filtering
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative036, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative036 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest != nullptr);

    g_isSystem = true;
    g_isPermissionGranted = true;

    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test_lock_1001", RunningLockType::RUNNINGLOCK_SCREEN);
    info.displayId = 1001;

    PowerErrors ret = pmsTest->CreateRunningLock(token, info);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);

    ret = pmsTest->Lock(token, -1);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);

    bool result = false;
    ret = pmsTest->IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_SCREEN, result, 1001);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_TRUE(result);

    result = false;
    ret = pmsTest->IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_SCREEN, result, 1002);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_FALSE(result);

    ret = pmsTest->UnLock(token);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);

    pmsTest->ReleaseRunningLock(token);

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative036 function end!");
}
#endif

/**
 * @tc.name: PowerMgrServiceNative037
 * @tc.desc: test QueryRunningLockLists with permission denied
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative037, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative037 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest != nullptr);

    g_isPermissionGranted = false;
    std::map<std::string, RunningLockInfo> lists;
    bool ret = pmsTest->QueryRunningLockLists(lists, 1001);
    EXPECT_FALSE(ret);

    g_isPermissionGranted = true;
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative037 function end!");
}

/**
 * @tc.name: PowerMgrServiceNative038
 * @tc.desc: test IsRunningLockEnabled with permission denied
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative038, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative038 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest != nullptr);

    g_isPermissionGranted = false;
    bool result = false;
    PowerErrors ret = pmsTest->IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_SCREEN, result, 1001);
    EXPECT_EQ(ret, PowerErrors::ERR_PERMISSION_DENIED);

    g_isPermissionGranted = true;
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative038 function end!");
}

/**
 * @tc.name: PowerMgrServiceNative039
 * @tc.desc: test IsRunningLockEnabled with DISPLAY_ID_ALL lock visible to specific displayId
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative039, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative039 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest != nullptr);

    g_isPermissionGranted = true;
    g_isSystem = true;

    sptr<IRemoteObject> tokenAll = new RunningLockTokenStub();
    RunningLockInfo infoAll("test_all_display", RunningLockType::RUNNINGLOCK_SCREEN);
    infoAll.displayId = UINT64_MAX;
    PowerErrors ret = pmsTest->CreateRunningLock(tokenAll, infoAll);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);

    ret = pmsTest->Lock(tokenAll, -1);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);

    bool result = false;
#ifdef POWER_MANAGER_LOCK_SUPPORT_MULTI_SCREEN
    ret = pmsTest->IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_SCREEN, result, 1001);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_TRUE(result);
#endif

    result = false;
    ret = pmsTest->IsRunningLockEnabled(RunningLockType::RUNNINGLOCK_SCREEN, result, UINT64_MAX);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_TRUE(result);

    ret = pmsTest->UnLock(tokenAll);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    pmsTest->ReleaseRunningLock(tokenAll);

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative039 function end!");
}

#ifdef POWER_MANAGER_ENABLE_MONITOR_RUNNING_LOCK_CHANGE
#ifdef POWER_MANAGER_LOCK_SUPPORT_MULTI_SCREEN
/**
 * @tc.name: PowerMgrServiceNative040
 * @tc.desc: test UnRegisterRunningLockChangedCallback keeps callbacks when other displayIds remain
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative040, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative040 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    EXPECT_TRUE(pmsTest != nullptr);

    class TestMultiDisplayIdCallback : public RunningLockChangedCallbackStub {
    public:
        TestMultiDisplayIdCallback() = default;
        virtual ~TestMultiDisplayIdCallback() = default;
    };

    sptr<TestMultiDisplayIdCallback> callback = new TestMultiDisplayIdCallback();

    g_isSystem = true;
    g_isPermissionGranted = true;

    auto runningLockMgr = pmsTest->GetRunningLockMgr();

    PowerErrors ret = pmsTest->RegisterRunningLockChangedCallback(callback, 1001);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    ret = pmsTest->RegisterRunningLockChangedCallback(callback, 1002);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);

    EXPECT_EQ(runningLockMgr->GetRunningLockChangedCallbackCount(), 2);
    EXPECT_TRUE(runningLockMgr->HasRunningLockChangedCallbacks(callback->AsObject()));

    ret = pmsTest->UnRegisterRunningLockChangedCallback(callback, 1001);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);

    EXPECT_EQ(runningLockMgr->GetRunningLockChangedCallbackCount(), 1);
    EXPECT_TRUE(runningLockMgr->HasRunningLockChangedCallbacks(callback->AsObject()));

    ret = pmsTest->UnRegisterRunningLockChangedCallback(callback, 1002);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);

    EXPECT_EQ(runningLockMgr->GetRunningLockChangedCallbackCount(), 0);
    EXPECT_FALSE(runningLockMgr->HasRunningLockChangedCallbacks(callback->AsObject()));

    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative040 function end!");
}
#endif
#endif

#ifdef POWER_MANAGER_ENABLE_SUSPEND_WITH_TAG
/**
 * @tc.name: PowerMgrServiceNative041
 * @tc.desc: Test TriggerUlsrSyncCallback - normal case and null holder
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative041, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative041 start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    sptr<UlsrCallbackTest> callback = new UlsrCallbackTest();
    g_isSystem = true;

    // Case 1: Trigger when ulsrCallbackHolder_ is nullptr - should skip gracefully
    pmsTest_->TriggerUlsrSyncCallback();

    // Case 2: Register callback and trigger
    PowerErrors ret = pmsTest_->RegisterUlsrCallback(callback, UlsrPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    pmsTest_->TriggerUlsrSyncCallback();

    // Clean up
    pmsTest_->UnRegisterUlsrCallback(callback);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative041 end!");
}

/**
 * @tc.name: PowerMgrServiceNative042
 * @tc.desc: Test OnUlsrTimerExpired
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative042, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative042 start!");
    auto pmsTest_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
    sptr<UlsrCallbackTest> callback = new UlsrCallbackTest();
    g_isSystem = true;

    // Register callback first
    PowerErrors ret = pmsTest_->RegisterUlsrCallback(callback, UlsrPriority::DEFAULT);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);

    // OnUlsrTimerExpired with screen on - should skip
    // Just verify no crash when called with various states
    pmsTest_->OnUlsrTimerExpired();

    // Clean up
    pmsTest_->UnRegisterUlsrCallback(callback);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNativeTest::PowerMgrServiceNative042 end!");
}
#endif // POWER_MANAGER_ENABLE_SUSPEND_WITH_TAG

/**
 * @tc.name: PowerMgrServiceNative043
 * @tc.desc: Test RefreshActivityInner with callTimeMs!=0 is throttled within 100ms
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative043, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative043 start!");
    auto stateMachine = g_pmsTest->GetPowerStateMachine();
    ASSERT_TRUE(stateMachine != nullptr);
    ::testing::NiceMock<MockStateAction>* stateActionMock = new ::testing::NiceMock<MockStateAction>;
    stateMachine->EnableMock(stateActionMock);
    EXPECT_CALL(*stateActionMock, GetDisplayState()).WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*stateActionMock, SetDisplayState(::testing::_, ::testing::_))
        .WillRepeatedly(::testing::Return(ActionResult::SUCCESS));
    stateMachine->mDeviceState_.lastRefreshActivityTime = 0;

    bool ret1 = g_pmsTest->RefreshActivityInner(CALLTIMEMS, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, false);
    EXPECT_TRUE(ret1);

    bool ret2 = g_pmsTest->RefreshActivityInner(CALLTIMEMS, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, false);
    EXPECT_FALSE(ret2);

    stateMachine->Init();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative043 end!");
}

/**
 * @tc.name: PowerMgrServiceNative044
 * @tc.desc: Test RefreshActivityInner with callTimeMs==0 bypasses throttle
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative044, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative044 start!");
    auto stateMachine = g_pmsTest->GetPowerStateMachine();
    ASSERT_TRUE(stateMachine != nullptr);
    ::testing::NiceMock<MockStateAction>* stateActionMock = new ::testing::NiceMock<MockStateAction>;
    stateMachine->EnableMock(stateActionMock);
    EXPECT_CALL(*stateActionMock, GetDisplayState()).WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*stateActionMock, SetDisplayState(::testing::_, ::testing::_))
        .WillRepeatedly(::testing::Return(ActionResult::SUCCESS));
    stateMachine->mDeviceState_.lastRefreshActivityTime = 0;

    bool ret1 = g_pmsTest->RefreshActivityInner(CALLTIMEMS, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, false);
    EXPECT_TRUE(ret1);

    bool ret2 = g_pmsTest->RefreshActivityInner(0, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, false);
    EXPECT_TRUE(ret2);

    stateMachine->Init();
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative044 end!");
}

/**
 * @tc.name: DelaySuspendDevice001
 * @tc.desc: Test DelaySuspendDevice permission and parameter validation
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(PowerMgrServiceNativeTest, DelaySuspendDevice001, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "DelaySuspendDevice001 start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest != nullptr);
    constexpr int64_t DELAY_MS = 3000;
    constexpr SuspendDeviceType REASON = SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH;
    // No system permission
    g_isSystem = false;
    g_isPermissionGranted = true;
    EXPECT_EQ(pmsTest->DelaySuspendDevice(REASON, DELAY_MS), PowerErrors::ERR_SYSTEM_API_DENIED);
    // System permission but no POWER_MANAGER permission
    g_isSystem = true;
    g_isPermissionGranted = false;
    EXPECT_EQ(pmsTest->DelaySuspendDevice(REASON, DELAY_MS), PowerErrors::ERR_PERMISSION_DENIED);
    g_isPermissionGranted = true;
    // Out of range delay: validation lives on the service side, the client only forwards
    EXPECT_EQ(pmsTest->DelaySuspendDevice(REASON, -1), PowerErrors::ERR_PARAM_INVALID);
    EXPECT_EQ(pmsTest->DelaySuspendDevice(REASON,
        PowerMgrService::DELAY_SUSPEND_MAX_DELAY_MS + 1), PowerErrors::ERR_PARAM_INVALID);
    // A rejected request must not arm the task, otherwise the next lid-close would be swallowed
    EXPECT_EQ(pmsTest->ffrtTimer_->GetTaskHandlePtr(TIMER_ID_SCREEN_TIMEOUT_CHECK), nullptr);
    POWER_HILOGI(LABEL_TEST, "DelaySuspendDevice001 end!");
}

/**
 * @tc.name: DelaySuspendDevice002
 * @tc.desc: Test IsInCallState fail-open and the ERR_SCREEN_OFF_BLOCKED contract
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(PowerMgrServiceNativeTest, DelaySuspendDevice002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "DelaySuspendDevice002 start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest != nullptr);
    g_isSystem = true;
    g_isPermissionGranted = true;
    constexpr SuspendDeviceType REASON = SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH;
    // Fail-open: call_manager is not compiled in or its SA is not running here, both read as no
    // call. The live-call branch depends on an external SA and is covered manually.
    EXPECT_FALSE(pmsTest->IsInCallState());
    // DMS switches the outer screen on only for this exact value, it is part of the contract
    EXPECT_EQ(static_cast<int32_t>(PowerErrors::ERR_SCREEN_OFF_BLOCKED), 4900701);
    // No SuspendDeviceType maps to a call block, a normal lid-close suspend stays unblockable.
    EXPECT_EQ(PowerStateMachine::MapReasonToBlockType(REASON),
        PowerStateMachine::ScreenOffBlockType::NONE);
    // Idle path: accepted and armed, and the longest delay is used so it cannot fire mid-test
    EXPECT_EQ(pmsTest->DelaySuspendDevice(REASON,
        PowerMgrService::DELAY_SUSPEND_MAX_DELAY_MS), PowerErrors::ERR_OK);
    EXPECT_NE(pmsTest->ffrtTimer_->GetTaskHandlePtr(TIMER_ID_SCREEN_TIMEOUT_CHECK), nullptr);
    pmsTest->CancelDelaySuspendTimer(UserActivityType::USER_ACTIVITY_TYPE_ATTENTION);
    EXPECT_EQ(pmsTest->ffrtTimer_->GetTaskHandlePtr(TIMER_ID_SCREEN_TIMEOUT_CHECK), nullptr);
    POWER_HILOGI(LABEL_TEST, "DelaySuspendDevice002 end!");
}

/**
 * @tc.name: DelaySuspendDevice003
 * @tc.desc: Test the debounce re-arm and which refresh types cancel the task
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(PowerMgrServiceNativeTest, DelaySuspendDevice003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "DelaySuspendDevice003 start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest != nullptr);
    g_isSystem = true;
    g_isPermissionGranted = true;
    auto stateMachine = pmsTest->GetPowerStateMachine();
    ASSERT_TRUE(stateMachine != nullptr);
    ::testing::NiceMock<MockStateAction>* stateActionMock = new ::testing::NiceMock<MockStateAction>;
    stateMachine->EnableMock(stateActionMock);
    EXPECT_CALL(*stateActionMock, GetDisplayState()).WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*stateActionMock, SetDisplayState(::testing::_, ::testing::_))
        .WillRepeatedly(::testing::Return(ActionResult::SUCCESS));
    constexpr SuspendDeviceType REASON = SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH;
    // Arm with the longest accepted delay so the task cannot fire while this test runs
    EXPECT_EQ(pmsTest->DelaySuspendDevice(REASON,
        PowerMgrService::DELAY_SUSPEND_MAX_DELAY_MS), PowerErrors::ERR_OK);
    EXPECT_NE(pmsTest->ffrtTimer_->GetTaskHandlePtr(TIMER_ID_SCREEN_TIMEOUT_CHECK), nullptr);
    // Debounce: a re-entrant request cancels the pending task and re-arms a fresh one
    EXPECT_EQ(pmsTest->DelaySuspendDevice(REASON,
        PowerMgrService::DELAY_SUSPEND_MAX_DELAY_MS), PowerErrors::ERR_OK);
    EXPECT_NE(pmsTest->ffrtTimer_->GetTaskHandlePtr(TIMER_ID_SCREEN_TIMEOUT_CHECK), nullptr);
    // Program-sourced types must not cancel the task, callTimeMs 0 bypasses throttling.
    pmsTest->RefreshActivityInner(0, UserActivityType::USER_ACTIVITY_TYPE_OTHER, false);
    pmsTest->RefreshActivityInner(0, UserActivityType::USER_ACTIVITY_TYPE_SWITCH, false);
    pmsTest->RefreshActivityInner(0, UserActivityType::USER_ACTIVITY_TYPE_CABLE, false);
    pmsTest->RefreshActivityInner(0, UserActivityType::USER_ACTIVITY_TYPE_APPLICATION, false);
    pmsTest->RefreshActivityInner(0, UserActivityType::USER_ACTIVITY_TYPE_SOFTWARE, false);
    EXPECT_NE(pmsTest->ffrtTimer_->GetTaskHandlePtr(TIMER_ID_SCREEN_TIMEOUT_CHECK), nullptr);
    // Real user input (eye detection / touch reported by swing) cancels the task
    pmsTest->RefreshActivityInner(0, UserActivityType::USER_ACTIVITY_TYPE_TOUCH, false);
    EXPECT_EQ(pmsTest->ffrtTimer_->GetTaskHandlePtr(TIMER_ID_SCREEN_TIMEOUT_CHECK), nullptr);
    // Cancelling twice is a no-op and never crashes on an unarmed timer slot
    pmsTest->CancelDelaySuspendTimer(UserActivityType::USER_ACTIVITY_TYPE_ATTENTION);
    EXPECT_EQ(pmsTest->ffrtTimer_->GetTaskHandlePtr(TIMER_ID_SCREEN_TIMEOUT_CHECK), nullptr);
    stateMachine->Init();
    POWER_HILOGI(LABEL_TEST, "DelaySuspendDevice003 end!");
}

/**
 * @tc.name: DelaySuspendDevice004
 * @tc.desc: Test DoDelaySuspendDevice is a no-op once cancelled, otherwise reaches INACTIVE
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(PowerMgrServiceNativeTest, DelaySuspendDevice004, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "DelaySuspendDevice004 start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest != nullptr);
    g_isSystem = true;
    g_isPermissionGranted = true;
    auto stateMachine = pmsTest->GetPowerStateMachine();
    ASSERT_TRUE(stateMachine != nullptr);
    ::testing::NiceMock<MockStateAction>* stateActionMock = new ::testing::NiceMock<MockStateAction>;
    stateMachine->EnableMock(stateActionMock);
    EXPECT_CALL(*stateActionMock, GetDisplayState()).WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*stateActionMock, SetDisplayState(::testing::_, ::testing::_))
        .WillRepeatedly(::testing::Return(ActionResult::SUCCESS));
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS,
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "test", "test");
    ASSERT_EQ(stateMachine->GetState(), PowerState::AWAKE);
    constexpr SuspendDeviceType REASON = SuspendDeviceType::SUSPEND_DEVICE_REASON_SWITCH;
    // (a) Cancelled before the task ran: the timer slot is cleared, the screen must stay on
    EXPECT_EQ(pmsTest->DelaySuspendDevice(REASON,
        PowerMgrService::DELAY_SUSPEND_MAX_DELAY_MS), PowerErrors::ERR_OK);
    pmsTest->CancelDelaySuspendTimer(UserActivityType::USER_ACTIVITY_TYPE_ATTENTION);
    EXPECT_EQ(pmsTest->ffrtTimer_->GetTaskHandlePtr(TIMER_ID_SCREEN_TIMEOUT_CHECK), nullptr);
    EXPECT_EQ(stateMachine->GetState(), PowerState::AWAKE);
    // (b) Armed and idle: the screen goes off through the state machine, not via SetDisplayState
    pmsTest->DoDelaySuspendDevice(REASON);
    EXPECT_EQ(stateMachine->GetState(), PowerState::INACTIVE);
    EXPECT_EQ(pmsTest->ffrtTimer_->GetTaskHandlePtr(TIMER_ID_SCREEN_TIMEOUT_CHECK), nullptr);
    // Restore for the following test cases
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS,
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "test", "test");
    stateMachine->Init();
    POWER_HILOGI(LABEL_TEST, "DelaySuspendDevice004 end!");
}

/**
 * @tc.name: DelaySuspendDevice005
 * @tc.desc: Test a registered LID screen-off filter blocks both the arming entry and the task body
 * @tc.type: FUNC
 * @tc.require: issueNone
 */
HWTEST_F(PowerMgrServiceNativeTest, DelaySuspendDevice005, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "DelaySuspendDevice005 start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    ASSERT_TRUE(pmsTest != nullptr);
    g_isSystem = true;
    g_isPermissionGranted = true;
    auto stateMachine = pmsTest->GetPowerStateMachine();
    ASSERT_TRUE(stateMachine != nullptr);
    ::testing::NiceMock<MockStateAction>* stateActionMock = new ::testing::NiceMock<MockStateAction>;
    stateMachine->EnableMock(stateActionMock);
    EXPECT_CALL(*stateActionMock, GetDisplayState()).WillRepeatedly(::testing::Return(DisplayState::DISPLAY_ON));
    EXPECT_CALL(*stateActionMock, SetDisplayState(::testing::_, ::testing::_))
        .WillRepeatedly(::testing::Return(ActionResult::SUCCESS));
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS,
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "test", "test");
    ASSERT_EQ(stateMachine->GetState(), PowerState::AWAKE);
    constexpr SuspendDeviceType REASON = SuspendDeviceType::SUSPEND_DEVICE_REASON_LID;
    sptr<IRemoteObject> token = sptr<IPCObjectProxy>::MakeSptr(0, u"DelaySuspendDevice005");
    ASSERT_EQ(pmsTest->SetLidFilteringStrategy(LidFilteringStrategy::LID_CLOSE_FILTERING, token),
        PowerErrors::ERR_OK);
    // (a) The arming entry is rejected and the one-shot flag is left untouched
    EXPECT_EQ(pmsTest->DelaySuspendDevice(REASON,
        PowerMgrService::DELAY_SUSPEND_MAX_DELAY_MS), PowerErrors::ERR_SCREEN_OFF_BLOCKED);
    EXPECT_EQ(pmsTest->ffrtTimer_->GetTaskHandlePtr(TIMER_ID_SCREEN_TIMEOUT_CHECK), nullptr);
    // (b) The task body re-checks the filter, the screen must stay on
    pmsTest->DoDelaySuspendDevice(REASON);
    EXPECT_EQ(stateMachine->GetState(), PowerState::AWAKE);
    // (c) Filter lifted: the very same task suspends normally
    ASSERT_EQ(pmsTest->SetLidFilteringStrategy(LidFilteringStrategy::LID_CLOSE_NOT_FILTERING, token),
        PowerErrors::ERR_OK);
    pmsTest->DoDelaySuspendDevice(REASON);
    EXPECT_EQ(stateMachine->GetState(), PowerState::INACTIVE);
    EXPECT_EQ(pmsTest->ffrtTimer_->GetTaskHandlePtr(TIMER_ID_SCREEN_TIMEOUT_CHECK), nullptr);
    // Restore for the following test cases
    stateMachine->WakeupDeviceInner(PID, CALLTIMEMS,
        WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON, "test", "test");
    stateMachine->Init();
    POWER_HILOGI(LABEL_TEST, "DelaySuspendDevice005 end!");
}

class HibernateCallbackTest : public SyncHibernateCallbackStub {
public:
    HibernateCallbackTest() = default;
    ~HibernateCallbackTest() override = default;
};

/**
 * @tc.name: PowerMgrServiceNative045
 * @tc.desc: Test RegisterSyncHibernateCallbackEx permission and parameter validation
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative045, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative045 start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    sptr<HibernateCallbackTest> callback = new HibernateCallbackTest();

    g_isSystem = false;
    EXPECT_EQ(pmsTest->RegisterSyncHibernateCallbackEx(callback, HibernateCallbackPriority::DEFAULT),
        PowerErrors::ERR_SYSTEM_API_DENIED);

    g_isSystem = true;
    g_isPermissionGranted = false;
    EXPECT_EQ(pmsTest->RegisterSyncHibernateCallbackEx(callback, HibernateCallbackPriority::DEFAULT),
        PowerErrors::ERR_PERMISSION_DENIED);

    g_isPermissionGranted = true;
    EXPECT_EQ(pmsTest->RegisterSyncHibernateCallbackEx(nullptr, HibernateCallbackPriority::DEFAULT),
        PowerErrors::ERR_PARAM_INVALID);

    EXPECT_EQ(pmsTest->RegisterSyncHibernateCallbackEx(callback, HibernateCallbackPriority::DEFAULT),
        PowerErrors::ERR_OK);
    EXPECT_EQ(pmsTest->UnRegisterSyncHibernateCallbackEx(callback), PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative045 end!");
}

/**
 * @tc.name: PowerMgrServiceNative046
 * @tc.desc: Test UnRegisterSyncHibernateCallbackEx permission and parameter validation
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceNativeTest, PowerMgrServiceNative046, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative046 start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    sptr<HibernateCallbackTest> callback = new HibernateCallbackTest();

    g_isSystem = false;
    EXPECT_EQ(pmsTest->UnRegisterSyncHibernateCallbackEx(callback), PowerErrors::ERR_SYSTEM_API_DENIED);

    g_isSystem = true;
    g_isPermissionGranted = false;
    EXPECT_EQ(pmsTest->UnRegisterSyncHibernateCallbackEx(callback), PowerErrors::ERR_PERMISSION_DENIED);

    g_isPermissionGranted = true;
    EXPECT_EQ(pmsTest->UnRegisterSyncHibernateCallbackEx(nullptr), PowerErrors::ERR_PARAM_INVALID);
    EXPECT_EQ(pmsTest->UnRegisterSyncHibernateCallbackEx(callback), PowerErrors::ERR_OK);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceNative046 end!");
}

} // namespace
