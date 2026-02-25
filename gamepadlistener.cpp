#include "gamepadlistener.h"
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#include <xinput.h>

#if defined(_MSC_VER)
#pragma comment(lib, "xinput.lib")
#endif

namespace {

const int POLL_MS = 15;
const WORD BUTTON_BITS[] = {
    XINPUT_GAMEPAD_A,      // 0
    XINPUT_GAMEPAD_B,      // 1
    XINPUT_GAMEPAD_X,      // 2
    XINPUT_GAMEPAD_Y,      // 3
    XINPUT_GAMEPAD_LEFT_SHOULDER,  // 4 LB
    XINPUT_GAMEPAD_RIGHT_SHOULDER, // 5 RB
    XINPUT_GAMEPAD_BACK,   // 6
    XINPUT_GAMEPAD_START,  // 7
    XINPUT_GAMEPAD_LEFT_THUMB,   // 8 L3
    XINPUT_GAMEPAD_RIGHT_THUMB,  // 9 R3
    XINPUT_GAMEPAD_DPAD_UP,      // 10
    XINPUT_GAMEPAD_DPAD_DOWN,    // 11
    XINPUT_GAMEPAD_DPAD_LEFT,    // 12
    XINPUT_GAMEPAD_DPAD_RIGHT,   // 13
};
const int NUM_BUTTONS = 14;

const SHORT DEADZONE = 7843;  // ~24% of 32767; XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE is 7843

static inline qreal normalizeThumb(SHORT v) {
    if (v > 0 && v < DEADZONE) return 0;
    if (v < 0 && v > -DEADZONE) return 0;
    return qBound(-1.0, v / 32767.0, 1.0);
}

} // namespace

class GamepadListener::Private {
public:
    QTimer *timer = nullptr;
    WORD prevButtons[4] = {0, 0, 0, 0};
    bool prevConnected[4] = {false, false, false, false};
};

GamepadListener::GamepadListener(QObject *parent) : QObject(parent)
{
    d = new Private;
    d->timer = new QTimer(this);
    connect(d->timer, &QTimer::timeout, this, [this]() {
        for (int ci = 0; ci < 4; ++ci) {
            XINPUT_STATE state = {};
            DWORD res = XInputGetState(ci, &state);
            bool connected = (res == ERROR_SUCCESS);
            d->prevConnected[ci] = connected;
            if (res != ERROR_SUCCESS)
                continue;

            WORD cur = state.Gamepad.wButtons;
            for (int bi = 0; bi < NUM_BUTTONS; ++bi) {
                bool nowDown = (cur & BUTTON_BITS[bi]) != 0;
                bool wasDown = (d->prevButtons[ci] & BUTTON_BITS[bi]) != 0;
                if (nowDown != wasDown) {
                    int code = gamepadCode(ci, bi);
                    if (nowDown)
                        emit keyPressed(code);
                    else
                        emit keyReleased(code);
                }
            }
            d->prevButtons[ci] = cur;

            qreal lx = normalizeThumb(state.Gamepad.sThumbLX);
            qreal ly = -normalizeThumb(state.Gamepad.sThumbLY);  // Y up = positive
            qreal rx = normalizeThumb(state.Gamepad.sThumbRX);
            qreal ry = -normalizeThumb(state.Gamepad.sThumbRY);
            emit leftStickChanged(ci, lx, ly);
            emit rightStickChanged(ci, rx, ry);
        }
    });
}

GamepadListener::~GamepadListener()
{
    stopPolling();
    delete d;
    d = nullptr;
}

void GamepadListener::startPolling()
{
    if (!d->timer->isActive())
        d->timer->start(POLL_MS);
}

void GamepadListener::stopPolling()
{
    d->timer->stop();
}

QList<int> GamepadListener::connectedControllerIndices() const
{
    QList<int> list;
    for (int ci = 0; ci < 4; ++ci) {
        XINPUT_STATE state = {};
        if (XInputGetState(ci, &state) == ERROR_SUCCESS)
            list.append(ci);
    }
    return list;
}

#endif // Q_OS_WIN