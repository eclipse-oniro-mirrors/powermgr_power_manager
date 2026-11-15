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

#include "hibernate_callback_base.h"
#include "power_log.h"
#include <unistd.h>

namespace OHOS {
namespace PowerMgr {

namespace {
constexpr auto HIBERNATE_CALLBACK_TIMEOUT = std::chrono::seconds(3);

HibernateCallbackEvent SelectHibernateCallbackEvent(bool isEnter, bool hibernateResult = false)
{
    if (isEnter) {
        return HibernateCallbackEvent::PRE_HIBERNATE;
    }
    return hibernateResult ? HibernateCallbackEvent::POST_HIBERNATE_SUCCESS :
        HibernateCallbackEvent::POST_HIBERNATE_FAILURE;
}
} // namespace

void HibernateCallbackCompletion::Complete()
{
    {
        std::lock_guard lock(mutex_);
        completed_ = true;
    }
    condition_.notify_one();
}

bool HibernateCallbackCompletion::WaitUntil(std::chrono::steady_clock::time_point deadline)
{
    std::unique_lock lock(mutex_);
    return condition_.wait_until(lock, deadline, [this] { return completed_; });
}

const char* GetHibernateCallbackMethodName(HibernateCallbackEvent event)
{
    return event == HibernateCallbackEvent::PRE_HIBERNATE ? "onHibernate" : "onWakeup";
}

void HibernateCallbackBase::OnSyncHibernate()
{
    POWER_HILOGI(FEATURE_SUSPEND, "OnSyncHibernate");
    DispatchCallback(SelectHibernateCallbackEvent(true));
}

void HibernateCallbackBase::OnSyncWakeup(bool hibernateResult)
{
    POWER_HILOGI(FEATURE_SUSPEND, "OnSyncWakeup, hibernateResult: %{public}d",
        static_cast<int32_t>(hibernateResult));
    DispatchCallback(SelectHibernateCallbackEvent(false, hibernateResult));
}

void HibernateCallbackBase::DispatchCallback(HibernateCallbackEvent event)
{
    auto completion = std::make_shared<HibernateCallbackCompletion>();
    auto startTime = std::chrono::steady_clock::now();
    if (!PostCallback(event, completion)) {
        return;
    }
    bool completed = completion->WaitUntil(std::chrono::steady_clock::now() + HIBERNATE_CALLBACK_TIMEOUT);
    auto costMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - startTime).count();
    const char* methodName = GetHibernateCallbackMethodName(event);
    auto pid = static_cast<int32_t>(getpid());
    auto uid = static_cast<int32_t>(getuid());
    if (completed) {
        POWER_HILOGI(FEATURE_SUSPEND,
            "%{public}s callback %{public}s completed, pid=%{public}d, uid=%{public}d, costMs=%{public}lld",
            runtimeName_, methodName, pid, uid, static_cast<long long>(costMs));
    } else if (timeoutIsError_) {
        POWER_HILOGE(FEATURE_SUSPEND,
            "%{public}s callback %{public}s timeout, pid=%{public}d, uid=%{public}d, costMs=%{public}lld",
            runtimeName_, methodName, pid, uid, static_cast<long long>(costMs));
    } else {
        POWER_HILOGW(FEATURE_SUSPEND,
            "%{public}s callback %{public}s timeout, pid=%{public}d, uid=%{public}d, costMs=%{public}lld",
            runtimeName_, methodName, pid, uid, static_cast<long long>(costMs));
    }
}

} // namespace PowerMgr
} // namespace OHOS
