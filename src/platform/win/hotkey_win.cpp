#include "platform/win/hotkey_win.h"
#include <QAbstractNativeEventFilter>
#include <QGuiApplication>
#include <windows.h>

namespace stickynotes::platform::win {
class HotkeyFilter : public QAbstractNativeEventFilter {
public:
    bool nativeEventFilter(const QByteArray&, void* msg, long*) override {
        auto* m = static_cast<MSG*>(msg);
        if (m->message == WM_HOTKEY) {
            emit hotkey(m->wParam);
        }
        return false;
    }
signals:
    void hotkey(int id);
};

static HotkeyFilter* g_filter = nullptr;
static QList<Hotkey_Win*> g_instances;

Hotkey_Win::Hotkey_Win() {
    if (!g_filter) {
        g_filter = new HotkeyFilter();
        qApp->installNativeEventFilter(g_filter);
    }
    g_instances << this;
    
    QObject::connect(g_filter, &HotkeyFilter::hotkey, this, [this](int id) {
        for (const auto& s : specs_) {
            if (s.id == id && cb_) {
                cb_(id);
                break;
            }
        }
    });
}

Hotkey_Win::~Hotkey_Win() {
    g_instances.removeAll(this);
    for (const auto& s : specs_) {
        ::UnregisterHotKey(nullptr, s.id);
    }
}

bool Hotkey_Win::registerHotkey(const Spec& s) {
    UINT mods = 0;
    if (s.mods & Qt::ControlModifier) mods |= MOD_CONTROL;
    if (s.mods & Qt::AltModifier) mods |= MOD_ALT;
    if (s.mods & Qt::ShiftModifier) mods |= MOD_SHIFT;
    if (s.mods & Qt::MetaModifier) mods |= MOD_WIN;
    
    if (!::RegisterHotKey(nullptr, s.id, mods, s.key)) {
        return false;
    }
    specs_ << s;
    return true;
}

bool Hotkey_Win::unregisterHotkey(int id) {
    if (!::UnregisterHotKey(nullptr, id)) {
        return false;
    }
    specs_.removeIf([id](const Spec& s) { return s.id == id; });
    return true;
}

void Hotkey_Win::setTriggeredCallback(TriggeredCb cb) {
    cb_ = std::move(cb);
}
}
