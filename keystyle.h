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
    qreal cornerRadius{0.0};
    int fontPointSize{10};
    bool fontBold{false};
    bool fontItalic{false};
    QString fontFamily;
    // Per-key color overrides for visualization only (editor ignores these); invalid = use global
    QColor keyColor;
    QColor keyColorPressed;
    QColor keyTextColor;
    QColor keyTextColorPressed;
    // Text alignment for key labels and standalone labels: 0=left, 1=center, 2=right
    int textAlignment{1};

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
        if (keyData.contains("CornerRadius"))
            s.cornerRadius = keyData.value("CornerRadius").toDouble(0.0);
        if (s.cornerRadius < 0)
            s.cornerRadius = 0;
        if (keyData.contains("KeyColor")) {
            QColor c(keyData.value("KeyColor").toString());
            if (c.isValid()) s.keyColor = c;
        }
        if (keyData.contains("KeyColorPressed")) {
            QColor c(keyData.value("KeyColorPressed").toString());
            if (c.isValid()) s.keyColorPressed = c;
        }
        if (keyData.contains("KeyTextColor")) {
            QColor c(keyData.value("KeyTextColor").toString());
            if (c.isValid()) s.keyTextColor = c;
        }
        if (keyData.contains("KeyTextColorPressed")) {
            QColor c(keyData.value("KeyTextColorPressed").toString());
            if (c.isValid()) s.keyTextColorPressed = c;
        }
        if (keyData.contains("TextAlignment")) {
            QString a = keyData.value("TextAlignment").toString();
            if (a == QLatin1String("left")) s.textAlignment = 0;
            else if (a == QLatin1String("right")) s.textAlignment = 2;
            else s.textAlignment = 1;  // center or any other
        }
        return s;
    }

    QJsonObject toJson() const {
        QJsonObject o;
        o.insert("OutlineColor", outlineColor.name());
        o.insert("OutlineWidth", outlineWidth);
        o.insert("FontSize", fontPointSize);
        o.insert("FontBold", fontBold);
        o.insert("FontItalic", fontItalic);
        if (cornerRadius > 0)
            o.insert("CornerRadius", cornerRadius);
        if (!fontFamily.isEmpty())
            o.insert("FontFamily", fontFamily);
        if (keyColor.isValid())
            o.insert("KeyColor", keyColor.name());
        if (keyColorPressed.isValid())
            o.insert("KeyColorPressed", keyColorPressed.name());
        if (keyTextColor.isValid())
            o.insert("KeyTextColor", keyTextColor.name());
        if (keyTextColorPressed.isValid())
            o.insert("KeyTextColorPressed", keyTextColorPressed.name());
        if (textAlignment != 1)
            o.insert("TextAlignment", textAlignment == 0 ? QStringLiteral("left") : (textAlignment == 2 ? QStringLiteral("right") : QStringLiteral("center")));
        return o;
    }
};

#endif // KEYSTYLE_H
