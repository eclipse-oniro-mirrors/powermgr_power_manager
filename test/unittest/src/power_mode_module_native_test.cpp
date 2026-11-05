/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "power_mode_module_native_test.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

void PowerModeModuleNativeTest::SetUpTestCase()
{
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    pmsTest->OnStart();
}

void PowerModeModuleNativeTest::TearDownTestCase()
{
}

void PowerModeModuleNativeTest::SetUp()
{
}

void PowerModeModuleNativeTest::TearDown()
{
}

void PowerModeTestCallback::OnPowerModeChanged(PowerMode mode)
{
    POWER_HILOGI(LABEL_TEST, "PowerModeTestCallback::OnPowerModeChanged.");
}

namespace OHOS::EventFwk {
bool CommonEventManager::SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber>& subscriber)
{
    bool ret = true;
    static std::list<bool> data = {false, true, true, true};
    if (!data.empty()) {
        ret = data.front();
        data.pop_front();
    }
    return ret;
}
} // OHOS::EventFwk

namespace {
using ModeActionPolicy = std::function<void(bool)>;
constexpr int32_t COMMON_EVENT_SERVICE_ID = 3299;

/**
 * @tc.name: PowerModeModuleNativeTest001
 * @tc.desc: test callback in PowerModeModule
 */
HWTEST_F (PowerModeModuleNativeTest, PowerModeModuleNativeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerModeModuleNativeTest001 start.";
    POWER_HILOGI(LABEL_TEST, "PowerModeModuleNativeTest001 function start!");
    shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    powerModeModuleTest->mode_ = PowerMode::PERFORMANCE_MODE;
    powerModeModuleTest->PublishPowerModeEvent();
    powerModeModuleTest->mode_ = PowerMode::EXTREME_POWER_SAVE_MODE;
    powerModeModuleTest->PublishPowerModeEvent();
    powerModeModuleTest->mode_ = static_cast<PowerMode>(POWERMODE);
    powerModeModuleTest->PublishPowerModeEvent();

    sptr<IPowerModeCallback> object = new PowerModeTestCallback();
    powerModeModuleTest->AddPowerModeCallback(object);
    powerModeModuleTest->Prepare();
    powerModeModuleTest->AddPowerModeCallback(nullptr);
    powerModeModuleTest->DelPowerModeCallback(object);
    powerModeModuleTest->DelPowerModeCallback(nullptr);

    powerModeModuleTest->callbackMgr_ = nullptr;
    powerModeModuleTest->Prepare();
    powerModeModuleTest->AddPowerModeCallback(object);
    powerModeModuleTest->DelPowerModeCallback(object);
    powerModeModuleTest->started_ = true;
    PowerMode mode = PowerMode::PERFORMANCE_MODE;
    powerModeModuleTest->EnableMode(mode, true);

    powerModeModuleTest->mode_ = PowerMode::PERFORMANCE_MODE;
    powerModeModuleTest->SetModeItem(mode);
    EXPECT_EQ(mode, powerModeModuleTest->GetModeItem());
    mode = static_cast<PowerMode>(POWERMODE);
    powerModeModuleTest->SetModeItem(mode);
    mode = static_cast<PowerMode>(POWERMODE2);
    powerModeModuleTest->SetModeItem(mode);
    mode = static_cast<PowerMode>(POWERMODE3);
    powerModeModuleTest->SetModeItem(mode);

    shared_ptr<PowerModeModule::CallbackManager> powerModeModuleManager =
                                            make_shared<PowerModeModule::CallbackManager>();
    powerModeModuleManager->OnRemoteDied(nullptr);

    POWER_HILOGI(LABEL_TEST, "PowerModeModuleNativeTest001 function end!");
    GTEST_LOG_(INFO) << "PowerModeModuleNativeTest001 end.";
}

/**
 * @tc.name: PowerModeModuleNativeTest002
 * @tc.desc: test PowerModeModule policy action
 * @tc.type: FUNC
 */
HWTEST_F (PowerModeModuleNativeTest, PowerModeModuleNativeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerModeModuleNativeTest002 start.";
    POWER_HILOGI(LABEL_TEST, "PowerModeModuleNativeTest002 function start!");
    auto modePolicy = DelayedSingleton<PowerModePolicy>::GetInstance();
    std::shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    powerModeModuleTest->EnableMode(PowerMode::PERFORMANCE_MODE, false);
    EXPECT_EQ(powerModeModuleTest->GetModeItem(), PowerMode::PERFORMANCE_MODE);
    modePolicy->switchMap_.clear();
    modePolicy->switchMap_.emplace(PowerModePolicy::ServiceType::SOC_PERF, PowerModeModule::SocPerformance::STANDARD);
    powerModeModuleTest->SetSocPerfState(false);
    modePolicy->switchMap_[PowerModePolicy::ServiceType::SOC_PERF] = PowerModeModule::SocPerformance::HIGH;
    powerModeModuleTest->SetSocPerfState(false);
    constexpr int32_t INVALID_SOC_PERFORMANCE_CODE = 3;
    modePolicy->switchMap_[PowerModePolicy::ServiceType::SOC_PERF] = INVALID_SOC_PERFORMANCE_CODE;
    powerModeModuleTest->SetSocPerfState(false);
    POWER_HILOGI(LABEL_TEST, "PowerModeModuleNativeTest002 function end!");
    GTEST_LOG_(INFO) << "PowerModeModuleNativeTest002 end.";
}

/**
 * @tc.name: PowerModeModuleNativeTest003
 * @tc.desc: test PowerModeModule InitPowerMode
 * @tc.type: FUNC
 */
HWTEST_F (PowerModeModuleNativeTest, PowerModeModuleNativeTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerModeModuleNativeTest003 start.";
    POWER_HILOGI(LABEL_TEST, "PowerModeModuleNativeTest003 function start!");
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    // case 0: test SubscribeCommonEvent DATA_SHARE_READY fail
    std::shared_ptr<PowerModeModule> powerModeModuleTest = make_shared<PowerModeModule>();
    EXPECT_TRUE(powerModeModuleTest->subscriberPtr_ == nullptr);
    powerModeModuleTest->SubscribeCommonEvent();
    EXPECT_TRUE(powerModeModuleTest->subscriberPtr_ == nullptr);
    // case 1: test SubscribeCommonEvent DATA_SHARE_READY success
    powerModeModuleTest->SubscribeCommonEvent();
    EXPECT_TRUE(powerModeModuleTest->subscriberPtr_ != nullptr);
    // case 2: test SubscribeCommonEvent DATA_SHARE_READY repeatedly
    powerModeModuleTest->SubscribeCommonEvent();
    EXPECT_TRUE(powerModeModuleTest->subscriberPtr_ != nullptr);
    // case 3: test PowerModeModule OnreceiveEvent empty want branch
    class TestPowerModeCallback : public IRemoteStub<IPowerModeCallback> {
    public:
        void OnPowerModeChanged(PowerMode mode) override
        {
            callCount_++;
        }
        uint32_t callCount_ {0};
    };
    sptr<TestPowerModeCallback> callback = new TestPowerModeCallback();
    pmsTest->GetPowerModeModule().AddPowerModeCallback(callback);
    OHOS::AAFwk::Want want;
    powerModeModuleTest->subscriberPtr_->OnReceiveEvent(OHOS::EventFwk::CommonEventData(want));
    EXPECT_TRUE(callback->callCount_ == 0);
    // case 4: test PowerModeModule OnreceiveEvent DATA_SHARE_READY branch
    want.SetAction(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_DATA_SHARE_READY);
    powerModeModuleTest->subscriberPtr_->OnReceiveEvent(OHOS::EventFwk::CommonEventData(want));
    EXPECT_TRUE(callback->callCount_ == 1); // InitPowerMode called only once
    // case 5: test PowerMgrService OnAddSystemAbilityInner
    pmsTest->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "");
    EXPECT_TRUE(callback->callCount_ == 1); // InitPowerMode called only once
    POWER_HILOGI(LABEL_TEST, "PowerModeModuleNativeTest003 function end!");
    GTEST_LOG_(INFO) << "PowerModeModuleNativeTest003 end.";
}

