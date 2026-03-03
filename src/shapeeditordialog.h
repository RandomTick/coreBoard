#ifndef SHAPEEDITORDIALOG_H
#define SHAPEEDITORDIALOG_H

#include <QDialog>
#include <QGraphicsItem>
#include <QPointF>
#include <QPolygonF>
#include <QList>
#include <QRectF>

class QGraphicsScene;
class QGraphicsView;
class QGraphicsPolygonItem;
class QGraphicsPathItem;
class QDoubleSpinBox;
class QLabel;
class QCheckBox;
class QPushButton;
class QTimer;

struct ShapeEditorState {
    int shapeType = 0;  // 0=Rect, 1=Ellipse, 2=Polygon, 3=Path
    QRectF shapeRect;
    QPolygonF shapePolygon;
    QList<QPolygonF> shapeHoles;
    QList<bool> holeIsCircular;
    QPointF textAnchorLocal;
    qreal rotationDegrees = 0;
    qreal scaleShapePercent = 100.0;
    qreal scaleHolePercent = 100.0;
    int selectedHoleForScaleIndex = -1;
};

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
    void onAnchorPressByUser();  // public so TextAnchorDot can call when user presses (for undo)
    void pushUndo();  // public so VertexDot/HoleCenterDot (in .cpp) can call before user modifies
    void setSelectedHoleForScale(int holeIndex);  // public so HoleCenterDot can call when user clicks hole center

signals:
    void requestItemReplacement(QGraphicsItem *oldItem, int newType, const QPolygonF &outer,
                                const QList<QPolygonF> &holes, const QList<bool> &holeIsCircular,
                                const QPointF &textPosLocal, const QPointF &itemPos, qreal w, qreal h);

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
    ShapeEditorState captureState() const;
    void restoreState(const ShapeEditorState &s);
    void undo();
    void redo();
    void updateUndoRedoButtons();
    void scaleShapeBy(qreal factor);
    void scaleHoleBy(int holeIndex, qreal factor);
    void commitArrowKeyMoveSegment();

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
    QList<ShapeEditorState> m_undoStack;
    QList<ShapeEditorState> m_redoStack;
    QPushButton *m_undoButton = nullptr;
    QPushButton *m_redoButton = nullptr;
    qreal m_lastScaleShapePercent = 100.0;
    qreal m_lastScaleHolePercent = 100.0;
    int m_selectedHoleForScaleIndex = -1;  // which hole is selected for scale (for path with holes)
    QDoubleSpinBox *m_scaleShapeSpinBox = nullptr;
    QDoubleSpinBox *m_scaleHoleSpinBox = nullptr;
    QWidget *m_scaleHoleRow = nullptr;  // row containing "Scale selected hole" (always visible when path with holes)
    qreal m_viewZoomFactor = 1.0;  // view zoom (mouse wheel), no clamp
    QTimer *m_arrowCommitTimer = nullptr;
    ShapeEditorState m_arrowSegmentStartState;
    int m_arrowDirection = 0;  // 1=Left, 2=Right, 3=Up, 4=Down
};

#endif // SHAPEEDITORDIALOG_H
