#pragma once
#include <ElaWidget.h>
#include "core/note.h"

class ElaPlainTextEdit;
class ElaLineEdit;
class ElaToolButton;

namespace stickynotes::ui {
class NoteEditor : public ElaWidget {
    Q_OBJECT
public:
    enum class Mode { ReadOnly, ReadWrite };
    explicit NoteEditor(QWidget* parent = nullptr);
    void setNote(const core::Note& n);
    core::Note note() const;
    void setMode(Mode m);
    void setFocus();
    ElaPlainTextEdit* textEdit() const { return edit_; }
signals:
    void contentChanged();
private:
    ElaToolButton* mkBtn(ElaIconType::IconName icon, const QString& tip, const QString& shortcut = QString());
    void wrapSelection(const QString& left, const QString& right);
    void prefixLines(const QString& prefix);
    void rebuildFromMd();

    QWidget* bar_ = nullptr;
    ElaLineEdit* titleEdit_ = nullptr;   // 独立标题输入
    ElaPlainTextEdit* edit_ = nullptr;
    core::Note current_;
    Mode mode_ = Mode::ReadWrite;
    bool internal_ = false;
};
}
