#include "core/file_notestore.h"
#include "core/note.h"
#include "core/category.h"
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QDateTime>

namespace stickynotes::core {

FileNoteStore::FileNoteStore(QString dataDir, platform::IFileSystem& fs)
    : dir_(std::move(dataDir)), fs_(fs) {
    fs_.ensureDir(dir_);
    fs_.ensureDir(dir_ + "/notes");
    ensureInbox();
    loadIndex();
}

void FileNoteStore::ensureInbox() {
    if (!categories_.contains("inbox")) {
        Category c; c.id = "inbox"; c.name = "Inbox"; c.color = "#0078D4";
        categories_.insert(c.id, c);
    }
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

QList<Note> FileNoteStore::all() const { return notes_.values(); }
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

bool FileNoteStore::remove(const QString& id) {
    if (!notes_.contains(id)) return false;
    notes_.remove(id);
    locks_.remove(id);
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
        if (!categoryId.isEmpty() && n.categoryId != categoryId) continue;
        if (keyword.isEmpty() ||
            n.title.contains(keyword, Qt::CaseInsensitive) ||
            n.content.contains(keyword, Qt::CaseInsensitive)) {
            out.append(n);
        }
    }
    return out;
}

QList<Category> FileNoteStore::categories() const { return categories_.values(); }

Category FileNoteStore::createCategory(const QString& name, const QString& color) {
    Category c;
    c.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    c.name = name; c.color = color;
    categories_.insert(c.id, c);
    if (catCb_) catCb_(c.id);
    return c;
}

void FileNoteStore::updateCategory(const Category& c) {
    categories_[c.id] = c;
    if (catCb_) catCb_(c.id);
}

bool FileNoteStore::removeCategory(const QString& id) {
    if (id == "inbox") return false;  // inbox 不可删
    if (!categories_.contains(id)) return false;
    categories_.remove(id);
    if (catCb_) catCb_(id);
    return true;
}

void FileNoteStore::setNoteChangedCallback(NoteChangedCb cb) { noteCb_ = std::move(cb); }
void FileNoteStore::setCategoryChangedCallback(CategoryChangedCb cb) { catCb_ = std::move(cb); }

// 视图写锁
bool FileNoteStore::acquire(const QString& id) {
    int cur = locks_.value(id, 0);
    if (cur == 0) { locks_[id] = 1; return true; }
    locks_[id] = cur + 1;
    return false;
}

void FileNoteStore::release(const QString& id) {
    int cur = locks_.value(id, 0);
    if (cur <= 1) {
        locks_.remove(id);
        // 引用计数降为 0：把暂存的修改落盘
        if (dirty_.contains(id)) {
            const Note& n = dirty_[id];
            persistNote(n);
            dirty_.remove(id);
        }
    } else {
        locks_[id] = cur - 1;
    }
}

bool FileNoteStore::isWritable(const QString& id) const {
    return locks_.value(id, 0) == 0;
}
}
