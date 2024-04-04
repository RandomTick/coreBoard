#include <QMouseEvent>
#include <QGraphicsItem>
#include "layouteditorgraphicsview.h"
#include "layouteditor.h"
#include "resizablerectitem.h"
//#include <iostream>

LayoutEditor *layoutEditor;

LayoutEditorGraphicsView::LayoutEditorGraphicsView(QWidget *parent) : QGraphicsView(parent) {
    layoutEditor = (LayoutEditor*)parent;
    currentItem = nullptr;
    activeAction = Actions::None;
    edgeOffset = QPointF(0, 0);
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
            startingBounds = getCorrectBoundingRect(item);
        }else{
            offset = QPointF();
            startingPosition = QPointF();
        }
    }
}

void LayoutEditorGraphicsView::enforceRectSize(QPointF &newPos, qreal &newWidth, qreal &newHeight){
    if (newWidth < 20.0 || newHeight < 20.0){
        //compensate for rectangle position

        newPos = QPointF(newPos.x(), newPos.y());
        Actions resize = Actions::Resize;
        Action *action = new Action(resize, currentItem, startingPosition, startingBounds);

        undoActions.push_back(action);
        redoActions.clear();

        action = nullptr;
        currentItem = nullptr;
        activeAction = Actions::None;
        layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
        setCursor(Qt::ArrowCursor);
    }

    if (newWidth < 25.0) {
        qreal deltaWidth = 25.0 - newWidth;
        newPos.setX(newPos.x() - deltaWidth);
    }
    if (newHeight < 25.0) {
        qreal deltaHeight = 25.0 - newHeight;
        newPos.setY(newPos.y() - deltaHeight);
    }

    // Enforce minimum width and height of 20
    newWidth = qMax(newWidth, 25.0);
    newHeight = qMax(newHeight, 25.0);

}

QRectF LayoutEditorGraphicsView::getCorrectBoundingRect(QGraphicsItem *item){
    QRectF startingBoundsWrong = item->boundingRect();
    return QRectF(0, 0, startingBoundsWrong.width() - 1, startingBoundsWrong.height() - 1);
}

