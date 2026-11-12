# 术语表

## 电源状态（PowerState）
整机电源状态的十态枚举：AWAKE=0、FREEZE、INACTIVE、STAND_BY、DOZE、SLEEP、HIBERNATE、SHUTDOWN、DIM、UNKNOWN（`interfaces/inner_api/native/include/power_state_machine_info.h:56-106`）。AWAKE 为亮屏活动态，DIM 为变暗过渡态，INACTIVE 为灭屏态，SLEEP 为内核休眠态。与 display_manager 的 DisplayState（屏幕物理状态）区分：本枚举是业务决策层状态，屏幕执行由 display_manager 完成。没有名为 ACTIVE 的状态（亮屏态是 AWAKE）。

## 状态变更原因（StateChangeReason）
状态迁移的业务归因枚举（60+ 值，`power_state_machine_info.h:211-269`），如 STATE_CHANGE_REASON_TIMEOUT、REFRESH、POWERKEY、APPLICATION、PRE_BRIGHT_AUTH_* 族。与 WakeupDeviceType/SuspendDeviceType 配对出现，是 allowMapByReason 按原因放行转移的键（`power_state_machine.cpp:163-199`）。

## 运行锁（RunningLock）
阻止系统进入休眠的持有型锁对象。客户端经 `RunningLock` 类 Lock/UnLock（`interfaces/inner_api/native/include/running_lock.h:51-57`），服务端由 `RunningLockMgr` 以 IRemoteObject token 为键管理。与内核 wakelock 的区别：RunningLock 是业务概念（带名字、类型、超时），生效时经 HDI Hold/UnholdRunningLockExt 下沉为内核锁（system_suspend_controller.cpp:253-266）。

## 运行锁类型（RunningLockType）
位掩码枚举（`running_lock_info.h:32-84`）：SCREEN=0、BACKGROUND=1、PROXIMITY_SCREEN_CONTROL=2、COORDINATION=4；7 种后台子类型为 BACKGROUND|1<<n（PHONE=3、NOTIFICATION=5、AUDIO=9、SPORT=17、NAVIGATION=33、TASK=65、USER_IDLE=129）。子类型影响锁代理策略与前台校验，不改变"阻止休眠"的基本语义。

## 锁计数器（LockCounter）
`RunningLockMgr` 内部按锁型维护的计数器（`services/native/src/runninglock/running_lock_mgr.h:121-144`），Increase/Decrease 时触发 activate 回调（转发到 RunningLockAction→HDI），计数归零即解锁。是同型多锁的仲裁核心。

## 唤醒源/休眠源（WakeupSource / SuspendSource）
JSON 配置声明的事件源（`services/native/profile/power_wakeup.json`：powerkey/keyborad[代码原文拼写]/mouse/touchscreen(click)/touchpad/pen/lid/switch/pickup/tp_touch；`power_suspend.json`：powerkey/timeout/lid/switch/tp_cover 各带 action/delayMs）。运行期由 WakeupController/SuspendController 解析生成 Monitor 对象（`wakeup_sources.h:60-72`），可被设置项在线覆盖。

## 电源键行为（powerkey）
电源键交互由 SuspendController 侧 PowerKeySuspendMonitor 与 WakeupController 侧 PowerkeyWakeupMonitor 处理：长按 3000ms（`suspend_controller.h:266`）触发关机弹窗路径，按压 10000ms（POWER_KEY_PRESS_DELAY_MS）为强制阈值；`const.power.powerkey.down_duration=3000` 可调（`etc/para/powermgr.para`）。`power_manager_feature_allow_interrupting_powerkey_off` 允许打断按键关机。

## 内核休眠协议（wakeup_count）
`Suspend::SuspendController::AutoSuspend`（actions/default/suspend/suspend_controller.h:58-63）实现的 Linux 标准休眠协议：循环读取 `/sys/power/wakeup_count` 并原样写回，写回成功说明期间无新唤醒事件，才向 `/sys/power/state` 写 "mem" 进入 S3；失败 100ms 后重试。注意：当前默认生产路径为 HDI 通道（StartSuspend/ForceSuspend），该 sysfs 实现保留为历史/测试通道（仅单测引用）。

