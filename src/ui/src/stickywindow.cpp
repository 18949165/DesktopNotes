#include "ui/stickywindow.h"
#ifdef Q_OS_WIN
#  include <windows.h>
#endif
#include <ElaToolButton.h>
#include <ElaPlainTextEdit.h>
#include <ElaLineEdit.h>
#include <ElaText.h>
#include <ElaDef.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QPalette>
#include <QApplication>

namespace stickynotes::ui {
StickyWindow::StickyWindow(app::AppContext& ctx, const core::Note& note, QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool),
      ctx_(ctx), note_(note) {
    setAttribute(Qt::WA_TranslucentBackground, false);
    resize(320, 280);
    buildUi();
    applyStyle();
    loadPosition();
}

StickyWindow::~StickyWindow() {
    savePosition();
}

void StickyWindow::buildUi() {
    // 标题栏
    titleBar_ = new QWidget(this);
    titleBar_->setFixedHeight(36);
    titleBar_->setObjectName("stickyTitle");

    // 标题显示（默认；双击切换到 titleEdit_）
    titleLabel_ = new ElaText(note_.title.isEmpty() ? "便签" : note_.title.left(20), titleBar_);
    titleLabel_->setTextPixelSize(14);
    titleLabel_->setIsWrapAnywhere(false);
    titleLabel_->setVisible(note_.title.isEmpty() == false);

    // 标题编辑（默认隐藏）
    titleEdit_ = new ElaLineEdit(titleBar_);
    titleEdit_->setText(note_.title);
    titleEdit_->setPlaceholderText("便签标题（可留空）");
    titleEdit_->setVisible(false);

    // 置顶按钮（用 ElaToolButton + ElaIconType::Thumbtack）
    pinBtn_ = new ElaToolButton(titleBar_);
    pinBtn_->setCheckable(true);
    pinBtn_->setChecked(true);             // 默认置顶
    pinBtn_->setElaIcon(ElaIconType::Thumbtack);
    pinBtn_->setIconSize(QSize(16, 16));
    pinBtn_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    pinBtn_->setFixedSize(28, 28);
    pinBtn_->setToolTip("置顶/取消置顶");
    pinBtn_->setIsTransparent(true);

    // 关闭按钮（用 ElaToolButton + ElaIconType::Xmark）
    closeBtn_ = new ElaToolButton(titleBar_);
    closeBtn_->setElaIcon(ElaIconType::Xmark);
    closeBtn_->setIconSize(QSize(16, 16));
    closeBtn_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    closeBtn_->setFixedSize(28, 28);
    closeBtn_->setToolTip("关闭");
    closeBtn_->setIsTransparent(true);

    auto* barLay = new QHBoxLayout(titleBar_);
    barLay->setContentsMargins(10, 0, 4, 0);
    barLay->setSpacing(4);
    barLay->addWidget(titleLabel_, 1);
    barLay->addWidget(titleEdit_, 1);
    barLay->addWidget(pinBtn_);
    barLay->addWidget(closeBtn_);

    // 双击标题 label -> 切换为编辑
    titleLabel_->installEventFilter(this);

    // 纯文本编辑区
    editor_ = new ElaPlainTextEdit(this);
    editor_->setFrameShape(QFrame::NoFrame);
    editor_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    editor_->setPlainText(note_.content);
    editor_->setPlaceholderText("便签内容…");

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(titleBar_);
    root->addWidget(editor_, 1);

    connect(pinBtn_,   &ElaToolButton::toggled,       this, &StickyWindow::onPinToggled);
    connect(closeBtn_, &ElaToolButton::clicked,       this, &StickyWindow::onClose);
    connect(editor_,   &ElaPlainTextEdit::textChanged, this, &StickyWindow::onSave);

    connect(titleEdit_, &ElaLineEdit::editingFinished, this, [this]() {
        note_.title = titleEdit_->text();
        titleEdit_->setVisible(false);
        titleLabel_->setText(note_.title.isEmpty() ? "便签" : note_.title.left(20));
        titleLabel_->setVisible(true);
        ctx_.notes->upsert(note_);
    });
    connect(titleEdit_, &ElaLineEdit::textChanged, this, [this](const QString& t) {
        titleLabel_->setText(t.isEmpty() ? "便签" : t.left(20));
    });
}

void StickyWindow::applyStyle() {
    titleBar_->setAutoFillBackground(true);
    auto pal = titleBar_->palette();
    pal.setColor(QPalette::Window, QColor(147, 197, 253));
    titleBar_->setPalette(pal);
}

void StickyWindow::onPinToggled(bool checked) {
    // Qt::Tool + WindowStaysOnTopHint 在 Win32 上有可靠性问题（Tool windows
    // 默认就在 owner 之上的 z-order 栈，且 hide/show 不一定能切 StaysOnTop 状态）。
    // 直接用 Win32 SetWindowPos 切 HWND_TOPMOST / HWND_NOTOPMOST，最稳。
    QWidget::setAttribute(Qt::WA_WState_Hidden, true);   // 抑制 hide 动画
    hide();
    Qt::WindowFlags base = Qt::FramelessWindowHint | Qt::Tool;
    if (checked) base |= Qt::WindowStaysOnTopHint;
    setWindowFlags(base);
    QApplication::processEvents();
    show();
#ifdef Q_OS_WIN
    if (checked) {
        // 提升到 TOPMOST
        ::SetWindowPos(HWND(reinterpret_cast<HWND>(winId())),
                       HWND_TOPMOST, 0, 0, 0, 0,
                       SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    } else {
        // 退回 NOTOPMOST
        ::SetWindowPos(HWND(reinterpret_cast<HWND>(winId())),
                       HWND_NOTOPMOST, 0, 0, 0, 0,
                       SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
#endif
    raise();
}

void StickyWindow::onClose() { close(); }

void StickyWindow::onSave() {
    note_.content = editor_->toPlainText();
    // title 独立维护：只有在 title 为空时（首次输入）才自动用首行填充
    if (note_.title.isEmpty() && !note_.content.isEmpty()) {
        note_.title = note_.content.section('\n', 0, 0).left(80);
        titleLabel_->setText(note_.title.left(20));
    }
    ctx_.notes->upsert(note_);
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

bool StickyWindow::eventFilter(QObject* obj, QEvent* e) {
    if (obj == titleLabel_ && e->type() == QEvent::MouseButtonDblClick) {
        titleLabel_->setVisible(false);
        titleEdit_->setVisible(true);
        titleEdit_->setFocus();
        titleEdit_->selectAll();
        return true;
    }
    return QWidget::eventFilter(obj, e);
}

void StickyWindow::closeEvent(QCloseEvent* e) {
    savePosition();
    e->accept();
}

void StickyWindow::savePosition() {
    note_.windowGeometry = geometry();
    ctx_.notes->upsert(note_);
}

void StickyWindow::loadPosition() {
    if (note_.windowGeometry.isValid() && note_.windowGeometry.width() > 0) {
        setGeometry(note_.windowGeometry);
    }
}
}
