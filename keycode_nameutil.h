#ifndef KEYCODE_NAMEUTIL_H
#define KEYCODE_NAMEUTIL_H

#include <QString>

// Custom virtual key codes for scroll wheel (not Windows VKs)
constexpr int VK_SCROLL_UP   = 0x1000;
constexpr int VK_SCROLL_DOWN = 0x1001;

/**
 * Converts a key/mouse code to a human-readable display string.
 * Format: "Name (code)" e.g. "Space (32)", "Left Click (1)".
 * Uses Windows API for keyboard keys, static map for mouse and scroll.
 */
QString keyCodeToDisplayName(int code);

#endif // KEYCODE_NAMEUTIL_H
