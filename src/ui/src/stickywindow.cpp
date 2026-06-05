#include "ui/stickywindow.h"
#include "core/markdown_codec.h"
#include <ElaIconButton.h>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QPalette>
#include <QApplication>
#include <QScreen>
#include <QStyle>
#include <ElaApplication.h>

namespace stickynotes::ui {
StickyWindow::StickyWindow(app::AppContext& ctx, const core::Note& note, QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool),
      ctx_(ctx), note_(note) {
    setAttribute(Qt::WA_TranslucentBackground, false);
    resize(320, 280);
    buildUi();
    applyStyle();
    loadPosition();

    if (!note_.remindAt.isNull() && note_.remindAt <= QDateTime::currentDateTime()) {
        startFlash();
    }
}

StickyWindow::~StickyWindow() {
    savePosition();
    stopFlash();
}

void StickyWindow::buildUi() {
    // 标题栏
    titleBar_ = new QWidget(this);
    titleBar_->setFixedHeight(28);
    titleBar_->setObjectName("stickyTitle");

    titleLabel_ = new QLabel(note_.title.isEmpty() ? "便签" : note_.title.left(20), titleBar_);
    titleLabel_->setObjectName("stickyTitleLabel");

    pinBtn_ = new QPushButton(titleBar_);
    pinBtn_->setObjectName("stickyPinBtn");
    pinBtn_->setCheckable(true);
    pinBtn_->setChecked(true);
    pinBtn_->setFixedSize(24, 24);
    pinBtn_->setToolTip("置顶/取消置顶");

    closeBtn_ = new QPushButton(titleBar_);
    closeBtn_->setObjectName("stickyCloseBtn");
    closeBtn_->setFixedSize(24, 24);
    closeBtn_->setToolTip("关闭");

    auto* barLay = new QHBoxLayout(titleBar_);
    barLay->setContentsMargins(8, 0, 4, 0);
    barLay->setSpacing(4);
    barLay->addWidget(titleLabel_, 1);
    barLay->addWidget(pinBtn_);
    barLay->addWidget(closeBtn_);

    // 编辑区
    editor_ = new QTextEdit(this);
    editor_->setFrameShape(QFrame::NoFrame);
    editor_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    editor_->setPlainText(note_.contentMd);
    editor_->setPlaceholderText("便签内容…");

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(titleBar_);
    root->addWidget(editor_, 1);

    connect(pinBtn_, &QPushButton::toggled, this, &StickyWindow::onPinToggled);
    connect(closeBtn_, &QPushButton::clicked, this, &StickyWindow::onClose);
    connect(editor_, &QTextEdit::textChanged, this, &StickyWindow::onSave);

    flashTimer_ = new QTimer(this);
    connect(flashTimer_, &QTimer::timeout, this, [this]() {
        auto pal = titleBar_->palette();
        pal.setColor(QPalette::Window, isFlashing_ ? QColor(255, 235, 130) : QColor(250, 220, 80));
        titleBar_->setPalette(pal);
        titleBar_->setAutoFillBackground(true);
        isFlashing_ = !isFlashing_;
        if (++flashCount_ > 12) stopFlash();
    });
}

void StickyWindow::applyStyle() {
    QString btnStyle = R"(
        QPushButton {
            background: transparent;
            border: none;
            border-radius: 4px;
        }
        QPushButton:hover { background: rgba(127,127,127,40); }
        QPushButton:pressed { background: rgba(127,127,127,80); }
    )";
    pinBtn_->setStyleSheet(btnStyle);
    closeBtn_->setStyleSheet(btnStyle);

    // 标题文字和按钮
    setStyleSheet(R"(
        QWidget#stickyTitle {
            background: #FAE550;
            border-bottom: 1px solid rgba(127,127,127,40%);
        }
        QLabel#stickyTitleLabel {
            color: #333;
            font-size: 12px;
            font-weight: 600;
            background: transparent;
        }
        QTextEdit {
            background: #FFF7AE;
            color: #222;
            border: none;
            padding: 8px;
            font-size: 13px;
            selection-background-color: #EBC900;
        }
    )");
    pinBtn_->setText("📌");
    closeBtn_->setText("✕");
    titleBar_->setAutoFillBackground(true);
    auto pal = titleBar_->palette();
    pal.setColor(QPalette::Window, QColor(250, 220, 80));
    titleBar_->setPalette(pal);
}

void StickyWindow::onPinToggled(bool checked) {
    Qt::WindowFlags base = Qt::FramelessWindowHint | Qt::Tool;
    if (checked) base |= Qt::WindowStaysOnTopHint;
    // 改变 window flags 必须 hide 后重新 show 才生效
    hide();
    setWindowFlags(base);
    show();
    pinBtn_->setText(checked ? "📌" : "📍");
}

void StickyWindow::onClose() { close(); }

void StickyWindow::onSave() {
    note_.contentMd = editor_->toPlainText();
    if (note_.contentMd.isEmpty()) note_.title = "";
    else note_.title = note_.contentMd.section('\n', 0, 0).left(80);
    titleLabel_->setText(note_.title.isEmpty() ? "便签" : note_.title.left(20));
    ctx_.notes->upsert(note_);
}

void StickyWindow::startFlash() {
    flashCount_ = 0;
    isFlashing_ = false;
    flashTimer_->start(350);
}

void StickyWindow::stopFlash() {
    if (flashTimer_) flashTimer_->stop();
    if (titleBar_) {
        auto pal = titleBar_->palette();
        pal.setColor(QPalette::Window, QColor(250, 220, 80));
        titleBar_->setPalette(pal);
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

void StickyWindow::mouseDoubleClickEvent(QMouseEvent* e) {
    // 双击标题栏 -> 关闭
    if (titleBar_->geometry().contains(e->pos())) {
        close();
    }
}

void StickyWindow::closeEvent(QCloseEvent* e) {
    savePosition();
    e->accept();
}

void StickyWindow::setNote(const core::Note& n) {
    note_ = n;
    if (editor_) editor_->setPlainText(n.contentMd);
    if (titleLabel_) titleLabel_->setText(n.title.isEmpty() ? "便签" : n.title.left(20));
}

void StickyWindow::savePosition() {
    note_.windowGeometry = geometry();
    ctx_.notes->upsert(note_);
}

void StickyWindow::loadPosition() {
    if (note_.windowGeometry.isValid() && note_.windowGeometry.width() > 0) {
        QRect g = note_.windowGeometry;
        auto* screen = QApplication::primaryScreen();
        if (screen) {
            QRect avail = screen->availableGeometry();
            if (!avail.intersects(g)) g.moveCenter(avail.center());
        }
        setGeometry(g);
    } else {
        auto* screen = QApplication::primaryScreen();
        if (screen) {
            QRect avail = screen->availableGeometry();
            move(avail.center() - QPoint(width() / 2, height() / 2));
        }
    }
}

core::Note StickyWindow::note() const { return note_; }
}