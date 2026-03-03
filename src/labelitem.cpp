#include "labelitem.h"
#include <QPainter>
#include <QTextDocument>
#include <QTextOption>

LabelItem::LabelItem(const QString &text, qreal x, qreal y, QGraphicsItem *parent)
    : QGraphicsTextItem(text, parent)
    , m_keyStyle(KeyStyle())
    , m_anchor(x, y)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setFont(m_keyStyle.font());
    setDefaultTextColor(m_keyStyle.keyTextColor.isValid() ? m_keyStyle.keyTextColor : Qt::white);
    document()->setDocumentMargin(2);
    QTextOption opt;
    opt.setAlignment(m_keyStyle.textAlignment == 0 ? Qt::AlignLeft : (m_keyStyle.textAlignment == 2 ? Qt::AlignRight : Qt::AlignHCenter));
    document()->setDefaultTextOption(opt);
    updateLabelWidth();
    updatePosition();
}

QString LabelItem::getText() const
{
    return toPlainText();
}

void LabelItem::setText(const QString &text)
{
    setPlainText(text);
    updateLabelWidth();
    updatePosition();
}

QString LabelItem::getShiftText() const
{
    return m_shiftText;
}

void LabelItem::setShiftText(const QString &text)
{
    m_shiftText = text;
    updateLabelWidth();
    updatePosition();
}

KeyStyle LabelItem::keyStyle() const
{
    return m_keyStyle;
}

void LabelItem::setKeyStyle(const KeyStyle &style)
{
    m_keyStyle = style;
    setFont(style.font());
    if (style.keyTextColor.isValid())
        setDefaultTextColor(style.keyTextColor);
    else
        setDefaultTextColor(Qt::white);
    QTextOption opt = document()->defaultTextOption();
    opt.setAlignment(style.textAlignment == 0 ? Qt::AlignLeft : (style.textAlignment == 2 ? Qt::AlignRight : Qt::AlignHCenter));
    document()->setDefaultTextOption(opt);
    updatePosition();
}

QPointF LabelItem::anchorScenePos() const
{
    return m_anchor;
}

QVariant LabelItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged) {
        if (!m_ignoreNextPositionChange) {
            QPointF newPos = pos();
            m_anchor += (newPos - m_lastPos);
        }
        m_lastPos = pos();
    }
    return QGraphicsTextItem::itemChange(change, value);
}

void LabelItem::updateLabelWidth()
{
    QString base = toPlainText();
    QRectF br = boundingRect();
    qreal w = br.width();
    if (!m_shiftText.isEmpty() && m_shiftText != base) {
        setPlainText(m_shiftText);
        qreal wShift = boundingRect().width();
        setPlainText(base);
        w = qMax(w, wShift);
    }
    m_labelWidth = w;
    if (m_labelWidth > 0)
        document()->setTextWidth(2 * m_labelWidth);
}

void LabelItem::updatePosition()
{
    QRectF textRect = boundingRect();
    qreal w = textRect.width();
    qreal h = textRect.height();
    qreal x, y;
    if (m_keyStyle.textAlignment == 0) {
        x = m_anchor.x();
        y = m_anchor.y() - h / 2;
    } else if (m_keyStyle.textAlignment == 2) {
        x = m_anchor.x() - w;
        y = m_anchor.y() - h / 2;
    } else {
        x = m_anchor.x() - w / 2;
        y = m_anchor.y() - h / 2;
    }
    m_ignoreNextPositionChange = true;
    setPos(x, y);
    m_ignoreNextPositionChange = false;
    m_lastPos = pos();
}
