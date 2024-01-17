#include <QMouseEvent>
#include <QGraphicsItem>
#include "layouteditor.h"




LayoutEditor::LayoutEditor(QWidget *parent) : QWidget(parent)
{

    view = new LayoutEditorGraphicsView(this);
    scene = new QGraphicsScene(view);
    view->setSceneAndStore(scene);


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

    addButton = new QPushButton("Add Shape", this);
    addButton->setStyleSheet(buttonStyleSheet);

    // Connect button signals to slots
    connect(undoButton, &QPushButton::clicked, view, &LayoutEditorGraphicsView::undoLastAction);
    connect(redoButton, &QPushButton::clicked, view, &LayoutEditorGraphicsView::redoLastAction);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
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


void LayoutEditor::addRectangle() {
    QGraphicsRectItem *rect = new QGraphicsRectItem(QRectF(0, 0, 100, 100));
    scene->addItem(rect);
}
