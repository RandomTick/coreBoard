#ifndef RESIZABLEPATHITEM_H
#define RESIZABLEPATHITEM_H

#include <QGraphicsPathItem>
#include <QGraphicsTextItem>
#include <QColor>
#include <QPointF>
#include <QPolygonF>
#include <QList>

struct KeyStyle;

class ResizablePathItem : public QGraphicsPathItem
{
public:
    ResizablePathItem(const QPolygonF &outer, const QList<QPolygonF> &holes,
                      const QString &text, const std::list<int> keycodes, QGraphicsItem *parent = nullptr);

    void setText(const QString &text);
    QString getText();
    void setShiftText(const QString &text);
    QString getShiftText() const;
    void setRect(const QRectF &rect);
    void setRect(qreal x, qreal y, qreal w, qreal h);
    void setSize(qreal w, qreal h);
    void setKeycodes(const std::list<int> &newKeycodes);
    std::list<int> getKeycodes();

    KeyStyle keyStyle() const;
    void setKeyStyle(const KeyStyle &style);

    QPointF textPosition() const;
    void setTextPosition(const QPointF &pos);
    bool hasCustomTextPosition() const { return m_hasCustomTextPosition; }
    void setTextPositionToCenter();

    QPolygonF outerPolygon() const;
    QList<QPolygonF> holes() const;
    void setPathFromOuterAndHoles(const QPolygonF &outer, const QList<QPolygonF> &holes);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    void rebuildPath();
    void centerText();

    QPolygonF m_outer;
    QList<QPolygonF> m_holes;
    QGraphicsTextItem *textItem = nullptr;
    QString m_shiftText;
    std::list<int> keyCodes;
    QColor m_keyColor, m_keyColorPressed, m_keyTextColor, m_keyTextColorPressed;
    bool m_hasCustomTextPosition = false;
    QPointF m_textPosition;
};

#endif // RESIZABLEPATHITEM_H
