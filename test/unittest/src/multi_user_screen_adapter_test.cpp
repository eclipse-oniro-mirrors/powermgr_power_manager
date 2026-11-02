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

#include <csignal>
#include <cstdlib>

#include "multi_user_screen_adapter.h"
#include "mock/multi_user_test_mock.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using DisplayState = OHOS::DisplayPowerMgr::DisplayState;

namespace {
class MultiUserScreenAdapterTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        TestMock::ResetDisplayMock();
        TestMock::ResetOsAccountMock();
    }
};

// uid -> userId is pure local arithmetic (uid / 200000), no IPC.
HWTEST_F(MultiUserScreenAdapterTest, GetUserIdFromUid_Arithmetic, TestSize.Level1)
{
    EXPECT_EQ(MultiUserScreenAdapter::GetUserIdFromUid(200000 * 100 + 5), 100);
    EXPECT_EQ(MultiUserScreenAdapter::GetUserIdFromUid(0), 0);
    EXPECT_EQ(MultiUserScreenAdapter::GetUserIdFromUid(-1), MultiUserScreenAdapter::INVALID_USER_ID);
}

// IsSystemAccount: true for U10X (userId >= 100), false for U0/SA.
HWTEST_F(MultiUserScreenAdapterTest, IsSystemAccount_Threshold, TestSize.Level1)
{
    EXPECT_TRUE(MultiUserScreenAdapter::IsSystemAccount(200000 * 100 + 5)); // U100
    EXPECT_FALSE(MultiUserScreenAdapter::IsSystemAccount(0));                // U0/SA
    EXPECT_FALSE(MultiUserScreenAdapter::IsSystemAccount(200000 * 99));     // userId 99 < 100
}

// GetDisplayIdByUserId: OsAccount IPC; success returns the mapped id, failure returns INVALID.
HWTEST_F(MultiUserScreenAdapterTest, GetDisplayIdByUserId_OsAccount, TestSize.Level1)
{
    TestMock::MockDisplayIdRet() = ERR_OK;
    TestMock::MockDisplayId() = 5;
    EXPECT_EQ(MultiUserScreenAdapter::GetDisplayIdByUserId(100), 5);
    EXPECT_GT(TestMock::MockDisplayIdCallCount(), 0);
    TestMock::MockDisplayIdRet() = static_cast<ErrCode>(-1);
    EXPECT_EQ(MultiUserScreenAdapter::GetDisplayIdByUserId(100),
        MultiUserScreenAdapter::INVALID_DISPLAY_ID);
}

// IsScreenOnForUser: map.find(bound displayId); ON/DIM -> true, OFF/absent -> false, no display -> false.
HWTEST_F(MultiUserScreenAdapterTest, IsScreenOnForUser_MapLookup, TestSize.Level1)
{
    TestMock::MockDisplayIdRet() = ERR_OK;
    TestMock::MockDisplayId() = 5;
    auto& states = TestMock::MockMultiScreenStates();
    states[5] = DisplayState::DISPLAY_ON;
    EXPECT_TRUE(MultiUserScreenAdapter::IsScreenOnForUser(100));
    states[5] = DisplayState::DISPLAY_DIM;
    EXPECT_TRUE(MultiUserScreenAdapter::IsScreenOnForUser(100));
    states[5] = DisplayState::DISPLAY_OFF;
    EXPECT_FALSE(MultiUserScreenAdapter::IsScreenOnForUser(100));
    states.clear(); // bound displayId not in map -> off
    EXPECT_FALSE(MultiUserScreenAdapter::IsScreenOnForUser(100));
    TestMock::MockDisplayIdRet() = static_cast<ErrCode>(-1); // OsAccount fail -> no bound display
    EXPECT_FALSE(MultiUserScreenAdapter::IsScreenOnForUser(100));
}

// IsAnyScreenOn: any map value ON/DIM -> true, else false (including empty map).
HWTEST_F(MultiUserScreenAdapterTest, IsAnyScreenOn_Aggregate, TestSize.Level1)
{
    auto& states = TestMock::MockMultiScreenStates();
    EXPECT_FALSE(MultiUserScreenAdapter::IsAnyScreenOn()); // empty -> false
    states[5] = DisplayState::DISPLAY_OFF;
    states[6] = DisplayState::DISPLAY_OFF;
    EXPECT_FALSE(MultiUserScreenAdapter::IsAnyScreenOn()); // all off
    states[6] = DisplayState::DISPLAY_ON;
    EXPECT_TRUE(MultiUserScreenAdapter::IsAnyScreenOn()); // any on
    states.clear();
    states[7] = DisplayState::DISPLAY_DIM;
    EXPECT_TRUE(MultiUserScreenAdapter::IsAnyScreenOn()); // DIM counts as on
}
} // namespace
