#pragma once
#include "platform/ifilesystem.h"

namespace stickynotes::platform {
class FileSystem_Qt final : public IFileSystem {
public:
    bool exists(const QString& path) const override;
    std::optional<QByteArray> readAll(const QString& path) const override;
    bool writeAtomic(const QString& path, const QByteArray& data) override;
    QStringList list(const QString& dir, const QStringList& nameFilters) const override;
    bool ensureDir(const QString& dir) override;
    bool removeFile(const QString& path) override;
};
}
