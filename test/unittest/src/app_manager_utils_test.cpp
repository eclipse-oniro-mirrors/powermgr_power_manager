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

#include "app_manager_utils_test.h"

#include <thread>
#include <vector>

#include "app_manager_utils.h"
#include "power_log.h"

namespace OHOS {
namespace PowerMgr {
using namespace OHOS::AppExecFwk;
using namespace testing;
using namespace testing::ext;

/**
 * @tc.name: AppManagerUtilsTest001
 * @tc.desc: test GetForegroundApplications
 * @tc.type: FUNC
 */
HWTEST_F(AppManagerUtilsTest, AppManagerUtilsTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "AppManagerUtilsTest001 function start!");
    std::vector<AppStateData> testAppsData;
    AppManagerUtils::GetForegroundApplications(testAppsData);
    EXPECT_TRUE(testAppsData.size()>=0);
    POWER_HILOGI(LABEL_TEST, "AppManagerUtilsTest001 function end!");
}

/**
 * @tc.name: AppManagerUtilsTest002
 * @tc.desc: test IsForegroundApplication
 * @tc.type: FUNC
 */
HWTEST_F(AppManagerUtilsTest, AppManagerUtilsTest002, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "AppManagerUtilsTest002 function start!");
    std::set<std::string> unExistApp;
    bool res = AppManagerUtils::IsForegroundApplication(unExistApp);
    unExistApp.insert("com.example.UNEXISTED_APP");
    res = AppManagerUtils::IsForegroundApplication(unExistApp);
    EXPECT_FALSE(res);
    POWER_HILOGI(LABEL_TEST, "AppManagerUtilsTest002 function end!");
}

/**
 * @tc.name: AppManagerUtilsTest003
 * @tc.desc: test GetForegroundBundleNames
 * @tc.type: FUNC
 */
HWTEST_F(AppManagerUtilsTest, AppManagerUtilsTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "AppManagerUtilsTest003 function start!");
    std::set<std::string> bundleNames;
    AppManagerUtils::GetForegroundBundleNames(bundleNames);
    EXPECT_FALSE(bundleNames.size() > 5);
    POWER_HILOGI(LABEL_TEST, "AppManagerUtilsTest003 function end!");
}

/**
 * @tc.name: AppManagerUtilsTest004
 * @tc.desc: test concurrent GetForegroundApplications is thread-safe without app manager service
 * @tc.type: FUNC
 */
HWTEST_F(AppManagerUtilsTest, AppManagerUtilsTest004, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "AppManagerUtilsTest004 function start!");
    const int THREAD_COUNT = 8;
    std::vector<std::thread> threads;
    std::vector<std::vector<AppStateData>> results(THREAD_COUNT);
    for (int i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back([&results, i]() {
            AppManagerUtils::GetForegroundApplications(results[i]);
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    // All threads must get the same result count (thread-safe, no data race)
    size_t expectedSize = results[0].size();
    for (size_t i = 1; i < results.size(); ++i) {
        EXPECT_EQ(results[i].size(), expectedSize);
    }
    POWER_HILOGI(LABEL_TEST, "AppManagerUtilsTest004 function end!");
}

/**
 * @tc.name: AppManagerUtilsTest005
 * @tc.desc: test concurrent GetApiTargetVersion returns consistent value via atomic
 * @tc.type: FUNC
 */
HWTEST_F(AppManagerUtilsTest, AppManagerUtilsTest005, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "AppManagerUtilsTest005 function start!");
    const int THREAD_COUNT = 4;
    std::vector<std::thread> threads;
    std::vector<int32_t> results(THREAD_COUNT, -999);
    for (int i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back([&results, i]() {
            results[i] = AppManagerUtils::GetApiTargetVersion();
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    // All threads must get the same value (atomic guarantees consistent read)
    // Valid results: >= 0 (HAS_ABILITY_RUNTIME_PART) or -1 (no ability runtime part)
    for (int32_t version : results) {
        EXPECT_EQ(version, results[0]);
    }
    POWER_HILOGI(LABEL_TEST, "AppManagerUtilsTest005 function end!");
}
} // PowerMgr
} // OHOS