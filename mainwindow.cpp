#include "mainwindow.h"
#include "./ui_mainwindow.h"

KeyboardWidget *m_keyboardWidget;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_keyboardWidget = new KeyboardWidget(this);

    m_keyboardWidget->loadLayout("C:/Users/SV5237/Documents/CoreBoard/keyboard_layout.json");
    m_keyboardWidget->show();

}

MainWindow::~MainWindow()
{
    delete ui;
}


KeyboardWidget* MainWindow::keyboardWidget() const {
    return m_keyboardWidget;
}
