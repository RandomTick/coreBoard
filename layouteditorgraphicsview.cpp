#include <QMouseEvent>
#include <QGraphicsItem>
#include "layouteditorgraphicsview.h"
#include "layouteditor.h"
#include <iostream>

LayoutEditor *layoutEditor;

LayoutEditorGraphicsView::LayoutEditorGraphicsView(QWidget *parent) : QGraphicsView(parent) {
    layoutEditor = (LayoutEditor*)parent;
    currentItem = nullptr;
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
            startingBounds = item->boundingRect();

        }else{
            offset = QPointF();
            startingPosition = QPointF();
        }
    }
}

void LayoutEditorGraphicsView::mouseMoveEvent(QMouseEvent *event) {
    if (currentItem) {

        int edgeOrCorner = isOnEdgeOrCorner(currentItem, event->pos());
        QRectF currentBounds = currentItem->boundingRect();

        if (edgeOrCorner != 0) {
            // Handle resizing for each shape differently
            QPointF newPos = mapToScene(event->pos());
            qreal xOffset = startingPosition.x() - newPos.x();
            qreal yOffset = startingPosition.y() - newPos.y();
            qreal newWidth = 0;
            qreal newHeight = 0;


            QGraphicsRectItem *rect = dynamic_cast<QGraphicsRectItem*>(currentItem);
            if (rect){ //is rectangle
            switch (edgeOrCorner) {
            case 1: // Top-Left Corner
                // Assuming xOffset and yOffset are the changes in width and height
                newWidth = startingBounds.width() + xOffset;
                newHeight = startingBounds.height() + yOffset;

                if (newWidth < 20.0 || newHeight < 20.0){
                    currentItem = nullptr;
                }

                // Enforce minimum width and height of 20
                newWidth = qMax(newWidth, 20.0);
                newHeight = qMax(newHeight, 20.0);



                // Update the rectangle's position and size
                rect->setPos(newPos);
                rect->setRect(0, 0, newWidth, newHeight);

                break;
            case 2: // Bottom-Left Corner
                newWidth = startingBounds.width() + xOffset;
                newHeight = startingBounds.height() + yOffset - currentBounds.height();

                newWidth = qMax(newWidth, 20.0);
                newHeight = qMax(newHeight, 20.0);

                rect->setPos(newPos.x() + xOffset + offset.x(), newPos.y() + offset.y()); // Update X position
                rect->setRect(0, 0, newWidth, newHeight);
                break;

            case 3: // Top-Right Corner
                newWidth = startingBounds.width() + xOffset;
                newHeight = startingBounds.height() + yOffset; // Height adjusts oppositely

                newWidth = qMax(newWidth, 20.0);
                newHeight = qMax(newHeight, 20.0);

                rect->setPos(startingBounds.topLeft().x() + offset.x(), startingBounds.topLeft().y() + yOffset); // Update Y position
                rect->setRect(0, 0, newWidth, newHeight);
                break;


            case 4: // Bottom-Right Corner
                newWidth = startingBounds.width() + xOffset;
                newHeight = startingBounds.height() + yOffset;

                newWidth = qMax(newWidth, 20.0);
                newHeight = qMax(newHeight, 20.0);

                // No position update needed
                rect->setRect(0, 0, newWidth, newHeight);
                break;

            case 5: // Left Edge
            case 6: // Right Edge
            case 7: // Top Edge
            case 8: // Bottom Edge
                break;
            }
            }

        }else{

            QPointF newPos = mapToScene(event->pos()) - offset;

            // Get the scene's boundaries
            qreal minX = scene->sceneRect().left();
            qreal minY = scene->sceneRect().top();
            qreal maxX = scene->sceneRect().right();
            qreal maxY = scene->sceneRect().bottom();


            // Adjust the position to keep the item within the boundaries
            qreal newX = qMax(minX, qMin(newPos.x(), maxX - startingBounds.width()));
            qreal newY = qMax(minY, qMin(newPos.y(), maxY - startingBounds.height()));
            currentItem->setPos(newX, newY);

            }
        }else{
        //implement hover event for resize
        QGraphicsItem *item = scene->itemAt(mapToScene(event->pos()), QTransform());
        int testEdgeOrCorner = isOnEdgeOrCorner(item, event->pos());



        switch (testEdgeOrCorner) {
        case 1:
        case 4:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case 2:
        case 3:
            setCursor(Qt::SizeBDiagCursor);
            break;
        case 5:
        case 6:
            setCursor(Qt::SizeHorCursor);
            break;
        case 7:
        case 8:
            setCursor(Qt::SizeVerCursor);
            break;
        default:
            setCursor(Qt::ArrowCursor); // Default cursor for no match
        }


    }
}


