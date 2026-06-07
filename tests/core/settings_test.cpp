#include <gtest/gtest.h>
#include "core/settings.h"
#include "core/fake_filesystem.h"
using stickynotes::core::Settings;
using stickynotes::testing::FakeFileSystem;

TEST(Settings, DefaultsWhenMissing) {
    FakeFileSystem fs;
    auto s = Settings::load("/missing.json", fs);
    EXPECT_EQ(s.hotkey, QString("Ctrl+Alt+N"));
    EXPECT_TRUE(s.dataDir.isEmpty());
}

TEST(Settings, RoundTrip) {
    FakeFileSystem fs;
    Settings s; s.hotkey = "Ctrl+Alt+X";
    s.dataDir = "/d"; ASSERT_TRUE(s.save(fs));
    auto s2 = Settings::load("/d/settings.json", fs);
    EXPECT_EQ(s2.hotkey, QString("Ctrl+Alt+X"));
    EXPECT_EQ(s2.dataDir, QString("/d"));
}

TEST(Settings, InvalidJsonFallsBack) {
    FakeFileSystem fs;
    fs.data["/bad.json"] = "not json";
    auto s = Settings::load("/bad.json", fs);
    EXPECT_EQ(s.hotkey, QString("Ctrl+Alt+N"));
}

TEST(Settings, SaveFailure) {
    FakeFileSystem fs;
    fs.failWrites = true;
    Settings s; s.dataDir = "/d";
    EXPECT_FALSE(s.save(fs));
}
