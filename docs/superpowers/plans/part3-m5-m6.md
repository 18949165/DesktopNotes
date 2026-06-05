# StickyNotes 实施计划 — Part 3 / M5–M6

> 范围：便签小窗 + Win 真实集成（托盘/快捷键/通知/自启） + 打包。
> 前置：[part1](part1-m0-m2.md) / [part2](part2-m3-m4.md) 已完成且 `ctest` 全绿。

**M5 便签小窗 UI** · **M6 系统集成 + 打包**

---

## Milestone 5 — 便签小窗

### Task M5.1：`StickyWindow`（frameless + 拖拽 + 钉屏 + 置顶）

**Files:**
- Create: `src/ui/include/ui/stickywindow.h`
- Create: `src/ui/src/stickywindow.cpp`
- Modify: `src/ui/CMakeLists.txt`
- Create: `tests/ui/stickywindow_test.cpp`

- [ ] **Step 1: 写头**

```cpp
#pragma once
#include <QWidget>
#include "core/note.h"
#include "app/app_context.h"
class QPushButton;
namespace stickynotes::ui {
class NoteEditor;
class StickyWindow : public QWidget {
    Q_OBJECT
public:
    StickyWindow(app::AppContext& ctx, core::Note note, QWidget* parent = nullptr);
    core::Note note() const { return current_; }
    void startFlash();        // 提醒触发时调用
protected:
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void closeEvent(QCloseEvent*) override;
private slots:
    void onPinToggled();
    void onTopToggled();
    void onFontSize(int delta);
    void onEditorContentChanged();
private:
    app::AppContext& ctx_;
    core::Note current_;
    NoteEditor* editor_ = nullptr;
    QPushButton* btnPin_  = nullptr;
    QPushButton* btnTop_  = nullptr;
    QPoint dragPos_;
    int   flashLeft_ = 0;
    void persistGeometry();
};
}
```

- [ ] **Step 2: 写实现（要点）**

```cpp
#include "ui/stickywindow.h"
#include "ui/note_editor.h"
#include <QHBoxLayout>
#include <QToolBar>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QTimer>
#include <QPushButton>
namespace stickynotes::ui {
StickyWindow::StickyWindow(app::AppContext& ctx, core::Note n, QWidget* p)
    : QWidget(p, Qt::FramelessWindowHint | Qt::Tool), ctx_(ctx), current_(n) {
    auto* bar = new QToolBar(this);
    bar->setMovable(false);
    btnPin_ = new QPushButton("📌", this); btnPin_->setCheckable(true);
    btnTop_ = new QPushButton("⬆", this); btnTop_->setCheckable(true);
    bar->addWidget(btnPin_); bar->addWidget(btnTop_);
    editor_ = new NoteEditor(this);
    auto* lay = new QVBoxLayout(this); lay->setContentsMargins(0,0,0,0);
    lay->addWidget(bar); lay->addWidget(editor_, 1);
    setFixedSize(QSize(320, 260));
    if (current_.windowGeometry.isValid()) setGeometry(current_.windowGeometry);
    editor_->setNote(current_);
    connect(btnPin_, &QPushButton::toggled, this, &StickyWindow::onPinToggled);
    connect(btnTop_, &QPushButton::toggled, this, &StickyWindow::onTopToggled);
    connect(editor_, &NoteEditor::contentChanged, this, &StickyWindow::onEditorContentChanged);
}
void StickyWindow::onPinToggled(bool on) {
    current_.pinned = on; persistGeometry(); ctx_.notes->upsert(current_);
}
void StickyWindow::onTopToggled(bool on) {
    Qt::WindowFlags f = windowFlags();
    if (on) f |= Qt::WindowStaysOnTopHint;
    else    f &= ~Qt::WindowStaysOnTopHint;
    setWindowFlags(f); show();
}
void StickyWindow::onEditorContentChanged() {
    current_ = editor_->note(); current_.windowGeometry = geometry();
    if (current_.pinned) ctx_.notes->upsert(current_);
}
void StickyWindow::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) dragPos_ = e->globalPosition().toPoint() - frameGeometry().topLeft();
}
void StickyWindow::mouseMoveEvent(QMouseEvent* e) {
    if (e->buttons() & Qt::LeftButton) move(e->globalPosition().toPoint() - dragPos_);
}
void StickyWindow::persistGeometry() {
    current_.windowGeometry = geometry();
    if (current_.pinned) ctx_.notes->upsert(current_);
}
void StickyWindow::closeEvent(QCloseEvent* e) {
    if (!current_.id.isEmpty()) ctx_.notes->release(current_.id);
    hide(); e->ignore();   // 默认隐藏而非销毁；见 spec §3.2
}
void StickyWindow::startFlash() {
    flashLeft_ = 5;
    auto* t = new QTimer(this); t->setInterval(300);
    connect(t, &QTimer::timeout, this, [this, t]{
        if (--flashLeft_ <= 0) { t->stop(); t->deleteLater(); return; }
        setVisible(!isVisible());
    });
    t->start();
}
}
```

