#ifndef LAYOUTEDITOR_H
#define LAYOUTEDITOR_H

#include <QGraphicsView>
#include <QGraphicsScene>

class LayoutEditor : public QGraphicsView
{
    Q_OBJECT

public:    
    explicit LayoutEditor(QWidget *parent = nullptr);
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QGraphicsScene *scene;
public slots:
    void addRectangle();

};

#endif // LAYOUTEDITOR_H
