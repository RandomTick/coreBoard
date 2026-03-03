#ifndef LAYOUTEDITOR_H
#define LAYOUTEDITOR_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPolygonF>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QJsonArray>
#include "layouteditorgraphicsview.h"
#include "resizablerectitem.h"
#include "resizableellipseitem.h"
#include "resizablepolygonitem.h"
#include "resizablepathitem.h"
#include "mousespeedindicatoritem.h"
#include "angularvieweritem.h"
#include "controlleritem.h"
#include "labelitem.h"
#include "keystyle.h"

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
    QPushButton *copyButton;
    QPushButton *pasteButton;
    QPushButton *pickStyleButton;
    QPushButton *applyStyleButton;
    QPushButton *saveButton;
    QPushButton *saveAsButton;
    QMenu *m_addShapeMenu = nullptr;
    QMenu *m_customShapeMenu = nullptr;
    QAction *m_actRect = nullptr;
    QAction *m_actCircle = nullptr;
    QAction *m_actStar = nullptr;
    QAction *m_actDiamond = nullptr;
    QAction *m_actHexagon = nullptr;
    QAction *m_actTriangle = nullptr;
    QAction *m_actMouseSpeedIndicator = nullptr;
    QMenu *m_angularViewerMenu = nullptr;
    QAction *m_actLeftStick = nullptr;
    QAction *m_actRightStick = nullptr;
    QAction *m_actController = nullptr;
    QAction *m_actLabel = nullptr;
    QString _currentLayoutPath;
    bool m_dirty = false;
    LayoutSettings *m_layoutSettings = nullptr;
    void addShape();
    void onPickStyleToggled(bool checked);
    void onApplyStyleClicked(bool checked);
    void onCopyToggled(bool checked);
    bool writeLayoutToFile(const QString &fileName);
public slots:
    ResizableRectItem * addRectangle(const QString &text, qreal h, qreal w, qreal x, qreal y, const std::list<int> keyCodes = {}, const KeyStyle &keyStyle = KeyStyle());
    ResizableEllipseItem * addEllipse(const QString &text, qreal h, qreal w, qreal x, qreal y, const std::list<int> keyCodes = {}, const KeyStyle &keyStyle = KeyStyle());
    ResizablePolygonItem * addPolygon(const QPolygonF &templatePolygon, const QString &text, qreal x, qreal y, qreal w, qreal h, const std::list<int> keyCodes = {}, const KeyStyle &keyStyle = KeyStyle());
    ResizablePathItem * addPathItem(const QPolygonF &outer, const QList<QPolygonF> &holes, const QString &text, qreal x, qreal y, qreal w, qreal h, const std::list<int> keyCodes = {}, const KeyStyle &keyStyle = KeyStyle(), const QList<bool> &holeIsCircular = QList<bool>());
    void addItemToScene(QGraphicsItem *item);
    void loadLayoutButton();
    void newLayout();
    void saveLayout();
    void saveLayoutAs();
    QGraphicsItem* createKey(const QJsonObject &keyData);
    MouseSpeedIndicatorItem* createMouseSpeedIndicator(const QJsonObject &keyData);
    MouseSpeedIndicatorItem* addMouseSpeedIndicator(qreal centerX, qreal centerY, qreal radius, const QString &label = QString());
    AngularViewerItem* createAngularViewer(const QJsonObject &keyData);
    AngularViewerItem* addAngularViewer(AngularViewerSubType subType, qreal x, qreal y, qreal w, qreal h, const QString &label = QString());
    ControllerItem* createController(const QJsonObject &keyData);
    ControllerItem* addController(qreal x, qreal y, qreal w, qreal h);
    LabelItem* createLabel(const QJsonObject &keyData);
    LabelItem* addLabel(const QString &text, qreal x, qreal y);
    void updateLanguage();
    void copyKeyFromItem(QGraphicsItem *item);
    void pasteKey();

};

#endif // LAYOUTEDITOR_H
