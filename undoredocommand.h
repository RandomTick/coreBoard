#ifndef UNDOREDOCOMMAND_H
#define UNDOREDOCOMMAND_H


class UndoRedoCommand
{
public:
    UndoRedoCommand();
    virtual void undo() = 0;
    virtual void redo() = 0;

};



#endif // UNDOREDOCOMMAND_H


