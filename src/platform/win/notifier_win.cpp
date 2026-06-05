#include "platform/win/notifier_win.h"
#include <QSystemTrayIcon>

namespace stickynotes::platform::win {
struct Notifier_Win::Impl {
    QSystemTrayIcon* tray = nullptr;
};

Notifier_Win::Notifier_Win() : d_(new Impl) {
    d_->tray = new QSystemTrayIcon();
}

Notifier_Win::~Notifier_Win() {
    delete d_->tray;
}

void Notifier_Win::show(const Notification& n) {
    d_->tray->showMessage(n.title, n.body, QSystemTrayIcon::Information, 5000);
}
}
