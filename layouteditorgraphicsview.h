#ifndef LAYOUTEDITORGRAPHICSVIEW_H
#define LAYOUTEDITORGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QList>
#include <QPair>
#include <QPolygonF>
#include <QTimer>
#include <QKeyEvent>
#include "keystyle.h"

struct EditShapeState {
    int type = 0;
    QRectF rect;
    QPolygonF polygon;
    QList<QPolygonF> holes;
    QPointF textPos;
};

class LayoutEditorGraphicsView : public QGraphicsView {
    Q_OBJECT
public:
    explicit LayoutEditorGraphicsView(QWidget *parent = nullptr);
    void setSceneAndStore(QGraphicsScene *externScene);
    enum Actions
    {
        None,
        Add,
        Remove,
        Move,
        Resize,
        ChangeText,
        ChangeKeyCodes,
        ChangeStyle,
        EditShape
    };
    class Action{
    public:
        Actions actionType;
        QGraphicsItem *item;
        QPointF position;
        QRectF size;
        QString oldText;
        QString newText;
        QString oldShiftText;
        QString newShiftText;
        std::list<int> oldKeycodes;
        std::list<int> newKeycodes;
        // Multi-item move: list of (item, start position); delta applied for undo/redo
        QList<QPair<QGraphicsItem*, QPointF>> moveItems;
        QPointF moveDelta;
        bool moveApplied = true;  // true = items have been moved by +delta
        // ChangeStyle: per-item old style, single new style; styleApplied true = items have newStyle
        QList<QPair<QGraphicsItem*, KeyStyle>> styleItems;
        KeyStyle styleNew;
        bool styleApplied = true;
        EditShapeState editShapeOld;
        EditShapeState editShapeNew;
        bool editShapeApplied = true;
        QGraphicsItem *editShapeOldItem = nullptr;  // when replacement occurred, old item for undo

        Action(Actions actionType, QGraphicsItem *item)
            : actionType(actionType), item(item){}

        Action(Actions actionType, QGraphicsItem *item, QPointF position)
            : actionType(actionType), item(item), position(position){}

        Action(Actions actionType, QGraphicsItem *item, QPointF position, QRectF size)
            : actionType(actionType), item(item), position(position), size(size) {}

        Action(Actions actionType, QGraphicsItem *item, QString oldText, QString newText)
            : actionType(actionType), item(item), oldText(oldText), newText(newText){}
        Action(Actions actionType, QGraphicsItem *item, QString oldText, QString newText, QString oldShiftText, QString newShiftText)
            : actionType(actionType), item(item), oldText(oldText), newText(newText), oldShiftText(oldShiftText), newShiftText(newShiftText){}

        Action(Actions actionType, QGraphicsItem *item, std::list<int> oldKeycodes, std::list<int> newKeycodes)
            : actionType(actionType), item(item), oldKeycodes(oldKeycodes), newKeycodes(newKeycodes){}
    };
    void doAction(Action *action);
    void undoLastAction();
    void redoLastAction();
    void addRectAction(QGraphicsItem* item);
    void clearUndoRedo();
    void setPickStyleMode(bool on);
    void setApplyStyleMode(bool on);
    void setCopyMode(bool on);
    bool hasPickedStyle() const { return m_hasPickedStyle; }
    void clearAlignmentHelpers();
    QList<QGraphicsItem*> selectedKeyItems() const;

signals:
    void stylePicked();
    void applyStyleModeExited();
    void keyCopied();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;




private:
    QGraphicsScene *scene;
    QGraphicsItem *currentItem;
    QPointF startingPosition;
    QPointF offset;
    QRectF startingBounds;
    std::vector<Action*> undoActions;
    std::vector<Action*> redoActions;
    int isOnEdgeOrCorner(QGraphicsItem *item, const QPointF &mousePos);
    void enforceRectSize(QPointF &newPos, qreal &newWidth, qreal &newHeight);
    void centerText(QGraphicsRectItem *rect);
    void updateAlignmentHelpers(QGraphicsItem* item);
    void updateAlignmentHelpersForSelection();
    void drawAlignmentLine(const QRectF& movingRect, qreal distance, Qt::Orientation orientation, bool isStartSide);
    bool rangesOverlap(qreal start1, qreal end1, qreal start2, qreal end2);
    void updateSizeHelpers(QGraphicsItem* item);
    void updateHoverState(QGraphicsItem *itemUnderCursor, const QPointF &mouseScenePos);
    void clearHoverBoundingBox();
    QRectF getCorrectBoundingRect(QGraphicsItem *item);
    QPointF edgeOffset;
    Actions activeAction;

    void commitArrowKeyMoveSegment();
    void nudgeSelection(int dx, int dy);

    int m_arrowDirection = 0;
    QPointF m_arrowSegmentDelta;
    bool m_pickStyleMode = false;
    bool m_applyStyleMode = false;
    bool m_copyMode = false;
    bool m_hasPickedStyle = false;
    KeyStyle m_pickedStyle;
    QList<QPair<QGraphicsItem*, QPointF>> m_arrowSegmentStarts;
    QTimer *m_arrowCommitTimer = nullptr;
    QList<QPair<QGraphicsItem*, QPointF>> m_dragItems;
    QRectF m_hoverBoundingBoxRect;
};

#endif // LAYOUTEDITORGRAPHICSVIEW_H
