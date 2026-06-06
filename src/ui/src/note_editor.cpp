#include "ui/note_editor.h"
#include <ElaLineEdit.h>
#include <ElaPlainTextEdit.h>
#include <QVBoxLayout>

namespace stickynotes::ui {
NoteEditor::NoteEditor(QWidget* parent) : ElaWidget(parent) {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    // 独立标题
    titleEdit_ = new ElaLineEdit(this);
    titleEdit_->setObjectName("noteTitleEdit");
    titleEdit_->setPlaceholderText("标题（可留空，自动用首行）");
    titleEdit_->setStyleSheet(
        "ElaLineEdit#noteTitleEdit{background:palette(window);border:0;"
        "border-bottom:1px solid rgba(127,127,127,30%);padding:10px 16px;"
        "font-size:15px;font-weight:600;color:palette(text);}"
    );

    // 纯文本编辑器（v1 抛弃富文本，只用纯文本记录）
    edit_ = new ElaPlainTextEdit(this);
    edit_->setFrameShape(QFrame::NoFrame);
    edit_->setStyleSheet("ElaPlainTextEdit{background:transparent;border:0;padding:14px 16px;font-size:14px;}");
    edit_->setPlaceholderText("开始记录你的想法…");

    lay->addWidget(titleEdit_);
    lay->addWidget(edit_, 1);

    connect(titleEdit_, &ElaLineEdit::textChanged, this, [this](const QString& t) {
        if (internal_) return;
        current_.title = t;
        emit contentChanged();
    });
    connect(edit_, &ElaPlainTextEdit::textChanged, this, [this]() {
        if (internal_) return;
        current_.content = edit_->toPlainText();
        emit contentChanged();
    });
}

void NoteEditor::setNote(const core::Note& n) {
    current_ = n;
    rebuildFromMd();
}

void NoteEditor::rebuildFromMd() {
    internal_ = true;
    titleEdit_->setText(current_.title);
    edit_->setPlainText(current_.content);
    internal_ = false;
}

core::Note NoteEditor::note() const { return current_; }

void NoteEditor::setMode(Mode m) {
    mode_ = m;
    bool ro = (m == Mode::ReadOnly);
    edit_->setReadOnly(ro);
    titleEdit_->setReadOnly(ro);
}

void NoteEditor::setFocus() {
    if (edit_) edit_->setFocus();
}
}