/**
 * @tc.name: PowerModePolicyNativeTest001
 * @tc.desc: test GetPowerMode in PowerModePolicy
 * @tc.type: FUNC
 */
HWTEST_F (PowerModeModuleNativeTest, PowerModePolicyNativeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PowerModePolicyNativeTest001 start.";
    POWER_HILOGI(LABEL_TEST, "PowerModePolicyNativeTest001 function start!");
    PowerModePolicy *powerModePolicyTest = new PowerModePolicy();
    EXPECT_TRUE(powerModePolicyTest->GetPowerModeValuePolicy(MODEITEM) == INIT_VALUE_FALSE);
    EXPECT_TRUE(powerModePolicyTest->GetPolicyFromMap(MODEITEM) == INIT_VALUE_FALSE);
    powerModePolicyTest->ReadPowerModePolicy(READODE);
    powerModePolicyTest->ComparePowerModePolicy();
    ModeActionPolicy action;
    powerModePolicyTest->AddAction(MODEITEM, action);
    POWER_HILOGI(LABEL_TEST, "PowerModePolicyNativeTest001 function end!");
    GTEST_LOG_(INFO) << "PowerModePolicyNativeTest001 end.";
}

/**
 * @tc.name: SetModeAndSubModeItem_001
 * @tc.desc: Test SetModeAndSubModeItem valid/invalid/same-mode-switch
 * @tc.type: FUNC
 */
