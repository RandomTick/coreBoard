#ifndef LAYOUTEDITOR_H
#define LAYOUTEDITOR_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPolygonF>
#include <QPushButton>
#include <QVBoxLayout>
#include <QJsonArray>
#include "layouteditorgraphicsview.h"
#include "resizablerectitem.h"
#include "resizableellipseitem.h"
#include "resizablepolygonitem.h"

class LayoutSettings;

class LayoutEditor : public QWidget
{
    Q_OBJECT

public:
    explicit LayoutEditor(QWidget *parent = nullptr);
    void updateButtons(bool undoCommandsExist, bool redoCommandsExist);
    void loadLayout(const QString &fileName);
    void setLayoutSettings(LayoutSettings *settings);
    void updateMinimumSizeFromScene();

    bool isDirty() const { return m_dirty; }
    QString currentLayoutPath() const { return _currentLayoutPath; }
    void markDirty();

signals:
    void layoutLoaded(const QString &path);

private:
    LayoutEditorGraphicsView *view;
    QGraphicsScene *scene;
    QPushButton *openButton;
    QPushButton *newButton;
    QPushButton *undoButton;
    QPushButton *redoButton;
    QPushButton *addButton;
    QPushButton *saveButton;
    QPushButton *saveAsButton;
    QString _currentLayoutPath;
    bool m_dirty = false;
    LayoutSettings *m_layoutSettings = nullptr;
    void addShape();
    bool writeLayoutToFile(const QString &fileName);
public slots:
    ResizableRectItem * addRectangle(const QString &text, qreal h, qreal w, qreal x, qreal y, const std::list<int> keyCodes = {});
    ResizableEllipseItem * addEllipse(const QString &text, qreal h, qreal w, qreal x, qreal y, const std::list<int> keyCodes = {});
    ResizablePolygonItem * addPolygon(const QPolygonF &templatePolygon, const QString &text, qreal x, qreal y, qreal w, qreal h, const std::list<int> keyCodes = {});
    void addItemToScene(QGraphicsItem *item);
    void loadLayoutButton();
    void newLayout();
    void saveLayout();
    void saveLayoutAs();
    void createKey(const QJsonObject &keyData);
    void updateLanguage();

};

#endif // LAYOUTEDITOR_H
