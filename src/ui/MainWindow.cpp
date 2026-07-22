#include "MainWindow.h"
#include "core/DatabaseManager.h"
#include "core/ClipboardMonitor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QApplication>
#include <QClipboard>
#include <QAction>
#include <QMessageBox>
#include <QGuiApplication>
#include <QScreen>
#include <QTimer>
#include <QStyle>
#include <QScrollBar>
#include <QShortcut>
#include <QItemSelectionModel>
#include <QPainter>
#include <QSettings>
#include <QFontDatabase>

static const char *BASE_STYLE = R"(
    QWidget { background-color: #F5F5F5; color: #333; font-family: 'Noto Sans CJK SC', 'Segoe UI', sans-serif; }
    QListView { background: white; border: none; border-radius: 6px; }
    QListView::item { border: none; padding: 0px; }
    QListView::item:selected { background: #E3F2FD; }
    QListView::item:hover:!selected { background: #F5F9FF; }
    QScrollBar:vertical {
        background: transparent; width: 6px; margin: 0;
    }
    QScrollBar::handle:vertical {
        background: #C0C0C0; border-radius: 3px; min-height: 30px;
    }
    QScrollBar::handle:vertical:hover { background: #A0A0A0; }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }
    QSplitter::handle { background: #E0E0E0; width: 1px; }
)";

MainWindow::MainWindow(DatabaseManager *db, ClipboardMonitor *monitor,
                       QWidget *parent)
    : QWidget(parent)
    , m_db(db)
    , m_monitor(monitor)
    , m_searchEdit(new QLineEdit(this))
    , m_listView(new QListView(this))
    , m_previewWidget(new ImagePreviewWidget(this))
    , m_infoLabel(new QLabel(this))
    , m_pinBtn(new QPushButton(this))
    , m_model(new HistoryListModel(this))
    , m_delegate(new HistoryListDelegate(this))
    , m_trayIcon(new QSystemTrayIcon(this))
    , m_trayMenu(new QMenu(this))
{
    QSettings s;
    m_autoHideAfterCopy = s.value("autoHideAfterCopy", true).toBool();

    setupUI();
    setupTray();
    setupConnections();
    loadHistory();

    auto *cleanupTimer = new QTimer(this);
    connect(cleanupTimer, &QTimer::timeout, this, [this]() {
        int removed = m_db->cleanupOldItems();
        if (removed > 0)
            loadHistory();
    });
    cleanupTimer->start(3600000);
}

void MainWindow::setupUI()
{
    setWindowTitle(QStringLiteral("\u526A\u5207\u677F\u5386\u53F2"));
    setMinimumSize(660, 540);
    resize(780, 640);
    setStyleSheet(BASE_STYLE);

    if (auto *screen = QGuiApplication::primaryScreen()) {
        auto geo = screen->availableGeometry();
        move(geo.center() - rect().center());
    }

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    auto *headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(4, 0, 4, 8);

    auto *titleLabel = new QLabel(QStringLiteral("\u526A\u5207\u677F\u5386\u53F2"), this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(15);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #222; background: transparent;");
    headerLayout->addWidget(titleLabel);

    headerLayout->addStretch();

    m_pinBtn->setText(QStringLiteral("\U0001F4CC"));
    m_pinBtn->setCheckable(true);
    m_pinBtn->setFixedSize(30, 30);
    m_pinBtn->setToolTip(QStringLiteral("\u7A97\u53E3\u7F6E\u9876"));
    m_pinBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; font-size: 18px; color: #999; }"
        "QPushButton:hover { color: #FFD700; }"
        "QPushButton:checked { color: #FFD700; }"
    );
    headerLayout->addWidget(m_pinBtn);

    mainLayout->addLayout(headerLayout);

    m_searchEdit->setPlaceholderText(QStringLiteral("\u641C\u7D22\u526A\u5207\u677F\u5386\u53F2..."));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedHeight(36);
    m_searchEdit->setStyleSheet(
        "QLineEdit { padding: 4px 12px 4px 34px; font-size: 13px; border: 1px solid #E0E0E0;"
        " border-radius: 18px; background: white; color: #333; }"
        "QLineEdit:focus { border-color: #4A90D9; background: white; }"
        "QLineEdit:hover { border-color: #BBB; }"
    );

    QPixmap searchPix(16, 16);
    searchPix.fill(Qt::transparent);
    {
        QPainter sp(&searchPix);
        sp.setRenderHint(QPainter::Antialiasing);
        sp.setPen(QPen(QColor("#999"), 2));
        sp.drawEllipse(2, 2, 9, 9);
        sp.setPen(QPen(QColor("#999"), 2.5));
        sp.drawLine(9, 9, 14, 14);
    }
    m_searchEdit->addAction(QIcon(searchPix), QLineEdit::LeadingPosition);
    mainLayout->addWidget(m_searchEdit);

    mainLayout->addSpacing(10);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);
    splitter->setChildrenCollapsible(false);

    auto *listContainer = new QWidget(this);
    auto *listLayout = new QVBoxLayout(listContainer);
    listLayout->setContentsMargins(0, 0, 0, 0);

    m_listView->setModel(m_model);
    m_listView->setItemDelegate(m_delegate);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setFrameShape(QFrame::NoFrame);
    m_listView->setUniformItemSizes(true);
    m_listView->setSpacing(1);
    m_listView->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical { background: transparent; width: 6px; margin: 0; }"
        "QScrollBar::handle:vertical { background: #C0C0C0; border-radius: 3px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: #A0A0A0; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
    );
    listLayout->addWidget(m_listView);
    splitter->addWidget(listContainer);

    auto *previewContainer = new QWidget(this);
    previewContainer->setStyleSheet("background: white; border-radius: 6px;");
    auto *previewLayout = new QVBoxLayout(previewContainer);
    previewLayout->setContentsMargins(8, 8, 8, 8);

    auto *previewTitle = new QLabel(QStringLiteral("\u9884\u89C8"), previewContainer);
    previewTitle->setStyleSheet("font-size: 11px; color: #999; background: transparent; padding: 2px;");
    previewLayout->addWidget(previewTitle);
    previewLayout->addWidget(m_previewWidget, 1);

    splitter->addWidget(previewContainer);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);

    mainLayout->addWidget(splitter, 1);

    mainLayout->addSpacing(6);

    auto *statusLayout = new QHBoxLayout;
    statusLayout->setContentsMargins(4, 0, 4, 0);
    m_infoLabel->setStyleSheet("color: #999; font-size: 11px; background: transparent;");
    statusLayout->addWidget(m_infoLabel);

    auto *hintLabel = new QLabel(QStringLiteral("Enter \u590D\u5236  |  Esc \u5173\u95ED  |  \u53F3\u952E\u66F4\u591A"), this);
    hintLabel->setStyleSheet("color: #BBB; font-size: 11px; background: transparent;");
    statusLayout->addStretch();
    statusLayout->addWidget(hintLabel);

    mainLayout->addLayout(statusLayout);

    setAttribute(Qt::WA_DeleteOnClose, false);
    setWindowFlags(Qt::WindowStaysOnTopHint);
}

void MainWindow::setupTray()
{
    m_trayAvailable = QSystemTrayIcon::isSystemTrayAvailable();
    if (!m_trayAvailable)
        return;

    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_FileDialogListView);
    m_trayIcon->setIcon(icon);
    m_trayIcon->setToolTip(QStringLiteral("\u526A\u5207\u677F\u5386\u53F2"));

    auto *showAction = m_trayMenu->addAction(QStringLiteral("\u663E\u793A"));
    connect(showAction, &QAction::triggered, this, &MainWindow::showWindow);

    auto *clearAction = m_trayMenu->addAction(QStringLiteral("\u6E05\u7A7A\u5386\u53F2"));
    connect(clearAction, &QAction::triggered, this, [this]() {
        auto ret = QMessageBox::question(
            this, QStringLiteral("\u786E\u8BA4"),
            QStringLiteral("\u786E\u5B9A\u6E05\u7A7A\u6240\u6709\u526A\u5207\u677F\u5386\u53F2\uFF1F"));
        if (ret == QMessageBox::Yes) {
            m_db->clearAll();
            m_model->clear();
            m_infoLabel->setText(QStringLiteral("0 \u6761\u8BB0\u5F55"));
        }
    });

    m_trayMenu->addSeparator();

    m_autoHideAction = m_trayMenu->addAction(QStringLiteral("\u590D\u5236\u540E\u81EA\u52A8\u5173\u95ED"));
    m_autoHideAction->setCheckable(true);
    m_autoHideAction->setChecked(m_autoHideAfterCopy);
    connect(m_autoHideAction, &QAction::toggled, this, [this](bool checked) {
        m_autoHideAfterCopy = checked;
        QSettings s;
        s.setValue("autoHideAfterCopy", checked);
    });

    m_trayMenu->addSeparator();

    auto *quitAction = m_trayMenu->addAction(QStringLiteral("\u9000\u51FA"));
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->show();

    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &MainWindow::onTrayActivated);
}

