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

#include "parallel_shutdown_callback_test.h"

#include <atomic>
#include <chrono>
#include <thread>

#include "power_log.h"
#include "power_mgr_client.h"
#define private public
#include "power_mgr_service.h"
#undef private
#include "shutdown/shutdown_client.h"

#include "mock_power_action.h"
#include "mock_state_action.h"

namespace OHOS {
namespace PowerMgr {
namespace UnitTest {
namespace {
sptr<PowerMgrService> g_service = nullptr;
MockPowerAction* g_mockPowerAction = nullptr;
MockStateAction* g_mockStateAction = nullptr;
std::atomic<bool> g_callback1Called{false};
std::atomic<bool> g_callback2Called{false};
std::atomic<bool> g_callback3Called{false};
std::atomic<int32_t> g_voteCount{0};
}
using namespace testing::ext;

void ParallelShutdownCallbackTest::SetUpTestCase()
{
    g_service = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_service->OnStart();
}

void ParallelShutdownCallbackTest::TearDownTestCase()
{
    g_service->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

void ParallelShutdownCallbackTest::SetUp()
{
    g_callback1Called = false;
    g_callback2Called = false;
    g_callback3Called = false;
    g_voteCount = 0;
    g_mockPowerAction = new MockPowerAction();
    g_mockStateAction = new MockStateAction();
    auto shutdownController = g_service->GetShutdownController();
    shutdownController->EnableMock(g_mockPowerAction, g_mockStateAction);
}

void ParallelShutdownCallbackTest::TearDown()
{
    auto shutdownController = g_service->GetShutdownController();
    while (shutdownController->started_) {
        std::this_thread::yield();
    }
    shutdownController->EnableMock(nullptr, nullptr);
}

void ParallelShutdownCallbackTest::ParallelShutdownCallback::OnParallelShutdownOrReboot(bool isReboot, uint64_t identity)
{
    g_callback1Called = true;
    g_voteCount++;
    g_service->GetShutdownController()->VoteCallback(identity);
}

void ParallelShutdownCallbackTest::ParallelShutdownCallback2::OnParallelShutdownOrReboot(bool isReboot, uint64_t identity)
{
    g_callback2Called = true;
    g_voteCount++;
    g_service->GetShutdownController()->VoteCallback(identity);
}

void ParallelShutdownCallbackTest::ParallelShutdownCallback3::OnParallelShutdownOrReboot(bool isReboot, uint64_t identity)
{
    g_callback3Called = true;
    g_voteCount++;
    g_service->GetShutdownController()->VoteCallback(identity);
}

/**
 * @tc.name: ParallelShutdownCallback001
 * @tc.desc: Test parallel shutdown callback registration and trigger (AC-1.1)
 * @tc.type: FUNC
 */
HWTEST_F(ParallelShutdownCallbackTest, ParallelShutdownCallback001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ParallelShutdownCallback001 function start!");
    auto callback = new ParallelShutdownCallback();
    g_service->RegisterShutdownCallback(callback);

    EXPECT_CALL(*g_mockPowerAction, Reboot(std::string("test_case"))).Times(::testing::AtLeast(1));
    g_service->RebootDevice("test_case");

    auto shutdownController = g_service->GetShutdownController();
    while (shutdownController->started_) {
        std::this_thread::yield();
    }
    EXPECT_TRUE(g_callback1Called);
    POWER_HILOGI(LABEL_TEST, "ParallelShutdownCallback001 function end!");
}

/**
 * @tc.name: ParallelShutdownCallback002
 * @tc.desc: Test parallel shutdown callback unregistration (AC-1.2)
 * @tc.type: FUNC
 */
HWTEST_F(ParallelShutdownCallbackTest, ParallelShutdownCallback002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ParallelShutdownCallback002 function start!");
    auto callback = new ParallelShutdownCallback();
    g_service->RegisterShutdownCallback(callback);
    g_service->UnRegisterShutdownCallback(callback);

    EXPECT_CALL(*g_mockPowerAction, Shutdown(std::string("test_case"))).Times(::testing::AtLeast(1));
    g_service->ShutDownDevice("test_case");

    auto shutdownController = g_service->GetShutdownController();
    while (shutdownController->started_) {
        std::this_thread::yield();
    }
    EXPECT_FALSE(g_callback1Called);
    POWER_HILOGI(LABEL_TEST, "ParallelShutdownCallback002 function end!");
}

/**
 * @tc.name: ParallelShutdownCallback003
 * @tc.desc: Test multiple parallel shutdown callbacks (AC-1.3)
 * @tc.type: FUNC
 */
HWTEST_F(ParallelShutdownCallbackTest, ParallelShutdownCallback003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ParallelShutdownCallback003 function start!");
    auto callback1 = new ParallelShutdownCallback();
    auto callback2 = new ParallelShutdownCallback2();
    auto callback3 = new ParallelShutdownCallback3();
    g_service->RegisterShutdownCallback(callback1);
    g_service->RegisterShutdownCallback(callback2);
    g_service->RegisterShutdownCallback(callback3);

    EXPECT_CALL(*g_mockPowerAction, Shutdown(std::string("test_case"))).Times(::testing::AtLeast(1));
    g_service->ShutDownDevice("test_case");

    auto shutdownController = g_service->GetShutdownController();
    while (shutdownController->started_) {
        std::this_thread::yield();
    }
    EXPECT_TRUE(g_callback1Called);
    EXPECT_TRUE(g_callback2Called);
    EXPECT_TRUE(g_callback3Called);
    POWER_HILOGI(LABEL_TEST, "ParallelShutdownCallback003 function end!");
}

/**
 * @tc.name: ParallelShutdownVote001
 * @tc.desc: Test all votes complete -> proceed (AC-2.1)
 * @tc.type: FUNC
 */
HWTEST_F(ParallelShutdownCallbackTest, ParallelShutdownVote001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ParallelShutdownVote001 function start!");
    auto callback1 = new ParallelShutdownCallback();
    auto callback2 = new ParallelShutdownCallback2();
    g_service->RegisterShutdownCallback(callback1);
    g_service->RegisterShutdownCallback(callback2);

    EXPECT_CALL(*g_mockPowerAction, Reboot(std::string("test_case"))).Times(::testing::AtLeast(1));
    g_service->RebootDevice("test_case");

    auto shutdownController = g_service->GetShutdownController();
    while (shutdownController->started_) {
        std::this_thread::yield();
    }
    EXPECT_TRUE(g_callback1Called);
    EXPECT_TRUE(g_callback2Called);
    EXPECT_EQ(g_voteCount.load(), 2);
    POWER_HILOGI(LABEL_TEST, "ParallelShutdownVote001 function end!");
}

/**
 * @tc.name: ParallelShutdownVote002
 * @tc.desc: Test vote has no veto semantics - shutdown proceeds (AC-4.1)
 * @tc.type: FUNC
 */
HWTEST_F(ParallelShutdownCallbackTest, ParallelShutdownVote002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ParallelShutdownVote002 function start!");
    auto callback = new ParallelShutdownCallback();
    g_service->RegisterShutdownCallback(callback);

    EXPECT_CALL(*g_mockPowerAction, Shutdown(std::string("test_case"))).Times(::testing::AtLeast(1));
    g_service->ShutDownDevice("test_case");

    auto shutdownController = g_service->GetShutdownController();
    while (shutdownController->started_) {
        std::this_thread::yield();
    }
    EXPECT_TRUE(g_callback1Called);
    EXPECT_EQ(g_voteCount.load(), 1);
    POWER_HILOGI(LABEL_TEST, "ParallelShutdownVote002 function end!");
}

/**
 * @tc.name: ParallelShutdownCallbackRegression001
 * @tc.desc: Test existing sync/async callbacks unchanged when no parallel callbacks (AC-5.1)
 * @tc.type: FUNC
 */
HWTEST_F(ParallelShutdownCallbackTest, ParallelShutdownCallbackRegression001, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "ParallelShutdownCallbackRegression001 function start!");

    EXPECT_CALL(*g_mockPowerAction, Reboot(std::string("test_case"))).Times(::testing::AtLeast(1));
    g_service->RebootDevice("test_case");

    auto shutdownController = g_service->GetShutdownController();
    while (shutdownController->started_) {
        std::this_thread::yield();
    }
    EXPECT_FALSE(g_callback1Called);
    EXPECT_FALSE(g_callback2Called);
    EXPECT_FALSE(g_callback3Called);
    POWER_HILOGI(LABEL_TEST, "ParallelShutdownCallbackRegression001 function end!");
}
} // namespace UnitTest
} // namespace PowerMgr
} // namespace OHOS
