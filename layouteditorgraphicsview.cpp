#include <QMouseEvent>
#include <QGraphicsItem>
#include "layouteditorgraphicsview.h"

LayoutEditorGraphicsView::LayoutEditorGraphicsView(QWidget *parent) : QGraphicsView(parent) {
}


void LayoutEditorGraphicsView::setSceneAndStore(QGraphicsScene *externScene){
    scene = externScene;
    this->setScene(scene);
}

void LayoutEditorGraphicsView::mousePressEvent(QMouseEvent *event) {
    QGraphicsItem *item = scene->itemAt(mapToScene(event->pos()), QTransform());
    if (event->button() == Qt::RightButton) {
        //TODO: implement right click stuff, like changing bind/shape etc.
        qDebug("right test");
        if(item){
            //change item properties
        }else{
            //change background properties/general stuff
        }
    }else{        
        if (item) {
            currentItem = item;
            offset = mapToScene(event->pos()) - item->pos();
        }else{
            offset = QPointF();
        }
    }
}

void LayoutEditorGraphicsView::mouseMoveEvent(QMouseEvent *event) {
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

void LayoutEditorGraphicsView::mouseReleaseEvent(QMouseEvent *event) {
    if (currentItem) {
        QPointF newPos = mapToScene(event->pos()) - offset;

        // Get the scene's boundaries
        //qreal minX = scene->sceneRect().left();
        //qreal minY = scene->sceneRect().top();
        //qreal maxX = scene->sceneRect().right();
        //qreal maxY = scene->sceneRect().bottom();

        // Get item's bounding rect
        //QRectF rect = currentItem->boundingRect();

        // Adjust the position to keep the item within the boundaries
        //qreal newX = qMax(minX, qMin(newPos.x(), maxX - rect.width()));
        //qreal newY = qMax(minY, qMin(newPos.y(), maxY - rect.height()));

        Actions move = Actions::Move;
        Action *action = new Action(move, currentItem, offset, new QRectF());
        undoActions.push_back(action);

        action = nullptr;
        currentItem = nullptr;
    }


}

void LayoutEditorGraphicsView::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    if (scene) {        
        QRectF newRect = QRectF(QPointF(0, 0), QSizeF(this->viewport()->size()));
        scene->setSceneRect(newRect);
    }
}

