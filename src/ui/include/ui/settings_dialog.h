#pragma once
#include <ElaDialog.h>
#include "app/app_context.h"
#include "core/settings.h"

class ElaLineEdit;
class ElaComboBox;
class ElaToggleSwitch;
class ElaPushButton;

namespace stickynotes::ui {
class SettingsDialog : public ElaDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(app::AppContext& ctx, QWidget* parent = nullptr);
private:
    void buildUi();
    void wireSignals();
    
    app::AppContext& ctx_;
    ElaLineEdit* hotkeyEdit_ = nullptr;
    ElaComboBox* themeCombo_ = nullptr;
    ElaToggleSwitch* soundSwitch_ = nullptr;
    ElaPushButton* okBtn_ = nullptr;
    ElaPushButton* cancelBtn_ = nullptr;
};
}