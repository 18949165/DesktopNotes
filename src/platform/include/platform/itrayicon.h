#pragma once
#include <QList>
#include <QString>
#include <functional>

namespace stickynotes::platform {
class ITrayIcon {
public:
    struct MenuItem {
        QString text;
        std::function<void()> onClick;
    };
    using LeftClickCb = std::function<void()>;

    virtual ~ITrayIcon() = default;
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual void setMenu(const QList<MenuItem>& items) = 0;
    virtual void setLeftClickCallback(LeftClickCb cb) = 0;
};
}
