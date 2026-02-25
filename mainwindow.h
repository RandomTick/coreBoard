#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QShowEvent>
#include "KeyboardWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class LayoutEditor;
class QStackedWidget;
class WindowsKeyListener;
#ifdef Q_OS_WIN
class WindowsMouseListener;
class GamepadListener;
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    KeyboardWidget* keyboardWidget() const;
    WindowsKeyListener* keyListener() const;
#ifdef Q_OS_WIN
    WindowsMouseListener* mouseListener() const;
    GamepadListener* gamepadListener() const;
#endif
    void resize(int width, int height);
    bool changeLanguage(const QApplication *a, const QString languageCode);
    Ui::MainWindow *ui;

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

signals:
    void languageChanged();

private slots:
    void onSwitchToView();
    void onAbout();
    void onColors();
    void onLabelDisplay();
    void onLanguage();
    void reloadVisualizationLayout(const QString &path);
    void ensureWindowFitsLayoutEditor();

private:
    void applyVisualizationColors();
    bool confirmLeaveEditor();

    KeyboardWidget *m_keyboardWidget = nullptr;
    LayoutEditor *m_layoutEditor = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;
    class LayoutSettings *m_layoutSettings = nullptr;
    WindowsKeyListener *m_keyListener = nullptr;
#ifdef Q_OS_WIN
    WindowsMouseListener *m_mouseListener = nullptr;
    GamepadListener *m_gamepadListener = nullptr;
#endif
};
#endif // MAINWINDOW_H
