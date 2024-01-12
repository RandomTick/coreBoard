#include "undoredocommand.h"


#include <QGraphicsItem>



class MoveCommand : public UndoRedoCommand {
public:
    MoveCommand(QGraphicsItem *item, const QPointF &oldPos, const QPointF &newPos)
        : item(item), oldPos(oldPos), newPos(newPos) {}

    void undo() override {
        item->setPos(oldPos);
    }

    void redo() override {
        item->setPos(newPos);
    }


private:
    QGraphicsItem *item;
    QPointF oldPos;
    QPointF newPos;
};
