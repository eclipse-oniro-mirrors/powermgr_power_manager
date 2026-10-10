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

#include <gtest/gtest.h>
#include <system_ability_definition.h>

#include <csignal>
#include <cstdlib>

#ifdef THERMAL_GTEST
#define private   public
#define protected public
#endif

#include "power_common.h"
#include "power_log.h"
#include "power_mgr_notify.h"
#include "power_mgr_service.h"
#include "multi_user_screen_adapter.h"
#include "mock/multi_user_test_mock.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using DisplayErrors = OHOS::DisplayPowerMgr::DisplayErrors;
using MultiScreenStateChangeReason = OHOS::DisplayPowerMgr::MultiScreenStateChangeReason;

namespace {
extern "C" void __gcov_dump(void);
// Flush gcov .gcda on a fatal signal: the process crashes in a static destructor at exit (after the
// suite), which would otherwise skip the gcov atexit flush and yield empty coverage data.
static void FlushGcovAndDie(int sig)
{
    __gcov_dump();
    _exit(128 + sig);
}
sptr<PowerMgrService> g_pms;

class MultiUserScreenAdapterTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        g_pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
        ASSERT_TRUE(g_pms != nullptr);
        g_pms->OnStart();
        signal(SIGSEGV, FlushGcovAndDie);
        signal(SIGABRT, FlushGcovAndDie);
        signal(SIGILL, FlushGcovAndDie);
    }
    static void TearDownTestCase() { __gcov_dump(); }
    void SetUp() override
    {
        ASSERT_TRUE(g_pms != nullptr);
        TestMock::ResetDisplayMock();
        TestMock::ResetOsAccountMock();
        TestMock::ResetCommonEventMock();
        // Ensure the notify module is ready for HandleMultiUserScreenChange (allocates screen wants).
        if (g_pms->powerMgrNotify_ == nullptr) {
            g_pms->powerMgrNotify_ = std::make_shared<PowerMgrNotify>();
        }
        g_pms->powerMgrNotify_->RegisterPublishEvents();
        g_pms->screenQueryCallingUid_ = -1; // reset caller-uid seam
#ifdef POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION
        MultiUserScreenAdapter::multiScreenCallback_ = nullptr;
        MultiUserScreenAdapter::SetScreenChangeHandler(nullptr);
#endif
    }
    void TearDown() override
    {
        g_pms->screenQueryCallingUid_ = -1;
    }
};

// ---- Pure-local caller-identity helpers (no IPC) ----
HWTEST_F(MultiUserScreenAdapterTest, GetUserIdFromUid_PureArithmetic, TestSize.Level1)
{
    EXPECT_EQ(MultiUserScreenAdapter::GetUserIdFromUid(0), 0);                  // system -> localId 0
    EXPECT_EQ(MultiUserScreenAdapter::GetUserIdFromUid(200000), 1);             // local account id = uid/200000
    EXPECT_EQ(MultiUserScreenAdapter::GetUserIdFromUid(100 * 200000), 100);    // U100
    EXPECT_EQ(MultiUserScreenAdapter::GetUserIdFromUid(-1), MultiUserScreenAdapter::INVALID_USER_ID);
    EXPECT_FALSE(MultiUserScreenAdapter::IsMultiUserUid(0));
    EXPECT_FALSE(MultiUserScreenAdapter::IsMultiUserUid(200000));                // localId 1 < START_MULTIPLE_USER_ID
    EXPECT_TRUE(MultiUserScreenAdapter::IsMultiUserUid(100 * 200000));          // localId 100 is multi-user
}

// ---- MultiUserScreenAdapter::IsDisplayOn (display_manager mock) ----
HWTEST_F(MultiUserScreenAdapterTest, IsDisplayOn_StateBranches, TestSize.Level1)
{
    TestMock::MockDisplayGetStateRet() = DisplayErrors::ERR_OK;
    TestMock::MockDisplayStateValue() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_ON;
    EXPECT_TRUE(MultiUserScreenAdapter::IsDisplayOn(0));
    TestMock::MockDisplayStateValue() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_DIM;
    EXPECT_TRUE(MultiUserScreenAdapter::IsDisplayOn(0));
    TestMock::MockDisplayStateValue() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_OFF;
    EXPECT_FALSE(MultiUserScreenAdapter::IsDisplayOn(0));
    TestMock::MockDisplayStateValue() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN;
    EXPECT_FALSE(MultiUserScreenAdapter::IsDisplayOn(0));
    // display_manager failure path -> conservatively off
    TestMock::MockDisplayGetStateRet() = static_cast<DisplayErrors>(1);
    EXPECT_FALSE(MultiUserScreenAdapter::IsDisplayOn(0));
}

