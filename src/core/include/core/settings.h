#pragma once
#include <QString>
#include "platform/ifilesystem.h"

namespace stickynotes::core {
struct Settings {
    enum class Theme { Light, Dark, Auto };
    Theme theme = Theme::Auto;
    QString hotkey = "Ctrl+Alt+N";
    bool soundEnabled = true;
    QString dataDir;

    static Settings load(const QString& jsonPath, platform::IFileSystem& fs);
    bool save(platform::IFileSystem& fs) const;
};
}
