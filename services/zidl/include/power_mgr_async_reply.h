/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_SERVICES_ASYNC_REPLY_H
#define POWERMGR_SERVICES_ASYNC_REPLY_H

#include <mutex>
#include <condition_variable>
#include <nocopyable.h>
#include "iremote_broker.h"

namespace OHOS {
namespace PowerMgr {
class IPowerMgrAsync : public IRemoteBroker {
public:
    enum PowerInterfaceId {
        SEND_ASYNC_REPLY = 0,
    };
    virtual void SendAsyncReply(int reply) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.powermgr.IPowerMgrAsync");
};

} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SERVICES_ASYNC_REPLY_H
