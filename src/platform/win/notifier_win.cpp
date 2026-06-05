#include "platform/win/notifier_win.h"
#include <QDebug>

namespace stickynotes::platform::win {
void Notifier_Win::show(const Notification& n) {
    qDebug().noquote() << "[Notify]" << n.title << "-" << n.body;
}
}
