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

#include "shutdown/takeover_shutdown_callback_stub.h"

#include "message_parcel.h"
#include "power_common.h"
#include "shutdown/takeover_shutdown_callback_ipc_interface_code.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"

namespace OHOS {
namespace PowerMgr {
int TakeOverShutdownCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    POWER_HILOGD(FEATURE_SHUTDOWN, "cmd=%{public}d, flags=%{public}d", code, option.GetFlags());
    const int32_t DFX_DELAY_S = 60;
    const int32_t INVALID_ID = 0;
    int32_t id = HiviewDFX::XCollie::GetInstance().SetTimer(
        "PowerShutdownCallback", DFX_DELAY_S, nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_LOG);
    if (id <= INVALID_ID) {
        POWER_HILOGE(COMP_SVC, "SetTimer failed");
    }
    std::u16string descripter = TakeOverShutdownCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        POWER_HILOGE(COMP_SVC, "Descriptor is not match");
        HiviewDFX::XCollie::GetInstance().CancelTimer(id);
        return E_GET_POWER_SERVICE_FAILED;
    }

    int32_t ret = ERR_OK;
    if (code == static_cast<uint32_t>(PowerMgr::TakeoverShutdownCallbackInterfaceCode::CMD_ON_TAKEOVER_SHUTDOWN)) {
        ret = OnTakeOverShutdownCallbackStub(data, reply);
    } else if (code ==
        static_cast<uint32_t>(PowerMgr::TakeoverShutdownCallbackInterfaceCode::CMD_ON_TAKEOVER_HIBERNATE)) {
        ret = OnTakeOverHibernateCallbackStub(data, reply);
    } else {
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}

int32_t TakeOverShutdownCallbackStub::OnTakeOverShutdownCallbackStub(MessageParcel& data, MessageParcel& reply)
{
    TakeOverInfo *info = data.ReadParcelable<TakeOverInfo>();
    bool isTakeOver = false;
    if (info != nullptr) {
        isTakeOver = OnTakeOverShutdown(*info);
        delete info;
    }
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Bool, isTakeOver, E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t TakeOverShutdownCallbackStub::OnTakeOverHibernateCallbackStub(MessageParcel& data, MessageParcel& reply)
{
    TakeOverInfo *info = data.ReadParcelable<TakeOverInfo>();
    bool isTakeOver = false;
    if (info != nullptr) {
        isTakeOver = OnTakeOverHibernate(*info);
        delete info;
    }
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Bool, isTakeOver, E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

} // namespace PowerMgr
} // namespace OHOS
