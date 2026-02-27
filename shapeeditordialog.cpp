#include "shapeeditordialog.h"
#include "resizablerectitem.h"
#include "resizableellipseitem.h"
#include "resizablepolygonitem.h"
#include "resizablepathitem.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsPathItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QKeyEvent>
#include <QEvent>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <algorithm>
#include <cmath>

class DraggableDot : public QGraphicsEllipseItem
{
public:
    explicit DraggableDot(qreal x, qreal y, qreal r = 6, QGraphicsItem *parent = nullptr)
        : QGraphicsEllipseItem(-r, -r, 2*r, 2*r, parent)
    {
        setPos(x, y);
        setBrush(QColor(100, 200, 255));
        setPen(QPen(Qt::darkBlue, 1));
        setFlag(QGraphicsItem::ItemIsMovable);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    }
};

class TextAnchorDot : public DraggableDot
{
public:
    ShapeEditorDialog *m_dialog = nullptr;
    explicit TextAnchorDot(qreal x, qreal y, ShapeEditorDialog *dialog, QGraphicsItem *parent = nullptr)
        : DraggableDot(x, y, 6, parent)
        , m_dialog(dialog)
    {}
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override
    {
        QVariant result = DraggableDot::itemChange(change, value);
        if (change == ItemPositionHasChanged && m_dialog) {
            m_dialog->onAnchorMovedByUser();
            m_dialog->updateOffsetLabel();
        }
        return result;
    }
};

class VertexDot : public QGraphicsEllipseItem
{
public:
    int m_index = 0;
    int m_holeIndex = -1;  // -1 = outer contour
    ShapeEditorDialog *m_dialog = nullptr;

    explicit VertexDot(qreal x, qreal y, int index, int holeIndex, ShapeEditorDialog *dialog, QGraphicsItem *parent = nullptr)
        : QGraphicsEllipseItem(-5, -5, 10, 10, parent)
        , m_index(index)
        , m_holeIndex(holeIndex)
        , m_dialog(dialog)
    {
        setPos(x, y);
        setBrush(m_holeIndex >= 0 ? QColor(200, 150, 255) : QColor(255, 200, 100));
        setPen(QPen(m_holeIndex >= 0 ? Qt::darkMagenta : Qt::darkYellow, 1));
        setFlag(QGraphicsItem::ItemIsMovable);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges);
        setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override
    {
        if (change == ItemPositionChange && m_dialog) {
            QPointF newPos = value.toPointF();
            m_dialog->updatePolygonFromVertices(m_holeIndex, m_index, newPos);
        }
        return QGraphicsEllipseItem::itemChange(change, value);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override
    {
        QGraphicsEllipseItem::mousePressEvent(event);
        if (event->button() == Qt::LeftButton && m_dialog)
            m_dialog->setSelectedVertex(m_holeIndex, m_index);
    }

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override
    {
        if (!m_dialog)
            return;
        QMenu menu;
        QAction *addAfter = menu.addAction(m_dialog->tr("Add point after this"));
        QAction *delPoint = menu.addAction(m_dialog->tr("Delete point"));
        QAction *delHole = nullptr;
        if (m_holeIndex >= 0)
            delHole = menu.addAction(m_dialog->tr("Delete hole"));
        QPointF sp = event->screenPos();
        QAction *selected = menu.exec(QPoint(static_cast<int>(sp.x()), static_cast<int>(sp.y())));
        if (selected == addAfter)
            m_dialog->addPointAfter(m_holeIndex, m_index);
        else if (selected == delPoint)
            m_dialog->deletePoint(m_holeIndex, m_index);
        else if (selected == delHole && m_holeIndex >= 0)
            m_dialog->deleteHole(m_holeIndex);
    }
};

class HoleCenterDot : public QGraphicsEllipseItem
{
public:
    int m_holeIndex = 0;
    ShapeEditorDialog *m_dialog = nullptr;

