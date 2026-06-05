#include <QApplication>
#include <QStandardPaths>
#include "app/app_context.h"
#include "app/app_entry.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("StickyNotes");
    QCoreApplication::setApplicationName("StickyNotes");
    auto dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto ctx = stickynotes::app::AppContext::production(dir);
    stickynotes::app::AppEntry entry(ctx, app);
    entry.run();
    return app.exec();
}

namespace stickynotes {
int runHeadless(QCoreApplication&) { return 0; }
}
