# StickyNotes 项目结构

Windows 平台桌面便签应用。基于 Qt 6 + ElaWidgetTools，遵循"核心 / 平台 / UI / 应用"四层分离架构。

---

## 1. 顶层目录

```
software/
├── CMakeLists.txt           # 顶层构建 + 依赖 (Qt6, googletest, ElaWidgetTools)
├── LICENSE                  # MIT 占位
├── src/
│   ├── core/                # 纯业务模型 + 存储（与 Qt 解耦）
│   ├── platform/            # 平台抽象 + Win 实现
│   ├── app/                 # 应用装配 + 入口
│   └── ui/                  # 表现层
├── tests/                   # GoogleTest 单元测试
├── scripts/                 # 打包脚本
├── docs/                    # 设计 / 计划文档
│   └── superpowers/
│       ├── specs/2026-06-05-stickynotes-design.md
│       └── plans/           # 2026-06-05-stickynotes-plan + part1/part2/part3
└── build/ build-release/    # 编译输出
```

---

## 2. src/core — 核心层

**职责**：领域模型 + 文件存储。**不依赖 Qt/平台**（仅依赖 `platform::IFileSystem` 抽象接口）。

```
src/core/
├── include/core/
│   ├── inotestore.h         # 便签存储接口（INoteStore）
│   ├── file_notestore.h     # INoteStore 实现声明
│   ├── note.h               # Note 领域对象
│   └── settings.h           # JSON 配置对象
└── src/
    ├── file_notestore.cpp   # 文件式存储（<data>/notes/<id>.md）
    ├── note_serialize.cpp   # Note ↔ JSON
    └── settings.cpp
```

### 关键类型

| 类型 | 说明 |
|---|---|
| `core::Note` | 便签。字段：`id, title, content, categoryId, tags, createdAt, updatedAt, deletedAt, pinned, windowGeometry` |
| `core::INoteStore` | 接口：`all / get / trash / create / upsert / softDelete / restore / permanentDelete / query / setNoteChangedCallback` |
| `core::FileNoteStore` | 唯一实现。文件格式：YAML front matter (JSON) + Markdown 正文 |
| `core::Settings` | `<data>/settings.json` 包装，含 `hotkey` 字符串、`dataDir` |

### 存储格式

`<id>.md`：
```
---
{ ... JSON: id, title, tags, createdAt, updatedAt, deletedAt, pinned, windowGeometry, ... }
---
<content 正文>
```

### 软删模型

| 操作 | `deletedAt` 字段 | 文件 |
|---|---|---|
| 软删 | `= now` | 保留 |
| 恢复 | 清空 | 保留 |
| 永久删 | (内存) | 真删 |

`all()` 排除 `deletedAt.isValid()`；`trash()` 只返回 `deletedAt.isValid()`。

### 变更通知

`setNoteChangedCallback(std::function<void(QString id)>)` —— 单一 std::function（MOC 不需要，跨线程友好）。修改/创建/删除/恢复任一操作触发。

---

## 3. src/platform — 平台抽象层

**职责**：跨平台接口 + Windows 实现。**不依赖 Qt Widgets**（只 Core + Win32）。

```
src/platform/
├── include/platform/
│   ├── ifilesystem.h        # 文件 I/O 抽象
│   ├── iclock.h             # 时钟抽象
│   ├── ihotkey.h            # 全局热键抽象
│   └── itrayicon.h          # 系统托盘抽象
└── win/
    ├── filesystem_qt.cpp    # 用 QFile 实现的 IFileSystem
    ├── system_clock.cpp     # QDateTime 包装
    ├── hotkey_win.cpp       # Win32 RegisterHotKey
    └── trayicon_win.cpp     # Win32 Shell_NotifyIcon
```

### 接口对照

