#include <gtest/gtest.h>
#include "core/note.h"
#include "core/category.h"
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
    n.remindAt = QDateTime::fromString("2026-06-10T09:00:00", Qt::ISODate);
    n.pinned = true;
    n.windowGeometry = QRect(10, 20, 300, 200);

    auto o = n.toJson();
    auto n2 = Note::fromJson(o);
    EXPECT_EQ(n2.id, n.id);
    EXPECT_EQ(n2.title, n.title);
    EXPECT_EQ(n2.content, n.content);
    EXPECT_EQ(n2.categoryId, n.categoryId);
    EXPECT_EQ(n2.tags, n.tags);
    EXPECT_EQ(n2.remindAt, n.remindAt);
    EXPECT_TRUE(n2.pinned);
    EXPECT_EQ(n2.windowGeometry, n.windowGeometry);
}

TEST(Note, EmptyRemindAtStaysInvalid) {
    Note n;
    n.id = "x";
    auto o = n.toJson();
    auto n2 = Note::fromJson(o);
    EXPECT_FALSE(n2.remindAt.isValid());
}

TEST(Note, IsValidById) {
    Note n;
    EXPECT_FALSE(n.isValid());
    n.id = "1";
    EXPECT_TRUE(n.isValid());
}

TEST(Category, RoundTrip) {
    Category c;
    c.id = "x"; c.name = "Inbox"; c.color = "#0078D4"; c.parentId = "";
    auto o = c.toJson();
    auto c2 = Category::fromJson(o);
    EXPECT_EQ(c2.id, c.id);
    EXPECT_EQ(c2.name, c.name);
    EXPECT_EQ(c2.color, c.color);
    EXPECT_EQ(c2.parentId, c.parentId);
}
