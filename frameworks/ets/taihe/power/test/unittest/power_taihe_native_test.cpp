/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ohos.power.proj.hpp"
#include "ohos.power.impl.hpp"
#include "ohos.power.user.hpp"
#include "taihe/runtime.hpp"
#include "power_mgr_client.h"
#include "power_shutdown_callback.h"
#include "power_hibernate_callback.h"
#include "power_errors.h"
#include "power_log.h"
#include "event_handler.h"
#include "event_runner.h"
#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <thread>

using namespace taihe;
using namespace ohos::power;
using namespace OHOS::PowerMgr;

using namespace testing;
using namespace testing::ext;

namespace {
constexpr int64_t HIBERNATE_CALLBACK_TEST_DELAY_MS = 50; // ms
constexpr int64_t MAIN_RUNNER_START_TIMEOUT_MS = 1000; // ms
PowerErrors g_error = PowerErrors::ERR_OK;
bool g_pass = false;
bool g_cbFlag = false;
PowerMode g_mode = PowerMode::NORMAL_MODE;
int32_t g_errCode = 0;
std::string g_configVal = "";
}

namespace taihe {
void set_business_error(int32_t err_code, taihe::string_view msg)
{
    g_errCode = err_code;
    (void)msg;
}
}

namespace OHOS::PowerMgr {
bool PowerMgrClient::IsScreenOn(bool needPrintLog)
{
    g_pass = true;
    return false;
}

PowerErrors PowerMgrClient::WakeupDevice(
    WakeupDeviceType reason, const std::string& detail, const std::string& apiVersion)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::ForceSuspendDevice(const std::string& apiVersion)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::SuspendDevice(
    SuspendDeviceType reason, bool suspendImmed, const std::string& apiVersion)
{
    g_pass = true;
    return g_error;
}

PowerMode PowerMgrClient::GetDeviceMode()
{
    g_pass = true;
    return g_mode;
}

PowerErrors PowerMgrClient::SetDeviceMode(const PowerMode mode)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::IsStandby(bool& isStandby)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::Hibernate(bool clearMemory, const std::string& reason)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::RestoreScreenOffTime()
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::OverrideScreenOffTime(int64_t timeout)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::RefreshActivity(UserActivityType type, const std::string& refreshReason)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::SetPowerKeyFilteringStrategy(PowerKeyFilteringStrategy strategy)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::RegisterAsyncShutdownCallback(
    const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::UnRegisterAsyncShutdownCallback(const sptr<IAsyncShutdownCallback>& callback)
{
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::GetPowerConfig(const std::string& sceneName, std::string& configVal)
{
    g_pass = true;
    configVal = g_configVal;
    return g_error;
}

PowerErrors PowerMgrClient::SetPowerConfig(const std::string& sceneName, const std::string& configVal)
{
    g_pass = true;
    g_configVal = configVal;
    return g_error;
}

PowerErrors PowerMgrClient::RegisterSyncHibernateCallbackEx(
    const sptr<ISyncHibernateCallback>& callback, HibernateCallbackPriority priority)
{
    (void)callback;
    (void)priority;
    g_pass = true;
    return g_error;
}

PowerErrors PowerMgrClient::UnRegisterSyncHibernateCallbackEx(const sptr<ISyncHibernateCallback>& callback)
{
    (void)callback;
    g_pass = true;
    return g_error;
}

} // namespace OHOS::PowerMgr

namespace {
class PowerTaiheNativeTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        // Callbacks are posted to the process main event runner, which is running in an app process
        // but not in the gtest process, so start it here and stop it after the suite.
        runner_ = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        if (runner_ == nullptr) {
            return;
        }
        auto started = std::make_shared<std::promise<void>>();
        auto startedFuture = started->get_future();
        runnerThread_ = std::thread([]() { runner_->Run(); });
        OHOS::AppExecFwk::EventHandler handler(runner_);
        handler.PostTask([started]() { started->set_value(); });
        (void)startedFuture.wait_for(std::chrono::milliseconds(MAIN_RUNNER_START_TIMEOUT_MS));
    }
    static void TearDownTestCase()
    {
        if (runner_ != nullptr) {
            runner_->Stop();
        }
        if (runnerThread_.joinable()) {
            runnerThread_.join();
        }
    }
    void SetUp()
    {
        EXPECT_FALSE(g_pass);
    }
    void TearDown()
    {
        g_pass = false;
        g_mode = PowerMode::NORMAL_MODE;
        g_cbFlag = false;
        g_error = PowerErrors::ERR_OK;
        g_errCode = 0;
        g_configVal.clear();
    }

private:
    static std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner_;
    static std::thread runnerThread_;
};

std::shared_ptr<OHOS::AppExecFwk::EventRunner> PowerTaiheNativeTest::runner_ = nullptr;
std::thread PowerTaiheNativeTest::runnerThread_;

/**
 * @tc.name: PowerTaiheNativeTest_001
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_001 start");
    IsActive();
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_001 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_002
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_002 start");
    string_view detail = "PowerTaiheNativeTest_002";
    Wakeup(detail);

    g_error = PowerErrors::ERR_FAILURE;
    Wakeup(detail);
    EXPECT_TRUE(g_errCode != static_cast<int32_t>(PowerErrors::ERR_FAILURE));

    g_error = PowerErrors::ERR_SYSTEM_API_DENIED;
    Wakeup(detail);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_SYSTEM_API_DENIED));
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_002 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_003
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_003 start");
    bool isImmediate = false;
    Suspend(&isImmediate);

    isImmediate = true;
    Suspend(&isImmediate);

    g_error = PowerErrors::ERR_FAILURE;
    Suspend(&isImmediate);
    EXPECT_TRUE(g_errCode != static_cast<int32_t>(PowerErrors::ERR_FAILURE));

    g_error = PowerErrors::ERR_SYSTEM_API_DENIED;
    Suspend(&isImmediate);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_SYSTEM_API_DENIED));
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_003 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_004
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_004, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_004 start");
    GetPowerMode();
    g_mode = PowerMode::POWER_SAVE_MODE;
    GetPowerMode();
    g_mode = PowerMode::PERFORMANCE_MODE;
    GetPowerMode();
    g_mode = PowerMode::EXTREME_POWER_SAVE_MODE;
    GetPowerMode();
    g_mode = PowerMode::CUSTOM_POWER_SAVE_MODE;
    GetPowerMode();
    g_mode = static_cast<PowerMode>(0);
    GetPowerMode();
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_004 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_005
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_005, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_005 start");
    DevicePowerMode mode(DevicePowerMode::key_t::MODE_NORMAL);
    SetPowerModeSync(mode);

    g_error = PowerErrors::ERR_FAILURE;
    SetPowerModeSync(mode);
    EXPECT_TRUE(g_errCode != static_cast<int32_t>(PowerErrors::ERR_FAILURE));

    g_error = PowerErrors::ERR_CONNECTION_FAIL;
    SetPowerModeSync(mode);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL));
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_005 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_006
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_006, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_006 start");
    IsStandby();

    g_error = PowerErrors::ERR_FAILURE;
    IsStandby();
    EXPECT_TRUE(g_errCode != static_cast<int32_t>(PowerErrors::ERR_FAILURE));

    g_error = PowerErrors::ERR_CONNECTION_FAIL;
    IsStandby();
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL));
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_006 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_007
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_007, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_007 start");
    bool clearMemory = false;
    Hibernate(clearMemory);

    g_error = PowerErrors::ERR_FAILURE;
    Hibernate(clearMemory);
    EXPECT_TRUE(g_errCode != static_cast<int32_t>(PowerErrors::ERR_FAILURE));

    clearMemory = true;
    g_error = PowerErrors::ERR_SYSTEM_API_DENIED;
    Hibernate(clearMemory);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_SYSTEM_API_DENIED));
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_007 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_008
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_008, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_008 start");
    int64_t timeout = 0;
    SetScreenOffTime(timeout);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID));

    constexpr int32_t INVALID_TIMEOUT = -2;
    SetScreenOffTime(INVALID_TIMEOUT);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID));

    constexpr int32_t RESTORE_DEFAULT_SCREENOFF_TIME = -1;
    SetScreenOffTime(RESTORE_DEFAULT_SCREENOFF_TIME);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID));

    timeout = 30000; // ms
    SetScreenOffTime(timeout);

    g_error = PowerErrors::ERR_FAILURE;
    SetScreenOffTime(timeout);
    EXPECT_TRUE(g_errCode != static_cast<int32_t>(PowerErrors::ERR_FAILURE));

    g_error = PowerErrors::ERR_SYSTEM_API_DENIED;
    SetScreenOffTime(timeout);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_SYSTEM_API_DENIED));
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_008 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_009
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICTIYR
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_009, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_009 start");
    RefreshActivity("PowerTaiheNativeTest_009");
    g_error = PowerErrors::ERR_FREQUENT_FUNCTION_CALL;
    RefreshActivity("PowerTaiheNativeTest_009");
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_FREQUENT_FUNCTION_CALL));
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_009 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_010
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICTIYR
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_010, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_010 start");
    struct MyCallbackImpl {
        void operator()(bool isReboot)
        {
            g_cbFlag = true;
            EXPECT_TRUE(isReboot == false);
        }
    };
    callback<void(bool)> shutdownCb =
        ::taihe::make_holder<MyCallbackImpl, ::taihe::callback<void(bool)>>();
    RegisterShutdownCallback(shutdownCb);

    std::shared_ptr<PowerShutdownCallback> cb = std::make_shared<PowerShutdownCallback>();
    cb->CreateCallback(shutdownCb);
    bool isReboot = false;
    cb->OnAsyncShutdownOrReboot(isReboot);
    EXPECT_TRUE(g_cbFlag);
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_010 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_011
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICTIYR
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_011, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_011 start");
    struct MyCallbackImpl {
        void operator()(MyUndefined const&)
        {
            g_cbFlag = true;
        }
    };
    callback<void(MyUndefined const&)> shutdownCb =
        ::taihe::make_holder<MyCallbackImpl, ::taihe::callback<void(MyUndefined const&)>>();
    optional<callback<void(MyUndefined const&)>> cb =
        optional<callback<void(MyUndefined const&)>>{std::in_place, shutdownCb};
    UnregisterShutdownCallback(cb);
    EXPECT_TRUE(g_cbFlag);
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_011 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_012
 * @tc.desc: test power taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICTIYR
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_012, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_012 start");
    g_error = PowerErrors::ERR_OK;
    SetPowerKeyFilteringStrategy(ohos::power::PowerKeyFilteringStrategy::key_t::DISABLE_LONG_PRESS_FILTERING);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_OK));
    EXPECT_TRUE(g_pass);

    g_error = PowerErrors::ERR_SYSTEM_API_DENIED;
    SetPowerKeyFilteringStrategy(ohos::power::PowerKeyFilteringStrategy::key_t::LONG_PRESS_FILTERING_ONCE);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_SYSTEM_API_DENIED));
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_012 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_013
 * @tc.desc: test GetPowerConfig interface
 * @tc.type: FUNC
 * @tc.require: issue#IXXXXX
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_013, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_013 start");
    string_view sceneName = "test_scene";
    g_configVal = "test_value";
    
    std::string result = std::string(GetPowerConfig(sceneName));
    EXPECT_TRUE(g_pass);
    EXPECT_EQ(result, "test_value");
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_013 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_014
 * @tc.desc: test GetPowerConfig with error code
 * @tc.type: FUNC
 * @tc.require: issue#IXXXXX
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_014, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_014 start");
    string_view sceneName = "test_scene";
    
    g_error = PowerErrors::ERR_FAILURE;
    std::string result = std::string(GetPowerConfig(sceneName));
    EXPECT_TRUE(g_errCode != static_cast<int32_t>(PowerErrors::ERR_FAILURE));
    
    g_error = PowerErrors::ERR_CONNECTION_FAIL;
    result = GetPowerConfig(sceneName);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL));
    EXPECT_TRUE(g_pass);
    
    g_error = PowerErrors::ERR_READ_OPERATION_FAILED;
    result = GetPowerConfig(sceneName);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_READ_OPERATION_FAILED));
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_014 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_015
 * @tc.desc: test SetPowerConfig interface
 * @tc.type: FUNC
 * @tc.require: issue#IXXXXX
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_015, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_015 start");
    string_view sceneName = "test_scene";
    string_view configVal = "new_value";
    
    SetPowerConfig(sceneName, configVal);
    EXPECT_TRUE(g_pass);
    EXPECT_EQ(g_configVal, "new_value");
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_015 end");
}

/**
 * @tc.name: PowerTaiheNativeTest_016
 * @tc.desc: test SetPowerConfig with error code
 * @tc.type: FUNC
 * @tc.require: issue#IXXXXX
 */
HWTEST_F(PowerTaiheNativeTest, PowerTaiheNativeTest_016, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_016 start");
    string_view sceneName = "test_scene";
    string_view configVal = "new_value";
    
    g_error = PowerErrors::ERR_FAILURE;
    SetPowerConfig(sceneName, configVal);
    EXPECT_TRUE(g_errCode != static_cast<int32_t>(PowerErrors::ERR_FAILURE));
    
    g_error = PowerErrors::ERR_PERMISSION_DENIED;
    SetPowerConfig(sceneName, configVal);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_PERMISSION_DENIED));
    EXPECT_TRUE(g_pass);
    
    g_error = PowerErrors::ERR_WRITE_OPERATION_FAILED;
    SetPowerConfig(sceneName, configVal);
    EXPECT_TRUE(g_errCode == static_cast<int32_t>(PowerErrors::ERR_WRITE_OPERATION_FAILED));
    EXPECT_TRUE(g_pass);
    POWER_HILOGI(LABEL_TEST, "PowerTaiheNativeTest_016 end");
}
class TestHibernateCallback {
public:
    explicit TestHibernateCallback(std::shared_ptr<std::atomic<bool>> entered) : entered_(std::move(entered)) {}

