#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "KeyboardWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    KeyboardWidget* keyboardWidget() const;
    void resize(int width, int height);
    bool changeLanguage(const QApplication *a, const QString languageCode);
    Ui::MainWindow *ui;
signals:
    void languageChanged();
};
#endif // MAINWINDOW_H
