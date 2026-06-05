#include <gtest/gtest.h>
#include <QDateTime>
#include "platform/fake_clock.h"
#include "mock_clock.h"

using stickynotes::platform::FakeClock;
using stickynotes::platform::MockClock;
using testing::Return;

TEST(FakeClock, ReturnsInitialTime) {
    auto t = QDateTime(QDate(2026, 1, 1), QTime(0, 0));
    FakeClock c(t);
    EXPECT_EQ(c.now(), t);
}

TEST(FakeClock, AdvancesBySeconds) {
    auto t = QDateTime(QDate(2026, 1, 1), QTime(0, 0));
    FakeClock c(t);
    c.advance(std::chrono::hours(24));
    EXPECT_EQ(c.now().date().day(), 2);
}

TEST(MockClock, StubsNow) {
    MockClock c;
    auto stubbed = QDateTime::fromString("2026-06-05T10:00:00", Qt::ISODate);
    EXPECT_CALL(c, now()).WillOnce(Return(stubbed));
    EXPECT_EQ(c.now(), stubbed);
}

TEST(MockClock, ExpectsMultipleCalls) {
    MockClock c;
    EXPECT_CALL(c, now()).Times(2)
        .WillOnce(Return(QDateTime::fromString("2026-01-01T00:00:00", Qt::ISODate)))
        .WillOnce(Return(QDateTime::fromString("2026-12-31T23:59:59", Qt::ISODate)));
    EXPECT_NE(c.now(), c.now());
}
