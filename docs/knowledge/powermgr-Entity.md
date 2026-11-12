# 业务实体知识

power_manager 模块核心业务实体的完整知识定义，涵盖静态概念、动态运行、规则约束与代码映射。总览请见 [架构设计](architecture.md)，术语定义见 [术语表](glossary.md)，业务背景见 [业务背景](business-context.md)。

## 电源状态数据模型（PowerState / StateChangeReason）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 电源状态数据模型（power_state_machine_info.h 枚举族） |
| 实体定义 | 跨进程共享的电源语义契约：状态集、事件类型与归因原因，是客户端、服务端、display_manager 三方对齐的词汇表 |
| 核心特征 | PowerState 十态（:56-106）；DisplayState 七值（:111-119）；UserActivityType 九类（:122-133）；WakeupDeviceType 36 类（:136-175）；SuspendDeviceType 18 类（:178-199）；StateChangeReason 60+ 值（:211-269）；辅助常量 REFRESH_ACTIVITY_NO_CHANGE_LIGHTS（:29）、SUSPEND_DEVICE_IMMEDIATELY（:35）、MIN_TIME_MS_BETWEEN_USERACTIVITIES=100ms（:45）、SHUTDOWN_FAST_REASON="STR"（:51） |
| 类型/分类 | 全部为 enum class / constexpr，随 `libpowermgr_client` 发布（interfaces/inner_api/native/include/power_state_machine_info.h） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | 客户端 API 以 SuspendDeviceType/WakeupDeviceType/UserActivityType 入参发起请求；状态机以 (PowerState, StateChangeReason) 对驱动 SetState；对外经 IPowerStateCallback 与公共事件广播状态 |
| 状态流转 | 本身无状态；组合语义上构成 AWAKE→DIM→INACTIVE→SLEEP 的超时链与唤醒回退 AWAKE，SHUTDOWN/HIBERNATE 为终态/特殊态 |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | RefreshActivity 两次调用至少间隔 100ms（CheckRefreshTime 强制，`power_state_machine.cpp:802-810`）；SUSPEND_DEVICE_IMMEDIATELY 与 NEED_DOZE 决定灭屏是否带过渡；UNKNOWN 用于异常兜底 |
| 并发约束 | 纯值类型无并发问题；作为 IPC 参数时按值序列化 |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | 全部客户端 API（power_mgr_client.h）、NAPI 枚举导出（power_module.cpp CreateDevicePowerMode） |
| 下游影响 | PowerStateMachine 三张规则表的键；display_manager 按 StateChangeReason 映射 Rosen reason（PowerUtils::GetDmsReasonByPowerReason） |
| 平级关联 | running_lock_info.h 的 RunningLockType；power_mode_info.h 的 PowerMode |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | PowerState/DisplayState/UserActivityType/WakeupDeviceType/SuspendDeviceType/StateChangeReason（interfaces/inner_api/native/include/power_state_machine_info.h） |
| 核心方法 | 无行为方法；PowerUtils 提供枚举转字符串与 reason 映射（power_state_machine.cpp 内使用） |

## IPC 接口层（IPowerMgr 与回调契约）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 电源服务 IPC 接口层 |
| 实体定义 | 客户端与服务端之间的调用契约集合：请求方向由 IDL 生成，回调方向由 zidl 手写 |
| 核心特征 | `IPowerMgr.idl` 141 行：sequenceable IRemoteObject/RunningLockInfo/VectorPair（:16-18）；声明 13 个回调接口（:19-31）；接口体约 75 个方法，首个方法 CreateRunningLockIpc 带 `[ipccode 0, customMsgOption flags=MessageOption::TF_IMAGE]`（:34）；oneway 方法：UpdateWorkSourceIpc/WakeupDeviceAsyncIpc/ForceSuspendDeviceIpc/HibernateIpc（:39,60,68,89） |
| 类型/分类 | 生成物（idl_gen_interface("powermgr_interface")，interfaces/inner_api/BUILD.gn:44；powermgr_proxy :53 / powermgr_stub :82）+ 手写（services/zidl/ 50 文件） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | 客户端 `GetPowerMgrProxy()` 持生成代理；服务端 `PowerMgrServiceAdapter : PowerMgrStub` 解码后转发 PowerMgrService（power_mgr_service_ipc_adapter.h:27-36）；异步强休/休眠经 IPowerMgrAsync 通道回包（zidl power_mgr_async_reply 的 SendAsyncReply(int reply)） |
| 状态流转 | 接口层无状态；代理对象随 SA 死亡失效，客户端经 PowerMgrDeathRecipient 重建（power_mgr_client.h:413-426） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 已发布方法的声明顺序即 IPC 码值，只增不改不插；oneway 仅用于无需回包的唤醒/强休路径；回调注册方法统一携带优先级参数（SleepPriority/ShutdownPriority/UlsrPriority 等） |
| 并发约束 | 回调触发在 FFRT/IPC 线程混合上下文，各 Holder 用 ffrt::mutex 保护（如 sleep_callback_holder.h） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | PowerMgrClient/RunningLock/ShutdownClient（frameworks/native）与 NAPI 层 |
| 下游影响 | PowerMgrService 全部业务方法；13 个 CallbackHolder/Controller 的注册入口 |
| 平级关联 | `libpowermgr_client.map` 版本脚本约束导出符号；callback 头位于 interfaces/inner_api/native/include/{shutdown,suspend,hibernate,ulsr}/ 与 services/zidl/include/ |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | IPowerMgr（生成）、IPowerMgrProxy/IPowerMgrStub（生成）、power_state_callback_proxy/stub 等 13 族（services/zidl/src/、include/） |
| 核心方法 | CreateRunningLockIpc（.idl:34）、SuspendDeviceIpc（:56）、WakeupDeviceIpc（:58）、ForceSuspendDeviceIgnoringWakelockIpc（:139）、InterruptForceSuspendIpc（:140） |

