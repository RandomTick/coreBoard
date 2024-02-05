#include <QMouseEvent>
#include <QGraphicsItem>
#include "layouteditor.h"
#include "mainwindow.h"
#include <QJsonObject>
#include <QLabel>
#include <QFileDialog>

LayoutEditor::LayoutEditor(QWidget *parent) : QWidget(parent)
{

    view = new LayoutEditorGraphicsView(this);
    scene = new QGraphicsScene(view);
    view->setSceneAndStore(scene);
    view->setMouseTracking(true);

    //connect translators
    MainWindow* myParentMainWindow = qobject_cast<MainWindow*>(this->parentWidget());
    connect(myParentMainWindow, &MainWindow::languageChanged, this, &LayoutEditor::updateLanguage);



    // Initialize the scene's size
    QRectF newRect = QRectF(QPointF(0, 0), QSizeF(view->viewport()->size()));
    scene->setSceneRect(newRect);

    // Initialize buttons
    QString buttonStyleSheet = "QPushButton {"
                               "background-color: transparent;"
                               "}"
                               "QPushButton:hover {"
                               "background-color: rgba(150, 51, 150, 0.4);"
                               "}";

    openButton = new QPushButton(tr("Open Layout"), this);
    openButton->setStyleSheet(buttonStyleSheet);

    undoButton = new QPushButton("", this);
    undoButton->setIcon(QIcon(":/icons/undo.png"));
    undoButton->setIconSize(QSize(18,18));
    undoButton->setFixedSize(24,24);
    undoButton->setStyleSheet(buttonStyleSheet);
    undoButton->setEnabled(false);

    redoButton = new QPushButton("", this);
    redoButton->setIcon(QIcon(":/icons/redo.png"));
    redoButton->setIconSize(QSize(18,18));
    redoButton->setFixedSize(24,24);
    redoButton->setStyleSheet(buttonStyleSheet);
    redoButton->setEnabled(false);

    addButton = new QPushButton(tr("Add Shape"), this);
    addButton->setStyleSheet(buttonStyleSheet);

    // Connect button signals to slots
    connect(openButton, &QPushButton::clicked, this, &LayoutEditor::loadLayoutButton);
    connect(undoButton, &QPushButton::clicked, view, &LayoutEditorGraphicsView::undoLastAction);
    connect(redoButton, &QPushButton::clicked, view, &LayoutEditorGraphicsView::redoLastAction);


    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(openButton);
    buttonLayout->addWidget(undoButton);
    buttonLayout->addWidget(redoButton);
    buttonLayout->addWidget(addButton);
    buttonLayout->addStretch(1);


    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(buttonLayout);
    layout->addWidget(view);

    setLayout(layout);
}

void LayoutEditor::updateButtons(bool undoCommandsExist, bool redoCommandsExist){
    undoButton->setEnabled(undoCommandsExist);
    redoButton->setEnabled(redoCommandsExist);
}



void LayoutEditor::loadLayoutButton(){
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select Layout File to Edit"), "", tr("Layout Files (*.json);;All Files (*)"));

    // Check if a file was selected (filePath is not empty)
    if (!filePath.isEmpty()) {
        // Call loadLayout with the selected file path
        loadLayout(filePath);
    }
}


void LayoutEditor::loadLayout(const QString &fileName){

}


void LayoutEditor::addRectangle() {
    QGraphicsRectItem *rect = new QGraphicsRectItem(QRectF(0, 0, 100, 100));
    //scene->addItem(rect);

    //ResizableRectItem *rect = new ResizableRectItem(QRectF(0, 0, 100, 100));
    scene->addItem(rect);
    rect->setPos(50,50);
}

void LayoutEditor::updateLanguage() {
    // Update the text of your widgets here
    addButton->setText(tr("Add Shape"));
    openButton->setText(tr("Open Layout"));
    // ...
}
