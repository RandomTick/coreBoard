#include "KeyboardWidget.h"
#include "keystyle.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
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
const int KeyColorPressedRole = Qt::UserRole + 3;
const int KeyTextColorRole = Qt::UserRole + 4;
const int KeyTextColorPressedRole = Qt::UserRole + 5;
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
}

void KeyboardWidget::loadLayout(const QString &fileName, int retryCount)
{
    if (fileName.isEmpty()) {
        m_scene->clear();
        m_keys.clear();
        keyCounter.clear();
        m_mouseSpeedIndicators.clear();
        m_angularViewers.clear();
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

    QByteArray data = file.readAll();
    file.close();
    applyLayoutData(data);
}

void KeyboardWidget::loadLayoutFromData(const QByteArray &jsonData)
{
    if (jsonData.isEmpty()) {
        m_scene->clear();
        m_keys.clear();
        keyCounter.clear();
        m_mouseSpeedIndicators.clear();
        m_angularViewers.clear();
        if (m_mouseIndicatorTimer) m_mouseIndicatorTimer->stop();
        update();
        return;
    }
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

    m_scene->clear();
    m_keys.clear();
    keyCounter.clear();
    m_mouseSpeedIndicators.clear();
    m_angularViewers.clear();
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

    QGraphicsTextItem *textItem = new QGraphicsTextItem(shapeItem);
    textItem->document()->setDocumentMargin(0);
    QTextOption opt;
    opt.setAlignment(Qt::AlignHCenter);
    textItem->document()->setDefaultTextOption(opt);
    textItem->setPlainText(label);
    textItem->setDefaultTextColor(textCol);
    textItem->setFont(keyStyle.font());
    QRectF shapeBr = shapeItem->boundingRect();
    textItem->setTextWidth(qMax(0.0, shapeBr.width() - 8));
    QRectF textBr = textItem->boundingRect();
    QPointF textCenterLocal = shapeBr.center();
    if (keyData.contains("TextPosition")) {
        QJsonObject tp = keyData["TextPosition"].toObject();
        qreal tpX = tp["X"].toDouble();
        qreal tpY = tp["Y"].toDouble();
        textCenterLocal = shapeItem->mapFromScene(QPointF(tpX, tpY));
        shapeItem->setData(TextPositionLocalRole, textCenterLocal);
    }
    textItem->setPos(textCenterLocal.x() - textBr.width() / 2, textCenterLocal.y() - textBr.height() / 2);
    textItem->setZValue(1);

    for (int i = 0; i < kc.size(); ++i) {
        int code = static_cast<int>(kc[i].toDouble(0));
        // Skip invalid/empty keycode (0); otherwise keys with no keycode show as pressed
        if (code != 0)
            m_keys[code].push_back(shapeItem);
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

void KeyboardWidget::createLabelOverlay(const QJsonObject &keyData)
{
    qreal x = keyData.value("X").toDouble();
    qreal y = keyData.value("Y").toDouble();
    QString text = keyData.value("Text").toString();
    KeyStyle keyStyle = KeyStyle::fromJson(keyData);
    QColor textColor = keyStyle.keyTextColor.isValid() ? keyStyle.keyTextColor : m_textColor;
    QGraphicsTextItem *label = new QGraphicsTextItem(text);
    label->setPos(x, y);
    if (keyStyle.fontPointSize > 0 || !keyStyle.fontFamily.isEmpty())
        label->setFont(keyStyle.font());
    label->setDefaultTextColor(textColor);
    label->setZValue(0);
    m_scene->addItem(label);
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
    changeKeyColor(key, m_highlightColor, m_highlightedTextColor, true);
}

void KeyboardWidget::onKeyReleased(int key)
{
    changeKeyColor(key, m_keyColor, m_textColor, false);
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
        if (!shape)
            continue;
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
                QRectF textBr = textItem->boundingRect();
                QRectF shapeBr = shape->boundingRect();
                QPointF anchor = shapeBr.center();
                QVariant v = shape->data(TextPositionLocalRole);
                if (v.isValid() && v.canConvert<QPointF>()) {
                    QPointF customPos = v.toPointF();
                    if (QPointF(customPos - shapeBr.center()).manhattanLength() > 0.5)
                        anchor = customPos;
                }
                textItem->setPos(anchor.x() - textBr.width() / 2, anchor.y() - textBr.height() / 2);
                break;
            }
        }
    }
}
