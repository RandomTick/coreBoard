#include "DialogKeycodeChange.h"
#include "keycode_nameutil.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

DialogKeycodeChange::DialogKeycodeChange(QWidget *parent, std::list<int> currentKeyCodes,
    WindowsKeyListener *mainKeyListener
#ifdef Q_OS_WIN
    , WindowsMouseListener *mainMouseListenerParam,
    GamepadListener *mainGamepadListenerParam
#endif
)
    : QDialog(parent), keyCodes(currentKeyCodes), mainKeyListener(mainKeyListener)
#ifdef Q_OS_WIN
    , mainMouseListener(mainMouseListenerParam),
    mainGamepadListener(mainGamepadListenerParam)
#endif
{
#ifdef Q_OS_WIN
    keyListener = mainKeyListener ? mainKeyListener : new WindowsKeyListener(this);
    mouseListener = mainMouseListener ? mainMouseListener : new WindowsMouseListener(this);
#endif

    setWindowTitle(tr("Change Key Codes"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *label = new QLabel(tr("Current Key Codes:"), this);
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
#ifdef Q_OS_WIN
        if (!keyListener && !mouseListener) return;
#else
        if (!keyListener) return;
#endif
        addButton->setEnabled(false);
        addButton->setText(tr("Press a key or mouse button..."));
        setFocus();
        auto finishAdd = [this, keyCodesDisplay, addButton](int keyCode) {
            if (keyListener)
                disconnect(keyListener, &WindowsKeyListener::keyPressed, this, nullptr);
#ifdef Q_OS_WIN
            if (mouseListener)
                disconnect(mouseListener, &WindowsMouseListener::keyPressed, this, nullptr);
            if (mainGamepadListener)
                disconnect(mainGamepadListener, &GamepadListener::keyPressed, this, nullptr);
            if (mouseListener && !this->mainMouseListener)
                mouseListener->stopListening();
#endif
            if (keyListener && !this->mainKeyListener)
                keyListener->stopListening();
            insertKeycode(keyCode);
            updateDisplay(keyCodesDisplay);
            addButton->setEnabled(true);
            addButton->setText(tr("Add Key Code"));
        };
        if (keyListener) {
            connect(keyListener, &WindowsKeyListener::keyPressed,
                this, finishAdd, Qt::SingleShotConnection);
            if (!this->mainKeyListener)
                keyListener->startListening();
        }
#ifdef Q_OS_WIN
        if (mouseListener) {
            connect(mouseListener, &WindowsMouseListener::keyPressed,
                this, finishAdd, Qt::SingleShotConnection);
            if (!this->mainMouseListener)
                mouseListener->startListening();
        }
        if (mainGamepadListener) {
            connect(mainGamepadListener, &GamepadListener::keyPressed,
                this, finishAdd, Qt::SingleShotConnection);
        }
#endif
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
    if (mainMouseListener) {
        mainMouseListener->setAsGlobalInstance();
    } else if (mouseListener) {
        mouseListener->stopListening();
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
    QStringList parts;
    for (const auto &code : keyCodes) {
        parts << keyCodeToDisplayName(code);
    }
    display->setText(parts.join(QStringLiteral("  ")));
}
