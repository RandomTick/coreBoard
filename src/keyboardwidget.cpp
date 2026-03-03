#include "KeyboardWidget.h"
#include "keystyle.h"
#include "controlleritem.h"
#ifdef Q_OS_WIN
#include "gamepadlistener.h"
#endif
#include <QSvgRenderer>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QGraphicsTextItem>
#include <QAbstractGraphicsShapeItem>
#include <QTextDocument>
#include <QTextOption>
#include <QVBoxLayout>
#include <QTimer>
#include <QCursor>
#include <QDateTime>
#include <cmath>

namespace {
const int BaseTextRole = Qt::UserRole;
const int ShiftTextRole = Qt::UserRole + 1;
const int KeyColorRole = Qt::UserRole + 2;
const int TextPositionLocalRole = Qt::UserRole + 6;  // QPointF in shape local coords, or invalid if use center
const int TextAlignmentRole = Qt::UserRole + 7;     // int: 0=left, 1=center, 2=right
const int KeyColorPressedRole = Qt::UserRole + 3;
const int KeyTextColorRole = Qt::UserRole + 4;
const int KeyTextColorPressedRole = Qt::UserRole + 5;
const int KeyCodesRole = Qt::UserRole + 8;       // QList<int> all key codes for this key (for OR + trigger fill)
const int TriggerCodesRole = Qt::UserRole + 9;   // QList<int> subset that are gamepad trigger codes
const int AnalogFillRole = Qt::UserRole + 10;    // QGraphicsPathItem* child used for trigger fill (bottom-to-top)

static Qt::Alignment alignmentFromInt(int a) {
    return (a == 0) ? Qt::AlignLeft : ((a == 2) ? Qt::AlignRight : Qt::AlignHCenter);
}

/// Returns the bounding rect of the shape path when stroked with the given pen width (for clipping the outline).
static QRectF strokedPathBounds(const QPainterPath &path, qreal penWidth)
{
    if (path.isEmpty() || penWidth <= 0.0)
        return path.boundingRect();
    QPainterPathStroker stroker;
    stroker.setWidth(penWidth);
    stroker.setJoinStyle(Qt::MiterJoin);
    stroker.setCapStyle(Qt::SquareCap);
    return stroker.createStroke(path).boundingRect();
}

/// Returns the stroked path (fill region expanded by pen width) for path-based clipping so the clip follows the shape (e.g. trapezoid).
static QPainterPath strokedPathForClip(const QPainterPath &path, qreal penWidth)
{
    if (path.isEmpty() || penWidth <= 0.0)
        return path;
    QPainterPathStroker stroker;
    stroker.setWidth(penWidth);
    stroker.setJoinStyle(Qt::MiterJoin);
    stroker.setCapStyle(Qt::SquareCap);
    stroker.setCurveThreshold(0.25);
    QPainterPath stroke = stroker.createStroke(path);
    stroke.setFillRule(Qt::WindingFill);
    return stroke;
}

/// Clip polygon to rect (Sutherland-Hodgman). Used for trigger fill so we don't rely on
/// QPainterPath::intersected(), which can misbehave for some trapezoids (e.g. Fine#2).
/// Single-edge helper: keep vertices on the "inside" side of the edge, add intersections when crossing.
static QPolygonF clipPolygonToRectOneEdge(const QPolygonF &in, int edgeKind, qreal edgeVal)
{
    QPolygonF out;
    const int n = in.size();
    for (int i = 0; i < n; ++i) {
        const QPointF &p = in[i];
        const QPointF &q = in[(i + 1) % n];
        bool pIn = false, qIn = false;
        if (edgeKind == 0) { pIn = p.x() >= edgeVal; qIn = q.x() >= edgeVal; }       // left
        else if (edgeKind == 1) { pIn = p.x() <= edgeVal; qIn = q.x() <= edgeVal; }   // right
        else if (edgeKind == 2) { pIn = p.y() <= edgeVal; qIn = q.y() <= edgeVal; }   // bottom
        else { pIn = p.y() >= edgeVal; qIn = q.y() >= edgeVal; }                     // top
        if (pIn && qIn)
            out.append(q);
        else if (pIn && !qIn) {
            qreal t = 0;
            if (edgeKind == 0 && qAbs(q.x() - p.x()) > 1e-9) { t = (edgeVal - p.x()) / (q.x() - p.x()); out.append(QPointF(edgeVal, p.y() + t * (q.y() - p.y()))); }
            else if (edgeKind == 1 && qAbs(q.x() - p.x()) > 1e-9) { t = (edgeVal - p.x()) / (q.x() - p.x()); out.append(QPointF(edgeVal, p.y() + t * (q.y() - p.y()))); }
            else if (edgeKind == 2 && qAbs(q.y() - p.y()) > 1e-9) { t = (edgeVal - p.y()) / (q.y() - p.y()); out.append(QPointF(p.x() + t * (q.x() - p.x()), edgeVal)); }
            else if (edgeKind == 3 && qAbs(q.y() - p.y()) > 1e-9) { t = (edgeVal - p.y()) / (q.y() - p.y()); out.append(QPointF(p.x() + t * (q.x() - p.x()), edgeVal)); }
        } else if (!pIn && qIn) {
            qreal t = 0;
            if (edgeKind == 0 && qAbs(q.x() - p.x()) > 1e-9) { t = (edgeVal - p.x()) / (q.x() - p.x()); out.append(QPointF(edgeVal, p.y() + t * (q.y() - p.y()))); }
            else if (edgeKind == 1 && qAbs(q.x() - p.x()) > 1e-9) { t = (edgeVal - p.x()) / (q.x() - p.x()); out.append(QPointF(edgeVal, p.y() + t * (q.y() - p.y()))); }
            else if (edgeKind == 2 && qAbs(q.y() - p.y()) > 1e-9) { t = (edgeVal - p.y()) / (q.y() - p.y()); out.append(QPointF(p.x() + t * (q.x() - p.x()), edgeVal)); }
            else if (edgeKind == 3 && qAbs(q.y() - p.y()) > 1e-9) { t = (edgeVal - p.y()) / (q.y() - p.y()); out.append(QPointF(p.x() + t * (q.x() - p.x()), edgeVal)); }
            out.append(q);
        }
    }
    return out;
}

static QPolygonF clipPolygonToRect(const QPolygonF &poly, const QRectF &rect)
{
    if (poly.size() < 3)
        return QPolygonF();
    qreal xMin = rect.left(), xMax = rect.right(), yMin = rect.top(), yMax = rect.bottom();
    QPolygonF work = poly;
    work = clipPolygonToRectOneEdge(work, 0, xMin);
    if (work.size() < 3) return QPolygonF();
    work = clipPolygonToRectOneEdge(work, 1, xMax);
    if (work.size() < 3) return QPolygonF();
    work = clipPolygonToRectOneEdge(work, 2, yMax);
    if (work.size() < 3) return QPolygonF();
    work = clipPolygonToRectOneEdge(work, 3, yMin);
    return work;
}

/// Returns the shape outline as QPainterPath in item local coords (for intersection with fill rect).
/// Uses WindingFill so intersection and stroking behave consistently for all polygons (e.g. trapezoids).
static QPainterPath shapePathFromItem(QGraphicsItem *item)
{
    QPainterPath path;
    if (QGraphicsRectItem *rectItem = qgraphicsitem_cast<QGraphicsRectItem*>(item)) {
        path.addRect(rectItem->rect());
    } else if (QGraphicsEllipseItem *ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(item)) {
        path.addEllipse(ellipseItem->rect());
    } else if (QGraphicsPolygonItem *polyItem = qgraphicsitem_cast<QGraphicsPolygonItem*>(item)) {
        path.addPolygon(polyItem->polygon());
    } else if (QGraphicsPathItem *pathItem = qgraphicsitem_cast<QGraphicsPathItem*>(item)) {
        path = pathItem->path();
    } else {
        path.addRect(item->boundingRect());
    }
    path.setFillRule(Qt::WindingFill);
    return path;
}

// Controller overlay: draws SVG and highlights for gamepad buttons (indices 0..15; 14=LT, 15=RT).
// Region data in SVG space (viewBox 0 0 580.032 580.032). Circle: (x,y)=center, r=radius. Rect: r<0 then (x,y,w,h).
// Regions and D-pad shape integrated from controller_regions.json so installer builds work without external file.
struct ControllerRegion { qreal x, y, w, h, r; };
static const ControllerRegion s_controllerRegionsBuiltin[16] = {
    { 438.047, 251.421, 0, 0, 19 },    // 0 A
    { 479.162, 211.323, 0, 0, 18.77 }, // 1 B
    { 399.932, 211.323, 0, 0, 18.77 }, // 2 X
    { 438.047, 173.226, 0, 0, 18.77 }, // 3 Y
    { 88, 106, 65, 18, -1 },          // 4 LB
    { 427, 106, 65, 18, -1 },         // 5 RB
    { 248, 211, 0, 0, 10 },            // 6 Back
    { 331.5, 211, 0, 0, 10 },          // 7 Start
    { 142, 210, 0, 0, 20 },            // 8 LStick
    { 365.5, 300, 0, 0, 20 },          // 9 RStick
    { 216, 296, 0, 0, 11 },            // 10 D-pad up
    { 216, 314, 0, 0, 11 },            // 11 D-pad down
    { 206, 305, 0, 0, 11 },            // 12 D-pad left
    { 226, 305, 0, 0, 11 },            // 13 D-pad right
    { 88, 70, 65, 28, -1 },            // 14 LT
    { 427, 70, 65, 28, -1 },           // 15 RT
};
static QVector<ControllerRegion> s_controllerRegions;
static const qreal s_dpadBaseWidth = 20.0;
static const qreal s_dpadBaseHeight = 26.0;
static const qreal s_dpadTipLength = 10.0;
static void ensureControllerRegionsLoaded() {
    if (!s_controllerRegions.isEmpty()) return;
    for (int i = 0; i < 16; ++i)
        s_controllerRegions.append(s_controllerRegionsBuiltin[i]);
}

// Inner ring radius in SVG space for stick indicator movement (fraction of region r).
static const qreal s_stickInnerRadiusFraction = 0.6;
// Stick indicator dot radius in SVG space (drawn at stick center + offset).
static const qreal s_stickIndicatorRadiusSvg = 14.0;

class ControllerViewItem : public QGraphicsRectItem {
public:
    ControllerViewItem(const QRectF &rect) : QGraphicsRectItem(rect) {
        setBrush(Qt::NoBrush);
        setPen(Qt::NoPen);
        setFlag(QGraphicsItem::ItemStacksBehindParent, false);
    }
    void setFillColor(const QColor &c) { m_fillColor = c; update(); }
    void setHighlightState(const QSet<int> &buttons, qreal leftTrigger, qreal rightTrigger) {
        if (m_buttons == buttons && qFuzzyCompare(m_leftTrigger, leftTrigger) && qFuzzyCompare(m_rightTrigger, rightTrigger))
            return;
        m_buttons = buttons;
        m_leftTrigger = leftTrigger;
        m_rightTrigger = rightTrigger;
        update();
    }
    void setLeftStick(qreal x, qreal y) {
        if (qFuzzyCompare(m_leftStickX, x) && qFuzzyCompare(m_leftStickY, y)) return;
        m_leftStickX = x; m_leftStickY = y; update();
    }
    void setRightStick(qreal x, qreal y) {
        if (qFuzzyCompare(m_rightStickX, x) && qFuzzyCompare(m_rightStickY, y)) return;
        m_rightStickX = x; m_rightStickY = y; update();
    }
    void setStickIndicatorColor(const QColor &c) { m_highlightColor = c; update(); }
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override {
        const QRectF r = rect();
        if (r.isEmpty()) return;
        ensureControllerRegionsLoaded();
        const qreal svgSize = 580;
        QColor fillColor = m_fillColor.isValid() ? m_fillColor : Qt::black;
        QSvgRenderer renderer(controllerSvgBytes(fillColor));
        if (!renderer.isValid()) return;
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        renderer.render(painter, r);
        const qreal sx = r.width() / svgSize, sy = r.height() / svgSize;
        QColor highlightBrush = m_highlightColor.isValid() ? m_highlightColor : Qt::white;
        painter->setBrush(highlightBrush);  // track color for buttons/d-pad/triggers
        painter->setPen(Qt::NoPen);
        // D-pad: rectangular base (s_dpadBaseWidth x s_dpadBaseHeight). Up/Down use that rect; Left/Right use same rect rotated 90° (baseHeight along X, baseWidth along Y).
        const qreal halfW = s_dpadBaseWidth / 2.0;
        auto drawDpadHouse = [&](qreal cx, qreal cy, int dir) {
            QPolygonF poly;
            switch (dir) {
            case 0: poly << QPointF(cx, cy + s_dpadTipLength) << QPointF(cx + halfW, cy) << QPointF(cx + halfW, cy - s_dpadBaseHeight) << QPointF(cx - halfW, cy - s_dpadBaseHeight) << QPointF(cx - halfW, cy); break;
            case 1: poly << QPointF(cx, cy - s_dpadTipLength) << QPointF(cx + halfW, cy) << QPointF(cx + halfW, cy + s_dpadBaseHeight) << QPointF(cx - halfW, cy + s_dpadBaseHeight) << QPointF(cx - halfW, cy); break;
            case 2: poly << QPointF(cx + s_dpadTipLength, cy) << QPointF(cx, cy + halfW) << QPointF(cx - s_dpadBaseHeight, cy + halfW) << QPointF(cx - s_dpadBaseHeight, cy - halfW) << QPointF(cx, cy - halfW); break;
            case 3: poly << QPointF(cx - s_dpadTipLength, cy) << QPointF(cx, cy + halfW) << QPointF(cx + s_dpadBaseHeight, cy + halfW) << QPointF(cx + s_dpadBaseHeight, cy - halfW) << QPointF(cx, cy - halfW); break;
            default: return;
            }
            for (int i = 0; i < poly.size(); ++i)
                poly[i] = QPointF(r.x() + poly[i].x() * sx, r.y() + poly[i].y() * sy);
            painter->drawPolygon(poly);
        };
        // Rounded corner radii in SVG space (match controller SVG: triggers rx=6, bumpers rx=4).
        const qreal triggerRx = 6.0, bumperRx = 4.0;
        for (int bi : m_buttons) {
            if (bi < 0 || bi > 15) continue;
            const ControllerRegion &reg = s_controllerRegions.at(bi);
            if (bi == 14 || bi == 15) continue; // triggers drawn below
            if (bi >= 10 && bi <= 13) {
                drawDpadHouse(reg.x, reg.y, bi - 10);
                continue;
            }
            if (reg.r >= 0) {
                painter->drawEllipse(QPointF(r.x() + reg.x * sx, r.y() + reg.y * sy), reg.r * sx, reg.r * sy);
            } else {
                QRectF rectItem(r.x() + reg.x * sx, r.y() + reg.y * sy, reg.w * sx, reg.h * sy);
                qreal rx = qMin(bumperRx * sx, rectItem.width() / 2);
                qreal ry = qMin(bumperRx * sy, rectItem.height() / 2);
                painter->drawRoundedRect(rectItem, rx, ry, Qt::AbsoluteSize);
            }
        }
        // Trigger fills: partial fill from bottom, rounded rect (radius 6 in SVG, capped to fit).
        if (s_controllerRegions.size() > 15) {
            const ControllerRegion &ltReg = s_controllerRegions.at(14);
            const ControllerRegion &rtReg = s_controllerRegions.at(15);
            auto drawTriggerFill = [&](qreal x, qreal y, qreal w, qreal h) {
                if (h <= 0.5) return;
                qreal rx = qMin(triggerRx * sx, qMin(w / 2, h / 2));
                qreal ry = qMin(triggerRx * sy, qMin(w / 2, h / 2));
                painter->drawRoundedRect(QRectF(x, y, w, h), rx, ry, Qt::AbsoluteSize);
            };
            qreal ltH = ltReg.h * sy * m_leftTrigger;
            if (ltH > 0.5) {
                drawTriggerFill(r.x() + ltReg.x * sx, r.y() + (ltReg.y + ltReg.h) * sy - ltH, ltReg.w * sx, ltH);
            }
            qreal rtH = rtReg.h * sy * m_rightTrigger;
            if (rtH > 0.5) {
                drawTriggerFill(r.x() + rtReg.x * sx, r.y() + (rtReg.y + rtReg.h) * sy - rtH, rtReg.w * sx, rtH);
            }
        }
        // Stick indicators: use region 8 and 9 centers; invert Y like AngularViewer (stick up = dot up)
        if (s_controllerRegions.size() >= 10) {
            const ControllerRegion &leftReg = s_controllerRegions.at(8);
            const ControllerRegion &rightReg = s_controllerRegions.at(9);
            qreal leftCx = leftReg.x, leftCy = leftReg.y;
            qreal rightCx = rightReg.x, rightCy = rightReg.y;
            qreal leftInnerR = (leftReg.r >= 0 ? leftReg.r : 14) * s_stickInnerRadiusFraction;
            qreal rightInnerR = (rightReg.r >= 0 ? rightReg.r : 21) * s_stickInnerRadiusFraction;
            qreal lx = m_leftStickX, ly = m_leftStickY;
            qreal n = lx * lx + ly * ly;
            if (n > 1.0 && n > 1e-12) { qreal s = 1.0 / std::sqrt(n); lx *= s; ly *= s; }
            qreal rx = m_rightStickX, ry = m_rightStickY;
            n = rx * rx + ry * ry;
            if (n > 1.0 && n > 1e-12) { qreal s = 1.0 / std::sqrt(n); rx *= s; ry *= s; }
            painter->setBrush(m_highlightColor.isValid() ? m_highlightColor : Qt::white);  // track color for stick dots
            qreal indR = s_stickIndicatorRadiusSvg;
            painter->drawEllipse(QPointF(r.x() + (leftCx + lx * leftInnerR) * sx, r.y() + (leftCy + ly * leftInnerR) * sy), indR * sx, indR * sy);
            painter->drawEllipse(QPointF(r.x() + (rightCx + rx * rightInnerR) * sx, r.y() + (rightCy + ry * rightInnerR) * sy), indR * sx, indR * sy);
        }
    }
private:
    QColor m_fillColor;
    QSet<int> m_buttons;
    qreal m_leftTrigger = 0, m_rightTrigger = 0;
    qreal m_leftStickX = 0, m_leftStickY = 0, m_rightStickX = 0, m_rightStickY = 0;
    QColor m_highlightColor;  // for stick indicator
};
}

