/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "shutdown/shutdown_client.h"

#include "client_lifecycle.h"
#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
ShutdownClient::ShutdownClient() {}
ShutdownClient::~ShutdownClient() {}

void ShutdownClient::RegisterShutdownCallback(
    const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority)
{
    auto proxy = ClientLifeCycle::GetProxy();
    RETURN_IF(proxy == nullptr);
    int32_t priorityValue = static_cast<int32_t>(priority);
    proxy->RegisterShutdownCallbackIpc(callback, priorityValue);
}

void ShutdownClient::UnRegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback)
{
    auto proxy = ClientLifeCycle::GetProxy();
    RETURN_IF(proxy == nullptr);
    proxy->UnRegisterShutdownCallbackIpc(callback);
}

void ShutdownClient::RegisterShutdownCallback(
    const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority)
{
    auto proxy = ClientLifeCycle::GetProxy();
    RETURN_IF(proxy == nullptr);
    int32_t priorityValue = static_cast<int32_t>(priority);
    proxy->RegisterShutdownCallbackIpc(callback, priorityValue);
}

void ShutdownClient::UnRegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback)
{
    auto proxy = ClientLifeCycle::GetProxy();
    RETURN_IF(proxy == nullptr);
    proxy->UnRegisterShutdownCallbackIpc(callback);
}

void ShutdownClient::RegisterShutdownCallback(
    const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority)
{
    auto proxy = ClientLifeCycle::GetProxy();
    RETURN_IF(proxy == nullptr);
    int32_t priorityValue = static_cast<int32_t>(priority);
    proxy->RegisterShutdownCallbackIpc(callback, priorityValue);
}

void ShutdownClient::UnRegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback)
{
    auto proxy = ClientLifeCycle::GetProxy();
    RETURN_IF(proxy == nullptr);
    proxy->UnRegisterShutdownCallbackIpc(callback);
}

PowerErrors ShutdownClient::RegisterShutdownCallback(const sptr<IParallelShutdownCallback>& callback)
{
    auto proxy = ClientLifeCycle::GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    proxy->RegisterShutdownCallbackIpc(callback, powerError);
    return static_cast<PowerErrors>(powerError);
}

PowerErrors ShutdownClient::UnRegisterShutdownCallback(const sptr<IParallelShutdownCallback>& callback)
{
    auto proxy = ClientLifeCycle::GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    proxy->UnRegisterShutdownCallbackIpc(callback, powerError);
    return static_cast<PowerErrors>(powerError);
}

PowerErrors ShutdownClient::VoteShutdown(uint64_t identity)
{
    auto proxy = ClientLifeCycle::GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, PowerErrors::ERR_CONNECTION_FAIL);
    int32_t powerError = static_cast<int32_t>(PowerErrors::ERR_CONNECTION_FAIL);
    proxy->VoteShutdownIpc(identity, powerError);
    return static_cast<PowerErrors>(powerError);
}
} // namespace PowerMgr
} // namespace OHOS
