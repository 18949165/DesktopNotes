#pragma once
#include <QString>
#include "platform/ifilesystem.h"

namespace stickynotes::core {
struct Settings {
    QString hotkey = "Ctrl+Alt+N";
    QString dataDir;

    static Settings load(const QString& jsonPath, platform::IFileSystem& fs);
    bool save(platform::IFileSystem& fs) const;
};
}
