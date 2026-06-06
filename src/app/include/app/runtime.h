#pragma once
#include <QObject>
#include "app/app_context.h"

class QApplication;

namespace stickynotes::ui {
class MainWindow;
}

namespace stickynotes::app {

class Runtime : public QObject {
    Q_OBJECT
public:
    Runtime(AppContext& ctx, QApplication& app);
    void start();
    void requestQuit();
    stickynotes::ui::MainWindow* mainWindow() const { return main_; }
    // 重新注册新建便签快捷键：先 unregister 旧的（若 id=1），再按新字符串注册
    void reregisterHotkey(const QString& hotkeyStr);
private:
    void wireHotkey();
    void wireTray();
    
    AppContext& ctx_;
    QApplication& app_;
    stickynotes::ui::MainWindow* main_ = nullptr;
};
}
