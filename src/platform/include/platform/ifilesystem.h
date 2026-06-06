#pragma once
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <optional>

namespace stickynotes::platform {
class IFileSystem {
public:
    virtual ~IFileSystem() = default;
    virtual bool exists(const QString& path) const = 0;
    virtual std::optional<QByteArray> readAll(const QString& path) const = 0;
    virtual bool writeAtomic(const QString& path, const QByteArray& data) = 0;
    virtual QStringList list(const QString& dir, const QStringList& nameFilters) const = 0;
    virtual bool ensureDir(const QString& dir) = 0;
    virtual bool removeFile(const QString& path) = 0;
};
}
