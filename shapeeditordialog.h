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

signals:
    void requestItemReplacement(QGraphicsItem *oldItem, int newType, const QPolygonF &outer,
                                const QList<QPolygonF> &holes, const QPointF &textPosLocal,
                                const QPointF &itemPos, qreal w, qreal h);

private:
    void setupCanvas();
    void applyChanges();
    void rebuildVertexDots();
    void refreshDisplay();

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
    QList<QGraphicsItem*> m_vertexDots;
};

#endif // SHAPEEDITORDIALOG_H
