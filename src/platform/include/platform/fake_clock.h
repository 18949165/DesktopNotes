#pragma once
#include "platform/iclock.h"
#include <chrono>
#include <utility>

namespace stickynotes::platform {
class FakeClock final : public IClock {
public:
    explicit FakeClock(QDateTime t) : t_(std::move(t)) {}
    QDateTime now() const override { return t_; }
    void advance(std::chrono::seconds s) { t_ = t_.addSecs(s.count()); }
private:
    QDateTime t_;
};
}
