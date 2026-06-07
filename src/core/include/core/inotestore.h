#pragma once
#include <QList>
#include <functional>
#include <optional>
#include "core/note.h"

namespace stickynotes::core {
class INoteStore {
public:
    using NoteChangedCb = std::function<void(const QString&)>;

    virtual ~INoteStore() = default;
    virtual QList<Note> all() const = 0;          // 排除已软删
    virtual std::optional<Note> get(const QString& id) const = 0;  // 含已软删
    virtual QList<Note> trash() const = 0;         // 仅已软删
    virtual Note create(const QString& categoryId) = 0;
    virtual void upsert(const Note& n) = 0;
    // 软删：deletedAt = now，文件保留
    virtual bool softDelete(const QString& id) = 0;
    // 恢复：deletedAt 清空
    virtual bool restore(const QString& id) = 0;
    // 永久删除：真删文件 + 内存
    virtual bool permanentDelete(const QString& id) = 0;
    virtual QList<Note> query(const QString& keyword,
                              const QString& categoryId = {}) const = 0;
    // 回调注册（M1.3 教训：接口不含 Q_OBJECT，signals 用 std::function）
    virtual void setNoteChangedCallback(NoteChangedCb cb) = 0;
};
}
