# StickyNotes 实施计划（总索引）

> 本计划按里程碑拆为 3 份执行文件，每份独立可编译、可测试：
>
> | 文件 | 范围 |
> | --- | --- |
> | [part1-m0-m2.md](part1-m0-m2.md) | 仓库骨架 + 平台抽象 + core 数据层 |
> | [part2-m3-m4.md](part2-m3-m4.md) | app 编排 + ReminderService + 主窗口 UI |
> | [part3-m5-m6.md](part3-m5-m6.md) | 便签小窗 + Win 集成 + 打包 |

**Goal:** 在 Windows + MSVC2022 + Qt6 + CMake 环境下交付一个**高内聚低耦合、可 GoogleTest 测试、UI 走 QFluentWidgets** 的桌面便签软件 v1。

**Architecture:** 严格 4 层 `platform → core → app → ui`；副作用（FS / 时钟 / 通知 / 快捷键 / 托盘）全走接口；`INoteStore` 是唯一真源并持视图锁。

**Tech Stack:** C++17、Qt 6.6+（Widgets）、CMake 3.21+、MSVC 2022、QFluentWidgets（FetchContent）、GoogleTest 1.15.2（FetchContent）、NSIS（CPack）。

---

## 全局约定（每份 part 都遵守）

### 目录

```
d:\DPracProj\software\
├── CMakeLists.txt                 # 顶层
├── src/
│   ├── platform/
│   │   ├── include/platform/      # 接口头
│   │   └── win/                   # Win 实现
│   ├── core/
│   │   ├── include/core/
│   │   └── src/
│   ├── app/
│   │   ├── include/app/
│   │   └── src/
│   └── ui/
│       ├── include/ui/
│       └── src/
├── tests/
│   ├── platform/   tests/core/   tests/app/   tests/ui/
├── third_party/                   # 不放源码，FetchContent 拉
├── cmake/                         # 自定义 CMake 模块
└── docs/superpowers/
    ├── specs/2026-06-05-stickynotes-design.md
    └── plans/                     # 本目录
```

### 命名

- 接口：I 前缀（`IFileSystem`）
- 实现：接口名去掉 I + 后缀（`FileSystem_Qt` / `FileSystem_Win`），或场景名（`FileNoteStore`）
- 私有成员：下划线结尾（`store_`）
- 测试夹具：`<Module>Test`（如 `FileNoteStoreTest`）

### 提交

- 一次提交 = 一个任务末尾的"Commit"步骤
- 提交信息：`feat(Mn): <一句话>` / `test(Mn): <一句话>` / `chore(Mn): <一句话>`
- 不在 plan 任务里自动 commit，由 executor 执行

### 覆盖率门禁

| 模块 | 行覆盖 | 分支覆盖 |
| --- | --- | --- |
| `core/` | ≥ 80% | ≥ 70% |
| `platform/win` | ≥ 60% | — |
| `app/` | ≥ 70% | — |
| `ui/` | 不强制 | — |

CI 命令：`ctest --output-on-failure`，覆盖率达门禁才放行。

---

## 任务索引（按里程碑）

| M | 任务 | 关键产物 |
| --- | --- | --- |
| M0.1 | 顶层 CMake + 最小 `app` 入口 | `cmake -S . -B build` 成功 |
| M0.2 | GoogleTest 集成 | `ctest` 跑通 1 个样例 |
| M0.3 | 模块骨架（空 target） | 4 个空 lib + 1 个 exe |
| M1.1 | `IFileSystem` 接口 + `FileSystem_Qt` 实现 | 接口 + Win 实现 |
| M1.2 | `IClock` + `SystemClock` + `FakeClock` | 含测试 |
| M1.3 | `INotifier/IHotkey/ITrayIcon` 接口 + Win 桩 | 仅接口 + Win 实现 |
| M1.4 | `MockFileSystem/Notifier/Hotkey/Tray` GoogleMock | 复用 |
| M2.1 | `Note/Category` 数据结构（Q_GADGET） | 含序列化 |
| M2.2 | `MarkdownCodec` 双向转换 | ≥ 10 用例 |
| M2.3 | `INoteStore` 接口（带视图锁） | 接口 + 文档 |
| M2.4 | `FileNoteStore` 实现 | 索引、原子写、损坏隔离、视图锁 |
| M2.5 | `Settings`（settings.json 读写） | 含默认值兜底 |
| M3.1 | `AppContext::production()` 装配 | 启动顺序 |
| M3.2 | `AppContext::test()` 注入 fake | 测试用 |
| M3.3 | `ReminderService` 调度 | 过期清理、去重、并发 tick |
| M4.x | 主窗口 UI（详见 part2） | — |
| M5.x | 小窗 UI（详见 part3） | — |
| M6.x | 打包（详见 part3） | — |

---

## 风险与缓解

| 风险 | 缓解 |
| --- | --- |
| QFluentWidgets FetchContent 拉取失败 | 备选：手动 git clone 到 `third_party/qfluentwidgets` 后 `add_subdirectory` |
| GoogleTest 与 MSVC CRT 冲突 | `set(gtest_force_shared_crt ON)` |
| `Q_OBJECT` 跨 moc 边界（`INoteStore`） | 该接口只放一个 `Q_OBJECT` 父类 + 信号；实现类继承并 emit |
| 视图锁实现死锁 | release 走 RAII + 单元测试覆盖重入场景 |
