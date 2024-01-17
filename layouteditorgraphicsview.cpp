#include <QMouseEvent>
#include <QGraphicsItem>
#include "layouteditorgraphicsview.h"
#include "layouteditor.h"

LayoutEditor *layoutEditor;

LayoutEditorGraphicsView::LayoutEditorGraphicsView(QWidget *parent) : QGraphicsView(parent) {
    layoutEditor = (LayoutEditor*)parent;

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
            startingPosition = item->pos();
        }else{
            offset = QPointF();
            startingPosition = QPointF();
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

        Actions move = Actions::Move;
        Action *action = new Action(move, currentItem, startingPosition, new QRectF());
        undoActions.push_back(action);
        redoActions.clear();

        action = nullptr;
        currentItem = nullptr;

        layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
    }
}


void LayoutEditorGraphicsView::undoLastAction(){
    //make sure there is a last action
    if (undoActions.empty())return;

    //retrieve action from the vector
    Action *lastAction = undoActions.back();
    undoActions.pop_back();

    doAction(lastAction);

    redoActions.push_back(lastAction);
    layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
}

void LayoutEditorGraphicsView::redoLastAction(){
    //make sure there is a redo action
    if (redoActions.empty())return;

    //retrieve action from the vector
    Action *lastAction = redoActions.back();
    redoActions.pop_back();

    doAction(lastAction);

    undoActions.push_back(lastAction);
    layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());

}



void LayoutEditorGraphicsView::doAction(Action *action){
    if (action->actionType == Move){
        //get current position
        QPointF currentPos = action->item->pos();
        //move item
        action->item->setPos(action->position);
        //update position in action
        action->position = currentPos;
    }
}


void LayoutEditorGraphicsView::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    if (scene) {        
        QRectF newRect = QRectF(QPointF(0, 0), QSizeF(this->viewport()->size()));
        scene->setSceneRect(newRect);
    }
}

