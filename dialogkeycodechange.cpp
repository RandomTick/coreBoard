#include "DialogKeycodeChange.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

DialogKeycodeChange::DialogKeycodeChange(QWidget *parent, std::list<int> currentKeyCodes)
    : QDialog(parent), keyCodes(currentKeyCodes)
{
    setWindowTitle("Change Key Codes");

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *label = new QLabel("Current Key Codes:", this);
    layout->addWidget(label);

    QLineEdit *keyCodesDisplay = new QLineEdit(this);
    keyCodesDisplay->setReadOnly(true);
    layout->addWidget(keyCodesDisplay);

    QHBoxLayout *controlButtonsLayout = new QHBoxLayout();
    layout->addLayout(controlButtonsLayout);

    QPushButton *cancelButton = new QPushButton(tr("Cancel"), this);
    controlButtonsLayout->addWidget(cancelButton);

    QPushButton *addButton = new QPushButton("Add Key Code", this);
    controlButtonsLayout->addWidget(addButton);

    QPushButton *clearButton = new QPushButton("Clear Key Codes", this);
    controlButtonsLayout->addWidget(clearButton);

    QPushButton *okButton = new QPushButton("OK", this);
    controlButtonsLayout->addWidget(okButton);


    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    connect(addButton, &QPushButton::clicked, this, [this, keyCodesDisplay]() {
        // Placeholder for key code input
        int keyCode = 123; // Replace with actual input method
        insertKeycode(keyCode);
        updateDisplay(keyCodesDisplay);
    });

    connect(clearButton, &QPushButton::clicked, this, [this, keyCodesDisplay]() {
        keyCodes.clear();
        updateDisplay(keyCodesDisplay);
    });

    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

    updateDisplay(keyCodesDisplay);
}

std::list<int> DialogKeycodeChange::getKeyCodes() const
{
    return keyCodes;
}

void DialogKeycodeChange::insertKeycode(const int keyCode)
{
    keyCodes.push_back(keyCode);
}

void DialogKeycodeChange::setKeyCodes(const std::list<int> newKeyCodes)
{
    keyCodes = newKeyCodes;
}

void DialogKeycodeChange::updateDisplay(QLineEdit *display)
{
    QString keyCodesStr;
    for (const auto &code : keyCodes) {
        keyCodesStr += QString::number(code) + " ";
    }
    display->setText(keyCodesStr.trimmed());
}
