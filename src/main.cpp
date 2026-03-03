#include "mainwindow.h"

#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/installer/coreBoard.ico"));

    MainWindow w;
    w.show();

    //select system language
    //TODO: make a menu and config to handle this/set it to something else
    //w.changeLanguage(&a, QLocale::system().name());

    return a.exec();
}

