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

#ifndef POWERMGR_UTILS_LIB_LOADER_INTERFACE_LOADER_H
#define POWERMGR_UTILS_LIB_LOADER_INTERFACE_LOADER_H

#include "library_loader.h"
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace OHOS {
namespace PowerMgr {
class InterfaceLoader : public LibraryLoader {
public:
    InterfaceLoader(const std::string& libPath, const std::vector<std::string>& symbolArr);
    virtual ~InterfaceLoader() noexcept;
    bool Init();
    void DeInit();
    void* QueryInterface(const std::string& symbol) const;

private:
    bool LoadAllInterfaces();

    std::unordered_map<std::string, void*> interfaces_;
    mutable std::shared_mutex shMutex_;
    bool isInited_ {false};
};

} // namespace PowerMgr
}

#endif