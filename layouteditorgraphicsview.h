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
        Add,
        Remove,
        Move,
        Resize,
        ChangeText,
        ChangeKey
    };
    class Action{
    private:
        Actions actionType;
        QGraphicsItem *item;
        QPointF position;
        QRectF *size;

    public:
        Action(Actions actionType, QGraphicsItem *item, QPointF position, QRectF *size)
            : actionType(actionType), item(item), position(position), size(size) {}

    };

    void doAction(Action action);
    std::vector<Action*> undoActions;
    std::vector<Action*> redoActions;


protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QGraphicsScene *scene;
    QGraphicsItem *currentItem;
    QPointF offset;
};

#endif // LAYOUTEDITORGRAPHICSVIEW_H
