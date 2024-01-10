#include <QMouseEvent>
#include <QGraphicsItem>
#include "layouteditor.h"



LayoutEditor::LayoutEditor(QWidget *parent) : QWidget(parent)
{
    view = new QGraphicsView(this);
    scene = new QGraphicsScene(view);
    view->setScene(scene);

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

    redoButton = new QPushButton("", this);
    redoButton->setIcon(QIcon(":/icons/redo.png"));
    redoButton->setIconSize(QSize(18,18));
    redoButton->setFixedSize(24,24);
    redoButton->setStyleSheet(buttonStyleSheet);

    // Connect button signals to slots
    // connect(undoButton, &QPushButton::clicked, this, &LayoutEditor::undoAction);
    // connect(redoButton, &QPushButton::clicked, this, &LayoutEditor::redoAction);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(undoButton);
    buttonLayout->addWidget(redoButton);
    buttonLayout->addStretch(1);


    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(buttonLayout);
    layout->addWidget(view);

    setLayout(layout);
}

void LayoutEditor::addRectangle() {
    QGraphicsRectItem *rect = new QGraphicsRectItem(QRectF(0, 0, 100, 100));
    rect->setFlags(QGraphicsItem::ItemIsMovable);
    scene->addItem(rect);
}
