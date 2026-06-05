#include "platform/win/trayicon_win.h"

namespace stickynotes::platform::win {
void TrayIcon_Win::show() {}
void TrayIcon_Win::hide() {}
void TrayIcon_Win::setMenu(const QList<MenuItem>&) {}
void TrayIcon_Win::setLeftClickCallback(LeftClickCb cb) { cb_ = std::move(cb); }
}
