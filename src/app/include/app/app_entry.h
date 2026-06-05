#pragma once
#include "app/app_context.h"
#include <QApplication>

namespace stickynotes::app {
class AppEntry {
public:
    explicit AppEntry(AppContext& ctx, QApplication& app);
    void run();
private:
    AppContext& ctx_;
    QApplication& app_;
};
}