    explicit HoleCenterDot(qreal x, qreal y, int holeIndex, ShapeEditorDialog *dialog, QGraphicsItem *parent = nullptr)
        : QGraphicsEllipseItem(-8, -8, 16, 16, parent)
        , m_holeIndex(holeIndex)
        , m_dialog(dialog)
    {
        setPos(x, y);
        setBrush(QColor(150, 255, 150));
        setPen(QPen(Qt::darkGreen, 1));
        setFlag(QGraphicsItem::ItemIsMovable);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges);
        setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override
    {
        if (change == ItemPositionChange && m_dialog) {
            QPointF newPos = value.toPointF();
            QPointF delta = newPos - pos();
            m_dialog->moveHoleBy(m_holeIndex, delta);
        }
        return QGraphicsEllipseItem::itemChange(change, value);
    }

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override
    {
        if (!m_dialog)
            return;
        QMenu menu;
        QAction *delHole = menu.addAction(m_dialog->tr("Delete hole"));
        QPointF sp = event->screenPos();
        if (menu.exec(QPoint(static_cast<int>(sp.x()), static_cast<int>(sp.y()))) == delHole)
            m_dialog->deleteHole(m_holeIndex);
    }
};

ShapeEditorDialog::ShapeEditorDialog(QWidget *parent, QGraphicsItem *shapeItem)
    : QDialog(parent)
    , m_shapeItem(shapeItem)
{
    setWindowTitle(tr("Edit shape"));
    setMinimumSize(400, 350);

    ResizableRectItem *rect = dynamic_cast<ResizableRectItem*>(shapeItem);
    ResizableEllipseItem *ellipse = dynamic_cast<ResizableEllipseItem*>(shapeItem);
    ResizablePolygonItem *polygon = dynamic_cast<ResizablePolygonItem*>(shapeItem);
    ResizablePathItem *pathItem = dynamic_cast<ResizablePathItem*>(shapeItem);

    if (rect) {
        m_shapeType = Rect;
        m_shapeRect = rect->rect();
        QRectF r = rect->rect();
        m_shapePolygon << QPointF(r.left(), r.top()) << QPointF(r.right(), r.top())
                       << QPointF(r.right(), r.bottom()) << QPointF(r.left(), r.bottom());
        m_textAnchorLocal = rect->textPosition();
    } else if (ellipse) {
        m_shapeType = Ellipse;
        m_shapeRect = ellipse->rect();
        m_textAnchorLocal = ellipse->textPosition();
    } else if (polygon) {
        m_shapeType = Polygon;
        m_shapePolygon = polygon->polygon();
        m_shapeRect = m_shapePolygon.boundingRect();
        m_textAnchorLocal = polygon->textPosition();
    } else if (pathItem) {
        m_shapeType = Path;
        m_shapePolygon = pathItem->outerPolygon();
        m_shapeHoles = pathItem->holes();
        m_holeIsCircular.resize(m_shapeHoles.size(), false);  // loaded holes are polygon
        m_shapeRect = m_shapePolygon.boundingRect();
        for (const QPolygonF &h : m_shapeHoles) {
            m_shapeRect = m_shapeRect.united(h.boundingRect());
        }
        m_textAnchorLocal = pathItem->textPosition();
    } else {
        return;
    }

    setupCanvas();
}

ShapeEditorDialog::~ShapeEditorDialog()
{
}

void ShapeEditorDialog::setupCanvas()
{
    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setRenderHint(QPainter::TextAntialiasing);
    m_view->setBackgroundBrush(QColor(50, 50, 50));
    m_view->setDragMode(QGraphicsView::NoDrag);
    m_view->setFocusPolicy(Qt::StrongFocus);
    m_view->installEventFilter(this);

    qreal margin = 20;
    QRectF viewRect = m_shapeRect.adjusted(-margin, -margin, margin, margin);
    m_scene->setSceneRect(viewRect);

    if (m_shapeType == Rect) {
        m_polygonDisplay = m_scene->addPolygon(m_shapePolygon, QPen(Qt::lightGray, 1), QBrush(QColor(80, 80, 80)));
        m_polygonDisplay->setZValue(0);
        rebuildVertexDots();
    } else if (m_shapeType == Ellipse) {
        QGraphicsEllipseItem *e = m_scene->addEllipse(m_shapeRect, QPen(Qt::lightGray, 1), QBrush(QColor(80, 80, 80)));
        e->setZValue(0);
    } else if (m_shapeType == Polygon) {
        m_polygonDisplay = m_scene->addPolygon(m_shapePolygon, QPen(Qt::lightGray, 1), QBrush(QColor(80, 80, 80)));
        m_polygonDisplay->setZValue(0);
        rebuildVertexDots();
    } else if (m_shapeType == Path) {
        QPainterPath path;
        path.addPolygon(m_shapePolygon);
        path.setFillRule(Qt::WindingFill);
        for (const QPolygonF &hole : m_shapeHoles) {
            QPainterPath holePath;
            holePath.addPolygon(hole);
            holePath.setFillRule(Qt::WindingFill);
            path = path.subtracted(holePath);
        }
        m_pathDisplay = m_scene->addPath(path, QPen(Qt::NoPen), QBrush(QColor(80, 80, 80)));
        m_pathDisplay->setZValue(0);
        QPainterPath outlinePath;
        if (!m_shapePolygon.isEmpty()) {
            outlinePath.moveTo(m_shapePolygon.first());
            for (int i = 1; i < m_shapePolygon.size(); ++i)
                outlinePath.lineTo(m_shapePolygon.at(i));
            outlinePath.closeSubpath();
            for (const QPolygonF &hole : m_shapeHoles) {
                if (hole.size() >= 2) {
                    outlinePath.moveTo(hole.first());
                    for (int i = 1; i < hole.size(); ++i)
                        outlinePath.lineTo(hole.at(i));
                    outlinePath.closeSubpath();
                }
            }
        }
        m_pathOutline = m_scene->addPath(outlinePath, QPen(Qt::lightGray, 1), Qt::NoBrush);
        m_pathOutline->setZValue(1);
        rebuildVertexDots();
    }

    m_textAnchorDot = new TextAnchorDot(m_textAnchorLocal.x(), m_textAnchorLocal.y(), this);
    m_scene->addItem(m_textAnchorDot);
    m_textAnchorDot->setZValue(10);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_view);

