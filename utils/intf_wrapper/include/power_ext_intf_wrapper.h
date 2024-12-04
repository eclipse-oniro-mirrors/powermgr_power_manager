/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_UTILS_INTF_WRPAAER_POWER_EXT_INTF_WRAPPER_H
#define POWERMGR_UTILS_INTF_WRPAAER_POWER_EXT_INTF_WRAPPER_H

#include <vector>
#include "interface_loader.h"

namespace OHOS {
namespace PowerMgr {
class PowerExtIntfWrapper final : public NoCopyable {
public:
    enum class ErrCode : uint32_t {
        ERR_OK = 0,
        ERR_NOT_FOUND = 1,
        ERR_FAILURE = 2
    };

    static PowerExtIntfWrapper& Instance();
    virtual ~PowerExtIntfWrapper() noexcept
    {
        DeInit();
    }

    void Init()
    {
        intfLoader_.Init();
    }

    void DeInit()
    {
        intfLoader_.DeInit();
    }

    ErrCode GetRebootCommand(const std::string& rebootReason, std::string& rebootCmd) const;

private:
    PowerExtIntfWrapper(const std::string& libPath, const std::vector<std::string>& symbolArr)
        : intfLoader_(libPath, symbolArr) {}

    InterfaceLoader intfLoader_;
};

} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_UTILS_INTF_WRPAAER_POWER_EXT_INTF_WRAPPER_H