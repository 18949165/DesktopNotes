#include <gtest/gtest.h>
#include <QApplication>
#include <QKeyEvent>
#include "ui/hotkey_edit.h"

using stickynotes::ui::HotkeyEdit;

static void sendKey(HotkeyEdit& w, int key, Qt::KeyboardModifiers mods = Qt::NoModifier) {
    QKeyEvent* ev = new QKeyEvent(QEvent::KeyPress, key, mods);
    QApplication::sendEvent(&w, ev);
    delete ev;
}

TEST(HotkeyEdit, CapturesSingleMainKey) {
    HotkeyEdit w;
    w.setKeySequence("");
    w.show();
    w.setFocus();
    QApplication::processEvents();
    sendKey(w, Qt::Key_N);
    EXPECT_EQ(w.keySequence(), QString("N"));
    w.close();
}

TEST(HotkeyEdit, CapturesModifierPlusKey) {
    HotkeyEdit w;
    w.setKeySequence("");
    w.show();
    w.setFocus();
    QApplication::processEvents();
    sendKey(w, Qt::Key_N, Qt::ControlModifier | Qt::AltModifier);
    QString got = w.keySequence();
    EXPECT_TRUE(got.contains("Ctrl")) << "got=" << got.toStdString();
    EXPECT_TRUE(got.contains("Alt"))  << "got=" << got.toStdString();
    EXPECT_TRUE(got.contains("N"))    << "got=" << got.toStdString();
    w.close();
}

TEST(HotkeyEdit, EscapeAbortsCapture) {
    HotkeyEdit w;
    w.setKeySequence("Original");
    w.show();
    w.setFocus();
    QApplication::processEvents();
    // Press only a modifier, then Escape: no commit, value stays "Original"
    sendKey(w, Qt::Key_Control);
    sendKey(w, Qt::Key_Escape);
    EXPECT_EQ(w.keySequence(), QString("Original"));
    w.close();
}

TEST(HotkeyEdit, BackspaceClears) {
    HotkeyEdit w;
    w.setKeySequence("Ctrl+Alt+N");
    w.show();
    w.setFocus();
    QApplication::processEvents();
    sendKey(w, Qt::Key_Backspace);
    EXPECT_EQ(w.keySequence(), QString());
    w.close();
}

// Custom main: gtest_main doesn't initialise QApplication.
int main(int argc, char** argv) {
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
