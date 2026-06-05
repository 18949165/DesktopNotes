#pragma once
#include "platform/inotifier.h"
#include <memory>

class QSystemTrayIcon;

namespace stickynotes::platform::win {
class Notifier_Win final : public INotifier {
public:
    Notifier_Win();
    ~Notifier_Win();
    void show(const Notification& n) override;
private:
    struct Impl;
    std::unique_ptr<Impl> d_;
};
}
