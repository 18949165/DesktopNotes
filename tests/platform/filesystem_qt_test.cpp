#include <gtest/gtest.h>
#include "platform/ifilesystem.h"
#include "platform/filesystem_qt.h"
#include <QStandardPaths>
#include <QTemporaryDir>

using stickynotes::platform::FileSystem_Qt;
using stickynotes::platform::IFileSystem;

TEST(FileSystem_Qt, AtomicWriteAndRead) {
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    FileSystem_Qt fs;
    const QString p = tmp.filePath("a.txt");
    ASSERT_TRUE(fs.writeAtomic(p, QByteArray("hello")));
    EXPECT_TRUE(fs.exists(p));
    auto data = fs.readAll(p);
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(*data, QByteArray("hello"));
}

TEST(FileSystem_Qt, EnsureDirIdempotent) {
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    FileSystem_Qt fs;
    const QString d = tmp.filePath("x/y/z");
    EXPECT_TRUE(fs.ensureDir(d));
    EXPECT_TRUE(fs.ensureDir(d));
    EXPECT_TRUE(fs.exists(d));
}

TEST(FileSystem_Qt, ReadMissingReturnsNullopt) {
    QTemporaryDir tmp;
    FileSystem_Qt fs;
    auto data = fs.readAll(tmp.filePath("nope.txt"));
    EXPECT_FALSE(data.has_value());
}

TEST(FileSystem_Qt, OverwritePreserves) {
    QTemporaryDir tmp;
    FileSystem_Qt fs;
    const QString p = tmp.filePath("b.txt");
    ASSERT_TRUE(fs.writeAtomic(p, "v1"));
    ASSERT_TRUE(fs.writeAtomic(p, "v2-longer"));
    auto data = fs.readAll(p);
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(*data, QByteArray("v2-longer"));
}
