#include "KeyboardWidget.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QLabel>

KeyboardWidget::KeyboardWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(200, 200);
}

std::map<int,QLabel*> keys;
QColor defaultColor = QColor(144,55,1);

void KeyboardWidget::loadLayout(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Failed to open layout file.");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray keys = doc.object().value("keys").toArray();

    for (const QJsonValue &value : keys) {
        createKey(value.toObject());
    }
}

void KeyboardWidget::createKey(const QJsonObject &keyData)
{
    int keyCode = keyData.value("keyCode").toInt();

    QRect geometry(
        keyData["position"].toObject()["x"].toInt(),
        keyData["position"].toObject()["y"].toInt(),
        keyData["size"].toObject()["width"].toInt(),
        keyData["size"].toObject()["height"].toInt()
        );

    QLabel *keyLabel = new QLabel(this);
    keyLabel->setGeometry(geometry);



    keyLabel->setText(keyData.value("label").toString());
    keyLabel->setStyleSheet(QString("background-color: #%1%2%3; border: 1px solid black;")
                           .arg(defaultColor.red(), 2, 16, QChar('0'))
                           .arg(defaultColor.green(), 2, 16, QChar('0'))
                           .arg(defaultColor.blue(), 2, 16, QChar('0')));
    keyLabel->setAlignment(Qt::AlignCenter); // Center the text
    keyLabel->show();
    keyLabel->update();


    keys[keyCode] = keyLabel;
}


void KeyboardWidget::changeKeyColor(const int &keyCode, const QColor &color) {

    QLabel *key = keys[keyCode];
    key->setStyleSheet(QString("background-color: #%1%2%3; border: 1px solid black;")
                           .arg(color.red(), 2, 16, QChar('0'))
                           .arg(color.green(), 2, 16, QChar('0'))
                           .arg(color.blue(), 2, 16, QChar('0')));

}

void KeyboardWidget::onKeyPressed(int key) {
    changeKeyColor(key, Qt::red); // Example: change to red when pressed
}

void KeyboardWidget::onKeyReleased(int key) {
    changeKeyColor(key, defaultColor); // Change back to original color
}

