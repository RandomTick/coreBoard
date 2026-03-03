#include <QStackedWidget>
#include <QVBoxLayout>
#include <QTranslator>
#include <QMessageBox>
#include <QFileInfo>
#include <QTimer>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "layouteditor.h"
#include "layoutsettings.h"
#include "settingsdialog.h"
#include "labelsettingsdialog.h"
#include "languagesettingsdialog.h"
#include "versioninfo.h"
#include "welcomedialog.h"
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#ifdef Q_OS_WIN
#include "windowskeylistener.h"
#include "windowsmouselistener.h"
#include "gamepadlistener.h"
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_layoutSettings = new LayoutSettings;
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    m_keyboardWidget = new KeyboardWidget(this);
    m_layoutEditor = new LayoutEditor(this);
    m_layoutEditor->setLayoutSettings(m_layoutSettings);

    m_stackedWidget->addWidget(m_keyboardWidget);
    m_stackedWidget->addWidget(m_layoutEditor);

#ifdef Q_OS_WIN
    m_keyListener = new WindowsKeyListener(this);
    m_keyListener->startListening();
    connect(m_keyListener, &WindowsKeyListener::keyPressed, m_keyboardWidget, &KeyboardWidget::onKeyPressed);
    connect(m_keyListener, &WindowsKeyListener::keyReleased, m_keyboardWidget, &KeyboardWidget::onKeyReleased);
    connect(m_keyListener, &WindowsKeyListener::shiftStateChanged, m_keyboardWidget, &KeyboardWidget::setShiftState);
    connect(m_keyListener, &WindowsKeyListener::capsLockStateChanged, m_keyboardWidget, &KeyboardWidget::setCapsLockState);

    m_mouseListener = new WindowsMouseListener(this);
    m_mouseListener->startListening();
    connect(m_mouseListener, &WindowsMouseListener::keyPressed, m_keyboardWidget, &KeyboardWidget::onKeyPressed);
    connect(m_mouseListener, &WindowsMouseListener::keyReleased, m_keyboardWidget, &KeyboardWidget::onKeyReleased);

    m_gamepadListener = new GamepadListener(this);
    m_gamepadListener->startPolling();
    connect(m_gamepadListener, &GamepadListener::keyPressed, m_keyboardWidget, &KeyboardWidget::onKeyPressed);
    connect(m_gamepadListener, &GamepadListener::keyReleased, m_keyboardWidget, &KeyboardWidget::onKeyReleased);
    connect(m_gamepadListener, &GamepadListener::leftStickChanged, m_keyboardWidget, &KeyboardWidget::onLeftStickChanged);
    connect(m_gamepadListener, &GamepadListener::rightStickChanged, m_keyboardWidget, &KeyboardWidget::onRightStickChanged);
    connect(m_gamepadListener, &GamepadListener::triggersChanged, m_keyboardWidget, &KeyboardWidget::onTriggersChanged);
#endif

    QString lastPath = m_layoutSettings->lastLayoutPath();
    QString pathToLoad;
    if (!lastPath.isEmpty() && QFileInfo::exists(lastPath)) {
        pathToLoad = lastPath;
    } else {
        pathToLoad = QLatin1String(":/default.json");
    }
    int tab = m_layoutSettings->lastTabIndex();
    // Defer layout load to after window is fully constructed (avoids startup crash when loading from ctor)
    QTimer::singleShot(0, this, [this, pathToLoad, tab]() {
        m_keyboardWidget->loadLayout(pathToLoad);
        m_layoutEditor->loadLayout(pathToLoad);
        m_stackedWidget->setCurrentIndex(qBound(0, tab, 1));
    });
    QTimer::singleShot(100, this, &MainWindow::ensureWindowFitsLayoutEditor);

    connect(ui->actionView, &QAction::triggered, this, &MainWindow::onSwitchToView);
    connect(ui->actionEdit, &QAction::triggered, [this]() {
        m_stackedWidget->setCurrentIndex(1);
        m_layoutSettings->setLastTabIndex(1);
        m_layoutSettings->save();
    });
    connect(ui->actionColors, &QAction::triggered, this, &MainWindow::onColors);
    connect(ui->actionLabelDisplay, &QAction::triggered, this, &MainWindow::onLabelDisplay);
    connect(ui->actionLanguage, &QAction::triggered, this, &MainWindow::onLanguage);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
    connect(ui->actionGettingStarted, &QAction::triggered, this, &MainWindow::onGettingStarted);
    connect(ui->actionNohBoardLayouts, &QAction::triggered, this, &MainWindow::onNohBoardLayouts);
    connect(ui->actionCheckForUpdate, &QAction::triggered, this, &MainWindow::onCheckForUpdate);
    connect(ui->actionAutoUpdateCheck, &QAction::toggled, this, &MainWindow::onAutoUpdateCheckToggled);
    connect(ui->actionReportProblem, &QAction::triggered, this, &MainWindow::onReportProblem);
    ui->actionAutoUpdateCheck->setChecked(m_layoutSettings->autoUpdateCheck());
    applyVisualizationColors();
    connect(m_layoutEditor, &LayoutEditor::layoutLoaded, this, &MainWindow::reloadVisualizationLayout);
    connect(m_layoutEditor, &LayoutEditor::layoutLoaded, this, [this]() {
        QTimer::singleShot(100, this, &MainWindow::ensureWindowFitsLayoutEditor);
    });

    // Load saved language (English default if not set)
    QString langCode = m_layoutSettings->languageCode();
    if (!langCode.isEmpty() && langCode != QLatin1String("en_US"))
        changeLanguage(qApp, langCode);
}

