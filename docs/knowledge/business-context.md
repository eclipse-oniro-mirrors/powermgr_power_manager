# 业务背景

## 应用定位

### 核心职责
power_manager（部件名 `@ohos/power_manager`，powermgr 子系统，系统能力 `SystemCapability.PowerManager.PowerManager.Core`，`bundle.json`）实现 OpenHarmony 电源管理服务（Power Manager，SA ID 3301，`sa_profile/3301.json`），进程 `powermgr`，库 `libpowermgrservice.z.so`，`run-on-create: true`。核心职责：

- **电源状态机管理**：以 `PowerState` 十态（AWAKE/FREEZE/INACTIVE/STAND_BY/DOZE/SLEEP/HIBERNATE/SHUTDOWN/DIM/UNKNOWN，`interfaces/inner_api/native/include/power_state_machine_info.h:56-106`）维护整机电源状态，所有状态变更经 `PowerStateMachine::SetState` 单一入口仲裁（forbidMap/allowMapByReason/lockMap 三张规则表，`services/native/src/power_state_machine.cpp:138-199`）。
- **休眠与唤醒**：S3 内存睡眠经 `SystemSuspendController` 调 HDI `StartSuspend`/`ForceSuspend`（`services/native/src/actions/default/system_suspend_controller.cpp:198-201`）；S4 磁盘休眠（HibernateController）；ULSR 低时延挂起、带 tag 挂起与忽略运行锁的强休（HDI V1_4 ForceSuspendEx，默认关闭）。`actions/default/suspend/` 保留 AutoSuspend+wakeup_count 的 sysfs 睡眠实现（历史通道，默认流程不调用）。
- **亮灭屏时序**：状态机通过 actions 层驱动 display_manager（Rosen `DisplayManagerLite::SuspendBegin`/`WakeUpBegin`/`SetDisplayState`，`services/native/src/actions/default/display/device_state_action.cpp:164,232,254`），并管理用户活动超时链（AWAKE→DIM→INACTIVE→SLEEP）。
- **运行锁（RunningLock）管理**：13 类锁型（SCREEN/BACKGROUND/PROXIMITY_SCREEN_CONTROL/COORDINATION 及 7 种后台子类型，`interfaces/inner_api/native/include/running_lock_info.h:32-84`），服务端 `RunningLockMgr` 计数仲裁，经 HDI Hold/UnholdRunningLockExt 下沉为内核锁（`system_suspend_controller.cpp:253-266`），支持后台任务框架批量代理（proxy/unproxy）。
- **关机/重启**：三族回调（takeover 接管/async 异步/sync 同步，按 ShutdownPriority 排序）+ `DoRebootExt` 落地（`services/native/src/actions/default/device_power_action.cpp:34-60`）；关机弹窗由独立 ArkTS 应用 `power_dialog` 承载，超时兜底在服务端。
- **电源模式与档位策略**：五档模式（NORMAL=600/POWER_SAVE/PERFORMANCE/EXTREME_POWER_SAVE/CUSTOM=650，`interfaces/inner_api/native/include/power_mode_info.h:24-32`），按 `power_mode_config.xml` 策略下发/恢复显示超时、亮度、振动等设置项。
- **事件源管理**：唤醒源/休眠源均为 JSON 配置驱动（`services/native/profile/power_wakeup.json`、`power_suspend.json`），支持设置项在线修改；距离传感器灭屏（通话场景）、双击/拾起/翻盖唤醒、按键/触摸输入监听（MMI）。
- **DFX 观测**：HiSysEvent 双 domain（POWER：RUNNINGLOCK/SCREEN/STATE 统计 + 状态迁移失败 FAULT；POWER_UE：亮灭屏行为事件）、Hitrace（TAG POWER）、hidumper 与 power-shell CLI。

