#include "ui/settings_dialog.h"
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>

namespace stickynotes::ui {
SettingsDialog::SettingsDialog(app::AppContext& ctx, QWidget* p) 
    : QDialog(p), ctx_(ctx) {
    setWindowTitle("设置");
    
    auto* form = new QFormLayout(this);
    
    auto* theme = new QComboBox(this);
    theme->addItems({"Auto", "Light", "Dark"});
    theme->setCurrentIndex((int)ctx_.settings->theme);
    
    auto* hotkey = new QLineEdit(ctx_.settings->hotkey, this);
    
    auto* sound = new QCheckBox("提醒声音", this);
    sound->setChecked(ctx_.settings->soundEnabled);
    
    form->addRow("主题", theme);
    form->addRow("全局快捷键", hotkey);
    form->addRow("", sound);
    
    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addRow(bb);
    
    connect(bb, &QDialogButtonBox::accepted, this, [this, theme, hotkey, sound]() {
        ctx_.settings->theme = (core::Settings::Theme)theme->currentIndex();
        ctx_.settings->hotkey = hotkey->text();
        ctx_.settings->soundEnabled = sound->isChecked();
        ctx_.settings->save(*ctx_.fs);
        accept();
    });
    
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
}
