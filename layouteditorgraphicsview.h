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
        ChangeKey
    };
    class Action{
    public:
        Actions actionType;
        QGraphicsItem *item;
        QPointF position;
        QRectF size;

        Action(Actions actionType, QGraphicsItem *item, QPointF position, QRectF size)
            : actionType(actionType), item(item), position(position), size(size) {}

    };
    void doAction(Action action);
    void undoLastAction();
    void redoLastAction();

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
    QRectF getCorrectBoundingRect(QGraphicsItem *item);
    QPointF edgeOffset;    
    Actions activeAction;



};

#endif // LAYOUTEDITORGRAPHICSVIEW_H
