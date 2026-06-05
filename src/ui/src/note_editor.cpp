#include "ui/note_editor.h"
#include "core/markdown_codec.h"
#include <QTextEdit>
#include <QToolBar>
#include <QVBoxLayout>
#include <QEvent>
#include <QAction>

namespace stickynotes::ui {
NoteEditor::NoteEditor(QWidget* parent) : QWidget(parent) {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    
    bar_ = new QToolBar(this);
    bar_->setMovable(false);
    
    auto* boldAct = bar_->addAction("B");
    connect(boldAct, &QAction::triggered, this, [this]() {
        edit_->setFontWeight(edit_->fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
    });
    
    auto* italicAct = bar_->addAction("I");
    connect(italicAct, &QAction::triggered, this, [this]() {
        QFont f = edit_->currentFont();
        f.setItalic(!f.italic());
        edit_->setCurrentFont(f);
    });
    
    auto* underlineAct = bar_->addAction("U");
    connect(underlineAct, &QAction::triggered, this, [this]() {
        QFont f = edit_->currentFont();
        f.setUnderline(!f.underline());
        edit_->setCurrentFont(f);
    });
    
    edit_ = new QTextEdit(this);
    lay->addWidget(bar_);
    lay->addWidget(edit_, 1);
    
    connect(edit_, &QTextEdit::textChanged, this, [this]() {
        if (internal_) return;
        current_.contentMd = core::MarkdownCodec::htmlToMarkdown(edit_->toHtml());
        current_.title = current_.contentMd.section('\n', 0, 0).left(80);
        emit contentChanged();
    });
    
    installEventFilter(this);
}

void NoteEditor::setNote(const core::Note& n) {
    current_ = n;
    rebuildFromMd();
}

void NoteEditor::rebuildFromMd() {
    internal_ = true;
    edit_->setHtml(core::MarkdownCodec::markdownToHtml(current_.contentMd));
    internal_ = false;
}

core::Note NoteEditor::note() const {
    return current_;
}

void NoteEditor::setMode(Mode m) {
    mode_ = m;
    edit_->setReadOnly(m == Mode::ReadOnly);
    updateBarEnabled();
}

void NoteEditor::updateBarEnabled() {
    for (auto* a : bar_->actions()) {
        a->setEnabled(mode_ == Mode::ReadWrite);
    }
}

bool NoteEditor::eventFilter(QObject* o, QEvent* e) {
    if (o == edit_) {
        if (e->type() == QEvent::FocusIn) {
            emit focusAcquired();
        }
        if (e->type() == QEvent::FocusOut) {
            emit focusLost();
        }
    }
    return QWidget::eventFilter(o, e);
}
}
