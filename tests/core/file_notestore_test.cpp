#include <gtest/gtest.h>
#include "core/file_notestore.h"
#include "core/fake_filesystem.h"

using stickynotes::core::FileNoteStore;
using stickynotes::testing::FakeFileSystem;

TEST(FileNoteStore, CreatePersistsAndAppearsInAll) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    auto n = s.create("inbox");
    EXPECT_TRUE(s.get(n.id).has_value());
    EXPECT_EQ(s.all().size(), 1);
    EXPECT_TRUE(fs.data.contains("/data/notes/" + n.id + ".md"));
}

TEST(FileNoteStore, ViewLockFirstAcquireWins) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    auto n = s.create("");
    EXPECT_TRUE(s.acquire(n.id));
    EXPECT_FALSE(s.acquire(n.id));
    EXPECT_FALSE(s.isWritable(n.id));
    s.release(n.id);                  // 2 -> 1
    EXPECT_FALSE(s.isWritable(n.id)); // 仍有 1 个持锁
    s.release(n.id);                  // 1 -> 0
    EXPECT_TRUE(s.isWritable(n.id));
}

TEST(FileNoteStore, CorruptMdIsSkipped) {
    FakeFileSystem fs;
    fs.data["/data/notes/bad.md"] = "no frontmatter";
    FileNoteStore s("/data", fs);
    EXPECT_EQ(s.all().size(), 0);
}

TEST(FileNoteStore, UpsertUpdatesFile) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    auto n = s.create("");
    n.title = "new";
    s.upsert(n);
    EXPECT_TRUE(fs.data["/data/notes/" + n.id + ".md"].contains("new"));
}

TEST(FileNoteStore, QueryFiltersByCategory) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    auto a = s.create("work");
    auto b = s.create("home");
    EXPECT_EQ(s.query("", "work").size(), 1);
    EXPECT_EQ(s.query("", "").size(), 2);
}

TEST(FileNoteStore, InboxCategoryAlwaysPresent) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    EXPECT_EQ(s.categories().size(), 1);
    EXPECT_EQ(s.categories().first().id, "inbox");
}

TEST(FileNoteStore, CannotRemoveInbox) {
    FakeFileSystem fs;
    FileNoteStore store("/data", fs);
    EXPECT_FALSE(store.removeCategory("inbox"));
}

TEST(FileNoteStore, SoftDeleteHidesFromAllAndShowsInTrash) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    auto a = s.create("inbox");
    auto b = s.create("inbox");
    EXPECT_TRUE(s.softDelete(a.id));
    EXPECT_EQ(s.all().size(), 1);
    EXPECT_EQ(s.trash().size(), 1);
    EXPECT_TRUE(s.trash().first().id == a.id);
    // 文件应保留
    EXPECT_TRUE(fs.data.contains("/data/notes/" + a.id + ".md"));
}

TEST(FileNoteStore, RestoreBringsBackToAll) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    auto n = s.create("inbox");
    s.softDelete(n.id);
    EXPECT_EQ(s.all().size(), 0);
    EXPECT_TRUE(s.restore(n.id));
    EXPECT_EQ(s.all().size(), 1);
    EXPECT_EQ(s.trash().size(), 0);
}

TEST(FileNoteStore, PermanentDeleteRemovesFile) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    auto n = s.create("inbox");
    EXPECT_TRUE(s.permanentDelete(n.id));
    EXPECT_EQ(s.all().size(), 0);
    EXPECT_EQ(s.trash().size(), 0);
    EXPECT_FALSE(fs.data.contains("/data/notes/" + n.id + ".md"));
}
