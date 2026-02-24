# coreBoard

A keyboard visualization and layout editor for streaming and recording. Display key presses on-screen in real time and design custom layouts—all with **full NohBoard layout compatibility**.

![License](https://img.shields.io/badge/license-GPL--3.0-blue)

---

## What is coreBoard?

For streaming, speedrunning, or video content, an on-screen keyboard overlay helps viewers see exactly which keys you're pressing. **coreBoard** gives you a fast, customizable overlay that works with OBS, Streamlabs, and similar tools.

You can use your existing **NohBoard layouts as-is**—no conversion needed. coreBoard reads and writes the same JSON format, so layouts are fully interchangeable.

> **Security note:** The app receives keyboard input for visualization. Do not type passwords while it is visible or while key capture is active. Use only in trusted setups.

---

## Features

| Feature | Description |
|--------|-------------|
| **Live overlay** | Real-time key press/release highlighting via Windows key listener |
| **Layout editor** | Rectangles, circles, stars, diamonds, hexagons, triangles, and advanced shapes (polygons with holes). Resize, rebind, and edit shapes (vertices, text anchor, corner radius). |
| **Copy / paste & style** | Copy a key then paste onto another; pick style from one key and apply to others |
| **Per-key colors** | Optional idle/pressed colors per key (key and text) in Edit Style |
| **NohBoard compatible** | Load and save the same `.json` layouts—switch seamlessly from NohBoard |
| **Undo/Redo** | Full history for layout and shape edits |
| **Languages** | English, Deutsch, Français (Settings → Language) |
| **Settings persistence** | Remembers last layout, recent files, and view/editor tab |
| **Cross-platform ready** | Qt-based; Windows supported, others possible |

---

## Quick Start

A **Windows installer** for the latest version is available under [Releases](https://github.com/RandomTick/coreBoard/releases).

### Build from source

**Requirements:** CMake 3.5+, Qt 5 or 6 (Widgets, Svg, LinguistTools), C++17 compiler

```bash
git clone https://github.com/RandomTick/coreBoard.git
cd coreBoard

cmake -B build -DCMAKE_PREFIX_PATH=<path-to-Qt>
cmake --build build
```

The executable is in `build/` (e.g. `build/CoreBoard.exe` on Windows). Run from the project root so layout files and resources are found.

### Run

1. **View mode** — Shows the current layout and highlights keys as you press them. Use this for streaming or recording.
2. **Editor tab** — Open, create, or edit layouts. Use **File → Open** to load any NohBoard `.json` layout, or start from the included default WASD layout.

---

## Usage

- **Open** — Load a `.json` layout (NohBoard format).
- **New** — Start a blank layout.
- **Add shape** — Rectangle, circle, star, diamond, hexagon, triangle, or advanced shapes (custom polygons, with optional holes). Assign text and key codes. Right‑click a key for **Edit style…**, **Edit shape…**, rebind, or delete.
- **Copy / Paste key** — Copy a key, then click where to paste. **Pick style** from a key, then **Apply** to others.
- **Save / Save as** — Write the layout to a JSON file (same format NohBoard uses).

Layout and window settings (last layout, recent layouts, tab) are restored on next launch.

---

## Switching from NohBoard?

See the **[NohBoard Migration Guide](docs/NOHBOARD_MIGRATION.md)** for a quick step-by-step guide. In short: your layouts work as-is.

---

## Project Structure

- **Keyboard widget** — Renders the layout and key states (including per-key colors)
- **Layout editor** — Scene-based editor: shapes, resize, rebind, shape editor (vertices, holes, text anchor), undo/redo
- **Layout settings** — Persists last layout, recent files, tab index
- **Windows key listener** — Feeds key press/release events to the overlay

---

## Contributing

Contributions welcome: bug reports, feature ideas, or pull requests. Translations (en/de/fr) are in `translations/`—help is appreciated there too.

---

## License

**coreBoard** is released under the [GNU General Public License v3.0](LICENSE).

## Acknowledgements

Inspired by **NohBoard**. coreBoard aims for full layout compatibility while adding a built-in editor and improved performance. We acknowledge NohBoard's work and build on it for streamers and content creators.
