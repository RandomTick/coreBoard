#ifndef LAYOUTEDITOR_H
#define LAYOUTEDITOR_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPushButton>
#include <QVBoxLayout>

class LayoutEditor : public QWidget
{
    Q_OBJECT

public:    
    explicit LayoutEditor(QWidget *parent = nullptr);


private:
    QGraphicsView *view;
    QGraphicsScene *scene;
    QPushButton *undoButton;
    QPushButton *redoButton;
public slots:
    void addRectangle();

};

#endif // LAYOUTEDITOR_H
