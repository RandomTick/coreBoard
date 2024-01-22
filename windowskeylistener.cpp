#include "windowskeylistener.h"
#include <windows.h>
#include <iostream>

HHOOK hHook;
WindowsKeyListener* instance = nullptr;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && instance) {
        KBDLLHOOKSTRUCT *pkbhs = (KBDLLHOOKSTRUCT *)lParam;
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if(pkbhs->vkCode == VK_RETURN) {
                if ((pkbhs->flags & LLKHF_EXTENDED) == LLKHF_EXTENDED) {
                    //std::cout << "Numpad Enter Pressed" << std::endl;
                    emit instance->keyPressed(VK_SEPARATOR);
                } else {
                    //std::cout << "Regular Enter Pressed" << std::endl;
                    emit instance->keyPressed(VK_RETURN);
                }
            } else {
                std::cout << "Key Pressed: " << pkbhs->vkCode << std::endl;
                emit instance->keyPressed(pkbhs->vkCode);
            }
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            if(pkbhs->vkCode == VK_RETURN) {
                if ((pkbhs->flags & LLKHF_EXTENDED) == LLKHF_EXTENDED) {
                    //std::cout << "Numpad Enter Released" << std::endl;
                    emit instance->keyReleased(VK_SEPARATOR);
                } else {
                    //std::cout << "Regular Enter Released" << std::endl;
                    emit instance->keyReleased(VK_RETURN);
                }
            } else {
                //std::cout << "Key Released: " << pkbhs->vkCode << std::endl;
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
    stopListening(); // Ensure the hook is released
    instance = nullptr;
}

void WindowsKeyListener::startListening() {
    if (!hHook) {

        hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, nullptr, 0);
        if (!hHook) {
            // Handle error
            qWarning("Failed to start listening to keyboard");
        }else{
            //qDebug("Startet listening");
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
