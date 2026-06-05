#pragma once
#include <QWidget>
#include "app/app_context.h"
#include "core/note.h"

class QTextEdit;
class QLabel;
class QPushButton;
class QTimer;

namespace stickynotes::ui {
class StickyWindow : public QWidget {
    Q_OBJECT
public:
    explicit StickyWindow(app::AppContext& ctx, const core::Note& note, QWidget* parent = nullptr);
    ~StickyWindow();
    core::Note note() const;
    void setNote(const core::Note& n);
protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void closeEvent(QCloseEvent* e) override;
private slots:
    void onPinToggled(bool checked);
    void onClose();
    void onSave();
    void startFlash();
    void stopFlash();
private:
    void buildUi();
    void applyStyle();
    void savePosition();
    void loadPosition();

    app::AppContext& ctx_;
    core::Note note_;
    QPoint dragPos_;

    // UI
    QWidget* titleBar_ = nullptr;
    QLabel* titleLabel_ = nullptr;
    QPushButton* pinBtn_ = nullptr;
    QPushButton* closeBtn_ = nullptr;
    QTextEdit* editor_ = nullptr;

    QTimer* flashTimer_ = nullptr;
    bool isFlashing_ = false;
    int flashCount_ = 0;
};
}