## 电源管理服务（PowerMgrService）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | PowerMgrService（SA 3301 门面） |
| 实体定义 | 电源管理唯一系统服务实例：持有状态机与全部控制器，对外实现 IPowerMgr 全部业务方法，对内编排启动时序与外部依赖 |
| 核心特征 | `final : SystemAbility(POWER_MANAGER_SERVICE_ID, true), PowerMgrServiceAdapter` + DelayedSpSingleton（power_mgr_service.cpp:164；power_mgr_service.h:58,68-70）；run-on-create: true（sa_profile/3301.json）；头文件 537 行声明约 75 个业务方法 + 控制器生命周期方法（power_mgr_service.h:85-225） |
| 类型/分类 | SystemAbility 派生；进程内单例 |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | OnStart：插件扫描 ModuleMgrScan（:176）→ Init（FFRTTimer/RunningLockMgr/ShutdownController/状态机/ScreenOffPreController，:208-245）→ 监听 7 个外部 SA（:182-193）→ SystemSuspendController::RegisterHdiStatusListener + PowerExtIntfWrapper::Init（:195-196）→ Publish → 置 bootevent.powermgr.ready=true（:199-204）；boot completed 回调再初始化设置观察者、输入监测、Suspend/Wakeup 控制器、模式、弹窗配置、保亮（:247-295） |
| 状态流转 | ready_ → isBootCompleted_ 两级就绪；OnStop/Reset 清理；OnAddSystemAbility/OnRemoveSystemAbility 处理 display_manager、device_standby 等上下线（:930-980） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 写类方法（关机/重启/灭屏/设模式）须服务端权限校验（AGENTS.md 架构不变量）；isBootCompleted_ 前后区分初始化批次；系统参数 const.power.* 在 Init 读取（during_call/lid_check/s4/external_screen_wakeup/powerkey_blocked_fold_mode，:231-241） |
| 并发约束 | 与 battery/thermal/display 共进程，禁止长阻塞；Dump 支持 hidumper（:89） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | PowerMgrServiceAdapter（生成 stub 的唯一业务入口） |
| 下游影响 | 状态机、RunningLockMgr、ShutdownController、SuspendController、WakeupController、HibernateController、PowerModeModule、ScreenOffPreController、PowerMgrNotify、ShutdownDialog、SettingHelper |
| 平级关联 | 监听 SUSPEND_MANAGER/DEVICE_STANDBY/DISPLAY_MANAGER(3308/4607)/MSDP/COMMON_EVENT/DEVICE_SERVICE_MANAGER 六类 SA（power_mgr_service.cpp:182-193） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | PowerMgrService（services/native/include/power_mgr_service.h） |
| 核心方法 | OnStart（.cpp:168）、Init（:208）、RegisterBootCompletedCallback（:247）、SuspendDevice/WakeupDevice（.h:95-98）、SetDeviceMode/GetDeviceMode（:158-159）、ShellDump（:160） |

## 客户端 SDK（PowerMgrClient）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | PowerMgrClient（客户端门面） |
| 实体定义 | 面向系统组件的进程内单例：封装代理获取、死亡重连与全部电源业务的转发；自身不持业务状态 |
| 核心特征 | 单例 + PowerMgrDeathRecipient（power_mgr_client.h:33-38,413-427）；CONNECT_RETRY_COUNT=5 / CONNECT_RETRY_MS=800000（:37-38）；静态 runningLocks_ 弱引用列表支撑服务重启后 RecoverRunningLocks（:430-431） |
| 类型/分类 | final 类，随 libpowermgr_client 发布 |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | 应用/系统组件调 Reboot/ShutDown/SuspendDevice/WakeupDevice/RefreshActivity/OverrideScreenOffTime/CreateRunningLock/回调注册等（:44-408）→ Connect 获取代理 → oneway/同步转发；SA 死亡触发 ResetProxy，下次调用自动重连 |
| 状态流转 | 无业务状态；仅 error_ 缓存最近错误（:433） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | Lock/UnLock 语义：后一次 Lock 的超时参数覆盖前次（running_lock.h:42-51）；LockScreenAfterTimingOut 多客户端以"任一非默认值生效"仲裁（:241-263） |
| 并发约束 | mutex_ 保护代理指针；RunningLock 客户端对象各自持 mutex（running_lock.h:65） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | NAPI（power_module.cpp/runninglock_module.cpp）、ShutdownClient（frameworks/native/shutdown/shutdown_client.cpp） |
| 下游影响 | IPowerMgrProxy 全部方法 |
| 平级关联 | RunningLock（客户端持有类）与 PowerMgrClient 通过 proxy_ 弱引用协作（running_lock.h:32） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | PowerMgrClient、RunningLock（interfaces/inner_api/native/include/power_mgr_client.h、running_lock.h） |
| 核心方法 | Connect/GetPowerMgrProxy/ResetProxy（:424-426）、CreateRunningLock（:276-279）、RecoverRunningLocks（:296） |

