#include "platform/system_clock.h"

namespace stickynotes::platform {
QDateTime SystemClock::now() const {
    return QDateTime::currentDateTime();
}
}
