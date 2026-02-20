#ifndef KEYBOARDWIDGET_H
#define KEYBOARDWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QJsonArray>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <map>
#include <vector>
#include <QColor>
#include "layoutsettings.h"

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
    void applyLayoutData(const QByteArray &jsonData);
    void changeKeyColor(const int &keyCode, const QColor &brushColor, const QColor &textColor);
    static void setShapeTextColor(QGraphicsItem *shapeItem, const QColor &color);
    void updateLabelsForShiftState();
    void resetCounter();

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

public slots:
    void onKeyPressed(int key);
    void onKeyReleased(int key);
};

#endif // KEYBOARDWIDGET_H
