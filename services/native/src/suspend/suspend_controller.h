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

#ifndef POWERMGR_SUSPEND_CONTROLLER_H
#define POWERMGR_SUSPEND_CONTROLLER_H

#include <cinttypes>
#include <functional>
#include <memory>
#include <vector>

#include "ffrt_utils.h"
#include "power_state_machine.h"
#ifdef HAS_SENSORS_SENSOR_PART
#include "sensor_agent.h"
#endif
#include "shutdown_controller.h"
#include "suspend_source_parser.h"
#include "suspend_sources.h"
#include "sleep_callback_holder.h"
#ifdef HAS_MULTIMODALINPUT_INPUT_PART
#include "i_input_event_consumer.h"
#endif

namespace OHOS {
namespace PowerMgr {

using SuspendListener = std::function<void(SuspendDeviceType, uint32_t, uint32_t)>;
class SuspendMonitor;
class SuspendEventHandler;
class SuspendController : public std::enable_shared_from_this<SuspendController> {
public:
    SuspendController(const std::shared_ptr<ShutdownController>& shutdownController,
        const std::shared_ptr<PowerStateMachine>& stateMachine, const std::shared_ptr<FFRTTimer>& ffrtTimer_);
    ~SuspendController();
    void Init();
    void ExecSuspendMonitorByReason(SuspendDeviceType reason);
    void RegisterSettingsObserver();
    void UnregisterSettingsObserver();
    void Execute();
    void Cancel();
    void StopSleep();
    void HandleEvent(int64_t delayTime);
    void CancelEvent();
    void HandleAction(SuspendDeviceType reason, uint32_t action);
    void RecordPowerKeyDown(bool interrupting = false);
    bool GetPowerkeyDownWhenScreenOff();

    void AddCallback(const sptr<ISyncSleepCallback>& callback, SleepPriority priority);
    void RemoveCallback(const sptr<ISyncSleepCallback>& callback);
    void TriggerSyncSleepCallback(bool isWakeup);
    void UpdateSuspendSources();

    std::shared_ptr<PowerStateMachine> GetStateMachine() const
    {
        return stateMachine_;
    }
    SuspendDeviceType GetLastReason() const
    {
        return sleepReason_;
    }
    uint32_t GetLastAction() const
    {
        return sleepAction_;
    }
    std::shared_ptr<SuspendMonitor> GetSpecifiedSuspendMonitor(SuspendDeviceType type) const;
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    void PowerOffInternalScreen(SuspendDeviceType type);
    void PowerOffAllScreens(SuspendDeviceType type);
#endif
    void StartSleepTimer(SuspendDeviceType reason, uint32_t action, uint32_t delay);
    void Reset();

#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    void SetForceSleepingFlag(bool isForceSleeping)
    {
        forceSleeping_.store(isForceSleeping, std::memory_order_relaxed);
    }
    bool GetForceSleepingFlag()
    {
        return forceSleeping_.load();
    }
#endif
#ifdef POWER_MANAGER_WAKEUP_ACTION
    bool GetLowCapacityPowerKeyFlag();
    void SetLowCapacityPowerKeyFlag(bool flag);
#endif

private:
    void ControlListener(SuspendDeviceType reason, uint32_t action, uint32_t delay);
    void HandleAutoSleep(SuspendDeviceType reason);
    void HandleForceSleep(SuspendDeviceType reason);
    void HandleHibernate(SuspendDeviceType reason);
    void HandleShutdown(SuspendDeviceType reason);
#ifdef POWER_MANAGER_ENABLE_EXTERNAL_SCREEN_MANAGEMENT
    bool IsPowerOffInernalScreenOnlyScene(SuspendDeviceType reason, SuspendAction action, bool isScreenOn) const;
    void ProcessPowerOffInternalScreenOnly(const sptr<PowerMgrService>& pms, SuspendDeviceType reason);
#endif

