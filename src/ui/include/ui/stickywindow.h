#pragma once
#include <QWidget>
#include "core/note.h"
#include "app/app_context.h"

class QPushButton;

namespace stickynotes::ui {
class NoteEditor;

class StickyWindow : public QWidget {
    Q_OBJECT
public:
    StickyWindow(app::AppContext& ctx, core::Note note, QWidget* parent = nullptr);
    core::Note note() const { return current_; }
    void startFlash();
protected:
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void closeEvent(QCloseEvent*) override;
private slots:
    void onPinToggled(bool on);
    void onTopToggled(bool on);
    void onEditorContentChanged();
private:
    void persistGeometry();
    
    app::AppContext& ctx_;
    core::Note current_;
    NoteEditor* editor_ = nullptr;
    QPushButton* btnPin_ = nullptr;
    QPushButton* btnTop_ = nullptr;
    QPoint dragPos_;
    int flashLeft_ = 0;
};
}
