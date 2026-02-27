#ifndef CONTROLLERITEM_H
#define CONTROLLERITEM_H

#include <QGraphicsRectItem>
#include <QPointF>
#include <QColor>
#include "keystyle.h"

/// Shared SVG bytes for the controller graphic. If fillColor is valid, SVG fill is replaced with it (otherwise black).
QByteArray controllerSvgBytes(const QColor &fillColor = QColor());

class ControllerItem : public QGraphicsRectItem {
public:
    explicit ControllerItem(const QRectF &rect, QGraphicsItem *parent = nullptr);

    void setRect(const QRectF &rect);
    void setRect(qreal x, qreal y, qreal w, qreal h);

    KeyStyle keyStyle() const;
    void setKeyStyle(const KeyStyle &style);

    /** Controller index 0..3 for which gamepad drives this overlay. Default 0. */
    int controllerIndex() const { return m_controllerIndex; }
    void setControllerIndex(int index) { m_controllerIndex = qBound(0, index, 3); }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    KeyStyle m_keyStyle;
    int m_controllerIndex = 0;
};

#endif // CONTROLLERITEM_H
