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
    opt.setAlignment(Qt::AlignHCenter);
    textItem->document()->setDefaultTextOption(opt);
    textItem->setPlainText(text);
    textItem->setFont(KeyStyle().font());
    keyCodes = keycodes;
    const qreal margin = 8;
    textItem->setTextWidth(qMax(0.0, rect.width() - margin));
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
    textItem->setTextWidth(qMax(0.0, rect().width() - margin));
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
    textItem->setTextWidth(qMax(0.0, rect.width() - margin));
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
    s.cornerRadius = m_cornerRadius;
    s.fontPointSize = textItem->font().pointSize();
    s.fontBold = textItem->font().bold();
    s.fontItalic = textItem->font().italic();
    s.fontFamily = textItem->font().family();
    return s;
}

void ResizableRectItem::setKeyStyle(const KeyStyle &style) {
    setPen(style.pen());
    m_cornerRadius = style.cornerRadius;
    textItem->setFont(style.font());
    centerText();
}