## S3 / S4
S3 为内存睡眠（suspend to RAM，state="mem"）；S4 为磁盘休眠（hibernate，`hibernate_controller.h`，内核接口 `/sys/hibernate/resume`，`etc/init/powermgr.cfg` 授权）。S4 由 `power_manager_feature_enable_s4`（默认 false）+ `const.power.enable_s4`（默认 true）双重控制，`DoRebootExt` 的 SHUTDOWN_FAST_REASON="STR"（`power_state_machine_info.h:51`）与快速关机相关。

## ULSR
Ultra-Low Suspend Resume 低时延挂起框架：经 `SetSuspendTag("ulsr")` + ForceSuspendDeviceIgnoringWakelock 进入（`power_mgr_client.h:160-165` 注释：tag 仅支持 "mem"/"ulsr"），回调族 IUlsrCallback（OnSyncUlsr/OnAsyncWakeup）。由 `power_manager_feature_enable_ulsr_plugin`/`enable_suspend_with_tag` 控制（默认关闭），插件与宿主接口有版本耦合（AGENTS.md 陷阱）。

## 接管（Takeover）
允许上层拦截系统决策的回调机制：TakeOverShutdownCallback 拦截关机/重启（`services/zidl/include/shutdown/`），TakeOverSuspendCallback 拦截挂起。返回 true 表示接管方自行处理，服务端中止原流程（`shutdown_controller.h:58-59`）。回调内禁止再触发关机/挂起决策（死锁风险）。

## 关机回调三族（Takeover/Async/Sync Shutdown）
ShutdownController 按 ShutdownPriority 分桶持有三类回调（`shutdown_controller.h:83-85` 三个 ShutdownCallbackHolder）：takeover 先行决定是否放行，async 异步通知不等待，sync 同步等待完成后再执行 DoRebootExt。注册入口分别为 RegisterShutdownCallback/AsyncShutdownCallback（`power_mgr_client.h:352-360`）。

## 同步睡眠回调（SyncSleepCallback）
SuspendController 在进入/退出内核睡眠前后同步触发的回调（`TriggerSyncSleepCallback(isWakeup)`，`suspend_controller.h:83`），按 SleepPriority 排序，供业务在真正断电前落盘/关外设。与 TakeOverSuspendCallback（拦截决策）区分。

## 灭屏前回调（ScreenOffPre）
`IScreenOffPreCallback`，在超时灭屏前 N 毫秒触发（`RegisterScreenStateCallback(remainTime, callback)`，`power_mgr_client.h:297`），由 ScreenOffPreController 以"EyeDetect"定时器调度（`screenoffpre/screen_off_pre_controller.h:35,60`）。与 takeover 的区别：不可拦截，只是提前通知。

## 预亮流程（pre-bright）
亮屏前预授权场景：WAKEUP_DEVICE_PRE_BRIGHT 族唤醒类型先亮屏后等授权结果（`power_state_machine.cpp:597-690`），授权成功/失败分别走 TriggerSyncSleepCallback(true) 或 StartSleepTimer 回睡眠；配 CHECK_PRE_BRIGHT_AUTH_TIMEOUT_MSG 定时器兜底。常见于人脸/支付亮屏。

## 电源模式（PowerMode）
五档模式枚举（`power_mode_info.h:24-32`）：NORMAL_MODE=600、POWER_SAVE_MODE、PERFORMANCE_MODE、EXTREME_POWER_SAVE_MODE、CUSTOM_POWER_SAVE_MODE=650。模式切换经 PowerModeModule 按策略下发/恢复设置项并广播 POWER_MODE 回调与公共事件。`power_manager_feature_enable_performance_mode` 默认关闭性能档。