## 电源状态机（PowerStateMachine）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | PowerStateMachine |
| 实体定义 | 整机电源状态的唯一持有者与仲裁者：以规则表约束状态转移，经 StateController 执行进入动作，驱动亮灭屏/休眠时序 |
| 核心特征 | 每态一个 StateController（TransitTo/RecordFailure，power_state_machine.h:369-400，controllerMap_ :431-439）；TransitResult 十种结果（:63-74）；DevicePowerState 记录 lastOnTime/lastWakeupEventTime/lastRefreshActivityTime 等（:51-61）；常量组 DEFAULT_DISPLAY_OFF_TIME_MS=30000（充电形态 AC 600000/DC 300000）、DEFAULT_SLEEP_TIME_MS=5000、MAX_DIM_TIME_MS=7500（:107-119） |
| 类型/分类 | 服务端核心组件，enable_shared_from_this（:85） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | SuspendDeviceInner（:546）/WakeupDeviceInner（:710）/RefreshActivityInner（:772）预处理后调 SetState（:2241）：stateMutex_ 上锁 → CheckFFRTTaskAvailability 作废过期定时任务 → ScreenChangeCheck/SettingStateFlag RAII → forceTimingOut_ 升级为强转 → StateController::TransitTo → WriteHiSysEvent；超时链由 FFRT 定时器驱动（SetDelayTimer :1424-1473，事件含活动超时/灭屏后休眠/距离灭屏/ULSR 超时） |
| 状态流转 | 规则表三张：forbidMap_ 禁止表（AWAKE 禁 SLEEP、INACTIVE 禁 DIM、SLEEP 禁 DIM 等，:138-161）；allowMapByReason 按原因放行（TIMEOUT: AWAKE→{DIM,INACTIVE}、DIM→INACTIVE、INACTIVE→SLEEP 等，:163-199）；lockMap_ 运行锁阻塞表（INACTIVE 阻塞于 RUNNINGLOCK_SCREEN、SLEEP 阻塞于 RUNNINGLOCK_COORDINATION，:81-116） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | SetState 返回 SUCCESS 或 ALREADY_IN_STATE 视为成功（:2288）；双击/拾起/插拔唤醒受接近/翻盖/运动状态门控（CanTransitTo :230）；DIM 进入可被活动打断（SettingStateFlag SETTING_DIM_INTERRUPTED，:1532-1538）；RefreshActivity 屏灭时忽略（:797-799） |
| 并发约束 | currentState_ 变更仅 stateMutex_ 内（:493-495）；屏幕变更上报节流 SCREEN_CHANGE_REPORT_INTERVAL_MS=600000（:107-119） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | PowerMgrService 持有并在 boot 后 InitState（power_mgr_service.cpp:273） |
| 下游影响 | DeviceStateAction（亮灭屏）、SuspendController（自动休眠）、WakeupController（监测注册）、PowerMgrNotify（公共事件）、HiSysEvent |
| 平级关联 | SettingHelper（超时来源）、RunningLockMgr（锁门控）、ProximityController（距离灭屏定时） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | PowerStateMachine、StateController、ScreenChangeCheck、SettingStateFlag（services/native/include/power_state_machine.h） |
| 核心方法 | SetState（.cpp:2241）、SetDelayTimer/CancelDelayTimer（:1424/:1484）、ResetInactiveTimer（:1532）、HandleActivityTimeout（:1673）、CheckRunningLock（:1815）、WriteHiSysEvent（:2291） |

