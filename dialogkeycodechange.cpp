#include "DialogKeycodeChange.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

DialogKeycodeChange::DialogKeycodeChange(QWidget *parent, std::list<int> currentKeyCodes, WindowsKeyListener *mainKeyListener)
    : QDialog(parent), keyCodes(currentKeyCodes), mainKeyListener(mainKeyListener)
{
#ifdef Q_OS_WIN
    keyListener = mainKeyListener ? mainKeyListener : new WindowsKeyListener(this);
#endif

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

    QPushButton *addButton = new QPushButton(tr("Add Key Code"), this);
    controlButtonsLayout->addWidget(addButton);

    QPushButton *clearButton = new QPushButton(tr("Clear Key Codes"), this);
    controlButtonsLayout->addWidget(clearButton);

    QPushButton *okButton = new QPushButton(tr("Save"), this);
    controlButtonsLayout->addWidget(okButton);


    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    connect(addButton, &QPushButton::clicked, this, [this, keyCodesDisplay, addButton]() {
        if (!keyListener) return;
        addButton->setEnabled(false);
        addButton->setText(tr("Press a key..."));
        setFocus();
        connect(keyListener, &WindowsKeyListener::keyPressed,
            this, [this, keyCodesDisplay, addButton](int keyCode) {
                insertKeycode(keyCode);
                updateDisplay(keyCodesDisplay);
                addButton->setEnabled(true);
                addButton->setText(tr("Add Key Code"));
            }, Qt::SingleShotConnection);
        if (!this->mainKeyListener)
            keyListener->startListening();
    });

    connect(clearButton, &QPushButton::clicked, this, [this, keyCodesDisplay]() {
        keyCodes.clear();
        updateDisplay(keyCodesDisplay);
    });

    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

    updateDisplay(keyCodesDisplay);
}

DialogKeycodeChange::~DialogKeycodeChange()
{
#ifdef Q_OS_WIN
    if (mainKeyListener) {
        mainKeyListener->setAsGlobalInstance();
    } else if (keyListener) {
        keyListener->stopListening();
    }
#endif
}

void DialogKeycodeChange::keyPressEvent(QKeyEvent *event)
{
    //Ignore all events
    return;

    //theoretically limit on escape, enter, numpad enter is necessary but no key should be needed
    if (event->key() != Qt::Key_Escape &&
        event->key() != Qt::Key_Enter)
    {
        // Pass the event to the base class

    }
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
