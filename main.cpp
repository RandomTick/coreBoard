#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QLocale>

#include "windowskeylistener.h"
#include "KeyboardWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.changeLanguage(&a, QLocale::system().name());
    w.show();

    //select system language
    //TODO: make a menu and config to handle this/set it to something else
    //
    //Handling for Windows
    #ifdef Q_OS_WIN
    WindowsKeyListener keyListener;
    keyListener.startListening();

    // Connect signals to slots
    QObject::connect(&keyListener, &WindowsKeyListener::keyPressed,
                     w.keyboardWidget(), &KeyboardWidget::onKeyPressed);
    QObject::connect(&keyListener, &WindowsKeyListener::keyReleased,
                     w.keyboardWidget(), &KeyboardWidget::onKeyReleased);
    #endif

    return a.exec();
}

