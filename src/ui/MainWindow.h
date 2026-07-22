#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QListView>
#include <QLabel>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QMenu>
#include "ui/HistoryListModel.h"
#include "ui/HistoryListDelegate.h"
#include "ui/ImagePreviewWidget.h"
#include "core/ClipboardItem.h"

class DatabaseManager;
class ClipboardMonitor;

class MainWindow : public QWidget {
    Q_OBJECT
public:
    explicit MainWindow(DatabaseManager *db, ClipboardMonitor *monitor,
                        QWidget *parent = nullptr);

public slots:
    void showWindow();
    void hideWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onNewClipboardItem(const ClipboardItem &item);
    void onSearchTextChanged(const QString &text);
    void onItemClicked(const QModelIndex &index);
    void onItemDoubleClicked(const QModelIndex &index);
    void onContextMenu(const QPoint &pos);
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void copyCurrentItem();

private:
    void setupUI();
    void setupTray();
    void setupConnections();
    void loadHistory();

    DatabaseManager *m_db;
    ClipboardMonitor *m_monitor;

    QLineEdit *m_searchEdit;
    QListView *m_listView;
    ImagePreviewWidget *m_previewWidget;
    QLabel *m_infoLabel;
    QPushButton *m_pinBtn;

    HistoryListModel *m_model;
    HistoryListDelegate *m_delegate;

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QAction *m_autoHideAction = nullptr;
    bool m_trayAvailable = false;
    bool m_autoHideAfterCopy = true;
};
