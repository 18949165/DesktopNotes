#include "ui/mainwindow.h"
#include "ui/note_editor.h"
#include "ui/stickywindow.h"
#include "ui/settings_dialog.h"
#include "core/note.h"
#include "core/category.h"
#include "app/app_context.h"

#include <ElaListView.h>
#include <ElaMenu.h>
#include <ElaMenuBar.h>
#include <ElaToolBar.h>
#include <ElaLineEdit.h>
#include <ElaMessageBar.h>
#include <ElaText.h>
#include <ElaPushButton.h>
#include <ElaTheme.h>

#include <QStandardItemModel>
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QTextEdit>
#include <QFileDialog>
#include <QStatusBar>
#include <QLabel>
#include <QShortcut>
#include <QKeySequence>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QButtonGroup>
#include <QApplication>
#include <QCloseEvent>

namespace stickynotes::ui {
MainWindow::MainWindow(app::AppContext& ctx, QWidget* parent)
    : ElaWindow(parent), ctx_(ctx) {
    setWindowTitle("StickyNotes  便签管理");
    resize(1280, 800);
    setIsNavigationBarEnable(true);
    setNavigationBarWidth(220);
    setIsCentralStackedWidgetTransparent(true);

    // 视觉分区：左侧导航栏（更深） vs 中央内容区（稍亮） vs 列表（再深一档），
    // 左侧栏和便签栏之间有明显竖向分隔
    setStyleSheet(
        "ElaNavigationBar#ElaNavigationBar{background-color:rgba(20,20,22,200);"
        "                                border-right:1px solid rgba(127,127,127,80);}"
        "QWidget#centralWidget{background-color:palette(window);"
        "                     border-left:1px solid rgba(127,127,127,80);}"
        "ElaListView{background-color:rgba(255,255,255,8);"
        "            border-right:1px solid rgba(127,127,127,60);"
        "            border-radius:6px;}"
        "ElaListView::item{min-height:40px;max-height:40px;"
        "                  border-bottom:1px solid rgba(127,127,127,60);"
        "                  padding:6px 10px;}"
        "ElaListView::item:hover{background:rgba(127,127,127,30);}"
        "ElaListView::item:selected{background:rgba(98,124,186,80);}"
    );

    // 主题由 ElaWindow AppBar 主题按钮控制；Settings 不持久化主题
    if (eTheme->getThemeMode() != ElaThemeType::Dark) {
        eTheme->setThemeMode(ElaThemeType::Dark);
    }

    buildUi();
    wireSignals();
    refreshCategories();
    refreshList();
    updateStatusBar();
}

void MainWindow::buildUi() {
    buildMenuBar();
    buildToolBar();
    buildBody();
    buildStatusBar();

    setWindowButtonFlag(ElaAppBarType::ThemeChangeButtonHint, true);
    setWindowButtonFlag(ElaAppBarType::MinimizeButtonHint, true);
    setWindowButtonFlag(ElaAppBarType::MaximizeButtonHint, true);
    setWindowButtonFlag(ElaAppBarType::CloseButtonHint, true);

    // 隐藏 ElaWindow 自带的用户卡片（避免显示默认 Ela Tool 信息）
    setUserInfoCardVisible(false);

    auto* titleText = new ElaText("StickyNotes", 18, this);
    titleText->setTextStyle(ElaTextType::TextStyle::Subtitle);
    setCustomWidget(ElaAppBarType::LeftArea, titleText);
}

void MainWindow::buildMenuBar() {
    auto* mb = new ElaMenuBar(this);
    auto* fileMenu = mb->addMenu("文件(&F)");
    auto* newAct = fileMenu->addElaIconAction(ElaIconType::Note, "新建便签", QKeySequence("Ctrl+N"));
    auto* exportAct = fileMenu->addElaIconAction(ElaIconType::Share, "导出全部…");
    auto* importAct = fileMenu->addElaIconAction(ElaIconType::FolderTree, "导入便签…");
    fileMenu->addSeparator();
    fileMenu->addAction("退出", qApp, &QApplication::quit);

    auto* editMenu = mb->addMenu("编辑(&E)");
    auto* delAct = editMenu->addElaIconAction(ElaIconType::TrashCan, "删除当前便签 (移到废纸篓)", QKeySequence("Delete"));
    auto* purActMenu = editMenu->addElaIconAction(ElaIconType::Eraser, "永久删除 (Shift+Del)", QKeySequence("Shift+Delete"));

    auto* viewMenu = mb->addMenu("视图(&V)");
    viewMenu->addElaIconAction(ElaIconType::Sun, "切换主题");
    auto* themeAct = viewMenu->actions().value(0);

    auto* helpMenu = mb->addMenu("帮助(&H)");
    helpMenu->addAction("关于", this, [this]() {
        ElaMessageBar::information(ElaMessageBarType::Top, "关于",
            "StickyNotes v1.0\n基于 Qt6 + ElaWidgetTools", 4000, this);
    });

    setMenuBar(mb);

    connect(newAct, &QAction::triggered, this, &MainWindow::onNewNote);
    connect(delAct, &QAction::triggered, this, &MainWindow::onDeleteNote);
    connect(purActMenu, &QAction::triggered, this, &MainWindow::onPermanentDeleteNote);
    connect(exportAct, &QAction::triggered, this, &MainWindow::onExportNotes);
    connect(importAct, &QAction::triggered, this, &MainWindow::onImportNotes);
    connect(themeAct, &QAction::triggered, this, [this]() {
        eTheme->setThemeMode(eTheme->getThemeMode() == ElaThemeType::Light
                             ? ElaThemeType::Dark : ElaThemeType::Light);
        updateStatusBar();
    });
}

void MainWindow::buildToolBar() {
    toolBar_ = new ElaToolBar("主工具栏", this);
    toolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto* newAct = toolBar_->addElaIconAction(ElaIconType::Note, "新建", QKeySequence("Ctrl+N"));
    auto* delAct = toolBar_->addElaIconAction(ElaIconType::TrashCan, "删除");
    delAct->setShortcut(QKeySequence("Delete"));
    auto* pinAct = toolBar_->addElaIconAction(ElaIconType::Star, "标记重要/取消重要");
    auto* restoreAct = toolBar_->addElaIconAction(ElaIconType::ArrowRotateLeft, "恢复");
    auto* purAct     = toolBar_->addElaIconAction(ElaIconType::Eraser, "永久删除");
    auto* remAct = toolBar_->addElaIconAction(ElaIconType::Bell, "提醒");
    auto* openAct = toolBar_->addElaIconAction(ElaIconType::Text, "弹出便签");
    toolBar_->addSeparator();
    auto* expAct = toolBar_->addElaIconAction(ElaIconType::Share, "导出");
    auto* impAct = toolBar_->addElaIconAction(ElaIconType::FolderTree, "导入");
    toolBar_->addSeparator();
    auto* setAct = toolBar_->addElaIconAction(ElaIconType::Sliders, "设置");

    addToolBar(Qt::TopToolBarArea, toolBar_);

    connect(newAct, &QAction::triggered, this, &MainWindow::onNewNote);
    connect(delAct, &QAction::triggered, this, &MainWindow::onDeleteNote);
    connect(restoreAct, &QAction::triggered, this, &MainWindow::onRestoreNote);
    connect(purAct, &QAction::triggered, this, &MainWindow::onPermanentDeleteNote);
    connect(pinAct, &QAction::triggered, this, [this]() {
        if (currentNoteId_.isEmpty()) return;
        auto n = ctx_.notes->get(currentNoteId_);
        if (!n) return;
        n->pinned = !n->pinned;
        ctx_.notes->upsert(*n);
        refreshList();
        ElaMessageBar::success(ElaMessageBarType::Top, "已更新",
            n->pinned ? "已标记为重要" : "已取消重要", 2000, this);
    });
    connect(remAct, &QAction::triggered, this, &MainWindow::onSetReminder);
    connect(openAct, &QAction::triggered, this, &MainWindow::onOpenSticky);
    connect(expAct, &QAction::triggered, this, &MainWindow::onExportNotes);
    connect(impAct, &QAction::triggered, this, &MainWindow::onImportNotes);
    connect(setAct, &QAction::triggered, this, &MainWindow::onShowSettings);
}

void MainWindow::buildBody() {
    // 中间容器：分类过滤 + 搜索 + 左右分栏
    auto* body = new QWidget(this);

    // 分类过滤按钮组
    catGroup_ = new QButtonGroup(this);
    catGroup_->setExclusive(true);
    auto* catBar = new QWidget(body);
    auto* catLay = new QHBoxLayout(catBar);
    catLay->setContentsMargins(0, 0, 0, 0);
    catLay->setSpacing(6);

    auto makeCatBtn = [&](const QString& text, const QString& id, ElaIconType::IconName icon) {
        auto* btn = new ElaPushButton(text, catBar);
        btn->setCheckable(true);
        btn->setIcon(QIcon()); // Ela 风格用文字即可
        btn->setFixedHeight(30);
        catGroup_->addButton(btn, catGroup_->buttons().size());
        btn->setProperty("catId", id);
        catLay->addWidget(btn);
        return btn;
    };
    makeCatBtn("📥  收件箱", "inbox",  ElaIconType::Broom);
    makeCatBtn("📅  今日",   "today",  ElaIconType::Calendar);
    makeCatBtn("⭐  重要",   "pinned", ElaIconType::Star);
    makeCatBtn("🗑  废纸篓", "trash",  ElaIconType::TrashCan);
    catLay->addStretch();
    if (auto* b = catGroup_->button(0)) b->setChecked(true);

    auto* search = new ElaLineEdit(body);
    search->setObjectName("searchEdit");
    search->setPlaceholderText("🔍  搜索标题或内容…");
    search->setIsClearButtonEnable(true);
    search->setFixedHeight(36);

    list_ = new ElaListView(body);
    list_->setIsTransparent(true);
    list_->setItemHeight(40);

    editor_ = new NoteEditor(body);

    auto* sp = new QSplitter(Qt::Horizontal, body);
    sp->addWidget(list_);
    sp->addWidget(editor_);
    sp->setStretchFactor(0, 1);
    sp->setStretchFactor(1, 3);
    sp->setSizes({320, 720});

    auto* lay = new QVBoxLayout(body);
    lay->setContentsMargins(12, 8, 12, 8);
    lay->setSpacing(8);
    lay->addWidget(catBar);
    lay->addWidget(search);
    lay->addWidget(sp, 1);

    editorPage_ = body;
    addCentralWidget(editorPage_);
    addPageNode("便签", editorPage_, ElaIconType::Notes);

    // 设置页：用一个普通容器 + 打开对话框按钮（ElaDialog 不能直接当 page）
    auto* settingsPage = new QWidget(this);
    auto* settingsLay = new QVBoxLayout(settingsPage);
    settingsLay->setContentsMargins(20, 20, 20, 20);
    settingsLay->setSpacing(12);
    auto* info = new ElaText("设置\n\n点击下方按钮打开设置对话框", 14, settingsPage);
    info->setAlignment(Qt::AlignCenter);
    auto* openBtn = new ElaPushButton("打开设置…", settingsPage);
    openBtn->setFixedHeight(40);
    connect(openBtn, &ElaPushButton::clicked, this, &MainWindow::onShowSettings);
    settingsLay->addStretch();
    settingsLay->addWidget(info);
    settingsLay->addWidget(openBtn);
    settingsLay->addStretch();
    addPageNode("设置", settingsPage, ElaIconType::Sliders);
}

void MainWindow::buildStatusBar() {
    auto* sb = statusBar();
    statusNoteCount_ = new QLabel("共 0 条便签", this);
    statusTheme_ = new QLabel("主题: 浅色", this);
    sb->addPermanentWidget(statusNoteCount_, 1);
    sb->addPermanentWidget(statusTheme_, 0);
}

void MainWindow::wireSignals() {
    // 搜索
    auto* search = findChild<ElaLineEdit*>("searchEdit");
    if (search) {
        connect(search, &ElaLineEdit::textChanged, this, [this](const QString&) {
            refreshList();
        });
    }

    // 列表
    if (list_) {
        connect(list_, &ElaListView::clicked, this, [this](const QModelIndex& idx) {
            onNoteSelected(idx.data(Qt::UserRole).toString());
        });
        connect(list_, &ElaListView::doubleClicked, this, [this](const QModelIndex& idx) {
            openStickyWindow(idx.data(Qt::UserRole).toString());
        });
    }

    // 编辑器
    if (editor_) {
        connect(editor_, &NoteEditor::contentChanged, this, [this]() {
            if (currentNoteId_.isEmpty()) return;
            auto n = ctx_.notes->get(currentNoteId_);
            if (!n) return;
            auto updated = editor_->note();
            updated.id = n->id;
            updated.categoryId = n->categoryId;
            updated.createdAt = n->createdAt;
            updated.updatedAt = QDateTime::currentDateTime();
            ctx_.notes->upsert(updated);
            refreshList();
        });
    }

    // 变更回调
    ctx_.notes->setNoteChangedCallback([this](const QString&) {
        refreshList();
        updateStatusBar();
    });
    ctx_.notes->setCategoryChangedCallback([this](const QString&) {
        refreshCategories();
    });

    // 分类过滤
    if (catGroup_) {
        connect(catGroup_, &QButtonGroup::idClicked, this, [this](int) {
            auto* btn = qobject_cast<QPushButton*>(catGroup_->checkedButton());
            if (btn) currentCategoryId_ = btn->property("catId").toString();
            refreshList();
        });
    }

    // 主题变化
    connect(eTheme, &ElaTheme::themeModeChanged, this, [this](ElaThemeType::ThemeMode) {
        updateStatusBar();
    });

    // AppBar 关闭按钮 -> 隐藏到托盘
    setIsDefaultClosed(false);
    connect(this, &ElaWindow::closeButtonClicked, this, [this]() {
        hide();
        ElaMessageBar::information(ElaMessageBarType::Top, "已隐藏",
            "便签已最小化到系统托盘，双击托盘图标可重新打开", 3000, this);
    });
}

void MainWindow::closeEvent(QCloseEvent* e) {
    // 默认不退出，仅隐藏
    if (e->spontaneous()) {
        e->ignore();
        hide();
    } else {
        e->accept();
    }
}

void MainWindow::refreshCategories() {
    // 内置分类不通过 INoteStore，由按钮组完成；自定义分类可追加（v1 暂未做）
}

void MainWindow::refreshList() {
    if (!list_) return;
    auto* m = new QStandardItemModel(list_);
    auto* search = findChild<ElaLineEdit*>("searchEdit");
    QString kw = search ? search->text() : "";

    QList<core::Note> items;
    if (currentCategoryId_ == "today") {
        auto today = QDate::currentDate();
        for (const auto& n : ctx_.notes->all())
            if (n.updatedAt.date() == today) items.append(n);
    } else if (currentCategoryId_ == "pinned") {
        for (const auto& n : ctx_.notes->all()) if (n.pinned) items.append(n);
    } else if (currentCategoryId_ == "trash") {
        items = ctx_.notes->trash();
    } else if (currentCategoryId_ == "inbox") {
        items = kw.isEmpty() ? ctx_.notes->query("") : ctx_.notes->query(kw);
    } else {
        items = ctx_.notes->query(kw, currentCategoryId_);
    }

    for (const auto& note : items) {
        auto* it = new QStandardItem();
        QString title = note.title.isEmpty() ? "(无标题)" : note.title;
        if (note.pinned) title = "⭐ " + title;
        it->setText(title);
        it->setData(note.id, Qt::UserRole);
        QString preview = note.content.left(80).replace('\n', ' ');
        if (!preview.isEmpty()) {
            it->setData(QString("%1\n%2").arg(title, preview), Qt::ToolTipRole);
        }
        m->appendRow(it);
    }
    list_->setModel(m);
}

void MainWindow::onNoteSelected(const QString& id) {
    currentNoteId_ = id;
    auto n = ctx_.notes->get(id);
    if (!n) return;
    editor_->setNote(*n);
    editor_->setMode(NoteEditor::Mode::ReadWrite);
}

void MainWindow::showAndRaise() {
    show();
    raise();
    activateWindow();
}

void MainWindow::openStickyWindow(const QString& id) {
    if (id.isEmpty()) return;
    if (stickyByNote_.contains(id)) {
        stickyByNote_[id]->show();
        stickyByNote_[id]->raise();
        stickyByNote_[id]->activateWindow();
        return;
    }
    auto n = ctx_.notes->get(id);
    if (!n) return;
    auto* w = new StickyWindow(ctx_, *n, this);
    stickyByNote_.insert(id, w);
    w->setAttribute(Qt::WA_DeleteOnClose, false);
    w->show();
}

void MainWindow::createNewNote() {
    QString cat = currentCategoryId_;
    if (cat == "today" || cat == "pinned" || cat == "trash") cat = "inbox";
    auto n = ctx_.notes->create(cat);
    refreshList();
    onNoteSelected(n.id);
    if (editor_) editor_->setFocus();
    ElaMessageBar::success(ElaMessageBarType::Top, "已创建", "新便签已创建", 1500, this);
}

void MainWindow::onNewNote() { createNewNote(); }

void MainWindow::onDeleteNote() {
    if (currentNoteId_.isEmpty()) {
        ElaMessageBar::warning(ElaMessageBarType::Top, "提示", "请先选中一条便签", 2000, this);
        return;
    }
    // 软删：移到废纸篓。文件保留。
    if (!ctx_.notes->softDelete(currentNoteId_)) {
        ElaMessageBar::error(ElaMessageBarType::Top, "失败", "无法删除只读便签", 2000, this);
        return;
    }
    currentNoteId_.clear();
    // 清空右侧编辑器，否则被软删便签的旧内容仍显示
    if (editor_) editor_->setNote(core::Note{});
    refreshList();
    updateStatusBar();
    ElaMessageBar::success(ElaMessageBarType::Top, "已移到废纸篓", "可在废纸篓里恢复或永久删除", 2500, this);
}

void MainWindow::onRestoreNote() {
    if (currentNoteId_.isEmpty()) return;
    if (!ctx_.notes->restore(currentNoteId_)) return;
    refreshList();
    updateStatusBar();
    ElaMessageBar::success(ElaMessageBarType::Top, "已恢复", nullptr, 2000, this);
}

void MainWindow::onPermanentDeleteNote() {
    if (currentNoteId_.isEmpty()) return;
    if (!ctx_.notes->permanentDelete(currentNoteId_)) return;
    currentNoteId_.clear();
    if (editor_) editor_->setNote(core::Note{});
    refreshList();
    updateStatusBar();
    ElaMessageBar::success(ElaMessageBarType::Top, "已永久删除", nullptr, 2000, this);
}

void MainWindow::onSetReminder() {
    if (currentNoteId_.isEmpty()) {
        ElaMessageBar::warning(ElaMessageBarType::Top, "提示", "请先选中一条便签", 2000, this);
        return;
    }
    auto n = ctx_.notes->get(currentNoteId_);
    if (!n) return;
    n->remindAt = QDateTime::currentDateTime().addSecs(300);
    ctx_.notes->upsert(*n);
    ElaMessageBar::success(ElaMessageBarType::Top, "已提醒",
        "将在 " + n->remindAt.toString("HH:mm") + " 提醒", 2000, this);
}

void MainWindow::onOpenSticky() {
    if (currentNoteId_.isEmpty()) {
        ElaMessageBar::warning(ElaMessageBarType::Top, "提示", "请先选中一条便签", 2000, this);
        return;
    }
    openStickyWindow(currentNoteId_);
}

void MainWindow::onExportNotes() {
    auto path = QFileDialog::getSaveFileName(this, "导出便签",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
        + "/stickynotes_export.md", "Markdown (*.md);;JSON (*.json)");
    if (path.isEmpty()) return;

    if (path.endsWith(".json", Qt::CaseInsensitive)) {
        QJsonArray arr;
        for (const auto& n : ctx_.notes->all()) {
            QJsonObject o;
            o["id"] = n.id; o["title"] = n.title; o["content"] = n.content;
            o["categoryId"] = n.categoryId; o["tags"] = n.tags.join(",");
            o["createdAt"] = n.createdAt.toString(Qt::ISODate);
            o["updatedAt"] = n.updatedAt.toString(Qt::ISODate);
            o["remindAt"]  = n.remindAt.toString(Qt::ISODate);
            o["pinned"]    = n.pinned;
            arr.append(o);
        }
        QJsonDocument doc(arr);
        QFile f(path);
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            f.write(doc.toJson(QJsonDocument::Indented));
        }
    } else {
        QFile f(path);
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QTextStream ts(&f);
            ts.setEncoding(QStringConverter::Utf8);
            for (const auto& n : ctx_.notes->all()) {
                ts << "=== " << (n.title.isEmpty() ? "(无标题)" : n.title) << " ===\n\n";
                ts << n.content << "\n\n---\n\n";
            }
        }
    }
    ElaMessageBar::success(ElaMessageBarType::Top, "已导出", path, 3000, this);
}

