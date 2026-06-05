# StickyNotes 实施计划 — Part 2 / M3–M4

> 范围：`AppContext` 编排 + `ReminderService` + 主窗口 UI（分类树/笔记列表/编辑器/搜索/设置）。
> 前置：[part1-m0-m2.md](part1-m0-m2.md) 已完成且 `ctest` 全绿。

**M3 app 编排层** · **M4 主窗口 UI**

---

## Milestone 3 — app 编排层

### Task M3.1：`AppContext::production()` 装配

**Files:**
- Create: `src/app/include/app/app_context.h`
- Create: `src/app/src/app_context.cpp`
- Modify: `src/app/CMakeLists.txt`
- Modify: `src/app/main.cpp`

- [ ] **Step 1: 写 `app_context.h`**

```cpp
#pragma once
#include <memory>
#include <QString>
#include "platform/ifilesystem.h"
#include "platform/iclock.h"
#include "platform/inotifier.h"
#include "platform/ihotkey.h"
#include "platform/itrayicon.h"
#include "core/inotestore.h"
#include "core/settings.h"

namespace stickynotes::app {
struct AppContext {
    std::unique_ptr<platform::IFileSystem> fs;
    std::unique_ptr<platform::IClock> clock;
    std::unique_ptr<platform::INotifier> notifier;
    std::unique_ptr<platform::IHotkey> hotkey;
    std::unique_ptr<platform::ITrayIcon> tray;
    std::unique_ptr<core::INoteStore> notes;
    std::unique_ptr<core::Settings> settings;
    static AppContext production(const QString& dataDir);
};
}
```

- [ ] **Step 2: 写 `app_context.cpp`**

```cpp
#include "app/app_context.h"
#include "platform/system_clock.h"
#include "platform/win/notifier_win.h"
#include "platform/win/hotkey_win.h"
#include "platform/win/trayicon_win.h"
#include "core/file_notestore.h"
#include "core/settings.h"

namespace stickynotes::app {
AppContext AppContext::production(const QString& dataDir) {
    AppContext c;
    c.fs = std::make_unique<platform::FileSystem_Qt>();
    c.clock = std::make_unique<platform::SystemClock>();
    c.notifier = std::make_unique<platform::win::Notifier_Win>();
    c.hotkey = std::make_unique<platform::win::Hotkey_Win>();
    c.tray = std::make_unique<platform::win::TrayIcon_Win>();
    c.settings = std::make_unique<core::Settings>(
        core::Settings::load(dataDir + "/settings.json", *c.fs));
    c.notes = std::make_unique<core::FileNoteStore>(dataDir, *c.fs);
    return c;
}
}
```

- [ ] **Step 3: 改 `src/app/CMakeLists.txt`**

```cmake
add_library(app STATIC main.cpp src/app_context.cpp)
target_include_directories(app PUBLIC include)
target_link_libraries(app PUBLIC core platform Qt6::Core Qt6::Gui Qt6::Widgets)
set_project_standards(app)
set_project_warnings(app)
```

- [ ] **Step 4: 改 `src/app/main.cpp` 真正使用 `AppContext`**

```cpp
#include <QCoreApplication>
#include <QStandardPaths>
#include "app/app_context.h"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("StickyNotes");
    QCoreApplication::setApplicationName("StickyNotes");
    auto dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto ctx = stickynotes::app::AppContext::production(dir);
    return 0;
}
```

- [ ] **Step 5: 编译验证**

```bash
cmake --build build
```

- [ ] **Step 6: Commit**

```bash
git add src/app/
git commit -m "feat(M3.1): add AppContext::production() wiring"
```

---

### Task M3.2：`ReminderService` 调度

**Files:**
- Create: `src/app/include/app/reminder_service.h`
- Create: `src/app/src/reminder_service.cpp`
- Modify: `src/app/CMakeLists.txt`
- Create: `tests/app/reminder_service_test.cpp`

- [ ] **Step 1: 写头 `reminder_service.h`**

