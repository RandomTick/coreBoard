#include "resizablepolygonitem.h"
#include "keystyle.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>
#include <QTextOption>

void ResizablePolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawPolygon(polygon());
    if (isSelected()) {
        painter->setPen(QPen(Qt::white, 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawPolygon(polygon());
    }
}

ResizablePolygonItem::ResizablePolygonItem(const QPolygonF &templatePolygon, const QString &text, const std::list<int> keycodes, QGraphicsItem *parent)
    : QGraphicsPolygonItem(templatePolygon, parent)
    , _templatePolygon(templatePolygon)
    , keyCodes(keycodes) {
    setFlag(QGraphicsItem::ItemIsSelectable);
    setPen(KeyStyle().pen());
    textItem = new QGraphicsTextItem(this);
    textItem->document()->setDocumentMargin(0);
    QTextOption opt;
    opt.setAlignment(Qt::AlignHCenter);
    textItem->document()->setDefaultTextOption(opt);
    textItem->setPlainText(text);
    textItem->setFont(KeyStyle().font());
    const qreal margin = 8;
    textItem->setTextWidth(qMax(0.0, boundingRect().width() - margin));
    centerText();
}

void ResizablePolygonItem::updatePolygonFromTemplate(qreal w, qreal h) {
    const qreal margin = 8;
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
    textItem->setTextWidth(qMax(0.0, w - margin));
    centerText();
}

void ResizablePolygonItem::setText(const QString &text) {
    textItem->setPlainText(text);
    const qreal margin = 8;
    textItem->setTextWidth(qMax(0.0, boundingRect().width() - margin));
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
