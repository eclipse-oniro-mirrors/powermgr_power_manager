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

#include "running_lock_test.h"

#include <ipc_skeleton.h>

#include "actions/irunning_lock_action.h"
#include "power_mgr_proxy.h"
#include "power_mgr_service.h"
#include "running_lock_mgr.h"
#include "power_log.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
constexpr int32_t VECTOR_MAX_SIZE = 102400;
constexpr int32_t TEST_PROCESS_COUNT = 2000;
constexpr int32_t US_PER_MS = 1000;
constexpr int32_t app0Uid = 8;
constexpr int32_t app1Uid = 9;
}

namespace {
/**
 * @tc.name: RunningLockTest001
 * @tc.desc: Test RunningLockInnerKit function, connect PowerMgrService and call member function.
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockTest, RunningLockTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest001 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock1 = powerMgrClient.CreateRunningLock("runninglock1", RunningLockType::RUNNINGLOCK_SCREEN);
    ASSERT_TRUE(runningLock1 != nullptr);

    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    runningLock1->Lock();
    ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";

    runningLock1->UnLock();
    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    POWER_HILOGI(LABEL_TEST, "RunningLockTest001 function end!");
}

/**
 * @tc.name: RunningLockTest002
 * @tc.desc: Test RunningLockInnerKit function, timeout lock.
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockTest, RunningLockTest002, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest002 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto runningLock1 = powerMgrClient.CreateRunningLock("runninglock1", RunningLockType::RUNNINGLOCK_SCREEN);
    ASSERT_TRUE(runningLock1 != nullptr);
    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    runningLock1->Lock();
    ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
    runningLock1->UnLock();
    ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
    POWER_HILOGI(LABEL_TEST, "PowerMgrUnitTest::RunningLockTest002 1");
    // lock 50ms
    runningLock1->Lock(50);
    usleep(4000);
    POWER_HILOGI(LABEL_TEST, "PowerMgrUnitTest::RunningLockTest002 2");
    ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
    usleep(1000);
    POWER_HILOGI(LABEL_TEST, "PowerMgrUnitTest::RunningLockTest002 3");
    ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
    // wait 100ms
    usleep(100000);
    POWER_HILOGI(LABEL_TEST, "RunningLockTest002 function end!");
}

/**
 * @tc.name: RunningLockTest003
 * @tc.desc: Test RunningLockInnerKit function, timeout lock.
 * @tc.type: FUNC
 */
HWTEST_F(RunningLockTest, RunningLockTest003, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest003 function start!");
    if (false) {
        auto& powerMgrClient = PowerMgrClient::GetInstance();
        auto runningLock1 = powerMgrClient.CreateRunningLock("runninglock005",
            RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
        ASSERT_TRUE(runningLock1 != nullptr);
        ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
        // after 8ms unlock
        runningLock1->Lock(30);
        runningLock1->Lock(80);
        POWER_HILOGI(LABEL_TEST, "PowerMgrUnitTest::RunningLockTest003 1");
        usleep(50000);
        ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
        usleep(50000);
        ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
        // no unlock
        POWER_HILOGI(LABEL_TEST, "PowerMgrUnitTest::RunningLockTest003 2");
        runningLock1->Lock(2);
        runningLock1->Lock(3);
        runningLock1->Lock();
        POWER_HILOGI(LABEL_TEST, "PowerMgrUnitTest::RunningLockTest003 3");
        usleep(8000);
        ASSERT_TRUE(runningLock1->IsUsed()) << "runningLock1->IsUsed() != true";
        // after 3ms unlock
        runningLock1->Lock(30);
        POWER_HILOGI(LABEL_TEST, "PowerMgrUnitTest::RunningLockTest003 4");
        usleep(50000);
        ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
        runningLock1->Lock(5);
        runningLock1->UnLock();
        ASSERT_TRUE(!runningLock1->IsUsed()) << "runningLock1->IsUsed() != false";
        POWER_HILOGI(LABEL_TEST, "PowerMgrUnitTest::RunningLockTest003 5");
    }
    POWER_HILOGI(LABEL_TEST, "RunningLockTest003 function end!");
}

/**
 * @tc.name: RunningLockTest004
 * @tc.desc: Test CreateRunningLock function.
 * @tc.type: FUNC
 * @tc.require: issueI6NWQD
 */
HWTEST_F(RunningLockTest, RunningLockTest004, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest004 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::shared_ptr<RunningLock> runningLock = powerMgrClient.CreateRunningLock(
        "background", RunningLockType::RUNNINGLOCK_BACKGROUND);
    EXPECT_NE(runningLock, nullptr);

    runningLock = powerMgrClient.CreateRunningLock("backgroudPhone", RunningLockType::RUNNINGLOCK_BACKGROUND_PHONE);
    EXPECT_NE(runningLock, nullptr);

    runningLock = powerMgrClient.CreateRunningLock(
        "backgroudNatification", RunningLockType::RUNNINGLOCK_BACKGROUND_NOTIFICATION);
    EXPECT_NE(runningLock, nullptr);

    runningLock = powerMgrClient.CreateRunningLock("backgroudAudio", RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
    EXPECT_NE(runningLock, nullptr);

    runningLock = powerMgrClient.CreateRunningLock("backgroudSport", RunningLockType::RUNNINGLOCK_BACKGROUND_SPORT);
    EXPECT_NE(runningLock, nullptr);

    runningLock = powerMgrClient.CreateRunningLock(
        "backgroudNavigation", RunningLockType::RUNNINGLOCK_BACKGROUND_NAVIGATION);
    EXPECT_NE(runningLock, nullptr);

    runningLock = powerMgrClient.CreateRunningLock("backgroudTask", RunningLockType::RUNNINGLOCK_BACKGROUND_TASK);
    EXPECT_NE(runningLock, nullptr);

    runningLock = powerMgrClient.CreateRunningLock("butt", RunningLockType::RUNNINGLOCK_BUTT);
    EXPECT_EQ(runningLock, nullptr);
    POWER_HILOGI(LABEL_TEST, "RunningLockTest004 function end!");
}

/**
 * @tc.name: RunningLockTest005
 * @tc.desc: Test ProxyRunningLock function.
 * @tc.type: FUNC
 * @tc.require: issueI6S0YY
 */
HWTEST_F(RunningLockTest, RunningLockTest005, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest005 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::shared_ptr<RunningLock> runningLock = powerMgrClient.CreateRunningLock(
        "background.test005", RunningLockType::RUNNINGLOCK_BACKGROUND);
    ASSERT_NE(runningLock, nullptr);

    pid_t curUid = getuid();
    pid_t curPid = getpid();
    runningLock->Lock();
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(true, curPid, curUid));
    EXPECT_FALSE(runningLock->IsUsed());
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(false, curPid, curUid));
    runningLock->UnLock();
    POWER_HILOGI(LABEL_TEST, "RunningLockTest005 function end!");
}

/**
 * @tc.name: RunningLockTest006
 * @tc.desc: Test ProxyRunningLock function.
 * @tc.type: FUNC
 * @tc.require: issueI6S0YY
 */
HWTEST_F(RunningLockTest, RunningLockTest006, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest006 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::shared_ptr<RunningLock> runningLock = powerMgrClient.CreateRunningLock(
        "background.test006", RunningLockType::RUNNINGLOCK_SCREEN);
    ASSERT_NE(runningLock, nullptr);

    pid_t curUid = getuid();
    pid_t curPid = getpid();

    runningLock->Lock(-1);
    EXPECT_TRUE(runningLock->IsUsed());

    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(true, curPid, curUid));
    EXPECT_FALSE(runningLock->IsUsed());

    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(false, curPid, curUid));
    EXPECT_TRUE(runningLock->IsUsed());
    runningLock->UnLock();
    POWER_HILOGI(LABEL_TEST, "RunningLockTest006 function end!");
}

