#pragma once
#include <ElaDialog.h>
#include "app/app_context.h"
#include "core/settings.h"
#include "ui/hotkey_edit.h"

class ElaLineEdit;
class ElaPushButton;
namespace stickynotes::ui {
class HotkeyEdit;
class SettingsDialog : public ElaDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(app::AppContext& ctx, QWidget* parent = nullptr);
private:
    void buildUi();
    void wireSignals();

    app::AppContext& ctx_;
    HotkeyEdit* hotkeyEdit_ = nullptr;     // 弹窗捕获按键
    ElaLineEdit* dataDirEdit_ = nullptr;
    ElaPushButton* browseBtn_ = nullptr;
    ElaPushButton* okBtn_ = nullptr;
    ElaPushButton* cancelBtn_ = nullptr;
};
}
