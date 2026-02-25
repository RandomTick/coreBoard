#ifndef ANGULARVIEWERITEM_H
#define ANGULARVIEWERITEM_H

#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QPointF>
#include "keystyle.h"

enum class AngularViewerSubType { LeftStick, RightStick };

class AngularViewerItem : public QGraphicsEllipseItem {
public:
    AngularViewerItem(const QRectF &rect, AngularViewerSubType subType, const QString &label = QString(), QGraphicsItem *parent = nullptr);

    AngularViewerSubType subType() const { return m_subType; }
    void setSubType(AngularViewerSubType t) { m_subType = t; }

    /** Controller index 0..3 for which gamepad drives this viewer. Persisted so e.g. "Controller 2" is kept across restarts even if disconnected. */
    int controllerIndex() const { return m_controllerIndex; }
    void setControllerIndex(int index) { m_controllerIndex = qBound(0, index, 3); }

    /** Flip stick X axis (default false). When true, left/right is reversed. */
    bool flipX() const { return m_flipX; }
    void setFlipX(bool on) { m_flipX = on; }
    /** Flip stick Y axis (default true so up=up). When true, up/down is reversed from raw. */
    bool flipY() const { return m_flipY; }
    void setFlipY(bool on) { m_flipY = on; }

    void setRect(const QRectF &rect);
    void setRect(qreal x, qreal y, qreal w, qreal h);

    QString getText() const;
    void setText(const QString &text);
    void setShiftText(const QString &text);
    QString getShiftText() const;

    KeyStyle keyStyle() const;
    void setKeyStyle(const KeyStyle &style);

    QPointF textPosition() const;
    void setTextPosition(const QPointF &pos);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QGraphicsTextItem *textItem;
    QString m_shiftText;
    AngularViewerSubType m_subType;
    int m_controllerIndex = 0;
    bool m_flipX = false;
    bool m_flipY = true;
    KeyStyle m_keyStyle;
    bool m_hasCustomTextPosition = false;
    QPointF m_textPosition;
    void centerText();
};

#endif // ANGULARVIEWERITEM_H
