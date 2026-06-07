#include <QApplication>
#include <QStandardPaths>
#include "app/app_context.h"
#include "app/runtime.h"
#include <ElaApplication.h>
#ifdef Q_OS_WIN
#  include <windows.h>
#endif

int main(int argc, char* argv[]) {
    // 阻止任何运行时(包括 ElaWidgetTools)在 GUI 进程中意外 AttachConsole/AllocConsole
    // 而弹出的黑色 cmd 控制台窗口。
#ifdef Q_OS_WIN
    FreeConsole();
#endif
    QApplication app(argc, argv);
    eApp->init();
#ifdef Q_OS_WIN
    FreeConsole();  // 再次防御: 某些库在 init 时也会附加
#endif
    QCoreApplication::setOrganizationName("StickyNotes");
    QCoreApplication::setApplicationName("StickyNotes");
    auto dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto ctx = stickynotes::app::AppContext::production(dir);
    stickynotes::app::Runtime rt(ctx, app);
    rt.start();
#ifdef Q_OS_WIN
    FreeConsole();  // 第三次: Runtime 构造后, 任何 tray/hotkey init 残留
#endif
    return app.exec();
}