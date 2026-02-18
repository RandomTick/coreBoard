#ifndef RESIZABLEPOLYGONITEM_H
#define RESIZABLEPOLYGONITEM_H

#include <QGraphicsPolygonItem>
#include <QGraphicsTextItem>

class ResizablePolygonItem : public QGraphicsPolygonItem {
public:
    ResizablePolygonItem(const QPolygonF &templatePolygon, const QString &text, const std::list<int> keycodes, QGraphicsItem *parent = nullptr);

    void setText(const QString &text);
    void setRect(const QRectF &rect);
    void setRect(qreal x, qreal y, qreal w, qreal h);
    void setSize(qreal w, qreal h);
    void setKeycodes(const std::list<int> newKeycodes);
    QString getText();
    std::list<int> getKeycodes();

private:
    QPolygonF _templatePolygon;
    QGraphicsTextItem *textItem;
    std::list<int> keyCodes;
    void centerText();
    void updatePolygonFromTemplate(qreal w, qreal h);
};

#endif // RESIZABLEPOLYGONITEM_H
