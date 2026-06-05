#pragma once
#include <gmock/gmock.h>
#include "platform/iclock.h"

namespace stickynotes::platform {
class MockClock : public IClock {
public:
    MOCK_METHOD(QDateTime, now, (), (const, override));
};
}
