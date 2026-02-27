#include "keycode_nameutil.h"
#include <QCoreApplication>
#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#include <windows.h>
#include "gamepadlistener.h"
#endif

#ifdef Q_OS_WIN
static const char *gamepadButtonName(int buttonIndex) {
    switch (buttonIndex) {
    case 0:  return "A";
    case 1:  return "B";
    case 2:  return "X";
    case 3:  return "Y";
    case 4:  return "LB";
    case 5:  return "RB";
    case 6:  return "Back";
    case 7:  return "Start";
    case 8:  return "L3";
    case 9:  return "R3";
    case 10: return "D-pad Up";
    case 11: return "D-pad Down";
    case 12: return "D-pad Left";
    case 13: return "D-pad Right";
    case 14: return "LT";
    case 15: return "RT";
    default: return "?";
    }
}
#endif

QString keyCodeToDisplayName(int code)
{
#ifdef Q_OS_WIN
    // Gamepad buttons (controller index encoded in code)
    if (code >= GAMEPAD_KEY_BASE && code < GAMEPAD_KEY_BASE + (4 << GAMEPAD_CONTROLLER_SHIFT)) {
        int ci = gamepadControllerIndex(code);
        int bi = gamepadButtonIndex(code);
        if (ci >= 0 && bi >= 0) {
            QString btn = QCoreApplication::translate("keycode_nameutil", gamepadButtonName(bi));
            return QCoreApplication::translate("keycode_nameutil", "%1 (Controller %2)").arg(btn).arg(ci + 1);
        }
    }

    // Mouse buttons (Windows VK codes)
    switch (code) {
    case 1:  return QCoreApplication::translate("keycode_nameutil", "Left Click (%1)").arg(code);
    case 2:  return QCoreApplication::translate("keycode_nameutil", "Right Click (%1)").arg(code);
    case 4:  return QCoreApplication::translate("keycode_nameutil", "Middle Click (%1)").arg(code);
    case 5:  return QCoreApplication::translate("keycode_nameutil", "Mouse 4 (%1)").arg(code);
    case 6:  return QCoreApplication::translate("keycode_nameutil", "Mouse 5 (%1)").arg(code);
    case VK_SCROLL_UP:   return QCoreApplication::translate("keycode_nameutil", "Scroll Up (%1)").arg(code);
    case VK_SCROLL_DOWN: return QCoreApplication::translate("keycode_nameutil", "Scroll Down (%1)").arg(code);
    default:
        break;
    }

    // Keyboard keys: use Windows GetKeyNameText
    if (code >= 0x08 && code <= 0xFE) {
        UINT vk = static_cast<UINT>(code);
        UINT sc = MapVirtualKeyExW(vk, MAPVK_VK_TO_VSC, nullptr);
        if (sc != 0) {
            // Extended keys need the extended bit (0x100) for correct names
            switch (vk) {
            case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
            case VK_PRIOR: case VK_NEXT: case VK_END: case VK_HOME:
            case VK_INSERT: case VK_DELETE:
            case VK_DIVIDE: case VK_NUMLOCK:
                sc |= 0x100;
                break;
            default:
                break;
            }
            wchar_t buf[64];
            if (GetKeyNameTextW(static_cast<LONG>(sc << 16), buf, 64) > 0) {
                QString name = QString::fromWCharArray(buf).trimmed();
                if (!name.isEmpty()) {
                    return name + QStringLiteral(" (") + QString::number(code) + QLatin1Char(')');
                }
            }
        }
    }

    // Unknown/fallback
    return QStringLiteral("(%1)").arg(code);
#else
    return QString::number(code);
#endif
}