    taihe::future<taihe::expected<void, taihe::error>> onHibernate()
    {
        entered_->store(true);
        auto [complete, future] = taihe::make_async_pair<taihe::expected<void, taihe::error>>();
        std::thread([complete = std::move(complete)]() mutable {
            std::this_thread::sleep_for(std::chrono::milliseconds(HIBERNATE_CALLBACK_TEST_DELAY_MS));
            complete.complete();
        }).detach();
        return std::move(future);
    }

    taihe::future<taihe::expected<void, taihe::error>> onWakeup(bool hibernateResult)
    {
        (void)hibernateResult;
        auto [complete, future] = taihe::make_async_pair<taihe::expected<void, taihe::error>>();
        complete.complete();
        return std::move(future);
    }

private:
    std::shared_ptr<std::atomic<bool>> entered_;
};

/**
 * @tc.name: HibernateCallbackWaitsForPromise
 * @tc.desc: test OnSyncHibernate waits for onHibernate promise
 * @tc.type: FUNC
 * @tc.require: issue#IXXXXX
 */
HWTEST_F(PowerTaiheNativeTest, HibernateCallbackWaitsForPromise, TestSize.Level1)
{
    auto entered = std::make_shared<std::atomic<bool>>(false);
    auto callback = taihe::make_holder<TestHibernateCallback, ohos::power::HibernateCallback>(entered);
    OHOS::sptr<PowerHibernateCallback> stub = new PowerHibernateCallback();
    stub->CreateCallback(callback);

    int64_t minWaitMs = 40; // ms
    int64_t maxWaitMs = 3000; // ms
    auto start = std::chrono::steady_clock::now();
    stub->OnSyncHibernate();
    auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_TRUE(entered->load());
    EXPECT_GE(elapsed, std::chrono::milliseconds(minWaitMs));
    EXPECT_LT(elapsed, std::chrono::milliseconds(maxWaitMs));
    stub->ReleaseCallback();
}
class TestWakeupHibernateCallback {
public:
    explicit TestWakeupHibernateCallback(std::shared_ptr<std::atomic<int32_t>> wakeupResult)
        : wakeupResult_(std::move(wakeupResult)) {}

