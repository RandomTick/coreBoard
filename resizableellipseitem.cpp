#include "resizableellipseitem.h"

ResizableEllipseItem::ResizableEllipseItem(const QRectF &rect, const QString &text, const std::list<int> keycodes, QGraphicsItem *parent)
    : QGraphicsEllipseItem(rect, parent) {
    textItem = new QGraphicsTextItem(this);
    textItem->setPlainText(text);
    keyCodes = keycodes;
    centerText();
}

void ResizableEllipseItem::setText(const QString &text) {
    textItem->setPlainText(text);
    centerText();
}

QString ResizableEllipseItem::getText() {
    return textItem->toPlainText();
}

void ResizableEllipseItem::setRect(const QRectF &rect) {
    QGraphicsEllipseItem::setRect(rect);
    centerText();
}

void ResizableEllipseItem::setRect(qreal x, qreal y, qreal w, qreal h) {
    setRect(QRectF(x, y, w, h));
}

void ResizableEllipseItem::setKeycodes(const std::list<int> newKeycodes) {
    keyCodes = newKeycodes;
}

std::list<int> ResizableEllipseItem::getKeycodes() {
    return keyCodes;
}

void ResizableEllipseItem::centerText() {
    QRectF textRect = textItem->boundingRect();
    QPointF center = rect().center() - QPointF(textRect.width() / 2, textRect.height() / 2);
    textItem->setPos(center);
}
