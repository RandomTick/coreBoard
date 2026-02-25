#include "mousespeedindicatoritem.h"
#include "keystyle.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>
#include <QTextOption>

void MouseSpeedIndicatorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setPen(m_keyStyle.pen());
    painter->setBrush(brush());
    painter->drawEllipse(rect());
    if (isSelected()) {
        painter->setPen(QPen(Qt::white, 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(rect().adjusted(-2, -2, 2, 2));
    }
    // Text/label not drawn here; use a separate Label shape if you need a label.
}

void MouseSpeedIndicatorItem::updateRectFromCenterAndRadius() {
    qreal r = radius();
    setRect(0, 0, 2 * r, 2 * r);
    centerText();
}

MouseSpeedIndicatorItem::MouseSpeedIndicatorItem(qreal centerX, qreal centerY, qreal radius, const QString &label, QGraphicsItem *parent)
    : QGraphicsEllipseItem(0, 0, 2 * radius, 2 * radius, parent) {
    setFlag(QGraphicsItem::ItemIsSelectable);
    setPos(centerX - radius, centerY - radius);
    m_keyStyle = KeyStyle();
    setPen(m_keyStyle.pen());
    setBrush(Qt::lightGray);
    textItem = new QGraphicsTextItem(this);
    textItem->document()->setDocumentMargin(0);
    QTextOption opt;
    opt.setAlignment(Qt::AlignHCenter);
    textItem->document()->setDefaultTextOption(opt);
    textItem->setPlainText(label);
    textItem->setFont(m_keyStyle.font());
    const qreal margin = 8;
    textItem->setTextWidth(qMax(0.0, rect().width() - margin));
    centerText();
}

QString MouseSpeedIndicatorItem::getText() const {
    return textItem->toPlainText();
}

void MouseSpeedIndicatorItem::setText(const QString &text) {
    textItem->setPlainText(text);
    const qreal margin = 8;
    textItem->setTextWidth(qMax(0.0, rect().width() - margin));
    centerText();
}

void MouseSpeedIndicatorItem::setShiftText(const QString &text) {
    m_shiftText = text;
}

QString MouseSpeedIndicatorItem::getShiftText() const {
    return m_shiftText.isEmpty() ? textItem->toPlainText() : m_shiftText;
}

qreal MouseSpeedIndicatorItem::radius() const {
    return rect().width() / 2.0;
}

void MouseSpeedIndicatorItem::setRadius(qreal r) {
    if (r < 1.0) r = 1.0;
    QPointF c = centerPos();
    setRect(0, 0, 2 * r, 2 * r);
    setPos(c.x() - r, c.y() - r);
    const qreal margin = 8;
    textItem->setTextWidth(qMax(0.0, rect().width() - margin));
    centerText();
}

void MouseSpeedIndicatorItem::setRect(const QRectF &rect) {
    Q_UNUSED(rect);
    // Keep current radius; caller should use setRadius to change size (enforces circle).
}

void MouseSpeedIndicatorItem::setRect(qreal x, qreal y, qreal w, qreal h) {
    Q_UNUSED(x); Q_UNUSED(y);
    qreal r = qMin(w, h) / 2.0;
    if (r < 1.0) r = 1.0;
    QPointF c = centerPos();
    QGraphicsEllipseItem::setRect(0, 0, 2 * r, 2 * r);
    setPos(c.x() - r, c.y() - r);
    textItem->setTextWidth(qMax(0.0, 2 * r - 8));
    centerText();
}

QPointF MouseSpeedIndicatorItem::centerPos() const {
    qreal r = radius();
    return pos() + QPointF(r, r);
}

void MouseSpeedIndicatorItem::setCenterPos(qreal x, qreal y) {
    qreal r = radius();
    setPos(x - r, y - r);
}

void MouseSpeedIndicatorItem::setCenterPos(const QPointF &p) {
    setCenterPos(p.x(), p.y());
}

void MouseSpeedIndicatorItem::centerText() {
    QRectF textRect = textItem->boundingRect();
    QPointF target = m_hasCustomTextPosition ? m_textPosition : rect().center();
    target -= QPointF(textRect.width() / 2, textRect.height() / 2);
    textItem->setPos(target);
}

QPointF MouseSpeedIndicatorItem::textPosition() const {
    return m_hasCustomTextPosition ? m_textPosition : rect().center();
}

void MouseSpeedIndicatorItem::setTextPosition(const QPointF &pos) {
    m_hasCustomTextPosition = true;
    m_textPosition = pos;
    centerText();
}

KeyStyle MouseSpeedIndicatorItem::keyStyle() const {
    return m_keyStyle;
}

void MouseSpeedIndicatorItem::setKeyStyle(const KeyStyle &style) {
    m_keyStyle = style;
    setPen(style.pen());
    setBrush(style.keyColor.isValid() ? style.keyColor : Qt::lightGray);
    textItem->setFont(style.font());
    centerText();
}