/**
 * @tc.name: RunningLockTest007
 * @tc.desc: Test ProxyRunningLock function.
 * @tc.type: FUNC
 * @tc.require: issueI6S0YY
 */
HWTEST_F(RunningLockTest, RunningLockTest007, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest007 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::shared_ptr<RunningLock> runningLock = powerMgrClient.CreateRunningLock(
        "background.test007", RunningLockType::RUNNINGLOCK_SCREEN);
    ASSERT_NE(runningLock, nullptr);

    pid_t curUid = getuid();
    pid_t curPid = getpid();
    int32_t timeoutMs = 1000;

    runningLock->Lock(timeoutMs);
    EXPECT_TRUE(runningLock->IsUsed());

    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(true, curPid, curUid));
    EXPECT_FALSE(runningLock->IsUsed());

    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(false, curPid, curUid));
    runningLock->UnLock();
    EXPECT_FALSE(runningLock->IsUsed());
    POWER_HILOGI(LABEL_TEST, "RunningLockTest007 function end!");
}

/**
 * @tc.name: RunningLockTest008
 * @tc.desc: Test ProxyRunningLock function, create runninglock first, then proxy
 * @tc.type: FUNC
 * @tc.require: issueI6S0YY
 */
HWTEST_F(RunningLockTest, RunningLockTest008, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest008 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    pid_t curUid = getuid();
    pid_t curPid = getpid();
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(true, curPid, curUid));

    std::shared_ptr<RunningLock> runningLock = powerMgrClient.CreateRunningLock(
        "background.test008", RunningLockType::RUNNINGLOCK_BACKGROUND);
    ASSERT_NE(runningLock, nullptr);
    runningLock->Lock();
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(false, curPid, curUid));
    runningLock->UnLock();
    POWER_HILOGI(LABEL_TEST, "RunningLockTest008 function end!");
}

