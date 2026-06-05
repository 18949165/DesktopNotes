#include "app/app_context.h"
#include "app/reminder_service.h"
#include "platform/filesystem_qt.h"
#include "platform/system_clock.h"
#include "platform/win/notifier_win.h"
#include "platform/win/hotkey_win.h"
#include "platform/win/trayicon_win.h"
#include "core/file_notestore.h"
#include "core/settings.h"
#include <QDir>

namespace stickynotes::app {
AppContext::AppContext() = default;
AppContext::~AppContext() = default;
AppContext::AppContext(AppContext&&) noexcept = default;
AppContext& AppContext::operator=(AppContext&&) noexcept = default;

AppContext AppContext::production(const QString& dataDir) {
    AppContext c;
    c.fs = std::make_unique<platform::FileSystem_Qt>();
    c.clock = std::make_unique<platform::SystemClock>();
    c.notifier = std::make_unique<platform::win::Notifier_Win>();
    c.hotkey = std::make_unique<platform::win::Hotkey_Win>();
    c.tray = std::make_unique<platform::win::TrayIcon_Win>();
    c.settings = std::make_unique<core::Settings>(
        core::Settings::load(dataDir + "/settings.json", *c.fs));
    c.settings->dataDir = dataDir;
    c.notes = std::make_unique<core::FileNoteStore>(dataDir, *c.fs);
    c.reminders = std::make_unique<ReminderService>(*c.notes, *c.clock, *c.notifier);
    return c;
}
}
