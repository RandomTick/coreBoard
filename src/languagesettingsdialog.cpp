#include "languagesettingsdialog.h"
#include "layoutsettings.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>

LanguageSettingsDialog::LanguageSettingsDialog(LayoutSettings *settings, QWidget *parent)
    : QDialog(parent)
    , m_settings(settings)
{
    setWindowTitle(tr("Language"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(new QLabel(tr("Interface language:")));
    m_languageCombo = new QComboBox(this);
    m_languageCombo->addItem(tr("English"), QStringLiteral("en_US"));
    m_languageCombo->addItem(QStringLiteral("Deutsch"), QStringLiteral("de_DE"));
    m_languageCombo->addItem(QStringLiteral("Français"), QStringLiteral("fr_FR"));
    mainLayout->addWidget(m_languageCombo);

    QString currentCode = m_settings ? m_settings->languageCode() : QString();
    if (currentCode.isEmpty())
        currentCode = QStringLiteral("en_US");
    int idx = m_languageCombo->findData(currentCode);
    if (idx >= 0)
        m_languageCombo->setCurrentIndex(idx);
    else
        m_languageCombo->setCurrentIndex(0);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);
}

LanguageSettingsDialog::~LanguageSettingsDialog()
{
}

void LanguageSettingsDialog::accept()
{
    if (m_settings) {
        m_settings->setLanguageCode(m_languageCombo->currentData().toString());
        m_settings->save();
    }
    QDialog::accept();
}
