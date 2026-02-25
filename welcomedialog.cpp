#include "welcomedialog.h"
#include "layoutsettings.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPushButton>

WelcomeDialog::WelcomeDialog(LayoutSettings *settings, QWidget *parent)
    : QDialog(parent)
    , m_settings(settings)
{
    setWindowTitle(tr("Welcome to coreBoard"));

    QLabel *body = new QLabel(this);
    body->setWordWrap(true);
    body->setTextFormat(Qt::RichText);
    body->setOpenExternalLinks(true);
    body->setText(
        tr("<p>coreBoard is a keyboard, mouse, and controller visualization tool for streaming and recording. "
           "It displays key presses and input on screen in real time and is designed to be easy to capture with OBS or similar software.</p>"
           "<p><b>Quick start:</b></p>"
           "<ul>"
           "<li><b>View</b> — The overlay to capture in OBS. Switch here when streaming.</li>"
           "<li><b>Edit Layout</b> — Design your overlay and open existing layouts.</li>"
           "<li><b>Using a NohBoard layout?</b> Switch to <b>Edit Layout</b>, click <b>Open Layout</b>, then select your .json file. You can start from your existing layout right away.</li>"
           "</ul>"
           "<p>Very custom NohBoard layouts might not be fully supported; if you run into issues, please report them (Help → Report a problem). "
           "Layouts saved from coreBoard may not be fully functional in NohBoard, as coreBoard adds features such as joystick viewers and controller bindings that NohBoard does not support.</p>")
    );

    m_dontShowAgainCheckBox = new QCheckBox(tr("Don't show this again."), this);

    QDialogButtonBox *buttons = new QDialogButtonBox(this);
    buttons->addButton(tr("Get started"), QDialogButtonBox::AcceptRole);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(body);
    mainLayout->addWidget(m_dontShowAgainCheckBox);
    mainLayout->addWidget(buttons);

    setMinimumWidth(420);
}

WelcomeDialog::~WelcomeDialog() = default;

void WelcomeDialog::accept()
{
    if (m_settings && m_dontShowAgainCheckBox->isChecked()) {
        m_settings->setWelcomeDialogShown(true);
        m_settings->save();
    }
    QDialog::accept();
}

void WelcomeDialog::showIfNeeded(LayoutSettings *settings, QWidget *parent)
{
    if (!settings || settings->welcomeDialogShown())
        return;
    WelcomeDialog dlg(settings, parent);
    dlg.exec();
}
