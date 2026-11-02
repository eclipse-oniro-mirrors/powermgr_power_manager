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
#include "napi/native_node_api.h"
#include "power_log.h"
#include "power_common.h"
#include <new>

namespace OHOS {
namespace PowerMgr {

constexpr const char* THEN = "then";
constexpr size_t PROMISE_THEN_HANDLER_COUNT = 2;

struct HibernateNotifyContext {
    std::shared_ptr<HibernateCallbackCompletion> completion;
    const char* methodName = nullptr;
    bool isRejected = false;
};

PowerHibernateCallback::~PowerHibernateCallback()
{
    ReleaseCallback();
}

void PowerHibernateCallback::ReleaseCallback()
{
    std::lock_guard lock(callbackMutex_);
    if (callbackRef_ != nullptr && env_ != nullptr) {
        napi_delete_reference(env_, callbackRef_);
    }
    callbackRef_ = nullptr;
    env_ = nullptr;
    ++generation_;
}

void PowerHibernateCallback::FinalizeEnv(void* arg)
{
    auto* self = static_cast<PowerHibernateCallback*>(arg);
    if (self == nullptr) {
        return;
    }
    // The env is torn down after cleanup hooks, so the reference can still be released here.
    self->ReleaseCallback();
    std::lock_guard lock(self->callbackMutex_);
    self->hookedEnv_ = nullptr;
}

void PowerHibernateCallback::ReplaceCallback(napi_env env, napi_ref callbackRef)
{
    std::lock_guard lock(callbackMutex_);
    if (callbackRef_ != nullptr && env_ != nullptr) {
        napi_delete_reference(env_, callbackRef_);
    }
    callbackRef_ = callbackRef;
    env_ = env;
    ++generation_;
    if (env != nullptr && hookedEnv_ != env) {
        if (napi_add_env_cleanup_hook(env, FinalizeEnv, this) == napi_ok) {
            hookedEnv_ = env;
        } else {
            POWER_HILOGW(FEATURE_SUSPEND, "add env cleanup hook failed");
        }
    }
}

napi_value PowerHibernateCallback::NotifyCallback(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data);
    auto* context = static_cast<HibernateNotifyContext*>(data);
    if (context != nullptr && context->completion != nullptr) {
        if (context->isRejected) {
            POWER_HILOGW(FEATURE_SUSPEND, "%{public}s Promise rejected", context->methodName);
        }
        context->completion->Complete();
    }
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    return undefined;
}

void PowerHibernateCallback::FinalizeNotifyContext(napi_env env, void* data, void* hint)
{
    (void)env;
    (void)hint;
    delete static_cast<HibernateNotifyContext*>(data);
}

bool PowerHibernateCallback::CreateNotifyFunction(napi_env env,
    const std::shared_ptr<HibernateCallbackCompletion>& completion,
    const char* methodName, bool isRejected, napi_value& notifyFunc)
{
    auto* notifyContext = new (std::nothrow) HibernateNotifyContext{completion, methodName, isRejected};
    if (notifyContext == nullptr) {
        return false;
    }
    if (napi_create_function(env, "notify", NAPI_AUTO_LENGTH, NotifyCallback,
        notifyContext, &notifyFunc) != napi_ok) {
        delete notifyContext;
        POWER_HILOGW(FEATURE_SUSPEND, "napi_create_function notify failed");
        return false;
    }
    if (napi_add_finalizer(env, notifyFunc, notifyContext, FinalizeNotifyContext,
        nullptr, nullptr) != napi_ok) {
        delete notifyContext;
        POWER_HILOGW(FEATURE_SUSPEND, "napi_add_finalizer notify failed");
        return false;
    }
    return true;
}

void PowerHibernateCallback::AttachPromiseCompletion(napi_env env, napi_value promise,
    const char* methodName, const std::shared_ptr<HibernateCallbackCompletion>& completion)
{
    napi_value notifyFunc = nullptr;
    if (!CreateNotifyFunction(env, completion, methodName, false, notifyFunc)) {
        completion->Complete();
        return;
    }
    napi_value rejectFunc = nullptr;
    if (!CreateNotifyFunction(env, completion, methodName, true, rejectFunc)) {
        completion->Complete();
        return;
    }
    napi_value thenFunc = nullptr;
    if (napi_get_named_property(env, promise, THEN, &thenFunc) != napi_ok) {
        POWER_HILOGW(FEATURE_SUSPEND, "napi_get_named_property then failed");
        completion->Complete();
        return;
    }
    napi_valuetype thenType = napi_undefined;
    napi_typeof(env, thenFunc, &thenType);
    if (thenType != napi_function) {
        POWER_HILOGW(FEATURE_SUSPEND, "promise.then is not a function");
        completion->Complete();
        return;
    }
    napi_value thenArgs[PROMISE_THEN_HANDLER_COUNT] = { notifyFunc, rejectFunc };
    if (napi_call_function(env, promise, thenFunc, PROMISE_THEN_HANDLER_COUNT, thenArgs, nullptr) != napi_ok) {
        POWER_HILOGW(FEATURE_SUSPEND, "napi_call_function then failed");
        completion->Complete();
    }
}

void PowerHibernateCallback::InvokeCallback(napi_env env, napi_value callbackObj, HibernateCallbackEvent event,
    const std::shared_ptr<HibernateCallbackCompletion>& completion)
{
    const char* methodName = GetHibernateCallbackMethodName(event);
    napi_value method = nullptr;
    napi_status status = napi_get_named_property(env, callbackObj, methodName, &method);
    if (status != napi_ok) {
        POWER_HILOGW(FEATURE_SUSPEND, "napi_get_named_property %{public}s failed. status = %{public}d",
            methodName, status);
        completion->Complete();
        return;
    }
    napi_valuetype methodType = napi_undefined;
    napi_typeof(env, method, &methodType);
    if (methodType != napi_function) {
        POWER_HILOGW(FEATURE_SUSPEND, "%{public}s is not a function", methodName);
        completion->Complete();
        return;
    }
    napi_value promise = nullptr;
    if (event != HibernateCallbackEvent::PRE_HIBERNATE) {
        napi_value param = nullptr;
        napi_get_boolean(env, event == HibernateCallbackEvent::POST_HIBERNATE_SUCCESS, &param);
        status = napi_call_function(env, callbackObj, method, 1, &param, &promise);
    } else {
        status = napi_call_function(env, callbackObj, method, 0, nullptr, &promise);
    }
    if (status != napi_ok) {
        POWER_HILOGW(FEATURE_SUSPEND, "napi_call_function %{public}s failed. status = %{public}d",
            methodName, status);
        completion->Complete();
        return;
    }
    napi_valuetype promiseType = napi_undefined;
    napi_typeof(env, promise, &promiseType);
    if (promiseType == napi_object) {
        AttachPromiseCompletion(env, promise, methodName, completion);
    } else {
        completion->Complete();
    }
}

void PowerHibernateCallback::RunCallback(HibernateCallbackEvent event,
    const std::shared_ptr<HibernateCallbackCompletion>& completion,
    uint64_t generation)
{
    std::lock_guard lock(callbackMutex_);
    if (callbackRef_ == nullptr || env_ == nullptr || generation_ != generation) {
        completion->Complete();
        return;
    }
    napi_handle_scope scope = nullptr;
    napi_status status = napi_open_handle_scope(env_, &scope);
    if (status != napi_ok || scope == nullptr) {
        POWER_HILOGW(FEATURE_SUSPEND, "Failed to open hibernate callback scope");
        completion->Complete();
        return;
    }
    napi_value callbackObj = nullptr;
    status = napi_get_reference_value(env_, callbackRef_, &callbackObj);
    if (status != napi_ok || callbackObj == nullptr) {
        POWER_HILOGW(FEATURE_SUSPEND, "napi_get_reference_value failed. status = %{public}d", status);
        completion->Complete();
    } else {
        InvokeCallback(env_, callbackObj, event, completion);
    }
    // A callback that throws synchronously leaves the exception pending, and every later NAPI call
    // on this env fails with napi_pending_exception. The outcome is already reported to the caller.
    bool pending = false;
    if (napi_is_exception_pending(env_, &pending) == napi_ok && pending) {
        napi_value exception = nullptr;
        napi_get_and_clear_last_exception(env_, &exception);
        POWER_HILOGW(FEATURE_SUSPEND, "clear pending exception of hibernate callback");
    }
    napi_close_handle_scope(env_, scope);
}

bool PowerHibernateCallback::PostCallback(HibernateCallbackEvent event,
    const std::shared_ptr<HibernateCallbackCompletion>& completion)
{
    napi_env dispatchEnv = nullptr;
    uint64_t generation = 0;
    {
        std::lock_guard lock(callbackMutex_);
        dispatchEnv = env_;
        generation = generation_;
    }
    if (dispatchEnv == nullptr) {
        return false;
    }
    sptr<PowerHibernateCallback> self(this);
    auto uvcallback = [self, event, completion, generation]() {
        self->RunCallback(event, completion, generation);
    };
    bool posted = napi_send_event(dispatchEnv, uvcallback, napi_eprio_low, "DispatchToWorker") == napi_status::napi_ok;
    if (!posted) {
        POWER_HILOGW(FEATURE_SUSPEND, "napi_send_event failed for hibernate callback");
    }
    return posted;
}

} // namespace PowerMgr
} // namespace OHOS
