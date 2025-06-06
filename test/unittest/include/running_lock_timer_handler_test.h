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

#ifndef POWERMGR_RUNNING_LOCK_TIMER_HANDLER_TEST_H
#define POWERMGR_RUNNING_LOCK_TIMER_HANDLER_TEST_H

#ifdef THERMAL_GTEST
#define private    public
#define protected  public
#endif

#include <memory>

#include <gtest/gtest.h>
#include "running_lock_timer_handler.h"

namespace OHOS {
namespace PowerMgr {

class RunningLockTimerHandlerTest : public testing::Test {};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_RUNNING_LOCK_TIMER_HANDLER_TEST_H