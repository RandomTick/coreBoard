#include "ResizableRectItem.h"

ResizableRectItem::ResizableRectItem(const QRectF &rect, const QString &text, QGraphicsItem *parent)
    : QGraphicsRectItem(rect, parent) {
    textItem = new QGraphicsTextItem(this);
    textItem->setPlainText(text);
    centerText();
}

void ResizableRectItem::setText(const QString &text) {
    textItem->setPlainText(text);
    centerText();
}

QString ResizableRectItem::getText() {
    return textItem->toPlainText();
}

void ResizableRectItem::setRect(const QRectF &rect) {
    QGraphicsRectItem::setRect(rect);
    centerText(); // Recenter the text whenever the rectangle size changes
}

void ResizableRectItem::setRect(qreal x, qreal y, qreal w, qreal h) {
    setRect(QRectF(x, y, w, h));
}

void ResizableRectItem::centerText() {
    // Ensure the text is centered within the new rectangle bounds
    QRectF textRect = textItem->boundingRect();
    QPointF center = rect().center() - QPointF(textRect.width() / 2, textRect.height() / 2);
    textItem->setPos(center);
}