// ---- MultiUserScreenAdapter::IsAnyScreenOn (whole-machine query + fallback) ----
HWTEST_F(MultiUserScreenAdapterTest, IsAnyScreenOn_AggregateAndFallback, TestSize.Level1)
{
    TestMock::MockDisplayGetStateRet() = DisplayErrors::ERR_OK;
    TestMock::MockDisplayIds() = {0, 1};
    TestMock::MockDisplayStateValue() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_OFF;
    EXPECT_FALSE(MultiUserScreenAdapter::IsAnyScreenOn());          // both off
    TestMock::MockDisplayStateValue() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_ON;
    EXPECT_TRUE(MultiUserScreenAdapter::IsAnyScreenOn());           // any on
    // exclude the only queried display that is on (display 0); display 1 still on -> true
    EXPECT_TRUE(MultiUserScreenAdapter::IsAnyScreenOn(0));
    // both off, exclude display 0 -> no usable info among the rest -> still queried (id 1) -> false
    TestMock::MockDisplayStateValue() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_OFF;
    EXPECT_FALSE(MultiUserScreenAdapter::IsAnyScreenOn(0));
    // single-screen fallback: empty display ids -> main display state
    TestMock::MockDisplayIds() = {};
    TestMock::MockMainDisplayState() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_ON;
    EXPECT_TRUE(MultiUserScreenAdapter::IsAnyScreenOn());
    TestMock::MockMainDisplayState() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_OFF;
    EXPECT_FALSE(MultiUserScreenAdapter::IsAnyScreenOn());
}

// ---- OsAccount display<->user mapping (mock) ----
HWTEST_F(MultiUserScreenAdapterTest, OsAccountMapping_SuccessAndFailure, TestSize.Level1)
{
    TestMock::MockLocalIdRet() = ERR_OK;
    TestMock::MockLocalId() = 100;
    EXPECT_EQ(MultiUserScreenAdapter::GetUserIdByDisplayId(0), 100);
    TestMock::MockLocalIdRet() = static_cast<ErrCode>(-1);
    EXPECT_EQ(MultiUserScreenAdapter::GetUserIdByDisplayId(0), MultiUserScreenAdapter::INVALID_USER_ID);

    TestMock::MockDisplayIdRet() = ERR_OK;
    TestMock::MockDisplayId() = 7;
    EXPECT_EQ(MultiUserScreenAdapter::GetDisplayIdByUserId(100), 7u);
    TestMock::MockDisplayIdRet() = static_cast<ErrCode>(-1);
    EXPECT_EQ(MultiUserScreenAdapter::GetDisplayIdByUserId(100), MultiUserScreenAdapter::INVALID_DISPLAY_ID);
}

// ---- MultiUserScreenAdapter::IsScreenOnForUser ----
HWTEST_F(MultiUserScreenAdapterTest, IsScreenOnForUser_ResolvedAndFailure, TestSize.Level1)
{
    TestMock::MockDisplayIdRet() = ERR_OK;
    TestMock::MockDisplayId() = 5;
    TestMock::MockDisplayGetStateRet() = DisplayErrors::ERR_OK;
    TestMock::MockDisplayStateValue() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_ON;
    EXPECT_TRUE(MultiUserScreenAdapter::IsScreenOnForUser(100));
    TestMock::MockDisplayStateValue() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_OFF;
    EXPECT_FALSE(MultiUserScreenAdapter::IsScreenOnForUser(100));
    // display resolution failure -> conservative screen off
    TestMock::MockDisplayIdRet() = static_cast<ErrCode>(-1);
    EXPECT_FALSE(MultiUserScreenAdapter::IsScreenOnForUser(100));
}

