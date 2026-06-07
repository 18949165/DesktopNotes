#include <gtest/gtest.h>
#include "core/note.h"
using namespace stickynotes::core;

TEST(Note, RoundTrip) {
    Note n;
    n.id = "abc";
    n.title = "T";
    n.content = "hello";
    n.categoryId = "inbox";
    n.tags = {"a", "b"};
    n.createdAt = QDateTime::fromString("2026-06-05T10:00:00", Qt::ISODate);
    n.updatedAt = n.createdAt;
    n.pinned = true;
    n.windowGeometry = QRect(10, 20, 300, 200);

    auto o = n.toJson();
    auto n2 = Note::fromJson(o);
    EXPECT_EQ(n2.id, n.id);
    EXPECT_EQ(n2.title, n.title);
    EXPECT_EQ(n2.content, n.content);
    EXPECT_EQ(n2.categoryId, n.categoryId);
    EXPECT_EQ(n2.tags, n.tags);
    EXPECT_TRUE(n2.pinned);
    EXPECT_EQ(n2.windowGeometry, n.windowGeometry);
}

TEST(Note, IsValidById) {
    Note n;
    EXPECT_FALSE(n.isValid());
    n.id = "1";
    EXPECT_TRUE(n.isValid());
}
