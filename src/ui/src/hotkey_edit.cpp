#include "ui/hotkey_edit.h"
#include <QFocusEvent>
#include <QKeyEvent>

namespace stickynotes::ui {
HotkeyEdit::HotkeyEdit(QWidget* parent) : QLineEdit(parent) {
    setReadOnly(false);
    setPlaceholderText("点击后按下组合键（如 Ctrl+Alt+N）");
    setClearButtonEnabled(false);
}

QString HotkeyEdit::keySequence() const { return captureText_; }

void HotkeyEdit::setKeySequence(const QString& s) {
    captureText_ = s;
    setText(s);
}

void HotkeyEdit::focusInEvent(QFocusEvent* e) {
    QLineEdit::focusInEvent(e);
    capturing_ = true;
    pendingText_.clear();
    setPlaceholderText("按组合键（Esc 取消 / Backspace 清空）");
    setText(QString());
}

void HotkeyEdit::focusOutEvent(QFocusEvent* e) {
    QLineEdit::focusOutEvent(e);
    if (capturing_) {
        commitCaptured();
        capturing_ = false;
    }
    setPlaceholderText("点击后按下组合键（如 Ctrl+Alt+N）");
}

void HotkeyEdit::keyPressEvent(QKeyEvent* e) {
    // 让 Ctrl/Alt/Win/Shift 这种修饰键单独按也能在累积窗口中显示
    if (!capturing_) capturing_ = true;

    // Esc → 放弃，恢复 captureText_
    if (e->key() == Qt::Key_Escape) {
        pendingText_.clear();
        commitCaptured();    // restore previous captureText_
        capturing_ = false;
        clearFocus();
        e->accept();
        return;
    }

    // Backspace → 清空
    if (e->key() == Qt::Key_Backspace && pendingText_.isEmpty()) {
        captureText_.clear();
        setText(QString());
        e->accept();
        return;
    }

    // 忽略单独的修饰键按压作为"主键完成"
    if (e->key() == Qt::Key_Control || e->key() == Qt::Key_Shift ||
        e->key() == Qt::Key_Alt     || e->key() == Qt::Key_Meta) {
        // 累积显示当前所有按下的 modifier + 已经按下的主键
        renderCaptured();
        e->accept();
        return;
    }

    // 收到主键 → 累积 modifier+主键
    pendingText_ = toPortableString(e->key(), e->modifiers());
    renderCaptured();

    // 接受并自动 commit：用户按一个主键就完成，避免"Shift 单独按也算"的歧义
    commitCaptured();
    capturing_ = false;
    clearFocus();
    e->accept();
}

void HotkeyEdit::renderCaptured() {
    // 把 modifiers 转成可见字符串：例如 "Ctrl+Shift"
    QString mods;
    Qt::KeyboardModifiers m;
    // 这里我们只看 modifiers，用 QGuiApplication 拿不到；用 QKeySequence::toString 的反向
    // 简化：累积时仅显示 pendingText_（已含 modifiers）
    if (!pendingText_.isEmpty()) {
        setText(pendingText_);
    } else {
        // 第一次按 modifier 时只展示当前按下的 modifier
        setText(QString());
    }
    Q_UNUSED(mods);
}

void HotkeyEdit::commitCaptured() {
    if (!pendingText_.isEmpty()) {
        captureText_ = pendingText_;
    }
    setText(captureText_);
    pendingText_.clear();
}

// Build a Qt-style portable string, e.g. "Ctrl+Alt+N" / "Shift+F5".
QString HotkeyEdit::toPortableString(int k, Qt::KeyboardModifiers mods) const {
    QStringList parts;
    if (mods & Qt::ControlModifier) parts << "Ctrl";
    if (mods & Qt::AltModifier)     parts << "Alt";
    if (mods & Qt::ShiftModifier)   parts << "Shift";
    if (mods & Qt::MetaModifier)    parts << "Meta";
    parts << QKeySequence(k).toString();   // e.g. "N", "F5", "Left"
    return parts.join('+');
}
}
