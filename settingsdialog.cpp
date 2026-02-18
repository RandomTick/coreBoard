#include "settingsdialog.h"
#include "layoutsettings.h"
#include <QColorDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QDialogButtonBox>

SettingsDialog::SettingsDialog(LayoutSettings *settings, QWidget *parent)
    : QDialog(parent)
    , m_settings(settings)
{
    setWindowTitle(tr("Settings"));

    m_keyColor = m_settings->keyColor();
    m_highlightColor = m_settings->highlightColor();
    m_backgroundColor = m_settings->backgroundColor();
    m_textColor = m_settings->textColor();
    m_highlightedTextColor = m_settings->highlightedTextColor();

    QGridLayout *grid = new QGridLayout();
    m_keyColorButton = new QPushButton(tr("Choose..."), this);
    m_highlightColorButton = new QPushButton(tr("Choose..."), this);
    m_backgroundColorButton = new QPushButton(tr("Choose..."), this);
    m_textColorButton = new QPushButton(tr("Choose..."), this);
    m_highlightedTextColorButton = new QPushButton(tr("Choose..."), this);
    grid->addWidget(new QLabel(tr("Key color:")), 0, 0);
    grid->addWidget(m_keyColorButton, 0, 1);
    grid->addWidget(new QLabel(tr("Highlight color:")), 1, 0);
    grid->addWidget(m_highlightColorButton, 1, 1);
    grid->addWidget(new QLabel(tr("Background color:")), 2, 0);
    grid->addWidget(m_backgroundColorButton, 2, 1);
    grid->addWidget(new QLabel(tr("Text color:")), 3, 0);
    grid->addWidget(m_textColorButton, 3, 1);
    grid->addWidget(new QLabel(tr("Highlighted text color:")), 4, 0);
    grid->addWidget(m_highlightedTextColorButton, 4, 1);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttons);

    updateColorButtonStyles();

    connect(m_keyColorButton, &QPushButton::clicked, this, &SettingsDialog::chooseKeyColor);
    connect(m_highlightColorButton, &QPushButton::clicked, this, &SettingsDialog::chooseHighlightColor);
    connect(m_backgroundColorButton, &QPushButton::clicked, this, &SettingsDialog::chooseBackgroundColor);
    connect(m_textColorButton, &QPushButton::clicked, this, &SettingsDialog::chooseTextColor);
    connect(m_highlightedTextColorButton, &QPushButton::clicked, this, &SettingsDialog::chooseHighlightedTextColor);
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::chooseKeyColor()
{
    QColor c = QColorDialog::getColor(m_keyColor, this, tr("Key color"));
    if (c.isValid()) {
        m_keyColor = c;
        updateColorButtonStyles();
    }
}

void SettingsDialog::chooseHighlightColor()
{
    QColor c = QColorDialog::getColor(m_highlightColor, this, tr("Highlight color"));
    if (c.isValid()) {
        m_highlightColor = c;
        updateColorButtonStyles();
    }
}

void SettingsDialog::chooseBackgroundColor()
{
    QColor c = QColorDialog::getColor(m_backgroundColor, this, tr("Background color"));
    if (c.isValid()) {
        m_backgroundColor = c;
        updateColorButtonStyles();
    }
}

void SettingsDialog::chooseTextColor()
{
    QColor c = QColorDialog::getColor(m_textColor, this, tr("Text color"));
    if (c.isValid()) {
        m_textColor = c;
        updateColorButtonStyles();
    }
}

void SettingsDialog::chooseHighlightedTextColor()
{
    QColor c = QColorDialog::getColor(m_highlightedTextColor, this, tr("Highlighted text color"));
    if (c.isValid()) {
        m_highlightedTextColor = c;
        updateColorButtonStyles();
    }
}

void SettingsDialog::updateColorButtonStyles()
{
    auto setButtonColor = [](QPushButton *btn, const QColor &color) {
        if (!btn) return;
        // Use black or white text so "Choose..." is readable on any background
        int luminance = (color.red() * 299 + color.green() * 587 + color.blue() * 114) / 1000;
        QString textColor = (luminance > 160) ? QLatin1String("black") : QLatin1String("white");
        QString style = QString("background-color: %1; color: %2; border: 1px solid #888; min-width: 80px;")
            .arg(color.name(), textColor);
        btn->setStyleSheet(style);
        btn->setAutoFillBackground(true);
    };
    setButtonColor(m_keyColorButton, m_keyColor);
    setButtonColor(m_highlightColorButton, m_highlightColor);
    setButtonColor(m_backgroundColorButton, m_backgroundColor);
    setButtonColor(m_textColorButton, m_textColor);
    setButtonColor(m_highlightedTextColorButton, m_highlightedTextColor);
}

void SettingsDialog::accept()
{
    m_settings->setKeyColor(m_keyColor);
    m_settings->setHighlightColor(m_highlightColor);
    m_settings->setBackgroundColor(m_backgroundColor);
    m_settings->setTextColor(m_textColor);
    m_settings->setHighlightedTextColor(m_highlightedTextColor);
    m_settings->save();
    QDialog::accept();
}
