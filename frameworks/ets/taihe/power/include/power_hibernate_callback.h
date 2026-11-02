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

#ifndef POWER_HIBERNATE_CALLBACK_H
#define POWER_HIBERNATE_CALLBACK_H

#include "event_handler.h"
#include "hibernate_callback_base.h"
#include "ohos.power.proj.hpp"
#include "ohos.power.impl.hpp"
#include "taihe/runtime.hpp"
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>

namespace OHOS {
namespace PowerMgr {
using namespace taihe;

class PowerHibernateCallback : public HibernateCallbackBase {
public:
    PowerHibernateCallback() : HibernateCallbackBase("ets", false) {}
    virtual ~PowerHibernateCallback() = default;

    void CreateCallback(ohos::power::weak::HibernateCallback callback);
    void ReleaseCallback();

private:
    struct InvocationControl {
        std::atomic<bool> enterBusy {false};
        std::atomic<bool> wakeupBusy {false};
        std::atomic<uint64_t> generation {0};
    };
    struct Invocation;

    bool PostCallback(HibernateCallbackEvent event,
        const std::shared_ptr<HibernateCallbackCompletion>& completion) override;
    bool PostInvocation(Invocation* invocation);
    static void RunInvocation(Invocation* invocation);
    std::optional<ohos::power::HibernateCallback> callback_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> handler_;
    std::mutex callbackMutex_;
    std::shared_ptr<InvocationControl> control_ = std::make_shared<InvocationControl>();
};

} // namespace PowerMgr
} // namespace OHOS

#endif // POWER_HIBERNATE_CALLBACK_H
