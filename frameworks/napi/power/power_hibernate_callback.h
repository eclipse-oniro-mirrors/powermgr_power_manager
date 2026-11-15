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

#ifndef POWERMGR_POWER_MANAGER_POWER_HIBERNATE_CALLBACK_H
#define POWERMGR_POWER_MANAGER_POWER_HIBERNATE_CALLBACK_H

#include "napi/native_api.h"
#include "hibernate_callback_base.h"
#include <cstdint>
#include <memory>
#include <mutex>

namespace OHOS {
namespace PowerMgr {

class PowerHibernateCallback : public HibernateCallbackBase {
public:
    PowerHibernateCallback() : HibernateCallbackBase("napi", true) {}
    virtual ~PowerHibernateCallback();

    void ReplaceCallback(napi_env env, napi_ref callbackRef);
    void ReleaseCallback();

private:
    bool PostCallback(HibernateCallbackEvent event,
        const std::shared_ptr<HibernateCallbackCompletion>& completion) override;
    void RunCallback(HibernateCallbackEvent event, const std::shared_ptr<HibernateCallbackCompletion>& completion,
        uint64_t generation);
    static void InvokeCallback(napi_env env, napi_value callbackObj, HibernateCallbackEvent event,
        const std::shared_ptr<HibernateCallbackCompletion>& completion);
    static void AttachPromiseCompletion(napi_env env, napi_value promise, const char* methodName,
        const std::shared_ptr<HibernateCallbackCompletion>& completion);
    static bool CreateNotifyFunction(napi_env env, const std::shared_ptr<HibernateCallbackCompletion>& completion,
        const char* methodName, bool isRejected, napi_value& notifyFunc);
    static napi_value NotifyCallback(napi_env env, napi_callback_info info);
    static void FinalizeNotifyContext(napi_env env, void* data, void* hint);
    static void FinalizeEnv(void* arg);

    napi_ref callbackRef_ { nullptr };
    napi_env env_ { nullptr };
    napi_env hookedEnv_ { nullptr };
    std::mutex callbackMutex_;
    uint64_t generation_ { 0 };
};

} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_POWER_HIBERNATE_CALLBACK_H
