#include "dialogtextchange.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSignalMapper>

DialogTextChange::DialogTextChange(QWidget *parent, QString currentText, QString currentShiftText) : QDialog(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(new QLabel(tr("Text (normal):")));
    lineEdit = new QLineEdit(this);
    mainLayout->addWidget(lineEdit);
    lineEdit->setText(currentText);

    mainLayout->addWidget(new QLabel(tr("Text when Shift held:")));
    shiftLineEdit = new QLineEdit(this);
    shiftLineEdit->setPlaceholderText(tr("Leave empty to use same as normal"));
    mainLayout->addWidget(shiftLineEdit);
    shiftLineEdit->setText(currentShiftText);

    // Symbol buttons layout
    QHBoxLayout *symbolsLayout = new QHBoxLayout();
    mainLayout->addLayout(symbolsLayout);

    // Initialize a signal mapper to map button clicks to specific slots
    QSignalMapper *signalMapper = new QSignalMapper(this);

    // List of symbols to create buttons for
    QStringList symbols = {"⇧", "↵", "⇑", "⇓"};
    foreach (const QString &symbol, symbols) {
        QPushButton *button = new QPushButton(symbol, this);
        button->setFixedSize(40, 40); // Makes buttons smaller
        symbolsLayout->addWidget(button);

        connect(button, SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(button, symbol);
    }

    connect(signalMapper, SIGNAL(mappedString(QString)), this, SLOT(insertSymbol(QString)));

    // Control buttons layout
    QHBoxLayout *controlButtonsLayout = new QHBoxLayout();
    mainLayout->addLayout(controlButtonsLayout);

    // Cancel button
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), this);
    controlButtonsLayout->addWidget(cancelButton);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // Accept button
    QPushButton *acceptButton = new QPushButton(tr("Save"), this);
    controlButtonsLayout->addWidget(acceptButton);
    connect(acceptButton, &QPushButton::clicked, this, &QDialog::accept);


}

void DialogTextChange::insertSymbol(const QString &symbol) {
    if (lineEdit->hasFocus())
        lineEdit->insert(symbol);
    else if (shiftLineEdit->hasFocus())
        shiftLineEdit->insert(symbol);
}

QString DialogTextChange::getText() const {
    return lineEdit->text();
}

QString DialogTextChange::getShiftText() const {
    return shiftLineEdit->text();
}
