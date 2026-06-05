#pragma once
#include <QDialog>
#include "app/app_context.h"

namespace stickynotes::ui {
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(app::AppContext& ctx, QWidget* parent = nullptr);
private:
    app::AppContext& ctx_;
};
}
