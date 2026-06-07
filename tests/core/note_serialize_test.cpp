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

TEST(Note, FromJsonContentMdFallback) {
    QJsonObject o;
    o["id"] = "abc";
    o["contentMd"] = "legacy markdown";
    auto n = Note::fromJson(o);
    EXPECT_EQ(n.content, "legacy markdown");
}

TEST(Note, FromJsonContentPreferredOverMd) {
    QJsonObject o;
    o["id"] = "abc";
    o["content"] = "new content";
    o["contentMd"] = "old content";
    auto n = Note::fromJson(o);
    EXPECT_EQ(n.content, "new content");
}

TEST(Note, FromJsonTitleFallsBackToContentFirstLine) {
    QJsonObject o;
    o["id"] = "abc";
    o["content"] = "First line\nSecond line";
    auto n = Note::fromJson(o);
    EXPECT_EQ(n.title, "First line");
}

TEST(Note, FromJsonTitleNotOverwrittenIfNotEmpty) {
    QJsonObject o;
    o["id"] = "abc";
    o["title"] = "Existing";
    o["content"] = "Should not override";
    auto n = Note::fromJson(o);
    EXPECT_EQ(n.title, "Existing");
}

TEST(Note, FromJsonReadsDeletedAt) {
    QJsonObject o;
    o["id"] = "abc";
    o["deletedAt"] = "2026-06-06T12:00:00";
    auto n = Note::fromJson(o);
    EXPECT_TRUE(n.deletedAt.isValid());
}

TEST(Note, FromJsonMissingFields) {
    QJsonObject o;
    o["id"] = "abc";
    auto n = Note::fromJson(o);
    EXPECT_TRUE(n.title.isEmpty());
    EXPECT_TRUE(n.content.isEmpty());
    EXPECT_TRUE(n.tags.isEmpty());
    EXPECT_FALSE(n.pinned);
    EXPECT_TRUE(n.windowGeometry.isNull());
}

TEST(Note, TagsSerialization) {
    Note n;
    n.id = "abc";
    n.tags = {"work", "urgent", "meeting"};
    auto o = n.toJson();
    auto n2 = Note::fromJson(o);
    EXPECT_EQ(n2.tags.size(), 3);
    EXPECT_TRUE(n2.tags.contains("urgent"));
}

TEST(Note, EmptyTags) {
    Note n;
    n.id = "abc";
    auto o = n.toJson();
    auto n2 = Note::fromJson(o);
    EXPECT_TRUE(n2.tags.isEmpty());
}
