# Windows Installer

Use this on Windows after a release package has been created.

## Requirements

- Inno Setup 6
- A completed `dist\tetris-win-<version>` folder from `package.bat`

## Build

1. Update `MyAppVersion` in `installer\tetris.iss` to match `VERSION`.
2. Run `.\package.bat`.
3. Open `installer\tetris.iss` in Inno Setup Compiler.
4. Build the script.

The installer will be written to `dist\tetris-setup-<version>.exe`.

## Smoke Test

- Install to the default Program Files path.
- Launch from the Start Menu shortcut.
- Optional: launch from the desktop shortcut.
- Confirm `Font\monogram.ttf` is installed beside `main.exe`.
- Play one short game and verify settings/high score files can be written.
