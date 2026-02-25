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
    opt.setAlignment(KeyStyle().textAlignment == 0 ? Qt::AlignLeft : (KeyStyle().textAlignment == 2 ? Qt::AlignRight : Qt::AlignHCenter));
    textItem->document()->setDefaultTextOption(opt);
    textItem->setPlainText(text);
    textItem->setFont(KeyStyle().font());
    const qreal margin = 8;
    qreal w = boundingRect().width() - margin;
    if (textItem->font().italic()) w += 10;
    textItem->setTextWidth(qMax(0.0, w));
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
    qreal docW = w - margin;
    if (textItem->font().italic()) docW += 10;
    textItem->setTextWidth(qMax(0.0, docW));
    centerText();
}

void ResizablePolygonItem::setText(const QString &text) {
    textItem->setPlainText(text);
    const qreal margin = 8;
    qreal w = boundingRect().width() - margin;
    if (textItem->font().italic()) w += 10;
    textItem->setTextWidth(qMax(0.0, w));
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

void ResizablePolygonItem::setPolygonDirect(const QPolygonF &polygon) {
    setPolygon(polygon);
    QRectF br = polygon.boundingRect();
    qreal w = br.width();
    qreal h = br.height();
    if (w < 1e-6) w = 1;
    if (h < 1e-6) h = 1;
    _templatePolygon.clear();
    for (const QPointF &p : polygon) {
        _templatePolygon << QPointF(100.0 * (p.x() - br.left()) / w, 100.0 * (p.y() - br.top()) / h);
    }
    const qreal margin = 8;
    qreal tw = w - margin;
    if (textItem->font().italic()) tw += 10;
    textItem->setTextWidth(qMax(0.0, tw));
    centerText();
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
    QPointF anchor = m_hasCustomTextPosition ? m_textPosition : br.center();
    Qt::Alignment align = textItem->document()->defaultTextOption().alignment();
    qreal x = (align & Qt::AlignLeft) ? anchor.x()
          : (align & Qt::AlignRight) ? (anchor.x() - textRect.width())
          : (anchor.x() - textRect.width() / 2);
    qreal y = anchor.y() - textRect.height() / 2;
    textItem->setPos(x, y);
}

QPointF ResizablePolygonItem::textPosition() const {
    return m_hasCustomTextPosition ? m_textPosition : boundingRect().center();
}

void ResizablePolygonItem::setTextPosition(const QPointF &pos) {
    m_hasCustomTextPosition = true;
    m_textPosition = pos;
    centerText();
}

KeyStyle ResizablePolygonItem::keyStyle() const {
    KeyStyle s;
    s.outlineColor = pen().color();
    s.outlineWidth = pen().widthF();
    s.fontPointSize = textItem->font().pointSize();
    s.fontBold = textItem->font().bold();
    s.fontItalic = textItem->font().italic();
    s.fontFamily = textItem->font().family();
    s.keyColor = m_keyColor;
    s.keyColorPressed = m_keyColorPressed;
    s.keyTextColor = m_keyTextColor;
    s.keyTextColorPressed = m_keyTextColorPressed;
    Qt::Alignment align = textItem->document()->defaultTextOption().alignment();
    s.textAlignment = (align & Qt::AlignRight) ? 2 : ((align & Qt::AlignLeft) ? 0 : 1);
    return s;
}

void ResizablePolygonItem::setKeyStyle(const KeyStyle &style) {
    setPen(style.pen());
    textItem->setFont(style.font());
    m_keyColor = style.keyColor;
    m_keyColorPressed = style.keyColorPressed;
    m_keyTextColor = style.keyTextColor;
    m_keyTextColorPressed = style.keyTextColorPressed;
    QTextOption opt = textItem->document()->defaultTextOption();
    opt.setAlignment(style.textAlignment == 0 ? Qt::AlignLeft : (style.textAlignment == 2 ? Qt::AlignRight : Qt::AlignHCenter));
    textItem->document()->setDefaultTextOption(opt);
    const qreal margin = 8;
    qreal w = boundingRect().width() - margin;
    if (textItem->font().italic()) w += 10;
    textItem->setTextWidth(qMax(0.0, w));
    centerText();
}
