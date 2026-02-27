#include "ResizableRectItem.h"
#include "keystyle.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>
#include <QTextOption>

ResizableRectItem::ResizableRectItem(const QRectF &rect, const QString &text, const std::list<int> keycodes, QGraphicsItem *parent)
    : QGraphicsRectItem(rect, parent) {
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
    if (textItem->font().italic()) w += 10;  // avoid clipping right-aligned italic
    textItem->setTextWidth(qMax(0.0, w));
    centerText();
}

void ResizableRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setPen(pen());
    painter->setBrush(brush());
    if (m_cornerRadius > 0) {
        QPainterPath path;
        path.addRoundedRect(rect(), m_cornerRadius, m_cornerRadius);
        painter->drawPath(path);
    } else {
        painter->drawRect(rect());
    }
    if (isSelected()) {
        painter->setPen(QPen(Qt::white, 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        QRectF selRect = rect().adjusted(-2, -2, 2, 2);
        if (m_cornerRadius > 0) {
            QPainterPath path;
            path.addRoundedRect(selRect, m_cornerRadius + 2, m_cornerRadius + 2);
            painter->drawPath(path);
        } else {
            painter->drawRect(selRect);
        }
    }
}

void ResizableRectItem::setText(const QString &text) {
    textItem->setPlainText(text);
    const qreal margin = 8;
    qreal w = rect().width() - margin;
    if (textItem->font().italic()) w += 10;
    textItem->setTextWidth(qMax(0.0, w));
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
    const qreal margin = 8;
    qreal w = rect.width() - margin;
    if (textItem->font().italic()) w += 10;
    textItem->setTextWidth(qMax(0.0, w));
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
    QRectF r = rect();
    QPointF anchor = m_hasCustomTextPosition ? m_textPosition : r.center();
    Qt::Alignment align = textItem->document()->defaultTextOption().alignment();
    qreal x = (align & Qt::AlignLeft) ? anchor.x()
          : (align & Qt::AlignRight) ? (anchor.x() - textRect.width())
          : (anchor.x() - textRect.width() / 2);
    qreal y = anchor.y() - textRect.height() / 2;
    textItem->setPos(x, y);
}

QPointF ResizableRectItem::textPosition() const {
    return m_hasCustomTextPosition ? m_textPosition : rect().center();
}

void ResizableRectItem::setTextPosition(const QPointF &pos) {
    m_hasCustomTextPosition = true;
    m_textPosition = pos;
    centerText();
}

void ResizableRectItem::setTextPositionToCenter() {
    m_hasCustomTextPosition = false;
    centerText();
}

KeyStyle ResizableRectItem::keyStyle() const {
    KeyStyle s;
    s.outlineColor = pen().color();
    s.outlineWidth = pen().widthF();
    s.cornerRadius = m_cornerRadius;
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

void ResizableRectItem::setKeyStyle(const KeyStyle &style) {
    setPen(style.pen());
    m_cornerRadius = style.cornerRadius;
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
