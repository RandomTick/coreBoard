#include "resizableellipseitem.h"
#include "keystyle.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>
#include <QTextOption>

void ResizableEllipseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawEllipse(rect());
    if (isSelected()) {
        painter->setPen(QPen(Qt::white, 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(rect().adjusted(-2, -2, 2, 2));
    }
}

ResizableEllipseItem::ResizableEllipseItem(const QRectF &rect, const QString &text, const std::list<int> keycodes, QGraphicsItem *parent)
    : QGraphicsEllipseItem(rect, parent) {
    setFlag(QGraphicsItem::ItemIsSelectable);
    setPen(KeyStyle().pen());
    textItem = new QGraphicsTextItem(this);
    textItem->document()->setDocumentMargin(0);
    QTextOption opt;
    opt.setAlignment(KeyStyle().textAlignment == 0 ? Qt::AlignLeft : (KeyStyle().textAlignment == 2 ? Qt::AlignRight : Qt::AlignHCenter));
    textItem->document()->setDefaultTextOption(opt);
    textItem->setPlainText(text);
    textItem->setFont(KeyStyle().font());
    keyCodes = keycodes;
    const qreal margin = 8;
    qreal w = rect.width() - margin;
    if (textItem->font().italic()) w += 10;
    textItem->setTextWidth(qMax(0.0, w));
    centerText();
}

void ResizableEllipseItem::setText(const QString &text) {
    textItem->setPlainText(text);
    const qreal margin = 8;
    qreal w = rect().width() - margin;
    if (textItem->font().italic()) w += 10;
    textItem->setTextWidth(qMax(0.0, w));
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
    const qreal margin = 8;
    qreal w = rect.width() - margin;
    if (textItem->font().italic()) w += 10;
    textItem->setTextWidth(qMax(0.0, w));
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
    QRectF r = rect();
    QPointF anchor = m_hasCustomTextPosition ? m_textPosition : r.center();
    Qt::Alignment align = textItem->document()->defaultTextOption().alignment();
    qreal x = (align & Qt::AlignLeft) ? anchor.x()
          : (align & Qt::AlignRight) ? (anchor.x() - textRect.width())
          : (anchor.x() - textRect.width() / 2);
    qreal y = anchor.y() - textRect.height() / 2;
    textItem->setPos(x, y);
}

QPointF ResizableEllipseItem::textPosition() const {
    return m_hasCustomTextPosition ? m_textPosition : rect().center();
}

void ResizableEllipseItem::setTextPosition(const QPointF &pos) {
    m_hasCustomTextPosition = true;
    m_textPosition = pos;
    centerText();
}

void ResizableEllipseItem::setTextPositionToCenter() {
    m_hasCustomTextPosition = false;
    centerText();
}

KeyStyle ResizableEllipseItem::keyStyle() const {
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

void ResizableEllipseItem::setKeyStyle(const KeyStyle &style) {
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
    qreal w = rect().width() - margin;
    if (textItem->font().italic()) w += 10;
    textItem->setTextWidth(qMax(0.0, w));
    centerText();
}
