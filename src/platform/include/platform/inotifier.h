#pragma once
#include <QString>

namespace stickynotes::platform {
class INotifier {
public:
    struct Notification {
        QString title;
        QString body;
        QString id;
    };
    virtual ~INotifier() = default;
    virtual void show(const Notification& n) = 0;
};
}
