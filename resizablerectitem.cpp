#include "ResizableRectItem.h"
#include "keystyle.h"

ResizableRectItem::ResizableRectItem(const QRectF &rect, const QString &text, const std::list<int> keycodes, QGraphicsItem *parent)
    : QGraphicsRectItem(rect, parent) {
    setFlag(QGraphicsItem::ItemIsSelectable);
    setPen(KeyStyle().pen());
    textItem = new QGraphicsTextItem(this);
    textItem->setPlainText(text);
    textItem->setFont(KeyStyle().font());
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

void ResizableRectItem::setShiftText(const QString &text) {
    m_shiftText = text;
}

QString ResizableRectItem::getShiftText() const {
    return m_shiftText.isEmpty() ? textItem->toPlainText() : m_shiftText;
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

KeyStyle ResizableRectItem::keyStyle() const {
    KeyStyle s;
    s.outlineColor = pen().color();
    s.outlineWidth = pen().widthF();
    s.fontPointSize = textItem->font().pointSize();
    s.fontBold = textItem->font().bold();
    s.fontItalic = textItem->font().italic();
    s.fontFamily = textItem->font().family();
    return s;
}

void ResizableRectItem::setKeyStyle(const KeyStyle &style) {
    setPen(style.pen());
    textItem->setFont(style.font());
    centerText();
}
