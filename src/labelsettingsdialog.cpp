#include "labelsettingsdialog.h"
#include "layoutsettings.h"
#include <QRadioButton>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QDialogButtonBox>

LabelSettingsDialog::LabelSettingsDialog(LayoutSettings *settings, QWidget *parent)
    : QDialog(parent)
    , m_settings(settings)
{
    setWindowTitle(tr("Label display"));

    m_followCapsAndShift = new QRadioButton(tr("Follow Caps Lock and Shift"), this);
    m_allUppercase = new QRadioButton(tr("All uppercase"), this);
    m_allLowercase = new QRadioButton(tr("All lowercase"), this);

    switch (m_settings->labelMode()) {
    case LabelMode::FollowCapsAndShift:
        m_followCapsAndShift->setChecked(true);
        break;
    case LabelMode::AllUppercase:
        m_allUppercase->setChecked(true);
        break;
    case LabelMode::AllLowercase:
        m_allLowercase->setChecked(true);
        break;
    }

    QButtonGroup *group = new QButtonGroup(this);
    group->addButton(m_followCapsAndShift);
    group->addButton(m_allUppercase);
    group->addButton(m_allLowercase);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_followCapsAndShift);
    mainLayout->addWidget(m_allUppercase);
    mainLayout->addWidget(m_allLowercase);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(buttons);
}

LabelSettingsDialog::~LabelSettingsDialog()
{
}

void LabelSettingsDialog::accept()
{
    LabelMode mode = LabelMode::FollowCapsAndShift;
    if (m_allUppercase->isChecked())
        mode = LabelMode::AllUppercase;
    else if (m_allLowercase->isChecked())
        mode = LabelMode::AllLowercase;

    m_settings->setLabelMode(mode);
    m_settings->save();
    QDialog::accept();
}
