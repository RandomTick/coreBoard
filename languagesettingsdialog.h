#ifndef LANGUAGESETTINGSDIALOG_H
#define LANGUAGESETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>

class LayoutSettings;

class LanguageSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LanguageSettingsDialog(LayoutSettings *settings, QWidget *parent = nullptr);
    ~LanguageSettingsDialog();

private:
    void accept() override;

    LayoutSettings *m_settings = nullptr;
    QComboBox *m_languageCombo = nullptr;
};

#endif // LANGUAGESETTINGSDIALOG_H
