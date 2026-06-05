#include "app/reminder_service.h"
#include "core/note.h"

namespace stickynotes::app {
ReminderService::ReminderService(core::INoteStore& s, platform::IClock& c,
                                 platform::INotifier& n, int ms, QObject* p)
    : QObject(p), store_(s), clock_(c), notifier_(n) {
    timer_.setInterval(ms);
    connect(&timer_, &QTimer::timeout, this, &ReminderService::onTick);
    store_.setNoteChangedCallback([this](const QString& id) { onNoteChanged(id); });
}
void ReminderService::start() { timer_.start(); onTick(); }
void ReminderService::stop()  { timer_.stop(); }

void ReminderService::onTick() {
    const auto now = clock_.now();
    for (const auto& n : store_.all()) {
        if (!n.remindAt.isValid() || fired_.contains(n.id)) continue;
        if (n.remindAt <= now) fire(store_, n);
    }
}
void ReminderService::onNoteChanged(QString id) {
    auto n = store_.get(id);
    if (n && (!n->remindAt.isValid() || n->remindAt > clock_.now()))
        fired_.remove(id);
}
void ReminderService::fire(core::INoteStore& s, const core::Note& n) {
    platform::INotifier::Notification notif;
    notif.title = "Reminder";
    notif.body = n.title;
    notif.id = n.id;
    notifier_.show(notif);
    fired_.insert(n.id);
    core::Note m = n; m.remindAt = {};
    s.upsert(m);
}
}
