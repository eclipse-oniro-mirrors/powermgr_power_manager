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

#ifndef POWERMGR_POWER_MANAGER_PARALLEL_SHUTDOWN_CALLBACK_PROXY_H
#define POWERMGR_POWER_MANAGER_PARALLEL_SHUTDOWN_CALLBACK_PROXY_H

#include "iremote_broker.h"
#include "iremote_proxy.h"

#include "shutdown/iparallel_shutdown_callback.h"

namespace OHOS {
namespace PowerMgr {
class ParallelShutdownCallbackProxy : public IRemoteProxy<IParallelShutdownCallback> {
public:
    explicit ParallelShutdownCallbackProxy(const sptr<IRemoteObject>& object)
        : IRemoteProxy<IParallelShutdownCallback>(object)
    {
    }
    ~ParallelShutdownCallbackProxy() override = default;
    virtual void OnParallelShutdownOrReboot(bool isReboot, uint64_t identity) override;

private:
    static inline BrokerDelegator<ParallelShutdownCallbackProxy> delegator_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_PARALLEL_SHUTDOWN_CALLBACK_PROXY_H
