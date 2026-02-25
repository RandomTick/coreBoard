#ifndef MOUSESPEEDINDICATORITEM_H
#define MOUSESPEEDINDICATORITEM_H

#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QPointF>
#include "keystyle.h"

class MouseSpeedIndicatorItem : public QGraphicsEllipseItem {
public:
    MouseSpeedIndicatorItem(qreal centerX, qreal centerY, qreal radius, const QString &label = QString(), QGraphicsItem *parent = nullptr);

    QString getText() const;
    void setText(const QString &text);
    void setShiftText(const QString &text);
    QString getShiftText() const;

    qreal radius() const;
    void setRadius(qreal r);
    void setRect(const QRectF &rect);
    void setRect(qreal x, qreal y, qreal w, qreal h);
    QPointF centerPos() const;
    void setCenterPos(qreal x, qreal y);
    void setCenterPos(const QPointF &p);

    KeyStyle keyStyle() const;
    void setKeyStyle(const KeyStyle &style);

    QPointF textPosition() const;
    void setTextPosition(const QPointF &pos);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QGraphicsTextItem *textItem;
    QString m_shiftText;
    KeyStyle m_keyStyle;
    bool m_hasCustomTextPosition = false;
    QPointF m_textPosition;
    void centerText();
    void updateRectFromCenterAndRadius();
};

#endif // MOUSESPEEDINDICATORITEM_H
