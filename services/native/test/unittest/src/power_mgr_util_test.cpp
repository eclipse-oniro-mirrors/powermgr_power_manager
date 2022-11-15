/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "power_mgr_util_test.h"

#include "permission.h"
#include "setting_observer.h"
#include "setting_provider.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
/**
 * @tc.name: PowerMgrUtilTest001
 * @tc.desc: test IsSystemCore
 * @tc.type: FUNC
 * @tc.require: issueI5IUHE
 */
HWTEST_F (PowerMgrUtilTest, PowerMgrUtilTest001, TestSize.Level2)
{
    EXPECT_FALSE(Permission::IsSystemCore());
    EXPECT_FALSE(Permission::IsSystemBasic());
    EXPECT_FALSE(Permission::IsSystemApl());
    EXPECT_FALSE(Permission::IsSystemHap());
    EXPECT_FALSE(Permission::IsSystem());
    EXPECT_TRUE(Permission::IsPermissionGranted(""));
    EXPECT_FALSE(Permission::IsSystemHapPermGranted(""));
}
} // namespace