    taihe::future<taihe::expected<void, taihe::error>> onHibernate()
    {
        auto [complete, future] = taihe::make_async_pair<taihe::expected<void, taihe::error>>();
        complete.complete();
        return std::move(future);
    }

    taihe::future<taihe::expected<void, taihe::error>> onWakeup(bool hibernateResult)
    {
        wakeupResult_->store(hibernateResult ? 1 : 0);
        auto [complete, future] = taihe::make_async_pair<taihe::expected<void, taihe::error>>();
        complete.complete();
        return std::move(future);
    }

private:
    std::shared_ptr<std::atomic<int32_t>> wakeupResult_;
};

/**
 * @tc.name: RegisterHibernateCallbackInvalidHandle
 * @tc.desc: test RegisterHibernateCallback rejects an invalid callback handle
 * @tc.type: FUNC
 */
HWTEST_F(PowerTaiheNativeTest, RegisterHibernateCallbackInvalidHandle, TestSize.Level1)
{
    ohos::power::weak::HibernateCallback callback({nullptr, nullptr});
    RegisterHibernateCallback(callback);
    EXPECT_EQ(g_errCode, static_cast<int32_t>(PowerErrors::ERR_PARAM_INVALID));
    EXPECT_FALSE(g_pass);
}

/**
 * @tc.name: RegisterHibernateCallbackSystemDenied
 * @tc.desc: test RegisterHibernateCallback with system api denied
 * @tc.type: FUNC
 * @tc.require: issue#IXXXXX
 */