KeyboardWidget::KeyboardWidget(QWidget *parent) : QWidget(parent)
    , m_keyColor(0, 0, 255)
    , m_highlightColor(Qt::red)
    , m_backgroundColor(53, 53, 53)
    , m_textColor(Qt::white)
    , m_highlightedTextColor(Qt::black)
{
    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene, this);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setRenderHint(QPainter::TextAntialiasing);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setBackgroundBrush(m_backgroundColor);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);
}

void KeyboardWidget::setKeyColor(const QColor &color)
{
    if (color.isValid())
        m_keyColor = color;
}

void KeyboardWidget::setHighlightColor(const QColor &color)
{
    if (color.isValid())
        m_highlightColor = color;
}

void KeyboardWidget::setBackgroundColor(const QColor &color)
{
    if (color.isValid()) {
        m_backgroundColor = color;
        m_view->setBackgroundBrush(m_backgroundColor);
    }
}

void KeyboardWidget::setTextColor(const QColor &color)
{
    if (color.isValid())
        m_textColor = color;
}

void KeyboardWidget::setHighlightedTextColor(const QColor &color)
{
    if (color.isValid())
        m_highlightedTextColor = color;
}

void KeyboardWidget::setShapeTextColor(QGraphicsItem *shapeItem, const QColor &color)
{
    if (!shapeItem || !color.isValid())
        return;
    for (QGraphicsItem *child : shapeItem->childItems()) {
        QGraphicsTextItem *textItem = qgraphicsitem_cast<QGraphicsTextItem*>(child);
        if (textItem) {
            textItem->setDefaultTextColor(color);
            return;
        }
    }
}

