#ifndef KEYBOARDWIDGET_H
#define KEYBOARDWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QJsonArray>

class KeyboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KeyboardWidget(QWidget *parent = nullptr);
    void loadLayout(const QString &fileName);


private:
    void createKey(const QJsonObject &keyData);
    void changeKeyColor(const int &keyLabel, const QColor &color);
    KeyboardWidget *m_keyboardWidget; // Assuming this is your KeyboardWidget member


public slots:
    void onKeyPressed(int key);
    void onKeyReleased(int key);
};

#endif // KEYBOARDWIDGET_H
