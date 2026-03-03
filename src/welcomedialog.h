#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QDialog>

class LayoutSettings;
class QCheckBox;

class WelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WelcomeDialog(LayoutSettings *settings, QWidget *parent = nullptr);
    ~WelcomeDialog();

    static void showIfNeeded(LayoutSettings *settings, QWidget *parent);

private slots:
    void accept() override;

private:
    LayoutSettings *m_settings = nullptr;
    QCheckBox *m_dontShowAgainCheckBox = nullptr;
};

#endif // WELCOMEDIALOG_H
