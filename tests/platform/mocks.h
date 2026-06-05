#pragma once
#include <gmock/gmock.h>
#include "platform/inotifier.h"
#include "platform/ihotkey.h"
#include "platform/itrayicon.h"

namespace stickynotes::platform {
class MockNotifier : public INotifier {
public:
    MOCK_METHOD(void, show, (const Notification&), (override));
};

class MockHotkey : public IHotkey {
public:
    MOCK_METHOD(bool, registerHotkey, (const Spec&), (override));
    MOCK_METHOD(bool, unregisterHotkey, (int), (override));
    MOCK_METHOD(void, setTriggeredCallback, (TriggeredCb), (override));
};

class MockTrayIcon : public ITrayIcon {
public:
    MOCK_METHOD(void, show, (), (override));
    MOCK_METHOD(void, hide, (), (override));
    MOCK_METHOD(void, setMenu, (const QList<MenuItem>&), (override));
    MOCK_METHOD(void, setLeftClickCallback, (LeftClickCb), (override));
};
}
