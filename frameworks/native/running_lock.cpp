/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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

#include "running_lock.h"

#include <cstdint>
#include <list>
#include <iosfwd>
#include <string>
#include <timer.h>
#include "errors.h"
#include "new"
#include "refbase.h"
#include "power_log.h"
#include "power_mgr_errors.h"
#include "running_lock_token_stub.h"

namespace OHOS {
namespace PowerMgr {
constexpr int32_t DEFAULT_TIMEOUT = 3000;
constexpr int32_t NOT_USE_TIMEOUT = -1;
RunningLock::RunningLock(const wptr<IPowerMgr>& proxy, const std::string& name, RunningLockType type)
    : proxy_(proxy)
{
    runningLockInfo_.name = name;
    runningLockInfo_.type = type;
}

RunningLock::~RunningLock()
{
    if (token_ != nullptr) {
        Release();
    }
}

PowerErrors RunningLock::Init()
{
    token_ = new (std::nothrow)RunningLockTokenStub();
    if (token_ == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Failed to create the RunningLockTokenStub");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    return Create();
}

PowerErrors RunningLock::Create()
{
    sptr<IPowerMgr> proxy = proxy_.promote();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "CProxy=null");
        return PowerErrors::ERR_CONNECTION_FAIL;
    }
    return proxy->CreateRunningLock(token_, runningLockInfo_);
}

PowerErrors RunningLock::Recover(const wptr<IPowerMgr>& proxy)
{
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "recover running lock name %{public}s type %{public}d",
        runningLockInfo_.name.c_str(), runningLockInfo_.type);
    proxy_ = proxy;
    return Create();
}

ErrCode RunningLock::UpdateWorkSource(const std::vector<int32_t>& workSources)
{
    sptr<IPowerMgr> proxy = proxy_.promote();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "UpProxy=null");
        return E_GET_POWER_SERVICE_FAILED;
    }
    if (!proxy->UpdateWorkSource(token_, workSources)) {
        return E_INNER_ERR;
    }
    return ERR_OK;
}

ErrCode RunningLock::Lock(int32_t timeOutMs)
{
    sptr<IPowerMgr> proxy = proxy_.promote();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "LProxy=null");
        return E_GET_POWER_SERVICE_FAILED;
    }
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Service side Lock call, timeOutMs=%{public}d", timeOutMs);
    if (runningLockInfo_.type == RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL) {
        timeOutMs = NOT_USE_TIMEOUT;
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "PROXIMITY not use timeout");
    }
    if (timeOutMs == 0) {
        timeOutMs = DEFAULT_TIMEOUT;
        POWER_HILOGW(FEATURE_RUNNING_LOCK, "use default timeout");
    }
    PowerErrors error = proxy->Lock(token_, timeOutMs);
    if (error != PowerErrors::ERR_OK) {
        return error == PowerErrors::ERR_PERMISSION_DENIED ? E_PERMISSION_DENIED : E_INNER_ERR;
    }
    return ERR_OK;
}

ErrCode RunningLock::UnLock()
{
    sptr<IPowerMgr> proxy = proxy_.promote();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "UnProxy=null");
        return E_GET_POWER_SERVICE_FAILED;
    }
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Service side UnLock call");
    PowerErrors error = proxy->UnLock(token_, runningLockInfo_.name);
    if (error != PowerErrors::ERR_OK) {
        return error == PowerErrors::ERR_PERMISSION_DENIED ? E_PERMISSION_DENIED : E_INNER_ERR;
    }
    return ERR_OK;
}

bool RunningLock::IsUsed()
{
    sptr<IPowerMgr> proxy = proxy_.promote();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "IProxy=null");
        return false;
    }
    bool ret = proxy->IsUsed(token_);
    POWER_HILOGD(FEATURE_RUNNING_LOCK, "Is Used: %{public}d", ret);
    return ret;
}

void RunningLock::Release()
{
    sptr<IPowerMgr> proxy = proxy_.promote();
    if (proxy == nullptr) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "RProxy=null");
        return;
    }
    // ReleaseRunningLock
    POWER_HILOGI(FEATURE_RUNNING_LOCK, "RlsN=%{public}s", runningLockInfo_.name.c_str());
    proxy->ReleaseRunningLock(token_, runningLockInfo_.name);
}
} // namespace PowerMgr
} // namespace OHOS