void KeyboardWidget::applyColors()
{
    m_view->setBackgroundBrush(m_backgroundColor);
    // When in editor mode the visualization is hidden; skip scene updates to avoid crashes.
    if (!isVisible())
        return;
    // Apply to all key shapes in the scene (including keys with no keycode)
    for (QGraphicsItem *item : m_scene->items()) {
        if (item->parentItem() != nullptr)
            continue;
        QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem*>(item);
        if (shape) {
            QColor effBrush = item->data(KeyColorRole).value<QColor>();
            QColor effText = item->data(KeyTextColorRole).value<QColor>();
            if (!effBrush.isValid()) effBrush = m_keyColor;
            if (!effText.isValid()) effText = m_textColor;
            shape->setBrush(effBrush);
            setShapeTextColor(shape, effText);
        }
    }
#ifdef Q_OS_WIN
    for (QGraphicsItem *item : m_scene->items()) {
        if (item->parentItem() != nullptr)
            continue;
        QGraphicsPathItem *fillItem = m_analogFillOverlays.value(item, nullptr);
        if (fillItem) {
            QColor c = item->data(KeyColorPressedRole).value<QColor>();
            if (!c.isValid()) c = m_highlightColor;
            fillItem->setBrush(c);
        }
        QGraphicsTextItem *highlightTextItem = m_analogHighlightedTextItems.value(item, nullptr);
        if (highlightTextItem)
            highlightTextItem->setDefaultTextColor(m_highlightedTextColor);
    }
#endif
}

void KeyboardWidget::loadLayout(const QString &fileName, int retryCount)
{
    if (fileName.isEmpty()) {
        m_lastLoadedPath.clear();
        // Clear trigger-fill maps before scene clear so we never hold pointers to deleted items
        m_itemsWithTriggerFill.clear();
        m_analogFillOverlays.clear();
        m_analogOutlineOverlays.clear();
        m_analogOutlineClipParents.clear();
        m_analogTextClipParents.clear();
        m_analogHighlightedTextItems.clear();
        m_scene->clear();
        m_keys.clear();
        keyCounter.clear();
        m_pressedKeys.clear();
        m_triggerValues.clear();
        m_mouseSpeedIndicators.clear();
        m_angularViewers.clear();
        m_controllerOverlays.clear();
        if (m_mouseIndicatorTimer) m_mouseIndicatorTimer->stop();
        update();
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        const int maxRetries = 8;
        if (retryCount < maxRetries) {
            QTimer::singleShot(80, this, [this, fileName, retryCount]() {
                loadLayout(fileName, retryCount + 1);
            });
        }
        return;
    }

    m_lastLoadedPath = fileName;
    QByteArray data = file.readAll();
    file.close();
    applyLayoutData(data);
}

void KeyboardWidget::reloadLayout()
{
    if (!m_lastLoadedPath.isEmpty()) {
        // Clean redraw: clear maps first, then reload from file so we never touch stale scene items.
        loadLayout(m_lastLoadedPath);
    } else {
        applyColors();
    }
}

void KeyboardWidget::loadLayoutFromData(const QByteArray &jsonData)
{
    if (jsonData.isEmpty()) {
        m_lastLoadedPath.clear();
        m_itemsWithTriggerFill.clear();
        m_analogFillOverlays.clear();
        m_analogOutlineOverlays.clear();
        m_analogOutlineClipParents.clear();
        m_analogTextClipParents.clear();
        m_analogHighlightedTextItems.clear();
        m_scene->clear();
        m_keys.clear();
        keyCounter.clear();
        m_pressedKeys.clear();
        m_triggerValues.clear();
        m_mouseSpeedIndicators.clear();
        m_angularViewers.clear();
        m_controllerOverlays.clear();
        m_labelOverlays.clear();
        if (m_mouseIndicatorTimer) m_mouseIndicatorTimer->stop();
        update();
        return;
    }
    m_lastLoadedPath.clear();  // we loaded from data, not from file
    applyLayoutData(jsonData);
}