void MainWindow::onImportNotes() {
    auto path = QFileDialog::getOpenFileName(this, "导入便签", "",
        "Text (*.txt);;JSON (*.json)");
    if (path.isEmpty()) return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        ElaMessageBar::error(ElaMessageBarType::Top, "失败", "无法读取文件", 2000, this);
        return;
    }
    int imported = 0;
    if (path.endsWith(".json", Qt::CaseInsensitive)) {
        auto doc = QJsonDocument::fromJson(f.readAll());
        for (const auto& v : doc.array()) {
            auto obj = v.toObject();
            core::Note n;
            n.id = obj["id"].toString();
            n.title = obj["title"].toString();
            n.content = obj["content"].toString();
            n.categoryId = obj["categoryId"].toString("inbox");
            n.tags = obj["tags"].toString().split(',', Qt::SkipEmptyParts);
            n.createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate);
            n.updatedAt = QDateTime::fromString(obj["updatedAt"].toString(), Qt::ISODate);
            n.remindAt  = QDateTime::fromString(obj["remindAt"].toString(), Qt::ISODate);
            n.pinned    = obj["pinned"].toBool();
            if (n.id.isEmpty()) n = ctx_.notes->create(n.categoryId);
            else ctx_.notes->upsert(n);
            ++imported;
        }
    } else {
        QString text = QString::fromUtf8(f.readAll());
        // 纯文本按 === title === 段分割（与导出格式一致）
        const QStringList parts = text.split("=== ", Qt::SkipEmptyParts);
        for (const auto& p : parts) {
            core::Note n = ctx_.notes->create("inbox");
            int end = p.indexOf(" ===");
            if (end < 0) { n.title = p.section('\n', 0, 0).trimmed(); n.content = p; }
            else {
                n.title = p.left(end).trimmed();
                int firstNL = p.indexOf('\n', end);
                n.content = (firstNL < 0) ? QString() : p.mid(firstNL + 1).trimmed();
            }
            ctx_.notes->upsert(n);
            ++imported;
        }
    }
    refreshList();
    updateStatusBar();
    ElaMessageBar::success(ElaMessageBarType::Top, "已导入",
        QString::number(imported) + " 条便签", 2500, this);
}

void MainWindow::onShowSettings() {
    auto* dlg = new SettingsDialog(ctx_, this);
    const int ret = dlg->exec();
    if (ret == QDialog::Accepted && reregisterHotkey_) {
        // 重新注册新建便签快捷键（用最新 settings->hotkey）
        reregisterHotkey_(ctx_.settings->hotkey);
    }
    dlg->deleteLater();
}

void MainWindow::onCategoryChanged(int) {
    refreshList();
}

void MainWindow::updateStatusBar() {
    if (!statusNoteCount_) return;
    int n = ctx_.notes->all().size();
    int pinned = 0;
    for (const auto& x : ctx_.notes->all()) if (x.pinned) ++pinned;
    statusNoteCount_->setText(QString("📝 共 %1 条便签  ·  重要 %2 条").arg(n).arg(pinned));
    if (statusTheme_) {
        auto mode = eTheme->getThemeMode() == ElaThemeType::Dark ? "🌙 深色" : "☀ 浅色";
        statusTheme_->setText(QString("主题: %1").arg(mode));
    }
}
}