#include <QApplication>
#include "core/DatabaseManager.h"
#include "core/ClipboardMonitor.h"
#include "ui/MainWindow.h"
#include "utils/HotkeyManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("ClipboardHistory");
    app.setOrganizationName("ClipboardTools");
    app.setQuitOnLastWindowClosed(false);

    DatabaseManager db;
    if (!db.initialize()) {
        qCritical("Failed to initialize database");
        return 1;
    }

    ClipboardMonitor monitor;
    monitor.start();

    MainWindow window(&db, &monitor);

    HotkeyManager hotkey(&window);
    hotkey.registerHotkey(QKeySequence("Ctrl+Alt+V"));
    QObject::connect(&hotkey, &HotkeyManager::activated,
                     &window, &MainWindow::showWindow);

    window.show();

    return app.exec();
}
