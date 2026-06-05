#pragma once
#include "platform/inotifier.h"

namespace stickynotes::platform::win {
class Notifier_Win final : public INotifier {
public:
    void show(const Notification& n) override;
};
}