    // Rotation for Rect, Polygon, Path (not for Ellipse).
    if (m_shapeType != Ellipse) {
        QHBoxLayout *rotationRow = new QHBoxLayout();
        rotationRow->addWidget(new QLabel(tr("Rotation (degrees):"), this));
        m_degreeSpinBox = new QDoubleSpinBox(this);
        m_degreeSpinBox->setRange(-360, 360);
        m_degreeSpinBox->setValue(0);
        m_degreeSpinBox->setSuffix(QStringLiteral(" \u00B0"));
        m_degreeSpinBox->setDecimals(1);
        rotationRow->addWidget(m_degreeSpinBox);
        rotationRow->addStretch();
        mainLayout->addLayout(rotationRow);
        connect(m_degreeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](qreal newDegrees) {
            qreal delta = newDegrees - m_rotationDegrees;
            m_rotationDegrees = newDegrees;
            applyRotationToShape(delta);
        });
    }

    // Center text: when checked, text stays at shape center (also when resizing in layout editor).
    m_centerTextCheckBox = new QCheckBox(tr("Center text"), this);
    bool initiallyCentered = true;
    if (ResizableRectItem *r = dynamic_cast<ResizableRectItem*>(m_shapeItem))
        initiallyCentered = !r->hasCustomTextPosition();
    else if (ResizableEllipseItem *e = dynamic_cast<ResizableEllipseItem*>(m_shapeItem))
        initiallyCentered = !e->hasCustomTextPosition();
    else if (ResizablePolygonItem *p = dynamic_cast<ResizablePolygonItem*>(m_shapeItem))
        initiallyCentered = !p->hasCustomTextPosition();
    else if (ResizablePathItem *path = dynamic_cast<ResizablePathItem*>(m_shapeItem))
        initiallyCentered = !path->hasCustomTextPosition();
    m_centerTextCheckBox->setChecked(initiallyCentered);
    mainLayout->addWidget(m_centerTextCheckBox);
    connect(m_centerTextCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        if (checked) {
            updateAnchorToCenterIfChecked();
        } else if (m_textAnchorDot) {
            m_textAnchorDot->setFlag(QGraphicsItem::ItemIsMovable, true);
        }
    });
    updateAnchorToCenterIfChecked();

    if (m_shapeType == Polygon || m_shapeType == Path || m_shapeType == Rect) {
        m_offsetLabel = new QLabel(tr("Select a point to see position (text anchor = 0,0). Arrow keys: 1 px; Shift+arrow: 10 px."), this);
        m_offsetLabel->setStyleSheet(QStringLiteral("color: #aaa;"));
        mainLayout->addWidget(m_offsetLabel);
    }

    QHBoxLayout *buttons = new QHBoxLayout();
    if (m_shapeType == Polygon || m_shapeType == Path || m_shapeType == Rect) {
        QPushButton *addHoleBtn = new QPushButton(tr("Add polygon hole"), this);
        connect(addHoleBtn, &QPushButton::clicked, this, &ShapeEditorDialog::addHole);
        buttons->addWidget(addHoleBtn);
        QPushButton *addCircularBtn = new QPushButton(tr("Add circular hole"), this);
        connect(addCircularBtn, &QPushButton::clicked, this, &ShapeEditorDialog::addCircularHole);
        buttons->addWidget(addCircularBtn);
    }
    QPushButton *cancelBtn = new QPushButton(tr("Cancel"), this);
    QPushButton *applyBtn = new QPushButton(tr("Apply"), this);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(applyBtn, &QPushButton::clicked, this, [this]() { applyChanges(); accept(); });
    buttons->addWidget(cancelBtn);
    buttons->addWidget(applyBtn);
    mainLayout->addLayout(buttons);
}

