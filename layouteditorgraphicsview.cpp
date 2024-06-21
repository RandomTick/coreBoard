#include <QMouseEvent>
#include <QGraphicsItem>
#include "layouteditorgraphicsview.h"
#include "layouteditor.h"
#include "resizablerectitem.h"
#include "symbolinputdialog.h"
#include <QMenu>
#include <QInputDialog>
#include <float.h>
//#include <iostream>

LayoutEditor *layoutEditor;
QList<QGraphicsItem*> alignmentHelpers;


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
    QGraphicsTextItem* textItem = dynamic_cast<QGraphicsTextItem*>(item);
    if(textItem){
        //when text gets selected, we need to select the closest rectangle in order to move/resize the correct one
        item = item->parentItem();
    }
    if (event->button() == Qt::RightButton) {
        //TODO: implement right click stuff, like changing bind/shape etc.
        qDebug("right test");
        if(item){
            //change item properties
            ResizableRectItem *rect = dynamic_cast<ResizableRectItem*>(item);
            if (rect){
                QMenu menu;
                QAction *actionRename = menu.addAction("Rename");
                QAction *actionRebind = menu.addAction("Rebind");
                QAction *actionDelete = menu.addAction("Delete");
                QAction *selectedAction = menu.exec(QCursor::pos());
                // Handle Actions
                if (selectedAction == actionRename) {
                    QString oldText = rect->getText();
                    SymbolInputDialog *dialog = new SymbolInputDialog(this, oldText);
                    if (dialog->exec() == QDialog::Accepted) {
                        QString text = dialog->getText();
                        rect->setText(text);
                    }
                    delete dialog;

                    //log action:
                    QString newText = rect->getText();
                    if (newText != oldText){
                        Action* action = new Action(Actions::ChangeText, rect, oldText, newText);
                        undoActions.push_back(action);
                        redoActions.clear();
                        layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
                    }

                } else if (selectedAction == actionRebind) {
                    qDebug("2");
                }else if (selectedAction == actionDelete) {
                    qDebug("3");
                }
                return;
            }
        }else{
            //change background properties/general stuff, possibly dont want to do this in here, preferably in the keyboardWidget?. Depends on wheather we want to
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

void LayoutEditorGraphicsView::centerText(QGraphicsRectItem *rect){

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
                //display size of rect
                updateSizeHelpers(rect);

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

            updateAlignmentHelpers(currentItem);

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
        bool validAction = false;

        if (!edgeOrCorner){
            //find out if starting position is different from current Position
            if (startingPosition != currentItem->pos()){
                action = new Action(Actions::Move, currentItem, startingPosition);
                validAction = true;
            }

        }else{
            if (startingPosition != currentItem->pos() || startingBounds != getCorrectBoundingRect(currentItem)){
                action = new Action(Actions::Resize, currentItem, startingPosition, startingBounds);
                validAction = true;
            }

        }

        qDeleteAll(alignmentHelpers);
        alignmentHelpers.clear();


        if (validAction){
            undoActions.push_back(action);
            redoActions.clear();
            action = nullptr;
            currentItem = nullptr;
            activeAction = Actions::None;
            layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
        }else{
            //TODO: select the item for moving with arrow keys or multiselect

            //for now
            action = nullptr;
            currentItem = nullptr;
            activeAction = Actions::None;
            layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
        }
    }
}

void LayoutEditorGraphicsView::addRectAction(QGraphicsItem* item){
    Action *action;
    action = new Action(Actions::Add, item);
    undoActions.push_back(action);
    redoActions.clear();
    action = nullptr;
    layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
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
    }else if(action->actionType == ChangeText && rect){
        rect->setText(action->oldText);
        //update action to reverse
        action->oldText = action->newText;
        action->newText = rect->getText();
    }else if(action->actionType == Add){
        action->actionType = Actions::Remove;
        action->oldText = rect->getText();
        QPointF currentPos = rect->pos();
        action->position = currentPos;
        QRectF currentBounds = getCorrectBoundingRect(rect);
        action->size = currentBounds;
        action->keyCodes = rect->getKeycodes();

        scene->removeItem(rect);
    }else if (action->actionType == Remove){
        ResizableRectItem *rect = layoutEditor->addRectangle(action->oldText, action->size.height(), action->size.width(), action->position.x(),action->position.y(), action->keyCodes);

        action->actionType = Actions::Add;
        action->item = rect;
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


void LayoutEditorGraphicsView::updateAlignmentHelpers(QGraphicsItem* movingItem) {
    // Clear existing alignment helpers
    for (auto* item : alignmentHelpers) {
        scene->removeItem(item);
        delete item;
    }
    alignmentHelpers.clear();

    qreal closestLeft = FLT_MAX;
    qreal closestRight = FLT_MAX;
    qreal closestTop = FLT_MAX;
    qreal closestBottom = FLT_MAX;


    QRectF movingRect = movingItem->boundingRect().translated(movingItem->pos());

    // Find the closest items in each direction
    for (QGraphicsItem* item : scene->items()) {
        if (item == movingItem || item->type() == QGraphicsLineItem::Type || item->type() == QGraphicsTextItem::Type) continue;

        QRectF itemRect = item->boundingRect().translated(item->pos());

        // For left/right calculations, check if the y ranges overlap
        if (rangesOverlap(movingRect.top(), movingRect.bottom(), itemRect.top(), itemRect.bottom())) {
            // Check left
            if (itemRect.right() < movingRect.left() && movingRect.left() - itemRect.right() < closestLeft) {
                closestLeft = movingRect.left() - itemRect.right();
            }
            // Check right
            if (itemRect.left() > movingRect.right() && itemRect.left() - movingRect.right() < closestRight) {
                closestRight = itemRect.left() - movingRect.right();
            }
        }

        // For top/bottom calculations, check if the x ranges overlap
        if (rangesOverlap(movingRect.left(), movingRect.right(), itemRect.left(), itemRect.right())) {
            // Check top
            if (itemRect.bottom() < movingRect.top() && movingRect.top() - itemRect.bottom() < closestTop) {
                closestTop = movingRect.top() - itemRect.bottom();
            }
            // Check bottom
            if (itemRect.top() > movingRect.bottom() && itemRect.top() - movingRect.bottom() < closestBottom) {
                closestBottom = itemRect.top() - movingRect.bottom();
            }
        }
    }

    // Check scene borders as potential closest edges
    QRectF sceneRect = scene->sceneRect();
    if (movingRect.left() - sceneRect.left() < closestLeft) {
        closestLeft = movingRect.left() - sceneRect.left();
    }
    if (sceneRect.right() - movingRect.right() < closestRight) {
        closestRight = sceneRect.right() - movingRect.right();
    }
    if (movingRect.top() - sceneRect.top() < closestTop) {
        closestTop = movingRect.top() - sceneRect.top();
    }
    if (sceneRect.bottom() - movingRect.bottom() < closestBottom) {
        closestBottom = sceneRect.bottom() - movingRect.bottom();
    }

    // Draw alignment lines for the closest items or borders
    drawAlignmentLine(movingRect, closestLeft, Qt::Horizontal, true); // Left
    drawAlignmentLine(movingRect, closestRight, Qt::Horizontal, false); // Right
    drawAlignmentLine(movingRect, closestTop, Qt::Vertical, true); // Top
    drawAlignmentLine(movingRect, closestBottom, Qt::Vertical, false); // Bottom
}

void LayoutEditorGraphicsView::drawAlignmentLine(const QRectF& movingRect, qreal distance, Qt::Orientation orientation, bool isStartSide) {
    if (distance == FLT_MAX) return; // No close item in this direction

    QPointF startPoint, endPoint, textPos;
    if (orientation == Qt::Horizontal) {
        qreal y = movingRect.top() + movingRect.height() / 2;
        startPoint.setY(y);
        endPoint.setY(y);
        if (isStartSide) {
            startPoint.setX(movingRect.left());
            endPoint.setX(movingRect.left() - distance);
            textPos = (startPoint + endPoint) / 2; // Default position for text
            // Adjust if too close to the left edge
            if (endPoint.x() < 0) textPos.setX(movingRect.left() - distance / 2);
        } else {
            startPoint.setX(movingRect.right());
            endPoint.setX(movingRect.right() + distance);
            textPos = (startPoint + endPoint) / 2;
            // Adjust if too close to the right edge
            if (endPoint.x() > scene->width() - 27) textPos.setX(movingRect.right() + distance / 2 - 27); // Assuming text width ~27px
        }
    } else { // Vertical
        qreal x = movingRect.left() + movingRect.width() / 2;
        startPoint.setX(x);
        endPoint.setX(x);
        if (isStartSide) {
            startPoint.setY(movingRect.top());
            endPoint.setY(movingRect.top() - distance);
            textPos = (startPoint + endPoint) / 2;
            // Adjust if too close to the top edge
            if (endPoint.y() < 0) textPos.setY(movingRect.top() - distance / 2);
        } else {
            startPoint.setY(movingRect.bottom());
            endPoint.setY(movingRect.bottom() + distance);
            textPos = (startPoint + endPoint) / 2;
            // Adjust if too close to the bottom edge
            if (endPoint.y() > scene->height() - 20) textPos.setY(movingRect.bottom() + distance / 2 - 20); // Assuming text height ~20px
        }
    }

    QGraphicsLineItem* line = scene->addLine(QLineF(startPoint, endPoint), QPen(Qt::red, 1, Qt::DashLine));
    alignmentHelpers.append(line);

    // Optionally, add distance text near the line
    if (distance < 0) distance = 0; //avoid weird formatting
    QGraphicsTextItem* textItem = scene->addText(QString::number(qRound(distance)) + " px");
    textItem->setDefaultTextColor(Qt::red);
    textItem->setPos(textPos);
    alignmentHelpers.append(textItem);
}

bool LayoutEditorGraphicsView::rangesOverlap(qreal start1, qreal end1, qreal start2, qreal end2) {
    qreal middle1 = (start1 + end1) / 2; // Calculate the middle point of the current item
    return middle1 >= start2 && middle1 <= end2; // Check if the middle point lies within the bounds of the other item

}


void LayoutEditorGraphicsView::updateSizeHelpers(QGraphicsItem* item) {
    // Ensure the item is valid and the type expected
    if (!item || !(item->type() == QGraphicsRectItem::Type || item->type() == QGraphicsEllipseItem::Type))
        return;

    qDeleteAll(alignmentHelpers);
    alignmentHelpers.clear();

    QRectF rect = item->boundingRect().translated(item->pos());

    // Horizontal size helper - Display width inside the rectangle
    QPointF startWidthPoint(rect.left(), rect.top() + rect.height() / 2);
    QPointF endWidthPoint(rect.left() + rect.width(), rect.top() + rect.height() / 2);
    QGraphicsLineItem* lineWidth = scene->addLine(QLineF(startWidthPoint, endWidthPoint), QPen(Qt::blue, 1, Qt::DashLine));
    alignmentHelpers.append(lineWidth);

    QGraphicsTextItem* textWidthItem = scene->addText(QString::number(qRound(rect.width())) + " px");
    textWidthItem->setDefaultTextColor(Qt::blue);
    // Position the width text item in the middle of the line, adjusting for the text's size
    textWidthItem->setPos(rect.left() + rect.width() / 4 - textWidthItem->boundingRect().width() / 2,
                          startWidthPoint.y() - textWidthItem->boundingRect().height() / 2 - 10);
    alignmentHelpers.append(textWidthItem);

    // Vertical size helper - Display height inside the rectangle
    QPointF startHeightPoint(rect.left() + rect.width() / 2, rect.top());
    QPointF endHeightPoint(rect.left() + rect.width() / 2, rect.top() + rect.height());
    QGraphicsLineItem* lineHeight = scene->addLine(QLineF(startHeightPoint, endHeightPoint), QPen(Qt::blue, 1, Qt::DashLine));
    alignmentHelpers.append(lineHeight);

    QGraphicsTextItem* textHeightItem = scene->addText(QString::number(qRound(rect.height())) + " px");
    textHeightItem->setDefaultTextColor(Qt::blue);
    // Position the height text item in the middle of the line, adjusting for the text's size
    textHeightItem->setPos(startHeightPoint.x() - textHeightItem->boundingRect().width() / 4 + 10,
                           rect.top() + rect.height() / 4 - textHeightItem->boundingRect().height() / 2);
    alignmentHelpers.append(textHeightItem);
}




