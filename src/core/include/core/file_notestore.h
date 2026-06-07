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
    QList<Note> trash() const override;
    Note create(const QString& categoryId) override;
    void upsert(const Note& n) override;
    bool softDelete(const QString& id) override;
    bool restore(const QString& id) override;
    bool permanentDelete(const QString& id) override;
    QList<Note> query(const QString& keyword, const QString& categoryId = {}) const override;
    void setNoteChangedCallback(NoteChangedCb cb) override;
private:
    void loadIndex();
    QString notePath(const QString& id) const;
    void persistNote(const Note& n);
    void fireNoteChanged(const QString& id);

    QString dir_;
    platform::IFileSystem& fs_;
    QHash<QString, Note> notes_;
    NoteChangedCb noteCb_;
};
}
