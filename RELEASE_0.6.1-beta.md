# coreBoard 0.6.1-beta

**Release date:** 2025-02-25

## Highlights

This beta adds **text alignment** for keys and labels, a **single anchor** for text position (editor and visualization match), and fixes for **italic clipping** and **label alignment** application.

---

## New features

### Text alignment (keys and labels)

- **Left / Center / Right** alignment for:
  - Key labels (text on keyboard shapes)
  - Standalone labels (Label elements)
- Alignment is available in **Edit style** for keys and in **Edit label style** for labels.
- Alignment is stored in the layout JSON (`TextAlignment`: `"left"`, `"center"`, or `"right"`) and applied in both the editor and the live visualization.
- When switching base/shift labels, text no longer jumps; alignment and fixed width keep the anchor stable.

### Single point of truth for text position

- The **draggable text anchor** in **Edit shape** (the dot you move for key text) is now the single reference point:
  - **Center:** anchor = center of the text
  - **Left:** anchor = left edge of the text
  - **Right:** anchor = right edge of the text
- Editor, Edit-shape handle, and live visualization all use the same rule, so positions match everywhere.
- **Standalone labels** use the same idea: (X, Y) in the layout is the anchor (center/left/right by alignment). Dragging a label moves the anchor.

### Standalone label improvements

- Labels use **anchor semantics** and a **fixed width** (max of base/shift text) so they don’t shift when toggling shift.
- Label **alignment** (Left/Center/Right) can be set in Edit label style and now **persists** when applied.
- Saving a label stores the **anchor** position; editor and visualization render labels in the same place.

---

## Bug fixes

- **Italic + right alignment:** Right-aligned italic text was clipped on the right. Extra space is now added for italic so the slant is no longer cut off (keys and editor).
- **Label style alignment:** Choosing Left or Right in Edit label style and clicking Apply now correctly applies and keeps the chosen alignment (previously it reverted to Center).
- **Polygon build:** Fixed a variable redeclaration in `resizablepolygonitem.cpp` that could break the build.

---

## Technical notes

- **KeyStyle** includes `textAlignment` (0=left, 1=center, 2=right) with JSON `TextAlignment`.
- **LabelItem** keeps an anchor in scene coords and updates it when the label is dragged; `anchorScenePos()` is used when saving.
- Visualization label overlays and key text position are computed from the same anchor and alignment rules as the editor.

---

## Downloads

See [Releases](https://github.com/RandomTick/coreBoard/releases) for Windows installers and source archives.
