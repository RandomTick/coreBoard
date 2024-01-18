#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <iostream>

#include "windowskeylistener.h"
#include "KeyboardWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    QString locale = QLocale::system().name();
    translator.load(locale, ":/translations");
    a.installTranslator(&translator);



    MainWindow w;
    w.show();

    //select system language
    //TODO: make a menu and config to handle this/set it to something else
    //w.changeLanguage("de_DE");

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

