#include "KeyItem.h"

KeyItem::KeyItem(const QJsonObject &keyData, QGraphicsItem *parent)
    : QGraphicsRectItem(parent), m_keyData(keyData) {
    // Set the position and size from keyData
    setRect(0, 0, keyData["size"].toObject()["width"].toInt(), keyData["size"].toObject()["height"].toInt());
    setPos(keyData["position"].toObject()["x"].toInt(), keyData["position"].toObject()["y"].toInt());
}

QJsonObject KeyItem::keyData() const {
    return m_keyData;
}

void KeyItem::setKeyData(const QJsonObject &data) {
    m_keyData = data;
}

QVariant KeyItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionChange && scene()) {
        // value is the new position.
        QPointF newPos = value.toPointF();
        // Perform bounds checking or emit a signal here if necessary.
    }
    return QGraphicsRectItem::itemChange(change, value);
}
