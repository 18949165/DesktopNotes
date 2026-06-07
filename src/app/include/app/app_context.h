#pragma once
#include <memory>
#include <QString>
#include "platform/ifilesystem.h"
#include "platform/iclock.h"
#include "platform/ihotkey.h"
#include "platform/itrayicon.h"
#include "core/inotestore.h"
#include "core/settings.h"

namespace stickynotes::app {
struct AppContext {
    std::unique_ptr<platform::IFileSystem> fs;
    std::unique_ptr<platform::IClock> clock;
    std::unique_ptr<platform::IHotkey> hotkey;
    std::unique_ptr<platform::ITrayIcon> tray;
    std::unique_ptr<core::INoteStore> notes;
    std::unique_ptr<core::Settings> settings;

    AppContext();
    ~AppContext();
    AppContext(AppContext&&) noexcept;
    AppContext& operator=(AppContext&&) noexcept;

    static AppContext production(const QString& dataDir);
};
}