void KeyboardWidget::applyLayoutData(const QByteArray &jsonData)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError || doc.isNull())
        return;

    QJsonObject rootObject = doc.object();
    elements = rootObject.value("Elements").toArray();

    // Clear trigger-fill maps before scene clear so we never hold pointers to deleted items
    m_itemsWithTriggerFill.clear();
    m_analogFillOverlays.clear();
    m_analogOutlineOverlays.clear();
    m_analogOutlineClipParents.clear();
    m_analogTextClipParents.clear();
    m_analogHighlightedTextItems.clear();
    m_scene->clear();
    m_keys.clear();
    keyCounter.clear();
    m_pressedKeys.clear();
    m_triggerValues.clear();
    m_mouseSpeedIndicators.clear();
    m_angularViewers.clear();
    m_controllerOverlays.clear();
    m_labelOverlays.clear();
    if (m_mouseIndicatorTimer)
        m_mouseIndicatorTimer->stop();

    // Compute bounding box from all elements (same logic as editor) so scene rect matches
    qreal overallMinX = 1e9, overallMinY = 1e9, overallMaxX = -1e9, overallMaxY = -1e9;
    bool hasBounds = false;
    for (const QJsonValue &element : elements) {
        QJsonObject keyData = element.toObject();
        QString type = keyData.value("__type").toString();
        if (type == QLatin1String("KeyboardKey")) {
            QJsonArray boundaries = keyData.value("Boundaries").toArray();
            if (boundaries.size() < 4)
                continue;
            for (const QJsonValue &pv : boundaries) {
                QJsonObject po = pv.toObject();
                qreal px = po["X"].toDouble();
                qreal py = po["Y"].toDouble();
                overallMinX = qMin(overallMinX, px);
                overallMinY = qMin(overallMinY, py);
                overallMaxX = qMax(overallMaxX, px);
                overallMaxY = qMax(overallMaxY, py);
                hasBounds = true;
            }
        } else if (type == QLatin1String("MouseSpeedIndicator")) {
            QJsonObject loc = keyData.value("Location").toObject();
            qreal cx = loc["X"].toDouble();
            qreal cy = loc["Y"].toDouble();
            qreal r = keyData.value("Radius").toDouble(15);
            if (r < 1) r = 1;
            overallMinX = qMin(overallMinX, cx - r);
            overallMinY = qMin(overallMinY, cy - r);
            overallMaxX = qMax(overallMaxX, cx + r);
            overallMaxY = qMax(overallMaxY, cy + r);
            hasBounds = true;
        } else if (type == QLatin1String("AngularViewer")) {
            QJsonArray boundaries = keyData.value("Boundaries").toArray();
            if (boundaries.size() >= 4) {
                for (const QJsonValue &pv : boundaries) {
                    QJsonObject po = pv.toObject();
                    qreal px = po["X"].toDouble();
                    qreal py = po["Y"].toDouble();
                    overallMinX = qMin(overallMinX, px);
                    overallMinY = qMin(overallMinY, py);
                    overallMaxX = qMax(overallMaxX, px);
                    overallMaxY = qMax(overallMaxY, py);
                    hasBounds = true;
                }
            }
        } else if (type == QLatin1String("Label")) {
            qreal lx = keyData.value("X").toDouble();
            qreal ly = keyData.value("Y").toDouble();
            overallMinX = qMin(overallMinX, lx);
            overallMinY = qMin(overallMinY, ly);
            overallMaxX = qMax(overallMaxX, lx + 80);
            overallMaxY = qMax(overallMaxY, ly + 20);
            hasBounds = true;
        } else if (type == QLatin1String("Controller")) {
            QJsonArray boundaries = keyData.value("Boundaries").toArray();
            if (boundaries.size() >= 4) {
                for (const QJsonValue &pv : boundaries) {
                    QJsonObject po = pv.toObject();
                    qreal px = po["X"].toDouble();
                    qreal py = po["Y"].toDouble();
                    overallMinX = qMin(overallMinX, px);
                    overallMinY = qMin(overallMinY, py);
                    overallMaxX = qMax(overallMaxX, px);
                    overallMaxY = qMax(overallMaxY, py);
                    hasBounds = true;
                }
            }
        }
    }

    for (const QJsonValue &element : elements) {
        QJsonObject keyData = element.toObject();
        QString type = keyData.value("__type").toString();
        if (type == QLatin1String("KeyboardKey")) {
            createKey(keyData);
        } else if (type == QLatin1String("MouseSpeedIndicator")) {
            createMouseSpeedIndicatorOverlay(keyData);
        } else if (type == QLatin1String("AngularViewer")) {
            createAngularViewerOverlay(keyData);
        } else if (type == QLatin1String("Label")) {
            createLabelOverlay(keyData);
        } else if (type == QLatin1String("Controller")) {
            createControllerOverlay(keyData);
        }
    }

    if (!m_mouseSpeedIndicators.isEmpty() && !m_mouseIndicatorTimer) {
        m_mouseIndicatorTimer = new QTimer(this);
        connect(m_mouseIndicatorTimer, &QTimer::timeout, this, &KeyboardWidget::updateMouseIndicatorsFromCursor);
    }
    if (m_mouseIndicatorTimer && !m_mouseSpeedIndicators.isEmpty())
        m_mouseIndicatorTimer->start(50);

    if (hasBounds) {
        qreal w = qMax(qreal(1), overallMaxX - overallMinX);
        qreal h = qMax(qreal(1), overallMaxY - overallMinY);
        m_scene->setSceneRect(overallMinX, overallMinY, w, h);
        setMinimumSize(static_cast<int>(w), static_cast<int>(h));
    } else {
        int maxWidth = rootObject.value("Width").toInt();
        int maxHeight = rootObject.value("Height").toInt();
        m_scene->setSceneRect(0, 0, qMax(1, maxWidth), qMax(1, maxHeight));
        setMinimumSize(maxWidth, maxHeight);
    }
    updateLabelsForShiftState();
    updateControllerOverlays();
    m_view->viewport()->update();
    update();
}

