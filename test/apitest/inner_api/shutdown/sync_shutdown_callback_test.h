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

#ifndef POWERMGR_POWER_MANAGER_ASYNC_SHUTDOWN_CALLBACK_TEST_H
#define POWERMGR_POWER_MANAGER_ASYNC_SHUTDOWN_CALLBACK_TEST_H

#include <gtest/gtest.h>

#include "shutdown/sync_shutdown_callback_stub.h"

namespace OHOS {
namespace PowerMgr {
namespace UnitTest {
class SyncShutdownCallbackTest : public testing::Test {
public:
    class SyncShutdownCallback : public SyncShutdownCallbackStub {
    public:
        ~SyncShutdownCallback() override = default;
        void OnSyncShutdown() override;
    };

    class HighPrioritySyncShutdownCallback : public SyncShutdownCallbackStub {
    public:
        ~HighPrioritySyncShutdownCallback() override = default;
        void OnSyncShutdown() override;
    };

    class LowPrioritySyncShutdownCallback : public SyncShutdownCallbackStub {
    public:
        ~LowPrioritySyncShutdownCallback() override = default;
        void OnSyncShutdown() override;
    };

    class NotSyncShutdownCallback : public SyncShutdownCallbackStub {
        ~NotSyncShutdownCallback() override = default;
        void OnSyncShutdown() override;
    };

protected:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};
} // namespace UnitTest
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_ASYNC_SHUTDOWN_CALLBACK_TEST_H
