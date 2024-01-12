#include <QStackedWidget>
#include <QVBoxLayout>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "layouteditor.h"

KeyboardWidget *m_keyboardWidget;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QStackedWidget *stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    m_keyboardWidget = new KeyboardWidget(this);
    m_keyboardWidget->loadLayout("C:/Users/SV5237/Documents/CoreBoard/keyboard_layout.json");

    LayoutEditor *m_layoutEditor = new LayoutEditor(this);


    stackedWidget->addWidget(m_keyboardWidget);
    stackedWidget->addWidget(m_layoutEditor);

    stackedWidget->setCurrentIndex(1);

    connect(ui->actionView, &QAction::triggered, [stackedWidget](){
        stackedWidget->setCurrentIndex(0);
    });
    connect(ui->actionEdit, &QAction::triggered, [stackedWidget](){
        stackedWidget->setCurrentIndex(1);
    });


    m_layoutEditor->addRectangle();
}

MainWindow::~MainWindow()
{
    delete ui;
}


KeyboardWidget* MainWindow::keyboardWidget() const {
    return m_keyboardWidget;
}