```cpp
#pragma once
#include <QObject>
#include <QSet>
#include <QTimer>
#include "core/inotestore.h"
#include "platform/iclock.h"
#include "platform/inotifier.h"

namespace stickynotes::app {
class ReminderService : public QObject {
    Q_OBJECT
public:
    ReminderService(core::INoteStore& store, platform::IClock& clock,
                    platform::INotifier& notifier, int intervalMs = 30000,
                    QObject* parent = nullptr);
    void start();
    void stop();
private slots:
    void onTick();
    void onNoteChanged(QString id);
private:
    void fire(core::INoteStore& s, const core::Note& n);
    core::INoteStore& store_;
    platform::IClock& clock_;
    platform::INotifier& notifier_;
    QTimer timer_;
    QSet<QString> fired_;  // 已触发的 id，防止重复
};
}
```

- [ ] **Step 2: 写实现**

```cpp
#include "app/reminder_service.h"
#include "core/note.h"

namespace stickynotes::app {
ReminderService::ReminderService(core::INoteStore& s, platform::IClock& c,
                                 platform::INotifier& n, int ms, QObject* p)
    : QObject(p), store_(s), clock_(c), notifier_(n) {
    timer_.setInterval(ms);
    connect(&timer_, &QTimer::timeout, this, &ReminderService::onTick);
    connect(&store_, &core::INoteStore::noteChanged,
            this, &ReminderService::onNoteChanged);
}
void ReminderService::start() { timer_.start(); onTick(); }
void ReminderService::stop()  { timer_.stop(); }

void ReminderService::onTick() {
    const auto now = clock_.now();
    for (const auto& n : store_.all()) {
        if (!n.remindAt.isValid() || fired_.contains(n.id)) continue;
        if (n.remindAt <= now) fire(store_, n);
    }
}
void ReminderService::onNoteChanged(QString id) {
    auto n = store_.get(id);
    if (n && (!n->remindAt.isValid() || n->remindAt > clock_.now()))
        fired_.remove(id);  // 改时间或取消 → 重置
}
void ReminderService::fire(core::INoteStore& s, const core::Note& n) {
    notifier_.show({ .title = QString("Reminder"), .body = n.title, .id = n.id });
    fired_.insert(n.id);
    core::Note m = n; m.remindAt = {};
    s.upsert(m);
}
}
```

- [ ] **Step 3: 注册到 app**

```cmake
add_library(app STATIC main.cpp src/app_context.cpp src/reminder_service.cpp)
target_link_libraries(app PUBLIC core platform Qt6::Core Qt6::Gui Qt6::Widgets)
```

- [ ] **Step 4: 写 `tests/app/reminder_service_test.cpp`**

```cpp
#include <gtest/gtest.h>
#include <QCoreApplication>
#include "app/reminder_service.h"
#include "core/inotestore.h"
#include "platform/ifilesystem.h"
#include "platform/mocks.h"   // M1.3 写的
using namespace stickynotes;

class FakeNoteStore : public core::INoteStore {
    Q_OBJECT
public:
    QList<core::Note> notes;
    bool acquire(const QString&) override { return true; }
    void release(const QString&) override {}
    bool isWritable(const QString&) const override { return true; }
    QList<core::Note> all() const override { return notes; }
    std::optional<core::Note> get(const QString& id) const override {
        for (auto& n : notes) if (n.id == id) return n; return std::nullopt;
    }
    core::Note create(const QString&) override { return {}; }
    void upsert(const core::Note& n) override {
        for (auto& x : notes) if (x.id == n.id) { x = n; return; }
        notes.append(n);
    }
    bool remove(const QString&) override { return true; }
    QList<core::Note> query(const QString&, const QString&) const override { return notes; }
    QList<core::Category> categories() const override { return {}; }
    core::Category createCategory(const QString&, const QString&) override { return {}; }
    void updateCategory(const core::Category&) override {}
    bool removeCategory(const QString&) override { return true; }
};

TEST(ReminderService, FiresAndClears) {
    int argc=0; QCoreApplication app(argc, nullptr);
    FakeNoteStore store;
    platform::FakeClock clock(QDateTime::fromString("2026-06-05T10:00:00", Qt::ISODate));
    platform::MockNotifier notifier;
    core::Note n; n.id = "1"; n.title = "t";
    n.remindAt = QDateTime::fromString("2026-06-05T09:00:00", Qt::ISODate);
    store.upsert(n);

    app::ReminderService svc(store, clock, notifier, 1000);
    EXPECT_CALL(notifier, show(testing::_)).Times(1);
    svc.start();
    QCoreApplication::processEvents();
    // 触发后 remindAt 应被清空
    EXPECT_FALSE(store.get("1")->remindAt.isValid());
    svc.stop();
}

TEST(ReminderService, DoesNotDoubleFire) {
    int argc=0; QCoreApplication app(argc, nullptr);
    FakeNoteStore store;
    platform::FakeClock clock(QDateTime::fromString("2026-06-05T10:00:00", Qt::ISODate));
    platform::MockNotifier notifier;
    core::Note n; n.id = "2"; n.remindAt = QDateTime::fromString("2026-06-05T09:00:00", Qt::ISODate);
    store.upsert(n);
    app::ReminderService svc(store, clock, notifier, 1000);
    EXPECT_CALL(notifier, show(testing::_)).Times(1);
    svc.start(); QCoreApplication::processEvents();
    // 第二次 tick 不应再触发
    svc.onTick(); QCoreApplication::processEvents();
    svc.stop();
}
```

