#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QCoreApplication>
#include "app/reminder_service.h"
#include "core/inotestore.h"
#include "platform/fake_clock.h"
#include "../platform/mocks.h"

namespace stickynotes {
class FakeNoteStore : public core::INoteStore {
public:
    QList<core::Note> notes;
    NoteChangedCb noteChangedCb;
    CategoryChangedCb categoryChangedCb;

    bool acquire(const QString&) override { return true; }
    void release(const QString&) override {}
    bool isWritable(const QString&) const override { return true; }
    void setNoteChangedCallback(NoteChangedCb cb) override { noteChangedCb = cb; }
    void setCategoryChangedCallback(CategoryChangedCb) override {}

    QList<core::Note> all() const override { return notes; }
    std::optional<core::Note> get(const QString& id) const override {
        for (auto& n : notes) if (n.id == id) return n;
        return std::nullopt;
    }
    core::Note create(const QString&) override { return {}; }
    void upsert(const core::Note& n) override {
        bool found = false;
        for (auto& x : notes) {
            if (x.id == n.id) {
                x = n;
                found = true;
                break;
            }
        }
        if (!found) notes.append(n);
        if (noteChangedCb) noteChangedCb(n.id);
    }
    bool remove(const QString&) override { return true; }
    QList<core::Note> query(const QString&, const QString&) const override { return notes; }
    QList<core::Category> categories() const override { return {}; }
    core::Category createCategory(const QString&, const QString&) override { return {}; }
    void updateCategory(const core::Category&) override {}
    bool removeCategory(const QString&) override { return true; }
};

TEST(ReminderService, FiresAndClears) {
    int argc = 0;
    QCoreApplication app(argc, nullptr);
    FakeNoteStore store;
    platform::FakeClock clock(QDateTime::fromString("2026-06-05T10:00:00", Qt::ISODate));
    platform::MockNotifier notifier;

    core::Note n;
    n.id = "1";
    n.title = "test reminder";
    n.remindAt = QDateTime::fromString("2026-06-05T09:00:00", Qt::ISODate);
    store.upsert(n);

    EXPECT_CALL(notifier, show(testing::_)).Times(1);
    app::ReminderService svc(store, clock, notifier, 1000);
    svc.start();
    QCoreApplication::processEvents();

    auto updatedNote = store.get("1");
    ASSERT_TRUE(updatedNote.has_value());
    EXPECT_FALSE(updatedNote->remindAt.isValid());
}

TEST(ReminderService, DoesNotDoubleFire) {
    int argc = 0;
    QCoreApplication app(argc, nullptr);
    FakeNoteStore store;
    platform::FakeClock clock(QDateTime::fromString("2026-06-05T10:00:00", Qt::ISODate));
    platform::MockNotifier notifier;

    core::Note n;
    n.id = "2";
    n.remindAt = QDateTime::fromString("2026-06-05T09:00:00", Qt::ISODate);
    store.upsert(n);

    EXPECT_CALL(notifier, show(testing::_)).Times(1);
    app::ReminderService svc(store, clock, notifier, 1000);
    svc.start();
    QCoreApplication::processEvents();
}
}
