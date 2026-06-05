#pragma once
#include "platform/iclock.h"

namespace stickynotes::platform {
class SystemClock final : public IClock {
public:
    QDateTime now() const override;
};
}