HWTEST_F(PowerTaiheNativeTest, RegisterHibernateCallbackSystemDenied, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RegisterHibernateCallbackSystemDenied start");
    auto entered = std::make_shared<std::atomic<bool>>(false);
    auto callback = taihe::make_holder<TestHibernateCallback, ohos::power::HibernateCallback>(entered);
    g_error = PowerErrors::ERR_SYSTEM_API_DENIED;
    RegisterHibernateCallback(callback);
    EXPECT_TRUE(g_pass);
    EXPECT_EQ(g_errCode, static_cast<int32_t>(PowerErrors::ERR_SYSTEM_API_DENIED));
    POWER_HILOGI(LABEL_TEST, "RegisterHibernateCallbackSystemDenied end");
}

/**
 * @tc.name: RegisterHibernateCallbackPermissionDenied
 * @tc.desc: test RegisterHibernateCallback with permission denied
 * @tc.type: FUNC
 * @tc.require: issue#IXXXXX
 */
HWTEST_F(PowerTaiheNativeTest, RegisterHibernateCallbackPermissionDenied, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RegisterHibernateCallbackPermissionDenied start");
    auto entered = std::make_shared<std::atomic<bool>>(false);
    auto callback = taihe::make_holder<TestHibernateCallback, ohos::power::HibernateCallback>(entered);
    g_error = PowerErrors::ERR_PERMISSION_DENIED;
    RegisterHibernateCallback(callback);
    EXPECT_TRUE(g_pass);
    EXPECT_EQ(g_errCode, static_cast<int32_t>(PowerErrors::ERR_PERMISSION_DENIED));
    POWER_HILOGI(LABEL_TEST, "RegisterHibernateCallbackPermissionDenied end");
}

