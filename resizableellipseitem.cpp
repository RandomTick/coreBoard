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
    opt.setAlignment(Qt::AlignHCenter);
    textItem->document()->setDefaultTextOption(opt);
    textItem->setPlainText(text);
    textItem->setFont(KeyStyle().font());
    keyCodes = keycodes;
    const qreal margin = 8;
    textItem->setTextWidth(qMax(0.0, rect.width() - margin));
    centerText();
}

void ResizableEllipseItem::setText(const QString &text) {
    textItem->setPlainText(text);
    const qreal margin = 8;
    textItem->setTextWidth(qMax(0.0, rect().width() - margin));
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
    textItem->setTextWidth(qMax(0.0, rect.width() - margin));
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
    QPointF target = m_hasCustomTextPosition ? m_textPosition : rect().center();
    target -= QPointF(textRect.width() / 2, textRect.height() / 2);
    textItem->setPos(target);
}

QPointF ResizableEllipseItem::textPosition() const {
    return m_hasCustomTextPosition ? m_textPosition : rect().center();
}

void ResizableEllipseItem::setTextPosition(const QPointF &pos) {
    m_hasCustomTextPosition = true;
    m_textPosition = pos;
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
    return s;
}

void ResizableEllipseItem::setKeyStyle(const KeyStyle &style) {
    setPen(style.pen());
    textItem->setFont(style.font());
    m_keyColor = style.keyColor;
    m_keyColorPressed = style.keyColorPressed;
    m_keyTextColor = style.keyTextColor;
    m_keyTextColorPressed = style.keyTextColorPressed;
    centerText();
}