/**
 * @tc.name: RunningLockTest009
 * @tc.desc: Test ProxyRunningLock function.
 * @tc.type: FUNC
 * @tc.require: issueI6S0YY
 */
HWTEST_F(RunningLockTest, RunningLockTest009, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest009 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::shared_ptr<RunningLock> screenRunningLock = powerMgrClient.CreateRunningLock(
        "screen.test009", RunningLockType::RUNNINGLOCK_SCREEN);
    ASSERT_NE(screenRunningLock, nullptr);
#ifdef HAS_SENSORS_SENSOR_PART
    std::shared_ptr<RunningLock> proximityRunningLock = powerMgrClient.CreateRunningLock(
        "proximity.test009", RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL);
    ASSERT_NE(proximityRunningLock, nullptr);
#endif

    pid_t curUid = getuid();
    pid_t curPid = getpid();
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(true, curPid, curUid));

    screenRunningLock->Lock();
    EXPECT_TRUE(screenRunningLock->IsUsed());
    screenRunningLock->UnLock();

#ifdef HAS_SENSORS_SENSOR_PART
    proximityRunningLock->Lock();
    EXPECT_TRUE(proximityRunningLock->IsUsed());
    proximityRunningLock->UnLock();
#endif

    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(false, curPid, curUid));
    POWER_HILOGI(LABEL_TEST, "RunningLockTest009 function end!");
}

/**
 * @tc.name: RunningLockTest010
 * @tc.desc: Test ProxyRunningLock function, pid is invalid
 * @tc.type: FUNC
 * @tc.require: issueI6S0YY
 */
HWTEST_F(RunningLockTest, RunningLockTest010, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest010 function start!");
    pid_t curUid = 1;
    pid_t curPid = -1;
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    EXPECT_FALSE(powerMgrClient.ProxyRunningLock(true, curPid, curUid));
    EXPECT_FALSE(powerMgrClient.ProxyRunningLock(false, curPid, curUid));
    POWER_HILOGI(LABEL_TEST, "RunningLockTest010 function end!");
}

/**
 * @tc.name: RunningLockTest011
 * @tc.desc: Test RunningLockProxt AddRunningLock function
 * @tc.type: FUNC
 * @tc.require: issueI7405P
 */
HWTEST_F(RunningLockTest, RunningLockTest011, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest011 function start!");
    auto runninglockProxy = std::make_shared<RunningLockProxy>();
    pid_t pid = 1;
    pid_t uid = -1;
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    runninglockProxy->AddRunningLock(0, 0, remoteObj);
    EXPECT_TRUE(runninglockProxy->IncreaseProxyCnt(pid, uid));
    EXPECT_TRUE(runninglockProxy->DecreaseProxyCnt(pid, uid));
    POWER_HILOGI(LABEL_TEST, "RunningLockTest011 function end!");
}

/**
 * @tc.name: RunningLockTest012
 * @tc.desc: Test RunningLockProxt RemoveRunningLock function
 * @tc.type: FUNC
 * @tc.require: issueI7405P
 */
HWTEST_F(RunningLockTest, RunningLockTest012, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest012 function start!");
    auto runninglockProxy = std::make_shared<RunningLockProxy>();
    pid_t pid = 1;
    pid_t uid = -1;
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    sptr<IRemoteObject> remoteObj2 = nullptr;
    runninglockProxy->AddRunningLock(pid, uid, remoteObj);
    runninglockProxy->RemoveRunningLock(pid, uid, remoteObj2);
    EXPECT_TRUE(runninglockProxy->IncreaseProxyCnt(0, 0));
    EXPECT_TRUE(runninglockProxy->DecreaseProxyCnt(0, 0));
    POWER_HILOGI(LABEL_TEST, "RunningLockTest012 function end!");
}

/**
 * @tc.name: RunningLockTest013
 * @tc.desc: Test RunningLockProxt RemoveRunningLock function
 * @tc.type: FUNC
 * @tc.require: issueI7405P
 */
