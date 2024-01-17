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
    QJsonObject rootObject = doc.object();
    QJsonArray elements = rootObject.value("Elements").toArray();

    for (const QJsonValue &element : elements) {
        if (element.toObject().value("__type").toString() == "KeyboardKey") {
            createKey(element.toObject());
        }
    }



    //adjust size
    int maxWidth = rootObject.value("Width").toInt();
    int maxHeight = rootObject.value("Height").toInt();


    /*
    for (const auto& keyLabel : findChildren<QLabel*>()) {
        int rightEdge = keyLabel->geometry().right();
        int bottomEdge = keyLabel->geometry().bottom();

        maxWidth = qMax(maxWidth, rightEdge);
        maxHeight = qMax(maxHeight, bottomEdge);
    }
    */

    setMinimumSize(maxWidth + 1, maxHeight + 1); // +1 to ensure a margin
    this->window()->resize(maxWidth, maxHeight);

}


void KeyboardWidget::createKey(const QJsonObject &keyData)
{
    QJsonArray boundaries = keyData.value("Boundaries").toArray();
    if (boundaries.size() < 4) {//TODO: Handle others than rectangles here!
        qWarning("Invalid boundaries data.");
        return;
    }

    // Assuming rectangular shape and the points are given in order
    int x = boundaries[0].toObject()["X"].toInt();
    int y = boundaries[0].toObject()["Y"].toInt();
    int width = boundaries[1].toObject()["X"].toInt() - x;
    int height = boundaries[3].toObject()["Y"].toInt() - y;

    QRect geometry(x, y, width, height);
    QLabel *keyLabel = new QLabel(this);
    keyLabel->setGeometry(geometry);

    //QString label = keyData.value("label").toString();
    //qDebug(QString("keyCode: %1 for key %2").arg(keyCode).arg(label).toStdString().c_str());
    QString label = keyData.value("Text").toString();



    keyLabel->setText(label);
    keyLabel->setStyleSheet(QString("background-color: #%1%2%3; border: 1px solid black;")
                           .arg(defaultColor.red(), 2, 16, QChar('0'))
                           .arg(defaultColor.green(), 2, 16, QChar('0'))
                           .arg(defaultColor.blue(), 2, 16, QChar('0')));
    keyLabel->setAlignment(Qt::AlignCenter); // Center the text
    keyLabel->show();
    keyLabel->update();


    QJsonArray keyCodes = keyData.value("KeyCodes").toArray();
    if (!keyCodes.isEmpty()) {
        for (int i = 0; i < keyCodes.size(); i++){
            keys[keyCodes[i].toInt()] = keyLabel;
        }
    }
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