void ShapeEditorDialog::applyRotationToShape(qreal deltaDegrees)
{
    if (qAbs(deltaDegrees) < 1e-6)
        return;
    qreal rad = deltaDegrees * M_PI / 180.0;
    qreal c = std::cos(rad), s = std::sin(rad);

    if (m_shapeType != Ellipse) {
        QRectF br = m_shapePolygon.boundingRect();
        for (const QPolygonF &h : m_shapeHoles)
            br = br.united(h.boundingRect());
        QPointF center = br.center();
        auto rotate = [&](QPointF p) {
            p -= center;
            return center + QPointF(p.x() * c - p.y() * s, p.x() * s + p.y() * c);
        };
        for (int i = 0; i < m_shapePolygon.size(); ++i)
            m_shapePolygon[i] = rotate(m_shapePolygon[i]);
        for (int h = 0; h < m_shapeHoles.size(); ++h)
            for (int i = 0; i < m_shapeHoles[h].size(); ++i)
                m_shapeHoles[h][i] = rotate(m_shapeHoles[h][i]);
        m_textAnchorLocal = rotate(m_textAnchorLocal);
        if (m_textAnchorDot)
            m_textAnchorDot->setPos(m_textAnchorLocal);
        refreshDisplay();
        rebuildVertexDots();
    }
    updateAnchorToCenterIfChecked();
}

QPointF ShapeEditorDialog::shapeCenter() const
{
    if (m_shapeType == Ellipse)
        return m_shapeRect.center();
    QRectF br = m_shapePolygon.boundingRect();
    for (const QPolygonF &h : m_shapeHoles)
        br = br.united(h.boundingRect());
    return br.center();
}

void ShapeEditorDialog::updateAnchorToCenterIfChecked()
{
    if (!m_centerTextCheckBox || !m_centerTextCheckBox->isChecked() || !m_textAnchorDot)
        return;
    QPointF center = shapeCenter();
    m_textAnchorLocal = center;
    m_anchorSetByCode = true;
    m_textAnchorDot->setPos(center);
    m_textAnchorDot->setFlag(QGraphicsItem::ItemIsMovable, false);
}

void ShapeEditorDialog::onAnchorMovedByUser()
{
    if (m_anchorSetByCode) {
        m_anchorSetByCode = false;
        return;
    }
    if (m_centerTextCheckBox)
        m_centerTextCheckBox->setChecked(false);
    if (m_textAnchorDot)
        m_textAnchorDot->setFlag(QGraphicsItem::ItemIsMovable, true);
    if (m_textAnchorDot)
        m_textAnchorLocal = m_textAnchorDot->pos();
}

void ShapeEditorDialog::rebuildVertexDots()
{
    m_selectedHoleIndex = -2;
    m_selectedVertexIndex = -1;
    updateOffsetLabel();
    for (QGraphicsItem *dot : m_vertexDots)
        m_scene->removeItem(dot);
    m_vertexDots.clear();

    auto addDotsForPoly = [this](const QPolygonF &poly, int holeIdx) {
        for (int i = 0; i < poly.size(); ++i) {
            const QPointF &p = poly.at(i);
            VertexDot *vd = new VertexDot(p.x(), p.y(), i, holeIdx, this);
            m_scene->addItem(vd);
            vd->setZValue(5);
            m_vertexDots.append(vd);
        }
    };

    if (m_shapeType == Polygon || m_shapeType == Path || m_shapeType == Rect) {
        addDotsForPoly(m_shapePolygon, -1);
        for (int h = 0; h < m_shapeHoles.size(); ++h) {
            bool isCircular = (h < m_holeIsCircular.size() && m_holeIsCircular[h]);
            if (isCircular) {
                QPolygonF &hole = m_shapeHoles[h];
                qreal cx = 0, cy = 0;
                for (const QPointF &p : hole) { cx += p.x(); cy += p.y(); }
                if (!hole.isEmpty()) { cx /= hole.size(); cy /= hole.size(); }
                HoleCenterDot *hd = new HoleCenterDot(cx, cy, h, this);
                m_scene->addItem(hd);
                hd->setZValue(5);
                m_vertexDots.append(hd);
            } else {
                addDotsForPoly(m_shapeHoles[h], h);
                qreal cx = 0, cy = 0;
                for (const QPointF &p : m_shapeHoles[h]) { cx += p.x(); cy += p.y(); }
                if (!m_shapeHoles[h].isEmpty()) { cx /= m_shapeHoles[h].size(); cy /= m_shapeHoles[h].size(); }
                HoleCenterDot *hd = new HoleCenterDot(cx, cy, h, this);
                m_scene->addItem(hd);
                hd->setZValue(5);
                m_vertexDots.append(hd);
            }
        }
    }
}