### 职责边界（本组件不负责）
- **屏幕状态与背光的最终执行**：亮灭屏经 display_manager（SA 3308）及其后的图形栈执行，本服务只发起请求并接收回调（`device_state_action.cpp`，显示状态真源在 display_manager）。
- **HDI 电源驱动与内核实现**：`/sys/power/*`、`/sys/hibernate/resume` 由内核提供，HDI Power 接口由 `drivers_interface_power` 仓维护，本仓是消费方（`min_hdi_proxy_version: ["libpower_proxy_1.3.z.so"]`，`sa_profile/3301.json`）。
- **关机弹窗 UI 与签名**：`power_dialog/` 为独立 ArkTS 应用（含签名目录），按 AGENTS.md 由 UI 团队维护，本服务仅负责拉起、长按监测与回包时序（`services/native/src/shutdown/shutdown_dialog.h`）。
- **设置项存储本体**：display off time、电源模式、唤醒源 JSON 等持久化在 settingsdata（DataShare），本服务经 `utils/setting/setting_provider.h` 读写订阅。
- **电池电量与充电管理**：电量、充电状态归 battery_manager（SA 3302，同进程）；本服务仅订阅公共事件（如通话状态、插拔）调整行为。
- **传感器驱动**：距离/霍尔/加速度传感器由 sensor 服务管理，本服务是订阅方（`sensor_agent.h`）。
- **关机充电 UI**：`power_manager_feature_poweroff_charge` 场景的 charger UI 在 battery_manager 仓（AGENTS.md 术语表）。
- **生成物与独立包**：`frameworks/ets/taihe/`（taihe FFI 生成产物）、`frameworks/cj/`（CJ 绑定）、`tools/ohos-powerManager/`（独立 npm CLI 包）按 AGENTS.md 属其他团队/生成物维护。

## 系统上下文

```plantuml
@startuml
title power_manager 系统上下文
left to right direction

actor "ArkTS 应用\n(@ohos.power / @ohos.runningLock)" as ArkTSApp
actor "C++ 系统组件\n(wms/bms/samgr 等)" as SysComp
actor "CLI\n(power-shell, 默认关闭)" as CLI

package "power_manager (SA 3301, 进程 powermgr)" {
  component "frameworks/napi/{power,runninglock}\nframeworks/ets/taihe, frameworks/cj" as NAPI
  component "PowerMgrClient / RunningLock\nShutdownClient (frameworks/native)" as Client
  component "PowerMgrService (SA 3301)\n+ PowerStateMachine + 控制器族" as Svc
  component "actions 层\nDeviceStateAction / DevicePowerAction\nRunningLockAction / SystemSuspendController" as Actions
}

package "外部服务" {
  component "display_manager (SA 3308/4607)\nDisplayManagerLite/DisplayPowerMgrClient" as DM
  component "HDI Power (drivers_interface_power)\nV1_2/V1_3/V1_4 IPowerInterface" as HDI
  component "multimodalinput (MMI)" as MMI
  component "sensor 服务" as Sensor
  component "settingsdata (DataShare)" as Settings
  component "startup init\n(DoRebootExt)" as Init
  component "power_dialog (ArkTS 弹窗)" as Dialog
}

kernel "Linux 内核" as Kernel {
  component "/sys/power/*, /sys/hibernate/*\n(sysfs 历史通道：wake_lock/state/wakeup_count)" as SysFs
}

ArkTSApp --> NAPI
SysComp --> Client
CLI --> Client
NAPI --> Client
Client -> Svc : IPC (IPowerMgr.idl 生成\nproxy/stub + zidl 回调)
Svc --> Actions
Actions -> DM : SuspendBegin/WakeUpBegin/\nSetDisplayState/SetScreenPowerById
Actions -> HDI : Suspend/Wakeup/Hibernate/\nAcquireRunningLock/ForceSuspendEx
Actions -> Init : DoRebootExt(重启/关机)
Actions ..> SysFs : 历史通道代码保留\n(AutoSuspend/RunningLockHub,\n默认流程不直写)
Svc --> Dialog : 拉起关机弹窗\n(KeyMonitor/AbilityConnection)
Svc <-- MMI : 电源键/触摸/笔/盖板事件
Svc <-- Sensor : 距离/霍尔/加速度
Svc <-> Settings : SettingProvider\n读写 + 观察
@enduml
```

