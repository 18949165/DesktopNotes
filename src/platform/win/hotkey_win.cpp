#include "platform/win/hotkey_win.h"

namespace stickynotes::platform::win {
bool Hotkey_Win::registerHotkey(const Spec&) { return true; }
bool Hotkey_Win::unregisterHotkey(int) { return true; }
void Hotkey_Win::setTriggeredCallback(TriggeredCb cb) { cb_ = std::move(cb); }
}