// ---- Register/Unregister callback (adapter-level, drives RegisterScreenStateCallback entry) ----
HWTEST_F(MultiUserScreenAdapterTest, RegisterUnregisterScreenStateCallback, TestSize.Level1)
{
    TestMock::MockDisplayRegisterRet() = DisplayErrors::ERR_OK;
    EXPECT_TRUE(MultiUserScreenAdapter::RegisterScreenStateCallback());
    EXPECT_GT(TestMock::MockRegisterCallCount(), 0);
    // idempotent: a second register reuses the cached callback object
    EXPECT_TRUE(MultiUserScreenAdapter::RegisterScreenStateCallback());
    EXPECT_TRUE(MultiUserScreenAdapter::UnregisterScreenStateCallback());
    EXPECT_GT(TestMock::MockUnregisterCallCount(), 0);
    // nothing registered now
    EXPECT_FALSE(MultiUserScreenAdapter::UnregisterScreenStateCallback());
    // register failure
    TestMock::MockDisplayRegisterRet() = static_cast<DisplayErrors>(1);
    EXPECT_FALSE(MultiUserScreenAdapter::RegisterScreenStateCallback());
}

// ---- T6: PowerMgrNotify::PublishScreenOnEventsToUser / PublishScreenOffEventsToUser (real entry) ----
HWTEST_F(MultiUserScreenAdapterTest, PublishScreenOnOffEventsToUser, TestSize.Level1)
{
    auto notify = std::make_shared<PowerMgrNotify>();
    notify->RegisterPublishEvents();
    TestMock::ResetCommonEventMock();
    notify->PublishScreenOnEventsToUser(1, "reason", 100);
    EXPECT_EQ(TestMock::MockPublishAsUserOnCount(), 1);
    EXPECT_EQ(TestMock::MockPublishAsUserLastUserId(), 100);
    notify->PublishScreenOffEventsToUser(1, "reason", 100);
    EXPECT_EQ(TestMock::MockPublishAsUserOffCount(), 1);
    // null want guard
    PowerMgrNotify emptyNotify;
    TestMock::ResetCommonEventMock();
    emptyNotify.PublishScreenOnEventsToUser(1, "reason", 100);
    EXPECT_EQ(TestMock::MockPublishAsUserCallCount(), 0);
}

// ---- T2: PowerMgrService::IsScreenOn SA/U0 branch (flag on) -> IsAnyScreenOn (real entry) ----
HWTEST_F(MultiUserScreenAdapterTest, IsScreenOn_SystemUser_AnyScreenOn, TestSize.Level1)
{
    g_pms->screenQueryCallingUid_ = 0; // force SA/U0 identity
    TestMock::MockDisplayIds() = {0};
    TestMock::MockDisplayGetStateRet() = DisplayErrors::ERR_OK;
    TestMock::MockDisplayStateValue() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_ON;
    EXPECT_TRUE(g_pms->IsScreenOn());
    EXPECT_GT(TestMock::MockGetStateCallCount(), 0); // adapter reached the display_manager mock
    TestMock::MockDisplayStateValue() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_OFF;
    EXPECT_FALSE(g_pms->IsScreenOn());
}

// ---- T3: PowerMgrService::IsScreenOn U10X branch (flag on) -> IsScreenOnForUser (real entry) ----
HWTEST_F(MultiUserScreenAdapterTest, IsScreenOn_MultiUserUser_PerDisplay, TestSize.Level1)
{
    constexpr int32_t u100Uid = 100 * MultiUserScreenAdapter::UID_TRANSFORM_DIVISOR;
    g_pms->screenQueryCallingUid_ = u100Uid;
    TestMock::MockDisplayIdRet() = ERR_OK;
    TestMock::MockDisplayId() = 5;
    TestMock::MockDisplayGetStateRet() = DisplayErrors::ERR_OK;
    TestMock::MockDisplayStateValue() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_ON;
    EXPECT_TRUE(g_pms->IsScreenOn());
    EXPECT_GT(TestMock::MockDisplayIdCallCount(), 0); // GetDisplayIdByUserId reached the account mock
    TestMock::MockDisplayStateValue() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_OFF;
    EXPECT_FALSE(g_pms->IsScreenOn());
    TestMock::MockDisplayIdRet() = static_cast<ErrCode>(-1); // resolution failure -> off
    EXPECT_FALSE(g_pms->IsScreenOn());
    g_pms->screenQueryCallingUid_ = -1;
}