void ShapeEditorDialog::updatePolygonFromVertices(int holeIndex, int movedIndex, const QPointF &movedPos)
{
    if (m_shapeType != Polygon && m_shapeType != Path && m_shapeType != Rect)
        return;

    auto collectPolyFromDots = [this, holeIndex, movedIndex, movedPos](int hIdx) -> QPolygonF {
        QPolygonF poly;
        QList<VertexDot*> dots;
        for (QGraphicsItem *item : m_vertexDots) {
            VertexDot *vd = dynamic_cast<VertexDot*>(item);
            if (vd && vd->m_holeIndex == hIdx)
                dots.append(vd);
        }
        std::sort(dots.begin(), dots.end(), [](VertexDot *a, VertexDot *b) { return a->m_index < b->m_index; });
        for (VertexDot *vd : dots) {
            QPointF pt = vd->pos();
            if (hIdx == holeIndex && vd->m_index == movedIndex && !movedPos.isNull())
                pt = movedPos;
            poly << pt;
        }
        return poly;
    };

    m_shapePolygon = collectPolyFromDots(-1);
    for (int h = 0; h < m_shapeHoles.size(); ++h) {
        if (h < m_holeIsCircular.size() && m_holeIsCircular[h])
            continue;
        m_shapeHoles[h] = collectPolyFromDots(h);
    }

    refreshDisplay();
    updateAnchorToCenterIfChecked();
    updateOffsetLabel();
}

void ShapeEditorDialog::setSelectedVertex(int holeIndex, int vertexIndex)
{
    m_selectedHoleIndex = holeIndex;
    m_selectedVertexIndex = vertexIndex;
    if (m_view)
        m_view->setFocus();
    updateOffsetLabel();
    for (QGraphicsItem *item : m_vertexDots) {
        VertexDot *vd = dynamic_cast<VertexDot*>(item);
        if (vd) {
            bool sel = (vd->m_holeIndex == holeIndex && vd->m_index == vertexIndex);
            vd->setPen(QPen(sel ? Qt::cyan : (vd->m_holeIndex >= 0 ? Qt::darkMagenta : Qt::darkYellow), sel ? 2 : 1));
        }
    }
}

void ShapeEditorDialog::updateOffsetLabel()
{
    if (!m_offsetLabel)
        return;
    if (m_selectedHoleIndex < -1 || m_selectedVertexIndex < 0) {
        m_offsetLabel->setText(tr("Select a point to see position (text anchor = 0,0). Arrow keys: 1 px; Shift+arrow: 10 px."));
        return;
    }
    VertexDot *vd = nullptr;
    for (QGraphicsItem *item : m_vertexDots) {
        VertexDot *d = dynamic_cast<VertexDot*>(item);
        if (d && d->m_holeIndex == m_selectedHoleIndex && d->m_index == m_selectedVertexIndex) {
            vd = d;
            break;
        }
    }
    if (!vd) {
        m_offsetLabel->setText(tr("Select a point to see position (text anchor = 0,0)."));
        return;
    }
    QPointF origin = m_textAnchorDot ? m_textAnchorDot->pos() : QPointF(0, 0);
    QPointF p = vd->pos();
    int px = qRound(p.x() - origin.x());
    int py = qRound(p.y() - origin.y());
    m_offsetLabel->setText(tr("Position: %1, %2  (text anchor = 0,0)").arg(px).arg(py));
}

void ShapeEditorDialog::moveSelectedVertexBy(int dx, int dy)
{
    if (m_selectedHoleIndex < -1 || m_selectedVertexIndex < 0)
        return;
    for (QGraphicsItem *item : m_vertexDots) {
        VertexDot *vd = dynamic_cast<VertexDot*>(item);
        if (vd && vd->m_holeIndex == m_selectedHoleIndex && vd->m_index == m_selectedVertexIndex) {
            vd->setPos(vd->pos() + QPointF(dx, dy));
            updatePolygonFromVertices(m_selectedHoleIndex, m_selectedVertexIndex, vd->pos());
            return;
        }
    }
}

