#ifndef DIALOGSTYLE_H
#define DIALOGSTYLE_H

#include <QDialog>
#include <QColor>
#include <QComboBox>
#include <QListView>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QStyledItemDelegate>

struct KeyStyle;

class DialogStyle : public QDialog {
    Q_OBJECT

public:
    /** When indicatorStyle is true, only outline color, outline width and key color (idle) are shown (for mouse/joystick indicators). */
    explicit DialogStyle(QWidget *parent, const KeyStyle &currentStyle, const QString &previewLabel = QString(), bool isRectangle = false, bool indicatorStyle = false);
    KeyStyle getStyle() const;

private:
    QPushButton *m_outlineColorButton = nullptr;
    QDoubleSpinBox *m_outlineWidthSpin = nullptr;
    QSpinBox *m_fontSizeSpin = nullptr;
    QCheckBox *m_fontBoldCheck = nullptr;
    QCheckBox *m_fontItalicCheck = nullptr;
    QComboBox *m_fontCombo = nullptr;
    QDoubleSpinBox *m_cornerRadiusSpin = nullptr;
    QPushButton *m_keyColorButton = nullptr;
    QPushButton *m_keyColorPressedButton = nullptr;
    QPushButton *m_keyTextColorButton = nullptr;
    QPushButton *m_keyTextColorPressedButton = nullptr;
    QComboBox *m_textAlignmentCombo = nullptr;
    QString m_previewLabel;
    QColor m_outlineColor;
    QColor m_keyColor;
    QColor m_keyColorPressed;
    QColor m_keyTextColor;
    QColor m_keyTextColorPressed;

    void chooseOutlineColor();
    void updateOutlineColorButton();
    void updateKeyColorButton(QPushButton *btn, const QColor &c);
};

#endif // DIALOGSTYLE_H
