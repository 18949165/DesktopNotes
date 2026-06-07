#include <gtest/gtest.h>
#include "platform/ihotkey.h"

using stickynotes::platform::IHotkey;
using Qt::KeyboardModifier;

TEST(IHotkeyParseSpec, CtrlAltN) {
    auto s = IHotkey::parseSpec("Ctrl+Alt+N", 1);
    EXPECT_EQ(s.id, 1);
    EXPECT_EQ(s.mods, Qt::ControlModifier | Qt::AltModifier);
    EXPECT_EQ(s.key, Qt::Key_N);
}

TEST(IHotkeyParseSpec, F5) {
    auto s = IHotkey::parseSpec("F5", 2);
    EXPECT_EQ(s.id, 2);
    EXPECT_EQ(s.mods, Qt::NoModifier);
    EXPECT_EQ(s.key, Qt::Key_F5);
}

TEST(IHotkeyParseSpec, ShiftF1) {
    auto s = IHotkey::parseSpec("Shift+F1", 1);
    EXPECT_EQ(s.mods, Qt::ShiftModifier);
    EXPECT_EQ(s.key, Qt::Key_F1);
}

TEST(IHotkeyParseSpec, MetaAltX) {
    auto s = IHotkey::parseSpec("Meta+Alt+X", 1);
    EXPECT_EQ(s.mods, Qt::MetaModifier | Qt::AltModifier);
    EXPECT_EQ(s.key, Qt::Key_X);
}

TEST(IHotkeyParseSpec, EmptyReturnsUnknown) {
    auto s = IHotkey::parseSpec("", 1);
    EXPECT_EQ(s.key, Qt::Key_unknown);
    EXPECT_EQ(s.mods, Qt::NoModifier);
}

TEST(IHotkeyParseSpec, InvalidStringReturnsUnknown) {
    auto s = IHotkey::parseSpec("+++", 1);
    EXPECT_EQ(s.key, Qt::Key_unknown);
}

TEST(IHotkeyParseSpec, DefaultIdIsOne) {
    auto s = IHotkey::parseSpec("Ctrl+N");
    EXPECT_EQ(s.id, 1);
}
