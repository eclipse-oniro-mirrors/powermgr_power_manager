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

#ifndef POWER_LOG_H
#define POWER_LOG_H

#define CONFIG_HILOG
#ifdef CONFIG_HILOG

#include <stdint.h>

#include "hilog/log.h"

namespace OHOS {
namespace PowerMgr {

#ifdef POWER_HILOGF
#undef POWER_HILOGF
#endif

#ifdef POWER_HILOGE
#undef POWER_HILOGE
#endif

#ifdef POWER_HILOGW
#undef POWER_HILOGW
#endif

#ifdef POWER_HILOGI
#undef POWER_HILOGI
#endif

#ifdef POWER_HILOGD
#undef POWER_HILOGD
#endif

#ifdef POWER_KHILOGF
#undef POWER_KHILOGF
#endif

#ifdef POWER_KHILOGE
#undef POWER_KHILOGE
#endif

#ifdef POWER_KHILOGW
#undef POWER_KHILOGW
#endif

#ifdef POWER_KHILOGI
#undef POWER_KHILOGI
#endif

#ifdef POWER_KHILOGD
#undef POWER_KHILOGD
#endif

namespace {
// Power manager reserved domain id range
constexpr unsigned int POWER_DOMAIN_ID_START = 0xD002900;
constexpr unsigned int POWER_DOMAIN_ID_END = POWER_DOMAIN_ID_START + 32;
constexpr unsigned int TEST_DOMAIN_ID = 0xD000F00;
}

enum PowerManagerLogLabel {
    // Component labels, you can add if needed
    COMP_LOCK = 0,
    COMP_FWK = 1,
    COMP_SVC = 2,
    COMP_HDI = 3,
    COMP_DRV = 4,
    COMP_UTILS = 5,
    // Feature labels, use to mark major features
    FEATURE_WAKEUP,
#ifdef POWER_MANAGER_WAKEUP_ACTION
    FEATURE_WAKEUP_ACTION,
#endif
    FEATURE_SUSPEND,
    FEATURE_RUNNING_LOCK,
    FEATURE_ACTIVITY,
    FEATURE_POWER_STATE,
    FEATURE_SCREEN_OFF_PRE,
    FEATURE_POWER_MODE,
    FEATURE_SHUTDOWN,
    FEATURE_INPUT,
    FEATURE_UTIL,
    // Test label
    LABEL_TEST,
    // The end of labels, max to the domain id range length 32
    LABEL_END,
};

enum PowerManagerLogDomain {
    DOMAIN_LOCK = POWER_DOMAIN_ID_START + COMP_LOCK, // 0xD002900
    DOMAIN_FRAMEWORK, // 0xD002901
    DOMAIN_SERVICE, // 0xD002902
    DOMAIN_HDI, // 0xD002903
    DOMAIN_DRIVER, // 0xD002904
    DOMAIN_UTILS, // 0xD002905
    DOMAIN_FEATURE_WAKEUP,
#ifdef POWER_MANAGER_WAKEUP_ACTION
    DOMAIN_FEATURE_WAKEUP_ACTION,
#endif
    DOMAIN_FEATURE_SUSPEND,
    DOMAIN_FEATURE_RUNNING_LOCK,
    DOMAIN_FEATURE_ACTIVITY,
    DOMAIN_FEATURE_POWER_STATE,
    DOMAIN_FEATURE_SCREEN_OFF_PRE,
    DOMAIN_FEATURE_POWER_MODE,
    DOMAIN_FEATURE_SHUTDOWN,
    DOMAIN_FEATURE_INPUT,
    DOMAIN_FEATURE_UTIL,
    DOMAIN_TEST = TEST_DOMAIN_ID, // 0xD000F00
    DOMAIN_END = POWER_DOMAIN_ID_END, // Max to 0xD002920, keep the sequence and length same as PowerManagerLogLabel
};

struct PowerManagerLogLabelDomain {
    unsigned int domainId;
    const char* tag;
};

// Keep the sequence and length same as PowerManagerLogDomain
static const PowerManagerLogLabelDomain POWER_LABEL[LABEL_END] = {
    {DOMAIN_LOCK,                  "PowerLock"},
    {DOMAIN_FRAMEWORK,            "PowerFwk"},
    {DOMAIN_SERVICE,              "PowerSvc"},
    {DOMAIN_HDI,                  "PowerHdi"},
    {DOMAIN_DRIVER,               "PowerDrv"},
    {DOMAIN_UTILS,                "PowerUtils"},
    {DOMAIN_FEATURE_WAKEUP,       "PowerWakeup"},
#ifdef POWER_MANAGER_WAKEUP_ACTION
    {DOMAIN_FEATURE_WAKEUP_ACTION,       "PowerWakeupAction"},
#endif
    {DOMAIN_FEATURE_SUSPEND,      "PowerSuspend"},
    {DOMAIN_FEATURE_RUNNING_LOCK, "PowerRunningLock"},
    {DOMAIN_FEATURE_ACTIVITY,     "PowerActivity"},
    {DOMAIN_FEATURE_POWER_STATE,  "PowerState"},
    {DOMAIN_FEATURE_SCREEN_OFF_PRE,  "PowerScreenOffPre"},
    {DOMAIN_FEATURE_POWER_MODE,   "PowerMode"},
    {DOMAIN_FEATURE_SHUTDOWN,     "PowerShutdown"},
    {DOMAIN_FEATURE_INPUT,        "PowerInput"},
    {DOMAIN_FEATURE_UTIL,         "PowerUtil"},
    {DOMAIN_TEST,                 "PowerTest"},
};

#define POWER_HILOGF(domain, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_FATAL, POWER_LABEL[domain].domainId, POWER_LABEL[domain].tag, ##__VA_ARGS__))
#define POWER_HILOGE(domain, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_ERROR, POWER_LABEL[domain].domainId, POWER_LABEL[domain].tag, ##__VA_ARGS__))
#define POWER_HILOGW(domain, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_WARN, POWER_LABEL[domain].domainId, POWER_LABEL[domain].tag, ##__VA_ARGS__))
#define POWER_HILOGI(domain, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_INFO, POWER_LABEL[domain].domainId, POWER_LABEL[domain].tag, ##__VA_ARGS__))
#define POWER_HILOGD(domain, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_DEBUG, POWER_LABEL[domain].domainId, POWER_LABEL[domain].tag, ##__VA_ARGS__))

constexpr OHOS::HiviewDFX::HiLogLabel POWER_KERNEL_LABEL = {
    LOG_KMSG,
    DOMAIN_SERVICE,
    "POWER"
};

#define POWER_KHILOGF(domain, ...) \
    do { \
        (void)OHOS::HiviewDFX::HiLog::Fatal(POWER_KERNEL_LABEL, __VA_ARGS__); \
        ((void)HILOG_IMPL(LOG_CORE, LOG_FATAL, POWER_LABEL[domain].domainId, POWER_LABEL[domain].tag, ##__VA_ARGS__)); \
    } while (0)

#define POWER_KHILOGE(domain, ...) \
    do { \
        (void)OHOS::HiviewDFX::HiLog::Error(POWER_KERNEL_LABEL, __VA_ARGS__); \
        ((void)HILOG_IMPL(LOG_CORE, LOG_ERROR, POWER_LABEL[domain].domainId, POWER_LABEL[domain].tag, ##__VA_ARGS__)); \
    } while (0)

#define POWER_KHILOGW(domain, ...) \
    do { \
        (void)OHOS::HiviewDFX::HiLog::Warn(POWER_KERNEL_LABEL, __VA_ARGS__); \
        ((void)HILOG_IMPL(LOG_CORE, LOG_WARN, POWER_LABEL[domain].domainId, POWER_LABEL[domain].tag, ##__VA_ARGS__)); \
    } while (0)

#define POWER_KHILOGI(domain, ...) \
    do { \
        (void)OHOS::HiviewDFX::HiLog::Info(POWER_KERNEL_LABEL, __VA_ARGS__); \
        ((void)HILOG_IMPL(LOG_CORE, LOG_INFO, POWER_LABEL[domain].domainId, POWER_LABEL[domain].tag, ##__VA_ARGS__)); \
    } while (0)

#define POWER_KHILOGD(domain, ...) \
    do { \
        (void)OHOS::HiviewDFX::HiLog::Debug(POWER_KERNEL_LABEL, __VA_ARGS__); \
        ((void)HILOG_IMPL(LOG_CORE, LOG_DEBUG, POWER_LABEL[domain].domainId, POWER_LABEL[domain].tag, ##__VA_ARGS__)); \
    } while (0)

} // namespace PowerMgr
} // namespace OHOS

#else

#define POWER_HILOGF(...)
#define POWER_HILOGE(...)
#define POWER_HILOGW(...)
#define POWER_HILOGI(...)
#define POWER_HILOGD(...)

#define POWER_KHILOGF(...)
#define POWER_KHILOGE(...)
#define POWER_KHILOGW(...)
#define POWER_KHILOGI(...)
#define POWER_KHILOGD(...)

#endif // CONFIG_HILOG

#endif // POWER_LOG_H
