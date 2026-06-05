#include "ui/settings_dialog.h"
#include <ElaLineEdit.h>
#include <ElaComboBox.h>
#include <ElaToggleSwitch.h>
#include <ElaPushButton.h>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace stickynotes::ui {
SettingsDialog::SettingsDialog(app::AppContext& ctx, QWidget* parent) 
    : ElaDialog(parent), ctx_(ctx) {
    setWindowTitle("设置");
    buildUi();
    wireSignals();
}

void SettingsDialog::buildUi() {
    themeCombo_ = new ElaComboBox(this);
    themeCombo_->addItems({"浅色", "深色", "跟随系统"});
    
    hotkeyEdit_ = new ElaLineEdit(this);
    hotkeyEdit_->setText("Ctrl+Alt+N");
    
    soundSwitch_ = new ElaToggleSwitch(this);
    
    okBtn_ = new ElaPushButton("确定", this);
    cancelBtn_ = new ElaPushButton("取消", this);
    
    auto* form = new QFormLayout();
    form->addRow("主题", themeCombo_);
    form->addRow("新建便签快捷键", hotkeyEdit_);
    form->addRow("启用声音", soundSwitch_);
    
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn_);
    btnLayout->addWidget(cancelBtn_);
    
    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addLayout(btnLayout);
}

void SettingsDialog::wireSignals() {
    connect(okBtn_, &ElaPushButton::clicked, this, [this]() {
        int idx = themeCombo_->currentIndex();
        ctx_.settings->theme = static_cast<core::Settings::Theme>(idx);
        ctx_.settings->save(*ctx_.fs);
        accept();
    });
    
    connect(cancelBtn_, &ElaPushButton::clicked, this, &SettingsDialog::reject);
}
}
