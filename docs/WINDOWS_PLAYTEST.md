# Windows Playtest Notes

This project uses Win32 drawing and keyboard timers, so the final feel needs a
real Windows run.

## Input Feel

- DAS default: 145 ms
- ARR default: 35 ms
- Hold left/right against a wall and near the floor.
- Try small taps after holding a direction.
- Check lock-delay nudges while left/right are held.

## UI Pass

- READY overlay fits the board.
- PAUSED overlay resumes on any key.
- F1 settings panel fits all rows, including NAME.
- Game-over leaderboard and name-entry overlay do not overlap text.
- Pixel font loads from `Font\monogram.ttf`.

## Audio Pass

- Move and soft-drop cues should stay subtle.
- Hard drop should feel heavier.
- Tetris and spin clear cues should stand out.
- Sound OFF in settings should silence all cues.

## Release Pass

- Package zip launches from a clean folder.
- `tetris_settings.txt`, `tetris_highscore.txt`, and `tetris_leaderboard.txt`
  are created beside `main.exe` after play.