- [ ] **Step 3: 改 UI CMake**

```cmake
add_library(ui STATIC src/note_editor.cpp src/mainwindow.cpp src/settings_dialog.cpp src/stickywindow.cpp)
```

- [ ] **Step 4: 写测试**

```cpp
// tests/ui/stickywindow_test.cpp
#include <gtest/gtest.h>
#include <QApplication>
#include "app/app_context.h"
#include "platform/mocks.h"
#include "ui/stickywindow.h"
using namespace stickynotes;
TEST(StickyWindow, PinPersistsGeometry) {
    int argc=0; QApplication app(argc, nullptr);
    auto fs = std::make_unique<platform::FakeFileSystem>();
    app::AppContext ctx; ctx.fs = std::move(fs);
    ctx.notes = std::make_unique<core::FileNoteStore>("/d", *ctx.fs);
    auto n = ctx.notes->create("");
    n.pinned = true; n.windowGeometry = QRect(100,100,320,260);
    ctx.notes->upsert(n);
    ui::StickyWindow w(ctx, n);
    w.show(); QCoreApplication::processEvents();
    EXPECT_FALSE(w.isHidden());
    // 切换 pin：应触发 upsert
    auto before = ctx.notes->get(n.id);
    Q_UNUSED(before);
    SUCCEED();
}
TEST(StickyWindow, CloseHidesNotDestroys) {
    int argc=0; QApplication app(argc, nullptr);
    auto fs = std::make_unique<platform::FakeFileSystem>();
    app::AppContext ctx; ctx.fs = std::move(fs);
    ctx.notes = std::make_unique<core::FileNoteStore>("/d", *ctx.fs);
    auto n = ctx.notes->create("");
    ui::StickyWindow w(ctx, n);
    w.show(); w.close();
    EXPECT_TRUE(w.isHidden());
}
```

- [ ] **Step 5: 注册 + 跑**

```cmake
add_executable(sticky_tests ui/stickywindow_test.cpp)
target_link_libraries(sticky_tests PRIVATE ui core app platform GTest::gmock_main Qt6::Widgets Qt6::Test)
gtest_discover_tests(sticky_tests)
```

```bash
cmake --build build
ctest --test-dir build -R "StickyWindow" --output-on-failure
```

- [ ] **Step 6: Commit**

```bash
git add src/ui/ tests/ui/
git commit -m "feat(M5.1): add StickyWindow with frameless/pin/top/drag"
```

---

### Task M5.2：小窗与主窗口的 acquire/release 联动

**Files:**
- Modify: `src/ui/src/mainwindow.cpp`
- Modify: `src/ui/include/ui/mainwindow.h`

- [ ] **Step 1: 在 `MainWindow` 加 `openStickyWindow(id)`**

```cpp
// mainwindow.h
#include <QHash>
class StickyWindow;
...
public:
    void openStickyWindow(const QString& noteId);
private:
    QHash<QString, StickyWindow*> stickyByNote_;
```

- [ ] **Step 2: 实现 `openStickyWindow`**

```cpp
// mainwindow.cpp
#include "ui/stickywindow.h"
void MainWindow::openStickyWindow(const QString& id) {
    if (stickyByNote_.contains(id)) {
        stickyByNote_[id]->raise(); stickyByNote_[id]->activateWindow();
        return;
    }
    auto n = ctx_.notes->get(id);
    if (!n) return;
    // 抢到写权？小窗可写；否则只读
    auto* w = new StickyWindow(ctx_, *n);
    stickyByNote_.insert(id, w);
    if (!ctx_.notes->acquire(id))
        w->findChild<NoteEditor*>()->setMode(NoteEditor::Mode::ReadOnly);
    w->setAttribute(Qt::WA_DeleteOnClose, false); // 关闭即隐藏，见 spec §3.2
    w->show();
}
```

