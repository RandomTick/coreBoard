#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>

#include "layouteditor.h"

LayoutEditor::LayoutEditor(QWidget *parent) : QWidget(parent)
{
    QGraphicsScene *scene = new QGraphicsScene(this);

    QGraphicsTextItem *textItem = new QGraphicsTextItem("Hello, World!");

    scene->addItem(textItem);

    // Create a QGraphicsView to visualize the scene
    QGraphicsView *view = new QGraphicsView(scene, this);

    // Set the QGraphicsView to fill the LayoutEditor widget
    view->setGeometry(this->rect()); // Adjust the geometry as necessary

}
