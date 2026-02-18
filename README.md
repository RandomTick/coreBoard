# coreBoard

**coreBoard** is a keyboard visualization and layout editor for streaming and recording. It shows key presses on-screen in real time and lets you design custom keyboard layouts. Built with C++ and Qt for performance and cross-platform use.

## Overview

For streaming, speedrunning, or video, an on-screen keyboard display helps viewers see inputs. **coreBoard** provides a fast, customizable keyboard overlay that you can capture in OBS or similar tools. It uses NohBoard-compatible JSON layouts so you can reuse or edit existing layouts.

**Note:** The application receives keyboard input for visualization. Do not type passwords while it is visible or while a key-capture is active; use it only in trusted setups.

## Features

- **Live keyboard view** — Real-time key press/release highlighting (Windows key listener).
- **Layout editor** — Create and edit keyboard layouts with:
  - **Shapes:** rectangles, ellipses, and polygons.
  - **Resize** — Drag handles to resize and move items.
  - **Key bindings** — Assign key codes to each key shape.
  - **Undo/Redo** — Full undo/redo for edits.
  - **Save/Load/New** — JSON layout files (NohBoard-compatible).
- **Layout settings** — Remembers last layout, recent files (e.g. last 5), and last tab (View vs Editor).
- **Cross-platform** — Qt-based; currently targeting Windows, with potential for other platforms.

## Building

### Requirements

- **CMake** 3.5 or newer  
- **Qt** 5 or 6 with components: **Widgets**, **LinguistTools**  
- C++17-capable compiler (e.g. MSVC, GCC, Clang)

### Build steps

Do **not** commit the `build` directory; it is ignored and should be generated locally.

```bash
# Clone and enter the project
git clone <repository-url>
cd coreBoard

# Configure (use your Qt path if needed)
cmake -B build -DCMAKE_PREFIX_PATH=<path-to-Qt>

# Build
cmake --build build
```

The executable is produced inside `build/` (e.g. `build/CoreBoard.exe` on Windows). Run it from the project root so it can find layout files and resources.

### CMake options

- `CMAKE_PREFIX_PATH` — Path to Qt (e.g. `C:/Qt/6.5.0/msvc2019_64` or `/usr/lib/qt6`).

## Usage

1. **View mode** — Shows the current layout and highlights keys as you press them. Use this for streaming or recording.
2. **Layout editor** — Switch to the editor to create or edit a layout:
   - **Open** — Load a `.json` layout (NohBoard format).
   - **New** — Start a blank layout.
   - **Add** — Add a rectangle, ellipse, or polygon; set text and key codes.
   - **Save / Save as** — Write the layout to a JSON file.

Layout and window settings (last layout, recent layouts, last tab) are stored and restored on the next run.

## Project structure (summary)

- **Keyboard widget** — Renders the layout and key states.
- **Layout editor** — Scene-based editor (rectangles, ellipses, polygons) with undo/redo.
- **Layout settings** — Persists last layout path, recent layouts, and tab index.
- **Windows key listener** — Feeds key press/release events to the keyboard widget.

## Contributing

Contributions are welcome: bug reports, feature ideas, or code changes. Translation scaffolding (Qt Linguist) exists for en/fr/de but strings are not yet translated—contributions there are welcome too. Please open an issue or pull request.

## License

**coreBoard** is released under the [GNU General Public License v3.0](LICENSE). You may use, modify, and distribute it under the terms of the license.

## Acknowledgements

Inspired by **NohBoard**. coreBoard aims to be compatible with NohBoard-style JSON layouts while adding a built-in editor and improved performance. We acknowledge NohBoard’s work and build on it for streamers and content creators.
