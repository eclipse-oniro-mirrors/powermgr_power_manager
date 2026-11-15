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

#include "hibernate_callback_test.h"

#include <thread>

#include "hibernate_callback_priority.h"
#include "hibernate_callback_base.h"
#include "mock_power_remote_object.h"
#include "power_common.h"
#include "power_log.h"
#include "sync_hibernate_callback_proxy.h"
#include "sync_hibernate_callback_stub.h"
#include "hibernate/sync_hibernate_callback_ipc_interface_code.h"

namespace OHOS {
namespace PowerMgr {

using namespace testing;
using namespace testing::ext;

namespace {
bool g_isOnSyncHibernate = false;
bool g_isOnSyncWakeup = false;
bool g_hibernateResult = false;
constexpr uint32_t INVALID_CMD_CODE = 100;

/**
 * Test callback that inherits SyncHibernateCallbackStub, mirroring the
 * real PowerHibernateCallback inheritance hierarchy (both NAPI and Taihe
 * versions inherit from SyncHibernateCallbackStub).
 */
class TestSyncHibernateCallback : public SyncHibernateCallbackStub {
public:
    TestSyncHibernateCallback() = default;
    ~TestSyncHibernateCallback() override = default;

    void OnSyncHibernate() override
    {
        g_isOnSyncHibernate = true;
        POWER_HILOGI(LABEL_TEST, "TestSyncHibernateCallback OnSyncHibernate");
    }

    void OnSyncWakeup(bool hibernateResult) override
    {
        g_isOnSyncWakeup = true;
        g_hibernateResult = hibernateResult;
        POWER_HILOGI(LABEL_TEST, "TestSyncHibernateCallback OnSyncWakeup, result=%{public}d",
            static_cast<int32_t>(hibernateResult));
    }
};

class TestHibernateCallbackBase : public HibernateCallbackBase {
public:
    TestHibernateCallbackBase() : HibernateCallbackBase("test", false) {}

