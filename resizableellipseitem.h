#ifndef RESIZABLEELLIPSEITEM_H
#define RESIZABLEELLIPSEITEM_H

#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>

struct KeyStyle;

class ResizableEllipseItem : public QGraphicsEllipseItem {
public:
    ResizableEllipseItem(const QRectF &rect, const QString &text, const std::list<int> keycodes, QGraphicsItem *parent = nullptr);

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

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QGraphicsTextItem *textItem;
    QString m_shiftText;
    std::list<int> keyCodes;
    void centerText();
};

#endif // RESIZABLEELLIPSEITEM_H
