#pragma once
#include "platform/itrayicon.h"
#include <memory>

class QSystemTrayIcon;
class QMenu;

namespace stickynotes::platform::win {
class TrayIcon_Win final : public ITrayIcon {
public:
    TrayIcon_Win();
    ~TrayIcon_Win();
    void show() override;
    void hide() override;
    void setMenu(const QList<MenuItem>& items) override;
    void setLeftClickCallback(LeftClickCb cb) override;
private:
    struct Impl;
    std::unique_ptr<Impl> d_;
    LeftClickCb cb_;
};
}
