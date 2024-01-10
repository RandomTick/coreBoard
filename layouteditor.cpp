#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QMouseEvent>
#include "layouteditor.h"

QGraphicsItem *currentItem = nullptr;
QPointF offset;

LayoutEditor::LayoutEditor(QWidget *parent) : QGraphicsView(parent)
{    
    scene = new QGraphicsScene(this);

    this->setScene(scene);
    QRectF newRect = QRectF(QPointF(0, 0), QSizeF(this->viewport()->size()));
    scene->setSceneRect(newRect);
}

void LayoutEditor::mousePressEvent(QMouseEvent *event) {
    // Logic to add/select a shape on mouse press
    QGraphicsItem *item = scene->itemAt(mapToScene(event->pos()), QTransform());
    if (item) {
        currentItem = item;
        offset = mapToScene(event->pos()) - item->pos();
    }else{
        offset = QPointF();
    }
}

void LayoutEditor::mouseMoveEvent(QMouseEvent *event) {
    if (currentItem) {
        QPointF newPos = mapToScene(event->pos()) - offset;

        // Get the scene's boundaries
        qreal minX = scene->sceneRect().left();
        qreal minY = scene->sceneRect().top();
        qreal maxX = scene->sceneRect().right();
        qreal maxY = scene->sceneRect().bottom();

        // Get item's bounding rect
        QRectF rect = currentItem->boundingRect();

        // Adjust the position to keep the item within the boundaries
        qreal newX = qMax(minX, qMin(newPos.x(), maxX - rect.width()));
        qreal newY = qMax(minY, qMin(newPos.y(), maxY - rect.height()));
        currentItem->setPos(newX, newY);

    }
}

void LayoutEditor::mouseReleaseEvent(QMouseEvent *event) {
    currentItem = nullptr;
}

void LayoutEditor::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    if (scene) {
        QRectF newRect = QRectF(QPointF(0, 0), QSizeF(this->viewport()->size()));
        scene->setSceneRect(newRect);
    }
}

void LayoutEditor::addRectangle() {
    QGraphicsRectItem *rect = new QGraphicsRectItem(QRectF(0, 0, 100, 100));
    rect->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    scene->addItem(rect);
}
