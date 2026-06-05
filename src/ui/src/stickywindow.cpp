#include "ui/stickywindow.h"
#include "ui/note_editor.h"
#include <QHBoxLayout>
#include <QToolBar>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QTimer>
#include <QPushButton>
#include <QVBoxLayout>

namespace stickynotes::ui {
StickyWindow::StickyWindow(app::AppContext& ctx, core::Note n, QWidget* p)
    : QWidget(p, Qt::FramelessWindowHint | Qt::Tool), ctx_(ctx), current_(n) {
    auto* bar = new QToolBar(this);
    bar->setMovable(false);
    
    btnPin_ = new QPushButton("📌", this);
    btnPin_->setCheckable(true);
    btnPin_->setChecked(n.pinned);
    
    btnTop_ = new QPushButton("⬆", this);
    btnTop_->setCheckable(true);
    
    auto* closeBtn = new QPushButton("✕", this);
    
    bar->addWidget(btnPin_);
    bar->addWidget(btnTop_);
    bar->addWidget(closeBtn);
    
    editor_ = new NoteEditor(this);
    
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(bar);
    lay->addWidget(editor_, 1);
    
    setFixedSize(QSize(320, 260));
    if (current_.windowGeometry.isValid()) {
        setGeometry(current_.windowGeometry);
    }
    
    editor_->setNote(current_);
    
    connect(btnPin_, &QPushButton::toggled, this, &StickyWindow::onPinToggled);
    connect(btnTop_, &QPushButton::toggled, this, &StickyWindow::onTopToggled);
    connect(closeBtn, &QPushButton::clicked, this, &StickyWindow::close);
    connect(editor_, &NoteEditor::contentChanged, this, &StickyWindow::onEditorContentChanged);
}

void StickyWindow::onPinToggled(bool on) {
    current_.pinned = on;
    persistGeometry();
    ctx_.notes->upsert(current_);
}

void StickyWindow::onTopToggled(bool on) {
    Qt::WindowFlags f = windowFlags();
    if (on) {
        f |= Qt::WindowStaysOnTopHint;
    } else {
        f &= ~Qt::WindowStaysOnTopHint;
    }
    setWindowFlags(f);
    show();
}

void StickyWindow::onEditorContentChanged() {
    current_ = editor_->note();
    current_.windowGeometry = geometry();
    if (current_.pinned) {
        ctx_.notes->upsert(current_);
    }
}

void StickyWindow::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        dragPos_ = e->globalPosition().toPoint() - frameGeometry().topLeft();
        e->accept();
    }
}

void StickyWindow::mouseMoveEvent(QMouseEvent* e) {
    if (e->buttons() & Qt::LeftButton) {
        move(e->globalPosition().toPoint() - dragPos_);
        e->accept();
    }
}

void StickyWindow::persistGeometry() {
    current_.windowGeometry = geometry();
    if (current_.pinned) {
        ctx_.notes->upsert(current_);
    }
}

void StickyWindow::closeEvent(QCloseEvent* e) {
    if (!current_.id.isEmpty()) {
        ctx_.notes->release(current_.id);
    }
    hide();
    e->ignore();
}

void StickyWindow::startFlash() {
    flashLeft_ = 5;
    auto* t = new QTimer(this);
    t->setInterval(300);
    connect(t, &QTimer::timeout, this, [this, t]() {
        if (--flashLeft_ <= 0) {
            t->stop();
            t->deleteLater();
            return;
        }
        setVisible(!isVisible());
    });
    t->start();
}
}
