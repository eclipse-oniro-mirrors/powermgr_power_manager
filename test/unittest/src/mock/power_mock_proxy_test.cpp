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

#include "power_mock_proxy_test.h"

#include <datetime_ex.h>

#include "mock_power_remote_object.h"
#include "parcel.h"
#include "power_log.h"
#include "power_mgr_client.h"
#include "power_mgr_proxy.h"
#include "power_mode_callback_proxy.h"
#include "power_state_callback_proxy.h"
#include "power_runninglock_callback_proxy.h"
#include "power_state_machine_info.h"
#include "running_lock.h"
#include "running_lock_info.h"
#include "running_lock_token_stub.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void PowerMockProxyTest::PowerModeTestCallback::OnPowerModeChanged(PowerMode mode)
{
    POWER_HILOGI(LABEL_TEST, "PowerModeTestCallback::OnPowerModeChanged.");
}

void PowerMockProxyTest::PowerStateTestCallback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGI(LABEL_TEST, "PowerStateTestCallback::OnPowerStateChanged.");
}

void PowerMockProxyTest::PowerRunningLockTestCallback::HandleRunningLockMessage(std::string message)
{
    POWER_HILOGI(LABEL_TEST, "PowerRunningLockTestCallback::HandleRunningLockMessage.");
}
namespace {
/**
 * @tc.name: PowerMockProxyTest001
 * @tc.desc: Test proxy when the parcel is mock
 * @tc.type: FUNC
 */
HWTEST_F(PowerMockProxyTest, PowerMockProxyTest001, TestSize.Level2)
{
    pid_t uid = 0;
    pid_t pid = 0;
    sptr<IPCObjectStub> remote = new IPCObjectStub();
    std::shared_ptr<PowerMgrProxy> sptrProxy = std::make_shared<PowerMgrProxy>(remote);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo info("test1", RunningLockType::RUNNINGLOCK_SCREEN);
    EXPECT_FALSE(sptrProxy->CreateRunningLock(token, info) == PowerErrors::ERR_OK);
    EXPECT_FALSE(sptrProxy->ReleaseRunningLock(token));
    EXPECT_FALSE(sptrProxy->IsRunningLockTypeSupported(RunningLockType::RUNNINGLOCK_BUTT));
    EXPECT_FALSE(sptrProxy->Lock(token) == PowerErrors::ERR_OK);
    EXPECT_FALSE(sptrProxy->UnLock(token) == PowerErrors::ERR_OK);
    EXPECT_FALSE(sptrProxy->IsUsed(token));
    EXPECT_FALSE(sptrProxy->ProxyRunningLock(true, pid, uid));
    EXPECT_FALSE(sptrProxy->ProxyRunningLocks(true, {std::make_pair(pid, uid)}));
    EXPECT_FALSE(sptrProxy->ResetRunningLocks());
}

/**
 * @tc.name: PowerMockProxyTest002
 * @tc.desc: Test proxy when the parcel is mock
 * @tc.type: FUNC
 */
HWTEST_F(PowerMockProxyTest, PowerMockProxyTest002, TestSize.Level2)
{
    sptr<IPCObjectStub> remote = new IPCObjectStub();
    std::shared_ptr<PowerMgrProxy> sptrProxy = std::make_shared<PowerMgrProxy>(remote);
    int32_t suspendReason = (static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_MAX)) + 1;
    SuspendDeviceType abnormaltype = SuspendDeviceType(suspendReason);
    EXPECT_EQ(sptrProxy->SuspendDevice(0, abnormaltype, false), PowerErrors::ERR_CONNECTION_FAIL);
    auto error =
        sptrProxy->WakeupDevice(GetTickCount(), WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, std::string("app call"));
    EXPECT_EQ(error, PowerErrors::ERR_CONNECTION_FAIL);
    EXPECT_FALSE(sptrProxy->RefreshActivity(GetTickCount(), UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, true));
    EXPECT_FALSE(sptrProxy->OverrideScreenOffTime(200) == PowerErrors::ERR_OK);
    EXPECT_FALSE(sptrProxy->RestoreScreenOffTime() == PowerErrors::ERR_OK);
    auto state = sptrProxy->GetState();
    EXPECT_EQ(state, PowerState::UNKNOWN);
    EXPECT_FALSE(sptrProxy->IsScreenOn());
    sptrProxy->SetDisplaySuspend(true);
    PowerMode mode1 = PowerMode::NORMAL_MODE;
    EXPECT_EQ(sptrProxy->SetDeviceMode(mode1), PowerErrors::ERR_CONNECTION_FAIL);
    auto mode2 = sptrProxy->GetDeviceMode();
    EXPECT_TRUE(mode2 == mode1);
}

/**
 * @tc.name: PowerMockProxyTest003
 * @tc.desc: Test proxy when the parcel is mock
 * @tc.type: FUNC
 */
HWTEST_F(PowerMockProxyTest, PowerMockProxyTest003, TestSize.Level2)
{
    sptr<IPCObjectStub> remote = new IPCObjectStub();
    std::shared_ptr<PowerMgrProxy> sptrProxy = std::make_shared<PowerMgrProxy>(remote);
    sptr<IPowerStateCallback> cb1 = new PowerStateTestCallback();
    sptr<IPowerModeCallback> cb3 = new PowerModeTestCallback();
    sptr<IPowerRunninglockCallback> cb5 =new PowerRunningLockTestCallback();
    EXPECT_FALSE(sptrProxy->RegisterPowerStateCallback(cb1));
    EXPECT_FALSE(sptrProxy->UnRegisterPowerStateCallback(cb1));
    EXPECT_FALSE(sptrProxy->RegisterPowerStateCallback(nullptr));
    EXPECT_FALSE(sptrProxy->UnRegisterPowerStateCallback(nullptr));
    EXPECT_FALSE(sptrProxy->RegisterPowerModeCallback(cb3));
    EXPECT_FALSE(sptrProxy->UnRegisterPowerModeCallback(cb3));
    EXPECT_FALSE(sptrProxy->RegisterPowerModeCallback(nullptr));
    EXPECT_FALSE(sptrProxy->UnRegisterPowerModeCallback(nullptr));
    EXPECT_FALSE(sptrProxy->RegisterRunningLockCallback(cb5));
    EXPECT_FALSE(sptrProxy->UnRegisterRunningLockCallback(cb5));
    EXPECT_FALSE(sptrProxy->RegisterRunningLockCallback(nullptr));
    EXPECT_FALSE(sptrProxy->UnRegisterRunningLockCallback(nullptr));
    EXPECT_EQ(sptrProxy->RebootDevice(" "), PowerErrors::ERR_CONNECTION_FAIL);
    EXPECT_EQ(sptrProxy->ShutDownDevice(" "), PowerErrors::ERR_CONNECTION_FAIL);
    EXPECT_FALSE(sptrProxy->ForceSuspendDevice(0) == PowerErrors::ERR_OK);
    static std::vector<std::string> dumpArgs;
    dumpArgs.push_back("-a");
    std::string errorCode = "remote error";
    std::string actualDebugInfo = sptrProxy->ShellDump(dumpArgs, dumpArgs.size());
    EXPECT_EQ(errorCode, actualDebugInfo);
}
} // namespace