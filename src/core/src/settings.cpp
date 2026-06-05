#include "core/settings.h"
#include <QJsonDocument>
#include <QJsonObject>

namespace stickynotes::core {

Settings Settings::load(const QString& jsonPath, platform::IFileSystem& fs) {
    Settings s;
    auto bytes = fs.readAll(jsonPath);
    if (!bytes) return s;
    auto doc = QJsonDocument::fromJson(*bytes);
    if (!doc.isObject()) return s;
    auto o = doc.object();
    s.hotkey = o.value("hotkey").toString(s.hotkey);
    s.dataDir = o.value("dataDir").toString(s.dataDir);
    return s;
}

bool Settings::save(platform::IFileSystem& fs) const {
    QJsonObject o;
    o["hotkey"] = hotkey;
    o["dataDir"] = dataDir;
    return fs.writeAtomic(dataDir + "/settings.json",
                          QJsonDocument(o).toJson(QJsonDocument::Indented));
}
}
