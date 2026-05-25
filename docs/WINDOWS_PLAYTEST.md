# Windows Playtest Notes

This project uses Win32 drawing and keyboard timers, so the final feel needs a
real Windows run.

## Input Feel

- Try `PRESET` values: `BEGINNER`, `BALANCED`, `FAST`, and `CUSTOM`.
- DAS balanced default: 145 ms.
- ARR balanced default: 35 ms.
- Hold left/right against a wall and near the floor.
- Try small taps after holding a direction.
- Check lock-delay nudges while left/right are held.
- Switch `CONTROL` between `HYBRID`, `ARROWS`, and `WASD`, then confirm the
  blocked scheme no longer moves pieces.

## UI Pass

- Main menu selection moves cleanly and starts the game with Enter/Space.
- Pause menu continues, restarts, opens settings, opens leaderboard, and quits.
- F1 settings panel fits all rows, including PRESET, CONTROL, WINDOW, VOLUME,
  and NAME.
- `WINDOW` scale 100%, 125%, and 150% keep the fixed pixel canvas readable.
- Game-over leaderboard and name-entry overlay do not overlap text.
- Pixel font loads from `Font\monogram.ttf`.

## Audio Pass

- Move and soft-drop cues should stay subtle.
- Hard drop should feel crisp and immediate rather than heavy or muffled.
- Tetris, spin, B2B, and perfect clear cues should feel punchy, sparkly, and
  clearly more rewarding than a normal clear.
- `VOLUME` changes cue loudness enough to notice without muting at non-zero
  values.
- Sound OFF in settings should silence all cues.

## Scoring Pass

- Normal line clear shows base clear feedback.
- Combo x2 or higher shows bonus feedback.
- Back-to-back Tetris or spin clear shows B2B feedback.
- Perfect clear shows perfect-clear feedback and bonus score.

## Release Pass

- Package zip launches from a clean folder.
- Inno Setup installer launches from Start Menu after install.
- Optional desktop shortcut launches when selected during install.
- `tetris_settings.txt`, `tetris_highscore.txt`, and `tetris_leaderboard.txt`
  are created beside `main.exe` after play.
