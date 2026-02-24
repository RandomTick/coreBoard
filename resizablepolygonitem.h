#ifndef RESIZABLEPOLYGONITEM_H
#define RESIZABLEPOLYGONITEM_H

#include <QGraphicsPolygonItem>
#include <QGraphicsTextItem>
#include <QColor>
#include <QPointF>

struct KeyStyle;

class ResizablePolygonItem : public QGraphicsPolygonItem {
public:
    ResizablePolygonItem(const QPolygonF &templatePolygon, const QString &text, const std::list<int> keycodes, QGraphicsItem *parent = nullptr);

    void setText(const QString &text);
    QString getText();
    void setShiftText(const QString &text);
    QString getShiftText() const;
    void setRect(const QRectF &rect);
    void setRect(qreal x, qreal y, qreal w, qreal h);
    void setSize(qreal w, qreal h);
    void setKeycodes(const std::list<int> newKeycodes);
    std::list<int> getKeycodes();

    KeyStyle keyStyle() const;
    void setKeyStyle(const KeyStyle &style);

    QPointF textPosition() const;
    void setTextPosition(const QPointF &pos);

    void setPolygonDirect(const QPolygonF &polygon);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QPolygonF _templatePolygon;
    QGraphicsTextItem *textItem;
    QString m_shiftText;
    std::list<int> keyCodes;
    QColor m_keyColor, m_keyColorPressed, m_keyTextColor, m_keyTextColorPressed;
    bool m_hasCustomTextPosition = false;
    QPointF m_textPosition;
    void centerText();
    void updatePolygonFromTemplate(qreal w, qreal h);
};

#endif // RESIZABLEPOLYGONITEM_H