HWTEST_F (PowerModeModuleNativeTest, SetModeAndSubModeItem_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetModeAndSubModeItem_001 start.";
    shared_ptr<PowerModeModule> module = make_shared<PowerModeModule>();
    EXPECT_EQ(module->GetSubModeItem(), PowerSubMode::SUB_MODE_INVALID);
    PowerErrors ret = module->SetModeAndSubModeItem(
        PowerMode::EXTREME_POWER_SAVE_MODE, PowerSubMode::SUB_MODE_SUPER_POWER_SAVE);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(module->GetModeItem(), PowerMode::EXTREME_POWER_SAVE_MODE);
    EXPECT_EQ(module->GetSubModeItem(), PowerSubMode::SUB_MODE_SUPER_POWER_SAVE);
    ret = module->SetModeAndSubModeItem(
        PowerMode::EXTREME_POWER_SAVE_MODE, PowerSubMode::SUB_MODE_EMERGENCY);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(module->GetSubModeItem(), PowerSubMode::SUB_MODE_EMERGENCY);
    PowerSubMode invalidSubMode = static_cast<PowerSubMode>(999);
    ret = module->SetModeAndSubModeItem(PowerMode::EXTREME_POWER_SAVE_MODE, invalidSubMode);
    EXPECT_EQ(ret, PowerErrors::ERR_PARAM_INVALID);
    EXPECT_EQ(module->GetSubModeItem(), PowerSubMode::SUB_MODE_EMERGENCY);
    GTEST_LOG_(INFO) << "SetModeAndSubModeItem_001 end.";
}

/**
 * @tc.name: SetModeItem_ForbidTransit_001
 * @tc.desc: Test SetModeItem forbidMap and subMode reset on leaving 603
 * @tc.type: FUNC
 */
HWTEST_F (PowerModeModuleNativeTest, SetModeItem_ForbidTransit_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetModeItem_ForbidTransit_001 start.";
    shared_ptr<PowerModeModule> module = make_shared<PowerModeModule>();
    module->mode_ = PowerMode::EXTREME_POWER_SAVE_MODE;
    module->subMode_ = PowerSubMode::SUB_MODE_SUPER_POWER_SAVE;
    PowerErrors ret = module->SetModeItem(PowerMode::PERFORMANCE_MODE);
    EXPECT_EQ(ret, PowerErrors::ERR_POWER_MODE_TRANSIT_FAILED);
    EXPECT_EQ(module->GetSubModeItem(), PowerSubMode::SUB_MODE_SUPER_POWER_SAVE);
    module->mode_ = PowerMode::EXTREME_POWER_SAVE_MODE;
    module->subMode_ = PowerSubMode::SUB_MODE_SUPER_POWER_SAVE;
    ret = module->SetModeItem(PowerMode::NORMAL_MODE);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(module->GetSubModeItem(), PowerSubMode::SUB_MODE_INVALID);
    GTEST_LOG_(INFO) << "SetModeItem_ForbidTransit_001 end.";
}

/**
 * @tc.name: GetDeviceSubMode_001
 * @tc.desc: Test GetDeviceSubMode query and PublishPowerModeEvent subMode
 * @tc.type: FUNC
 */
HWTEST_F (PowerModeModuleNativeTest, GetDeviceSubMode_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDeviceSubMode_001 start.";
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto& module = pmsTest->GetPowerModeModule();
    module.mode_ = PowerMode::EXTREME_POWER_SAVE_MODE;
    module.subMode_ = PowerSubMode::SUB_MODE_SUPER_POWER_SAVE;
    PowerSubMode subMode = pmsTest->GetDeviceSubMode();
    EXPECT_EQ(subMode, PowerSubMode::SUB_MODE_SUPER_POWER_SAVE);
    module.mode_ = PowerMode::NORMAL_MODE;
    module.subMode_ = PowerSubMode::SUB_MODE_INVALID;
    module.PublishPowerModeEvent();
    EXPECT_EQ(module.GetModeItem(), PowerMode::NORMAL_MODE);
    EXPECT_EQ(module.GetSubModeItem(), PowerSubMode::SUB_MODE_INVALID);
    module.mode_ = PowerMode::EXTREME_POWER_SAVE_MODE;
    module.PublishPowerModeEvent();
    EXPECT_EQ(module.GetModeItem(), PowerMode::EXTREME_POWER_SAVE_MODE);
    GTEST_LOG_(INFO) << "GetDeviceSubMode_001 end.";
}

/**
 * @tc.name: SetDeviceMode_001
 * @tc.desc: Test SetDeviceMode permission and association validation
 * @tc.type: FUNC
 */
