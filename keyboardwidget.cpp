#include "KeyboardWidget.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QLabel>

KeyboardWidget::KeyboardWidget(QWidget *parent) : QWidget(parent)
{
}

std::map<int, QLabel*> keys;
std::map<QString, int> keyCounter;

//TODO: change colors to use from config
QColor defaultColor = QColor(0,0,255);
QColor hightlightedColor = Qt::red;

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


    //adjust size
    int maxWidth = 0;
    int maxHeight = 0;

    for (const auto& keyLabel : findChildren<QLabel*>()) {
        int rightEdge = keyLabel->geometry().right();
        int bottomEdge = keyLabel->geometry().bottom();

        maxWidth = qMax(maxWidth, rightEdge);
        maxHeight = qMax(maxHeight, bottomEdge);
    }

    setMinimumSize(maxWidth + 1, maxHeight + 1); // +1 to ensure a margin

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

    QString label = keyData.value("label").toString();
    //qDebug(QString("keyCode: %1 for key %2").arg(keyCode).arg(label).toStdString().c_str());



    keyLabel->setText(label);
    keyLabel->setStyleSheet(QString("background-color: #%1%2%3; border: 1px solid black;")
                           .arg(defaultColor.red(), 2, 16, QChar('0'))
                           .arg(defaultColor.green(), 2, 16, QChar('0'))
                           .arg(defaultColor.blue(), 2, 16, QChar('0')));
    keyLabel->setAlignment(Qt::AlignCenter); // Center the text
    keyLabel->show();
    keyLabel->update();


    keys[keyCode] = keyLabel;
    keyCounter[label] = 0;
}


void KeyboardWidget::changeKeyColor(const int &keyCode, const QColor &color) {


    //make sure key is in the layout before accessing it:
    if (keys[keyCode] == nullptr){
        return;
    }

    QLabel *key = keys[keyCode];
    key->setStyleSheet(QString("background-color: #%1%2%3; border: 1px solid black;")
                           .arg(color.red(), 2, 16, QChar('0'))
                           .arg(color.green(), 2, 16, QChar('0'))
                           .arg(color.blue(), 2, 16, QChar('0')));

    //somehow track count (no permanent increment, rather on rising edge)
}
void resetCounter(){
    for (std::pair<const QString, int>  i : keyCounter){
        i.second = 0;
    }
}

void KeyboardWidget::onKeyPressed(int key) {
    changeKeyColor(key, hightlightedColor);
}

void KeyboardWidget::onKeyReleased(int key) {
    changeKeyColor(key, defaultColor);
}

