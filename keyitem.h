// KeyItem.h
#ifndef KEYITEM_H
#define KEYITEM_H

#include <QGraphicsRectItem>
#include <QJsonObject>

class KeyItem : public QGraphicsRectItem {
public:
    KeyItem(const QJsonObject &keyData, QGraphicsItem *parent = nullptr);

    // Use QJsonObject to hold key data. This can later be used to serialize back to JSON.
    QJsonObject keyData() const;
    void setKeyData(const QJsonObject &data);

protected:
    // Override this to handle item interaction
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    QJsonObject m_keyData;
};

#endif // KEYITEM_H
