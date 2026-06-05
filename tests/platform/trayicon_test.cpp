#include <gtest/gtest.h>
#include "mocks.h"

using stickynotes::platform::MockTrayIcon;

TEST(MockTrayIcon, ShowHide) {
    MockTrayIcon t;
    EXPECT_CALL(t, show()).Times(1);
    EXPECT_CALL(t, hide()).Times(1);
    t.show();
    t.hide();
}

TEST(MockTrayIcon, CallbackStored) {
    MockTrayIcon t;
    EXPECT_CALL(t, setLeftClickCallback(testing::_)).Times(1);
    t.setLeftClickCallback([](){});
}
