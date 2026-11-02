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

#include <cstdint>

#ifdef POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION
#include "errors.h"
#include "os_account_manager.h"

using namespace OHOS;

namespace {
// Link-time mock state for OsAccountManager display<->user mapping. The adapter only ever calls
// these two static methods (the hot IsScreenOn uid->userId path uses pure local arithmetic, no IPC),
// so providing these two definitions here lets the test drive the per-user broadcast and
// IsScreenOnForUser success/failure paths without a live account IPC.
struct MockOsAccount {
    ErrCode localIdRet {ERR_OK};
    int32_t localId {100};
    ErrCode displayIdRet {ERR_OK};
    uint64_t displayId {0};
    int localIdCallCount {0};
    int displayIdCallCount {0};
};
MockOsAccount g_mockOsAccount;
} // namespace

namespace OHOS::PowerMgr::TestMock {
void ResetOsAccountMock()
{
    g_mockOsAccount = MockOsAccount {};
}
ErrCode& MockLocalIdRet()
{
    return g_mockOsAccount.localIdRet;
}
int32_t& MockLocalId()
{
    return g_mockOsAccount.localId;
}
ErrCode& MockDisplayIdRet()
{
    return g_mockOsAccount.displayIdRet;
}
uint64_t& MockDisplayId()
{
    return g_mockOsAccount.displayId;
}
int& MockLocalIdCallCount()
{
    return g_mockOsAccount.localIdCallCount;
}
int& MockDisplayIdCallCount()
{
    return g_mockOsAccount.displayIdCallCount;
}
} // namespace OHOS::PowerMgr::TestMock

namespace OHOS {
namespace AccountSA {
ErrCode OsAccountManager::GetForegroundOsAccountLocalId(const uint64_t displayId, int32_t& localId)
{
    (void)displayId;
    g_mockOsAccount.localIdCallCount++;
    localId = g_mockOsAccount.localId;
    return g_mockOsAccount.localIdRet;
}

ErrCode OsAccountManager::GetForegroundOsAccountDisplayId(const int32_t localId, uint64_t& displayId)
{
    (void)localId;
    g_mockOsAccount.displayIdCallCount++;
    displayId = g_mockOsAccount.displayId;
    return g_mockOsAccount.displayIdRet;
}
} // namespace AccountSA
} // namespace OHOS
#endif // POWER_MANAGER_ENABLE_MULTI_USER_SCREEN_ADAPTATION