HWTEST_F(RunningLockTest, RunningLockTest013, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest013 function start!");
    auto runninglockProxy = std::make_shared<RunningLockProxy>();
    pid_t pid = 1;
    pid_t uid = -1;
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    runninglockProxy->RemoveRunningLock(pid, uid, remoteObj);
    EXPECT_TRUE(runninglockProxy->IncreaseProxyCnt(pid, uid));
    POWER_HILOGI(LABEL_TEST, "RunningLockTest013 function end!");
}

/**
 * @tc.name: RunningLockTest014
 * @tc.desc: Test ProxyRunningLocks function
 * @tc.type: FUNC
 * @tc.require: issueI7MNRN
 */
HWTEST_F(RunningLockTest, RunningLockTest014, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest014 function start!");
    pid_t curUid = getuid();
    pid_t curPid = getpid();
    std::vector<std::pair<pid_t, pid_t>> processInfos;
    processInfos.push_back(std::pair<pid_t, pid_t>(curPid, curUid));
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    EXPECT_TRUE(powerMgrClient.ProxyRunningLocks(true, processInfos));

    for (int i = 0; i < TEST_PROCESS_COUNT; ++i) {
        processInfos.emplace_back(static_cast<pid_t>(US_PER_MS + i), static_cast<pid_t>(TEST_PROCESS_COUNT + i));
    }
    EXPECT_FALSE(powerMgrClient.ProxyRunningLocks(true, processInfos));
    POWER_HILOGI(LABEL_TEST, "RunningLockTest014 function end!");
}

/**
 * @tc.name: RunningLockTest015
 * @tc.desc: Test Recover function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RunningLockTest, RunningLockTest015, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest015 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pms->OnStart();
    auto runningLockMgr = pms->GetRunningLockMgr();
    sptr<IPowerMgr> ptr;
    ptr.ForceSetRefPtr(static_cast<IPowerMgr*>(pms.GetRefPtr()));
    pms.GetRefPtr()->IncStrongRef(pms.GetRefPtr());
    RunningLock runninglock1(ptr, "runninglock_recover_test", RunningLockType::RUNNINGLOCK_SCREEN);
    runninglock1.Init();
    const auto& infos = runningLockMgr->GetRunningLockMap();
    const auto iter = std::find_if(infos.begin(), infos.end(), [](const auto& pair) {
        return pair.second->GetName() == "runninglock_recover_test";
    });
    EXPECT_TRUE(iter != infos.end());
    const sptr<IRemoteObject> token = iter->first;
    runningLockMgr->ReleaseLock(token);
    EXPECT_TRUE(infos.count(token) == 0);
    runninglock1.Recover(ptr);
    EXPECT_TRUE(infos.count(token) > 0);
    POWER_HILOGI(LABEL_TEST, "RunningLockTest015 function end!");
}

/**
 * @tc.name: RunningLockTest016
 * @tc.desc: Test ProxyRunningLock function
 * @tc.type: FUNC
 * @tc.require
 */
HWTEST_F(RunningLockTest, RunningLockTest016, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest016 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    pid_t curUid = getuid();
    pid_t curPid = getpid();

    std::shared_ptr<RunningLock> runningLock = powerMgrClient.CreateRunningLock(
        "background.test016", RunningLockType::RUNNINGLOCK_BACKGROUND);
    ASSERT_NE(runningLock, nullptr);
    runningLock->Lock();
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(true, curPid, curUid));
    EXPECT_FALSE(runningLock->IsUsed());
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(false, curPid, curUid));
    EXPECT_TRUE(runningLock->IsUsed());
    runningLock->UnLock();
    POWER_HILOGI(LABEL_TEST, "RunningLockTest016 function end!");
}

/**
 * @tc.name: RunningLockTest017
 * @tc.desc: Test UpdateWorkSource function
 * @tc.type: FUNC
 * @tc.require
 */
HWTEST_F(RunningLockTest, RunningLockTest017, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest017 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    pid_t curUid = getuid();
    pid_t curPid = getpid();

    std::shared_ptr<RunningLock> runningLock = powerMgrClient.CreateRunningLock(
        "background.test017", RunningLockType::RUNNINGLOCK_BACKGROUND);
    ASSERT_NE(runningLock, nullptr);
    runningLock->Lock();
    std::vector<int32_t> workSource { 0 };
    EXPECT_TRUE(runningLock->UpdateWorkSource(workSource) == 0);
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(true, curPid, 0));
    EXPECT_FALSE(runningLock->IsUsed());
    EXPECT_TRUE(powerMgrClient.ProxyRunningLock(false, curPid, 0));
    EXPECT_TRUE(runningLock->IsUsed());
    std::vector<int32_t> hugeWorkSource(VECTOR_MAX_SIZE + 1, 1);
    EXPECT_EQ(runningLock->UpdateWorkSource(hugeWorkSource), E_INNER_ERR);
    runningLock->UnLock();
    POWER_HILOGI(LABEL_TEST, "RunningLockTest017 function end!");
}

