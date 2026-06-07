#include <gtest/gtest.h>
#include <QJsonDocument>
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

TEST(FileNoteStore, QueryExcludesSoftDeleted) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    auto a = s.create("inbox");
    auto b = s.create("inbox");
    s.softDelete(a.id);
    // query("") 用于列表"全部"视图：必须排除已软删
    EXPECT_EQ(s.query("").size(), 1);
    EXPECT_EQ(s.query("").first().id, b.id);
    // 带关键词也是
    EXPECT_EQ(s.query("", "inbox").size(), 1);
}

// ── 边界条件 ──────────────────────────────────────────

TEST(FileNoteStore, SoftDeleteNonExistent) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    EXPECT_FALSE(s.softDelete("nonexistent"));
}

TEST(FileNoteStore, RestoreNonExistent) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    EXPECT_FALSE(s.restore("nonexistent"));
}

TEST(FileNoteStore, RestoreNonDeleted) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    auto n = s.create("inbox");
    // restore 对非软删笔记也有效（仅清空 deletedAt）
    EXPECT_TRUE(s.restore(n.id));
    EXPECT_EQ(s.all().size(), 1);
}

TEST(FileNoteStore, PermanentDeleteNonExistent) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    EXPECT_FALSE(s.permanentDelete("nonexistent"));
}

TEST(FileNoteStore, UpdateNonExistent) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    Note n; n.id = "phantom";
    // upsert 对不存在 id 也执行插入/更新
    s.upsert(n);
    EXPECT_TRUE(s.get(n.id).has_value());
}

TEST(FileNoteStore, QueryByKeyword) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    auto a = s.create("inbox");
    a.title = "Meeting notes";
    a.content = "Discuss Q3 planning";
    s.upsert(a);
    auto b = s.create("inbox");
    b.title = "Shopping list";
    b.content = "Milk and eggs";
    s.upsert(b);

    EXPECT_EQ(s.query("Meeting").size(), 1);
    EXPECT_EQ(s.query("list").size(), 1);
    EXPECT_EQ(s.query("Q3").size(), 1);
    EXPECT_EQ(s.query("nonexistent").size(), 0);
    EXPECT_EQ(s.query("").size(), 2);             // 空关键词=全部
}

TEST(FileNoteStore, QueryKeywordMatchesContent) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    auto n = s.create("inbox");
    n.content = "Important reminder for tomorrow";
    s.upsert(n);
    EXPECT_EQ(s.query("reminder").size(), 1);
}

TEST(FileNoteStore, QueryCaseInsensitive) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    auto n = s.create("inbox");
    n.title = "Project Alpha";
    s.upsert(n);
    EXPECT_EQ(s.query("alpha").size(), 1);
    EXPECT_EQ(s.query("ALPHA").size(), 1);
}

TEST(FileNoteStore, LoadIndexMultipleValidFiles) {
    FakeFileSystem fs;
    // 预填两个合法笔记文件
    Note n1; n1.id = "id1"; n1.title = "First";
    Note n2; n2.id = "id2"; n2.title = "Second";
    QByteArray fm1 = QJsonDocument(n1.toJson()).toJson(QJsonDocument::Compact);
    QByteArray fm2 = QJsonDocument(n2.toJson()).toJson(QJsonDocument::Compact);
    fs.data["/data/notes/id1.md"] = "---\n" + fm1 + "\n---\nbody1";
    fs.data["/data/notes/id2.md"] = "---\n" + fm2 + "\n---\nbody2";

    FileNoteStore s("/data", fs);
    EXPECT_EQ(s.all().size(), 2);
}

TEST(FileNoteStore, LoadIndexMixedValidAndCorrupt) {
    FakeFileSystem fs;
    Note n; n.id = "good"; n.title = "Good";
    QByteArray fm = QJsonDocument(n.toJson()).toJson(QJsonDocument::Compact);
    fs.data["/data/notes/good.md"]   = "---\n" + fm + "\n---\nbody";
    fs.data["/data/notes/bad1.md"]    = "no frontmatter";
    fs.data["/data/notes/bad2.md"]    = "---\nno closing\n";

    FileNoteStore s("/data", fs);
    EXPECT_EQ(s.all().size(), 1);
    EXPECT_TRUE(s.get("good").has_value());
}

TEST(FileNoteStore, CallbackFiredOnCreate) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    QString changedId;
    s.setNoteChangedCallback([&](const QString& id) { changedId = id; });
    auto n = s.create("inbox");
    EXPECT_EQ(changedId, n.id);
}

TEST(FileNoteStore, CallbackFiredOnUpsert) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    QString changedId;
    s.setNoteChangedCallback([&](const QString& id) { changedId = id; });
    auto n = s.create("inbox");
    changedId.clear();
    s.upsert(n);
    EXPECT_EQ(changedId, n.id);
}

TEST(FileNoteStore, CallbackFiredOnSoftDelete) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    QString changedId;
    auto n = s.create("inbox");
    s.setNoteChangedCallback([&](const QString& id) { changedId = id; });
    s.softDelete(n.id);
    EXPECT_EQ(changedId, n.id);
}

TEST(FileNoteStore, EmptyStore) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    EXPECT_EQ(s.all().size(), 0);
    EXPECT_EQ(s.trash().size(), 0);
    EXPECT_EQ(s.query("").size(), 0);
    EXPECT_FALSE(s.get("nonexistent").has_value());
}

TEST(FileNoteStore, GetReturnsNulloptForNonExistent) {
    FakeFileSystem fs;
    FileNoteStore s("/data", fs);
    EXPECT_FALSE(s.get("phantom").has_value());
}
