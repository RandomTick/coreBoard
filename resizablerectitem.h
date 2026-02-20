#ifndef RESIZABLERECTITEM_H
#define RESIZABLERECTITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>

struct KeyStyle;

class ResizableRectItem : public QGraphicsRectItem {
public:
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

private:
    QGraphicsTextItem *textItem;
    QString m_shiftText;
    std::list<int> keyCodes;
    void centerText();
};

#endif // RESIZABLERECTITEM_H