/**
 * @tc.name: RunningLockTest018
 * @tc.desc: Test UpdateWorkSource function
 * @tc.type: FUNC
 * @tc.require
 */
HWTEST_F(RunningLockTest, RunningLockTest018, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest018 function start!");
    auto runninglockProxy = std::make_shared<RunningLockProxy>();
    pid_t pid = 1;
    pid_t uid = -1;
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    sptr<IRemoteObject> remoteObj2 = nullptr;
    runninglockProxy->AddRunningLock(pid, uid, remoteObj);
    runninglockProxy->RemoveRunningLock(pid, uid, remoteObj2);

    EXPECT_TRUE(runninglockProxy->UpdateWorkSource(pid, uid, remoteObj, {{0, {"test", false}}}));
    runninglockProxy->RemoveRunningLock(pid, uid, remoteObj);
    EXPECT_TRUE(runninglockProxy->IncreaseProxyCnt(pid, uid));
    EXPECT_TRUE(runninglockProxy->DecreaseProxyCnt(pid, uid));
    POWER_HILOGI(LABEL_TEST, "RunningLockTest018 function end!");
}

/**
 * @tc.name: RunningLockTest019
 * @tc.desc: Test UpdateWorkSource function
 * @tc.type: FUNC
 * @tc.require
 */
HWTEST_F(RunningLockTest, RunningLockTest019, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest019 function start!");
    auto runninglockProxy = std::make_shared<RunningLockProxy>();
    pid_t pid = 1;
    pid_t uid = -1;
    sptr<IRemoteObject> remoteObj = new RunningLockTokenStub();
    sptr<IRemoteObject> remoteObj2 = nullptr;
    runninglockProxy->AddRunningLock(pid, uid, remoteObj);
    runninglockProxy->RemoveRunningLock(pid, uid, remoteObj2);

    EXPECT_TRUE(runninglockProxy->UpdateWorkSource(pid, uid, remoteObj, {{0, {"test019", false}}}));
    EXPECT_TRUE(runninglockProxy->UpdateProxyState(pid, uid, remoteObj, true));
    EXPECT_TRUE(runninglockProxy->UpdateProxyState(pid, uid, remoteObj, false));
    runninglockProxy->RemoveRunningLock(pid, uid, remoteObj);
    EXPECT_TRUE(runninglockProxy->IncreaseProxyCnt(pid, uid));
    EXPECT_TRUE(runninglockProxy->DecreaseProxyCnt(pid, uid));
    POWER_HILOGI(LABEL_TEST, "RunningLockTest019 function end!");
}

/**
 * @tc.name: RunningLockTest020
 * @tc.desc: Test UpdateWorkSource function
 * @tc.type: FUNC
 * @tc.require
 */
HWTEST_F (RunningLockTest, RunningLockTest020, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest020 function start!");
    auto runninglockProxy = std::make_shared<RunningLockProxy>();
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    std::shared_ptr<RunningLock> runningLock = powerMgrClient.CreateRunningLock("backgroudAudio.test020",
        RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
    ASSERT_NE(runningLock, nullptr);

    pid_t curUid = 1;
    runningLock->Lock();
    EXPECT_TRUE(runningLock->IsUsed());
    EXPECT_FALSE(runninglockProxy->IsExistAudioStream(curUid));
    runningLock->UnLock();
    EXPECT_FALSE(runningLock->IsUsed());
    POWER_HILOGI(LABEL_TEST, "RunningLockTest020 function end!");
}

HWTEST_F(RunningLockTest, RunningLockTest021, TestSize.Level1)
{
    POWER_HILOGI(LABEL_TEST, "RunningLockTest021 function start!");
    EXPECT_NE(stub_, nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor());
    data.WriteInt32(1);
    VectorPair vectorPairInfos;
    std::vector<std::pair<pid_t, pid_t>> processInfos;
    processInfos.reserve(VECTOR_MAX_SIZE + 1);
    for (int i = 0; i <= VECTOR_MAX_SIZE; ++i) {
        processInfos.emplace_back(i, i + 100);
    }
    vectorPairInfos.SetProcessInfos(processInfos);
    EXPECT_FALSE(data.WriteParcelable(&vectorPairInfos));
    int32_t result = stub_->OnRemoteRequest(
        static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_PROXY_RUNNING_LOCKS_IPC), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_DATA);
    POWER_HILOGI(LABEL_TEST, "RunningLockTest021 function end!");
}
} // namespace