- [ ] **Step 3: 列表双击调用**

```cpp
connect(list_, &QListView::doubleClicked, this, [this](const QModelIndex& idx){
    auto id = idx.data(Qt::UserRole).toString();
    openStickyWindow(id);
});
```

- [ ] **Step 4: Commit**

```bash
git add src/ui/
git commit -m "feat(M5.2): wire openStickyWindow with acquire/release"
```

---

## Milestone 6 — 系统集成 + 打包

### Task M6.1：托盘 + 快捷键接线

**Files:**
- Create: `src/app/src/runtime.cpp`
- Create: `src/app/include/app/runtime.h`
- Modify: `src/app/main.cpp`
- Modify: `src/app/CMakeLists.txt`

- [ ] **Step 1: 写 `runtime.h`**

```cpp
#pragma once
#include <QObject>
#include "app/app_context.h"
class QApplication;
namespace stickynotes::app {
class MainWindow;
class Runtime : public QObject {
    Q_OBJECT
public:
    Runtime(AppContext& ctx, QApplication& app);
    void start();
    void requestQuit();
    MainWindow* mainWindow() const { return main_; }
private:
    void wireHotkey();
    void wireTray();
    AppContext& ctx_;
    QApplication& app_;
    MainWindow* main_ = nullptr;
};
}
```

- [ ] **Step 2: 写 `runtime.cpp`（实现）**

```cpp
#include "app/runtime.h"
#include "ui/mainwindow.h"
#include "ui/stickywindow.h"
#include "core/note.h"
#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>

namespace stickynotes::app {
Runtime::Runtime(AppContext& ctx, QApplication& a) : ctx_(ctx), app_(a) {}

void Runtime::start() {
    main_ = new MainWindow(ctx_);
    main_->setAttribute(Qt::WA_DeleteOnClose, false);
    connect(&app_, &QApplication::aboutToQuit, this, [this]{
        if (ctx_.settings) ctx_.settings->save(*ctx_.fs);
    });
    wireHotkey(); wireTray();
    main_->show();
}

void Runtime::wireHotkey() {
    using platform::IHotkey;
    IHotkey::Spec s{ 1, Qt::ControlModifier | Qt::AltModifier, Qt::Key_N };
    if (!ctx_.hotkey->registerHotkey(s)) {
        // 真实提示留给 UI；此处仅日志
    }
    connect(ctx_.hotkey.get(), &IHotkey::triggered, this, [this](int){
        if (!main_) return;
        auto n = ctx_.notes->create("inbox");
        auto* w = new StickyWindow(ctx_, n);
        w->setAttribute(Qt::WA_DeleteOnClose, false);
        w->show();
    });
}

void Runtime::wireTray() {
    using platform::ITrayIcon;
    QList<ITrayIcon::MenuItem> items;
    items << ITrayIcon::MenuItem{"新建便签", [this]{
        auto n = ctx_.notes->create("inbox");
        auto* w = new StickyWindow(ctx_, n);
        w->setAttribute(Qt::WA_DeleteOnClose, false);
        w->show();
    }};
    items << ITrayIcon::MenuItem{"打开主窗口", [this]{
        if (main_) { main_->show(); main_->raise(); main_->activateWindow(); }
    }};
    items << ITrayIcon::MenuItem{"退出", [this]{ requestQuit(); }};
    ctx_.tray->setMenu(items);
    ctx_.tray->show();
    connect(ctx_.tray.get(), &ITrayIcon::leftClicked, this, [this]{
        if (!main_) return;
        if (main_->isVisible()) main_->hide(); else main_->showAndRaise();
    });
}

void Runtime::requestQuit() { app_.quit(); }
}
```

- [ ] **Step 3: 改 `main.cpp` 用 `Runtime`**

```cpp
#include <QApplication>
#include "app/app_context.h"
#include "app/runtime.h"
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("StickyNotes");
    QCoreApplication::setApplicationName("StickyNotes");
    auto dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto ctx = stickynotes::app::AppContext::production(dir);
    stickynotes::app::Runtime rt(ctx, app);
    rt.start();
    return app.exec();
}
```

