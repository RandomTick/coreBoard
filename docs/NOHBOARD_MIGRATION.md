# Switching from NohBoard to coreBoard

coreBoard is **fully compatible** with NohBoard layouts. You can use the same `.json` layout files without conversion.

---

## Quick steps

1. **Install coreBoard**  
   Build from source or use a released installer.

2. **Copy your layouts**  
   NohBoard typically stores layouts in its install folder or your user data. Copy your `.json` layout files to any folder you like.

3. **Open in coreBoard**  
   Launch coreBoard → switch to **Editor** tab → **Open** → select your NohBoard layout file.

4. **Done**  
   Your layout loads as-is. Keys, shapes, fonts, and positions are preserved.

---

## Using in OBS (or similar)

1. Add **Window Capture** or **Game Capture** as a source.
2. Select the coreBoard window (use **View** mode for the clean overlay).
3. Position and resize as needed.
4. Optionally make the window background transparent in coreBoard's settings if supported.

Same workflow as NohBoard; only the application changes.

---

## Layout compatibility

coreBoard reads and writes the same JSON structure as NohBoard:

- Same `Elements` array with `Boundaries`, `KeyCodes`, `Text`, `FontFamily`, etc.
- Same shapes: rectangles, ellipses, polygons
- Same visual and key-binding properties

Layouts can be edited in either app and used in both.

---

## Differences to expect

| NohBoard | coreBoard |
|----------|-----------|
| Standalone viewer + external editor | Viewer and editor built into one app |
| — | Built-in undo/redo in the editor |
| — | Recent files list and layout persistence |

Your layouts will look and behave the same. coreBoard is a drop-in replacement that keeps your workflow intact.
