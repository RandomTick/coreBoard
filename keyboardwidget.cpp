#include "KeyboardWidget.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsTextItem>
#include <QAbstractGraphicsShapeItem>
#include <QVBoxLayout>
#include <QTimer>
#include <cmath>

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
            shape->setBrush(m_keyColor);
            setShapeTextColor(shape, m_textColor);
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

    for (const QJsonValue &element : elements) {
        if (element.toObject().value("__type").toString() == "KeyboardKey") {
            createKey(element.toObject());
        }
    }

    int maxWidth = rootObject.value("Width").toInt();
    int maxHeight = rootObject.value("Height").toInt();
    m_scene->setSceneRect(0, 0, qMax(1, maxWidth), qMax(1, maxHeight));
    setMinimumSize(maxWidth, maxHeight);
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
    QJsonArray kc = keyData.value("KeyCodes").toArray();

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
            QGraphicsRectItem *rect = new QGraphicsRectItem(0, 0, w, h);
            rect->setPos(minX, minY);
            rect->setBrush(m_keyColor);
            rect->setPen(QPen(Qt::black, 1));
            m_scene->addItem(rect);
            shapeItem = rect;
        }
    }

    if (!shapeItem && boundaries.size() >= 32) {
        QGraphicsEllipseItem *ellipse = new QGraphicsEllipseItem(0, 0, w, h);
        ellipse->setPos(minX, minY);
        ellipse->setBrush(m_keyColor);
        ellipse->setPen(QPen(Qt::black, 1));
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
        polyItem->setBrush(m_keyColor);
        polyItem->setPen(QPen(Qt::black, 1));
        m_scene->addItem(polyItem);
        shapeItem = polyItem;
    }

    if (!shapeItem)
        return;

    QGraphicsTextItem *textItem = new QGraphicsTextItem(shapeItem);
    textItem->setPlainText(label);
    textItem->setDefaultTextColor(m_textColor);
    QRectF textBr = textItem->boundingRect();
    QRectF shapeBr = shapeItem->boundingRect();
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

void KeyboardWidget::changeKeyColor(const int &keyCode, const QColor &brushColor, const QColor &textColor)
{
    auto it = m_keys.find(keyCode);
    if (it == m_keys.end())
        return;
    for (QGraphicsItem *item : it->second) {
        if (!item)
            continue;
        QAbstractGraphicsShapeItem *shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem*>(item);
        if (shape) {
            shape->setBrush(brushColor);
            setShapeTextColor(shape, textColor);
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
    changeKeyColor(key, m_highlightColor, m_highlightedTextColor);
}

void KeyboardWidget::onKeyReleased(int key)
{
    changeKeyColor(key, m_keyColor, m_textColor);
}
