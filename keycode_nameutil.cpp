#include "keycode_nameutil.h"
#include <QCoreApplication>
#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

QString keyCodeToDisplayName(int code)
{
#ifdef Q_OS_WIN
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