void KeyboardWidget::createKey(const QJsonObject &keyData)
{
    QJsonArray boundaries = keyData.value("Boundaries").toArray();
    if (boundaries.size() < 4) {
        qWarning("Invalid boundaries data.");
        return;
    }

    QString label = keyData.value("Text").toString();
    QString shiftText = keyData.value("ShiftText").toString();
    if (shiftText.isEmpty())
        shiftText = label;
    QJsonArray kc = keyData.value("KeyCodes").toArray();
    KeyStyle keyStyle = KeyStyle::fromJson(keyData);
    QColor brushColor = keyStyle.keyColor.isValid() ? keyStyle.keyColor : m_keyColor;
    QColor textCol = keyStyle.keyTextColor.isValid() ? keyStyle.keyTextColor : m_textColor;

    qreal minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
    for (const QJsonValue &pv : boundaries) {
        QJsonObject po = pv.toObject();
        qreal px = po["X"].toDouble();
        qreal py = po["Y"].toDouble();
        minX = qMin(minX, px);
        minY = qMin(minY, py);
        maxX = qMax(maxX, px);
        maxY = qMax(maxY, py);
    }
    qreal w = maxX - minX;
    qreal h = maxY - minY;

    QAbstractGraphicsShapeItem *shapeItem = nullptr;

    if (boundaries.size() == 4) {
        QPolygonF fourPoints;
        for (const QJsonValue &pv : boundaries) {
            QJsonObject po = pv.toObject();
            fourPoints << QPointF(po["X"].toDouble(), po["Y"].toDouble());
        }
        bool isAxisAlignedRect = true;
        for (const QPointF &p : fourPoints) {
            if ((p.x() != minX && p.x() != maxX) || (p.y() != minY && p.y() != maxY)) {
                isAxisAlignedRect = false;
                break;
            }
        }
        if (isAxisAlignedRect && w > 0 && h > 0) {
            if (keyStyle.cornerRadius > 0) {
                QPainterPath path;
                path.addRoundedRect(0, 0, w, h, keyStyle.cornerRadius, keyStyle.cornerRadius);
                QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);
                pathItem->setPos(minX, minY);
                pathItem->setBrush(brushColor);
                pathItem->setPen(keyStyle.pen());
                m_scene->addItem(pathItem);
                shapeItem = pathItem;
            } else {
                QGraphicsRectItem *rect = new QGraphicsRectItem(0, 0, w, h);
                rect->setPos(minX, minY);
                rect->setBrush(brushColor);
                rect->setPen(keyStyle.pen());
                m_scene->addItem(rect);
                shapeItem = rect;
            }
        }
    }

    QString shapeType = keyData.value("ShapeType").toString();
    if (!shapeItem && (shapeType == "ellipse" || (shapeType.isEmpty() && boundaries.size() >= 32))) {
        QGraphicsEllipseItem *ellipse = new QGraphicsEllipseItem(0, 0, w, h);
        ellipse->setPos(minX, minY);
        ellipse->setBrush(brushColor);
        ellipse->setPen(keyStyle.pen());
        m_scene->addItem(ellipse);
        shapeItem = ellipse;
    }

    if (!shapeItem) {
        QPolygonF poly;
        for (const QJsonValue &pv : boundaries) {
            QJsonObject po = pv.toObject();
            qreal px = po["X"].toDouble();
            qreal py = po["Y"].toDouble();
            poly << QPointF(px - minX, py - minY);
        }
        QList<QPolygonF> holesList;
        if (keyData.contains("Holes")) {
            QJsonArray holesArray = keyData["Holes"].toArray();
            for (const QJsonValue &holeVal : holesArray) {
                QJsonArray holeArr = holeVal.toArray();
                QPolygonF holePoly;
                for (const QJsonValue &pv : holeArr) {
                    QJsonObject po = pv.toObject();
                    holePoly << QPointF(po["X"].toDouble() - minX, po["Y"].toDouble() - minY);
                }
                if (holePoly.size() >= 3)
                    holesList.append(holePoly);
            }
        }
        if (!holesList.isEmpty()) {
            QPainterPath path;
            path.addPolygon(poly);
            for (const QPolygonF &hole : holesList) {
                QPainterPath holePath;
                holePath.addPolygon(hole);
                path = path.subtracted(holePath);
            }
            QGraphicsPathItem *pathItem = new QGraphicsPathItem(path);
            pathItem->setPos(minX, minY);
            pathItem->setBrush(brushColor);
            pathItem->setPen(keyStyle.pen());
            m_scene->addItem(pathItem);
            shapeItem = pathItem;
        } else {
            QGraphicsPolygonItem *polyItem = new QGraphicsPolygonItem(poly);
            polyItem->setPos(minX, minY);
            polyItem->setBrush(brushColor);
            polyItem->setPen(keyStyle.pen());
            m_scene->addItem(polyItem);
            shapeItem = polyItem;
        }
    }

    if (!shapeItem)
        return;

    shapeItem->setData(BaseTextRole, label);
    shapeItem->setData(ShiftTextRole, shiftText);
    if (keyStyle.keyColor.isValid())
        shapeItem->setData(KeyColorRole, keyStyle.keyColor);
    if (keyStyle.keyColorPressed.isValid())
        shapeItem->setData(KeyColorPressedRole, keyStyle.keyColorPressed);
    if (keyStyle.keyTextColor.isValid())
        shapeItem->setData(KeyTextColorRole, keyStyle.keyTextColor);
    if (keyStyle.keyTextColorPressed.isValid())
        shapeItem->setData(KeyTextColorPressedRole, keyStyle.keyTextColorPressed);

    shapeItem->setBrush(brushColor);

    const int textAlign = keyStyle.textAlignment;
    shapeItem->setData(TextAlignmentRole, textAlign);

    QGraphicsTextItem *textItem = new QGraphicsTextItem(shapeItem);
    textItem->document()->setDocumentMargin(0);
    QTextOption opt;
    opt.setAlignment(alignmentFromInt(textAlign));
    textItem->document()->setDefaultTextOption(opt);
    textItem->setPlainText(label);
    textItem->setDefaultTextColor(textCol);
    textItem->setFont(keyStyle.font());
    QRectF shapeBr = shapeItem->boundingRect();
    qreal docWidth = qMax(0.0, shapeBr.width() - 8);
    if (keyStyle.fontItalic)
        docWidth += 10;  // extra so italic slant isn't clipped when right-aligned
    textItem->setTextWidth(docWidth);
    QRectF textBr = textItem->boundingRect();
    QPointF anchorLocal = shapeBr.center();
    if (keyData.contains("TextPosition")) {
        QJsonObject tp = keyData["TextPosition"].toObject();
        qreal tpX = tp["X"].toDouble();
        qreal tpY = tp["Y"].toDouble();
        anchorLocal = shapeItem->mapFromScene(QPointF(tpX, tpY));
        shapeItem->setData(TextPositionLocalRole, anchorLocal);
    }
    // Anchor is the edit-shape truth: center=center, left=left edge, right=right edge
    qreal textX = (textAlign == 0) ? anchorLocal.x()
                 : (textAlign == 2) ? (anchorLocal.x() - textBr.width())
                 : (anchorLocal.x() - textBr.width() / 2);
    textItem->setPos(textX, anchorLocal.y() - textBr.height() / 2);
    textItem->setZValue(1);

    QList<int> codeList;
    QList<int> triggerCodeList;
    for (int i = 0; i < kc.size(); ++i) {
        int code = static_cast<int>(kc[i].toDouble(0));
        if (code == 0)
            continue;
        m_keys[code].push_back(shapeItem);
        codeList.append(code);
#ifdef Q_OS_WIN
        if (isGamepadTriggerCode(code))
            triggerCodeList.append(code);
#endif
    }
    shapeItem->setData(KeyCodesRole, QVariant::fromValue(codeList));
    shapeItem->setData(TriggerCodesRole, QVariant::fromValue(triggerCodeList));

    if (!triggerCodeList.isEmpty()) {
        m_itemsWithTriggerFill.insert(shapeItem);
        QGraphicsPathItem *fillPathItem = new QGraphicsPathItem(shapeItem);
        fillPathItem->setBrush(shapeItem->data(KeyColorPressedRole).value<QColor>().isValid()
                               ? shapeItem->data(KeyColorPressedRole).value<QColor>()
                               : m_highlightColor);
        fillPathItem->setPen(Qt::NoPen);  // no pen on fill so the horizontal "top cut" doesn't get a visible edge
        fillPathItem->setZValue(0.5);
        fillPathItem->setPos(0, 0);
        m_analogFillOverlays[shapeItem] = fillPathItem;

        QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem*>(shapeItem);
        if (shape && shape->pen().style() != Qt::NoPen) {
            QGraphicsPathItem *clipParent = new QGraphicsPathItem(shapeItem);
            clipParent->setPen(Qt::NoPen);
            clipParent->setBrush(Qt::NoBrush);
            clipParent->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
            clipParent->setZValue(0.55);
            clipParent->setPos(0, 0);
            QGraphicsPathItem *outlinePathItem = new QGraphicsPathItem(clipParent);
            outlinePathItem->setBrush(Qt::NoBrush);
            outlinePathItem->setPen(shape->pen());
            outlinePathItem->setPath(shapePathFromItem(shapeItem));
            outlinePathItem->setPos(0, 0);
            m_analogOutlineOverlays[shapeItem] = outlinePathItem;
            m_analogOutlineClipParents[shapeItem] = clipParent;
        }
        // Highlighted text clipped to fill region (half-and-half effect when trigger is partial)
        QGraphicsPathItem *textClipParent = new QGraphicsPathItem(shapeItem);
        textClipParent->setPen(Qt::NoPen);
        textClipParent->setBrush(Qt::NoBrush);
        textClipParent->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
        textClipParent->setZValue(1.05);
        textClipParent->setPos(0, 0);
        QGraphicsTextItem *highlightedTextItem = new QGraphicsTextItem(textClipParent);
        highlightedTextItem->document()->setDocumentMargin(textItem->document()->documentMargin());
        highlightedTextItem->document()->setDefaultTextOption(textItem->document()->defaultTextOption());
        highlightedTextItem->setPlainText(textItem->toPlainText());
        highlightedTextItem->setDefaultTextColor(m_highlightedTextColor);
        highlightedTextItem->setFont(textItem->font());
        highlightedTextItem->setTextWidth(textItem->textWidth());
        highlightedTextItem->setPos(textItem->pos());
        highlightedTextItem->setZValue(0);
        m_analogTextClipParents[shapeItem] = textClipParent;
        m_analogHighlightedTextItems[shapeItem] = highlightedTextItem;
    }
    keyCounter[label] = 0;
}

