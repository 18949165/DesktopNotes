#pragma once
#include "platform/ihotkey.h"
#include <QList>

namespace stickynotes::platform::win {
class Hotkey_Win final : public IHotkey {
public:
    Hotkey_Win();
    ~Hotkey_Win() override;
    bool registerHotkey(const Spec& s) override;
    bool unregisterHotkey(int id) override;
    void setTriggeredCallback(TriggeredCb cb) override;
private:
    TriggeredCb cb_;
    QList<Spec> specs_;
};
}
