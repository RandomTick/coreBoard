#include <QMouseEvent>
#include <QFocusEvent>
#include <QGraphicsItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QPainter>
#include "layouteditorgraphicsview.h"
#include "layouteditor.h"
#include "resizablerectitem.h"
#include "resizableellipseitem.h"
#include "resizablepolygonitem.h"
#include "dialogtextchange.h"
#include "dialogkeycodechange.h"
#include "dialogstyle.h"
#include "mainwindow.h"
#include "windowskeylistener.h"
#ifdef Q_OS_WIN
#include "windowsmouselistener.h"
#endif
#include "keystyle.h"
#include <QMenu>
#include <QKeyEvent>
#include <QSet>
#include <float.h>

static KeyStyle keyStyleForItem(QGraphicsItem *item) {
    if (ResizableRectItem *r = dynamic_cast<ResizableRectItem*>(item)) return r->keyStyle();
    if (ResizableEllipseItem *e = dynamic_cast<ResizableEllipseItem*>(item)) return e->keyStyle();
    if (ResizablePolygonItem *p = dynamic_cast<ResizablePolygonItem*>(item)) return p->keyStyle();
    return KeyStyle();
}
static void setKeyStyleForItem(QGraphicsItem *item, const KeyStyle &s) {
    if (ResizableRectItem *r = dynamic_cast<ResizableRectItem*>(item)) { r->setKeyStyle(s); return; }
    if (ResizableEllipseItem *e = dynamic_cast<ResizableEllipseItem*>(item)) { e->setKeyStyle(s); return; }
    if (ResizablePolygonItem *p = dynamic_cast<ResizablePolygonItem*>(item)) p->setKeyStyle(s);
}

LayoutEditor *layoutEditor;
QList<QGraphicsItem*> alignmentHelpers;


LayoutEditorGraphicsView::LayoutEditorGraphicsView(QWidget *parent) : QGraphicsView(parent) {
    layoutEditor = (LayoutEditor*)parent;
    currentItem = nullptr;
    activeAction = Actions::None;
    edgeOffset = QPointF(0, 0);
    setDragMode(QGraphicsView::RubberBandDrag);
    setFocusPolicy(Qt::StrongFocus);
    m_arrowDirection = 0;
    m_arrowCommitTimer = new QTimer(this);
    m_arrowCommitTimer->setSingleShot(true);
    connect(m_arrowCommitTimer, &QTimer::timeout, this, &LayoutEditorGraphicsView::commitArrowKeyMoveSegment);
}


void LayoutEditorGraphicsView::setSceneAndStore(QGraphicsScene *externScene){
    scene = externScene;
    this->setScene(scene);
    connect(scene, &QGraphicsScene::selectionChanged, this, &LayoutEditorGraphicsView::updateAlignmentHelpersForSelection);
}

