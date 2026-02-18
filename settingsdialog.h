#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QColor>

class LayoutSettings;
class QPushButton;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(LayoutSettings *settings, QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    void chooseKeyColor();
    void chooseHighlightColor();
    void chooseBackgroundColor();
    void chooseTextColor();
    void chooseHighlightedTextColor();
    void accept() override;

private:
    void updateColorButtonStyles();
    LayoutSettings *m_settings = nullptr;
    QPushButton *m_keyColorButton = nullptr;
    QPushButton *m_highlightColorButton = nullptr;
    QPushButton *m_backgroundColorButton = nullptr;
    QPushButton *m_textColorButton = nullptr;
    QPushButton *m_highlightedTextColorButton = nullptr;
    QColor m_keyColor;
    QColor m_highlightColor;
    QColor m_backgroundColor;
    QColor m_textColor;
    QColor m_highlightedTextColor;
};

#endif // SETTINGSDIALOG_H
