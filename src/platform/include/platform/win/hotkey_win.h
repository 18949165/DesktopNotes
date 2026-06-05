#pragma once
#include "platform/ihotkey.h"

namespace stickynotes::platform::win {
class Hotkey_Win final : public IHotkey {
public:
    bool registerHotkey(const Spec& s) override;
    bool unregisterHotkey(int id) override;
    void setTriggeredCallback(TriggeredCb cb) override;
private:
    TriggeredCb cb_;
};
}