void LayoutEditorGraphicsView::mousePressEvent(QMouseEvent *event) {
    QGraphicsItem *item = scene->itemAt(mapToScene(event->pos()), QTransform());
    QGraphicsTextItem* textItem = dynamic_cast<QGraphicsTextItem*>(item);
    if(textItem){
        //when text gets selected, we need to select the closest rectangle in order to move/resize the correct one
        item = item->parentItem();
    }
    if (event->button() == Qt::RightButton) {
        if (m_pickStyleMode || m_applyStyleMode || m_copyMode) {
            m_pickStyleMode = false;
            m_applyStyleMode = false;
            m_copyMode = false;
            setCursor(Qt::ArrowCursor);
            emit applyStyleModeExited();
            event->accept();
            return;
        }
        if(item){
            ResizableRectItem *rect = dynamic_cast<ResizableRectItem*>(item);
            ResizableEllipseItem *ellipse = dynamic_cast<ResizableEllipseItem*>(item);
            ResizablePolygonItem *polygon = dynamic_cast<ResizablePolygonItem*>(item);
            if (rect || ellipse || polygon){
                QMenu menu;
                QAction *actionRename = menu.addAction(tr("Rename"));
                QAction *actionRebind = menu.addAction(tr("Rebind"));
                QAction *actionStyle = menu.addAction(tr("Edit style..."));
                menu.addSeparator();
                QAction *actionDelete = menu.addAction(tr("Delete"));
                QAction *selectedAction = menu.exec(QCursor::pos());
                QGraphicsItem *keyItem = rect ? static_cast<QGraphicsItem*>(rect) : (ellipse ? static_cast<QGraphicsItem*>(ellipse) : static_cast<QGraphicsItem*>(polygon));

                QList<QGraphicsItem*> styleTargets;
                if (item->isSelected()) {
                    for (QGraphicsItem *sel : scene->selectedItems()) {
                        if (dynamic_cast<ResizableRectItem*>(sel) || dynamic_cast<ResizableEllipseItem*>(sel) || dynamic_cast<ResizablePolygonItem*>(sel))
                            styleTargets.append(sel);
                    }
                }
                if (styleTargets.isEmpty())
                    styleTargets.append(keyItem);
                auto getText = [rect, ellipse, polygon]() { return rect ? rect->getText() : (ellipse ? ellipse->getText() : polygon->getText()); };
                auto setText = [rect, ellipse, polygon](const QString &t) { if (rect) rect->setText(t); else if (ellipse) ellipse->setText(t); else polygon->setText(t); };
                auto getShiftText = [rect, ellipse, polygon]() { return rect ? rect->getShiftText() : (ellipse ? ellipse->getShiftText() : polygon->getShiftText()); };
                auto setShiftText = [rect, ellipse, polygon](const QString &t) { if (rect) rect->setShiftText(t); else if (ellipse) ellipse->setShiftText(t); else polygon->setShiftText(t); };
                auto getKeycodes = [rect, ellipse, polygon]() { return rect ? rect->getKeycodes() : (ellipse ? ellipse->getKeycodes() : polygon->getKeycodes()); };
                auto setKeycodes = [rect, ellipse, polygon](const std::list<int> &kc) { if (rect) rect->setKeycodes(kc); else if (ellipse) ellipse->setKeycodes(kc); else polygon->setKeycodes(kc); };

                if (selectedAction == actionRename) {
                    QString oldText = getText();
                    QString oldShiftText = getShiftText();
                    DialogTextChange *dialog = new DialogTextChange(this, oldText, oldShiftText);
                    if (dialog->exec() == QDialog::Accepted) {
                        setText(dialog->getText());
                        setShiftText(dialog->getShiftText());
                    }
                    delete dialog;
                    QString newText = getText();
                    QString newShiftText = getShiftText();
                    if (newText != oldText || newShiftText != oldShiftText) {
                        Action *action = new Action(Actions::ChangeText, keyItem, oldText, newText, oldShiftText, newShiftText);
                        undoActions.push_back(action);
                        redoActions.clear();
                        layoutEditor->markDirty();
                        layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
                    }
                } else if (selectedAction == actionRebind) {
                    std::list<int> oldKeycodes = getKeycodes();
                    std::list<int> newKeycodes = oldKeycodes;
                    QWidget *pw = layoutEditor->parentWidget();
                    MainWindow *mainWin = pw ? qobject_cast<MainWindow*>(pw->parentWidget()) : nullptr;
                    WindowsKeyListener *appKeyListener = mainWin ? mainWin->keyListener() : nullptr;
#ifdef Q_OS_WIN
                    WindowsMouseListener *appMouseListener = mainWin ? mainWin->mouseListener() : nullptr;
                    DialogKeycodeChange *dialog = new DialogKeycodeChange(this, oldKeycodes, appKeyListener, appMouseListener);
#else
                    DialogKeycodeChange *dialog = new DialogKeycodeChange(this, oldKeycodes, appKeyListener);
#endif
                    if (dialog->exec() == QDialog::Accepted){
                        newKeycodes = dialog->getKeyCodes();
                        setKeycodes(newKeycodes);
                    }
                    delete dialog;
                    if (oldKeycodes != newKeycodes){
                        Action* action = new Action(Actions::ChangeKeyCodes, keyItem, oldKeycodes, newKeycodes);
                        undoActions.push_back(action);
                        redoActions.clear();
                    layoutEditor->markDirty();
                    layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
                    }
                } else if (selectedAction == actionStyle) {
                    KeyStyle current = keyStyleForItem(styleTargets.first());
                    QString previewLabel = (rect ? rect->getText() : (ellipse ? ellipse->getText() : polygon->getText()));
                    DialogStyle *dialog = new DialogStyle(this, current, previewLabel, rect != nullptr);
                    if (dialog->exec() == QDialog::Accepted) {
                        KeyStyle newStyle = dialog->getStyle();
                        Action *action = new Action(Actions::ChangeStyle, nullptr);
                        action->styleNew = newStyle;
                        for (QGraphicsItem *target : styleTargets) {
                            action->styleItems.append(qMakePair(target, keyStyleForItem(target)));
                            setKeyStyleForItem(target, newStyle);
                        }
                        undoActions.push_back(action);
                        redoActions.clear();
                        layoutEditor->markDirty();
                        layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
                    }
                    delete dialog;
                } else if (selectedAction == actionDelete) {
                    Action * action = new Action(Actions::Remove, keyItem);
                    action->oldText = getText();
                    action->position = keyItem->pos();
                    action->size = getCorrectBoundingRect(keyItem);
                    action->oldKeycodes = getKeycodes();
                    undoActions.push_back(action);
                    redoActions.clear();
                    currentItem = nullptr;
                    layoutEditor->markDirty();
                    layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
                    scene->removeItem(keyItem);
                }
                return;
            }
        }else{
            //change background properties/general stuff, possibly dont want to do this in here, preferably in the keyboardWidget?. Depends on wheather we want to
        }
    } else {
        if (event->button() == Qt::LeftButton) {
            if (m_copyMode && !item) {
                event->accept();
                return;
            }
            if (item) {
                ResizableRectItem *rect = dynamic_cast<ResizableRectItem*>(item);
                ResizableEllipseItem *ellipse = dynamic_cast<ResizableEllipseItem*>(item);
                ResizablePolygonItem *polygon = dynamic_cast<ResizablePolygonItem*>(item);
                if (rect || ellipse || polygon) {
                    if (m_pickStyleMode) {
                        m_pickedStyle = keyStyleForItem(item);
                        m_hasPickedStyle = true;
                        m_pickStyleMode = false;
                        setCursor(Qt::ArrowCursor);
                        emit stylePicked();
                        return;
                    }
                    if (m_copyMode) {
                        emit keyCopied();
                        layoutEditor->copyKeyFromItem(item);
                        m_copyMode = false;
                        return;
                    }
                    if (m_applyStyleMode && m_hasPickedStyle) {
                        QList<QGraphicsItem*> targets;
                        if (item->isSelected()) {
                            for (QGraphicsItem *sel : scene->selectedItems()) {
                                if (dynamic_cast<ResizableRectItem*>(sel) || dynamic_cast<ResizableEllipseItem*>(sel) || dynamic_cast<ResizablePolygonItem*>(sel))
                                    targets.append(sel);
                            }
                        }
                        if (targets.isEmpty())
                            targets.append(item);
                        Action *action = new Action(Actions::ChangeStyle, nullptr);
                        action->styleNew = m_pickedStyle;
                        for (QGraphicsItem *target : targets) {
                            action->styleItems.append(qMakePair(target, keyStyleForItem(target)));
                            setKeyStyleForItem(target, m_pickedStyle);
                        }
                        undoActions.push_back(action);
                        redoActions.clear();
                        layoutEditor->markDirty();
                        layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
                        return;
                    }
                    if (event->modifiers() & Qt::ControlModifier) {
                        item->setSelected(!item->isSelected());
                    } else {
                        QList<QGraphicsItem*> sel = selectedKeyItems();
                        if (!item->isSelected() || sel.size() <= 1) {
                            scene->clearSelection();
                            item->setSelected(true);
                        }
                    }
                    currentItem = item;
                    offset = mapToScene(event->pos()) - item->pos();
                    startingPosition = item->pos();
                    startingBounds = getCorrectBoundingRect(item);
                    m_dragItems.clear();
                    QList<QGraphicsItem*> toDrag = item->isSelected() ? selectedKeyItems() : QList<QGraphicsItem*>{item};
                    for (QGraphicsItem *dragItem : toDrag)
                        m_dragItems.append(qMakePair(dragItem, dragItem->pos()));
                    event->accept();
                } else {
                    if (m_copyMode) {
                        event->accept();
                        return;
                    }
                    currentItem = nullptr;
                    QGraphicsView::mousePressEvent(event);
                }
            } else {
                currentItem = nullptr;
                QGraphicsView::mousePressEvent(event);
            }
        } else {
            if (item) {
                currentItem = item;
                offset = mapToScene(event->pos()) - item->pos();
                startingPosition = item->pos();
                startingBounds = getCorrectBoundingRect(item);
                m_dragItems.clear();
                if (dynamic_cast<ResizableRectItem*>(item) || dynamic_cast<ResizableEllipseItem*>(item) || dynamic_cast<ResizablePolygonItem*>(item)) {
                    QList<QGraphicsItem*> toDrag = item->isSelected() ? selectedKeyItems() : QList<QGraphicsItem*>{item};
                    for (QGraphicsItem *dragItem : toDrag)
                        m_dragItems.append(qMakePair(dragItem, dragItem->pos()));
                }
            } else {
                offset = QPointF();
                startingPosition = QPointF();
                m_dragItems.clear();
            }
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
                    layoutEditor->markDirty();
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
    QPointF mouseScenePos = mapToScene(event->pos());

    if (currentItem) {
        int edgeOrCorner = isOnEdgeOrCorner(currentItem, mouseScenePos);
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
            ResizableEllipseItem *ellipse = dynamic_cast<ResizableEllipseItem*>(currentItem);
            ResizablePolygonItem *polygon = dynamic_cast<ResizablePolygonItem*>(currentItem);
            if (rect || ellipse || polygon){
                switch (edgeOrCorner) {
                    case 1: // Top-Left Corner
                        newWidth = startingBounds.width() + xOffset;
                        newHeight = startingBounds.height() + yOffset;

                        enforceRectSize(newPos, newWidth, newHeight);

                        if (rect) { rect->setPos(newPos - edgeOffset); rect->setRect(0, 0, newWidth, newHeight); }
                        else if (ellipse) { ellipse->setPos(newPos - edgeOffset); ellipse->setRect(0, 0, newWidth, newHeight); }
                        else { polygon->setPos(newPos - edgeOffset); polygon->setSize(newWidth, newHeight); }
                        break;
                    case 2: // Bottom-Left Corner
                        yOffset = startingPosition.y() - newPos.y() + startingBounds.height();

                        newWidth = startingBounds.width() + xOffset;
                        newHeight = startingBounds.height() - yOffset - edgeOffset.y();

                        enforceRectSize(newPos, newWidth, newHeight);

                        if (rect) { rect->setPos(newPos.x() - edgeOffset.x(), startingPosition.y()); rect->setRect(0, 0, newWidth, newHeight); }
                        else if (ellipse) { ellipse->setPos(newPos.x() - edgeOffset.x(), startingPosition.y()); ellipse->setRect(0, 0, newWidth, newHeight); }
                        else { polygon->setPos(newPos.x() - edgeOffset.x(), startingPosition.y()); polygon->setSize(newWidth, newHeight); }
                        break;
                    case 3: // Top-Right Corner
                        xOffset = startingPosition.x() - newPos.x() + startingBounds.width();
                        newWidth = startingBounds.width() - xOffset + edgeOffset.x();
                        newHeight = startingBounds.height() + yOffset;

                        enforceRectSize(newPos, newWidth, newHeight);

                        if (rect) { rect->setPos(startingPosition.x(), newPos.y() - edgeOffset.y()); rect->setRect(0, 0, newWidth, newHeight); }
                        else if (ellipse) { ellipse->setPos(startingPosition.x(), newPos.y() - edgeOffset.y()); ellipse->setRect(0, 0, newWidth, newHeight); }
                        else { polygon->setPos(startingPosition.x(), newPos.y() - edgeOffset.y()); polygon->setSize(newWidth, newHeight); }
                        break;
                    case 4: // Bottom-Right Corner
                        xOffset = startingPosition.x() - newPos.x() + startingBounds.width();
                        yOffset = startingPosition.y() - newPos.y() + startingBounds.height();
                        newWidth = startingBounds.width() - xOffset + edgeOffset.x();
                        newHeight = startingBounds.height() - yOffset - edgeOffset.y();

                        enforceRectSize(newPos, newWidth, newHeight);

                        if (rect) rect->setRect(0, 0, newWidth, newHeight); else if (ellipse) ellipse->setRect(0, 0, newWidth, newHeight); else polygon->setSize(newWidth, newHeight);
                        break;

                    case 5: // Left Edge
                        newWidth = startingBounds.width() + xOffset;
                        newHeight = startingBounds.height();

                        enforceRectSize(newPos, newWidth, newHeight);

                        if (rect) { rect->setPos(newPos.x() - edgeOffset.x(), newPos.y() - edgeOffset.y() + yOffset); rect->setRect(0, 0, newWidth, newHeight); }
                        else if (ellipse) { ellipse->setPos(newPos.x() - edgeOffset.x(), newPos.y() - edgeOffset.y() + yOffset); ellipse->setRect(0, 0, newWidth, newHeight); }
                        else { polygon->setPos(newPos.x() - edgeOffset.x(), newPos.y() - edgeOffset.y() + yOffset); polygon->setSize(newWidth, newHeight); }
                        break;
                    case 6: // Right Edge
                        xOffset = startingPosition.x() - newPos.x() + startingBounds.width();
                        newWidth = startingBounds.width() - xOffset + edgeOffset.x();
                        newHeight = startingBounds.height();

                        enforceRectSize(newPos, newWidth, newHeight);

                        if (rect) { rect->setPos(startingPosition.x(), newPos.y() - edgeOffset.y() + yOffset); rect->setRect(0, 0, newWidth, newHeight); }
                        else if (ellipse) { ellipse->setPos(startingPosition.x(), newPos.y() - edgeOffset.y() + yOffset); ellipse->setRect(0, 0, newWidth, newHeight); }
                        else { polygon->setPos(startingPosition.x(), newPos.y() - edgeOffset.y() + yOffset); polygon->setSize(newWidth, newHeight); }
                        break;
                    case 7: // Top Edge
                        newWidth = startingBounds.width();
                        newHeight = startingBounds.height() + yOffset;

                        enforceRectSize(newPos, newWidth, newHeight);

                        if (rect) { rect->setPos(newPos.x() - edgeOffset.x() + xOffset, newPos.y() - edgeOffset.y()); rect->setRect(0, 0, newWidth, newHeight); }
                        else if (ellipse) { ellipse->setPos(newPos.x() - edgeOffset.x() + xOffset, newPos.y() - edgeOffset.y()); ellipse->setRect(0, 0, newWidth, newHeight); }
                        else { polygon->setPos(newPos.x() - edgeOffset.x() + xOffset, newPos.y() - edgeOffset.y()); polygon->setSize(newWidth, newHeight); }
                        break;
                    case 8: // Bottom Edge
                        newWidth = startingBounds.width();
                        newHeight = - yOffset + edgeOffset.y();

                        enforceRectSize(newPos, newWidth, newHeight);

                        if (rect) { rect->setPos(newPos.x() - edgeOffset.x() + xOffset, startingPosition.y()); rect->setRect(0, 0, newWidth, newHeight); }
                        else if (ellipse) { ellipse->setPos(newPos.x() - edgeOffset.x() + xOffset, startingPosition.y()); ellipse->setRect(0, 0, newWidth, newHeight); }
                        else { polygon->setPos(newPos.x() - edgeOffset.x() + xOffset, startingPosition.y()); polygon->setSize(newWidth, newHeight); }
                        break;
                }
                if (rect) rect->setText(rect->getText());
                else if (ellipse) ellipse->setText(ellipse->getText());
                else if (polygon) polygon->setText(polygon->getText());

                QPointF mouseScene = mapToScene(event->pos());
                if (currentItem && currentItem->sceneBoundingRect().contains(mouseScene)) {
                    updateSizeHelpers(currentItem);
                } else {
                    for (QGraphicsItem *helper : alignmentHelpers) {
                        if (scene) scene->removeItem(helper);
                        delete helper;
                    }
                    alignmentHelpers.clear();
                }
            }
        } else if (activeAction == None || activeAction == Move) {
            activeAction = Actions::Move;
            QPointF newPos = mouseScenePos - offset;
            QPointF delta = newPos - startingPosition;

            qreal minX = scene->sceneRect().left();
            qreal minY = scene->sceneRect().top();
            qreal maxX = scene->sceneRect().right();
            qreal maxY = scene->sceneRect().bottom();

            qreal newX = qMax(minX, qMin(newPos.x(), maxX - startingBounds.width()));
            qreal newY = qMax(minY, qMin(newPos.y(), maxY - startingBounds.height()));
            delta = QPointF(newX, newY) - startingPosition;

            QList<QPair<QGraphicsItem*, QPointF>> itemsToMove = m_dragItems.isEmpty() ? QList<QPair<QGraphicsItem*, QPointF>>{qMakePair(currentItem, startingPosition)} : m_dragItems;
            for (const auto &p : itemsToMove) {
                QPointF np = p.second + delta;
                QRectF br = getCorrectBoundingRect(p.first);
                qreal ix = qMax(minX, qMin(np.x(), maxX - br.width()));
                qreal iy = qMax(minY, qMin(np.y(), maxY - br.height()));
                p.first->setPos(ix, iy);
            }

            updateAlignmentHelpersForSelection();
        }
    }

    // Cursor and hover bounding box: always update when not in pick/apply style mode
    if (!m_pickStyleMode && !m_applyStyleMode && !m_copyMode) {
        QGraphicsItem *item = currentItem;
        if (!item) {
            item = scene->itemAt(mouseScenePos, QTransform());
            if (item && alignmentHelpers.contains(item))
                item = nullptr;
            if (item) {
                QGraphicsTextItem *textItem = dynamic_cast<QGraphicsTextItem*>(item);
                if (textItem && textItem->parentItem())
                    item = textItem->parentItem();
            }
        }
        updateHoverState(item, mouseScenePos);
    }

    QGraphicsView::mouseMoveEvent(event);
}

void LayoutEditorGraphicsView::mouseReleaseEvent(QMouseEvent *event) {
    if (!currentItem) {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }
    {
        int edgeOrCorner = isOnEdgeOrCorner(currentItem, mapToScene(event->pos()));

        Action *action;
        bool validAction = false;

        if (!edgeOrCorner){
            if (startingPosition != currentItem->pos()){
                action = new Action(Actions::Move, nullptr);
                action->moveItems = m_dragItems.isEmpty() ? QList<QPair<QGraphicsItem*, QPointF>>{qMakePair(currentItem, startingPosition)} : m_dragItems;
                action->moveDelta = currentItem->pos() - startingPosition;
                action->moveApplied = true;
                validAction = true;
            }

        }else{
            if (startingPosition != currentItem->pos() || startingBounds != getCorrectBoundingRect(currentItem)){
                action = new Action(Actions::Resize, currentItem, startingPosition, startingBounds);
                validAction = true;
            }

        }

        if (validAction){
            undoActions.push_back(action);
            redoActions.clear();
            action = nullptr;
            currentItem = nullptr;
            activeAction = Actions::None;
                    layoutEditor->markDirty();
                    layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
                    layoutEditor->updateMinimumSizeFromScene();
        } else {
            action = nullptr;
            currentItem = nullptr;
            activeAction = Actions::None;
            layoutEditor->markDirty();
            layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
        }
        updateAlignmentHelpersForSelection();
        // Restore default cursor after resize/move ends
        if (!m_pickStyleMode && !m_applyStyleMode && !m_copyMode)
            setCursor(Qt::ArrowCursor);
    }
}


void LayoutEditorGraphicsView::addRectAction(QGraphicsItem* item){
    Action *action;
    action = new Action(Actions::Add, item);
    undoActions.push_back(action);
    redoActions.clear();
    action = nullptr;
                    layoutEditor->markDirty();
                    layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
                    layoutEditor->updateMinimumSizeFromScene();
}

void LayoutEditorGraphicsView::clearUndoRedo(){
    for (Action *a : undoActions) delete a;
    undoActions.clear();
    for (Action *a : redoActions) delete a;
    redoActions.clear();
}

void LayoutEditorGraphicsView::clearAlignmentHelpers(){
    alignmentHelpers.clear();
    clearHoverBoundingBox();
}

void LayoutEditorGraphicsView::undoLastAction(){
    //make sure there is a last action
    if (undoActions.empty())return;

    //retrieve action from the vector
    Action *lastAction = undoActions.back();
    undoActions.pop_back();

    doAction(lastAction);

    redoActions.push_back(lastAction);
                    layoutEditor->markDirty();
                    layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
                    layoutEditor->updateMinimumSizeFromScene();
}

void LayoutEditorGraphicsView::redoLastAction(){
    //make sure there is a redo action
    if (redoActions.empty())return;

    //retrieve action from the vector
    Action *lastAction = redoActions.back();
    redoActions.pop_back();

    doAction(lastAction);

    undoActions.push_back(lastAction);
    layoutEditor->markDirty();
    layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
    layoutEditor->updateMinimumSizeFromScene();
}


void LayoutEditorGraphicsView::doAction(Action *action){
    ResizableRectItem *rect = action->item ? dynamic_cast<ResizableRectItem*>(action->item) : nullptr;
    ResizableEllipseItem *ellipse = action->item ? dynamic_cast<ResizableEllipseItem*>(action->item) : nullptr;
    ResizablePolygonItem *polygon = action->item ? dynamic_cast<ResizablePolygonItem*>(action->item) : nullptr;
    if (action->actionType == Move) {
        if (!action->moveItems.isEmpty()) {
            if (action->moveApplied) {
                for (const auto &p : action->moveItems)
                    p.first->setPos(p.first->pos() - action->moveDelta);
                action->moveApplied = false;
            } else {
                for (const auto &p : action->moveItems)
                    p.first->setPos(p.first->pos() + action->moveDelta);
                action->moveApplied = true;
            }
        } else if (action->item) {
            QPointF currentPos = action->item->pos();
            action->item->setPos(action->position);
            action->position = currentPos;
        }
    } else if (action->actionType == Resize && (rect || ellipse || polygon)) {
        QPointF currentPos = action->item->pos();
        QPointF newPos = QPointF(action->position.x(), action->position.y());
        if (rect) rect->setPos(newPos); else if (ellipse) ellipse->setPos(newPos); else polygon->setPos(newPos);
        action->position = currentPos;
        QRectF currentBounds = getCorrectBoundingRect(action->item);
        qreal w = action->size.width();
        qreal h = action->size.height();
        if (rect) rect->setRect(0, 0, w, h); else if (ellipse) ellipse->setRect(0, 0, w, h); else polygon->setSize(w, h);
        action->size = currentBounds;
    } else if (action->actionType == ChangeText && (rect || ellipse || polygon)) {
        if (rect) {
            rect->setText(action->oldText);
            rect->setShiftText(action->oldShiftText);
        } else if (ellipse) {
            ellipse->setText(action->oldText);
            ellipse->setShiftText(action->oldShiftText);
        } else {
            polygon->setText(action->oldText);
            polygon->setShiftText(action->oldShiftText);
        }
        action->oldText = action->newText;
        action->newText = rect ? rect->getText() : (ellipse ? ellipse->getText() : polygon->getText());
        action->oldShiftText = action->newShiftText;
        action->newShiftText = rect ? rect->getShiftText() : (ellipse ? ellipse->getShiftText() : polygon->getShiftText());
    } else if (action->actionType == ChangeKeyCodes && (rect || ellipse || polygon)) {
        if (rect) rect->setKeycodes(action->oldKeycodes); else if (ellipse) ellipse->setKeycodes(action->oldKeycodes); else polygon->setKeycodes(action->oldKeycodes);
        action->oldKeycodes = action->newKeycodes;
        action->newKeycodes = rect ? rect->getKeycodes() : (ellipse ? ellipse->getKeycodes() : polygon->getKeycodes());
    } else if (action->actionType == Add) {
        action->actionType = Actions::Remove;
        scene->removeItem(action->item);
    } else if (action->actionType == Remove) {
        action->actionType = Actions::Add;
        layoutEditor->addItemToScene(action->item);
    } else if (action->actionType == ChangeStyle) {
        if (action->styleApplied) {
            for (const auto &p : action->styleItems)
                setKeyStyleForItem(p.first, p.second);
            action->styleApplied = false;
        } else {
            for (const auto &p : action->styleItems)
                setKeyStyleForItem(p.first, action->styleNew);
            action->styleApplied = true;
        }
    }
}



QList<QGraphicsItem*> LayoutEditorGraphicsView::selectedKeyItems() const {
    QList<QGraphicsItem*> out;
    if (!scene) return out;
    for (QGraphicsItem *item : scene->selectedItems()) {
        if (dynamic_cast<ResizableRectItem*>(item) || dynamic_cast<ResizableEllipseItem*>(item) || dynamic_cast<ResizablePolygonItem*>(item))
            out.append(item);
    }
    return out;
}

void LayoutEditorGraphicsView::nudgeSelection(int dx, int dy) {
    QList<QGraphicsItem*> items = selectedKeyItems();
    if (items.isEmpty()) return;
    qreal minX = scene->sceneRect().left();
    qreal minY = scene->sceneRect().top();
    qreal maxX = scene->sceneRect().right();
    qreal maxY = scene->sceneRect().bottom();
    for (QGraphicsItem *item : items) {
        QRectF br = getCorrectBoundingRect(item);
        qreal newX = qMax(minX, qMin(item->pos().x() + dx, maxX - br.width()));
        qreal newY = qMax(minY, qMin(item->pos().y() + dy, maxY - br.height()));
        item->setPos(newX, newY);
    }
}

void LayoutEditorGraphicsView::commitArrowKeyMoveSegment() {
    if (m_arrowSegmentStarts.isEmpty() || (m_arrowSegmentDelta.x() == 0 && m_arrowSegmentDelta.y() == 0)) {
        m_arrowSegmentStarts.clear();
        m_arrowDirection = 0;
        return;
    }
    Action *action = new Action(Actions::Move, nullptr);
    action->moveItems = m_arrowSegmentStarts;
    action->moveDelta = m_arrowSegmentDelta;
    action->moveApplied = true;
    undoActions.push_back(action);
    redoActions.clear();
    layoutEditor->markDirty();
    layoutEditor->updateButtons(!undoActions.empty(), !redoActions.empty());
    layoutEditor->updateMinimumSizeFromScene();
    m_arrowSegmentStarts.clear();
    m_arrowSegmentDelta = QPointF(0, 0);
    m_arrowDirection = 0;
}

void LayoutEditorGraphicsView::setPickStyleMode(bool on) {
    m_pickStyleMode = on;
    m_applyStyleMode = on ? false : m_applyStyleMode;
    m_copyMode = on ? false : m_copyMode;
    if (on)
        setCursor(Qt::CrossCursor);
    else if (!m_applyStyleMode && !m_copyMode)
        setCursor(Qt::ArrowCursor);
}

void LayoutEditorGraphicsView::setApplyStyleMode(bool on) {
    m_applyStyleMode = on;
    m_pickStyleMode = on ? false : m_pickStyleMode;
    m_copyMode = on ? false : m_copyMode;
    if (on)
        setCursor(Qt::CrossCursor);
    else if (!m_pickStyleMode && !m_copyMode)
        setCursor(Qt::ArrowCursor);
}

void LayoutEditorGraphicsView::setCopyMode(bool on) {
    m_copyMode = on;
    m_pickStyleMode = on ? false : m_pickStyleMode;
    m_applyStyleMode = on ? false : m_applyStyleMode;
    if (on)
        setCursor(Qt::CrossCursor);
    else if (!m_pickStyleMode && !m_applyStyleMode)
        setCursor(Qt::ArrowCursor);
}

void LayoutEditorGraphicsView::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape && (m_pickStyleMode || m_applyStyleMode || m_copyMode)) {
        m_pickStyleMode = false;
        m_applyStyleMode = false;
        m_copyMode = false;
        setCursor(Qt::ArrowCursor);
        emit applyStyleModeExited();
        event->accept();
        return;
    }
    int key = event->key();
    int direction = 0;
    int dx = 0, dy = 0;
    int step = (event->modifiers() & Qt::ControlModifier) ? 10 : 1;
    if (key == Qt::Key_Left) { direction = 1; dx = -step; }
    else if (key == Qt::Key_Right) { direction = 2; dx = step; }
    else if (key == Qt::Key_Up) { direction = 3; dy = -step; }
    else if (key == Qt::Key_Down) { direction = 4; dy = step; }
    if (direction != 0) {
        QList<QGraphicsItem*> items = selectedKeyItems();
        if (!items.isEmpty()) {
            event->accept();
            m_arrowCommitTimer->stop();
            if (direction != m_arrowDirection) {
                commitArrowKeyMoveSegment();
                m_arrowDirection = direction;
                m_arrowSegmentDelta = QPointF(0, 0);
                m_arrowSegmentStarts.clear();
                for (QGraphicsItem *item : items)
                    m_arrowSegmentStarts.append(qMakePair(item, item->pos()));
            }
            m_arrowSegmentDelta += QPointF(dx, dy);
            nudgeSelection(dx, dy);
            updateAlignmentHelpersForSelection();
            return;
        }
    }
    QGraphicsView::keyPressEvent(event);
}

