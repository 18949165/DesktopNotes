#pragma once
#include <ElaWidget.h>
#include "core/note.h"

class ElaPlainTextEdit;
class ElaLineEdit;

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
    void rebuildFromMd();

    ElaLineEdit* titleEdit_ = nullptr;   // 独立标题输入
    ElaPlainTextEdit* edit_ = nullptr;   // 纯文本内容编辑
    core::Note current_;
    Mode mode_ = Mode::ReadWrite;
    bool internal_ = false;
};
}