// ---- T4/T5: MultiScreenCallback::OnMultiScreenDisplayStateChanged (real callback entry) ----
HWTEST_F(MultiUserScreenAdapterTest, OnMultiScreenDisplayStateChanged_PerUserAndWholeMachine, TestSize.Level1)
{
    // Route the normalized event into the service handler (same wiring as OnAddSystemAbility).
    g_pms->RegisterMultiUserScreenCallback();
    sptr<MultiUserScreenAdapter::MultiScreenCallback> cb = new MultiUserScreenAdapter::MultiScreenCallback();
    ASSERT_NE(cb, nullptr);

    TestMock::MockDisplayGetStateRet() = DisplayErrors::ERR_OK;
    TestMock::MockDisplayIds() = {0, 1};

    // T4: per-user broadcast only. display 0 -> userId 100; display 1 still on -> no whole-machine flip.
    TestMock::MockLocalIdRet() = ERR_OK;
    TestMock::MockLocalId() = 100;
    TestMock::MockDisplayStateValue() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_ON;
    TestMock::ResetCommonEventMock();
    cb->OnMultiScreenDisplayStateChanged(0, "scr0", OHOS::DisplayPowerMgr::DisplayState::DISPLAY_ON,
        MultiScreenStateChangeReason::STATE_CHANGE_REASON_DEFAULT);
    EXPECT_EQ(TestMock::MockPublishAsUserOnCount(), 1);
    EXPECT_EQ(TestMock::MockPublishAsUserLastUserId(), 100);
    EXPECT_EQ(TestMock::MockWholeMachineCallCount(), 0);

    // T5a: whole-machine SCREEN_ON. all-off -> any-on, per-user resolution fails -> skip per-user.
    TestMock::MockLocalIdRet() = static_cast<ErrCode>(-1);
    TestMock::MockDisplayStateValue() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_OFF; // other screens off
    TestMock::ResetCommonEventMock();
    cb->OnMultiScreenDisplayStateChanged(0, "scr0", OHOS::DisplayPowerMgr::DisplayState::DISPLAY_ON,
        MultiScreenStateChangeReason::STATE_CHANGE_REASON_DEFAULT);
    EXPECT_EQ(TestMock::MockPublishAsUserCallCount(), 0);
    EXPECT_EQ(TestMock::MockWholeMachineOnCount(), 1);

    // T5b: whole-machine SCREEN_OFF. any-on -> all-off.
    TestMock::ResetCommonEventMock();
    cb->OnMultiScreenDisplayStateChanged(0, "scr0", OHOS::DisplayPowerMgr::DisplayState::DISPLAY_OFF,
        MultiScreenStateChangeReason::STATE_CHANGE_REASON_DEFAULT);
    EXPECT_EQ(TestMock::MockWholeMachineOffCount(), 1);

    // Screen off but another screen on -> no broadcast at all.
    TestMock::MockDisplayStateValue() = OHOS::DisplayPowerMgr::DisplayState::DISPLAY_ON; // display 1 on
    TestMock::ResetCommonEventMock();
    cb->OnMultiScreenDisplayStateChanged(0, "scr0", OHOS::DisplayPowerMgr::DisplayState::DISPLAY_OFF,
        MultiScreenStateChangeReason::STATE_CHANGE_REASON_DEFAULT);
    EXPECT_EQ(TestMock::MockPublishAsUserCallCount(), 0);
    EXPECT_EQ(TestMock::MockWholeMachineCallCount(), 0);
}

// ---- T7: PowerMgrService::OnAddSystemAbility register path (real entry, integration) ----
HWTEST_F(MultiUserScreenAdapterTest, OnAddSystemAbility_RegistersMultiUserCallback, TestSize.Level1)
{
    MultiUserScreenAdapter::multiScreenCallback_ = nullptr;
    TestMock::ResetDisplayMock();
    TestMock::MockDisplayRegisterRet() = DisplayErrors::ERR_OK;
    g_pms->OnAddSystemAbility(DISPLAY_MANAGER_SERVICE_ID, "");
    EXPECT_GT(TestMock::MockRegisterCallCount(), 0);
}
} // namespace