void MainWindow::setupConnections()
{
    m_searchEdit->installEventFilter(this);

    connect(m_listView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &current, const QModelIndex &) {
        onItemClicked(current);
    });

    connect(m_monitor, &ClipboardMonitor::newItem,
            this, &MainWindow::onNewClipboardItem);

    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &MainWindow::onSearchTextChanged);

    connect(m_listView, &QListView::clicked,
            this, &MainWindow::onItemClicked);
    connect(m_listView, &QListView::doubleClicked,
            this, &MainWindow::onItemDoubleClicked);
    connect(m_listView, &QListView::customContextMenuRequested,
            this, &MainWindow::onContextMenu);

    auto *focusSearch = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(focusSearch, &QShortcut::activated, this, [this]() {
        m_searchEdit->setFocus();
        m_searchEdit->selectAll();
    });

    auto *upArrow = new QShortcut(QKeySequence("Ctrl+P"), this);
    connect(upArrow, &QShortcut::activated, this, [this]() {
        int row = m_listView->currentIndex().row();
        if (row > 0) m_listView->setCurrentIndex(m_model->index(row - 1));
    });

    auto *downArrow = new QShortcut(QKeySequence("Ctrl+N"), this);
    connect(downArrow, &QShortcut::activated, this, [this]() {
        int row = m_listView->currentIndex().row();
        if (row < m_model->rowCount() - 1)
            m_listView->setCurrentIndex(m_model->index(row + 1));
    });

    m_pinBtn->hide();
}

