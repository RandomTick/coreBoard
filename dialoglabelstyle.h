#ifndef DIALOGLABELSTYLE_H
#define DIALOGLABELSTYLE_H

#include <QDialog>
#include <QColor>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>

struct KeyStyle;

class DialogLabelStyle : public QDialog {
    Q_OBJECT

public:
    explicit DialogLabelStyle(QWidget *parent, const KeyStyle &currentStyle, const QString &previewLabel = QString());
    KeyStyle getStyle() const;

private:
    QComboBox *m_fontCombo = nullptr;
    QSpinBox *m_fontSizeSpin = nullptr;
    QPushButton *m_textColorButton = nullptr;
    QColor m_textColor;
    QString m_previewLabel;

    void updateTextColorButton();
};

#endif
