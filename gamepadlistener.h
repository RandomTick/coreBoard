#ifndef GAMEPADLISTENER_H
#define GAMEPADLISTENER_H

#include <QObject>

#ifdef Q_OS_WIN

/**
 * Polls up to 4 XInput gamepads. Emits keyPressed/keyReleased with codes that encode
 * controller index so multiple controllers can be bound (e.g. speedruns with 2 pads).
 * Code = GAMEPAD_KEY_BASE + (controllerIndex << 8) + buttonIndex.
 * Also emits leftStickChanged/rightStickChanged(controllerIndex, x, y) for angular viewers.
 */
class GamepadListener : public QObject {
    Q_OBJECT

public:
    explicit GamepadListener(QObject *parent = nullptr);
    ~GamepadListener();

    void startPolling();
    void stopPolling();

    /// Currently connected controller indices (0..3). Optional for "Select controller" UI.
    QList<int> connectedControllerIndices() const;

signals:
    void keyPressed(int code);
    void keyReleased(int code);
    void leftStickChanged(int controllerIndex, qreal x, qreal y);
    void rightStickChanged(int controllerIndex, qreal x, qreal y);
    /// Analog triggers 0..1 per controller. Emitted every poll for keys that show fill-from-bottom.
    void triggersChanged(int controllerIndex, qreal leftTrigger, qreal rightTrigger);

private:
    class Private;
    Private *d = nullptr;
};

// Gamepad key code: 0x2000 + (controllerIndex << 8) + buttonIndex (0..15; 14=LT, 15=RT)
constexpr int GAMEPAD_KEY_BASE = 0x2000;
constexpr int GAMEPAD_CONTROLLER_SHIFT = 8;
constexpr int GAMEPAD_BUTTON_MASK = 0xFF;
constexpr int GAMEPAD_LEFT_TRIGGER_BUTTON = 14;
constexpr int GAMEPAD_RIGHT_TRIGGER_BUTTON = 15;

inline int gamepadCode(int controllerIndex, int buttonIndex) {
    return GAMEPAD_KEY_BASE + ((controllerIndex & 3) << GAMEPAD_CONTROLLER_SHIFT) + (buttonIndex & GAMEPAD_BUTTON_MASK);
}
inline int gamepadControllerIndex(int code) {
    if (code < GAMEPAD_KEY_BASE || code >= GAMEPAD_KEY_BASE + (4 << GAMEPAD_CONTROLLER_SHIFT)) return -1;
    return (code - GAMEPAD_KEY_BASE) >> GAMEPAD_CONTROLLER_SHIFT;
}
inline int gamepadButtonIndex(int code) {
    if (code < GAMEPAD_KEY_BASE) return -1;
    return (code - GAMEPAD_KEY_BASE) & GAMEPAD_BUTTON_MASK;
}
inline bool isGamepadTriggerCode(int code) {
    int bi = gamepadButtonIndex(code);
    return bi == GAMEPAD_LEFT_TRIGGER_BUTTON || bi == GAMEPAD_RIGHT_TRIGGER_BUTTON;
}

#endif // Q_OS_WIN

#endif // GAMEPADLISTENER_H
