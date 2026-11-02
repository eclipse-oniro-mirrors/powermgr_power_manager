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

#ifndef POWERMGR_HIBERNATE_CALLBACK_BASE_H
#define POWERMGR_HIBERNATE_CALLBACK_BASE_H

#include "sync_hibernate_callback_stub.h"
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>

namespace OHOS {
namespace PowerMgr {

enum class HibernateCallbackEvent {
    PRE_HIBERNATE,
    POST_HIBERNATE_SUCCESS,
    POST_HIBERNATE_FAILURE,
};

class HibernateCallbackCompletion {
public:
    void Complete();
    bool WaitUntil(std::chrono::steady_clock::time_point deadline);

private:
    std::mutex mutex_;
    std::condition_variable condition_;
    bool completed_ = false;
};

const char* GetHibernateCallbackMethodName(HibernateCallbackEvent event);

class HibernateCallbackBase : public SyncHibernateCallbackStub {
public:
    HibernateCallbackBase(const char* runtimeName, bool timeoutIsError)
        : runtimeName_(runtimeName), timeoutIsError_(timeoutIsError) {}
    ~HibernateCallbackBase() override = default;

    void OnSyncHibernate() final;
    void OnSyncWakeup(bool hibernateResult) final;

protected:
    virtual bool PostCallback(HibernateCallbackEvent event,
        const std::shared_ptr<HibernateCallbackCompletion>& completion) = 0;

private:
    void DispatchCallback(HibernateCallbackEvent event);
    const char* runtimeName_;
    bool timeoutIsError_;
};

} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_HIBERNATE_CALLBACK_BASE_H