## 模式策略（PowerSaveMode / ModePolicy）
`power_mode_config.xml` 解析出的策略缓存：`policyCache_[mode] = list<ModePolicy{id,value,recover_flag}>`（`services/native/include/power_save_mode.h:33-54`），recover_flag 决定切回 NORMAL 时是否恢复原值。PowerModeModule 的 RunAction 按此对 display off time、亮度、振动、旋转等逐项下发。

## 距离传感器控制（ProximityController）
通话场景的灭屏控制：RunningLockMgr 内嵌 ProximityController 继承 ProximityControllerBase（`running_lock_mgr.h:146-156`），接近（OnClose）延迟灭屏、远离（OnAway）亮屏；`power_manager_feature_proximity_controller_override` 允许产品覆盖判定，`support_filtering_proximity_event` 提供事件过滤策略（SetProxFilteringStrategy）。

## 设置提供者（SettingProvider / SettingHelper）
SettingProvider 是 utils/setting 的 DataShare 封装（跨仓共享，`utils/setting/include/setting_provider.h:25-43`），提供 Get/Put/Observer；SettingHelper 是本仓对全部电源相关键的静态门面（display off time、唤醒源 JSON、模式、双击/拾起/翻盖、通话中亮屏、AOD，`services/native/src/setting/setting_helper.h:34-114`）。充电形态下 display off time/sleep time 分 AC/DC 两套键。

## 系统休眠控制器（SystemSuspendController）
HDI Power 的唯一服务端门面（DelayedRefSingleton，`actions/default/system_suspend_controller.h:38-58`）：Suspend/Wakeup/Hibernate、Acquire/ReleaseRunningLock、SetSuspendTag、ForceSuspendEx（V1_4）、GetWakeupReason、Set/GetPowerConfig；内部 WAKEUP_HOLDER "OHOSPowerMgr.WakeupHolder" 为唤醒保持锁名。

## FFRT
Function Flow Runtime 协作式调度框架（utils/ffrt 封装 FFRTTimer/FFRTQueue/FFRTMutexMap）。本服务全部定时逻辑（活动超时、DIM、灭屏前回调、ULSR 超时）跑在 FFRT 定时器上（`power_state_machine.cpp:1424-1473`），SetState 用任务 id 作废过期定时任务。任务必须显式 wait 或 detach（AGENTS.md 陷阱）。

## 插件与钩子（ModuleMgrScan / HookMgr）
服务启动时扫描 `/system/lib[64]/powerplugin/autorun` 加载插件（`power_mgr_service.cpp:90-92,176`）；关机/重启前经 PowerHookMgr 执行 POWER_PRE_DO_REBOOT/POWER_PRE_DO_SHUTDOWN 钩子（`device_power_action.cpp:40-41,56`），钩子可改写重启命令（如 updater）。是产品定制关机充电等行为的标准扩展点。

## 关机弹窗（power_dialog）
独立 ArkTS 应用（`power_dialog/`，含签名与 hvigor 工程）：长按电源键时由 ShutdownDialog 拉起（AbilityConnection + KeyMonitor，`shutdown_dialog.h:31-39`），用户选择后回包驱动真实关机/重启；服务端持超时兜底。禁止将弹窗等待改为同步阻塞（AGENTS.md 陷阱）。

## 多 SA 共进程（powermgr 进程）
SA 3301（power_manager）、3302（battery）、3303（thermal）、3308（display）共用 powermgr 进程（AGENTS.md 术语表）。init 配置 uid powermgr、apl system_core、critical [0,4,60]（`etc/init/powermgr.cfg`）。任何 SA 的阻塞/崩溃影响整个电源子系统。

## 启动就绪参数（bootevent.powermgr.ready）
OnStart Publish 成功后置 true 的系统参数（`power_mgr_service.cpp:204`），初值 false（`etc/para/powermgr.para`）。配套 `persist.powermgr.stopservice` 用于停止服务调试。启动顺序：OnStart→Init（状态机/运行锁/关机控制器）→Publish→boot completed 回调再初始化输入/唤醒/休眠/模式等重依赖（`power_mgr_service.cpp:247-295`）。
