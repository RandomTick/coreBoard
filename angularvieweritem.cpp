#include "angularvieweritem.h"
#include <QCoreApplication>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>
#include <QTextOption>

void AngularViewerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setPen(m_keyStyle.pen());
    painter->setBrush(brush());
    painter->drawEllipse(rect());
    if (isSelected()) {
        painter->setPen(QPen(Qt::white, 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(rect().adjusted(-2, -2, 2, 2));
    }
}

void AngularViewerItem::centerText() {
    QRectF textRect = textItem->boundingRect();
    QPointF target = m_hasCustomTextPosition ? m_textPosition : rect().center();
    target -= QPointF(textRect.width() / 2, textRect.height() / 2);
    textItem->setPos(target);
}

AngularViewerItem::AngularViewerItem(const QRectF &rect, AngularViewerSubType subType, const QString &label, QGraphicsItem *parent)
    : QGraphicsEllipseItem(rect, parent), m_subType(subType) {
    setFlag(QGraphicsItem::ItemIsSelectable);
    m_keyStyle = KeyStyle();
    setPen(m_keyStyle.pen());
    setBrush(Qt::lightGray);
    textItem = new QGraphicsTextItem(this);
    textItem->document()->setDocumentMargin(0);
    QTextOption opt;
    opt.setAlignment(Qt::AlignHCenter);
    textItem->document()->setDefaultTextOption(opt);
    textItem->setPlainText(label.isEmpty() ? (subType == AngularViewerSubType::LeftStick ? QCoreApplication::translate("AngularViewerItem", "L") : QCoreApplication::translate("AngularViewerItem", "R")) : label);
    textItem->setFont(m_keyStyle.font());
    const qreal margin = 8;
    textItem->setTextWidth(qMax(0.0, rect.width() - margin));
    centerText();
}

QString AngularViewerItem::getText() const {
    return textItem->toPlainText();
}

void AngularViewerItem::setText(const QString &text) {
    textItem->setPlainText(text);
    const qreal margin = 8;
    textItem->setTextWidth(qMax(0.0, this->rect().width() - margin));
    centerText();
}

void AngularViewerItem::setShiftText(const QString &text) {
    m_shiftText = text;
}

QString AngularViewerItem::getShiftText() const {
    return m_shiftText.isEmpty() ? textItem->toPlainText() : m_shiftText;
}

void AngularViewerItem::setRect(const QRectF &rect) {
    Q_UNUSED(rect);
    // Enforce circle in setRect(qreal,qreal,qreal,qreal)
}

void AngularViewerItem::setRect(qreal x, qreal y, qreal w, qreal h) {
    Q_UNUSED(x); Q_UNUSED(y);
    qreal s = qMin(w, h);
    if (s < 1) s = 1;
    QPointF c = pos() + QPointF(this->rect().width() / 2, this->rect().height() / 2);
    QGraphicsEllipseItem::setRect(0, 0, s, s);
    setPos(c.x() - s / 2, c.y() - s / 2);
    const qreal margin = 8;
    textItem->setTextWidth(qMax(0.0, s - margin));
    centerText();
}

QPointF AngularViewerItem::textPosition() const {
    return m_hasCustomTextPosition ? m_textPosition : rect().center();
}

void AngularViewerItem::setTextPosition(const QPointF &pos) {
    m_hasCustomTextPosition = true;
    m_textPosition = pos;
    centerText();
}

KeyStyle AngularViewerItem::keyStyle() const {
    return m_keyStyle;
}

void AngularViewerItem::setKeyStyle(const KeyStyle &style) {
    m_keyStyle = style;
    setPen(style.pen());
    setBrush(style.keyColor.isValid() ? style.keyColor : Qt::lightGray);
    textItem->setFont(style.font());
    centerText();
}
