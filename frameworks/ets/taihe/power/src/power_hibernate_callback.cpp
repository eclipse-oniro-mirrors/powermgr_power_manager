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

#include "power_hibernate_callback.h"
#include "event_runner.h"
#include "power_log.h"
#include <new>

namespace OHOS {
namespace PowerMgr {

struct PowerHibernateCallback::Invocation {
    ohos::power::HibernateCallback callback;
    std::shared_ptr<HibernateCallbackCompletion> completion;
    std::shared_ptr<InvocationControl> control;
    uint64_t generation;
    HibernateCallbackEvent event;
};

void PowerHibernateCallback::CreateCallback(ohos::power::weak::HibernateCallback callback)
{
    std::lock_guard lock(callbackMutex_);
    control_->generation.fetch_add(1, std::memory_order_release);
    callback_.emplace(callback);
    control_ = std::make_shared<InvocationControl>();
    if (handler_ == nullptr) {
        // RegisterHibernateCallback runs on the ets thread, so the runner obtained here belongs to it.
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner =
            OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        if (runner != nullptr) {
            handler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
        } else {
            POWER_HILOGE(FEATURE_SUSPEND, "ets thread runner is null");
        }
    }
}

void PowerHibernateCallback::ReleaseCallback()
{
    std::lock_guard lock(callbackMutex_);
    callback_.reset();
    control_->generation.fetch_add(1, std::memory_order_release);
}

void PowerHibernateCallback::RunInvocation(Invocation* invocation)
{
    std::unique_ptr<Invocation> invocationGuard(invocation);
    auto completion = invocation->completion;
    auto control = invocation->control;
    if (control->generation.load(std::memory_order_acquire) == invocation->generation) {
        auto callbackFuture = invocation->event == HibernateCallbackEvent::PRE_HIBERNATE ?
            invocation->callback->onHibernate() :
            invocation->callback->onWakeup(invocation->event == HibernateCallbackEvent::POST_HIBERNATE_SUCCESS);
        callbackFuture.on_complete([completion](auto&& outcome) {
            if (!outcome) {
                POWER_HILOGW(FEATURE_SUSPEND, "ets callback Promise rejected");
            }
            completion->Complete();
        });
    } else {
        POWER_HILOGW(FEATURE_SUSPEND, "generation mismatch, skip ets callback");
        completion->Complete();
    }
    auto& busy = invocation->event == HibernateCallbackEvent::PRE_HIBERNATE ?
        control->enterBusy : control->wakeupBusy;
    busy.store(false, std::memory_order_release);
}

bool PowerHibernateCallback::PostInvocation(Invocation* invocation)
{
    auto control = invocation->control;
    bool isEnter = invocation->event == HibernateCallbackEvent::PRE_HIBERNATE;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> handler;
    {
        std::lock_guard lock(callbackMutex_);
        handler = handler_;
    }
    bool posted = (handler != nullptr) && handler->PostTask([invocation]() { RunInvocation(invocation); });
    if (!posted) {
        delete invocation;
        auto& busy = isEnter ? control->enterBusy : control->wakeupBusy;
        busy.store(false, std::memory_order_release);
        POWER_HILOGE(FEATURE_SUSPEND, "post to ets thread failed, isEnter=%{public}d",
            static_cast<int32_t>(isEnter));
        return false;
    }
    return true;
}

bool PowerHibernateCallback::PostCallback(HibernateCallbackEvent event,
    const std::shared_ptr<HibernateCallbackCompletion>& completion)
{
    bool isEnter = event == HibernateCallbackEvent::PRE_HIBERNATE;
    std::optional<ohos::power::HibernateCallback> callback;
    std::shared_ptr<InvocationControl> control;
    uint64_t generation = 0;
    {
        std::lock_guard lock(callbackMutex_);
        if (!callback_.has_value()) {
            POWER_HILOGW(FEATURE_SUSPEND, "no ets callback stored, skip, isEnter=%{public}d",
                static_cast<int32_t>(isEnter));
            return false;
        }
        callback.emplace(*callback_);
        control = control_;
        generation = control->generation.load(std::memory_order_acquire);
    }

    auto& busy = isEnter ? control->enterBusy : control->wakeupBusy;
    bool expected = false;
    if (!busy.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        POWER_HILOGW(FEATURE_SUSPEND, "previous callback still running, skip, isEnter=%{public}d",
            static_cast<int32_t>(isEnter));
        return false;
    }
    auto* invocation = new (std::nothrow) Invocation {
        std::move(*callback), completion, control, generation, event};
    if (invocation == nullptr) {
        busy.store(false, std::memory_order_release);
        POWER_HILOGE(FEATURE_SUSPEND, "allocate invocation failed");
        return false;
    }
    return PostInvocation(invocation);
}

} // namespace PowerMgr
} // namespace OHOS
