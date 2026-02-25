#include "labelitem.h"
#include <QPainter>
#include <QTextDocument>

LabelItem::LabelItem(const QString &text, qreal x, qreal y, QGraphicsItem *parent)
    : QGraphicsTextItem(text, parent)
    , m_keyStyle(KeyStyle())
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setPos(x, y);
    setFont(m_keyStyle.font());
    setDefaultTextColor(m_keyStyle.keyTextColor.isValid() ? m_keyStyle.keyTextColor : Qt::white);
    document()->setDocumentMargin(2);
}

QString LabelItem::getText() const
{
    return toPlainText();
}

void LabelItem::setText(const QString &text)
{
    setPlainText(text);
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
}
