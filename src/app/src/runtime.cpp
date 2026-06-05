#include "app/runtime.h"
#include "ui/mainwindow.h"
#include "ui/stickywindow.h"
#include "core/note.h"
#include <QApplication>
#include <QMenu>

namespace stickynotes::app {
Runtime::Runtime(AppContext& ctx, QApplication& a) 
    : QObject(nullptr), ctx_(ctx), app_(a) {}

void Runtime::start() {
    main_ = new ui::MainWindow(ctx_);
    main_->setAttribute(Qt::WA_DeleteOnClose, false);
    
    connect(&app_, &QApplication::aboutToQuit, this, [this]() {
        if (ctx_.settings) {
            ctx_.settings->save(*ctx_.fs);
        }
    });
    
    wireHotkey();
    wireTray();
    
    main_->show();
}

void Runtime::wireHotkey() {
    using platform::IHotkey;
    IHotkey::Spec s{ 1, Qt::ControlModifier | Qt::AltModifier, Qt::Key_N };
    if (!ctx_.hotkey->registerHotkey(s)) {
        // 真实提示留给 UI；此处仅日志
    }
    
    ctx_.hotkey->setTriggeredCallback([this](int) {
        if (!main_) return;
        auto n = ctx_.notes->create("inbox");
        auto* w = new ui::StickyWindow(ctx_, n);
        w->setAttribute(Qt::WA_DeleteOnClose, false);
        w->show();
    });
}

void Runtime::wireTray() {
    using platform::ITrayIcon;
    
    QList<ITrayIcon::MenuItem> items;
    
    items << ITrayIcon::MenuItem{"新建便签", [this]() {
        auto n = ctx_.notes->create("inbox");
        auto* w = new ui::StickyWindow(ctx_, n);
        w->setAttribute(Qt::WA_DeleteOnClose, false);
        w->show();
    }};
    
    items << ITrayIcon::MenuItem{"打开主窗口", [this]() {
        if (main_) {
            main_->show();
            main_->raise();
            main_->activateWindow();
        }
    }};
    
    items << ITrayIcon::MenuItem{"退出", [this]() {
        requestQuit();
    }};
    
    ctx_.tray->setMenu(items);
    ctx_.tray->show();
    
    ctx_.tray->setLeftClickCallback([this]() {
        if (!main_) return;
        if (main_->isVisible()) {
            main_->hide();
        } else {
            main_->showAndRaise();
        }
    });
}

void Runtime::requestQuit() {
    app_.quit();
}
}
