#pragma once
#include <QObject>
#include "app/app_context.h"

class QApplication;

namespace stickynotes::app {
class MainWindow;

class Runtime : public QObject {
    Q_OBJECT
public:
    Runtime(AppContext& ctx, QApplication& app);
    void start();
    void requestQuit();
    MainWindow* mainWindow() const { return main_; }
private:
    void wireHotkey();
    void wireTray();
    
    AppContext& ctx_;
    QApplication& app_;
    MainWindow* main_ = nullptr;
};
}
