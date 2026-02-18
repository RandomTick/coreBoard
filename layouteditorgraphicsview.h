#ifndef LAYOUTEDITORGRAPHICSVIEW_H
#define LAYOUTEDITORGRAPHICSVIEW_H

#include <QGraphicsView>


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
        ChangeKeyCodes
    };
    class Action{
    public:
        Actions actionType;
        QGraphicsItem *item;
        QPointF position;
        QRectF size;
        QString oldText;
        QString newText;
        std::list<int> oldKeycodes;
        std::list<int> newKeycodes;

        Action(Actions actionType, QGraphicsItem *item)
            : actionType(actionType), item(item){}

        Action(Actions actionType, QGraphicsItem *item, QPointF position)
            : actionType(actionType), item(item), position(position){}

        Action(Actions actionType, QGraphicsItem *item, QPointF position, QRectF size)
            : actionType(actionType), item(item), position(position), size(size) {}

        Action(Actions actionType, QGraphicsItem *item, QString oldText, QString newText)
            : actionType(actionType), item(item), oldText(oldText), newText(newText){}

        Action(Actions actionType, QGraphicsItem *item, std::list<int> oldKeycodes, std::list<int> newKeycodes)
            : actionType(actionType), item(item), oldKeycodes(oldKeycodes), newKeycodes(newKeycodes){}
    };
    void doAction(Action action);
    void undoLastAction();
    void redoLastAction();
    void addRectAction(QGraphicsItem* item);
    void clearUndoRedo();
    void clearAlignmentHelpers();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;




private:
    QGraphicsScene *scene;
    QGraphicsItem *currentItem;
    QPointF startingPosition;
    QPointF offset;
    QRectF startingBounds;
    std::vector<Action*> undoActions;
    std::vector<Action*> redoActions;
    void doAction(Action* action);
    int isOnEdgeOrCorner(QGraphicsItem *item, const QPointF &mousePos);
    void enforceRectSize(QPointF &newPos, qreal &newWidth, qreal &newHeight);
    void centerText(QGraphicsRectItem *rect);
    void updateAlignmentHelpers(QGraphicsItem* item);
    void drawAlignmentLine(const QRectF& movingRect, qreal distance, Qt::Orientation orientation, bool isStartSide);
    bool rangesOverlap(qreal start1, qreal end1, qreal start2, qreal end2);
    void updateSizeHelpers(QGraphicsItem* item);
    QRectF getCorrectBoundingRect(QGraphicsItem *item);
    QPointF edgeOffset;    
    Actions activeAction;



};

#endif // LAYOUTEDITORGRAPHICSVIEW_H