void MainWindow::ensureWindowFitsLayoutEditor()
{
    QSize need = m_layoutEditor->minimumSize();
    QSize central = m_stackedWidget->size();
    if (need.width() <= 0 || need.height() <= 0)
        return;
    // Resize window so central widget is exactly the required size (allows shrink = "as small as possible")
    if (central.width() > 0 && central.height() > 0) {
        int newW = width() + (need.width() - central.width());
        int newH = height() + (need.height() - central.height());
        QMainWindow::resize(newW, newH);
    } else {
        // Not laid out yet; use estimated chrome (menu bar + frame)
        const int chromeW = 24;
        const int chromeH = 80;
        QMainWindow::resize(need.width() + chromeW, need.height() + chromeH);
    }
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, tr("About coreBoard"),
        tr("<h3>coreBoard</h3>"
           "<p>coreBoard is a keyboard, mouse, and controller visualization tool for streaming and video. "
           "It displays key presses and input on screen in real time and is designed to be fast and easy to capture with OBS or similar software.</p>"
           "<h4>Features</h4>"
           "<p><b>Layout editor (Edit Layout):</b> Add and edit shapes — rectangles, circles, polygons (star, diamond, hexagon, triangle), "
           "free-form paths, mouse speed indicator, and left/right joystick viewers. "
           "Per key shape: rebind (keyboard, mouse, or controller), rename, edit style, edit shape. "
           "Mouse/joystick elements: rename, edit style. "
           "Edit style: outline color and width, corner radius, font family/size/bold/italic, per-key colors. "
           "Open, new, save, recent files, undo/redo, copy/paste, move and resize.</p>"
           "<p><b>Visualization (View):</b> Real-time key, mouse, and controller display; global colors and label display.</p>"
           "<p><b>Settings:</b> Colors, label display (follow caps/shift, uppercase, lowercase), language.</p>"
           "<h4>Compatibility</h4>"
           "<p><b>NohBoard → coreBoard:</b> You can open NohBoard .json layouts in coreBoard (Edit Layout → Open Layout). "
           "Very custom NohBoard layouts might not be fully supported; please report issues (Help → Report a problem).</p>"
           "<p><b>coreBoard → NohBoard:</b> Layouts saved from coreBoard may not be fully functional in NohBoard, "
           "since coreBoard adds features such as joystick (angular) viewers and controller button bindings that NohBoard does not support.</p>"
           "<p>Built with C++ and Qt. All source code is publicly available under the GPL-3.0 license:<br>"
           "<a href=\"https://github.com/RandomTick/coreBoard\">https://github.com/RandomTick/coreBoard</a></p>"
           "<p>Version %1</p>").arg(QLatin1String(APP_VERSION)));
}

void MainWindow::onColors()
{
    SettingsDialog dlg(m_layoutSettings, this);
    if (dlg.exec() == QDialog::Accepted)
        applyVisualizationColors();
}

void MainWindow::onLabelDisplay()
{
    LabelSettingsDialog dlg(m_layoutSettings, this);
    if (dlg.exec() == QDialog::Accepted) {
        m_keyboardWidget->setLabelMode(m_layoutSettings->labelMode());
        m_keyboardWidget->update();
    }
}

void MainWindow::onLanguage()
{
    LanguageSettingsDialog dlg(m_layoutSettings, this);
    if (dlg.exec() == QDialog::Accepted) {
        QString code = m_layoutSettings->languageCode();
        if (code.isEmpty())
            code = QStringLiteral("en_US");
        changeLanguage(qApp, code);
    }
}