bool ShapeEditorDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_view && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();
        int dx = 0, dy = 0;
        int step = (keyEvent->modifiers() & Qt::ShiftModifier) ? 10 : 1;
        if (key == Qt::Key_Left) dx = -step;
        else if (key == Qt::Key_Right) dx = step;
        else if (key == Qt::Key_Up) dy = -step;
        else if (key == Qt::Key_Down) dy = step;
        if ((dx != 0 || dy != 0) && m_selectedHoleIndex >= -1 && m_selectedVertexIndex >= 0) {
            moveSelectedVertexBy(dx, dy);
            keyEvent->accept();
            return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}

void ShapeEditorDialog::refreshDisplay()
{
    if (m_polygonDisplay && (m_shapeType == Polygon || m_shapeType == Rect))
        m_polygonDisplay->setPolygon(m_shapePolygon);
    else if (m_pathDisplay && m_shapeType == Path) {
        QPainterPath path;
        if (!m_shapePolygon.isEmpty()) {
            path.addPolygon(m_shapePolygon);
            path.setFillRule(Qt::WindingFill);
            for (const QPolygonF &hole : m_shapeHoles) {
                if (hole.size() >= 3) {
                    QPainterPath holePath;
                    holePath.addPolygon(hole);
                    holePath.setFillRule(Qt::WindingFill);
                    path = path.subtracted(holePath);
                }
            }
        }
        m_pathDisplay->setPath(path);
        QPainterPath outlinePath;
        if (!m_shapePolygon.isEmpty()) {
            outlinePath.moveTo(m_shapePolygon.first());
            for (int i = 1; i < m_shapePolygon.size(); ++i)
                outlinePath.lineTo(m_shapePolygon.at(i));
            outlinePath.closeSubpath();
            for (const QPolygonF &hole : m_shapeHoles) {
                if (hole.size() >= 2) {
                    outlinePath.moveTo(hole.first());
                    for (int i = 1; i < hole.size(); ++i)
                        outlinePath.lineTo(hole.at(i));
                    outlinePath.closeSubpath();
                }
            }
        }
        if (!m_pathOutline) {
            m_pathOutline = m_scene->addPath(outlinePath, QPen(Qt::lightGray, 1), Qt::NoBrush);
            m_pathOutline->setZValue(1);
        } else {
            m_pathOutline->setPath(outlinePath);
        }
    }
}

void ShapeEditorDialog::addPointAfter(int holeIndex, int vertexIndex)
{
    if (m_shapeType != Polygon && m_shapeType != Path && m_shapeType != Rect)
        return;

    QPolygonF *target = (holeIndex < 0) ? &m_shapePolygon : (holeIndex < m_shapeHoles.size() ? &m_shapeHoles[holeIndex] : nullptr);
    if (!target || vertexIndex < 0 || vertexIndex >= target->size())
        return;
    int next = (vertexIndex + 1) % target->size();
    QPointF mid = (target->at(vertexIndex) + target->at(next)) / 2;
    target->insert(vertexIndex + 1, mid);

    refreshDisplay();
    rebuildVertexDots();
    updateAnchorToCenterIfChecked();
}

void ShapeEditorDialog::deletePoint(int holeIndex, int vertexIndex)
{
    if (m_shapeType != Polygon && m_shapeType != Path && m_shapeType != Rect)
        return;

    QPolygonF *target = (holeIndex < 0) ? &m_shapePolygon : (holeIndex < m_shapeHoles.size() ? &m_shapeHoles[holeIndex] : nullptr);
    if (!target || vertexIndex < 0 || vertexIndex >= target->size())
        return;
    const int minPoints = (holeIndex < 0) ? 3 : 3;  // outer needs 3+ for valid polygon; holes need 3+
    if (target->size() <= minPoints)
        return;
    target->removeAt(vertexIndex);

    refreshDisplay();
    rebuildVertexDots();
    updateAnchorToCenterIfChecked();
}

void ShapeEditorDialog::addHole()
{
    if (m_shapeType != Path && m_shapeType != Polygon && m_shapeType != Rect)
        return;
    if (m_shapeType == Polygon || m_shapeType == Rect) {
        m_shapeType = Path;
        if (m_polygonDisplay) {
            m_scene->removeItem(m_polygonDisplay);
            delete m_polygonDisplay;
            m_polygonDisplay = nullptr;
        }
        m_pathDisplay = m_scene->addPath(QPainterPath(), QPen(Qt::lightGray, 1), QBrush(QColor(80, 80, 80)));
        m_pathDisplay->setZValue(0);
    }
    QRectF br = m_shapePolygon.boundingRect();
    qreal cx = br.center().x();
    qreal cy = br.center().y();
    qreal s = qMin(br.width(), br.height()) * 0.2;
    if (s < 5) s = 5;
    QPolygonF hole;
    hole << QPointF(cx - s, cy - s) << QPointF(cx + s, cy - s) << QPointF(cx + s, cy + s) << QPointF(cx - s, cy + s);
    m_shapeHoles.append(hole);
    m_holeIsCircular.append(false);
    if (!m_pathDisplay && (m_shapeType == Polygon || m_shapeType == Rect)) {
        m_shapeType = Path;
        m_polygonDisplay = nullptr;
        m_pathDisplay = m_scene->addPath(QPainterPath(), QPen(Qt::lightGray, 1), QBrush(QColor(80, 80, 80)));
        m_pathDisplay->setZValue(0);
    }
    refreshDisplay();
    rebuildVertexDots();
    updateAnchorToCenterIfChecked();
}

