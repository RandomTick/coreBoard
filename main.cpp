#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include "globalkeylistener.h"
#include "KeyboardWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "CoreBoard_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.show();

    //Handling for Windows
    #ifdef Q_OS_WIN
    GlobalKeyListener keyListener;
    keyListener.startListening();

    // Connect signals to slots
    QObject::connect(&keyListener, &GlobalKeyListener::keyPressed,
                     w.keyboardWidget(), &KeyboardWidget::onKeyPressed);
    QObject::connect(&keyListener, &GlobalKeyListener::keyReleased,
                     w.keyboardWidget(), &KeyboardWidget::onKeyReleased);
    #endif

    return a.exec();
}
