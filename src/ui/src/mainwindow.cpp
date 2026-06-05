#include "ui/mainwindow.h"
#include "ui/note_editor.h"
#include "ui/stickywindow.h"
#include "core/note.h"
#include "core/category.h"
#include <QSplitter>
#include <QLineEdit>
#include <QTreeView>
#include <QListView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QWidget>

namespace stickynotes::ui {
MainWindow::MainWindow(app::AppContext& ctx, QWidget* parent) 
    : QMainWindow(parent), ctx_(ctx) {
    buildUi();
    wireSignals();
    setWindowTitle("StickyNotes");
    resize(1100, 720);
}

void MainWindow::buildUi() {
    search_ = new QLineEdit(this);
    search_->setPlaceholderText("搜索…");
    
    tree_ = new QTreeView(this);
    list_ = new QListView(this);
    editor_ = new NoteEditor(this);
    
    auto* sp = new QSplitter(Qt::Horizontal, this);
    sp->addWidget(tree_);
    sp->addWidget(list_);
    sp->addWidget(editor_);
    sp->setStretchFactor(2, 1);
    
    auto* central = new QWidget(this);
    auto* lay = new QVBoxLayout(central);
    lay->addWidget(search_);
    lay->addWidget(sp, 1);
    
    setCentralWidget(central);
    
    refreshCategories();
    refreshList(currentCategoryId_);
}

void MainWindow::wireSignals() {
    connect(search_, &QLineEdit::textChanged, this, [this](const QString& t) {
        refreshList(currentCategoryId_, t);
    });
    
    connect(editor_, &NoteEditor::contentChanged, this, [this]() {
        if (!currentNoteId_.isEmpty()) {
            auto n = editor_->note();
            ctx_.notes->release(currentNoteId_);
            ctx_.notes->acquire(currentNoteId_);
            ctx_.notes->upsert(n);
        }
    });
    
    connect(editor_, &NoteEditor::focusAcquired, this, [this]() {
        if (!currentNoteId_.isEmpty() && !ctx_.notes->acquire(currentNoteId_)) {
            editor_->setMode(NoteEditor::Mode::ReadOnly);
        }
    });
    
    connect(editor_, &NoteEditor::focusLost, this, [this]() {
        if (!currentNoteId_.isEmpty()) {
            ctx_.notes->release(currentNoteId_);
        }
    });
    
    connect(list_, &QListView::doubleClicked, this, [this](const QModelIndex& idx) {
        auto id = idx.data(Qt::UserRole).toString();
        onNoteSelected(id);
    });
}

void MainWindow::refreshCategories() {
    auto* m = new QStandardItemModel(tree_);
    for (const auto& c : ctx_.notes->categories()) {
        auto* it = new QStandardItem(c.name);
        it->setData(c.id, Qt::UserRole);
        m->appendRow(it);
    }
    tree_->setModel(m);
}

void MainWindow::refreshList(const QString& cat, const QString& kw) {
    auto* m = new QStandardItemModel(list_);
    auto items = kw.isEmpty() ? ctx_.notes->query("") : ctx_.notes->query(kw, cat);
    for (const auto& note : items) {
        auto* it = new QStandardItem(note.title.isEmpty() ? "(无标题)" : note.title);
        it->setData(note.id, Qt::UserRole);
        m->appendRow(it);
    }
    list_->setModel(m);
}

void MainWindow::onNoteSelected(const QString& id) {
    if (!currentNoteId_.isEmpty()) {
        ctx_.notes->release(currentNoteId_);
    }
    currentNoteId_ = id;
    auto n = ctx_.notes->get(id);
    if (n) {
        editor_->setNote(*n);
        if (ctx_.notes->acquire(id)) {
            editor_->setMode(NoteEditor::Mode::ReadWrite);
        } else {
            editor_->setMode(NoteEditor::Mode::ReadOnly);
        }
    }
}

void MainWindow::showAndRaise() {
    show();
    raise();
    activateWindow();
}

void MainWindow::openStickyWindow(const QString& id) {
    if (stickyByNote_.contains(id)) {
        stickyByNote_[id]->raise();
        stickyByNote_[id]->activateWindow();
        return;
    }
    
    auto n = ctx_.notes->get(id);
    if (!n) return;
    
    auto* w = new StickyWindow(ctx_, *n);
    stickyByNote_.insert(id, w);
    
    if (!ctx_.notes->acquire(id)) {
        w->findChild<NoteEditor*>()->setMode(NoteEditor::Mode::ReadOnly);
    }
    
    w->setAttribute(Qt::WA_DeleteOnClose, false);
    w->show();
}
}