HWTEST_F (PowerModeModuleNativeTest, SetDeviceMode_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetDeviceMode_001 start.";
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto& module = pmsTest->GetPowerModeModule();
    module.mode_ = PowerMode::NORMAL_MODE;
    PowerErrors ret = pmsTest->SetDeviceMode(
        PowerMode::NORMAL_MODE, PowerSubMode::SUB_MODE_SUPER_POWER_SAVE);
    EXPECT_EQ(ret, PowerErrors::ERR_POWER_MODE_TRANSIT_FAILED);
    ret = pmsTest->SetDeviceMode(
        PowerMode::NORMAL_MODE, PowerSubMode::SUB_MODE_EMERGENCY);
    EXPECT_EQ(ret, PowerErrors::ERR_POWER_MODE_TRANSIT_FAILED);
    PowerSubMode invalidSubMode = static_cast<PowerSubMode>(999);
    ret = pmsTest->SetDeviceMode(
        PowerMode::EXTREME_POWER_SAVE_MODE, invalidSubMode);
    EXPECT_EQ(ret, PowerErrors::ERR_PARAM_INVALID);
    ret = pmsTest->SetDeviceMode(
        PowerMode::EXTREME_POWER_SAVE_MODE, PowerSubMode::SUB_MODE_INVALID);
    EXPECT_EQ(ret, PowerErrors::ERR_POWER_MODE_TRANSIT_FAILED);
    ret = pmsTest->SetDeviceMode(
        PowerMode::NORMAL_MODE, PowerSubMode::SUB_MODE_INVALID);
    EXPECT_EQ(ret, PowerErrors::ERR_POWER_MODE_TRANSIT_FAILED);
    ret = pmsTest->SetDeviceMode(
        PowerMode::EXTREME_POWER_SAVE_MODE, PowerSubMode::SUB_MODE_SUPER_POWER_SAVE);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(module.GetSubModeItem(), PowerSubMode::SUB_MODE_SUPER_POWER_SAVE);
    GTEST_LOG_(INFO) << "SetDeviceMode_001 end.";
}

/**
 * @tc.name: SetDeviceMode_002
 * @tc.desc: Test SetDeviceMode (old API) resets subMode correctly
 * @tc.type: FUNC
 */
HWTEST_F (PowerModeModuleNativeTest, SetDeviceMode_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetDeviceMode_002 start.";
    auto pmsTest = DelayedSpSingleton<PowerMgrService>::GetInstance();
    auto& module = pmsTest->GetPowerModeModule();
    module.mode_ = PowerMode::NORMAL_MODE;
    module.subMode_ = PowerSubMode::SUB_MODE_INVALID;
    PowerErrors ret = pmsTest->SetDeviceMode(PowerMode::EXTREME_POWER_SAVE_MODE);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(module.GetModeItem(), PowerMode::EXTREME_POWER_SAVE_MODE);
    EXPECT_EQ(module.GetSubModeItem(), PowerSubMode::SUB_MODE_EMERGENCY);
    ret = pmsTest->SetDeviceMode(PowerMode::NORMAL_MODE);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(module.GetModeItem(), PowerMode::NORMAL_MODE);
    EXPECT_EQ(module.GetSubModeItem(), PowerSubMode::SUB_MODE_INVALID);
    GTEST_LOG_(INFO) << "SetDeviceMode_002 end.";
}

/**
 * @tc.name: SetModeAndSubModeItem_002
 * @tc.desc: Test same-mode subMode switch and rollback on failure
 * @tc.type: FUNC
 */
HWTEST_F (PowerModeModuleNativeTest, SetModeAndSubModeItem_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetModeAndSubModeItem_002 start.";
    shared_ptr<PowerModeModule> module = make_shared<PowerModeModule>();
    module->mode_ = PowerMode::EXTREME_POWER_SAVE_MODE;
    module->subMode_ = PowerSubMode::SUB_MODE_SUPER_POWER_SAVE;
    PowerErrors ret = module->SetModeAndSubModeItem(
        PowerMode::EXTREME_POWER_SAVE_MODE, PowerSubMode::SUB_MODE_EMERGENCY);
    EXPECT_EQ(ret, PowerErrors::ERR_OK);
    EXPECT_EQ(module->GetSubModeItem(), PowerSubMode::SUB_MODE_EMERGENCY);
    module->mode_ = PowerMode::EXTREME_POWER_SAVE_MODE;
    module->subMode_ = PowerSubMode::SUB_MODE_EMERGENCY;
    ret = module->SetModeAndSubModeItem(
        PowerMode::PERFORMANCE_MODE, PowerSubMode::SUB_MODE_SUPER_POWER_SAVE);
    EXPECT_EQ(ret, PowerErrors::ERR_POWER_MODE_TRANSIT_FAILED);
    EXPECT_EQ(module->GetSubModeItem(), PowerSubMode::SUB_MODE_EMERGENCY);
    GTEST_LOG_(INFO) << "SetModeAndSubModeItem_002 end.";
}
}