void MainWindow::onGettingStarted()
{
    WelcomeDialog dlg(m_layoutSettings, this);
    dlg.exec();
}

void MainWindow::onNohBoardLayouts()
{
    QMessageBox::information(this, tr("NohBoard layouts"),
        tr("To use a NohBoard layout: switch to <b>Edit Layout</b>, click <b>Open Layout</b>, then choose your .json file. No conversion needed.\n\n"
           "Very custom NohBoard layouts might not be fully supported; if something doesn't look right, report it (Help → Report a problem)."));
}

void MainWindow::onCheckForUpdate()
{
    runUpdateCheck(false);
}

void MainWindow::onAutoUpdateCheckToggled(bool checked)
{
    m_layoutSettings->setAutoUpdateCheck(checked);
    m_layoutSettings->save();
}

void MainWindow::runUpdateCheck(bool isAuto)
{
    if (m_updateCheckReply) {
        m_updateCheckReply->abort();
        m_updateCheckReply->deleteLater();
        m_updateCheckReply = nullptr;
    }
    m_updateCheckIsAuto = isAuto;
    if (!m_networkAccessManager)
        m_networkAccessManager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl(QStringLiteral("https://api.github.com/repos/RandomTick/coreBoard/releases/latest")));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("coreBoard"));
    m_updateCheckReply = m_networkAccessManager->get(request);
    connect(m_updateCheckReply, &QNetworkReply::finished, this, &MainWindow::onUpdateCheckFinished);
}

static QList<int> versionParts(const QString &s)
{
    QList<int> parts;
    for (const QString &seg : s.trimmed().split(QLatin1Char('.'))) {
        bool ok = false;
        int n = seg.toInt(&ok);
        parts.append(ok ? n : 0);
    }
    return parts.isEmpty() ? QList<int>() << 0 : parts;
}

// Returns true if a > b (e.g. 1.0.1 > 1.0.0)
static bool versionGreaterThan(const QString &a, const QString &b)
{
    const QList<int> va = versionParts(a);
    const QList<int> vb = versionParts(b);
    for (int i = 0; i < qMax(va.size(), vb.size()); ++i) {
        int na = i < va.size() ? va[i] : 0;
        int nb = i < vb.size() ? vb[i] : 0;
        if (na != nb)
            return na > nb;
    }
    return false;
}

void MainWindow::onUpdateCheckFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply != m_updateCheckReply)
        return;
    m_updateCheckReply->deleteLater();
    m_updateCheckReply = nullptr;

    const bool isAuto = m_updateCheckIsAuto;

    if (reply->error() != QNetworkReply::NoError) {
        if (!isAuto)
            QMessageBox::information(this, tr("Check for Update"),
                tr("Could not check for updates. Please check your internet connection or try again later."));
        return;
    }
    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (!doc.isObject() || parseError.error != QJsonParseError::NoError) {
        if (!isAuto)
            QMessageBox::information(this, tr("Check for Update"),
                tr("Could not read update information. Please try again later."));
        return;
    }
    QString tagName = doc.object().value(QLatin1String("tag_name")).toString();
    if (tagName.startsWith(QLatin1Char('v')))
        tagName = tagName.mid(1);
    QString htmlUrl = doc.object().value(QLatin1String("html_url")).toString();
    QString currentVersion = QLatin1String(APP_VERSION);
    if (versionGreaterThan(tagName, currentVersion)) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Check for Update"));
        msgBox.setText(tr("A new version is available: %1 (you have %2).").arg(tagName, currentVersion));
        msgBox.setInformativeText(tr("Open the release page in your browser to download it."));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.button(QMessageBox::Ok)->setText(tr("Open release page"));
        msgBox.button(QMessageBox::Cancel)->setText(tr("Later"));
        if (msgBox.exec() == QMessageBox::Ok && !htmlUrl.isEmpty())
            QDesktopServices::openUrl(QUrl(htmlUrl));
    } else {
        if (!isAuto)
            QMessageBox::information(this, tr("Check for Update"),
                tr("You're up to date. (Version %1)").arg(QLatin1String(APP_VERSION)));
    }
}

void MainWindow::onReportProblem()
{
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/RandomTick/coreBoard/issues")));
}

