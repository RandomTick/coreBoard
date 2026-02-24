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
#include <cmath>

namespace {
const int BaseTextRole = Qt::UserRole;
const int ShiftTextRole = Qt::UserRole + 1;
const int KeyColorRole = Qt::UserRole + 2;
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

    // Compute bounding box from all elements (same logic as editor) so scene rect matches
    qreal overallMinX = 1e9, overallMinY = 1e9, overallMaxX = -1e9, overallMaxY = -1e9;
    bool hasBounds = false;
    for (const QJsonValue &element : elements) {
        QJsonObject keyData = element.toObject();
        if (keyData.value("__type").toString() != "KeyboardKey")
            continue;
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
    }

    for (const QJsonValue &element : elements) {
        if (element.toObject().value("__type").toString() == "KeyboardKey") {
            createKey(element.toObject());
        }
    }

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

    if (!shapeItem && boundaries.size() >= 32) {
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
        QGraphicsPolygonItem *polyItem = new QGraphicsPolygonItem(poly);
        polyItem->setPos(minX, minY);
        polyItem->setBrush(brushColor);
        polyItem->setPen(keyStyle.pen());
        m_scene->addItem(polyItem);
        shapeItem = polyItem;
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
    textItem->setPos(shapeBr.center().x() - textBr.width() / 2, shapeBr.center().y() - textBr.height() / 2);
    textItem->setZValue(1);

    for (int i = 0; i < kc.size(); ++i) {
        int code = static_cast<int>(kc[i].toDouble(0));
        // Skip invalid/empty keycode (0); otherwise keys with no keycode show as pressed
        if (code != 0)
            m_keys[code].push_back(shapeItem);
    }
    keyCounter[label] = 0;
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
                textItem->setPos(shapeBr.center().x() - textBr.width() / 2, shapeBr.center().y() - textBr.height() / 2);
                break;
            }
        }
    }
}
