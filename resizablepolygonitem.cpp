#include "resizablepolygonitem.h"
#include "keystyle.h"

ResizablePolygonItem::ResizablePolygonItem(const QPolygonF &templatePolygon, const QString &text, const std::list<int> keycodes, QGraphicsItem *parent)
    : QGraphicsPolygonItem(templatePolygon, parent)
    , _templatePolygon(templatePolygon)
    , keyCodes(keycodes) {
    setFlag(QGraphicsItem::ItemIsSelectable);
    setPen(KeyStyle().pen());
    textItem = new QGraphicsTextItem(this);
    textItem->setPlainText(text);
    textItem->setFont(KeyStyle().font());
    centerText();
}

void ResizablePolygonItem::updatePolygonFromTemplate(qreal w, qreal h) {
    QRectF tb = _templatePolygon.boundingRect();
    qreal tw = tb.width();
    qreal th = tb.height();
    if (tw < 1e-6) tw = 1;
    if (th < 1e-6) th = 1;
    qreal sx = w / tw;
    qreal sy = h / th;
    QPolygonF scaled;
    for (const QPointF &p : _templatePolygon) {
        scaled << QPointF((p.x() - tb.left()) * sx, (p.y() - tb.top()) * sy);
    }
    setPolygon(scaled);
    centerText();
}

void ResizablePolygonItem::setText(const QString &text) {
    textItem->setPlainText(text);
    centerText();
}

QString ResizablePolygonItem::getText() {
    return textItem->toPlainText();
}

void ResizablePolygonItem::setShiftText(const QString &text) {
    m_shiftText = text;
}

QString ResizablePolygonItem::getShiftText() const {
    return m_shiftText.isEmpty() ? textItem->toPlainText() : m_shiftText;
}

void ResizablePolygonItem::setRect(const QRectF &rect) {
    setRect(rect.x(), rect.y(), rect.width(), rect.height());
}

void ResizablePolygonItem::setRect(qreal x, qreal y, qreal w, qreal h) {
    setPos(x, y);
    updatePolygonFromTemplate(w, h);
}

void ResizablePolygonItem::setSize(qreal w, qreal h) {
    updatePolygonFromTemplate(w, h);
}

void ResizablePolygonItem::setKeycodes(const std::list<int> newKeycodes) {
    keyCodes = newKeycodes;
}

std::list<int> ResizablePolygonItem::getKeycodes() {
    return keyCodes;
}

void ResizablePolygonItem::centerText() {
    QRectF br = boundingRect();
    QRectF textRect = textItem->boundingRect();
    QPointF center = br.center() - QPointF(textRect.width() / 2, textRect.height() / 2);
    textItem->setPos(center);
}

KeyStyle ResizablePolygonItem::keyStyle() const {
    KeyStyle s;
    s.outlineColor = pen().color();
    s.outlineWidth = pen().widthF();
    s.fontPointSize = textItem->font().pointSize();
    s.fontBold = textItem->font().bold();
    s.fontItalic = textItem->font().italic();
    s.fontFamily = textItem->font().family();
    return s;
}

void ResizablePolygonItem::setKeyStyle(const KeyStyle &style) {
    setPen(style.pen());
    textItem->setFont(style.font());
    centerText();
}
