#include "core/file_notestore.h"
#include "core/note.h"
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QDateTime>

namespace stickynotes::core {

FileNoteStore::FileNoteStore(QString dataDir, platform::IFileSystem& fs)
    : dir_(std::move(dataDir)), fs_(fs) {
    fs_.ensureDir(dir_);
    fs_.ensureDir(dir_ + "/notes");
    loadIndex();
}

QString FileNoteStore::notePath(const QString& id) const {
    return dir_ + "/notes/" + id + ".md";
}

void FileNoteStore::loadIndex() {
    auto files = fs_.list(dir_ + "/notes", {"*.md"});
    for (const auto& f : files) {
        auto bytes = fs_.readAll(dir_ + "/notes/" + f);
        if (!bytes) continue;
        QString text = QString::fromUtf8(*bytes);
        if (!text.startsWith("---")) continue;
        int end = text.indexOf("\n---", 3);
        if (end < 0) continue;
        QString fm = text.mid(3, end - 3).trimmed();
        QString body = text.mid(end + 4).trimmed();
        QJsonDocument doc = QJsonDocument::fromJson(fm.toUtf8());
        if (!doc.isObject()) continue;  // 损坏：忽略
        Note n = Note::fromJson(doc.object());
        n.content = body;
        notes_.insert(n.id, n);
    }
}

QList<Note> FileNoteStore::all() const {
    QList<Note> out;
    for (const auto& n : notes_)
        if (!n.deletedAt.isValid()) out.append(n);
    return out;
}
QList<Note> FileNoteStore::trash() const {
    QList<Note> out;
    for (const auto& n : notes_)
        if (n.deletedAt.isValid()) out.append(n);
    return out;
}
std::optional<Note> FileNoteStore::get(const QString& id) const {
    if (!notes_.contains(id)) return std::nullopt;
    return notes_[id];
}

Note FileNoteStore::create(const QString& categoryId) {
    Note n;
    n.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    n.categoryId = categoryId.isEmpty() ? "inbox" : categoryId;
    n.createdAt = n.updatedAt = QDateTime::currentDateTime();
    n.title = "Untitled";
    notes_.insert(n.id, n);
    persistNote(n);
    fireNoteChanged(n.id);
    return n;
}

void FileNoteStore::upsert(const Note& n) {
    Note m = n; m.updatedAt = QDateTime::currentDateTime();
    notes_[m.id] = m;
    persistNote(m);
    fireNoteChanged(m.id);
}

bool FileNoteStore::softDelete(const QString& id) {
    if (!notes_.contains(id)) return false;
    Note n = notes_[id];
    n.deletedAt = QDateTime::currentDateTime();
    notes_[id] = n;
    persistNote(n);
    fireNoteChanged(id);
    return true;
}

bool FileNoteStore::restore(const QString& id) {
    if (!notes_.contains(id)) return false;
    Note n = notes_[id];
    n.deletedAt = QDateTime();
    notes_[id] = n;
    persistNote(n);
    fireNoteChanged(id);
    return true;
}

bool FileNoteStore::permanentDelete(const QString& id) {
    if (!notes_.contains(id)) return false;
    notes_.remove(id);
    // 真正从磁盘删除：否则下次启动 loadIndex 会把 <id>.md 又加载回来
    fs_.removeFile(notePath(id));
    fireNoteChanged(id);
    return true;
}

void FileNoteStore::persistNote(const Note& n) {
    QJsonObject meta = n.toJson();
    QByteArray fm = QJsonDocument(meta).toJson(QJsonDocument::Compact);
    QByteArray body = n.content.toUtf8();
    QByteArray all = "---\n" + fm + "\n---\n" + body;
    fs_.writeAtomic(notePath(n.id), all);
}

void FileNoteStore::fireNoteChanged(const QString& id) {
    if (noteCb_) noteCb_(id);
}

QList<Note> FileNoteStore::query(const QString& keyword, const QString& categoryId) const {
    QList<Note> out;
    for (const auto& n : notes_) {
        // 与 all() 保持一致：默认排除已软删
        if (n.deletedAt.isValid()) continue;
        if (!categoryId.isEmpty() && n.categoryId != categoryId) continue;
        if (keyword.isEmpty() ||
            n.title.contains(keyword, Qt::CaseInsensitive) ||
            n.content.contains(keyword, Qt::CaseInsensitive)) {
            out.append(n);
        }
    }
    return out;
}

void FileNoteStore::setNoteChangedCallback(NoteChangedCb cb) { noteCb_ = std::move(cb); }
}