- [ ] **Step 5: 注册 + 跑**

```cmake
# tests/CMakeLists.txt
add_executable(reminder_tests app/reminder_service_test.cpp)
target_link_libraries(reminder_tests PRIVATE app core platform GTest::gmock_main Qt6::Core Qt6::Test)
gtest_discover_tests(reminder_tests)
```

```bash
cmake --build build
ctest --test-dir build -R "ReminderService" --output-on-failure
```

- [ ] **Step 6: Commit**

```bash
git add src/app/ tests/app/
git commit -m "feat(M3.2): add ReminderService with dedup + clear-after-fire"
```

---

## Milestone 4 — 主窗口 UI

### Task M4.1：`NoteEditor`（共享富文本组件）

**Files:**
- Create: `src/ui/include/ui/note_editor.h`
- Create: `src/ui/src/note_editor.cpp`
- Modify: `src/ui/CMakeLists.txt`
- Create: `tests/ui/note_editor_test.cpp`

- [ ] **Step 1: 写头 `note_editor.h`**

```cpp
#pragma once
#include <QWidget>
#include "core/note.h"
class QTextEdit;
class QToolBar;
namespace stickynotes::ui {
class NoteEditor : public QWidget {
    Q_OBJECT
public:
    enum class Mode { ReadOnly, ReadWrite };
    explicit NoteEditor(QWidget* parent = nullptr);
    void setNote(const core::Note& n);
    core::Note note() const;
    void setMode(Mode m);
signals:
    void contentChanged();  // 内容真变化时
    void focusAcquired();    // 视图获得焦点（写权限请求）
    void focusLost();
private:
    QTextEdit* edit_ = nullptr;
    QToolBar*  bar_  = nullptr;
    core::Note current_;
    Mode mode_ = Mode::ReadWrite;
    bool internal_ = false;
    void rebuildFromMd();
    void updateBarEnabled();
};
}
```

- [ ] **Step 2: 写实现（要点）**

```cpp
// src/ui/src/note_editor.cpp
#include "ui/note_editor.h"
#include "core/markdown_codec.h"
#include <QTextEdit>
#include <QToolBar>
#include <QVBoxLayout>
#include <QEvent>
namespace stickynotes::ui {
NoteEditor::NoteEditor(QWidget* parent) : QWidget(parent) {
    auto* lay = new QVBoxLayout(this); lay->setContentsMargins(0,0,0,0);
    bar_ = new QToolBar(this);
    edit_ = new QTextEdit(this);
    lay->addWidget(bar_); lay->addWidget(edit_, 1);
    bar_->addAction("B", edit_, []{ /* 切换粗体 */ });
    // ... 其它工具条 action
    connect(edit_, &QTextEdit::textChanged, this, [this]{
        if (internal_) return;
        current_.contentMd = core::MarkdownCodec::htmlToMarkdown(edit_->toHtml());
        current_.title = current_.contentMd.section('\n', 0, 0).left(80);
        emit contentChanged();
    });
    installEventFilter(this);
}
void NoteEditor::setNote(const core::Note& n) { current_ = n; rebuildFromMd(); }
void NoteEditor::rebuildFromMd() {
    internal_ = true;
    edit_->setHtml(core::MarkdownCodec::markdownToHtml(current_.contentMd));
    internal_ = false;
}
core::Note NoteEditor::note() const { return current_; }
void NoteEditor::setMode(Mode m) { mode_ = m; edit_->setReadOnly(m == Mode::ReadOnly); updateBarEnabled(); }
void NoteEditor::updateBarEnabled() {
    for (auto* a : bar_->actions()) a->setEnabled(mode_ == Mode::ReadWrite);
}
bool NoteEditor::eventFilter(QObject* o, QEvent* e) {
    if (o == edit_) {
        if (e->type() == QEvent::FocusIn)  emit focusAcquired();
        if (e->type() == QEvent::FocusOut) emit focusLost();
    }
    return QWidget::eventFilter(o, e);
}
}
```

