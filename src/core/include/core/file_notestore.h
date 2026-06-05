#pragma once
#include "core/inotestore.h"
#include "platform/ifilesystem.h"
#include <QHash>
#include <QString>

namespace stickynotes::core {
class FileNoteStore final : public INoteStore {
public:
    FileNoteStore(QString dataDir, platform::IFileSystem& fs);
    QList<Note> all() const override;
    std::optional<Note> get(const QString& id) const override;
    Note create(const QString& categoryId) override;
    void upsert(const Note& n) override;
    bool remove(const QString& id) override;
    QList<Note> query(const QString& keyword, const QString& categoryId = {}) const override;
    QList<Category> categories() const override;
    Category createCategory(const QString& name, const QString& color = "#0078D4") override;
    void updateCategory(const Category& c) override;
    bool removeCategory(const QString& id) override;
    bool acquire(const QString& id) override;
    void release(const QString& id) override;
    bool isWritable(const QString& id) const override;
    void setNoteChangedCallback(NoteChangedCb cb) override;
    void setCategoryChangedCallback(CategoryChangedCb cb) override;
private:
    void ensureInbox();
    void loadIndex();
    QString notePath(const QString& id) const;
    void persistNote(const Note& n);
    void fireNoteChanged(const QString& id);

    QString dir_;
    platform::IFileSystem& fs_;
    QHash<QString, Note> notes_;
    QHash<QString, Category> categories_;
    QHash<QString, int> locks_;
    QHash<QString, Note> dirty_;
    NoteChangedCb noteCb_;
    CategoryChangedCb catCb_;
};
}