## 运行锁管理器（RunningLockMgr）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | RunningLockMgr |
| 实体定义 | 运行锁的服务端仲裁者：按 token 管理锁对象、按锁型计数、处理代理/恢复/强制释放，并把生效动作下发给 HDI |
| 核心特征 | RunningLockMap = map<sptr<IRemoteObject>, shared_ptr<RunningLockInner>>（running_lock_mgr.h:45）；LockCounter 计数器族（:121-144）；每型 InitLocksType* 初始化（:109-115）；CHECK_TIMEOUT_INTERVAL_MS=60s 巡检（:92）；RunningLockDeathRecipient 死亡释放（:158-163） |
| 类型/分类 | 服务端组件，持 pms_ 弱引用 |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | CreateRunningLock → Lock/UnLock（超时则由 running_lock_timer_handler 定时释放）→ Release；BGTASK 场景 ProxyRunningLocks(pid,uid 列表) 批量代理；音频场景 IsExistAudioStream 特判（:99）；强休时 ForceSleepReleaseLock（:100）；RUNNINGLOCK_BACKGROUND_USER_IDLE 需监听强制休眠公共事件自行释放（running_lock_info.h:73-79） |
| 状态流转 | 锁对象四态：DISABLE→ENABLE→（PROXIED↔UNPROXIED_RESTORE）→DISABLE（running_lock_inner.h:27-32）；代理由系统调优触发，恢复时按名单重建 |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | INACTIVE 转移被 RUNNINGLOCK_SCREEN 阻塞、SLEEP 被 RUNNINGLOCK_COORDINATION 阻塞（power_state_machine.cpp:81-116）；代理锁不计入休眠阻塞；每次持/放锁写 RUNNINGLOCK HiSysEvent（:59） |
| 并发约束 | mutex_/screenLockListsMutex_ 双锁（:179-180）；后台场景锁列表单独维护 unSceneLockLists_（:190） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | PowerMgrService::CreateRunningLock/Lock/UnLock/ProxyRunningLock* |
| 下游影响 | RunningLockAction→SystemSuspendController::AcquireRunningLock（HDI）；状态机 CheckRunningLock |
| 平级关联 | RunningLockProxy（BGTASK 名单）、RunningLockCallbackManager（变更回调）、ProximityController（内嵌，:146-156） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | RunningLockMgr、LockCounter、RunningLockInner、RunningLockDeathRecipient（services/native/src/runninglock/running_lock_mgr.h、running_lock_inner.h） |
| 核心方法 | CreateRunningLock（:52）、Lock/UnLock（:57-58）、ProxyRunningLocks（:87）、ResetRunningLocks（:90）、ExistValidRunningLock（:77）、WriteHiSysEvent（:59） |

## 休眠控制器（SuspendController）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | SuspendController（业务休眠编排） |
| 实体定义 | 休眠事件源与动作的编排者：解析休眠源配置、注册输入监测、按 reason+action 决定灭屏/自动休眠/强休/休眠/关机 |
| 核心特征 | ctor 依赖 ShutdownController+状态机+FFRTTimer（suspend_controller.h:55-56）；SuspendMonitor 族：PowerKey（长按 3000ms 弹窗、POWER_KEY_PRESS_DELAY_MS=10000，:254-269）、Timeout、Lid、Switch、TPCover（:271-303）；FORCE_SLEEP_DELAY_MS=8000（:162）；强制休眠忽略锁超时 30000ms（:164）；TakeOverSuspendCallback 桶（:77-81） |
| 类型/分类 | 服务端控制器，enable_shared_from_this |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | 配置源 Init（power_suspend.json）→ Monitor 触发 ControlListener(reason,action,delay) → HandleAction 分派：ACTION_AUTO_SUSPEND→SuspendWhenScreenOff（状态机灭屏后 StartSleepTimer）、ACTION_FORCE_SUSPEND→HandleForceSleep、ACTION_HIBERNATE→HandleHibernate、ACTION_SHUTDOWN→HandleShutdown；HandleEvent(delayTime) 承接状态机的灭屏后休眠定时（power_state_machine.cpp:1439-1447） |
| 状态流转 | sleepReason_/sleepAction_/sleepDuration_ 记录最近一次休眠参数；powerkeyDownWhenScreenOff_ 记录灭屏态按键（:173-178）；强休经 condition_variable 等待回调完成并可 InterruptForceSuspendCallback 打断（:68-71,182-189） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 通话中亮屏场景 CheckDuringCall 拦截自动休眠（:168）；充电形态 AC/DC 各自 sleep time（:130-133）；Takeover 回调放行才继续；SyncSleepCallback 在睡/醒两侧触发（TriggerSyncSleepCallback(isWakeup) :83） |
| 并发约束 | mutex_/sleepCbMutex_/suspendMutex_/ffrtMutexMap_ 多锁分区（:179-181,191）；外设关断回调在独立 FFRT 队列 |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | PowerMgrService::SuspendControllerInit（power_mgr_service.cpp:281） |
| 下游影响 | 状态机 SuspendDeviceInner、ShutdownController（HandleShutdown）、HibernateController |
| 平级关联 | WakeupController（互斥守卫）、SettingHelper（休眠源/AC-DC 超时）、SleepCallbackHolder |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | SuspendController、SuspendMonitor 及五个子类（services/native/src/suspend/suspend_controller.h） |
| 核心方法 | Init、HandleAction（:67）、HandleEvent（:65）、StartSleepTimer（:103）、TriggerSyncSleepCallback（:83）、UpdateSuspendSources（:84） |