## 核心输入
- **IPC 接口调用**：`IPowerMgr.idl` 定义约 75 个方法（亮灭屏、休眠唤醒、运行锁、关机重启、模式、回调注册，`interfaces/inner_api/IPowerMgr.idl:33-141`），来自系统组件与 NAPI 桥；唤醒/强休走 oneway 异步（:60,68）。
- **输入事件**：MMI 按键（电源键长按 3000ms，`suspend_controller.h:266`）、触摸/触控板/笔事件（`wakeup_controller.h:120-135`）；传感器（距离、霍尔 lid、双击、拾起）。
- **配置文件**：`services/native/profile/power_wakeup.json`（各唤醒源 enable/click）、`power_suspend.json`（各休眠源 action/delayMs）、`power_mode_config.xml`（模式策略）、`power_wakeup_action.json`、`power_vibrator.json`。
- **系统参数**：`etc/para/powermgr.para`（bootevent.powermgr.ready、const.power.powerkey.down_duration=3000、enable_s4、enable_lid_check、external_screen_wakeup、powerkey_blocked_fold_mode 等）。
- **设置项**：display off time、电源模式、唤醒/休眠源 JSON、双击/拾起/翻盖开关、通话中亮屏（SettingHelper，`services/native/src/setting/setting_helper.h:34-114`）。
- **HDI 回调**：PowerHdiCallback OnSuspend/OnWakeup（V1_2）与带 tag 版本（V1_3，`services/native/include/power_hdi_callback.h:26-42`）。
- **SA 上下线**：监听 suspend manager、device_standby、display_manager（3308/4607）、MSDP、公共事件服务、HDF 设备管理（`power_mgr_service.cpp:182-193`）。
- **开机完成回调**：boot completed 后统一初始化各控制器（`power_mgr_service.cpp:247-295`）。

## 核心输出
- **显示状态变更**：经 Rosen 接口驱动真实亮灭屏（SuspendBegin/WakeUpBegin/SetDisplayState/SetScreenPowerById，`device_state_action.cpp:164,232,254,277-278`），并向 display_manager 注册 DisplayPowerCallback 接收完成回调。
- **内核电源动作**：HDI Suspend/Wakeup/Hibernate/ForceSuspendEx 与运行锁 Hold/UnholdRunningLockExt（system_suspend_controller.cpp:253-266）、`DoRebootExt` 关机重启；`/sys/power/*`、`/sys/hibernate/resume` 由 init 预授权（etc/init/powermgr.cfg），sysfs 通道代码（AutoSuspend/RunningLockHub）保留但默认流程不调用。
- **公共事件**：亮屏/灭屏/强休进出/S4 进出事件（PowerMgrNotify，`services/native/include/power_mgr_notify.h:31-41`），供全系统业务感知。
- **HiSysEvent**：domain POWER（RUNNINGLOCK/SCREEN/STATE 统计，SCREEN_ON_TIMEOUT/SCREEN_OFF_TIMEOUT/STATE_CORRECTION/SCREEN_STATE_TRANSIT_FAILED 故障，`powermanager.yaml`）与 POWER_UE（亮灭屏行为），供功率统计与质量分析。
- **回调通知**：13 类 IPC 回调（状态、同步睡眠/休眠、模式、灭屏前、关机三族、运行锁、ULSR、异步回包 IPowerMgrAsync）。
- **诊断输出**：hidumper（PowerMgrDumper）、power-shell CLI（`tools/ohos-powerManager/`）、`bootevent.powermgr.ready=true` 启动就绪信号。
