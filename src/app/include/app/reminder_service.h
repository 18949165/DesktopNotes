#pragma once
#include <QObject>
#include <QSet>
#include <QTimer>
#include "core/inotestore.h"
#include "platform/iclock.h"
#include "platform/inotifier.h"

namespace stickynotes::app {
class ReminderService : public QObject {
    Q_OBJECT
public:
    ReminderService(core::INoteStore& store, platform::IClock& clock,
                    platform::INotifier& notifier, int intervalMs = 30000,
                    QObject* parent = nullptr);
    void start();
    void stop();
private slots:
    void onTick();
    void onNoteChanged(QString id);
private:
    void fire(core::INoteStore& s, const core::Note& n);
    core::INoteStore& store_;
    platform::IClock& clock_;
    platform::INotifier& notifier_;
    QTimer timer_;
    QSet<QString> fired_;
};
}
