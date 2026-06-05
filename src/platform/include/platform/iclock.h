#pragma once
#include <QDateTime>

namespace stickynotes::platform {
class IClock {
public:
    virtual ~IClock() = default;
    virtual QDateTime now() const = 0;
};
}
