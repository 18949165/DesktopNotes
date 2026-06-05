#include <QCoreApplication>
#include <QtGlobal>
#include "app/app_entry.h"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("StickyNotes");
    QCoreApplication::setApplicationName("StickyNotes");
    return stickynotes::runHeadless(app);
}

namespace stickynotes {
int runHeadless(QCoreApplication&) { return 0; }
}
