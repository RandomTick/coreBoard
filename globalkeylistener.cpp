#include "globalkeylistener.h"
#include <windows.h>

HHOOK hHook;
GlobalKeyListener* instance = nullptr;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && instance) {
        KBDLLHOOKSTRUCT *pkbhs = (KBDLLHOOKSTRUCT *)lParam;
        if (wParam == WM_KEYDOWN) {
            emit instance->keyPressed(pkbhs->vkCode);
        } else if (wParam == WM_KEYUP) {
            emit instance->keyReleased(pkbhs->vkCode);
        }
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

GlobalKeyListener::GlobalKeyListener(QObject *parent) : QObject(parent) {
    // Constructor
    instance = this;
}

GlobalKeyListener::~GlobalKeyListener() {
    stopListening(); // Ensure the hook is released
    instance = nullptr;
}

void GlobalKeyListener::startListening() {
    if (!hHook) {
        hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, nullptr, 0);
        if (!hHook) {
            // Handle error
            qWarning("Failed to start listening to keyboard");
        }else{
            qDebug("Startet listening");
        }
    }
}

void GlobalKeyListener::stopListening() {
    if (hHook) {
        UnhookWindowsHookEx(hHook);
        hHook = nullptr;
    }
}
