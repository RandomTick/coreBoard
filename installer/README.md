# NSIS installer assets

Place optional image files here to customize the installer appearance:

| File        | Purpose                  | Recommended size |
|-------------|--------------------------|------------------|
| coreBoard.ico | Installer & uninstaller icon | 32×32 or 256×256 |
| welcome.bmp   | Welcome and finish page image | 164×314 px       |
| header.bmp    | Banner at top of installer pages | 150×57 px     |

If these files exist, CMake will use them automatically when building the NSIS installer.
