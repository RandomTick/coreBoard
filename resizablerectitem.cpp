#include "ResizableRectItem.h"

ResizableRectItem::ResizableRectItem(const QRectF &rect, const QString &text, const std::list<int> keycodes, QGraphicsItem *parent)
    : QGraphicsRectItem(rect, parent) {
    textItem = new QGraphicsTextItem(this);
    textItem->setPlainText(text);
    keyCodes = keycodes;
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
    centerText();
}

void ResizableRectItem::setRect(qreal x, qreal y, qreal w, qreal h) {
    setRect(QRectF(x, y, w, h));
}

void ResizableRectItem::setKeycodes(const std::list<int> newKeycodes){
    keyCodes = newKeycodes;
}

std::list<int> ResizableRectItem::getKeycodes(){
    return keyCodes;
}

void ResizableRectItem::centerText() {
    QRectF textRect = textItem->boundingRect();
    QPointF center = rect().center() - QPointF(textRect.width() / 2, textRect.height() / 2);
    textItem->setPos(center);
}