void LayoutEditorGraphicsView::mouseMoveEvent(QMouseEvent *event) {
    if (currentItem) {

        int edgeOrCorner = isOnEdgeOrCorner(currentItem, event->pos());
        //QRectF currentBounds = getCorrectBoundingRect(currentItem);

        if (edgeOrCorner != 0 && (activeAction == None || activeAction == Resize)) {
            activeAction = Actions::Resize;

            // Handle resizing for each shape differently
            QPointF newPos = mapToScene(event->pos());
            qreal xOffset = startingPosition.x() - newPos.x() + edgeOffset.x();
            qreal yOffset = startingPosition.y() - newPos.y() + edgeOffset.y();
            qreal newWidth = 0;
            qreal newHeight = 0;


            ResizableRectItem *rect = dynamic_cast<ResizableRectItem*>(currentItem);
            if (rect){ //is rectangle
                switch (edgeOrCorner) {
                    case 1: // Top-Left Corner
                        newWidth = startingBounds.width() + xOffset;
                        newHeight = startingBounds.height() + yOffset;

                        enforceRectSize(newPos, newWidth, newHeight);

                        // Update the rectangle's position and size
                        rect->setPos(newPos - edgeOffset);
                        rect->setRect(0, 0, newWidth, newHeight);
                        break;
                    case 2: // Bottom-Left Corner
                        yOffset = startingPosition.y() - newPos.y() + startingBounds.height();

                        newWidth = startingBounds.width() + xOffset;
                        newHeight = startingBounds.height() - yOffset - edgeOffset.y();

                        enforceRectSize(newPos, newWidth, newHeight);

                        rect->setPos(newPos.x() - edgeOffset.x()  , startingPosition.y() );
                        rect->setRect(0, 0, newWidth, newHeight);
                        break;
                    case 3: // Top-Right Corner
                        xOffset = startingPosition.x() - newPos.x() + startingBounds.width();
                        newWidth = startingBounds.width() - xOffset + edgeOffset.x();
                        newHeight = startingBounds.height() + yOffset;

                        enforceRectSize(newPos, newWidth, newHeight);

                        rect->setPos(startingPosition.x(), newPos.y() - edgeOffset.y());
                        rect->setRect(0, 0, newWidth, newHeight);
                        break;
                    case 4: // Bottom-Right Corner
                        xOffset = startingPosition.x() - newPos.x() + startingBounds.width();
                        yOffset = startingPosition.y() - newPos.y() + startingBounds.height();
                        newWidth = startingBounds.width() - xOffset + edgeOffset.x();
                        newHeight = startingBounds.height() - yOffset - edgeOffset.y();

                        enforceRectSize(newPos, newWidth, newHeight);

                        rect->setRect(0, 0, newWidth, newHeight);
                        break;

                    case 5: // Left Edge
                        newWidth = startingBounds.width() + xOffset;
                        newHeight = startingBounds.height();

                        enforceRectSize(newPos, newWidth, newHeight);

                        rect->setPos(newPos.x() - edgeOffset.x(), newPos.y() - edgeOffset.y() + yOffset);
                        rect->setRect(0, 0, newWidth, newHeight);
                        break;
                    case 6: // Right Edge
                        xOffset = startingPosition.x() - newPos.x() + startingBounds.width();
                        newWidth = startingBounds.width() - xOffset + edgeOffset.x();
                        newHeight = startingBounds.height();

                        enforceRectSize(newPos, newWidth, newHeight);

                        rect->setPos(startingPosition.x(), newPos.y() - edgeOffset.y() + yOffset);
                        rect->setRect(0, 0, newWidth, newHeight);
                        break;
                    case 7: // Top Edge
                        newWidth = startingBounds.width();
                        newHeight = startingBounds.height() + yOffset;

                        enforceRectSize(newPos, newWidth, newHeight);

                        rect->setPos(newPos.x() - edgeOffset.x() + xOffset, newPos.y() - edgeOffset.y());
                        rect->setRect(0, 0, newWidth, newHeight);
                        break;
                    case 8: // Bottom Edge
                        newWidth = startingBounds.width();
                        newHeight = - yOffset + edgeOffset.y();

                        enforceRectSize(newPos, newWidth, newHeight);

                        rect->setPos(newPos.x() - edgeOffset.x() + xOffset, startingPosition.y());
                        rect->setRect(0, 0, newWidth, newHeight);
                        break;
                }
            }

        }else if (activeAction == None || activeAction == Move){

            activeAction = Actions::Move;
            //Move
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
        }
        //implement hover event for resize, need to do this seperately, because we do not have currentItem

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
            if (activeAction == None){
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
        activeAction = Actions::None;

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
    ResizableRectItem *rect = dynamic_cast<ResizableRectItem*>(action->item);
    if (action->actionType == Move){
        //get current position
        QPointF currentPos = action->item->pos();
        //move item
        action->item->setPos(action->position);
        //update position in action
        action->position = currentPos;
    }else if(action->actionType == Resize && rect){
        QPointF currentPos = rect->pos();
        //std::cout << currentPos.x() << ", " << currentPos.y() << std::endl;
        //move item
        QPointF newPos = QPointF(action->position.x(), action->position.y());
        rect->setPos(newPos);
        //update position in action
        action->position = currentPos;


        //get current size
        QRectF currentBounds = getCorrectBoundingRect(rect);
        //std::cout << currentBounds.width() << ", " << currentBounds.height() << std::endl;
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

    QGraphicsRectItem* rectItem = dynamic_cast<QGraphicsRectItem*>(item);
    if (!rectItem) {
        return 0;// prevent 1 when hovering over text, might need to be changed if we allow other shapes.
    }

    QRectF rect = item->boundingRect();
    rect = item->mapRectToScene(rect); // Map to scene coordinates

    const qreal margin = 10.0;

    // Check corners and edges and set edgeOffset
    if (mousePos.x() >= rect.left() - margin && mousePos.x() <= rect.left() + margin) {
        if (activeAction == Actions::None){
            edgeOffset.setX(mousePos.x() - rect.left()); // Horizontal distance from left edge
        }
        if (mousePos.y() >= rect.top() - margin && mousePos.y() <= rect.top() + margin) {
            if (activeAction == Actions::None){
                edgeOffset.setY(mousePos.y() - rect.top()); // Vertical distance from top edge
            }
            return 1; // Top-left corner
        } else if (mousePos.y() >= rect.bottom() - margin && mousePos.y() <= rect.bottom() + margin) {
            if (activeAction == Actions::None){
                edgeOffset.setY(mousePos.y() - rect.bottom()); // Vertical distance from bottom edge
            }
            return 2; // Bottom-left corner
        }
        return 5; // Left edge
    } else if (mousePos.x() >= rect.right() - margin && mousePos.x() <= rect.right() + margin) {
        if (activeAction == Actions::None){
            edgeOffset.setX(qAbs(mousePos.x() - rect.right())); // Horizontal distance from right edge
        }
        if (mousePos.y() >= rect.top() - margin && mousePos.y() <= rect.top() + margin) {
            if (activeAction == Actions::None){
                edgeOffset.setY(mousePos.y() - rect.top()); // Vertical distance from top edge
            }
            return 3; // Top-right corner
        } else if (mousePos.y() >= rect.bottom() - margin && mousePos.y() <= rect.bottom() + margin) {
            if (activeAction == Actions::None){
                edgeOffset.setY(mousePos.y() - rect.bottom()); // Vertical distance from bottom edge
            }
            return 4; // Bottom-right corner
        }
        return 6; // Right edge
    }

    if (mousePos.y() >= rect.top() - margin && mousePos.y() <= rect.top() + margin) {
        if (activeAction == Actions::None){
            edgeOffset.setY(mousePos.y() - rect.top()); // Vertical distance from top edge
        }
        return 7; // Top edge
    } else if (mousePos.y() >= rect.bottom() - margin && mousePos.y() <= rect.bottom() + margin) {
        if (activeAction == Actions::None){
            edgeOffset.setY(mousePos.y() - rect.bottom()); // Vertical distance from bottom edge
        }
        return 8; // Bottom edge
    }

    return 0; // Not on edge or corner
}