/**
 * @tc.name: RegisterHibernateCallbackClientFailed
 * @tc.desc: test RegisterHibernateCallback with client failed
 * @tc.type: FUNC
 * @tc.require: issue#IXXXXX
 */
HWTEST_F(PowerTaiheNativeTest, RegisterHibernateCallbackClientFailed, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RegisterHibernateCallbackClientFailed start");
    auto entered = std::make_shared<std::atomic<bool>>(false);
    auto callback = taihe::make_holder<TestHibernateCallback, ohos::power::HibernateCallback>(entered);
    g_error = PowerErrors::ERR_CONNECTION_FAIL;
    RegisterHibernateCallback(callback);
    EXPECT_TRUE(g_pass);
    EXPECT_EQ(g_errCode, static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL));
    POWER_HILOGI(LABEL_TEST, "RegisterHibernateCallbackClientFailed end");
}

/**
 * @tc.name: RegisterHibernateCallbackSuccess
 * @tc.desc: test RegisterHibernateCallback success
 * @tc.type: FUNC
 * @tc.require: issue#IXXXXX
 */
HWTEST_F(PowerTaiheNativeTest, RegisterHibernateCallbackSuccess, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RegisterHibernateCallbackSuccess start");
    auto entered = std::make_shared<std::atomic<bool>>(false);
    auto callback = taihe::make_holder<TestHibernateCallback, ohos::power::HibernateCallback>(entered);
    RegisterHibernateCallback(callback);
    EXPECT_TRUE(g_pass);
    EXPECT_EQ(g_errCode, 0);
    UnregisterHibernateCallback();
    POWER_HILOGI(LABEL_TEST, "RegisterHibernateCallbackSuccess end");
}

/**
 * @tc.name: UnregisterHibernateCallbackSystemDenied
 * @tc.desc: test UnregisterHibernateCallback with system api denied
 * @tc.type: FUNC
 * @tc.require: issue#IXXXXX
 */
HWTEST_F(PowerTaiheNativeTest, UnregisterHibernateCallbackSystemDenied, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "UnregisterHibernateCallbackSystemDenied start");
    g_error = PowerErrors::ERR_SYSTEM_API_DENIED;
    UnregisterHibernateCallback();
    EXPECT_TRUE(g_pass);
    EXPECT_EQ(g_errCode, static_cast<int32_t>(PowerErrors::ERR_SYSTEM_API_DENIED));
    POWER_HILOGI(LABEL_TEST, "UnregisterHibernateCallbackSystemDenied end");
}