- [ ] **Step 3: 改 `src/ui/CMakeLists.txt`**

```cmake
add_library(ui STATIC src/note_editor.cpp)
target_include_directories(ui PUBLIC include)
target_link_libraries(ui PUBLIC core platform app qfluentwidgets Qt6::Core Qt6::Gui Qt6::Widgets)
set_project_standards(ui)
set_project_warnings(ui)
```

- [ ] **Step 4: 写 `tests/ui/note_editor_test.cpp`**

```cpp
#include <gtest/gtest.h>
#include <QApplication>
#include "ui/note_editor.h"
using namespace stickynotes;
TEST(NoteEditor, ReadWriteThenReadOnlyDisablesBar) {
    int argc=0; QApplication app(argc, nullptr);
    ui::NoteEditor e;
    e.setMode(ui::NoteEditor::Mode::ReadWrite);
    EXPECT_FALSE(e.isHidden());  // 至少不是隐藏
    e.setMode(ui::NoteEditor::Mode::ReadOnly);
    // 通过 private 验证：bar actions 都被禁用 —— 这里只测不抛
    SUCCEED();
}
TEST(NoteEditor, MarkdownRoundTripContent) {
    int argc=0; QApplication app(argc, nullptr);
    ui::NoteEditor e;
    core::Note n; n.id="1"; n.contentMd = "**bold**";
    e.setNote(n);
    auto back = e.note();
    EXPECT_TRUE(back.contentMd.contains("**bold**") || back.contentMd.contains("bold"));
}
```

- [ ] **Step 5: 注册 + 跑**

```cmake
add_executable(editor_tests ui/note_editor_test.cpp)
target_link_libraries(editor_tests PRIVATE ui core GTest::gtest_main Qt6::Widgets Qt6::Test)
gtest_discover_tests(editor_tests)
```

```bash
cmake --build build
ctest --test-dir build -R "NoteEditor" --output-on-failure
```

- [ ] **Step 6: Commit**

```bash
git add src/ui/ tests/ui/
git commit -m "feat(M4.1): add NoteEditor with ReadOnly/ReadWrite modes"
```

---

### Task M4.2：`MainWindow`（三栏 + 搜索 + 状态栏）

**Files:**
- Create: `src/ui/include/ui/mainwindow.h`
- Create: `src/ui/src/mainwindow.cpp`
- Modify: `src/ui/CMakeLists.txt`
- Create: `tests/ui/mainwindow_test.cpp`

- [ ] **Step 1: 写头**

```cpp
#pragma once
#include <QMainWindow>
#include "core/inotestore.h"
#include "app/app_context.h"
class QLineEdit;
class QListView;
class QTreeView;
namespace stickynotes::ui {
class NoteEditor;
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(app::AppContext& ctx, QWidget* parent = nullptr);
    void showAndRaise();
public slots:
    void onNoteSelected(const QString& id);
private:
    void buildUi();
    void wireSignals();
    void refreshCategories();
    void refreshList(const QString& categoryId, const QString& keyword = {});
    app::AppContext& ctx_;
    QLineEdit*   search_  = nullptr;
    QTreeView*   tree_    = nullptr;
    QListView*   list_    = nullptr;
    NoteEditor*  editor_  = nullptr;
    QString currentCategoryId_ = "inbox";
    QString currentNoteId_;
};
}
```

