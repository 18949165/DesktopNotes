#pragma once
#include "platform/ifilesystem.h"
#include <QHash>

namespace stickynotes::testing {
class FakeFileSystem : public stickynotes::platform::IFileSystem {
public:
    QHash<QString, QByteArray> data;
    bool failWrites = false;
    bool exists(const QString& p) const override { return data.contains(p); }
    std::optional<QByteArray> readAll(const QString& p) const override {
        if (!data.contains(p)) return std::nullopt;
        return data[p];
    }
    bool writeAtomic(const QString& p, const QByteArray& b) override {
        if (failWrites) return false;
        data[p] = b; return true;
    }
    QStringList list(const QString& dir, const QStringList& filters) const override {
        QStringList r;
        for (auto it = data.begin(); it != data.end(); ++it)
            if (it.key().startsWith(dir)) r << it.key().section('/', -1);
        return r;
    }
    bool ensureDir(const QString&) override { return true; }
    bool removeFile(const QString& p) override { return static_cast<int>(data.remove(p)) > 0; }
};
}