| 接口 | Win 实现 | 关键 API |
|---|---|---|
| `IFileSystem` | `FileSystem_Qt` | `readAll / writeAtomic / list(glob) / ensureDir / removeFile` |
| `IClock` | `SystemClock` | `now()` |
| `IHotkey` | `Hotkey_Win` | `registerHotkey(Spec) / unregisterHotkey(id) / setTriggeredCallback` |
| `ITrayIcon` | `TrayIcon_Win` | `show / hide / setMenu / setLeftClickCallback` |

**显式未提供** `INotifier`（提醒通知）— 提醒功能已剥离。

### 关键设计

- 接口用 `std::function` 回调（非 Qt signal/slot），**不依赖 Q_OBJECT**
- 所有实现都在 `win/` 子目录；为未来 Mac/Linux 留扩展点（待加 `platform/mac/`, `platform/linux/`）

---

## 4. src/app — 应用层

**职责**：装配 + 生命周期 + 入口。

```
src/app/
├── include/app/
│   ├── app_context.h        # 所有服务的 unique_ptr 容器
│   └── runtime.h            # 启动/退出/热键重注册
├── src/
│   ├── app_context.cpp      # production() 工厂：实例化所有服务
│   └── runtime.cpp          # wireHotkey/wireTray + show MainWindow
└── main.cpp                 # QApplication + FreeConsole 防御 + 启动
```

### AppContext

```cpp
struct AppContext {
    std::unique_ptr<platform::IFileSystem> fs;
    std::unique_ptr<platform::IClock> clock;
    std::unique_ptr<platform::IHotkey> hotkey;
    std::unique_ptr<platform::ITrayIcon> tray;
    std::unique_ptr<core::INoteStore> notes;
    std::unique_ptr<core::Settings> settings;
};
```

`AppContext::production(dataDir)` 工厂函数，new 出所有具体服务并连接。**所有权清晰，无裸指针跨层传递**。

### Runtime::start()

```
1. new MainWindow(ctx_)
2. setReregisterHotkey lambda
3. aboutToQuit → settings.save()
4. wireHotkey(): registerHotkey, callback → create + StickyWindow
5. wireTray():   setMenu(新建/打开/退出) + left-click toggle
6. main_->show()
```

### 入口 main.cpp

```cpp
FreeConsole();                  // 防御 Ela init() 触发 conhost
QApplication app(argc, argv);
eApp->init();                   // ElaWidgetTools
FreeConsole();                  // 再次防御
... 装配 AppContext + Runtime ...
rt.start();
FreeConsole();                  // 第三次防御
app.exec();
```

---

## 5. src/ui — 表现层

**职责**：Qt Widgets 窗口、控件、布局、Ela 主题。

```
src/ui/
├── include/ui/
│   ├── mainwindow.h
│   ├── stickywindow.h
│   ├── note_editor.h
│   ├── settings_dialog.h
│   └── hotkey_edit.h
└── src/    # 对应 .cpp
```

### 类关系

```
MainWindow (ElaWindow)
├── 工具栏 (ElaToolBar)         → onNewNote / onDeleteNote / onRestoreNote /
│                                  onPermanentDeleteNote / onOpenSticky /
│                                  onExportNotes / onImportNotes / onShowSettings /
│                                  主题切换
├── 分类按钮组 (QButtonGroup)    → refreshList
├── 列表 (ElaListView)           → onNoteSelected
├── 编辑器 (NoteEditor)
│     ├── ElaLineEdit 标题
│     └── ElaPlainTextEdit 内容
├── 状态栏 (QLabel × 2)          → updateStatusBar
└── 设置弹窗 (SettingsDialog)
      ├── 数据目录 (QFileDialog)
      └── 全局快捷键 (HotkeyEdit)
              └→ 触发 Runtime::reregisterHotkey

StickyWindow (QWidget, Frameless + StaysOnTopHint)
├── 自定义标题栏 (titleBar_)
│     ├── ElaText 标题
│     ├── ElaLineEdit 标题编辑
│     ├── ElaToolButton 置顶
│     └── ElaToolButton 关闭
└── ElaPlainTextEdit 内容
      └→ onSave → ctx.notes->upsert(note)
```