void KeyboardWidget::createMouseSpeedIndicatorOverlay(const QJsonObject &keyData)
{
    QJsonObject loc = keyData.value("Location").toObject();
    qreal cx = loc["X"].toDouble();
    qreal cy = loc["Y"].toDouble();
    qreal r = keyData.value("Radius").toDouble(15);
    if (r < 1) r = 1;
    KeyStyle keyStyle = KeyStyle::fromJson(keyData);

    QGraphicsEllipseItem *track = new QGraphicsEllipseItem(cx - r, cy - r, 2 * r, 2 * r);
    track->setBrush(Qt::NoBrush);
    track->setPen(keyStyle.pen());
    track->setZValue(0);
    m_scene->addItem(track);

    const qreal indR = 5;
    QGraphicsEllipseItem *indicator = new QGraphicsEllipseItem(-indR, -indR, 2 * indR, 2 * indR);
    indicator->setBrush(keyStyle.keyColor.isValid() ? keyStyle.keyColor : m_keyColor);
    indicator->setPen(keyStyle.pen());
    indicator->setPos(cx, cy);
    indicator->setZValue(1);
    m_scene->addItem(indicator);

    MouseSpeedIndicatorOverlay entry;
    entry.centerX = cx;
    entry.centerY = cy;
    entry.radius = r;
    entry.trackItem = track;
    entry.indicatorItem = indicator;
    m_mouseSpeedIndicators.append(entry);
}

void KeyboardWidget::createAngularViewerOverlay(const QJsonObject &keyData)
{
    QJsonArray boundaries = keyData.value("Boundaries").toArray();
    if (boundaries.size() < 4) return;
    qreal minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
    for (const QJsonValue &pv : boundaries) {
        QJsonObject po = pv.toObject();
        qreal px = po["X"].toDouble();
        qreal py = po["Y"].toDouble();
        minX = qMin(minX, px);
        minY = qMin(minY, py);
        maxX = qMax(maxX, px);
        maxY = qMax(maxY, py);
    }
    qreal w = qMax(qreal(1), maxX - minX);
    qreal h = qMax(qreal(1), maxY - minY);
    QString subTypeStr = keyData.value("SubType").toString();
    bool isLeftStick = (subTypeStr != QLatin1String("rightStick"));
    int controllerIndex = keyData.value("ControllerIndex").toInt(0);
    controllerIndex = qBound(0, controllerIndex, 3);
    bool flipX = keyData.value("FlipX").toBool(false);
    bool flipY = keyData.value("FlipY").toBool(true);
    KeyStyle keyStyle = KeyStyle::fromJson(keyData);

    QGraphicsEllipseItem *track = new QGraphicsEllipseItem(0, 0, w, h);
    track->setPos(minX, minY);
    track->setBrush(Qt::NoBrush);
    track->setPen(keyStyle.pen());
    track->setZValue(0);
    m_scene->addItem(track);

    const qreal indR = 5;
    QGraphicsEllipseItem *indicator = new QGraphicsEllipseItem(-indR, -indR, 2 * indR, 2 * indR);
    indicator->setBrush(keyStyle.keyColor.isValid() ? keyStyle.keyColor : m_keyColor);
    indicator->setPen(keyStyle.pen());
    indicator->setPos(minX + w / 2, minY + h / 2);
    indicator->setZValue(1);
    m_scene->addItem(indicator);

    AngularViewerOverlay entry;
    entry.controllerIndex = controllerIndex;
    entry.isLeftStick = isLeftStick;
    entry.flipX = flipX;
    entry.flipY = flipY;
    entry.rect = QRectF(minX, minY, w, h);
    entry.trackItem = track;
    entry.indicatorItem = indicator;
    m_angularViewers.append(entry);
}

void KeyboardWidget::createControllerOverlay(const QJsonObject &keyData)
{
    QJsonArray boundaries = keyData.value("Boundaries").toArray();
    if (boundaries.size() < 4) return;
    qreal minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
    for (const QJsonValue &pv : boundaries) {
        QJsonObject po = pv.toObject();
        qreal px = po["X"].toDouble();
        qreal py = po["Y"].toDouble();
        minX = qMin(minX, px);
        minY = qMin(minY, py);
        maxX = qMax(maxX, px);
        maxY = qMax(maxY, py);
    }
    qreal w = qMax(qreal(1), maxX - minX);
    qreal h = qMax(qreal(1), maxY - minY);
    int controllerIndex = keyData.value("ControllerIndex").toInt(0);
    controllerIndex = qBound(0, controllerIndex, 3);

    KeyStyle keyStyle = KeyStyle::fromJson(keyData);
    // Outline color = controller body (SVG fill). Track color (keyColor) = highlights (pressed states).
    QColor fillColor = keyStyle.outlineColor.isValid() ? keyStyle.outlineColor : m_keyColor;
    QColor highlightColor = keyStyle.keyColor.isValid() ? keyStyle.keyColor : fillColor;
    ControllerViewItem *item = new ControllerViewItem(QRectF(0, 0, w, h));
    item->setFillColor(fillColor);
    item->setStickIndicatorColor(highlightColor);
    item->setPos(minX, minY);
    item->setZValue(0.5);
    m_scene->addItem(item);

    ControllerOverlay entry;
    entry.item = item;
    entry.controllerIndex = controllerIndex;
    m_controllerOverlays.append(entry);
}

void KeyboardWidget::createLabelOverlay(const QJsonObject &keyData)
{
    qreal x = keyData.value("X").toDouble();
    qreal y = keyData.value("Y").toDouble();
    QString text = keyData.value("Text").toString();
    QString shiftText = keyData.value("ShiftText").toString();
    if (shiftText.isEmpty())
        shiftText = text;
    KeyStyle keyStyle = KeyStyle::fromJson(keyData);
    QColor textColor = keyStyle.keyTextColor.isValid() ? keyStyle.keyTextColor : m_textColor;
    QGraphicsTextItem *label = new QGraphicsTextItem(text);
    QTextOption labelOpt;
    labelOpt.setAlignment(alignmentFromInt(keyStyle.textAlignment));
    label->document()->setDefaultTextOption(labelOpt);
    label->document()->setDocumentMargin(2);  // match LabelItem
    if (keyStyle.fontPointSize > 0 || !keyStyle.fontFamily.isEmpty())
        label->setFont(keyStyle.font());
    label->setDefaultTextColor(textColor);
    label->setZValue(0);
    m_scene->addItem(label);
    QRectF br = label->boundingRect();
    qreal wBase = br.width();
    qreal w = wBase;
    if (!shiftText.isEmpty() && shiftText != text) {
        label->setPlainText(shiftText);
        qreal wShift = label->boundingRect().width();
        label->setPlainText(text);
        w = qMax(wBase, wShift);
    }
    if (w > 0) {
        label->document()->setTextWidth(2 * w);  // fixed width so shift toggle doesn't jump
    }
    br = label->boundingRect();
    qreal docW = br.width();
    qreal docH = br.height();
    qreal px = (keyStyle.textAlignment == 0) ? x : (keyStyle.textAlignment == 2) ? (x - docW) : (x - docW / 2);
    qreal py = y - docH / 2;
    label->setPos(px, py);  // (X,Y) = anchor (left/center/right by alignment), match editor
    LabelOverlay entry;
    entry.textItem = label;
    entry.baseText = text;
    entry.shiftText = shiftText;
    entry.anchorX = x;
    entry.anchorY = y;
    entry.labelWidth = w;
    entry.textAlignment = keyStyle.textAlignment;
    m_labelOverlays.append(entry);
}