## 唤醒控制器（WakeupController）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | WakeupController |
| 实体定义 | 唤醒事件源的注册与分发者：按 power_wakeup.json 生成各源 Monitor，事件到达后经状态机亮屏并维护设置开关 |
| 核心特征 | WakeupMonitor 族 11 种：Powerkey/Keyboard/Mousekey/Touchpad/Pen/SingleClick/DoubleClick/Lid/Switch/Pickup/TPTouch（wakeup_controller.h:176-266）；InputCallback : IInputEventConsumer 处理按键/触摸/轴事件（:120-135）；SleepGuard RAII 防窗口漏锁（:83-95）；静态配置改写接口（SetWakeupDoubleClickSensor/ChangeWakeupSourceConfig 等 :59-66） |
| 类型/分类 | 服务端控制器，enable_shared_from_this |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | Init 解析 JSON → RegisterMonitor 按需订阅（MMI 按键、传感器双击/拾起、盖板 switch）→ 事件到达 Notify→HandleWakeup→stateMachine_->WakeupDeviceInner；亮屏完成回调 NotifyDisplayActionDone（:53）联动 DeviceStateAction 的 DisplayPowerCallback |
| 状态流转 | monitorMap_ 按状态注册/注销（状态机 SetState 前后调用 RegisterMonitor，power_state_machine.cpp:2283,2287）；eventHandleMap_ 记录各源最近事件时间用于 CheckEventReciveTime 去抖（:75,108） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 触摸唤醒区分单击/双击（WAKEUP_DEVICE_DOUBLE_CLICK 需 click=2 配置）；双击/拾起开关来自设置项并支持 MSM（PickupConnectMotionConfig :64）；翻盖 lid 唤醒受 const.power.enable_lid_check 与 lid_type_for_only_external_screen 影响 |
| 并发约束 | mutex_/monitorMutex_/eventHandleMutex_/mmiMonitorMutex_ 分区（:111-115）；传感器回调线程安全由 ProximityControllerBase 保证 |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | PowerMgrService::WakeupControllerInit（power_mgr_service.cpp:282） |
| 下游影响 | PowerStateMachine::WakeupDeviceInner；设置项（唤醒源 JSON 回写） |
| 平级关联 | SuspendController（StopSleep 互斥）、WakeupActionController（POWER_MANAGER_WAKEUP_ACTION，wakeup_action 配置匹配）、外部屏管理（PowerOnInternalScreen/PowerOnAllScreens :76-79） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | WakeupController、WakeupMonitor 及 11 子类、InputCallback、SleepGuard（services/native/src/wakeup/wakeup_controller.h） |
| 核心方法 | RegisterMonitor（:46）、Wakeup（:52）、NotifyDisplayActionDone（:53）、ExecWakeupMonitorByReason（:51）、CheckEventReciveTime（:75） |

## 关机重启控制（ShutdownController 与 ShutdownDialog）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 关机重启控制（ShutdownController / ShutdownDialog / DevicePowerAction） |
| 实体定义 | 关机/重启全流程编排：回调三族分阶段执行、电源键长按弹窗交互、最终经 startup init 落地 |
| 核心特征 | 三桶 ShutdownCallbackHolder：takeover/async/sync（shutdown_controller.h:83-85）；started_ 原子防重入（:87）；DevicePowerAction 的命令集 UPDATER/SHUTDOWN/FLASH（device_power_action.cpp:25-29）；ShutdownDialog 持 AbilityConnection 与长按监测（shutdown_dialog.h:31-39,55-63） |
| 类型/分类 | 服务端控制器 + 独立弹窗应用协作 |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | Reboot/Shutdown → RebootOrShutdown：TakeOverShutdownAction 先问询接管回调（AllowedToBeTakenOver 判 reason）→ Prepare（TurnOffScreen、PublishShutdownEvent）→ TriggerAsyncShutdownCallback → TriggerSyncShutdownCallback（阻塞至各优先级完成）→ DevicePowerAction::Reboot/Shutdown → HOOK POWER_PRE_DO_* → DoRebootExt；电源键路径：SuspendController 长按监测 → ShutdownDialog::ConnectSystemUi 拉起 power_dialog → 用户选择回包后进入上述流程 |
| 状态流转 | started_: false→true（流程中，IsShuttingDown 可查）→ 终态由内核重启完成；isShutdownDialogForbid_ 可被设置禁弹窗（shutdown_dialog.h:69） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | ForceRebootDevice 忽略 takeover、60 秒内保证 DoRebootExt（power_mgr_client.h:47-55）；takeover 回调内禁止再触发关机（死锁）；GetShutdownReason 提供上次关机原因查询（≤96 字节，power_mgr_client.h:338-343） |
| 并发约束 | started_ 原子保证单次执行；对话框等待不阻塞主流程（FFRTQueue "shutdown_dialog"） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | PowerMgrService::RebootDevice/ShutDownDevice（power_mgr_service.h:90-93） |
| 下游影响 | DoRebootExt（startup init）、power_dialog（ArkTS）、TurnOffScreen 经状态机 |
| 平级关联 | SuspendController（HandleShutdown 转入）、HookMgr 插件（POWER_PRE_DO_REBOOT 可改 updater 命令）、hibernate 路径复用 takeover 桶（TriggerTakeOverHibernateCallback :59） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | ShutdownController、ShutdownCallbackHolder（含拼写 holer 的源文件名 shutdown_callback_holer.h）、ShutdownDialog、DevicePowerAction（services/native/src/shutdown/、actions/default/） |
| 核心方法 | RebootOrShutdown（shutdown_controller.h:65）、TriggerTakeOverShutdownCallback（:58）、TriggerSyncShutdownCallback（:61）、DoRebootExt 调用点（device_power_action.cpp:48,59）、StartDialog（shutdown_dialog.h:65） |

