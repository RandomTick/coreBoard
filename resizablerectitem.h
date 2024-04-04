#ifndef RESIZABLERECTITEM_H
#define RESIZABLERECTITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>

class ResizableRectItem : public QGraphicsRectItem {
public:
    ResizableRectItem(const QRectF &rect, QString text, QGraphicsItem *parent = nullptr)
        : QGraphicsRectItem(rect, parent) {
        textItem = new QGraphicsTextItem(this);
        textItem->setPlainText(text); // Set default text or pass it in the constructor
        centerText();
    }

    void setText(const QString &text) {
        textItem->setPlainText(text);
        centerText();
    }

    void setRect(const QRectF &rect) {
        QGraphicsRectItem::setRect(rect);
        centerText(); // Recenter the text whenever the rectangle size changes
    }

    void setRect(qreal x, qreal y, qreal w, qreal h) {
        setRect(QRectF(x, y, w, h));
    }

private:
    QGraphicsTextItem *textItem;

    void centerText() {
        // Ensure the text is centered within the new rectangle bounds
        QRectF textRect = textItem->boundingRect();
        QPointF center = rect().center() - QPointF(textRect.width() / 2, textRect.height() / 2);
        textItem->setPos(center);
    }
};

#endif // RESIZABLERECTITEM_H