void ShapeEditorDialog::addCircularHole()
{
    if (m_shapeType != Path && m_shapeType != Polygon && m_shapeType != Rect)
        return;
    if (m_shapeType == Polygon || m_shapeType == Rect) {
        if (!m_pathDisplay) {
            m_shapeType = Path;
            m_polygonDisplay = nullptr;
            m_pathDisplay = m_scene->addPath(QPainterPath(), QPen(Qt::lightGray, 1), QBrush(QColor(80, 80, 80)));
            m_pathDisplay->setZValue(0);
        }
    }
    QRectF br = m_shapePolygon.boundingRect();
    qreal cx = br.center().x();
    qreal cy = br.center().y();
    qreal r = qMin(br.width(), br.height()) * 0.15;
    if (r < 4) r = 4;
    const int n = 32;
    QPolygonF hole;
    for (int i = 0; i < n; ++i) {
        qreal a = 2.0 * M_PI * i / n;
        hole << QPointF(cx + r * cos(a), cy + r * sin(a));
    }
    m_shapeHoles.append(hole);
    m_holeIsCircular.append(true);
    refreshDisplay();
    rebuildVertexDots();
    updateAnchorToCenterIfChecked();
}

void ShapeEditorDialog::moveHoleBy(int holeIndex, const QPointF &delta)
{
    if (holeIndex < 0 || holeIndex >= m_shapeHoles.size())
        return;
    QPolygonF &hole = m_shapeHoles[holeIndex];
    for (int i = 0; i < hole.size(); ++i)
        hole[i] += delta;
    for (QGraphicsItem *item : m_vertexDots) {
        VertexDot *vd = dynamic_cast<VertexDot*>(item);
        if (vd && vd->m_holeIndex == holeIndex)
            vd->setPos(vd->pos() + delta);
    }
    refreshDisplay();
}

void ShapeEditorDialog::deleteHole(int holeIndex)
{
    if (m_shapeType != Path || holeIndex < 0 || holeIndex >= m_shapeHoles.size())
        return;
    m_shapeHoles.removeAt(holeIndex);
    if (holeIndex < m_holeIsCircular.size())
        m_holeIsCircular.removeAt(holeIndex);
    rebuildVertexDots();
    refreshDisplay();
    if (m_view && m_view->viewport())
        m_view->viewport()->update();
}