void LayoutEditorGraphicsView::keyReleaseEvent(QKeyEvent *event) {
    int key = event->key();
    if (key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Up || key == Qt::Key_Down) {
        m_arrowCommitTimer->start(250);
    }
    QGraphicsView::keyReleaseEvent(event);
}

void LayoutEditorGraphicsView::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    if (scene) {
        QRectF newRect = QRectF(QPointF(0, 0), QSizeF(this->viewport()->size()));
        scene->setSceneRect(newRect);
    }
}

void LayoutEditorGraphicsView::leaveEvent(QEvent *event) {
    if (currentItem) {
        for (QGraphicsItem *helper : alignmentHelpers) {
            if (scene) scene->removeItem(helper);
            delete helper;
        }
        alignmentHelpers.clear();
        currentItem = nullptr;
        activeAction = Actions::None;
        if (!m_pickStyleMode && !m_applyStyleMode && !m_copyMode)
            setCursor(Qt::ArrowCursor);
    }
    clearHoverBoundingBox();
    QGraphicsView::leaveEvent(event);
}

void LayoutEditorGraphicsView::focusOutEvent(QFocusEvent *event) {
    if (currentItem) {
        for (QGraphicsItem *helper : alignmentHelpers) {
            if (scene) scene->removeItem(helper);
            delete helper;
        }
        alignmentHelpers.clear();
        currentItem = nullptr;
        activeAction = Actions::None;
        if (!m_pickStyleMode && !m_applyStyleMode && !m_copyMode)
            setCursor(Qt::ArrowCursor);
    }
    clearHoverBoundingBox();
    QGraphicsView::focusOutEvent(event);
}