    void TriggerSyncSleepCallbackInner(
        SleepCallbackHolder::SleepCallbackContainerType& callbacks, const std::string& priority, bool isWakeup);
    static constexpr int32_t FORCE_SLEEP_DELAY_MS = 8000;
    void SuspendWhenScreenOff(SuspendDeviceType reason, uint32_t action, uint32_t delay);
    std::vector<SuspendSource> sourceList_;
    std::map<SuspendDeviceType, std::shared_ptr<SuspendMonitor>> monitorMap_;
    std::shared_ptr<ShutdownController> shutdownController_;
    std::shared_ptr<PowerStateMachine> stateMachine_;
    uint32_t sleepDuration_ {0};
    int64_t sleepTime_ {-1};
    SuspendDeviceType sleepReason_ {0};
    uint32_t sleepAction_ {0};
    uint32_t sleepType_ {0};
    bool powerkeyDownWhenScreenOff_ = false;
    std::mutex mutex_;
    std::mutex sleepCbMutex_;
    std::shared_ptr<FFRTTimer> ffrtTimer_;
    FFRTMutexMap ffrtMutexMap_;
#ifdef POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST
    std::atomic<bool> forceSleeping_ {false};
#endif
    sptr<IPowerStateCallback> suspendPowerStateCallback_ {nullptr};
#ifdef POWER_MANAGER_WAKEUP_ACTION
    std::atomic<bool> isLowCapacityPowerKey_ {false};
#endif
};

class SuspendMonitor {
public:
    const static std::shared_ptr<SuspendMonitor> CreateMonitor(SuspendSource& source);

    virtual ~SuspendMonitor() = default;
    virtual bool Init() = 0;
    virtual void Cancel() = 0;
    virtual void HandleEvent()
    {
        // do nothing in base class
    }
    SuspendDeviceType GetReason() const
    {
        return reason_;
    }
    uint32_t GetAction() const
    {
        return action_;
    }
    uint32_t GetDelay() const
    {
        return delayMs_;
    }
    void RegisterListener(SuspendListener listener)
    {
        listener_ = listener;
    }

    void Notify()
    {
        if (listener_ == nullptr) {
            return;
        }
        listener_(reason_, action_, delayMs_);
    }
protected:
    explicit SuspendMonitor(const SuspendSource& source)
    {
        reason_ = source.GetReason();
        action_ = source.GetAction();
        delayMs_ = source.GetDelay();
    }

    SuspendDeviceType reason_;
    uint32_t action_;
    uint32_t delayMs_;
    SuspendListener listener_;
};

class PowerKeySuspendMonitor : public SuspendMonitor {
public:
    explicit PowerKeySuspendMonitor(SuspendSource& source) : SuspendMonitor(source) {}
    ~PowerKeySuspendMonitor() override = default;
    bool Init() override;
    void Cancel() override;
    static inline std::atomic<bool> powerkeyScreenOff_ {false};
private:
    void BeginPowerkeyScreenOff() const;
    void EndPowerkeyScreenOff() const;
    void ReceivePowerkeyCallback(std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent);
    static constexpr int32_t LONG_PRESS_DELAY_MS = 3000;
    static constexpr int32_t POWER_KEY_PRESS_DELAY_MS = 10000;
    int32_t powerkeyReleaseId_ {-1};
};

class TimeoutSuspendMonitor : public SuspendMonitor {
public:
    explicit TimeoutSuspendMonitor(SuspendSource& source) : SuspendMonitor(source) {}
    ~TimeoutSuspendMonitor() override = default;
    bool Init() override;
    void Cancel() override;
    void HandleEvent() override;
};

class LidSuspendMonitor : public SuspendMonitor {
public:
    explicit LidSuspendMonitor(SuspendSource& source) : SuspendMonitor(source) {}
    ~LidSuspendMonitor() override = default;
    bool Init() override;
    void Cancel() override;
};

class SwitchSuspendMonitor : public SuspendMonitor {
public:
    explicit SwitchSuspendMonitor(SuspendSource& source) : SuspendMonitor(source) {}
    ~SwitchSuspendMonitor() override = default;
    bool Init() override;
    void Cancel() override;
};

class TPCoverSuspendMonitor : public SuspendMonitor, public std::enable_shared_from_this<TPCoverSuspendMonitor> {
public:
    explicit TPCoverSuspendMonitor(SuspendSource& source) : SuspendMonitor(source) {}
    ~TPCoverSuspendMonitor() override = default;
    bool Init() override;
    void Cancel() override;
    int32_t TPCoverReleaseId_ {-1};
};

} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_SUSPEND_CONTROLLER_H
