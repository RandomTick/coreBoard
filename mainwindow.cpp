#include <QStackedWidget>
#include <QVBoxLayout>
#include <QTranslator>
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
    //m_keyboardWidget->loadLayout("C:/Users/SV5237/Documents/CoreBoard/keyboard_layout.json");
    m_keyboardWidget->loadLayout("C:/Users/SV5237/Documents/CoreBoard/nohBoard_example.json");
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

void MainWindow::resize(int width, int height){
    this->resize(width,height);
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

