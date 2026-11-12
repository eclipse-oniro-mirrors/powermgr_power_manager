# 编码铁律

## IPC 接口演进
- **禁止**修改 `IPowerMgr.idl` 中已发布方法的位置、签名或参数顺序（码值由声明顺序生成，`interfaces/inner_api/IPowerMgr.idl:34` 起 ipccode 0 连续），新增接口只能追加到文件尾部，回调新增须同步在 `services/zidl/` 成对手写 proxy/stub 并带死亡清理。旧客户端按码值寻址，错位即调用到错误接口。
- **禁止**绕过 `PowerMgrServiceAdapter` 直接在业务方法里解码 IPC 参数；生成 stub 的 `*Ipc` 方法统一在 `services/native/include/power_mgr_service_ipc_adapter.h:27-36` 转发到 `PowerMgrService` 实现，两处签名必须一致。

## 状态机与并发
- **禁止**任何业务直接读写 `PowerStateMachine` 的 `currentState_`/`screenState_` 字段；唯一合法路径是 `SetState`（`services/native/src/power_state_machine.cpp:2241`，内部持 `stateMutex_`），头文件注释明确"currentState_ 变更须在 stateMutex_ 内"（`services/native/include/power_state_machine.h:493-495`）。
- **禁止**在服务端混用 `std::mutex` 与 `ffrt::mutex` 保护同一状态；服务核心的定时器/队列/锁统一使用 FFRT 家族（FFRTTimer、FFRTQueue、ffrt::mutex，`power_mgr_service.cpp:211-213`），SetState 依赖 `ffrt::this_task::get_id` 识别并作废过期定时任务（`power_state_machine.cpp:2247,2250`）。

## HDI 与内核通道
- **禁止**业务类直接 include HDI 头或打开 `/sys/power/*`；HDI 只经 `SystemSuspendController`（V1_3 门面，`services/native/src/actions/default/system_suspend_controller.h:38`），仓内仅有的 sysfs 直写代码是 `RunningLockHub`（wake_lock/wake_unlock，`actions/default/suspend/running_lock_hub.h:45-46`）与 `Suspend::SuspendController::AutoSuspend`（state/wakeup_count，`actions/default/suspend/suspend_controller.h:61-63`）——两者为历史通道，默认流程不调用，新增功能禁止再走 sysfs。
- **禁止**在 HDI 句柄失效后继续缓存调用；必须经 `RegisterHdiStatusListener` + `ResetHandle` 走重连路径（`system_suspend_controller.h:47,58`），sysfs fd 用 `UniqueFd` 托管（`running_lock_hub.h:48-49`）。

## 特性开关与条件编译
- **禁止**只改 `powermgr.gni` 的 `declare_args()` 默认值而不同步 defines 映射与 `bundle.json` features（45 个开关，`powermgr.gni:16-116`；AGENTS.md 列为项目特定陷阱"defines 双写"）。产品扩展逻辑一律用 `#ifdef POWER_MANAGER_*` 包裹（如 enable_s4、enable_ulsr_plugin、enable_force_suspend_ignoring_wakelock）。

## 日志与 DFX
- **禁止**无域日志：统一 `POWER_HILOG*(COMP_SVC/COMP_FWK/FEATURE_*)`，域枚举与标签映射在 `utils/native/include/power_log.h:79-124`（如 FEATURE_SUSPEND→"PowerSuspend"）；亮灭屏/状态迁移埋点用 HiSysEvent（domain POWER/POWER_UE，`powermanager.yaml`、`powermanager_POWER_UE.yaml`），关键路径加 Hitrace（HITRACE_TAG_POWER，`power_state_machine.cpp:549-551`）。禁止为通过测试删除事件或日志（AGENTS.md 禁止事项）。

## 错误处理
- **禁止**吞错误返回；对外方法返回 `PowerErrors`（`interfaces/inner_api/native/include/power_errors.h`），状态迁移失败必须经 `TransitResult` 枚举上报（`power_state_machine.h:63-74`）并写 `SCREEN_STATE_TRANSIT_FAILED` FAULT 事件（`power_state_machine.cpp:2298-2302`）。
- **禁止**在关机/休眠路径分配大内存或做阻塞 IO（AGENTS.md 禁止事项）；该路径在低内存/低 IO 能力下执行。

## 回调生命周期
- **禁止**注册 IPC 回调不带死亡通知；所有回调容器以 `sptr<IRemoteObject>` 为键并实现 `OnRemoteDied` 清理（`running_lock_mgr.h:158-163`、`screenoffpre/screen_off_pre_controller.h:41-51`、`hibernate/hibernate_controller.h:47-54`）。takeover 回调内**禁止**再触发关机/挂起决策，防止死锁（AGENTS.md 陷阱）。
