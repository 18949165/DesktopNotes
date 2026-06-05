#include "ui/note_editor.h"
#include "core/markdown_codec.h"
#include <ElaToolButton.h>
#include <ElaLineEdit.h>
#include <ElaPlainTextEdit.h>
#include <QHBoxLayout>
#include <QTextCursor>

namespace stickynotes::ui {
NoteEditor::NoteEditor(QWidget* parent) : ElaWidget(parent) {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    // 工具栏：用 ElaToolButton 自定义，文字 + icon 显式分两段
    bar_ = new QWidget(this);
    bar_->setFixedHeight(38);
    bar_->setObjectName("noteEditorBar");
    bar_->setStyleSheet(
        "QWidget#noteEditorBar{background:palette(window);border-bottom:1px solid rgba(127,127,127,30%);}"
        "QWidget#noteEditorBar QToolButton{background:transparent;border:0;border-radius:4px;padding:4px 10px;color:palette(text);font-size:12px;}"
        "QWidget#noteEditorBar QToolButton:hover{background:rgba(127,127,127,30);}"
        "QWidget#noteEditorBar QToolButton:pressed{background:rgba(127,127,127,60);}"
    );
    auto* barLay = new QHBoxLayout(bar_);
    barLay->setContentsMargins(8, 4, 8, 4);
    barLay->setSpacing(2);

    auto* boldBtn    = mkBtn(ElaIconType::Bold,       "粗体 (Ctrl+B)", "Ctrl+B");
    auto* italicBtn  = mkBtn(ElaIconType::Italic,     "斜体 (Ctrl+I)", "Ctrl+I");
    auto* underBtn   = mkBtn(ElaIconType::Underline,  "下划线 (Ctrl+U)", "Ctrl+U");
    auto* headBtn    = mkBtn(ElaIconType::Heading,    "标题");
    auto* listBtn    = mkBtn(ElaIconType::List,       "列表");
    auto* quoteBtn   = mkBtn(ElaIconType::QuoteRight, "引用");
    auto* linkBtn    = mkBtn(ElaIconType::Link,       "链接");

    barLay->addWidget(boldBtn);
    barLay->addWidget(italicBtn);
    barLay->addWidget(underBtn);
    barLay->addSpacing(8);
    barLay->addWidget(headBtn);
    barLay->addWidget(listBtn);
    barLay->addWidget(quoteBtn);
    barLay->addSpacing(8);
    barLay->addWidget(linkBtn);
    barLay->addStretch();

    // 独立标题
    titleEdit_ = new ElaLineEdit(this);
    titleEdit_->setObjectName("noteTitleEdit");
    titleEdit_->setPlaceholderText("标题（可留空，自动用首行）");
    titleEdit_->setStyleSheet(
        "ElaLineEdit#noteTitleEdit{background:palette(window);border:0;"
        "border-bottom:1px solid rgba(127,127,127,30%);padding:10px 16px;"
        "font-size:15px;font-weight:600;color:palette(text);}"
    );

    // 编辑器
    edit_ = new ElaPlainTextEdit(this);
    edit_->setFrameShape(QFrame::NoFrame);
    edit_->setStyleSheet("ElaPlainTextEdit{background:transparent;border:0;padding:14px 16px;font-size:14px;}");
    edit_->setPlaceholderText("开始记录你的想法…（支持 Markdown：**粗体** *斜体* # 标题 - 列表 > 引用）");

    lay->addWidget(bar_);
    lay->addWidget(titleEdit_);
    lay->addWidget(edit_, 1);

    connect(titleEdit_, &ElaLineEdit::textChanged, this, [this](const QString& t) {
        if (internal_) return;
        current_.title = t;
        emit contentChanged();
    });
    connect(edit_, &ElaPlainTextEdit::textChanged, this, [this]() {
        if (internal_) return;
        current_.contentMd = edit_->toPlainText();
        emit contentChanged();
    });
    connect(boldBtn,   &QToolButton::clicked, this, [this]() { wrapSelection("**", "**"); });
    connect(italicBtn, &QToolButton::clicked, this, [this]() { wrapSelection("*", "*"); });
    connect(underBtn,  &QToolButton::clicked, this, [this]() { wrapSelection("<u>", "</u>"); });
    connect(headBtn,   &QToolButton::clicked, this, [this]() { prefixLines("# "); });
    connect(listBtn,   &QToolButton::clicked, this, [this]() { prefixLines("- "); });
    connect(quoteBtn,  &QToolButton::clicked, this, [this]() { prefixLines("> "); });
    connect(linkBtn,   &QToolButton::clicked, this, [this]() { wrapSelection("[", "](https://)"); });
}

ElaToolButton* NoteEditor::mkBtn(ElaIconType::IconName icon, const QString& tip, const QString& shortcut) {
    auto* b = new ElaToolButton(this);
    b->setElaIcon(icon);
    b->setIconSize(QSize(14, 14));
    b->setToolButtonStyle(Qt::ToolButtonTextOnly);
    static const QHash<ElaIconType::IconName, QString> map = {
        { ElaIconType::Bold,       "B" },
        { ElaIconType::Italic,     "I" },
        { ElaIconType::Underline,  "U" },
        { ElaIconType::Heading,    "H1" },
        { ElaIconType::List,       "≡" },
        { ElaIconType::QuoteRight, "\u201C" },
        { ElaIconType::Link,       "🔗" },
    };
    b->setText(map.value(icon, "?"));
    b->setFixedSize(34, 28);
    b->setToolTip(tip);
    if (!shortcut.isEmpty()) b->setShortcut(QKeySequence(shortcut));
    return b;
}

void NoteEditor::setNote(const core::Note& n) {
    current_ = n;
    rebuildFromMd();
}

void NoteEditor::rebuildFromMd() {
    internal_ = true;
    titleEdit_->setText(current_.title);
    edit_->setPlainText(current_.contentMd);
    internal_ = false;
}

core::Note NoteEditor::note() const { return current_; }

void NoteEditor::setMode(Mode m) {
    mode_ = m;
    bool ro = (m == Mode::ReadOnly);
    edit_->setReadOnly(ro);
    titleEdit_->setReadOnly(ro);
    if (bar_) bar_->setEnabled(!ro);
}

void NoteEditor::setFocus() {
    if (edit_) edit_->setFocus();
}

void NoteEditor::wrapSelection(const QString& left, const QString& right) {
    if (!edit_) return;
    QTextCursor c = edit_->textCursor();
    if (c.hasSelection()) {
        c.insertText(left + c.selectedText() + right);
    } else {
        c.insertText(left + "文本" + right);
    }
}

void NoteEditor::prefixLines(const QString& prefix) {
    if (!edit_) return;
    QTextCursor c = edit_->textCursor();
    int start = c.selectionStart();
    int end   = c.selectionEnd();
    QString text = edit_->toPlainText();
    if (start < 0 || start > text.length()) return;
    if (start > end) std::swap(start, end);
    while (start > 0 && text[start - 1] != '\n') --start;
    while (end < text.length() && text[end] != '\n') ++end;
    QString block = text.mid(start, end - start);
    QStringList lines = block.split('\n');
    for (auto& l : lines) l = prefix + l;
    QString newBlock = lines.join('\n');
    c.beginEditBlock();
    c.setPosition(start);
    c.setPosition(end, QTextCursor::KeepAnchor);
    c.insertText(newBlock);
    c.endEditBlock();
}
}
