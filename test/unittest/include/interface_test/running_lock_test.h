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

#ifndef POWERMGR_RUNNING_LOCK_TEST_H
#define POWERMGR_RUNNING_LOCK_TEST_H

#include <map>
#include <memory>
#include <stdlib.h>

#include <gtest/gtest.h>

#include "power_mgr_client.h"
#include "power_mgr_service.h"
#include "running_lock_proxy.h"
#include "running_lock_token_stub.h"

namespace OHOS {
namespace PowerMgr {
class RunningLockTest : public testing::Test {
public:
    RunningLockTest()
    {
        sptr<PowerMgrService> service = DelayedSpSingleton<PowerMgrService>::GetInstance();
        service->OnStart();
        stub_ = static_cast<PowerMgrStub*>(service);
    }

    ~RunningLockTest() = default;

    void TestRunningLockInnerExisit(sptr<IRemoteObject>& token, RunningLockInfo& runningLockInfo)
    {
        auto lockMap = runningLockMgr_->GetRunningLockMap();
        auto iterator = lockMap.find(token);
        ASSERT_TRUE(iterator != lockMap.end());
        auto runningLockInner = iterator->second;
        ASSERT_TRUE(runningLockInner != nullptr);
        ASSERT_TRUE(runningLockInner->GetType() == runningLockInfo.type);
        ASSERT_TRUE((runningLockInfo.name).compare(runningLockInner->GetName()) == 0);
    }

    void TestRunningLockInnerNoExisit(sptr<IRemoteObject>& token)
    {
        auto lockMap = runningLockMgr_->GetRunningLockMap();
        auto iterator = lockMap.find(token);
        ASSERT_TRUE(iterator == lockMap.end());
    }

    inline void DumpRunningLockInfo()
    {
        system("hidumper -s 3301 -a -runninglock");
    }

    static std::shared_ptr<RunningLockMgr> runningLockMgr_;
protected:
    sptr<PowerMgrStub> stub_ {nullptr};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_TEST_H