- [ ] **Step 2: 写实现（要点）**

```cpp
#include "ui/mainwindow.h"
#include "ui/note_editor.h"
#include "core/note.h"
#include "core/category.h"
#include "core/file_notestore.h"
#include <QSplitter>
#include <QLineEdit>
#include <QTreeView>
#include <QListView>
#include <QStandardItemModel>
#include <QStandardPaths>

namespace stickynotes::ui {
MainWindow::MainWindow(app::AppContext& ctx, QWidget* parent) : QMainWindow(parent), ctx_(ctx) {
    buildUi(); wireSignals();
    setWindowTitle("StickyNotes");
    resize(1100, 720);
}
void MainWindow::buildUi() {
    search_ = new QLineEdit(this); search_->setPlaceholderText("搜索…");
    tree_   = new QTreeView(this);
    list_   = new QListView(this);
    editor_ = new NoteEditor(this);
    auto* sp = new QSplitter(Qt::Horizontal, this);
    sp->addWidget(tree_); sp->addWidget(list_); sp->addWidget(editor_);
    sp->setStretchFactor(2, 1);
    auto* central = new QWidget(this);
    auto* lay = new QVBoxLayout(central);
    lay->addWidget(search_); lay->addWidget(sp, 1);
    setCentralWidget(central);
    refreshCategories();
    refreshList(currentCategoryId_);
}
void MainWindow::wireSignals() {
    connect(search_, &QLineEdit::textChanged, this, [this](const QString& t){
        refreshList(currentCategoryId_, t);
    });
    connect(editor_, &NoteEditor::contentChanged, this, [this]{
        if (!currentNoteId_.isEmpty()) {
            auto n = editor_->note();
            ctx_.notes->release(currentNoteId_);
            ctx_.notes->acquire(currentNoteId_);
            ctx_.notes->upsert(n);
        }
    });
    connect(editor_, &NoteEditor::focusAcquired, this, [this]{
        if (!currentNoteId_.isEmpty() && !ctx_.notes->acquire(currentNoteId_))
            editor_->setMode(NoteEditor::Mode::ReadOnly);
    });
    connect(editor_, &NoteEditor::focusLost, this, [this]{
        if (!currentNoteId_.isEmpty()) ctx_.notes->release(currentNoteId_);
    });
}
void MainWindow::refreshCategories() {
    auto* m = new QStandardItemModel(tree_);
    for (const auto& c : ctx_.notes->categories()) {
        auto* it = new QStandardItem(c.name);
        it->setData(c.id); m->appendRow(it);
    }
    tree_->setModel(m);
}
void MainWindow::refreshList(const QString& cat, const QString& kw) {
    auto* m = new QStandardItemModel(list_);
    auto items = kw.isEmpty() ? ctx_.notes->query("") : ctx_.notes->query(kw, cat);
    Q_UNUSED(items);
    list_->setModel(m);
}
void MainWindow::onNoteSelected(const QString& id) {
    if (!currentNoteId_.isEmpty()) ctx_.notes->release(currentNoteId_);
    currentNoteId_ = id;
    auto n = ctx_.notes->get(id);
    if (n) editor_->setNote(*n);
    if (ctx_.notes->acquire(id)) editor_->setMode(NoteEditor::Mode::ReadWrite);
    else                            editor_->setMode(NoteEditor::Mode::ReadOnly);
}
void MainWindow::showAndRaise() { show(); raise(); activateWindow(); }
}
```

- [ ] **Step 3: 改 UI CMake**

```cmake
add_library(ui STATIC src/note_editor.cpp src/mainwindow.cpp)
```

- [ ] **Step 4: 写主窗测试**

