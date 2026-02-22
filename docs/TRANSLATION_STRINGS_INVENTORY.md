# coreBoard – Complete User-Facing Strings Inventory

This document lists **all** user-facing strings in the application for translation. Use it as a reference when adding or updating translations.

---

## 1. Main Window (mainwindow.ui)

| ID / Location | Type | English (Source) |
|---------------|------|------------------|
| window title | windowTitle | CoreBoard |
| menuView | menu title | View |
| menuSettings | menu title | Settings |
| actionView | action | Visualization |
| actionEdit | action | Edit Layout |
| actionColors | action | Colors... |
| actionLabelDisplay | action | Label display... |
| actionAbout | action | Info |

---

## 2. Main Window (mainwindow.cpp)

| Location | Type | English (Source) |
|----------|------|------------------|
| onAbout | QMessageBox title | About coreBoard |
| onAbout | body | coreBoard description, GPL-3.0, GitHub URL, Version %1 |
| confirmLeaveEditor | QMessageBox title | Unsaved changes |
| confirmLeaveEditor | body | Save changes to the layout before switching? |
| closeEvent | QMessageBox title | Unsaved changes |
| closeEvent | body | Save changes to the layout before closing? |

Note: QMessageBox buttons (Save, Discard, Cancel) are Qt standard and translated via Qt's own translations if available.

---

## 3. Layout Editor (layouteditor.cpp)

| Location | Type | English (Source) |
|----------|------|------------------|
| openButton | button | Open Layout |
| newButton | button | New Layout |
| addButton | button | Add Shape |
| pickStyleButton | tooltip | Pick style from a key |
| applyStyleButton | tooltip | Apply picked style to key(s) |
| addShapeMenu | menu item | Rectangle |
| addShapeMenu | menu item | Circle |
| addShapeMenu | submenu | Custom shape |
| customShapeMenu | menu item | Star |
| customShapeMenu | menu item | Diamond |
| customShapeMenu | menu item | Hexagon |
| customShapeMenu | menu item | Triangle |
| saveButton | button | Save |
| saveAsButton | button | Save As |
| loadLayoutButton menu | menu item | Open... |
| loadLayoutButton menu | menu item | No recent files |
| QFileDialog (load) | title | Select Layout File to Edit |
| QFileDialog (load) | filter | Layout Files (*.json);;All Files (*) |
| QFileDialog (save) | title | Save Layout As |
| QFileDialog (save) | filter | Layout Files (*.json);;All Files (*) |
| loadLayout QMessageBox | title | Layout not found |
| loadLayout QMessageBox | body | The layout file could not be opened. It may have been moved or deleted.\n\n"%1"\n\nIt has been removed from the recent list. |

---

## 4. Layout Editor Graphics View (layouteditorgraphicsview.cpp)

| Location | Type | English (Source) |
|----------|------|------------------|
| context menu | menu item | Rename |
| context menu | menu item | Rebind |
| context menu | menu item | Edit style... |
| context menu | menu item | Delete |
| alignment/size helpers | suffix |  px (e.g. "42 px") |

Note: " px" appears in drawAlignmentLine and updateSizeHelpers. Consider `tr("%1 px").arg(value)` if localizing unit suffixes.

---

## 5. Settings Dialog – Colors (settingsdialog.cpp)

| Location | Type | English (Source) |
|----------|------|------------------|
| setWindowTitle | title | Colors |
| m_keyColorButton | button | Choose... |
| m_highlightColorButton | button | Choose... |
| m_backgroundColorButton | button | Choose... |
| m_textColorButton | button | Choose... |
| m_highlightedTextColorButton | button | Choose... |
| label | label | Key color: |
| label | label | Highlight color: |
| label | label | Background color: |
| label | label | Text color: |
| label | label | Highlighted text color: |
| QColorDialog | title | Key color |
| QColorDialog | title | Highlight color |
| QColorDialog | title | Background color |
| QColorDialog | title | Text color |
| QColorDialog | title | Highlighted text color |

Note: QDialogButtonBox Ok/Cancel are Qt standard.

---

## 6. Label Settings Dialog (labelsettingsdialog.cpp)

