#include "dialogtextchange.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSignalMapper>
#include <QKeyEvent>

DialogTextChange::DialogTextChange(QWidget *parent, QString currentText, QString currentShiftText) : QDialog(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(new QLabel(tr("Text (normal):")));
    lineEdit = new QPlainTextEdit(this);
    lineEdit->setMaximumBlockCount(5);
    lineEdit->setPlaceholderText(tr("Use ¶ or Ctrl+Enter for newlines"));
    mainLayout->addWidget(lineEdit);
    lineEdit->setPlainText(currentText);

    mainLayout->addWidget(new QLabel(tr("Text when Shift held:")));
    shiftLineEdit = new QPlainTextEdit(this);
    shiftLineEdit->setMaximumBlockCount(5);
    shiftLineEdit->setPlaceholderText(tr("Leave empty to use same as normal"));
    mainLayout->addWidget(shiftLineEdit);
    shiftLineEdit->setPlainText(currentShiftText);

    // Symbol buttons layout
    QHBoxLayout *symbolsLayout = new QHBoxLayout();
    mainLayout->addLayout(symbolsLayout);

    // Initialize a signal mapper to map button clicks to specific slots
    QSignalMapper *signalMapper = new QSignalMapper(this);

    // List of symbols to create buttons for (including pilcrow for newline)
    QStringList symbols = {"⇧", "↵", "⇑", "⇓", QString::fromUtf8("¶")};
    foreach (const QString &symbol, symbols) {
        QPushButton *button = new QPushButton(symbol, this);
        button->setFixedSize(40, 40);
        button->setFocusPolicy(Qt::NoFocus);
        symbolsLayout->addWidget(button);

        connect(button, SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(button, symbol == QString::fromUtf8("¶") ? QString("\n") : symbol);
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

void DialogTextChange::keyPressEvent(QKeyEvent *event) {
    if ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_Return) {
        if (QWidget *focus = focusWidget()) {
            if (focus == lineEdit || focus == shiftLineEdit) {
                static_cast<QPlainTextEdit*>(focus)->insertPlainText("\n");
                event->accept();
                return;
            }
        }
    }
    QDialog::keyPressEvent(event);
}

void DialogTextChange::insertSymbol(const QString &symbol) {
    if (lineEdit->hasFocus())
        lineEdit->insertPlainText(symbol);
    else if (shiftLineEdit->hasFocus())
        shiftLineEdit->insertPlainText(symbol);
}

QString DialogTextChange::getText() const {
    return lineEdit->toPlainText();
}

QString DialogTextChange::getShiftText() const {
    return shiftLineEdit->toPlainText();
}
