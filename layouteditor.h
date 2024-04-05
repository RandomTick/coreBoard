#ifndef LAYOUTEDITOR_H
#define LAYOUTEDITOR_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPushButton>
#include <QVBoxLayout>
#include <QJsonArray>
#include "layouteditorgraphicsview.h"

class LayoutEditor : public QWidget
{
    Q_OBJECT

public:    
    explicit LayoutEditor(QWidget *parent = nullptr);
    void updateButtons(bool undoCommandsExist, bool redoCommandsExist);
    void loadLayout(const QString &fileName);

private:
    LayoutEditorGraphicsView *view;
    QGraphicsScene *scene;
    QPushButton *openButton;
    QPushButton *undoButton;
    QPushButton *redoButton;
    QPushButton *addButton;
public slots:
    void addRectangle(const QString &text, qreal h, qreal w, qreal x, qreal y);
    void loadLayoutButton();
    void createKey(const QJsonObject &keyData);
    void updateLanguage();

};

#endif // LAYOUTEDITOR_H