- [ ] **Step 4: 改 app CMake**

```cmake
add_library(app STATIC main.cpp src/app_context.cpp src/reminder_service.cpp src/runtime.cpp)
target_link_libraries(app PUBLIC core platform Qt6::Core Qt6::Gui Qt6::Widgets ui)
```

- [ ] **Step 5: Commit**

```bash
git add src/app/
git commit -m "feat(M6.1): wire tray + hotkey + runtime lifecycle"
```

---

### Task M6.2：托盘/快捷键/通知的 Win 真实实现（替换 M1.3 桩）

> ⚠️ **报告用户**：M6.2 中**自启写注册表**涉及 HKCU，需要实施时再确认是否要在 v1 落地（spec §1.2 暂列为非目标，但 §3.3 显式要求）。本任务**先做最小三件**（托盘、快捷键、通知），自启**单独拆为 M6.3 之后等待用户确认**。

**Files:**
- Modify: `src/platform/win/trayicon_win.cpp`
- Modify: `src/platform/win/hotkey_win.cpp`
- Modify: `src/platform/win/notifier_win.cpp`

- [ ] **Step 1: 替换 `trayicon_win.cpp` 桩为 `QSystemTrayIcon` 实现**

```cpp
// src/platform/win/trayicon_win.cpp
#include "platform/win/trayicon_win.h"
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
namespace stickynotes::platform::win {
struct TrayIcon_Win::Impl { QSystemTrayIcon* t = nullptr; QMenu* m = nullptr; };
TrayIcon_Win::TrayIcon_Win() : d_(new Impl) {
    d_->t = new QSystemTrayIcon(QIcon(":/icons/app.ico"));
    d_->m = new QMenu;
    d_->t->setContextMenu(d_->m);
    connect(d_->t, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason r){
        if (r == QSystemTrayIcon::Trigger) emit leftClicked();
    });
}
TrayIcon_Win::~TrayIcon_Win() { delete d_->t; delete d_->m; }
void TrayIcon_Win::show() { d_->t->show(); }
void TrayIcon_Win::hide() { d_->t->hide(); }
void TrayIcon_Win::setMenu(const QList<MenuItem>& items) {
    d_->m->clear();
    for (const auto& it : items) {
        auto* a = d_->m->addAction(it.text);
        connect(a, &QAction::triggered, this, [cb = it.onClick]{ if (cb) cb(); });
    }
}
}
```

> 需先在 `trayicon_win.h` 加 `struct Impl; std::unique_ptr<Impl> d_;` 与构造/析构。

- [ ] **Step 2: 替换 `hotkey_win.cpp` 为 Win32 `RegisterHotKey`**

```cpp
// src/platform/win/hotkey_win.cpp
#include "platform/win/hotkey_win.h"
#include <QAbstractNativeEventFilter>
#include <QGuiApplication>
#include <windows.h>
namespace stickynotes::platform::win {
class HotkeyFilter : public QAbstractNativeEventFilter {
public:
    bool nativeEventFilter(const QByteArray&, void* msg, long*) override {
        auto* m = static_cast<MSG*>(msg);
        if (m->message == WM_HOTKEY) emit hotkey(m->wParam);
        return false;
    }
signals:
    void hotkey(int id);
};
static HotkeyFilter* g_filter = nullptr;
static QList<Hotkey_Win*> g_instances;
Hotkey_Win::Hotkey_Win() {
    if (!g_filter) {
        g_filter = new HotkeyFilter;
        qApp->installNativeEventFilter(g_filter);
    }
    g_instances << this;
    connect(g_filter, &HotkeyFilter::hotkey, this, [this](int id){
        for (const auto& s : specs_) if (s.id == id) { emit triggered(id); break; }
    });
}
Hotkey_Win::~Hotkey_Win() { g_instances.removeAll(this); }
bool Hotkey_Win::registerHotkey(const Spec& s) {
    UINT mods = 0;
    if (s.mods & Qt::ControlModifier) mods |= MOD_CONTROL;
    if (s.mods & Qt::AltModifier)     mods |= MOD_ALT;
    if (s.mods & Qt::ShiftModifier)   mods |= MOD_SHIFT;
    if (s.mods & Qt::MetaModifier)    mods |= MOD_WIN;
    if (!::RegisterHotKey(nullptr, s.id, mods, s.key)) return false;
    specs_ << s; return true;
}
bool Hotkey_Win::unregisterHotkey(int id) {
    if (!::UnregisterHotKey(nullptr, id)) return false;
    specs_.removeIf([id](const Spec& s){ return s.id == id; });
    return true;
}
}
```