void LayoutEditorGraphicsView::mouseReleaseEvent(QMouseEvent *event) {
    if (currentItem) {

        int edgeOrCorner = isOnEdgeOrCorner(currentItem, event->pos());

        Action *action;

        if (!edgeOrCorner){
            Actions move = Actions::Move;
            action = new Action(move, currentItem, startingPosition, QRectF());
        }else{
            Actions resize = Actions::Resize;
            action = new Action(resize, currentItem, startingPosition, startingBounds);
        }
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
    QGraphicsRectItem *rect = dynamic_cast<QGraphicsRectItem*>(action->item);
    if (action->actionType == Move){
        //get current position
        QPointF currentPos = action->item->pos();
        //move item
        action->item->setPos(action->position);
        //update position in action
        action->position = currentPos;
    }else if(action->actionType == Resize && rect){
        QPointF currentPos = rect->pos();
        //move item
        rect->setPos(action->position);
        //update position in action
        action->position = currentPos;


        //get current size
        QRectF currentBounds = rect->boundingRect();
        //resize
        qreal w = action->size.width();
        qreal h = action->size.height();
        rect->setRect(0, 0, w, h);
        //update size in action
        action->size = currentBounds;
    }
}


void LayoutEditorGraphicsView::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    if (scene) {        
        QRectF newRect = QRectF(QPointF(0, 0), QSizeF(this->viewport()->size()));
        scene->setSceneRect(newRect);
    }
}

int LayoutEditorGraphicsView::isOnEdgeOrCorner(QGraphicsItem *item, const QPointF &mousePos) {
    if (!item) {
        return 0; // Safety check, in case the item is null
    }

    QRectF rect = item->boundingRect();
    rect = item->mapRectToScene(rect); // Map to scene coordinates

    const qreal margin = 10.0;

    // Check corners
    if (mousePos.x() >= rect.left() - margin && mousePos.x() <= rect.left() + margin) {
        if (mousePos.y() >= rect.top() - margin && mousePos.y() <= rect.top() + margin) {
            return 1; // Top-left corner
        } else if (mousePos.y() >= rect.bottom() - margin && mousePos.y() <= rect.bottom() + margin) {
            return 2; // Bottom-left corner
        }
    } else if (mousePos.x() >= rect.right() - margin && mousePos.x() <= rect.right() + margin) {
        if (mousePos.y() >= rect.top() - margin && mousePos.y() <= rect.top() + margin) {
            return 3; // Top-right corner
        } else if (mousePos.y() >= rect.bottom() - margin && mousePos.y() <= rect.bottom() + margin) {
            return 4; // Bottom-right corner
        }
    }

    // Check edges
    if (mousePos.x() >= rect.left() - margin && mousePos.x() <= rect.left() + margin) {
        return 5; // Left edge
    } else if (mousePos.x() >= rect.right() - margin && mousePos.x() <= rect.right() + margin) {
        return 6; // Right edge
    } else if (mousePos.y() >= rect.top() - margin && mousePos.y() <= rect.top() + margin) {
        return 7; // Top edge
    } else if (mousePos.y() >= rect.bottom() - margin && mousePos.y() <= rect.bottom() + margin) {
        return 8; // Bottom edge
    }

    return 0; // Not on edge or corner
}