/**
 * @tc.name: UnregisterHibernateCallbackPermissionDenied
 * @tc.desc: test UnregisterHibernateCallback with permission denied
 * @tc.type: FUNC
 * @tc.require: issue#IXXXXX
 */
HWTEST_F(PowerTaiheNativeTest, UnregisterHibernateCallbackPermissionDenied, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "UnregisterHibernateCallbackPermissionDenied start");
    g_error = PowerErrors::ERR_PERMISSION_DENIED;
    UnregisterHibernateCallback();
    EXPECT_TRUE(g_pass);
    EXPECT_EQ(g_errCode, static_cast<int32_t>(PowerErrors::ERR_PERMISSION_DENIED));
    POWER_HILOGI(LABEL_TEST, "UnregisterHibernateCallbackPermissionDenied end");
}

/**
 * @tc.name: UnregisterHibernateCallbackClientFailed
 * @tc.desc: test UnregisterHibernateCallback with client failed
 * @tc.type: FUNC
 * @tc.require: issue#IXXXXX
 */
HWTEST_F(PowerTaiheNativeTest, UnregisterHibernateCallbackClientFailed, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "UnregisterHibernateCallbackClientFailed start");
    g_error = PowerErrors::ERR_CONNECTION_FAIL;
    UnregisterHibernateCallback();
    EXPECT_TRUE(g_pass);
    EXPECT_EQ(g_errCode, static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL));
    POWER_HILOGI(LABEL_TEST, "UnregisterHibernateCallbackClientFailed end");
}

/**
 * @tc.name: UnregisterHibernateCallbackSuccess
 * @tc.desc: test UnregisterHibernateCallback success
 * @tc.type: FUNC
 * @tc.require: issue#IXXXXX
 */
HWTEST_F(PowerTaiheNativeTest, UnregisterHibernateCallbackSuccess, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "UnregisterHibernateCallbackSuccess start");
    UnregisterHibernateCallback();
    EXPECT_TRUE(g_pass);
    EXPECT_EQ(g_errCode, 0);
    POWER_HILOGI(LABEL_TEST, "UnregisterHibernateCallbackSuccess end");
}

/**
 * @tc.name: HibernateCallbackSkipsWithoutCallback
 * @tc.desc: test OnSyncHibernate skips without callback
 * @tc.type: FUNC
 * @tc.require: issue#IXXXXX
 */
HWTEST_F(PowerTaiheNativeTest, HibernateCallbackSkipsWithoutCallback, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "HibernateCallbackSkipsWithoutCallback start");
    OHOS::sptr<PowerHibernateCallback> stub = new PowerHibernateCallback();
    int64_t maxWaitMs = 100; // ms
    auto start = std::chrono::steady_clock::now();
    stub->OnSyncHibernate();
    stub->OnSyncWakeup(false);
    auto elapsed = std::chrono::steady_clock::now() - start;
    EXPECT_LT(elapsed, std::chrono::milliseconds(maxWaitMs));
    POWER_HILOGI(LABEL_TEST, "HibernateCallbackSkipsWithoutCallback end");
}

/**
 * @tc.name: HibernateCallbackOnWakeupWaitsForPromise
 * @tc.desc: test OnSyncWakeup waits for onWakeup promise
 * @tc.type: FUNC
 * @tc.require: issue#IXXXXX
 */
HWTEST_F(PowerTaiheNativeTest, HibernateCallbackOnWakeupWaitsForPromise, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "HibernateCallbackOnWakeupWaitsForPromise start");
    auto wakeupResult = std::make_shared<std::atomic<int32_t>>(-1);
    auto callback = taihe::make_holder<TestWakeupHibernateCallback, ohos::power::HibernateCallback>(wakeupResult);
    OHOS::sptr<PowerHibernateCallback> stub = new PowerHibernateCallback();
    stub->CreateCallback(callback);

    int64_t maxWaitMs = 3000; // ms
    auto start = std::chrono::steady_clock::now();
    stub->OnSyncWakeup(true);
    auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_EQ(wakeupResult->load(), 1);
    EXPECT_LT(elapsed, std::chrono::milliseconds(maxWaitMs));
    stub->ReleaseCallback();
    POWER_HILOGI(LABEL_TEST, "HibernateCallbackOnWakeupWaitsForPromise end");
}
} // namespace

