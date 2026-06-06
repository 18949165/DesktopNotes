#pragma once
#include <QWidget>
#include "app/app_context.h"
#include "core/note.h"

class ElaPlainTextEdit;
class ElaLineEdit;
class ElaToolButton;
class ElaText;
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
    bool eventFilter(QObject* obj, QEvent* e) override;
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

    // UI（全 Ela 控件）
    QWidget* titleBar_ = nullptr;
    ElaText* titleLabel_ = nullptr;
    ElaLineEdit* titleEdit_ = nullptr;          // 双击标题切换为编辑
    ElaToolButton* pinBtn_ = nullptr;
    ElaToolButton* closeBtn_ = nullptr;
    ElaPlainTextEdit* editor_ = nullptr;        // 纯文本编辑

    QTimer* flashTimer_ = nullptr;
    bool isFlashing_ = false;
    int flashCount_ = 0;
};
}
