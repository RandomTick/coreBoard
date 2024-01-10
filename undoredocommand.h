#ifndef UNDOREDOCOMMAND_H
#define UNDOREDOCOMMAND_H

#include <QGraphicsItem>

class UndoRedoCommand
{
public:
    UndoRedoCommand();
    virtual void undo() = 0;
    virtual void redo() = 0;
};

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



#endif // UNDOREDOCOMMAND_H


