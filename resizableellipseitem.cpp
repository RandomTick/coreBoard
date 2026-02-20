#include "resizableellipseitem.h"
#include "keystyle.h"

ResizableEllipseItem::ResizableEllipseItem(const QRectF &rect, const QString &text, const std::list<int> keycodes, QGraphicsItem *parent)
    : QGraphicsEllipseItem(rect, parent) {
    setFlag(QGraphicsItem::ItemIsSelectable);
    setPen(KeyStyle().pen());
    textItem = new QGraphicsTextItem(this);
    textItem->setPlainText(text);
    textItem->setFont(KeyStyle().font());
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

void ResizableEllipseItem::setShiftText(const QString &text) {
    m_shiftText = text;
}

QString ResizableEllipseItem::getShiftText() const {
    return m_shiftText.isEmpty() ? textItem->toPlainText() : m_shiftText;
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

KeyStyle ResizableEllipseItem::keyStyle() const {
    KeyStyle s;
    s.outlineColor = pen().color();
    s.outlineWidth = pen().widthF();
    s.fontPointSize = textItem->font().pointSize();
    s.fontBold = textItem->font().bold();
    s.fontItalic = textItem->font().italic();
    s.fontFamily = textItem->font().family();
    return s;
}

void ResizableEllipseItem::setKeyStyle(const KeyStyle &style) {
    setPen(style.pen());
    textItem->setFont(style.font());
    centerText();
}
