#ifndef KEYBOARDWIDGET_H
#define KEYBOARDWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QJsonArray>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <map>
#include <vector>
#include <QList>
#include <QRectF>
#include <QColor>
#include <QTimer>
#include "layoutsettings.h"

struct MouseSpeedIndicatorOverlay {
    qreal centerX = 0, centerY = 0, radius = 15;
    QGraphicsEllipseItem *trackItem = nullptr;
    QGraphicsEllipseItem *indicatorItem = nullptr;
    QPointF lastScenePos;
    qint64 lastTime = 0;
    QList<QPointF> speedHistory;
    static const int kMouseSmooth = 5;
};

struct AngularViewerOverlay {
    int controllerIndex = 0;
    bool isLeftStick = true;
    bool flipX = false;
    bool flipY = true;
    QRectF rect;
    QGraphicsEllipseItem *trackItem = nullptr;
    QGraphicsEllipseItem *indicatorItem = nullptr;
};

struct LabelOverlay {
    QGraphicsTextItem *textItem = nullptr;
    QString baseText;
    QString shiftText;
    qreal anchorX = 0;  // top-left from layout
    qreal anchorY = 0;
    qreal labelWidth = 0;  // fixed width (max of base/shift) so position doesn't jump
    int textAlignment = 1;  // 0=left, 1=center, 2=right
};

class KeyboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KeyboardWidget(QWidget *parent = nullptr);
    void loadLayout(const QString &fileName, int retryCount = 0);
    void loadLayoutFromData(const QByteArray &jsonData);
    QJsonArray elements;

    void setKeyColor(const QColor &color);
    void setHighlightColor(const QColor &color);
    void setBackgroundColor(const QColor &color);
    void setTextColor(const QColor &color);
    void setHighlightedTextColor(const QColor &color);
    void applyColors();  // re-apply current colors to all keys and scene

    void setLabelMode(LabelMode mode);
    void setShiftState(bool pressed);
    void setCapsLockState(bool on);

private:
    void createKey(const QJsonObject &keyData);
    void createMouseSpeedIndicatorOverlay(const QJsonObject &keyData);
    void createAngularViewerOverlay(const QJsonObject &keyData);
    void createLabelOverlay(const QJsonObject &keyData);
    void applyLayoutData(const QByteArray &jsonData);
    void changeKeyColor(const int &keyCode, const QColor &brushColor, const QColor &textColor, bool isPressed);
    static void setShapeTextColor(QGraphicsItem *shapeItem, const QColor &color);
    void updateLabelsForShiftState();
    void resetCounter();
    void updateMouseIndicatorsFromCursor();

    QGraphicsView *m_view = nullptr;
    QGraphicsScene *m_scene = nullptr;
    std::map<int, std::vector<QGraphicsItem*>> m_keys;
    std::map<QString, int> keyCounter;
    QColor m_keyColor;
    QColor m_highlightColor;
    QColor m_backgroundColor;
    QColor m_textColor;
    QColor m_highlightedTextColor;
    LabelMode m_labelMode = LabelMode::FollowCapsAndShift;
    bool m_shiftPressed = false;
    bool m_capsLockOn = false;

    QList<MouseSpeedIndicatorOverlay> m_mouseSpeedIndicators;
    QList<AngularViewerOverlay> m_angularViewers;
    QList<LabelOverlay> m_labelOverlays;
    QTimer *m_mouseIndicatorTimer = nullptr;

public slots:
    void onKeyPressed(int key);
    void onKeyReleased(int key);
    void onLeftStickChanged(int controllerIndex, qreal x, qreal y);
    void onRightStickChanged(int controllerIndex, qreal x, qreal y);
};

#endif // KEYBOARDWIDGET_H
