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
private:
    void wireHotkey();
    void wireTray();
    
    AppContext& ctx_;
    QApplication& app_;
    stickynotes::ui::MainWindow* main_ = nullptr;
};
}