### 关键控件

| 控件 | 角色 | 设计点 |
|---|---|---|
| `NoteEditor` | 标题 + 内容编辑 | `Mode::ReadWrite` 当前唯一分支（ReadOnly 枚举值保留为将来） |
| `StickyWindow` | 独立便签浮窗 | 置顶切换用 `SetWindowPos(HWND_TOPMOST/NOTOPMOST)` 而非 Qt flag |
| `HotkeyEdit` | QLineEdit 子类 | 点击后捕获组合键（`focusInEvent` → `keyPressEvent` → `commitCaptured`），解决中文 IME 干扰 |
| `SettingsDialog` | 数据目录 + 快捷键 | `setReregisterHotkey` 回调注入 hotkey 重注册 |

### 已知问题

- `MainWindow::stickyByNote_` 缓存 StickyWindow 但从不 remove → 内存累积（已知 bug）

---

## 6. tests/ — 测试

```
tests/
├── smoke_test.cpp
├── platform/                # 平台接口契约 + Win 实现
│   ├── clock_test.cpp       # IClock / SystemClock
│   ├── hotkey_test.cpp      # IHotkey::parseSpec 等
│   ├── trayicon_test.cpp    # Mock + 接口契约
│   ├── ihotkey_parsespec_test.cpp
│   ├── mocks.h              # MockHotkey, MockTrayIcon
│   └── mock_clock.h         # FakeClock (IClock 实现)
├── core/
│   ├── file_notestore_test.cpp  # CRUD / 软删 / 恢复 / 永久删
│   ├── note_serialize_test.cpp  # Note ↔ JSON 往返
│   ├── settings_test.cpp        # 读写
│   └── fake_filesystem.h        # IFileSystem 测试替身
├── ui/
│   └── hotkey_edit_test.cpp     # HotkeyEdit 输入捕获
└── CMakeLists.txt
```

### 测试结构

- **`FakeClock`**：IClock 测试替身（`FakeClock.h`），可手动设时间/推时间
- **`FakeFileSystem`**：IFileSystem 测试替身（`fake_filesystem.h`），内存模拟
- **`MockHotkey` / `MockTrayIcon`**：GMock 实现的接口 mock（`mocks.h`）
- **`FakeClock`** 与 **gtest 配合**：构造、CRUD、序列化都用 Fake 替代真实平台

---

## 7. scripts/ — 打包脚本

```
scripts/
├── package-portable.ps1     # 绿色版打包：Release + windeployqt + 拷贝
└── package-installer.ps1    # NSIS 安装包：写 .nsi + makensis 编译
```

两个脚本相互独立，共享 `dist/` 输出目录。

---

## 8. docs/ — 文档

```
docs/superpowers/
├── specs/
│   └── 2026-06-05-stickynotes-design.md   # 主设计稿
└── plans/
    ├── 2026-06-05-stickynotes-plan.md     # 总计划
    ├── part1-m0-m2.md                    # 里程碑 0-2
    ├── part2-m3-m4.md                    # 里程碑 3-4
    └── part3-m5-m6.md                    # 里程碑 5-6
```

设计稿覆盖：四层架构、核心数据结构、文件格式、平台接口、UI 控件清单、ElaWidgetTools 集成、Milestone 划分。

---

## 9. 四层依赖关系

```
         ┌──────────────┐
         │   main.cpp   │
         └──────┬───────┘
                │
     ┌──────────┼──────────┐
     │          │          │
     ▼          ▼          ▼
  ┌─────┐  ┌───────┐  ┌─────────┐
  │app::│  │app::  │  │ ElaApp  │
  │Run  │◄─┤AppCtx │  │(Ela SDK)│
  └──┬──┘  └───┬───┘  └─────────┘
     │         │
     ▼         ▼
  ┌─────┐  ┌───────┐  ┌─────────┐
  │ui:: │◄─┤core:: │◄─┤platform::│
  │Main │  │INote… │  │Ixxx     │
  └─────┘  └───────┘  └─────────┘
```

