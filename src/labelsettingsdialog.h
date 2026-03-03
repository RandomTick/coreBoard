#ifndef LABELSETTINGSDIALOG_H
#define LABELSETTINGSDIALOG_H

#include <QDialog>
#include "layoutsettings.h"

class QRadioButton;

class LabelSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LabelSettingsDialog(LayoutSettings *settings, QWidget *parent = nullptr);
    ~LabelSettingsDialog();

private slots:
    void accept() override;

private:
    LayoutSettings *m_settings = nullptr;
    QRadioButton *m_followCapsAndShift = nullptr;
    QRadioButton *m_allUppercase = nullptr;
    QRadioButton *m_allLowercase = nullptr;
};

#endif // LABELSETTINGSDIALOG_H
