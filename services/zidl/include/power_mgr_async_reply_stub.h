/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_SERVICES_ASYNC_REPLY_STUB_H
#define POWERMGR_SERVICES_ASYNC_REPLY_STUB_H

#include <iremote_stub.h>
#include "power_mgr_async_reply.h"

namespace OHOS {
namespace PowerMgr {

class PowerMgrStubAsync : public IRemoteStub<IPowerMgrAsync> {
public:
    DISALLOW_COPY_AND_MOVE(PowerMgrStubAsync);
    PowerMgrStubAsync() = default;
    ~PowerMgrStubAsync() override = default;
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    void SendAsyncReply(int reply) override;
    int WaitForAsyncReply(int timeout);
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    int asyncReply_ = { 0 };
    bool notified {false};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SERVICES_ASYNC_REPLY_STUB_H
