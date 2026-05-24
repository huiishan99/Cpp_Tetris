# Release Checklist

Use this before sharing a Windows build.

## Build

- Run `.\build.bat`.
- Run `.\package.bat`.
- Confirm `dist\tetris-win-<version>.zip` exists.
- Confirm `dist\tetris-win-latest.zip` exists.
- Confirm `SHA256SUMS.txt` contains the release zip hash.

## Package Contents

- `main.exe`
- `README.md`
- `VERSION`
- `DEVLOG.md`
- `ROADMAP.md`
- `Font\monogram.ttf`
- Runtime DLLs from `lib\` when present

## Smoke Test

- Launch `main.exe` from the unpacked package folder.
- Start a game from READY.
- Move, rotate clockwise, rotate counter-clockwise, hold, hard drop, pause, and restart.
- Open F1 settings, change DAS/ARR, edit NAME, toggle sound, close settings, restart the app, and confirm settings persisted.
- Get a qualifying score and confirm leaderboard name entry saves to `tetris_leaderboard.txt`.
- Confirm high score and leaderboard survive relaunch.
