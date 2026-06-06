#pragma once
#include <Qt>
#include <QString>
#include <QKeySequence>
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

    // Convenience: parse a Qt-style string like "Ctrl+Alt+N" / "F5" into Spec.
    // Returns Spec{id=0, mods=NoModifier, key=Key_unknown} on parse failure.
    static Spec parseSpec(const QString& s, int id = 1) {
        Spec spec{ id, Qt::NoModifier, Qt::Key_unknown };
        QKeySequence ks(s);
        if (ks.isEmpty()) return spec;
        // QKeySequence stores the first key+mods internally (Qt6 returns
        // a QKeyCombination from operator[]; it has .key() and .keyboardModifiers()).
        const QKeyCombination kc = ks[0];
        spec.mods = kc.keyboardModifiers();
        spec.key  = kc.key();
        return spec;
    }
};
}
