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

#include "shutdown/sync_shutdown_callback_stub.h"

#include "message_parcel.h"
#include "power_common.h"
#include "xcollie.h"
#include "xcollie_define.h"

namespace OHOS {
namespace PowerMgr {
int SyncShutdownCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    POWER_HILOGD(FEATURE_SHUTDOWN, "cmd=%{public}d, flags=%{public}d", code, option.GetFlags());
    const int32_t DFX_DELAY_MS = 10000;
    int32_t id = HiviewDFX::XCollie::GetInstance().SetTimer(
        "PowerShutdownCallback", DFX_DELAY_MS, nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_NOOP);
    std::u16string descripter = SyncShutdownCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        POWER_HILOGE(COMP_SVC, "Descriptor is not match");
        return E_GET_POWER_SERVICE_FAILED;
    }

    int32_t ret;
    if (code == ISyncShutdownCallback::CMD_ON_SYNC_SHUTDOWN) {
        ret = OnSyncShutdownCallbackStub(data, reply);
    } else {
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}

int32_t SyncShutdownCallbackStub::OnSyncShutdownCallbackStub(MessageParcel& data, MessageParcel& reply)
{
    OnSyncShutdown();
    return ERR_OK;
}

} // namespace PowerMgr
} // namespace OHOS