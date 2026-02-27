#ifndef SHAPEEDITORDIALOG_H
#define SHAPEEDITORDIALOG_H

#include <QDialog>
#include <QGraphicsItem>
#include <QPointF>
#include <QPolygonF>
#include <QList>

class QGraphicsScene;
class QGraphicsView;
class QGraphicsPolygonItem;
class QGraphicsPathItem;
class QDoubleSpinBox;
class QLabel;
class QCheckBox;

class ShapeEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShapeEditorDialog(QWidget *parent, QGraphicsItem *shapeItem);
    ~ShapeEditorDialog();

    void updatePolygonFromVertices(int holeIndex, int movedIndex, const QPointF &movedPos);
    void moveHoleBy(int holeIndex, const QPointF &delta);
    void addPointAfter(int holeIndex, int vertexIndex);
    void deletePoint(int holeIndex, int vertexIndex);
    void addHole();
    void addCircularHole();
    void deleteHole(int holeIndex);
    void setSelectedVertex(int holeIndex, int vertexIndex);
    void updateOffsetLabel();  // public so TextAnchorDot can call when anchor is moved
    void onAnchorMovedByUser();  // public so TextAnchorDot can call when user drags the anchor

signals:
    void requestItemReplacement(QGraphicsItem *oldItem, int newType, const QPolygonF &outer,
                                const QList<QPolygonF> &holes, const QPointF &textPosLocal,
                                const QPointF &itemPos, qreal w, qreal h);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupCanvas();
    void applyChanges();
    void rebuildVertexDots();
    void refreshDisplay();
    void applyRotationToShape(qreal degrees);
    void moveSelectedVertexBy(int dx, int dy);
    QPointF shapeCenter() const;
    void updateAnchorToCenterIfChecked();

    QGraphicsItem *m_shapeItem = nullptr;
    QGraphicsScene *m_scene = nullptr;
    QGraphicsView *m_view = nullptr;
    QPointF m_textAnchorLocal;
    enum ShapeType { Rect, Ellipse, Polygon, Path };
    ShapeType m_shapeType = Rect;
    QRectF m_shapeRect;
    QPolygonF m_shapePolygon;
    QList<QPolygonF> m_shapeHoles;
    QList<bool> m_holeIsCircular;  // parallel to m_shapeHoles
    QGraphicsItem *m_textAnchorDot = nullptr;
    QGraphicsPolygonItem *m_polygonDisplay = nullptr;
    QGraphicsPathItem *m_pathDisplay = nullptr;
    QGraphicsPathItem *m_pathOutline = nullptr;  // explicit outline so all edges draw after hole delete
    QList<QGraphicsItem*> m_vertexDots;
    qreal m_rotationDegrees = 0;
    QDoubleSpinBox *m_degreeSpinBox = nullptr;
    QLabel *m_offsetLabel = nullptr;
    QCheckBox *m_centerTextCheckBox = nullptr;
    bool m_anchorSetByCode = false;
    int m_selectedHoleIndex = -2;
    int m_selectedVertexIndex = -1;
};

#endif // SHAPEEDITORDIALOG_H