## S4 休眠控制器（HibernateController）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | HibernateController |
| 实体定义 | S4 磁盘休眠的编排者：休眠前后同步回调、结果回报与失败回退 |
| 核心特征 | HibernateStatus 三值（SUCCESS/FAILURE/INVALID，hibernate_controller.h:30-34）；按 HibernateCallbackPriority 排序的回调集（:56-58）；PreHibernate/PostHibernate 两段式（:42-43）；HibernateDeathRecipient 清理（:47-54） |
| 类型/分类 | 服务端控制器，由 POWER_MANAGER_POWER_ENABLE_S4 条件编译（power_mgr_service.h:213-215） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | PowerMgrService::Hibernate（oneway IPC，IPowerMgr.idl:89）→ PreHibernate 触发同步回调 → SystemSuspendController::Hibernate()（HDI）→ PostHibernate(result) 按结果通知；SuspendController 的 ACTION_HIBERNATE 也可转入（HandleHibernate） |
| 状态流转 | 回调按 pre/post 两段触发；进出休眠各发公共事件（PowerMgrNotify::PublishEnter/ExitHibernateEvent，power_mgr_notify.h:38-41） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 编译开关 power_manager_feature_enable_s4（默认 false）+ 运行参数 const.power.enable_s4（默认 true）双重门控（powermgr.gni:27；power_mgr_service.cpp:236-238）；唤醒源接管 IsWakeupDeviceSkip 在休眠态拦截唤醒（power_state_machine.cpp:697-699） |
| 并发约束 | 回调在 HDI 完成前后同步执行，禁止回调内长阻塞（低内存路径） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | PowerMgrService::HibernateControllerInit |
| 下游影响 | SystemSuspendController::Hibernate（HDI）、PowerMgrNotify |
| 平级关联 | ShutdownController（takeover 休眠问询）、SuspendController（ACTION_HIBERNATE） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | HibernateController（services/native/src/hibernate/hibernate_controller.h） |
| 核心方法 | Hibernate(bool clearMemory)（:39）、PreHibernate（:42）、PostHibernate（:43）、TriggerCallbacks（:61） |

