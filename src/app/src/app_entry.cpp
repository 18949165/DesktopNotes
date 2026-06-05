#include "app/app_entry.h"
#include "app/reminder_service.h"

namespace stickynotes::app {
AppEntry::AppEntry(AppContext& ctx, QApplication& app)
    : ctx_(ctx), app_(app) {}

void AppEntry::run() {
    if (ctx_.reminders) ctx_.reminders->start();
    if (ctx_.tray) ctx_.tray->show();
}
}
