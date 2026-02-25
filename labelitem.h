#ifndef LABELITEM_H
#define LABELITEM_H

#include <QGraphicsTextItem>
#include <QPointF>
#include "keystyle.h"

class LabelItem : public QGraphicsTextItem {
public:
    explicit LabelItem(const QString &text, qreal x, qreal y, QGraphicsItem *parent = nullptr);

    QString getText() const;
    void setText(const QString &text);

    KeyStyle keyStyle() const;
    void setKeyStyle(const KeyStyle &style);

private:
    KeyStyle m_keyStyle;
};

#endif // LABELITEM_H