void MainWindow::loadHistory()
{
    auto items = m_db->getAllItems();
    m_model->setItems(items);
    m_infoLabel->setText(QString("%1 \u6761\u8BB0\u5F55").arg(items.size()));
    if (!items.isEmpty())
        m_listView->setCurrentIndex(m_model->index(0));
}

void MainWindow::showWindow()
{
    show();
    raise();
    activateWindow();
    m_searchEdit->setFocus();
    m_searchEdit->selectAll();
}

void MainWindow::hideWindow()
{
    hide();
    m_searchEdit->clear();
    m_previewWidget->clear();
}

void MainWindow::onNewClipboardItem(const ClipboardItem &item)
{
    qint64 id = m_db->addItem(item);
    if (id > 0) {
        ClipboardItem saved = item;
        saved.id = id;
        m_model->prependItem(saved);
        m_infoLabel->setText(QString("%1 \u6761\u8BB0\u5F55").arg(m_model->rowCount()));
    }
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    m_previewWidget->clear();
    QVector<ClipboardItem> items;
    if (text.isEmpty())
        items = m_db->getAllItems();
    else
        items = m_db->searchItems(text);

    m_model->setItems(items);
    m_infoLabel->setText(QString("%1 \u6761\u8BB0\u5F55").arg(items.size()));
    if (!items.isEmpty()) {
        m_listView->setCurrentIndex(m_model->index(0));
        m_previewWidget->setText(items[0].contentText);
    }
}

void MainWindow::onItemClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    auto item = m_model->getItem(index.row());

    if (item.type == ClipboardItem::Image && !item.imageData.isEmpty())
        m_previewWidget->setImage(item.imageData);
    else
        m_previewWidget->setText(item.contentText);
}

void MainWindow::onItemDoubleClicked(const QModelIndex &index)
{
    if (index.isValid())
        copyCurrentItem();
}

void MainWindow::copyCurrentItem()
{
    auto idx = m_listView->currentIndex();
    if (!idx.isValid()) return;

    auto item = m_model->getItem(idx.row());
    m_monitor->copyToClipboard(item);

    if (m_autoHideAfterCopy)
        hideWindow();
}

void MainWindow::onContextMenu(const QPoint &pos)
{
    QModelIndex index = m_listView->indexAt(pos);
    if (!index.isValid()) return;

    auto item = m_model->getItem(index.row());

    QMenu menu(this);
    auto *copyAct = menu.addAction(QStringLiteral("\u590D\u5236"));
    auto *favAct = menu.addAction(
        item.isFavorite ? QStringLiteral("\u53D6\u6D88\u6536\u85CF")
                        : QStringLiteral("\u6536\u85CF"));
    menu.addSeparator();
    auto *delAct = menu.addAction(QStringLiteral("\u5220\u9664"));

    auto *selected = menu.exec(m_listView->viewport()->mapToGlobal(pos));

    if (selected == copyAct) {
        copyCurrentItem();
    } else if (selected == favAct) {
        m_db->toggleFavorite(item.id);
        item.isFavorite = !item.isFavorite;
        m_model->updateItem(item);
        loadHistory();
    } else if (selected == delAct) {
        m_db->removeItem(item.id);
        m_model->removeItem(item.id);
        m_infoLabel->setText(QString("%1 \u6761\u8BB0\u5F55").arg(m_model->rowCount()));
    }
}

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger ||
        reason == QSystemTrayIcon::DoubleClick) {
        if (isVisible())
            hideWindow();
        else
            showWindow();
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_searchEdit && event->type() == QEvent::KeyPress) {
        auto *key = static_cast<QKeyEvent *>(event);
        int row = m_listView->currentIndex().row();
        if (key->key() == Qt::Key_Down) {
            if (row < 0) row = 0;
            else if (row < m_model->rowCount() - 1) row++;
            m_listView->setCurrentIndex(m_model->index(row));
            return true;
        }
        if (key->key() == Qt::Key_Up) {
            if (row < 0) row = m_model->rowCount() - 1;
            else if (row > 0) row--;
            m_listView->setCurrentIndex(m_model->index(row));
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hideWindow();
    } else if (event->key() == Qt::Key_Return ||
               event->key() == Qt::Key_Enter) {
        copyCurrentItem();
    }
    QWidget::keyPressEvent(event);
}

void MainWindow::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    hideWindow();
    event->ignore();
}
