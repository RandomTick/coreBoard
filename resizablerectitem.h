#ifndef RESIZABLERECTITEM_H
#define RESIZABLERECTITEM_H

#include<list>

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>

class ResizableRectItem : public QGraphicsRectItem {
public:
    ResizableRectItem(const QRectF &rect, const QString &text, const std::list<int> keycodes, QGraphicsItem *parent = nullptr);

    void setText(const QString &text);
    void setRect(const QRectF &rect);
    void setRect(qreal x, qreal y, qreal w, qreal h);
    void setKeycodes(const std::list<int> newKeycodes);
    QString getText();
    std::list<int> getKeycodes();

private:
    QGraphicsTextItem *textItem;
    std::list<int> keyCodes;
    void centerText();
};

#endif // RESIZABLERECTITEM_H
