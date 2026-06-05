#include <gtest/gtest.h>
#include "mocks.h"

using stickynotes::platform::MockHotkey;
using testing::Return;

TEST(MockHotkey, UnregisterSequence) {
    MockHotkey h;
    EXPECT_CALL(h, unregisterHotkey(1))
        .Times(2)
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    EXPECT_TRUE(h.unregisterHotkey(1));
    EXPECT_FALSE(h.unregisterHotkey(1));
}

TEST(MockHotkey, CallbackStored) {
    MockHotkey h;
    EXPECT_CALL(h, setTriggeredCallback(testing::_)).Times(1);
    h.setTriggeredCallback([](int){});
}