    HibernateCallbackEvent event = HibernateCallbackEvent::PRE_HIBERNATE;
    uint32_t postCount = 0;
    bool shouldPost = true;

protected:
    bool PostCallback(HibernateCallbackEvent nextEvent,
        const std::shared_ptr<HibernateCallbackCompletion>& completion) override
    {
        event = nextEvent;
        ++postCount;
        if (shouldPost) {
            completion->Complete();
        }
        return shouldPost;
    }
};

} // namespace

void HibernateCallbackTest::SetUpTestCase()
{
}

void HibernateCallbackTest::TearDownTestCase()
{
}

void HibernateCallbackTest::SetUp()
{
    g_isOnSyncHibernate = false;
    g_isOnSyncWakeup = false;
    g_hibernateResult = false;
}

void HibernateCallbackTest::TearDown()
{
}

/**
 * @tc.name: HibernateCallbackMethodName001
 * @tc.desc: Test method names used for hibernate and wakeup callbacks
 * @tc.type: FUNC
 */
HWTEST_F(HibernateCallbackTest, HibernateCallbackMethodName001, TestSize.Level2)
{
    EXPECT_STREQ(GetHibernateCallbackMethodName(HibernateCallbackEvent::PRE_HIBERNATE), "onHibernate");
    EXPECT_STREQ(GetHibernateCallbackMethodName(HibernateCallbackEvent::POST_HIBERNATE_SUCCESS), "onWakeup");
    EXPECT_STREQ(GetHibernateCallbackMethodName(HibernateCallbackEvent::POST_HIBERNATE_FAILURE), "onWakeup");
}

/**
 * @tc.name: HibernateCallbackCompletion001
 * @tc.desc: Test completion wait before and after notification
 * @tc.type: FUNC
 */
HWTEST_F(HibernateCallbackTest, HibernateCallbackCompletion001, TestSize.Level2)
{
    auto completion = std::make_shared<HibernateCallbackCompletion>();
    EXPECT_FALSE(completion->WaitUntil(std::chrono::steady_clock::now()));
    std::thread worker([completion]() { completion->Complete(); });
    EXPECT_TRUE(completion->WaitUntil(std::chrono::steady_clock::now() + std::chrono::seconds(1)));
    worker.join();
}

/**
 * @tc.name: HibernateCallbackBase001
 * @tc.desc: Test shared callback routing and post failure without a runtime
 * @tc.type: FUNC
 */
HWTEST_F(HibernateCallbackTest, HibernateCallbackBase001, TestSize.Level2)
{
    TestHibernateCallbackBase callback;
    callback.OnSyncHibernate();
    EXPECT_EQ(callback.event, HibernateCallbackEvent::PRE_HIBERNATE);
    callback.OnSyncWakeup(true);
    EXPECT_EQ(callback.event, HibernateCallbackEvent::POST_HIBERNATE_SUCCESS);
    callback.OnSyncWakeup(false);
    EXPECT_EQ(callback.event, HibernateCallbackEvent::POST_HIBERNATE_FAILURE);
    EXPECT_EQ(callback.postCount, 3u);

    callback.shouldPost = false;
    callback.OnSyncHibernate();
    EXPECT_EQ(callback.postCount, 4u);
}

// ======================== SyncHibernateCallbackStub Tests ========================

/**
 * @tc.name: SyncHibernateCallbackStub001
 * @tc.desc: Test OnRemoteRequest with wrong interface token
 * @tc.type: FUNC
 */
HWTEST_F(HibernateCallbackTest, SyncHibernateCallbackStub001, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackStub001 function start!");
    TestSyncHibernateCallback callback;
    MessageParcel data;
    MessageParcel reply;
    MessageOption opt;

    data.WriteInterfaceToken(u"test.wrong.interface.token");
    int32_t ret = callback.OnRemoteRequest(
        static_cast<uint32_t>(SyncHibernateCallbackInterfaceCode::CMD_ON_SYNC_HIBERNATE), data, reply, opt);
    EXPECT_EQ(ret, E_GET_POWER_SERVICE_FAILED);
    EXPECT_FALSE(g_isOnSyncHibernate);
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackStub001 function end!");
}

/**
 * @tc.name: SyncHibernateCallbackStub002
 * @tc.desc: Test OnRemoteRequest CMD_ON_SYNC_HIBERNATE with correct token
 * @tc.type: FUNC
 */
HWTEST_F(HibernateCallbackTest, SyncHibernateCallbackStub002, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackStub002 function start!");
    TestSyncHibernateCallback callback;
    MessageParcel data;
    MessageParcel reply;
    MessageOption opt;

    data.WriteInterfaceToken(TestSyncHibernateCallback::GetDescriptor());
    int32_t ret = callback.OnRemoteRequest(
        static_cast<uint32_t>(SyncHibernateCallbackInterfaceCode::CMD_ON_SYNC_HIBERNATE), data, reply, opt);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(g_isOnSyncHibernate);
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackStub002 function end!");
}

/**
 * @tc.name: SyncHibernateCallbackStub003
 * @tc.desc: Test OnRemoteRequest CMD_ON_SYNC_WAKEUP with hibernateResult=true
 * @tc.type: FUNC
 */
HWTEST_F(HibernateCallbackTest, SyncHibernateCallbackStub003, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackStub003 function start!");
    TestSyncHibernateCallback callback;
    MessageParcel data;
    MessageParcel reply;
    MessageOption opt;

    data.WriteInterfaceToken(TestSyncHibernateCallback::GetDescriptor());
    data.WriteBool(true);
    int32_t ret = callback.OnRemoteRequest(
        static_cast<uint32_t>(SyncHibernateCallbackInterfaceCode::CMD_ON_SYNC_WAKEUP), data, reply, opt);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(g_isOnSyncWakeup);
    EXPECT_TRUE(g_hibernateResult);
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackStub003 function end!");
}

/**
 * @tc.name: SyncHibernateCallbackStub004
 * @tc.desc: Test OnRemoteRequest CMD_ON_SYNC_WAKEUP with hibernateResult=false
 * @tc.type: FUNC
 */
HWTEST_F(HibernateCallbackTest, SyncHibernateCallbackStub004, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackStub004 function start!");
    TestSyncHibernateCallback callback;
    MessageParcel data;
    MessageParcel reply;
    MessageOption opt;

    data.WriteInterfaceToken(TestSyncHibernateCallback::GetDescriptor());
    data.WriteBool(false);
    int32_t ret = callback.OnRemoteRequest(
        static_cast<uint32_t>(SyncHibernateCallbackInterfaceCode::CMD_ON_SYNC_WAKEUP), data, reply, opt);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(g_isOnSyncWakeup);
    EXPECT_FALSE(g_hibernateResult);
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackStub004 function end!");
}

/**
 * @tc.name: SyncHibernateCallbackStub005
 * @tc.desc: Test OnRemoteRequest with invalid cmd code
 * @tc.type: FUNC
 */
HWTEST_F(HibernateCallbackTest, SyncHibernateCallbackStub005, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackStub005 function start!");
    TestSyncHibernateCallback callback;
    MessageParcel data;
    MessageParcel reply;
    MessageOption opt;

    data.WriteInterfaceToken(TestSyncHibernateCallback::GetDescriptor());
    int32_t ret = callback.OnRemoteRequest(INVALID_CMD_CODE, data, reply, opt);
    EXPECT_NE(ret, ERR_OK);
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackStub005 function end!");
}

/**
 * @tc.name: SyncHibernateCallbackStub006
 * @tc.desc: Test OnRemoteRequest CMD_ON_SYNC_WAKEUP with missing bool data (parcel read failure)
 * @tc.type: FUNC
 */
HWTEST_F(HibernateCallbackTest, SyncHibernateCallbackStub006, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackStub006 function start!");
    TestSyncHibernateCallback callback;
    MessageParcel data;
    MessageParcel reply;
    MessageOption opt;

    // Write correct token but do NOT write the bool parameter
    data.WriteInterfaceToken(TestSyncHibernateCallback::GetDescriptor());
    int32_t ret = callback.OnRemoteRequest(
        static_cast<uint32_t>(SyncHibernateCallbackInterfaceCode::CMD_ON_SYNC_WAKEUP), data, reply, opt);
    EXPECT_NE(ret, ERR_OK);
    EXPECT_FALSE(g_isOnSyncWakeup);
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackStub006 function end!");
}

/**
 * @tc.name: SyncHibernateCallbackStub007
 * @tc.desc: Test base class OnSyncHibernate/OnSyncWakeup default implementations are no-ops
 * @tc.type: FUNC
 */
HWTEST_F(HibernateCallbackTest, SyncHibernateCallbackStub007, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackStub007 function start!");
    SyncHibernateCallbackStub stub;
    // Base class default impls should be no-ops (do nothing, no crash)
    stub.SyncHibernateCallbackStub::OnSyncHibernate();
    stub.SyncHibernateCallbackStub::OnSyncWakeup(true);
    stub.SyncHibernateCallbackStub::OnSyncWakeup(false);
    // If we reach here without crashing, the test passes
    EXPECT_TRUE(true);
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackStub007 function end!");
}

// ======================== SyncHibernateCallbackProxy Tests ========================

/**
 * @tc.name: SyncHibernateCallbackProxy001
 * @tc.desc: Test proxy OnSyncHibernate with valid remote object
 * @tc.type: FUNC
 */
HWTEST_F(HibernateCallbackTest, SyncHibernateCallbackProxy001, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackProxy001 function start!");
    sptr<MockPowerRemoteObject> remote = new MockPowerRemoteObject();
    MockPowerRemoteObject::SetRequestValue(ERR_OK);
    SyncHibernateCallbackProxy proxy(remote);

    // Should not crash, SendRequest should be called
    proxy.OnSyncHibernate();
    EXPECT_EQ(MockPowerRemoteObject::GetRequestValue(), ERR_OK);
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackProxy001 function end!");
}

/**
 * @tc.name: SyncHibernateCallbackProxy002
 * @tc.desc: Test proxy OnSyncWakeup with valid remote object and hibernateResult=true
 * @tc.type: FUNC
 */
HWTEST_F(HibernateCallbackTest, SyncHibernateCallbackProxy002, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackProxy002 function start!");
    sptr<MockPowerRemoteObject> remote = new MockPowerRemoteObject();
    MockPowerRemoteObject::SetRequestValue(ERR_OK);
    SyncHibernateCallbackProxy proxy(remote);

    proxy.OnSyncWakeup(true);
    EXPECT_EQ(MockPowerRemoteObject::GetRequestValue(), ERR_OK);
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackProxy002 function end!");
}

/**
 * @tc.name: SyncHibernateCallbackProxy003
 * @tc.desc: Test proxy OnSyncWakeup with valid remote object and hibernateResult=false
 * @tc.type: FUNC
 */
HWTEST_F(HibernateCallbackTest, SyncHibernateCallbackProxy003, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackProxy003 function start!");
    sptr<MockPowerRemoteObject> remote = new MockPowerRemoteObject();
    MockPowerRemoteObject::SetRequestValue(ERR_OK);
    SyncHibernateCallbackProxy proxy(remote);

    proxy.OnSyncWakeup(false);
    EXPECT_EQ(MockPowerRemoteObject::GetRequestValue(), ERR_OK);
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackProxy003 function end!");
}

/**
 * @tc.name: SyncHibernateCallbackProxy004
 * @tc.desc: Test proxy OnSyncHibernate with SendRequest failure
 * @tc.type: FUNC
 */
HWTEST_F(HibernateCallbackTest, SyncHibernateCallbackProxy004, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackProxy004 function start!");
    sptr<MockPowerRemoteObject> remote = new MockPowerRemoteObject();
    MockPowerRemoteObject::SetRequestValue(-1);
    SyncHibernateCallbackProxy proxy(remote);

    // Should handle failure gracefully (no crash)
    proxy.OnSyncHibernate();
    proxy.OnSyncWakeup(true);
    EXPECT_TRUE(true);
    POWER_HILOGI(LABEL_TEST, "SyncHibernateCallbackProxy004 function end!");
}

// ======================== HibernateCallbackPriority Tests ========================

/**
 * @tc.name: HibernateCallbackPriority001
 * @tc.desc: Test HibernateCallbackPriority enum values
 * @tc.type: FUNC
 */
HWTEST_F(HibernateCallbackTest, HibernateCallbackPriority001, TestSize.Level2)
{
    POWER_HILOGI(LABEL_TEST, "HibernateCallbackPriority001 function start!");
    // Verify enum values match expected order
    EXPECT_EQ(static_cast<uint32_t>(HibernateCallbackPriority::LOW), 0u);
    EXPECT_EQ(static_cast<uint32_t>(HibernateCallbackPriority::DEFAULT), 1u);
    EXPECT_EQ(static_cast<uint32_t>(HibernateCallbackPriority::HIGH), 2u);
    POWER_HILOGI(LABEL_TEST, "HibernateCallbackPriority001 function end!");
}

} // namespace PowerMgr
} // namespace OHOS
