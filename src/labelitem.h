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
    QString getShiftText() const;
    void setShiftText(const QString &text);

    KeyStyle keyStyle() const;
    void setKeyStyle(const KeyStyle &style);

    /** Anchor in scene coords: single point of truth. Meaning by alignment: center=center, left=left edge, right=right edge. */
    QPointF anchorScenePos() const;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    void updatePosition();
    void updateLabelWidth();

    KeyStyle m_keyStyle;
    QString m_shiftText;
    QPointF m_anchor;           // scene coords, meaning depends on alignment
    qreal m_labelWidth = 0;
    QPointF m_lastPos;
    bool m_ignoreNextPositionChange = false;
};

#endif // LABELITEM_H
