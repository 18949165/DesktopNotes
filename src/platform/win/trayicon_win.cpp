#include "platform/win/trayicon_win.h"
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QIcon>

namespace stickynotes::platform::win {
struct TrayIcon_Win::Impl {
    QSystemTrayIcon* tray = nullptr;
    QMenu* menu = nullptr;
};

TrayIcon_Win::TrayIcon_Win() : d_(new Impl) {
    d_->tray = new QSystemTrayIcon();
    d_->menu = new QMenu();
    d_->tray->setContextMenu(d_->menu);
    QIcon icon;
    icon.addFile(":/icons/note.ico");
    if (icon.isNull()) {
        QPixmap pix(32, 32);
        pix.fill(Qt::yellow);
        icon = QIcon(pix);
    }
    d_->tray->setIcon(icon);
    
    QObject::connect(d_->tray, &QSystemTrayIcon::activated, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger && cb_) {
            cb_();
        }
    });
}

TrayIcon_Win::~TrayIcon_Win() {
    delete d_->tray;
    delete d_->menu;
}

void TrayIcon_Win::show() {
    d_->tray->show();
}

void TrayIcon_Win::hide() {
    d_->tray->hide();
}

void TrayIcon_Win::setMenu(const QList<MenuItem>& items) {
    d_->menu->clear();
    for (const auto& item : items) {
        auto* action = d_->menu->addAction(item.text);
        QObject::connect(action, &QAction::triggered, [cb = item.onClick]() {
            if (cb) cb();
        });
    }
}

void TrayIcon_Win::setLeftClickCallback(LeftClickCb cb) {
    cb_ = std::move(cb);
}
}