| Location | Type | English (Source) |
|----------|------|------------------|
| setWindowTitle | title | Label display |
| m_followCapsAndShift | radio | Follow Caps Lock and Shift |
| m_allUppercase | radio | All uppercase |
| m_allLowercase | radio | All lowercase |

---

## 7. Edit Style Dialog (dialogstyle.cpp)

| Location | Type | English (Source) |
|----------|------|------------------|
| kFontSampleText | constant | Sample 123 (font preview in list) |
| setWindowTitle | title | Edit style |
| m_outlineColorButton | button | Outline color... |
| form label | label | Outline color: |
| form label | label | Outline width (px): |
| form label | label | Font size: |
| form label | label | Bold: |
| form label | label | Italic: |
| m_fontCombo | combo item | (Default) |
| form label | label | Font family: |
| cancelBtn | button | Cancel |
| applyBtn | button | Apply |
| QColorDialog | title | Outline color |

---

## 8. Text Change Dialog (dialogtextchange.cpp)

| Location | Type | English (Source) |
|----------|------|------------------|
| label | label | Text (normal): |
| label | label | Text when Shift held: |
| shiftLineEdit | placeholder | Leave empty to use same as normal |
| cancelButton | button | Cancel |
| acceptButton | button | Save |

Note: Symbol buttons (⇧ ↵ ⇑ ⇓) are Unicode symbols and typically not translated.

---

## 9. Key Code Change Dialog (dialogkeycodechange.cpp)

| Location | Type | English (Source) |
|----------|------|------------------|
| setWindowTitle | title | Change Key Codes |
| label | label | Current Key Codes: |
| cancelButton | button | Cancel |
| addButton | button | Add Key Code |
| addButton (waiting) | button | Press a key... |
| clearButton | button | Clear Key Codes |
| okButton | button | Save |

---

## 10. Language Settings Dialog (NEW – languagesettingsdialog)

| Location | Type | English (Source) |
|----------|------|------------------|
| setWindowTitle | title | Language |
| label | label | Interface language: |
| combo item | option | English |
| combo item | option | Deutsch |

---

## 11. Main Menu – Language action (NEW)

| ID | Type | English (Source) |
|----|------|------------------|
| actionLanguage | action | Language... |

---

## 12. version_info.ui (if used at runtime)

| Location | Type | English (Source) |
|----------|------|------------------|
| windowTitle | title | MainWindow |
| labels | label | TextLabel |
| menu | title | Config |
| menu | title | Info |
| action | text | Layout |
| action | text | Binding |

---

## 13. versioninfo.h

| Constant | Value |
|----------|-------|
| APP_VERSION | 0.2.0-alpha |

---

## 14. Qt Standard (handled by Qt)

- QDialogButtonBox: Ok, Cancel, Apply, etc.
- QMessageBox: Save, Discard, Cancel
- QFileDialog: standard buttons and labels (if Qt translations are installed)

---

## Translation File Codes

- **en_US** – English (default, first)
- **de_DE** – Deutsch (German, second)

---

## Updating Translation Files

After adding or changing translatable strings, run:

```bash
lupdate -no-obsolete -locations relative . -ts translations/CoreBoard_en_US.ts translations/CoreBoard_de_DE.ts
```

Or build the project; the Qt Linguist integration in CMake will update the .ts files. Then use Qt Linguist or a text editor to fill in translations in the .ts files. Run `lrelease` (or build) to compile .qm files for the app.

---

## Files to Include in TRANSLATABLE_SOURCES

- mainwindow.ui
- mainwindow.cpp, mainwindow.h
- layouteditor.cpp, layouteditor.h
- layouteditorgraphicsview.cpp, layouteditorgraphicsview.h
- settingsdialog.cpp
- labelsettingsdialog.cpp
- dialogstyle.cpp
- dialogtextchange.cpp
- dialogkeycodechange.cpp
- languagesettingsdialog.cpp (NEW)
- main.cpp
- versioninfo.h
- version_info.ui
- keyboardwidget.cpp, keyboardwidget.h
- windowskeylistener.cpp, windowskeylistener.h