## 睡眠执行栈（SystemSuspendController / AutoSuspend / RunningLockHub）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 睡眠执行栈（HDI 门面与 sysfs 历史通道） |
| 实体定义 | actions 层的执行通道集合：SystemSuspendController 是 HDI Power 唯一生产门面（睡眠/唤醒/持锁/配置全部经它）；AutoSuspend 与 RunningLockHub 是保留的 sysfs 直写实现（默认流程未调用，仅单测引用） |
| 核心特征 | SystemSuspendController : DelayedRefSingleton（system_suspend_controller.h:38）；WAKEUP_HOLDER="OHOSPowerMgr.WakeupHolder"（:63）；HDI 回调 PowerHdfCallback : V1_2::IPowerHdiCallback（:64-74）+ PowerHdiCallbackExt : V1_3::IPowerHdiCallbackExt（power_hdi_callback.h:35-42）；持锁走 HDI Hold/UnholdRunningLockExt（.cpp:253-266）；历史通道：AutoSuspend（SUSPEND_STATE="mem"、/sys/power/state、wakeup_count、100ms 重试，actions/default/suspend/suspend_controller.h:61-67）与 RunningLockHub（/sys/power/wake_lock|wake_unlock，running_lock_hub.h:45-46），sc_ 成员声明于 :79 但默认未初始化 |
| 类型/分类 | 生产通道（HDI）+ 历史通道（sysfs 线程/文件写入器，测试资产） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | 状态机经 DeviceStateAction::GoToSleep（device_state_action.cpp:303-308）→ SystemSuspendController::Suspend(onSuspend,onWakeup,force)：force 时 HDI ForceSuspend、否则 allowSleepTask_ 放行后 StartSuspend（.cpp:198-201）→ 睡/醒经 HDI OnSuspend/OnWakeup 回调上抛；唤醒经 DeviceStateAction::Wakeup→HDI StopSuspend（:61）；持锁经 RunningLockAction→AcquireRunningLock→HDI HoldRunningLockExt（.cpp:253-266）；显示完成回调由 DisplayPowerCallback 收口（OnDisplayStateChanged→WakeUpEnd/SuspendEnd + DISPLAY_ON/OFF_DONE 通知，:315-342） |
| 状态流转 | allowSleepTask_ 控制休眠放行（AllowAutoSleep/DisallowAutoSleep :50-51）；HDI 句柄失效经 hdiServStatListener_ 重建（:82-83）；历史通道 AutoSuspend 的 suspendBlockCounter_ 阻塞计数（actions/default/suspend :41-42）默认不生效 |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | SetSuspendTag 决定内核睡眠模式（mem/ulsr）；ForceSuspendEx 仅 V1_4（POWER_MANAGER_ENABLE_FORCE_SUSPEND_IGNORING_WAKELOCK，:30-31,53-54）；GetWakeupReason 供 HiSysEvent 归因 |
| 并发约束 | SystemSuspendController 用 FFRTQueue "power_system_suspend_controller" 串行化（:85）；历史通道 AutoSuspend 为独立 daemon 线程、sysfs fd 用 UniqueFd（running_lock_hub.h:48-49），均不在默认路径 |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | RunningLockMgr（锁）、PowerStateMachine/DeviceStateAction（睡眠）、SuspendController（强休 Ex） |
| 下游影响 | HDI Power V1_2/V1_3/V1_4、/sys/power/*、/sys/hibernate/*（init 预授权，etc/init/powermgr.cfg） |
| 平级关联 | hdi_service_status_listener（HDI 上下线）、DeviceStateAction 的 DisplayPowerCallback（亮灭屏完成通知）、SuspendController（强休 ForceSuspendEx 入口） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | SystemSuspendController（生产）、Suspend::SuspendController::AutoSuspend、Suspend::RunningLockHub（历史，services/native/src/actions/default/） |
| 核心方法 | Suspend/Wakeup/Hibernate（system_suspend_controller.h:40-42）、Acquire/ReleaseRunningLock（:43-44，.cpp:253-266）、ForceSuspendEx（:53-54）、AutoSuspendLoop/SuspendEnter（suspend_controller.h:51-54，历史） |

## 电源模式模块（PowerModeModule）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | PowerModeModule（模式与档位策略） |
| 实体定义 | 电源模式的持有与执行者：模式切换时按 XML 策略下发/恢复设置项，并通知回调与公共事件 |
| 核心特征 | mode_/lastMode_ 双值（power_mode_module.h:81-82）；forbidMap_ 模式禁止转移（:84）；CallbackManager（死亡感知 + WaitingCallback，:58-72）；RunAction 的八个 Set* 动作（DisplayOffTime/SleepTime/AutoAdjustBrightness/LcdBrightness/Vibration/WindowRotation/IntellVoiceState/SocPerfState，:101-108）；SocPerformance STANDARD/HIGH（:40-43） |
| 类型/分类 | 服务端组件，PowerMgrService 持有 |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | SetDeviceMode → CanTransitTo（InitPowerModeTransitMap）→ EnableMode(mode)：记录 lastMode、UpdateModepolicy 按 policyCache 逐项下发、RegisterSaveModeObserver 监听省电模式开关、PublishPowerModeEvent；恢复路径 RunAction(isBoot) 在开机重放（IsBootNeedActions :95） |
| 状态流转 | mode_ ∈ {600,601,602,603,650}；设置项观察者（SettingHelper::RegisterSettingPowerModeObserver）与模式双向同步（power_mgr_service.cpp:328-346 的 UpdateFunc） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 模式策略来自 power_mode_config.xml（PowerSaveMode::policyCache_，power_save_mode.h:54，SLEEP_FILTER_VALUE=102 :29）；省电模式禁用判定 IsPowerSaveModeDisabled（power_mode_module.h:49）；恢复值来自 recover_flag 与 ReadPowerModeRecoverMap |
| 并发约束 | mutex_ + once_flag isReady_（:111-112）；回调通知经 CallbackManager::WaitingCallback 收敛 |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | PowerMgrService::SetDeviceMode/GetDeviceMode（:158-159）与设置观察者注册（.cpp:328-334） |
| 下游影响 | SettingProvider 写（display off time/亮度/振动/旋转/智慧语音）、IPowerModeCallback、公共事件订阅者 |
| 平级关联 | PowerSaveMode（XML 解析）、SettingHelper（SaveCurrentMode/ReadCurrentMode :101-102） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | PowerModeModule、PowerSaveMode、ModePolicy（services/native/include/power_mode_module.h、power_save_mode.h） |
| 核心方法 | SetModeItem（:47）、EnableMode（:50）、UpdateModepolicy（:99）、RunAction（:100）、PublishPowerModeEvent（:87） |

## 设置助手（SettingHelper 与 SettingProvider）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 设置项读写门面（SettingHelper / SettingProvider） |
| 实体定义 | 全部电源相关持久化键的统一出入口：DataShare 读写、观察者注册、多用户与充电形态适配 |
| 核心特征 | SettingProvider : NoCopyable 单例（setting_provider.h:25-43）：Get/Put(String/Int/Long/Bool)Value、CreateObserver、UpdateCurrentUserId、CopyDataForUpdateScene；SettingHelper 静态门面覆盖 display off time、休眠/唤醒源 JSON、双击/拾起/翻盖、模式、通话亮屏、AOD（setting_helper.h:26-114） |
| 类型/分类 | utils/setting 为跨仓共享层（battery/thermal 同用）；SettingHelper 为本仓私有 |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | 服务端各控制器经 Register*Observer 订阅变化（如状态机 RegisterDisplayOffTimeObserver、唤醒源 RegisterSettingWakeupSourcesObserver）；模式模块经 SaveCurrentMode 持久化；boot 后 CopyDataForUpdateScene/UpdateCurrentUserId 完成多用户切换（power_mgr_service.cpp:264-267） |
| 状态流转 | 键值变更 → SettingObserver::UpdateFunc → 各控制器改写行为（如 WakeupParseJsonConfig 重建唤醒源） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 充电形态下 display off time/sleep time/suspend sources 分 AC/DC 两套键（setting_helper.h:42-60）；键合法性先 IsValidKey/Is*SettingValid 校验；禁止在 utils 层加入 power 私有键（AGENTS.md） |
| 并发约束 | DataShare 异步回调线程，UpdateFunc 内禁止重入 Put |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | PowerStateMachine、SuspendController、WakeupController、PowerModeModule |
| 下游影响 | settingsdata（DataShare）；跨仓消费者 battery_manager/thermal_manager 共用 provider |
| 平级关联 | ParamCacher（para 缓存：active_time_before_long_time_dim 等，power_state_machine.cpp:1545-1548） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | SettingProvider（utils/setting/include/setting_provider.h）、SettingHelper（services/native/src/setting/setting_helper.h） |
| 核心方法 | SettingHelper::GetSettingDisplayOffTime（:62）、RegisterSettingWakeupSourcesObserver（:87）、SaveCurrentMode/ReadCurrentMode（:101-102）、SettingProvider::CreateObserver（setting_provider.h:38） |

## 通知与诊断（PowerMgrNotify / PowerMgrDumper）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 对外通知与诊断（PowerMgrNotify / PowerMgrDumper） |
| 实体定义 | 状态广播与可观测性出口：Notify 面向全系统发公共事件，Dumper 面向 hidumper/CLI 输出诊断文本 |
| 核心特征 | Notify 预构建 Want（screenOnWant_/screenOffWant_ 等，power_mgr_notify.h:49-58）批量发布；事件族：ScreenOn/ScreenOff、Enter/ExitForceSleep、Enter/ExitHibernate；Dumper 提供 Dump/DumpArg/DumpRunningLockInfo/ShowUsage（power_mgr_dumper.h:30-35） |
| 类型/分类 | 服务端辅助组件；Dump 同时支撑 ShellDumpIpc（IPowerMgr.idl:93，CLI 远程调用） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | 状态机亮灭屏 → PublishScreenOnEvents/PublishScreenOffEvents（带 eventTime 与 reason）；PublishCustomizedEvent 供定制事件（:42）；hidumper -s 3301 或 power-shell dump → ShellDump → PowerMgrDumper::Dump 汇总状态机/运行锁/休眠栈信息（system_suspend_controller.h:45 的 Dump） |
| 状态流转 | RegisterPublishEvents 在服务 Init 时注册一次（power_mgr_service.cpp:476） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 亮灭屏双写公共事件 + POWER_UE HiSysEvent（SCREEN_ON/SCREEN_OFF，powermanager_POWER_UE.yaml）；强制休眠广播受 POWER_MANAGER_ENABLE_FORCE_SLEEP_BROADCAST 门控 |
| 并发约束 | 发布经 ffrt（power_mgr_notify.h:25）；Dumper 读运行锁需持 runningLocks_ 快照 |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | PowerStateMachine、SuspendController（强休广播）、HibernateController（S4 事件） |
| 下游影响 | 全系统公共事件订阅者（锁屏、通话等）、HiView/功率统计 |
| 平级关联 | power-shell CLI（tools/ohos-powerManager，默认 support_power_cli=false）、POWER_HILOG 日志体系（utils/native/include/power_log.h:79-124） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | PowerMgrNotify（services/native/include/power_mgr_notify.h）、PowerMgrDumper（services/native/include/power_mgr_dumper.h） |
| 核心方法 | PublishScreenOffEvents/PublishScreenOnEvents（power_mgr_notify.h:32-33）、PublishEnterHibernateEvent（:39）、PowerMgrDumper::Dump（power_mgr_dumper.h:30） |
