#include "platform/filesystem_qt.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

namespace stickynotes::platform {
bool FileSystem_Qt::exists(const QString& path) const {
    return QFileInfo::exists(path);
}

std::optional<QByteArray> FileSystem_Qt::readAll(const QString& path) const {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return std::nullopt;
    return f.readAll();
}

bool FileSystem_Qt::writeAtomic(const QString& path, const QByteArray& data) {
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return false;
    if (f.write(data) != data.size()) return false;
    return f.commit();
}

QStringList FileSystem_Qt::list(const QString& dir, const QStringList& nameFilters) const {
    QDir d(dir);
    return d.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);
}

bool FileSystem_Qt::ensureDir(const QString& dir) {
    return QDir().mkpath(dir);
}
}