void KeyboardWidget::updateMouseIndicatorsFromCursor()
{
    if (m_mouseSpeedIndicators.isEmpty() || !m_view) return;
    QPoint globalPos = QCursor::pos();
    QPoint viewPos = m_view->mapFromGlobal(globalPos);
    QPointF scenePos = m_view->mapToScene(viewPos);
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (MouseSpeedIndicatorOverlay &entry : m_mouseSpeedIndicators) {
        if (!entry.indicatorItem) continue;
        QPointF speed(0, 0);
        if (entry.lastTime > 0) {
            qint64 dt = now - entry.lastTime;
            if (dt >= 30) {
                qreal dx = scenePos.x() - entry.lastScenePos.x();
                qreal dy = scenePos.y() - entry.lastScenePos.y();
                qreal scale = 50.0 / qMax(qint64(1), dt);
                speed.setX(dx * scale);
                speed.setY(dy * scale);
            }
        }
        entry.lastScenePos = scenePos;
        entry.lastTime = now;

        entry.speedHistory.append(speed);
        while (entry.speedHistory.size() > MouseSpeedIndicatorOverlay::kMouseSmooth)
            entry.speedHistory.removeFirst();
        QPointF avg(0, 0);
        for (const QPointF &v : entry.speedHistory) {
            avg.setX(avg.x() + v.x());
            avg.setY(avg.y() + v.y());
        }
        int n = entry.speedHistory.size();
        if (n > 0) {
            avg.setX(avg.x() / n);
            avg.setY(avg.y() / n);
        }
        qreal mag = std::sqrt(avg.x() * avg.x() + avg.y() * avg.y());
        const qreal pixelsPerFrameForFullRadius = 80.0;
        qreal r = qMin(mag / pixelsPerFrameForFullRadius, 1.0) * entry.radius;
        qreal dx = 0, dy = 0;
        if (mag > 1e-6) {
            dx = (avg.x() / mag) * r;
            dy = (avg.y() / mag) * r;
        }
        entry.indicatorItem->setPos(entry.centerX + dx, entry.centerY + dy);
    }
}

void KeyboardWidget::onLeftStickChanged(int controllerIndex, qreal x, qreal y)
{
    for (const AngularViewerOverlay &entry : m_angularViewers) {
        if (entry.controllerIndex != controllerIndex || !entry.isLeftStick || !entry.indicatorItem) continue;
        qreal sx = entry.flipX ? -x : x;
        qreal sy = entry.flipY ? -y : y;
        qreal hw = entry.rect.width() / 2;
        qreal hh = entry.rect.height() / 2;
        qreal ox = sx * hw;
        qreal oy = -sy * hh;
        if (hw > 1e-6 && hh > 1e-6) {
            qreal n = (ox * ox) / (hw * hw) + (oy * oy) / (hh * hh);
            if (n > 1.0) {
                qreal s = 1.0 / std::sqrt(n);
                ox *= s;
                oy *= s;
            }
        }
        qreal cx = entry.rect.x() + entry.rect.width() / 2;
        qreal cy = entry.rect.y() + entry.rect.height() / 2;
        entry.indicatorItem->setPos(cx + ox, cy + oy);
    }
    for (const ControllerOverlay &overlay : m_controllerOverlays) {
        if (overlay.controllerIndex != controllerIndex) continue;
        ControllerViewItem *ctrlItem = qgraphicsitem_cast<ControllerViewItem*>(overlay.item);
        if (ctrlItem) ctrlItem->setLeftStick(x, y);
    }
}

void KeyboardWidget::onRightStickChanged(int controllerIndex, qreal x, qreal y)
{
    for (const AngularViewerOverlay &entry : m_angularViewers) {
        if (entry.controllerIndex != controllerIndex || entry.isLeftStick || !entry.indicatorItem) continue;
        qreal sx = entry.flipX ? -x : x;
        qreal sy = entry.flipY ? -y : y;
        qreal hw = entry.rect.width() / 2;
        qreal hh = entry.rect.height() / 2;
        qreal ox = sx * hw;
        qreal oy = -sy * hh;
        if (hw > 1e-6 && hh > 1e-6) {
            qreal n = (ox * ox) / (hw * hw) + (oy * oy) / (hh * hh);
            if (n > 1.0) {
                qreal s = 1.0 / std::sqrt(n);
                ox *= s;
                oy *= s;
            }
        }
        qreal cx = entry.rect.x() + entry.rect.width() / 2;
        qreal cy = entry.rect.y() + entry.rect.height() / 2;
        entry.indicatorItem->setPos(cx + ox, cy + oy);
    }
    for (const ControllerOverlay &overlay : m_controllerOverlays) {
        if (overlay.controllerIndex != controllerIndex) continue;
        ControllerViewItem *ctrlItem = qgraphicsitem_cast<ControllerViewItem*>(overlay.item);
        if (ctrlItem) ctrlItem->setRightStick(x, y);
    }
}

void KeyboardWidget::changeKeyColor(const int &keyCode, const QColor &brushColor, const QColor &textColor, bool isPressed)
{
    auto it = m_keys.find(keyCode);
    if (it == m_keys.end())
        return;
    for (QGraphicsItem *item : it->second) {
        if (!item)
            continue;
        QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem*>(item);
        if (shape) {
            QColor effBrush = isPressed ? item->data(KeyColorPressedRole).value<QColor>() : item->data(KeyColorRole).value<QColor>();
            QColor effText = isPressed ? item->data(KeyTextColorPressedRole).value<QColor>() : item->data(KeyTextColorRole).value<QColor>();
            if (!effBrush.isValid()) effBrush = brushColor;
            if (!effText.isValid()) effText = textColor;
            shape->setBrush(effBrush);
            setShapeTextColor(shape, effText);
        }
    }
}

void KeyboardWidget::resetCounter()
{
    for (auto &p : keyCounter)
        p.second = 0;
}

void KeyboardWidget::onKeyPressed(int key)
{
#ifdef Q_OS_WIN
    if (!isGamepadTriggerCode(key)) {
        m_pressedKeys.insert(key);
        changeKeyColor(key, m_highlightColor, m_highlightedTextColor, true);
    }
    updateTriggerFills();
    updateControllerOverlays();
#else
    changeKeyColor(key, m_highlightColor, m_highlightedTextColor, true);
#endif
}

void KeyboardWidget::onKeyReleased(int key)
{
#ifdef Q_OS_WIN
    if (!isGamepadTriggerCode(key)) {
        m_pressedKeys.remove(key);
        changeKeyColor(key, m_keyColor, m_textColor, false);
    }
    updateTriggerFills();
    updateControllerOverlays();
#else
    changeKeyColor(key, m_keyColor, m_textColor, false);
#endif
}

void KeyboardWidget::onTriggersChanged(int controllerIndex, qreal leftTrigger, qreal rightTrigger)
{
#ifdef Q_OS_WIN
    m_triggerValues[gamepadCode(controllerIndex, GAMEPAD_LEFT_TRIGGER_BUTTON)] = leftTrigger;
    m_triggerValues[gamepadCode(controllerIndex, GAMEPAD_RIGHT_TRIGGER_BUTTON)] = rightTrigger;
    updateTriggerFills();
    updateControllerOverlays();
#endif
}

void KeyboardWidget::updateControllerOverlays()
{
    for (ControllerOverlay &overlay : m_controllerOverlays) {
        QSet<int> buttons;
        qreal lt = 0, rt = 0;
#ifdef Q_OS_WIN
        for (int code : m_pressedKeys) {
            if (gamepadControllerIndex(code) == overlay.controllerIndex) {
                int bi = gamepadButtonIndex(code);
                if (bi >= 0 && bi <= 15)
                    buttons.insert(bi);
            }
        }
        lt = m_triggerValues.value(gamepadCode(overlay.controllerIndex, GAMEPAD_LEFT_TRIGGER_BUTTON), 0);
        rt = m_triggerValues.value(gamepadCode(overlay.controllerIndex, GAMEPAD_RIGHT_TRIGGER_BUTTON), 0);
#endif
        ControllerViewItem *item = qgraphicsitem_cast<ControllerViewItem*>(overlay.item);
        if (item)
            item->setHighlightState(buttons, lt, rt);
    }
}

