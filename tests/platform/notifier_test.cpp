#include <gtest/gtest.h>
#include "mocks.h"

using stickynotes::platform::MockNotifier;
using testing::Truly;

TEST(MockNotifier, ShowReceivesFields) {
    MockNotifier n;
    EXPECT_CALL(n, show(Truly([](const auto& x) {
        return x.title == "t" && x.body == "b" && x.id == "1";
    })));
    n.show({"t", "b", "1"});
}

TEST(MockNotifier, NoCallsIsOk) {
    MockNotifier n;
    EXPECT_CALL(n, show(testing::_)).Times(0);
    SUCCEED();
}