void MainWindow::applyVisualizationColors()
{
    m_keyboardWidget->setKeyColor(m_layoutSettings->keyColor());
    m_keyboardWidget->setHighlightColor(m_layoutSettings->highlightColor());
    m_keyboardWidget->setBackgroundColor(m_layoutSettings->backgroundColor());
    m_keyboardWidget->setTextColor(m_layoutSettings->textColor());
    m_keyboardWidget->setHighlightedTextColor(m_layoutSettings->highlightedTextColor());
    m_keyboardWidget->setLabelMode(m_layoutSettings->labelMode());
    // Full reload when we have a path so the visualization is redrawn from scratch (no stale scene iteration).
    m_keyboardWidget->reloadLayout();
}

void MainWindow::onSwitchToView()
{
    if (m_stackedWidget->currentIndex() != 1)
        m_stackedWidget->setCurrentIndex(0);
    else if (!m_layoutEditor->isDirty())
        m_stackedWidget->setCurrentIndex(0);
    else if (confirmLeaveEditor())
        m_stackedWidget->setCurrentIndex(0);
    if (m_stackedWidget->currentIndex() == 0) {
        m_layoutSettings->setLastTabIndex(0);
        m_layoutSettings->save();
        // Reload visualization after switch so it shows current layout and key codes
        QString path = m_layoutEditor->currentLayoutPath();
        QTimer::singleShot(0, this, [this, path]() {
            reloadVisualizationLayout(path);
        });
    }
}

void MainWindow::reloadVisualizationLayout(const QString &path)
{
    if (path.isEmpty())
        return;
    // Same path as "Open": reload visualization from file. Defer so after Save the file is closed.
    QString pathCopy = path;
    QTimer::singleShot(150, this, [this, pathCopy]() {
        m_keyboardWidget->loadLayout(pathCopy);
        m_keyboardWidget->update();
    });
}

bool MainWindow::confirmLeaveEditor()
{
    QMessageBox::StandardButton b = QMessageBox::question(this, tr("Unsaved changes"),
        tr("Save changes to the layout before switching?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if (b == QMessageBox::Cancel)
        return false;
    if (b == QMessageBox::Save) {
        m_layoutEditor->saveLayout();
        if (m_layoutEditor->isDirty())
            return false;
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_stackedWidget->currentIndex() == 1 && m_layoutEditor->isDirty()) {
        QMessageBox::StandardButton b = QMessageBox::question(this, tr("Unsaved changes"),
            tr("Save changes to the layout before closing?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if (b == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
        if (b == QMessageBox::Save) {
            m_layoutEditor->saveLayout();
            if (m_layoutEditor->isDirty()) {
                event->ignore();
                return;
            }
        }
    }
    m_layoutSettings->setLastTabIndex(m_stackedWidget->currentIndex());
    m_layoutSettings->save();
    event->accept();
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (!m_layoutSettings->welcomeDialogShown())
        QTimer::singleShot(200, this, [this]() { WelcomeDialog::showIfNeeded(m_layoutSettings, this); });
    if (m_layoutSettings->autoUpdateCheck() && !m_autoUpdateCheckDone) {
        m_autoUpdateCheckDone = true;
        QTimer::singleShot(500, this, [this]() { runUpdateCheck(true); });
    }
    if (m_stackedWidget->currentIndex() == 1)
        QTimer::singleShot(100, this, &MainWindow::ensureWindowFitsLayoutEditor);
}

MainWindow::~MainWindow()
{
    delete m_layoutSettings;
    m_layoutSettings = nullptr;
    delete ui;
}


KeyboardWidget* MainWindow::keyboardWidget() const {
    return m_keyboardWidget;
}

WindowsKeyListener* MainWindow::keyListener() const {
    return m_keyListener;
}

#ifdef Q_OS_WIN
WindowsMouseListener* MainWindow::mouseListener() const {
    return m_mouseListener;
}
GamepadListener* MainWindow::gamepadListener() const {
    return m_gamepadListener;
}
#endif

void MainWindow::resize(int width, int height){
    QMainWindow::resize(width, height);
}

bool MainWindow::changeLanguage(const QApplication *a, const QString languageCode) {
    // Static pointer to ensure the translator persists
    static QTranslator* translator = nullptr;


    // Delete the previous translator if it exists
    if (translator) {
        a->removeTranslator(translator);
        delete translator;
        translator = nullptr;
    }

    // Create a new translator
    translator = new QTranslator;


    QString fileName = QString(":/translations/CoreBoard_%1.qm").arg(languageCode);
    if (!translator->load(fileName)){
        delete translator;
        translator = nullptr;
        return false;
    }

    a->installTranslator(translator);


    ui->retranslateUi(this); //this tranlsates the ui file

    emit languageChanged();

    return true;
}
