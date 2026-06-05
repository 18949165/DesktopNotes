#pragma once
#include <QWidget>
#include "core/note.h"
class QTextEdit;
class QToolBar;

namespace stickynotes::ui {
class NoteEditor : public QWidget {
    Q_OBJECT
public:
    enum class Mode { ReadOnly, ReadWrite };
    explicit NoteEditor(QWidget* parent = nullptr);
    void setNote(const core::Note& n);
    core::Note note() const;
    void setMode(Mode m);
signals:
    void contentChanged();
    void focusAcquired();
    void focusLost();
private:
    void rebuildFromMd();
    void updateBarEnabled();
    bool eventFilter(QObject* o, QEvent* e) override;
    QTextEdit* edit_ = nullptr;
    QToolBar* bar_ = nullptr;
    core::Note current_;
    Mode mode_ = Mode::ReadWrite;
    bool internal_ = false;
};
}
