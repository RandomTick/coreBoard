#ifndef LAYOUTEDITOR_H
#define LAYOUTEDITOR_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPushButton>
#include <QVBoxLayout>
#include "layouteditorgraphicsview.h"

class LayoutEditor : public QWidget
{
    Q_OBJECT

public:    
    explicit LayoutEditor(QWidget *parent = nullptr);
    void updateButtons(bool undoCommandsExist, bool redoCommandsExist);

private:
    LayoutEditorGraphicsView *view;
    QGraphicsScene *scene;
    QPushButton *undoButton;
    QPushButton *redoButton;
    QPushButton *addButton;
public slots:
    void addRectangle();
    void updateLanguage();
};

#endif // LAYOUTEDITOR_H
