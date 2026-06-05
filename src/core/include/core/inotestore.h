#pragma once
#include <QObject>
#include <QList>
#include <optional>
#include "core/note.h"
#include "core/category.h"

namespace stickynotes::core {
class INoteStore : public QObject {
    Q_OBJECT
public:
    virtual ~INoteStore() override = default;
    virtual QList<Note> all() const = 0;
    virtual std::optional<Note> get(const QString& id) const = 0;
    virtual Note create(const QString& categoryId) = 0;
    virtual void upsert(const Note& n) = 0;
    virtual bool remove(const QString& id) = 0;
    virtual QList<Note> query(const QString& keyword,
                              const QString& categoryId = {}) const = 0;
    virtual QList<Category> categories() const = 0;
    virtual Category createCategory(const QString& name,
                                   const QString& color = "#0078D4") = 0;
    virtual void updateCategory(const Category& c) = 0;
    virtual bool removeCategory(const QString& id) = 0;
    // 视图写权限：详见 spec §3.4 同步模型
    virtual bool acquire(const QString& id) = 0;
    virtual void release(const QString& id) = 0;
    virtual bool isWritable(const QString& id) const = 0;
signals:
    void noteChanged(QString id);
    void categoryChanged(QString id);
};
}