void KeyboardWidget::updateTriggerFills()
{
#ifdef Q_OS_WIN
    if (!isVisible())
        return;
    for (QGraphicsItem *item : m_scene->items()) {
        if (item->parentItem() != nullptr)
            continue;
        if (!m_itemsWithTriggerFill.contains(item) || !m_analogFillOverlays.contains(item))
            continue;
        QList<int> triggerCodes = item->data(TriggerCodesRole).value<QList<int>>();
        QList<int> keyCodes = item->data(KeyCodesRole).value<QList<int>>();
        qreal effective = 0.0;
        for (int code : triggerCodes) {
            if (m_triggerValues.contains(code))
                effective = qMax(effective, m_triggerValues[code]);
        }
        for (int code : keyCodes) {
            if (!isGamepadTriggerCode(code) && m_pressedKeys.contains(code)) {
                effective = 1.0;
                break;
            }
        }
        QGraphicsPathItem *fillPathItem = m_analogFillOverlays.value(item);
        if (!fillPathItem)
            continue;
        effective = qBound(0.0, effective, 1.0);
        QPainterPath shapePath = shapePathFromItem(item);
        QRectF pathBr = shapePath.boundingRect();
        qreal w = pathBr.width();
        qreal h = pathBr.height();
        if (w < 1e-6 || h < 1e-6)
            continue;
        QRectF bandRect(pathBr.x(), pathBr.bottom() - h * effective, w, h * effective);

        QPainterPath fillPath;
        QGraphicsPolygonItem *polyItem = qgraphicsitem_cast<QGraphicsPolygonItem*>(item);
        if (polyItem) {
            // Use explicit polygon-rect clip for polygons so trapezoids (e.g. Fine#2) don't get wrong fill from QPainterPath::intersected().
            QPolygonF clipped = clipPolygonToRect(polyItem->polygon(), bandRect);
            if (clipped.size() >= 3) {
                fillPath.addPolygon(clipped);
                fillPath.setFillRule(Qt::WindingFill);
            }
        } else {
            QPainterPath bottomRectPath;
            bottomRectPath.addRect(bandRect);
            bottomRectPath.setFillRule(Qt::WindingFill);
            fillPath = shapePath.intersected(bottomRectPath);
            fillPath.setFillRule(Qt::WindingFill);
            if (!fillPath.isEmpty()) {
                QPainterPath simplified = fillPath.simplified();
                if (!simplified.isEmpty())
                    fillPath = simplified;
            }
        }
        fillPathItem->setPath(fillPath);
        const bool showFill = effective > 0.0 && !fillPath.isEmpty();
        fillPathItem->setVisible(showFill);

        QGraphicsPathItem *textClipParent = m_analogTextClipParents.value(item);
        QGraphicsTextItem *highlightedTextItem = m_analogHighlightedTextItems.value(item);
        if (textClipParent && highlightedTextItem) {
            textClipParent->setVisible(showFill);
            if (showFill) {
                textClipParent->setPath(fillPath);
            }
        }

        QGraphicsPathItem *outlinePathItem = m_analogOutlineOverlays.value(item);
        QGraphicsPathItem *outlineClipParent = m_analogOutlineClipParents.value(item);
        if (outlinePathItem && outlineClipParent) {
            outlineClipParent->setVisible(showFill);
            if (showFill) {
                qreal penW = outlinePathItem->pen().widthF();
                if (penW <= 0.0) penW = 1.0;
                // Clip to a path that follows the fill/shape (stroked), not a rectangle, so angled
                // edges (e.g. trapezoid) are never cut off.
                QPainterPath clipPath = (effective >= 1.0)
                    ? strokedPathForClip(shapePath, penW + 1.0)   // full fill: clip to stroked shape + margin
                    : strokedPathForClip(fillPath, penW);         // partial: clip to stroked fill region
                outlineClipParent->setPath(clipPath);
            }
        }
    }
#endif
}

void KeyboardWidget::setLabelMode(LabelMode mode)
{
    m_labelMode = mode;
    updateLabelsForShiftState();
}

void KeyboardWidget::setShiftState(bool pressed)
{
    m_shiftPressed = pressed;
    updateLabelsForShiftState();
}

void KeyboardWidget::setCapsLockState(bool on)
{
    m_capsLockOn = on;
    updateLabelsForShiftState();
}

void KeyboardWidget::updateLabelsForShiftState()
{
    bool useShiftLabel = false;
    switch (m_labelMode) {
    case LabelMode::FollowCapsAndShift:
        useShiftLabel = (m_shiftPressed != m_capsLockOn);
        break;
    case LabelMode::AllUppercase:
        useShiftLabel = true;
        break;
    case LabelMode::AllLowercase:
        useShiftLabel = false;
        break;
    }

    for (QGraphicsItem *item : m_scene->items()) {
        if (item->parentItem() != nullptr)
            continue;
        QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem*>(item);
        if (shape) {
            QString baseText = shape->data(BaseTextRole).toString();
            QString shiftText = shape->data(ShiftTextRole).toString();
            if (baseText.isNull() && shiftText.isNull())
                continue;
            if (shiftText.isEmpty())
                shiftText = baseText;
            if (baseText.isEmpty())
                baseText = shiftText;
            QString displayText = useShiftLabel ? shiftText : baseText;
            for (QGraphicsItem *child : shape->childItems()) {
                QGraphicsTextItem *textItem = qgraphicsitem_cast<QGraphicsTextItem*>(child);
                if (textItem) {
                    textItem->setPlainText(displayText);
                    QVariant alignV = shape->data(TextAlignmentRole);
                    int align = alignV.isValid() ? qBound(0, alignV.toInt(), 2) : 1;
                    QTextOption opt = textItem->document()->defaultTextOption();
                    opt.setAlignment(alignmentFromInt(align));
                    textItem->document()->setDefaultTextOption(opt);
                    QRectF textBr = textItem->boundingRect();
                    QRectF shapeBr = shape->boundingRect();
                    QPointF anchor = shapeBr.center();
                    QVariant v = shape->data(TextPositionLocalRole);
                    if (v.isValid() && v.canConvert<QPointF>()) {
                        QPointF customPos = v.toPointF();
                        if (QPointF(customPos - shapeBr.center()).manhattanLength() > 0.5)
                            anchor = customPos;
                    }
                    // Anchor = edit-shape truth: center=center, left=left edge, right=right edge
                    qreal textX = (align == 0) ? anchor.x()
                                 : (align == 2) ? (anchor.x() - textBr.width())
                                 : (anchor.x() - textBr.width() / 2);
                    textItem->setPos(textX, anchor.y() - textBr.height() / 2);
                    break;
                }
            }
            continue;
        }
    }
    for (const LabelOverlay &overlay : m_labelOverlays) {
        if (!overlay.textItem)
            continue;
        QString baseText = overlay.baseText;
        QString shiftText = overlay.shiftText.isEmpty() ? overlay.baseText : overlay.shiftText;
        QString displayText = useShiftLabel ? shiftText : baseText;
        overlay.textItem->setPlainText(displayText);
        QTextOption labelOpt = overlay.textItem->document()->defaultTextOption();
        labelOpt.setAlignment(alignmentFromInt(overlay.textAlignment));
        overlay.textItem->document()->setDefaultTextOption(labelOpt);
        qreal w = overlay.labelWidth;
        if (w > 0) {
            overlay.textItem->document()->setTextWidth(2 * w);
        }
        QRectF br = overlay.textItem->boundingRect();
        qreal docW = br.width();
        qreal docH = br.height();
        qreal px = (overlay.textAlignment == 0) ? overlay.anchorX : (overlay.textAlignment == 2) ? (overlay.anchorX - docW) : (overlay.anchorX - docW / 2);
        qreal py = overlay.anchorY - docH / 2;
        overlay.textItem->setPos(px, py);  // anchor = single point of truth
    }
}
