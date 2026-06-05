#pragma once
#include "platform/itrayicon.h"

namespace stickynotes::platform::win {
class TrayIcon_Win final : public ITrayIcon {
public:
    void show() override;
    void hide() override;
    void setMenu(const QList<MenuItem>& items) override;
    void setLeftClickCallback(LeftClickCb cb) override;
private:
    LeftClickCb cb_;
};
}