int LayoutEditorGraphicsView::isOnEdgeOrCorner(QGraphicsItem *item, const QPointF &mousePos) {
    if (!item) {
        return 0;
    }
    if (!dynamic_cast<QGraphicsRectItem*>(item) && !dynamic_cast<QGraphicsEllipseItem*>(item) && !dynamic_cast<QGraphicsPolygonItem*>(item)) {
        return 0;
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


static bool rangesOverlapStatic(qreal start1, qreal end1, qreal start2, qreal end2) {
    qreal middle1 = (start1 + end1) / 2;
    return middle1 >= start2 && middle1 <= end2;
}

static void addAlignmentHelpersForItem(QGraphicsScene *scene, QList<QGraphicsItem*> &alignmentHelpers,
    QGraphicsItem *movingItem, const QSet<QGraphicsItem*> &excludeItems) {
    qreal closestLeft = FLT_MAX;
    qreal closestRight = FLT_MAX;
    qreal closestTop = FLT_MAX;
    qreal closestBottom = FLT_MAX;

    QRectF movingRect = movingItem->boundingRect().translated(movingItem->pos());

    for (QGraphicsItem* item : scene->items()) {
        if (excludeItems.contains(item) || item->type() == QGraphicsLineItem::Type || item->type() == QGraphicsTextItem::Type) continue;

        QRectF itemRect = item->boundingRect().translated(item->pos());

        if (rangesOverlapStatic(movingRect.top(), movingRect.bottom(), itemRect.top(), itemRect.bottom())) {
            if (itemRect.right() < movingRect.left() && movingRect.left() - itemRect.right() < closestLeft)
                closestLeft = movingRect.left() - itemRect.right();
            if (itemRect.left() > movingRect.right() && itemRect.left() - movingRect.right() < closestRight)
                closestRight = itemRect.left() - movingRect.right();
        }
        if (rangesOverlapStatic(movingRect.left(), movingRect.right(), itemRect.left(), itemRect.right())) {
            if (itemRect.bottom() < movingRect.top() && movingRect.top() - itemRect.bottom() < closestTop)
                closestTop = movingRect.top() - itemRect.bottom();
            if (itemRect.top() > movingRect.bottom() && itemRect.top() - movingRect.bottom() < closestBottom)
                closestBottom = itemRect.top() - movingRect.bottom();
        }
    }

    QRectF sceneRect = scene->sceneRect();
    if (movingRect.left() - sceneRect.left() < closestLeft) closestLeft = movingRect.left() - sceneRect.left();
    if (sceneRect.right() - movingRect.right() < closestRight) closestRight = sceneRect.right() - movingRect.right();
    if (movingRect.top() - sceneRect.top() < closestTop) closestTop = movingRect.top() - sceneRect.top();
    if (sceneRect.bottom() - movingRect.bottom() < closestBottom) closestBottom = sceneRect.bottom() - movingRect.bottom();

    auto drawLine = [&](const QRectF &r, qreal d, Qt::Orientation o, bool start) {
        if (d == FLT_MAX) return;
        QPointF sp, ep, tp;
        if (o == Qt::Horizontal) {
            qreal y = r.top() + r.height() / 2;
            sp.setY(y); ep.setY(y);
            if (start) { sp.setX(r.left()); ep.setX(r.left() - d); tp = (sp + ep) / 2; if (ep.x() < 0) tp.setX(r.left() - d / 2); }
            else { sp.setX(r.right()); ep.setX(r.right() + d); tp = (sp + ep) / 2; if (ep.x() > scene->width() - 27) tp.setX(r.right() + d / 2 - 27); }
        } else {
            qreal x = r.left() + r.width() / 2;
            sp.setX(x); ep.setX(x);
            if (start) { sp.setY(r.top()); ep.setY(r.top() - d); tp = (sp + ep) / 2; if (ep.y() < 0) tp.setY(r.top() - d / 2); }
            else { sp.setY(r.bottom()); ep.setY(r.bottom() + d); tp = (sp + ep) / 2; if (ep.y() > scene->height() - 20) tp.setY(r.bottom() + d / 2 - 20); }
        }
        alignmentHelpers.append(scene->addLine(QLineF(sp, ep), QPen(Qt::red, 1, Qt::DashLine)));
        QGraphicsTextItem *ti = scene->addText(QString::number(qRound(d < 0 ? 0 : d)) + " px");
        ti->setDefaultTextColor(Qt::red);
        ti->setPos(tp);
        alignmentHelpers.append(ti);
    };
    drawLine(movingRect, closestLeft, Qt::Horizontal, true);
    drawLine(movingRect, closestRight, Qt::Horizontal, false);
    drawLine(movingRect, closestTop, Qt::Vertical, true);
    drawLine(movingRect, closestBottom, Qt::Vertical, false);
}

void LayoutEditorGraphicsView::updateAlignmentHelpers(QGraphicsItem* movingItem) {
    for (auto* item : alignmentHelpers) { scene->removeItem(item); delete item; }
    alignmentHelpers.clear();
    QSet<QGraphicsItem*> ex;
    ex.insert(movingItem);
    addAlignmentHelpersForItem(scene, alignmentHelpers, movingItem, ex);
}

void LayoutEditorGraphicsView::updateAlignmentHelpersForSelection() {
    for (auto* item : alignmentHelpers) { scene->removeItem(item); delete item; }
    alignmentHelpers.clear();
    QList<QGraphicsItem*> items = selectedKeyItems();
    if (items.isEmpty()) return;
    QSet<QGraphicsItem*> exclude;
    for (QGraphicsItem *i : items) exclude.insert(i);
    for (QGraphicsItem *item : items)
        addAlignmentHelpersForItem(scene, alignmentHelpers, item, exclude);
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


void LayoutEditorGraphicsView::clearHoverBoundingBox() {
    if (!m_hoverBoundingBoxRect.isNull()) {
        m_hoverBoundingBoxRect = QRectF();
        viewport()->update();
    }
}

void LayoutEditorGraphicsView::updateHoverState(QGraphicsItem *itemUnderCursor, const QPointF &mouseScenePos) {
    bool isKeyShape = itemUnderCursor && (dynamic_cast<ResizableRectItem*>(itemUnderCursor) ||
        dynamic_cast<ResizableEllipseItem*>(itemUnderCursor) ||
        dynamic_cast<ResizablePolygonItem*>(itemUnderCursor));

    if (!isKeyShape) {
        clearHoverBoundingBox();
        setCursor(Qt::ArrowCursor);
        return;
    }

    int edgeOrCorner = isOnEdgeOrCorner(itemUnderCursor, mouseScenePos);
    switch (edgeOrCorner) {
        case 1: case 4: setCursor(Qt::SizeFDiagCursor); break;
        case 2: case 3: setCursor(Qt::SizeBDiagCursor); break;
        case 5: case 6: setCursor(Qt::SizeHorCursor); break;
        case 7: case 8: setCursor(Qt::SizeVerCursor); break;
        default: setCursor(Qt::ArrowCursor); break;
    }

    // Show dotted bounding box on hover when not dragging (painted in drawForeground, not a scene item)
    if (!currentItem) {
        QRectF sceneRect = itemUnderCursor->boundingRect().translated(itemUnderCursor->scenePos());
        if (m_hoverBoundingBoxRect != sceneRect) {
            m_hoverBoundingBoxRect = sceneRect;
            viewport()->update();
        }
    } else {
        clearHoverBoundingBox();
    }
}

void LayoutEditorGraphicsView::drawForeground(QPainter *painter, const QRectF &rect) {
    QGraphicsView::drawForeground(painter, rect);
    if (!m_hoverBoundingBoxRect.isNull()) {
        painter->save();
        painter->setPen(QPen(QColor(128, 128, 128), 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(m_hoverBoundingBoxRect);
        painter->restore();
    }
}

void LayoutEditorGraphicsView::updateSizeHelpers(QGraphicsItem* item) {
    // Ensure the item is valid and the type expected
    if (!item || !(item->type() == QGraphicsRectItem::Type || item->type() == QGraphicsEllipseItem::Type || item->type() == QGraphicsPolygonItem::Type))
        return;

    for (QGraphicsItem *helper : alignmentHelpers) {
        if (scene) scene->removeItem(helper);
        delete helper;
    }
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




