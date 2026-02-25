#ifndef DIALOGKEYCODECHANGE_H
#define DIALOGKEYCODECHANGE_H

#include <QDialog>
#include <QLineEdit>
#include <QKeyEvent>
#include "windowskeylistener.h"
#ifdef Q_OS_WIN
#include "windowsmouselistener.h"
#include "gamepadlistener.h"
#endif

class DialogKeycodeChange : public QDialog {
    Q_OBJECT

public:
    explicit DialogKeycodeChange(QWidget *parent = nullptr, std::list<int> currentKeyCodes = {},
        class WindowsKeyListener *mainKeyListener = nullptr,
#ifdef Q_OS_WIN
        class WindowsMouseListener *mainMouseListener = nullptr,
        class GamepadListener *mainGamepadListener = nullptr
#endif
    );
    ~DialogKeycodeChange();
    std::list<int> getKeyCodes() const;

private slots:
    void insertKeycode(const int keyCode);
    void setKeyCodes(const std::list<int> newKeyCodes);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    WindowsKeyListener* keyListener = nullptr;
    WindowsKeyListener* mainKeyListener = nullptr;
#ifdef Q_OS_WIN
    WindowsMouseListener* mouseListener = nullptr;
    WindowsMouseListener* mainMouseListener = nullptr;
    GamepadListener* mainGamepadListener = nullptr;
#endif
    std::list<int> keyCodes;

    void updateDisplay(QLineEdit *display);
};

#endif // DIALOGKEYCODECHANGE_H