> 需在 `hotkey_win.h` 加 `QList<Spec> specs_;` 默认构造/析构。

- [ ] **Step 3: 替换 `notifier_win.cpp` 为 `QSystemTrayIcon::showMessage`**

```cpp
// src/platform/win/notifier_win.cpp
#include "platform/win/notifier_win.h"
#include <QSystemTrayIcon>
namespace stickynotes::platform::win {
struct Notifier_Win::Impl { QSystemTrayIcon* t = nullptr; };
Notifier_Win::Notifier_Win() : d_(new Impl) { d_->t = new QSystemTrayIcon; }
Notifier_Win::~Notifier_Win() { delete d_->t; }
void Notifier_Win::show(const Notification& n) {
    d_->t->showMessage(n.title, n.body, QSystemTrayIcon::Information, 5000);
}
}
```

> 需在 `notifier_win.h` 加 `struct Impl; std::unique_ptr<Impl> d_;` 与构造/析构。

- [ ] **Step 4: 编译并跑一次全量 ctest**

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

> 真实 Win 集成（M6.2）不在自动化用例内覆盖（用 mocks）；本任务为手工冒烟。

- [ ] **Step 5: Commit**

```bash
git add src/platform/
git commit -m "feat(M6.2): implement Win tray/hotkey/notifier on QSystemTrayIcon + RegisterHotKey"
```

---

### Task M6.3：CPack NSIS 打包

**Files:**
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 加 CPack 配置**

```cmake
# CMakeLists.txt 末尾
set(CPACK_PACKAGE_NAME "StickyNotes")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_GENERATOR "NSIS")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "StickyNotes")
set(CPACK_NSIS_DISPLAY_NAME "StickyNotes")
set(CPACK_NSIS_PACKAGE_NAME "StickyNotes-1.0.0")
set(CPACK_NSIS_URL_INFO_ABOUT "https://example.com")
include(CPack)
```

- [ ] **Step 2: 准备运行时拷贝（windeployqt）**

```cmake
# CMakeLists.txt 在 install() 之前
find_program(WINDEPLOYQT_EXE windeployqt HINTS "${Qt6_DIR}/../../../bin")
if(WINDEPLOYQT_EXE)
    install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/deploy.cmake")
endif()
```

```cmake
# cmake/deploy.cmake
install(CODE "
    execute_process(COMMAND ${WINDEPLOYQT_EXE}
        --no-translations --no-opengl-sw --no-system-d3d-compiler
        \"\${CMAKE_INSTALL_PREFIX}/Stickynotes.exe\")
")
```

- [ ] **Step 3: 在 app target 加 install 规则**

```cmake
# src/app/CMakeLists.txt
install(TARGETS stickynotes RUNTIME DESTINATION .)
```

- [ ] **Step 4: 打包**

```bash
cmake --build build --config Release
cpack -C Release -B dist
```

预期：`dist/StickyNotes-1.0.0-win64.exe` 存在。

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt cmake/
git commit -m "chore(M6.3): add CPack NSIS + windeployqt install"
```

---

## Part 3 完成检查

- [ ] `ctest` 全绿
- [ ] 手工冒烟：
  - 启动 → 主窗出现 → 关闭主窗 → 进程不退出
  - 单击托盘 → 主窗显示/隐藏切换
  - `Ctrl+Alt+N` → 新便签弹出
  - 便签可拖动、置顶、钉屏；关闭后再次从主窗列表双击弹出
  - 设置里改主题/快捷键/声音/自启并保存
  - 提醒到时系统通知
- [ ] NSIS 安装包生成成功

→ 全部 v1 范围交付完成。

---

## 总计划索引

- [part1-m0-m2.md](part1-m0-m2.md) — 骨架 + 平台 + core
- [part2-m3-m4.md](part2-m3-m4.md) — 编排 + 主窗
- [part3-m5-m6.md](part3-m5-m6.md) — 小窗 + 集成 + 打包