void ShapeEditorDialog::applyChanges()
{
    if (!m_shapeItem)
        return;

    QPointF newTextPos = m_textAnchorDot->pos();

    if ((m_shapeType == Polygon || m_shapeType == Path || m_shapeType == Rect) && !m_vertexDots.isEmpty()) {
        auto collectPoly = [this](int hIdx) -> QPolygonF {
            QPolygonF poly;
            QList<VertexDot*> dots;
            for (QGraphicsItem *item : m_vertexDots) {
                VertexDot *vd = dynamic_cast<VertexDot*>(item);
                if (vd && vd->m_holeIndex == hIdx)
                    dots.append(vd);
            }
            std::sort(dots.begin(), dots.end(), [](VertexDot *a, VertexDot *b) { return a->m_index < b->m_index; });
            for (VertexDot *vd : dots)
                poly << vd->pos();
            return poly;
        };
        m_shapePolygon = collectPoly(-1);
        for (int h = 0; h < m_shapeHoles.size(); ++h) {
            if (h < m_holeIsCircular.size() && m_holeIsCircular[h])
                continue;  // circular hole geometry comes from moveHoleBy, not vertex dots
            m_shapeHoles[h] = collectPoly(h);
        }
    }

    ResizableRectItem *rect = dynamic_cast<ResizableRectItem*>(m_shapeItem);
    ResizableEllipseItem *ellipse = dynamic_cast<ResizableEllipseItem*>(m_shapeItem);
    ResizablePolygonItem *polygon = dynamic_cast<ResizablePolygonItem*>(m_shapeItem);
    ResizablePathItem *pathItem = dynamic_cast<ResizablePathItem*>(m_shapeItem);

    if (rect) {
        if (m_shapeType == Rect && m_shapePolygon.size() == 4 && m_shapeHoles.isEmpty()) {
            qreal x0 = qMin(m_shapePolygon[0].x(), qMin(m_shapePolygon[1].x(), qMin(m_shapePolygon[2].x(), m_shapePolygon[3].x())));
            qreal y0 = qMin(m_shapePolygon[0].y(), qMin(m_shapePolygon[1].y(), qMin(m_shapePolygon[2].y(), m_shapePolygon[3].y())));
            qreal x1 = qMax(m_shapePolygon[0].x(), qMax(m_shapePolygon[1].x(), qMax(m_shapePolygon[2].x(), m_shapePolygon[3].x())));
            qreal y1 = qMax(m_shapePolygon[0].y(), qMax(m_shapePolygon[1].y(), qMax(m_shapePolygon[2].y(), m_shapePolygon[3].y())));
            bool axisAligned = true;
            for (int i = 0; i < 4; ++i) {
                const QPointF &p = m_shapePolygon[i];
                if ((qAbs(p.x() - x0) > 0.5 && qAbs(p.x() - x1) > 0.5) || (qAbs(p.y() - y0) > 0.5 && qAbs(p.y() - y1) > 0.5))
                    axisAligned = false;
            }
            if (axisAligned && (x1 - x0) > 1 && (y1 - y0) > 1) {
                rect->setRect(0, 0, x1 - x0, y1 - y0);
                rect->setPos(rect->pos() + QPointF(x0, y0));
                if (m_centerTextCheckBox && m_centerTextCheckBox->isChecked())
                    rect->setTextPositionToCenter();
                else
                    rect->setTextPosition(newTextPos - QPointF(x0, y0));
                return;
            }
        }
        QRectF br = m_shapePolygon.boundingRect();
        for (const QPolygonF &h : m_shapeHoles)
            br = br.united(h.boundingRect());
        QPointF tl = br.topLeft();
        QPolygonF outerLocal;
        for (const QPointF &p : m_shapePolygon)
            outerLocal << (p - tl);
        QList<QPolygonF> holesLocal;
        for (const QPolygonF &h : m_shapeHoles) {
            QPolygonF hl;
            for (const QPointF &p : h) hl << (p - tl);
            holesLocal << hl;
        }
        QPointF itemPos = rect->mapToScene(tl);
        int replaceType = holesLocal.isEmpty() ? 3 : 4;
        emit requestItemReplacement(m_shapeItem, replaceType, outerLocal, holesLocal, newTextPos - tl, itemPos, br.width(), br.height());
        return;
    } else if (ellipse) {
        if (m_centerTextCheckBox && m_centerTextCheckBox->isChecked())
            ellipse->setTextPositionToCenter();
        else
            ellipse->setTextPosition(newTextPos);
    } else if (polygon) {
        if (m_shapeType == Path && !m_shapeHoles.isEmpty()) {
            QRectF br = m_shapePolygon.boundingRect();
            for (const QPolygonF &h : m_shapeHoles)
                br = br.united(h.boundingRect());
            QPointF tl = br.topLeft();
            QPolygonF outerLocal;
            for (const QPointF &p : m_shapePolygon)
                outerLocal << (p - tl);
            QList<QPolygonF> holesLocal;
            for (const QPolygonF &h : m_shapeHoles) {
                QPolygonF hl;
                for (const QPointF &p : h) hl << (p - tl);
                holesLocal << hl;
            }
            QPointF itemPos = polygon->mapToScene(tl);
            emit requestItemReplacement(m_shapeItem, 4, outerLocal, holesLocal, newTextPos - tl, itemPos, br.width(), br.height());
            return;
        }
        if (m_centerTextCheckBox && m_centerTextCheckBox->isChecked())
            polygon->setTextPositionToCenter();
        else
            polygon->setTextPosition(newTextPos);
        if (m_shapeType == Polygon && m_shapePolygon.size() >= 4)
            polygon->setPolygonDirect(m_shapePolygon);
    } else if (pathItem) {
        if (m_centerTextCheckBox && m_centerTextCheckBox->isChecked())
            pathItem->setTextPositionToCenter();
        else
            pathItem->setTextPosition(newTextPos);
        pathItem->setPathFromOuterAndHoles(m_shapePolygon, m_shapeHoles);
    }
}