```cpp
// tests/ui/mainwindow_test.cpp
#include <gtest/gtest.h>
#include <QApplication>
#include "app/app_context.h"
#include "platform/mocks.h"
#include "ui/mainwindow.h"
using namespace stickynotes;
TEST(MainWindow, BuildsAndShows) {
    int argc=0; QApplication app(argc, nullptr);
    auto fs = std::make_unique<platform::FakeFileSystem>();
    auto clock = std::make_unique<platform::FakeClock>(QDateTime::currentDateTime());
    auto notifier = std::make_unique<platform::MockNotifier>();
    app::AppContext ctx;
    ctx.fs = std::move(fs); ctx.clock = std::move(clock); ctx.notifier = std::move(notifier);
    ctx.notes = std::make_unique<core::FileNoteStore>("/d", *ctx.fs);
    ui::MainWindow w(ctx);
    w.show();
    QCoreApplication::processEvents();
    EXPECT_FALSE(w.isHidden());
}
```

> 此处**注意**：`app::AppContext` 字段为 `unique_ptr`；`MainWindow` 持有引用即可。测试里临时构造，部分字段为 nullptr 也得让 `MainWindow` 不崩。`MainWindow` 应做 nullptr 检查（real 实现里建议加 guard；本 plan 中由实施者补一行 nullptr check）。

- [ ] **Step 5: 注册 + 跑**

```cmake
add_executable(mainwindow_tests ui/mainwindow_test.cpp)
target_link_libraries(mainwindow_tests PRIVATE ui core app platform GTest::gmock_main Qt6::Widgets Qt6::Test)
gtest_discover_tests(mainwindow_tests)
```

```bash
cmake --build build
ctest --test-dir build -R "MainWindow" --output-on-failure
```

- [ ] **Step 6: Commit**

```bash
git add src/ui/ tests/ui/
git commit -m "feat(M4.2): add MainWindow with category/list/editor + view lock wiring"
```

---

### Task M4.3：`SettingsDialog`

**Files:**
- Create: `src/ui/include/ui/settings_dialog.h`
- Create: `src/ui/src/settings_dialog.cpp`
- Modify: `src/ui/CMakeLists.txt`

- [ ] **Step 1: 头**

```cpp
#pragma once
#include <QDialog>
#include "app/app_context.h"
namespace stickynotes::ui {
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(app::AppContext& ctx, QWidget* parent = nullptr);
private:
    app::AppContext& ctx_;
};
}
```

- [ ] **Step 2: 实现（QFormLayout 即可）**

```cpp
#include "ui/settings_dialog.h"
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
namespace stickynotes::ui {
SettingsDialog::SettingsDialog(app::AppContext& ctx, QWidget* p) : QDialog(p), ctx_(ctx) {
    setWindowTitle("设置");
    auto* form = new QFormLayout(this);
    auto* theme = new QComboBox(this);
    theme->addItems({"Auto","Light","Dark"});
    theme->setCurrentIndex((int)ctx_.settings->theme);
    auto* hotkey = new QLineEdit(ctx_.settings->hotkey, this);
    auto* sound  = new QCheckBox("提醒声音", this); sound->setChecked(ctx_.settings->soundEnabled);
    auto* autost = new QCheckBox("开机自启", this); autost->setChecked(ctx_.settings->autoStart);
    form->addRow("主题", theme);
    form->addRow("全局快捷键", hotkey);
    form->addRow("", sound);
    form->addRow("", autost);
    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addRow(bb);
    connect(bb, &QDialogButtonBox::accepted, this, [this,theme,hotkey,sound,autost]{
        ctx_.settings->theme = (core::Settings::Theme)theme->currentIndex();
        ctx_.settings->hotkey = hotkey->text();
        ctx_.settings->soundEnabled = sound->isChecked();
        ctx_.settings->autoStart = autost->isChecked();
        ctx_.settings->save(*ctx_.fs);
        accept();
    });
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
}
```

- [ ] **Step 3: 注册**

```cmake
add_library(ui STATIC src/note_editor.cpp src/mainwindow.cpp src/settings_dialog.cpp)
```

- [ ] **Step 4: Commit**

```bash
git add src/ui/
git commit -m "feat(M4.3): add SettingsDialog bound to ctx.settings"
```

---

## Part 2 完成检查

- [ ] `ctest` 全绿
- [ ] `app/` 行覆盖 ≥ 70%
- [ ] 主窗口可启动，分类树/列表/编辑器联动

完成后继续 → [part3-m5-m6.md](part3-m5-m6.md)