| 层 | 依赖 |
|---|---|
| `core` | 仅 `platform::IFileSystem`（抽象接口） |
| `platform` | 仅 Qt Core（接口本身不依赖 Qt；实现用到 QtCore/Win32） |
| `ui` | `core` + `app::AppContext` + `app::Runtime` + ElaWidgetTools |
| `app` | `core` + `platform` + `ui` + ElaApplication |

**编译依赖单向**：app → ui → core + platform；core 不反向依赖任何上层。

---

## 10. 数据持久化位置

| 数据 | 路径 |
|---|---|
| 便签 | `%APPDATA%\StickyNotes\notes\<id>.md` |
| 配置 | `%APPDATA%\StickyNotes\settings.json` |
| 设置中的 `dataDir` | 可改，默认是上面路径 |

`%APPDATA%\StickyNotes` = `QStandardPaths::AppDataLocation` 解析结果。

---

## 11. 显式不做的部分

| 功能 | 状态 |
|---|---|
| 提醒通知 | 已剥离 |
| 自定义分类 | UI 占位 + 接口保留，未启用 |
| 富文本 / Markdown 渲染 | 仅纯文本 |
| 便签搜索 | 接口 `query()` 有，UI 未接 |
| 便签排序 | 列表固定按 `updatedAt` 降序 |
| 云同步 / 多端 | v1 不做 |
| macOS / Linux | 仅 Windows 平台实现 |

---

## 12. 关键设计决策的"为什么"

| 决策 | 备选 | 选择原因 |
|---|---|---|
| 软删用 `deletedAt` 字段 | 独立 Trash 子目录 | 简单；恢复只改一个字段 |
| 便签存为 Markdown + YAML | 纯 JSON / SQLite | 用户可直接打开看；人类可读；可手动改 |
| 平台接口用 std::function 回调 | Qt signal/slot | 接口不含 Q_OBJECT，core/platform 层不依赖 Qt |
| StickyWindow 独立浮窗 | 单窗口多 tab | 符合桌面便签用户习惯（贴边） |
| HotkeyEdit 自定义 | QKeySequenceEdit | 解决中文 IME 干扰；行为可测 |
| FetchContent 拉 Ela | find_package 预装 | 开发环境无需预装第三方 |
| AppContext unique_ptr | 共享指针 | 所有权清晰，无环依赖 |

---

## 13. Git 备份标签

| 标签 | commit | 状态 |
|---|---|---|
| `backup/before-deadcode-cleanup` | `5dc3a31` | 死代码清理前 |
| `backup/after-deadcode-cleanup` | `90172b3` | 死代码清理后 |
| `backup/with-packaging` | `ec77325` | 加打包后 |
| `backup/with-installer` | `c929d97` | 加 NSIS 安装包后 |
| `backup/with-freeconsole` | `cfb7fe7` | 修 conhost 窗口后（当前 HEAD） |

---

## 14. 维护入口

- **加新便签字段**：`core::Note` → `core::Note::toJson/fromJson` → `core::FileNoteStore::loadIndex/persistNote`（注意旧数据兼容）
- **加新平台接口**：`platform/include/xxx.h` → `platform/win/xxx.cpp` → `AppContext` 注入 → 消费方通过 `ctx_.xxx` 取
- **改 Ela 行为**：通常在 `src/app/runtime.cpp` 找入口，避免直接改 FetchContent 源码
- **扩展跨平台**：加 `platform/mac/`, `platform/linux/` 平行实现，`AppContext::production` 工厂里按宏分支选

---

## 15. 许可证

本项目代码 MIT License（详见 [LICENSE](LICENSE)）。
第三方依赖保留各自许可证（Qt: LGPL/商业双许可；ElaWidgetTools: Apache-2.0；GoogleTest: BSD-3）。
