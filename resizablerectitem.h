#ifndef RESIZABLERECTITEM_H
#define RESIZABLERECTITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>

class ResizableRectItem : public QGraphicsRectItem {
public:
    ResizableRectItem(const QRectF &rect, const QString &text, QGraphicsItem *parent = nullptr);

    void setText(const QString &text);
    void setRect(const QRectF &rect);
    void setRect(qreal x, qreal y, qreal w, qreal h);
    QString getText();

private:
    QGraphicsTextItem *textItem;
    void centerText();
};

#endif // RESIZABLERECTITEM_H
