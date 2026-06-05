#pragma once
#include <Qt>
#include <functional>

namespace stickynotes::platform {
class IHotkey {
public:
    struct Spec {
        int id;
        Qt::KeyboardModifiers mods;
        Qt::Key key;
    };
    using TriggeredCb = std::function<void(int)>;

    virtual ~IHotkey() = default;
    virtual bool registerHotkey(const Spec& s) = 0;
    virtual bool unregisterHotkey(int id) = 0;
    virtual void setTriggeredCallback(TriggeredCb cb) = 0;
};
}
