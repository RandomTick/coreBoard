#include "windowskeylistener.h"
#include <windows.h>
//#include <iostream>

HHOOK hHook;
WindowsKeyListener* instance = nullptr;
static bool leftShiftDown = false;
static bool rightShiftDown = false;

static bool isShiftKey(UINT vkCode) {
    return vkCode == VK_LSHIFT || vkCode == VK_RSHIFT;
}

static bool isCapsLockKey(UINT vkCode) {
    return vkCode == VK_CAPITAL;
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && instance) {
        KBDLLHOOKSTRUCT *pkbhs = (KBDLLHOOKSTRUCT *)lParam;
        UINT vk = pkbhs->vkCode;
        if (isCapsLockKey(vk) && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
            bool capsOn = (GetKeyState(VK_CAPITAL) & 1) != 0;
            emit instance->capsLockStateChanged(capsOn);
        }
        if (isShiftKey(vk)) {
            bool wasPressed = leftShiftDown || rightShiftDown;
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                if (vk == VK_LSHIFT)
                    leftShiftDown = true;
                else
                    rightShiftDown = true;
                if (!wasPressed)
                    emit instance->shiftStateChanged(true);
            } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                if (vk == VK_LSHIFT)
                    leftShiftDown = false;
                else
                    rightShiftDown = false;
                if (wasPressed && !leftShiftDown && !rightShiftDown)
                    emit instance->shiftStateChanged(false);
            }
        }
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if(pkbhs->vkCode == VK_RETURN) {
                if ((pkbhs->flags & LLKHF_EXTENDED) == LLKHF_EXTENDED) {
                    emit instance->keyPressed(VK_SEPARATOR);
                } else {
                    emit instance->keyPressed(VK_RETURN);
                }
            } else {
                emit instance->keyPressed(pkbhs->vkCode);
            }
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            if(pkbhs->vkCode == VK_RETURN) {
                if ((pkbhs->flags & LLKHF_EXTENDED) == LLKHF_EXTENDED) {
                    emit instance->keyReleased(VK_SEPARATOR);
                } else {
                    emit instance->keyReleased(VK_RETURN);
                }
            } else {
                emit instance->keyReleased(pkbhs->vkCode);
            }
        }
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}


WindowsKeyListener::WindowsKeyListener(QObject *parent) : QObject(parent) {
    // Constructor
    instance = this;
}

WindowsKeyListener::~WindowsKeyListener() {
    if (instance == this) {
        stopListening();
        instance = nullptr;
    }
}

void WindowsKeyListener::setAsGlobalInstance() {
    instance = this;
}

void WindowsKeyListener::startListening() {
    if (!hHook) {
        hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, nullptr, 0);
        if (hHook) {
            bool capsOn = (GetKeyState(VK_CAPITAL) & 1) != 0;
            emit instance->capsLockStateChanged(capsOn);
        } else {
            // Handle error
            qWarning("Failed to start listening to keyboard");
        }
    }
}

void WindowsKeyListener::stopListening() {
    if (hHook) {
        //qDebug("Stop listening");
        UnhookWindowsHookEx(hHook);
        hHook = nullptr;
    }
}
