#include "windowsmouselistener.h"
#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#include "keycode_nameutil.h"
#include <windows.h>
#include <QTimer>

static HHOOK hMouseHook = nullptr;
static WindowsMouseListener *mouseInstance = nullptr;

static LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && mouseInstance) {
        if (wParam == WM_LBUTTONDOWN) {
            mouseInstance->keyPressed(1);
        } else if (wParam == WM_LBUTTONUP) {
            mouseInstance->keyReleased(1);
        } else if (wParam == WM_RBUTTONDOWN) {
            mouseInstance->keyPressed(2);
        } else if (wParam == WM_RBUTTONUP) {
            mouseInstance->keyReleased(2);
        } else if (wParam == WM_MBUTTONDOWN) {
            mouseInstance->keyPressed(4);
        } else if (wParam == WM_MBUTTONUP) {
            mouseInstance->keyReleased(4);
        } else if (wParam == WM_XBUTTONDOWN || wParam == WM_XBUTTONUP) {
            UINT button = GET_XBUTTON_WPARAM(wParam);
            int vk = (button == XBUTTON1) ? 5 : 6;
            if (wParam == WM_XBUTTONDOWN) {
                mouseInstance->keyPressed(vk);
            } else {
                mouseInstance->keyReleased(vk);
            }
        } else if (wParam == WM_MOUSEWHEEL) {
            MSLLHOOKSTRUCT *pmhs = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
            short delta = GET_WHEEL_DELTA_WPARAM(pmhs->mouseData);
            int code = (delta > 0) ? VK_SCROLL_UP : VK_SCROLL_DOWN;
            mouseInstance->keyPressed(code);
            QTimer::singleShot(150, mouseInstance, [code]() {
                if (mouseInstance)
                    mouseInstance->keyReleased(code);
            });
        }
    }
    return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

WindowsMouseListener::WindowsMouseListener(QObject *parent) : QObject(parent)
{
    mouseInstance = this;
}

WindowsMouseListener::~WindowsMouseListener()
{
    if (mouseInstance == this) {
        stopListening();
        mouseInstance = nullptr;
    }
}

void WindowsMouseListener::setAsGlobalInstance()
{
    mouseInstance = this;
}

void WindowsMouseListener::startListening()
{
    if (!hMouseHook) {
        hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, nullptr, 0);
        if (!hMouseHook) {
            qWarning("Failed to start listening to mouse");
        }
    }
}

void WindowsMouseListener::stopListening()
{
    if (hMouseHook) {
        UnhookWindowsHookEx(hMouseHook);
        hMouseHook = nullptr;
    }
}

#endif // Q_OS_WIN
