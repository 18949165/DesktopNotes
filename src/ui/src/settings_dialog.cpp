#include "ui/settings_dialog.h"
#include <ElaLineEdit.h>
#include <ElaPushButton.h>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>

namespace stickynotes::ui {
SettingsDialog::SettingsDialog(app::AppContext& ctx, QWidget* parent)
    : ElaDialog(parent), ctx_(ctx) {
    setWindowTitle("设置");
    buildUi();
    wireSignals();
}

void SettingsDialog::buildUi() {
    // 快捷键：HotkeyEdit 点击后捕获组合键（Ctrl+Alt+N）
    hotkeyEdit_ = new HotkeyEdit(this);
    hotkeyEdit_->setKeySequence(ctx_.settings->hotkey);
    hotkeyEdit_->setMinimumWidth(240);

    dataDirEdit_ = new ElaLineEdit(this);
    dataDirEdit_->setText(ctx_.settings->dataDir);

    browseBtn_ = new ElaPushButton("浏览…", this);

    auto* dataRow = new QHBoxLayout();
    dataRow->addWidget(dataDirEdit_, 1);
    dataRow->addWidget(browseBtn_);

    okBtn_ = new ElaPushButton("确定", this);
    cancelBtn_ = new ElaPushButton("取消", this);

    auto* form = new QFormLayout();
    form->addRow("新建便签快捷键", hotkeyEdit_);
    form->addRow("数据存储目录",   dataRow);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn_);
    btnLayout->addWidget(cancelBtn_);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addLayout(btnLayout);
}

void SettingsDialog::wireSignals() {
    connect(browseBtn_, &ElaPushButton::clicked, this, [this]() {
        const QString dir = QFileDialog::getExistingDirectory(
            this, "选择数据存储目录", dataDirEdit_->text());
        if (!dir.isEmpty()) dataDirEdit_->setText(dir);
    });

    connect(okBtn_, &ElaPushButton::clicked, this, [this]() {
        ctx_.settings->hotkey = hotkeyEdit_->keySequence();
        ctx_.settings->dataDir = dataDirEdit_->text();
        ctx_.settings->save(*ctx_.fs);
        accept();
    });

    connect(cancelBtn_, &ElaPushButton::clicked, this, &SettingsDialog::reject);
}
}
