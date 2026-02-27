#ifndef RESIZABLERECTITEM_H
#define RESIZABLERECTITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QColor>
#include <QPointF>

struct KeyStyle;

class ResizableRectItem : public QGraphicsRectItem {
public:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    ResizableRectItem(const QRectF &rect, const QString &text, const std::list<int> keycodes, QGraphicsItem *parent = nullptr);

    void setText(const QString &text);
    QString getText();
    void setShiftText(const QString &text);
    QString getShiftText() const;
    void setRect(const QRectF &rect);
    void setRect(qreal x, qreal y, qreal w, qreal h);
    void setKeycodes(const std::list<int> newKeycodes);
    std::list<int> getKeycodes();

    KeyStyle keyStyle() const;
    void setKeyStyle(const KeyStyle &style);

    QPointF textPosition() const;
    void setTextPosition(const QPointF &pos);
    bool hasCustomTextPosition() const { return m_hasCustomTextPosition; }
    void setTextPositionToCenter();

private:
    QGraphicsTextItem *textItem;
    QString m_shiftText;
    qreal m_cornerRadius = 0;
    std::list<int> keyCodes;
    QColor m_keyColor, m_keyColorPressed, m_keyTextColor, m_keyTextColorPressed;
    bool m_hasCustomTextPosition = false;
    QPointF m_textPosition;
    void centerText();
};

#endif // RESIZABLERECTITEM_H
