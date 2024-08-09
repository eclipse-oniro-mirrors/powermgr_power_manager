/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "power_mode_callback_stub.h"

#include <message_parcel.h>
#include "errors.h"
#include "ipc_object_stub.h"
#include "power_common.h"
#include "power_log.h"
#include "power_mgr_errors.h"
#include "power_mode_callback_ipc_interface_code.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"

namespace OHOS {
namespace PowerMgr {
int PowerModeCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    POWER_HILOGD(COMP_SVC, "cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    const int DFX_DELAY_S = 10;
    int id = HiviewDFX::XCollie::GetInstance().SetTimer("PowerModeCallbackStub", DFX_DELAY_S, nullptr, nullptr,
        HiviewDFX::XCOLLIE_FLAG_LOG);
    std::u16string descripter = PowerModeCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        POWER_HILOGE(COMP_SVC, "Descriptor is not match");
        HiviewDFX::XCollie::GetInstance().CancelTimer(id);
        return E_GET_POWER_SERVICE_FAILED;
    }

    int ret = ERR_OK;
    if (code == static_cast<uint32_t>(PowerMgr::PowerModeCallbackInterfaceCode::POWER_MODE_CHANGED)) {
        ret = OnPowerModeCallbackStub(data);
    } else {
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}

int32_t PowerModeCallbackStub::OnPowerModeCallbackStub(MessageParcel& data)
{
    uint32_t mode;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, mode, E_READ_PARCEL_ERROR);
    OnPowerModeChanged(static_cast<PowerMode>(mode));
    return ERR_OK;
}
} // namespace PowerMgr
} // namespace OHOS
