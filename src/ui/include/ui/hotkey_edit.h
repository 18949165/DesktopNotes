#pragma once
#include <QLineEdit>

namespace stickynotes::ui {
// HotkeyEdit: a single-line text input that captures a Qt-style shortcut
// (e.g. "Ctrl+Alt+N") by listening to key presses. Unlike the standard
// QKeySequenceEdit, it is a single QLineEdit so it can be styled with
// QSS to match the rest of the app.
//
// Semantics:
//   - focusIn  : switch to capture mode (greyed text shows a hint)
//   - any key  : accumulate modifier(s) + main key, render as "Ctrl+Alt+N"
//   - Esc      : abort capture and restore the previous value
//   - Backspace: clear the captured value
//   - focusOut : accept the captured value (if any)
class HotkeyEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit HotkeyEdit(QWidget* parent = nullptr);
    QString keySequence() const;
    void setKeySequence(const QString& s);
signals:
    void keySequenceChanged(const QString& s);
protected:
    void focusInEvent(QFocusEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
private:
    void renderCaptured();
    void commitCaptured();
    QString toPortableString(int k, Qt::KeyboardModifiers mods) const;
    QString captureText_;
    QString pendingText_;
    bool capturing_ = false;
};
}
