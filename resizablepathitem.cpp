#include "resizablepathitem.h"
#include "keystyle.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>
#include <QTextOption>

ResizablePathItem::ResizablePathItem(const QPolygonF &outer, const QList<QPolygonF> &holes,
                                     const QString &text, const std::list<int> keycodes, QGraphicsItem *parent)
    : ResizablePathItem(outer, holes, QList<bool>(), text, keycodes, parent)
{
}

ResizablePathItem::ResizablePathItem(const QPolygonF &outer, const QList<QPolygonF> &holes,
                                     const QList<bool> &holeIsCircular,
                                     const QString &text, const std::list<int> keycodes, QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
    , m_outer(outer)
    , m_holes(holes)
    , m_holeIsCircular(holeIsCircular)
    , keyCodes(keycodes)
{
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
    rebuildPath();
    centerText();
}

void ResizablePathItem::rebuildPath()
{
    QPainterPath path;
    path.addPolygon(m_outer);
    for (const QPolygonF &hole : m_holes) {
        QPainterPath holePath;
        holePath.addPolygon(hole);
        path = path.subtracted(holePath);
    }
    setPath(path);
    const qreal margin = 8;
    if (textItem) {
        qreal w = boundingRect().width() - margin;
        if (textItem->font().italic()) w += 10;
        textItem->setTextWidth(qMax(0.0, w));
    }
}

void ResizablePathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawPath(path());
    if (isSelected()) {
        painter->setPen(QPen(Qt::white, 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(path());
    }
}

void ResizablePathItem::setText(const QString &text)
{
    textItem->setPlainText(text);
    const qreal margin = 8;
    qreal w = boundingRect().width() - margin;
    if (textItem->font().italic()) w += 10;
    textItem->setTextWidth(qMax(0.0, w));
    centerText();
}

QString ResizablePathItem::getText()
{
    return textItem->toPlainText();
}

void ResizablePathItem::setShiftText(const QString &text)
{
    m_shiftText = text;
}

QString ResizablePathItem::getShiftText() const
{
    return m_shiftText.isEmpty() ? textItem->toPlainText() : m_shiftText;
}

void ResizablePathItem::setRect(const QRectF &rect)
{
    setRect(rect.x(), rect.y(), rect.width(), rect.height());
}

void ResizablePathItem::setRect(qreal x, qreal y, qreal w, qreal h)
{
    setPos(x, y);
    QRectF obr = m_outer.boundingRect();
    qreal ow = obr.width();
    qreal oh = obr.height();
    if (ow < 1e-6) ow = 1;
    if (oh < 1e-6) oh = 1;
    qreal sx = w / ow;
    qreal sy = h / oh;
    QPolygonF scaledOuter;
    for (const QPointF &p : m_outer)
        scaledOuter << QPointF((p.x() - obr.left()) * sx, (p.y() - obr.top()) * sy);
    QList<QPolygonF> scaledHoles;
    for (const QPolygonF &hole : m_holes) {
        QPolygonF sh;
        for (const QPointF &p : hole)
            sh << QPointF((p.x() - obr.left()) * sx, (p.y() - obr.top()) * sy);
        scaledHoles << sh;
    }
    m_outer = scaledOuter;
    m_holes = scaledHoles;
    // m_holeIsCircular unchanged by scaling
    rebuildPath();
    centerText();
}

void ResizablePathItem::setSize(qreal w, qreal h)
{
    setRect(pos().x(), pos().y(), w, h);
}

void ResizablePathItem::setKeycodes(const std::list<int> &newKeycodes)
{
    keyCodes = newKeycodes;
}

std::list<int> ResizablePathItem::getKeycodes()
{
    return keyCodes;
}

void ResizablePathItem::centerText()
{
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

QPointF ResizablePathItem::textPosition() const
{
    return m_hasCustomTextPosition ? m_textPosition : boundingRect().center();
}

void ResizablePathItem::setTextPosition(const QPointF &pos)
{
    m_hasCustomTextPosition = true;
    m_textPosition = pos;
    centerText();
}

void ResizablePathItem::setTextPositionToCenter()
{
    m_hasCustomTextPosition = false;
    centerText();
}

QPolygonF ResizablePathItem::outerPolygon() const
{
    return m_outer;
}

QList<QPolygonF> ResizablePathItem::holes() const
{
    return m_holes;
}

QList<bool> ResizablePathItem::holeIsCircular() const
{
    return m_holeIsCircular;
}

void ResizablePathItem::setPathFromOuterAndHoles(const QPolygonF &outer, const QList<QPolygonF> &holes, const QList<bool> &holeIsCircular)
{
    m_outer = outer;
    m_holes = holes;
    m_holeIsCircular = holeIsCircular.size() == static_cast<int>(holes.size())
        ? holeIsCircular
        : QList<bool>(holes.size(), false);
    rebuildPath();
    centerText();
}

KeyStyle ResizablePathItem::keyStyle() const
{
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

void ResizablePathItem::setKeyStyle(const KeyStyle &style)
{
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
