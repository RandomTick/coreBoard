#ifndef KEYSTYLE_H
#define KEYSTYLE_H

#include <QColor>
#include <QString>
#include <QJsonObject>
#include <QFont>
#include <QPen>

struct KeyStyle {
    QColor outlineColor{Qt::black};
    qreal outlineWidth{1.0};
    int fontPointSize{10};
    bool fontBold{false};
    bool fontItalic{false};
    QString fontFamily;

    QPen pen() const {
        return QPen(outlineColor, outlineWidth);
    }

    QFont font() const {
        QFont f(fontFamily.isEmpty() ? QFont().family() : fontFamily, -1, fontBold ? QFont::Bold : QFont::Normal);
        f.setPointSize(fontPointSize > 0 ? fontPointSize : 10);
        f.setItalic(fontItalic);
        return f;
    }

    static KeyStyle fromJson(const QJsonObject &keyData) {
        KeyStyle s;
        if (keyData.contains("OutlineColor")) {
            QString str = keyData.value("OutlineColor").toString();
            if (!str.isEmpty())
                s.outlineColor = QColor(str);
        }
        if (keyData.contains("OutlineWidth"))
            s.outlineWidth = keyData.value("OutlineWidth").toDouble(1.0);
        if (s.outlineWidth < 0.1)
            s.outlineWidth = 1.0;
        if (keyData.contains("FontSize"))
            s.fontPointSize = keyData.value("FontSize").toInt(10);
        if (keyData.contains("FontBold"))
            s.fontBold = keyData.value("FontBold").toBool(false);
        if (keyData.contains("FontItalic"))
            s.fontItalic = keyData.value("FontItalic").toBool(false);
        if (keyData.contains("FontFamily"))
            s.fontFamily = keyData.value("FontFamily").toString();
        return s;
    }

    QJsonObject toJson() const {
        QJsonObject o;
        o.insert("OutlineColor", outlineColor.name());
        o.insert("OutlineWidth", outlineWidth);
        o.insert("FontSize", fontPointSize);
        o.insert("FontBold", fontBold);
        o.insert("FontItalic", fontItalic);
        if (!fontFamily.isEmpty())
            o.insert("FontFamily", fontFamily);
        return o;
    }
};

#endif // KEYSTYLE_H
