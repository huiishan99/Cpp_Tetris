# Devlog

This file records project changes, the reason for each change, verification
notes, and follow-up work. Keep it current so future bugs can be traced back to
specific edits.

## Change Policy

- Add one entry for every code, asset, build, or documentation change.
- Keep newest entries at the top.
- Include the files touched, why the change was made, what risk it carries, and
  how it was verified.
- If a change cannot be built or tested locally, say that explicitly.

## Entry Template

```md
### YYYY-MM-DD - Short summary

- Changed: ...
- Why: ...
- Risk: ...
- Verified: ...
- Follow-ups: ...
```

## Entries

### 2026-05-25 - Retune drop sound cues

- Changed: Replaced the low hard-drop thud with a short brighter snap,
  lightened soft-drop/move/rotate/hold cues, and made small input cues skip
  while another cue is actively playing.
- Why: Windows playtesting showed the drop sound still felt heavy, muffled,
  and delayed even after moving playback off the main thread.
- Risk: Beep-based sound is still limited by Windows tone playback, so the
  final taste needs another real Windows listen.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc
  tests/core_tests.cpp src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp
  src/high_score.cpp src/leaderboard.cpp src/position.cpp src/settings.cpp
  src/settings_options.cpp src/sound.cpp -o /private/tmp/tetris_core_tests`
  and ran `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: If the beep palette still feels harsh, replace these cues with
  short bundled WAV effects instead of generated tones.

### 2026-05-25 - Fix Windows sound latency and leaderboard spacing

- Changed: Moved Windows beep cues onto a background sound queue, clear stale
  movement cues before important sounds, shut the sound thread down on exit,
  widened leaderboard/game-over panels, and drew leaderboard names/scores in
  clipped columns with right-aligned scores.
- Why: Windows `Beep` blocks the caller and made input/paint feel delayed;
  the leaderboard rows were too narrow, causing player names and scores to
  overlap on real Windows font rendering.
- Risk: The sound worker uses Win32 threading and still needs a Windows
  playtest to confirm cue timing feels snappy. Long names are now ellipsized
  instead of overflowing.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc
  tests/core_tests.cpp src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp
  src/high_score.cpp src/leaderboard.cpp src/position.cpp src/settings.cpp
  src/settings_options.cpp src/sound.cpp -o /private/tmp/tetris_core_tests`
  and ran `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Recheck packaged Windows build sound feel and leaderboard
  spacing at 100%, 125%, and 150% window scales.

### 2026-05-25 - Refresh Windows playtest checklist

- Changed: Updated `docs/WINDOWS_PLAYTEST.md` for menus, tuning presets,
  control schemes, window scaling, volume, B2B, perfect clear, and installer
  checks.
- Why: Keep the manual Windows QA pass aligned with the newer gameplay,
  settings, audio, and release flows.
- Risk: Documentation-only change.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/leaderboard.cpp src/position.cpp src/settings.cpp
  src/settings_options.cpp src/sound.cpp -o /private/tmp/tetris_core_tests`
  and ran `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Fill the checklist with actual pass/fail notes after the next
  Windows run.

### 2026-05-25 - Add Windows installer script

- Changed: Added an Inno Setup installer script, installer build notes, release
  checklist installer steps, and README packaging instructions.
- Why: Let the project ship as a normal Windows installer after `package.bat`
  creates the release folder.
- Risk: The installer script targets Windows and Inno Setup 6, so it cannot be
  compiled in this macOS environment.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/leaderboard.cpp src/position.cpp src/settings.cpp
  src/settings_options.cpp src/sound.cpp -o /private/tmp/tetris_core_tests`
  and ran `/private/tmp/tetris_core_tests`; all core tests passed. Inno Setup
  installer build still needs Windows verification.
- Follow-ups: Build `dist\tetris-setup-0.9.0.exe` on Windows and add its hash
  to release notes.

### 2026-05-25 - Add B2B and perfect clear sound cues

- Changed: Added dedicated B2B and perfect clear sound cue functions, triggered
  them from line-clear scoring, updated README sound notes, and covered the new
  cue calls in core tests.
- Why: Give advanced clears a stronger payoff and make the newer scoring
  states easier to notice while playing.
- Risk: Extra beep sequences can make special clears feel longer on Windows;
  timing should be checked during the next playtest.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/leaderboard.cpp src/position.cpp src/settings.cpp
  src/settings_options.cpp src/sound.cpp -o /private/tmp/tetris_core_tests`
  and ran `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Replace blocking `Beep` cues with real asynchronous audio if the
  Windows build feels delayed.

### 2026-05-25 - Extract settings option helpers

- Changed: Added `src/settings_options.h/.cpp` for tuning preset and control
  scheme helpers, updated CMake/tests/README to include it, and moved the
  duplicated helper logic out of `src/main.cpp`.
- Why: Keep settings labels and preset values testable outside the Win32 UI.
- Risk: Refactor-only settings helper change; preset and control scheme
  behavior should remain the same.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/leaderboard.cpp src/position.cpp src/settings.cpp
  src/settings_options.cpp src/sound.cpp -o /private/tmp/tetris_core_tests`
  and ran `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Consider moving more Win32-independent UI formatting helpers into
  portable modules after Windows smoke testing.

### 2026-05-25 - Deduplicate game runtime state reset

- Changed: Added `Game::InitializeRuntimeState` and routed constructors plus
  restart reset through it.
- Why: Keep new scoring flags, combo/B2B state, lock-delay state, and clear
  feedback state initialized in one place.
- Risk: Refactor-only core logic change; constructor and restart behavior
  should remain the same.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/leaderboard.cpp src/position.cpp src/settings.cpp
  src/settings_options.cpp src/sound.cpp -o /private/tmp/tetris_core_tests`
  and ran `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Continue splitting larger modules only after Windows smoke tests
  confirm the newer UI and settings flows.

### 2026-05-25 - Upgrade gameplay settings

- Changed: Added persisted tuning preset, control scheme, window scale, and
  sound volume settings. The settings overlay now includes preset, control,
  window, and expanded sound rows; the Win32 renderer scales the fixed canvas
  to the selected window size. Updated README and core tests.
- Why: Make Windows hand-tuning faster and give players practical presets
  instead of only raw DAS/ARR numbers.
- Risk: Settings file schema expanded and Win32 paint now uses `StretchBlt` for
  scaled windows. Existing settings still load through defaults for missing
  keys, but window scaling needs Windows visual testing.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/leaderboard.cpp src/position.cpp src/settings.cpp src/sound.cpp -o
  /private/tmp/tetris_core_tests` and ran `/private/tmp/tetris_core_tests`;
  all core tests passed.
- Follow-ups: Add full per-action key rebinding later if the control presets
  are not enough.

### 2026-05-25 - Add combo, B2B, and perfect clear feedback

- Changed: Added combo score bonuses, back-to-back difficult-clear tracking,
  perfect clear detection, score breakdown accessors, status-panel feedback,
  README scoring notes, and core tests for the new scoring states.
- Why: Make clears feel more rewarding and give advanced play a clearer arcade
  payoff beyond the base line-clear score.
- Risk: Scoring changed for consecutive clears and perfect clears. Existing
  high scores remain readable, but future scores can be higher than older
  versions for the same board sequence.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/leaderboard.cpp src/position.cpp src/settings.cpp src/sound.cpp -o
  /private/tmp/tetris_core_tests` and ran `/private/tmp/tetris_core_tests`;
  all core tests passed.
- Follow-ups: Add richer audio cues for B2B and perfect clear after Windows
  sound testing.

### 2026-05-25 - Polish menu keyboard handling

- Changed: Let `Q` quit from the main and pause menus, and clear held
  horizontal movement state when entering pause, game over, or a non-started
  menu state.
- Why: Keep the README `Q` shortcut consistent across menus and prevent a held
  left/right key from leaking through after pause or game-over transitions.
- Risk: Small Win32 input-routing change; portable gameplay rules are
  unchanged.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/leaderboard.cpp src/position.cpp src/settings.cpp src/sound.cpp -o
  /private/tmp/tetris_core_tests` and ran `/private/tmp/tetris_core_tests`;
  all core tests passed. Win32 key flow still needs a Windows smoke test.
- Follow-ups: Verify Q, pause, resume, and held-left/right behavior on Windows.

### 2026-05-25 - Add main and pause menus

- Changed: Replaced the old ready/pause prompts with selectable main and pause
  menus, added an in-game leaderboard panel, routed restart through a UI reset
  helper, and updated README controls.
- Why: Make the game feel like a complete playable app instead of a prototype
  that starts from any random key press.
- Risk: Win32 keyboard routing changed around start, pause, leaderboard, and
  settings overlays. The portable core is unchanged, but the menu flow needs a
  Windows smoke test.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/leaderboard.cpp src/position.cpp src/settings.cpp src/sound.cpp -o
  /private/tmp/tetris_core_tests` and ran `/private/tmp/tetris_core_tests`;
  all core tests passed. Win32 menu flow still needs a Windows smoke test.
- Follow-ups: Add mouse support for menu items after Windows testing if the
  keyboard menu feels good.

### 2026-05-25 - Centralize app configuration

- Changed: Added `src/app_config.h` for Win32 window size, board layout, timer
  IDs, persistence file names, and font names. Updated `src/main.cpp` to read
  those values from the shared config header and updated the README project map.
- Why: Keep UI shell constants in one obvious place before larger Win32 drawing
  or input refactors.
- Risk: Mechanical constant replacement touches window creation, drawing,
  timers, persistence, and font loading. The portable core is unchanged, but
  the Win32 GUI still needs a Windows smoke test.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/leaderboard.cpp src/position.cpp src/settings.cpp src/sound.cpp -o
  /private/tmp/tetris_core_tests` and ran `/private/tmp/tetris_core_tests`;
  all core tests passed.
- Follow-ups: Split `src/main.cpp` further into drawing, input, and persistence
  bridge modules after another Windows playtest.

### 2026-05-25 - Add README gameplay preview

- Changed: Added a stylized gameplay preview SVG for the README and a
  `docs/CAPTURE_MEDIA.md` guide for replacing it with a real Windows
  screenshot or GIF after playtesting.
- Why: Give the project page an immediate visual signal while avoiding a fake
  claim that a real Win32 screenshot was captured in this macOS environment.
- Risk: The preview is illustrative, not an exact runtime screenshot.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/leaderboard.cpp src/position.cpp src/settings.cpp src/sound.cpp -o
  /private/tmp/tetris_core_tests` and ran `/private/tmp/tetris_core_tests`;
  all core tests passed.
- Follow-ups: Capture and commit a real Windows screenshot or gameplay GIF
  after the next Windows playtest.

### 2026-05-25 - Improve release packaging

- Changed: Added `VERSION`, release and Windows playtest checklists under
  `docs/`, and upgraded `package.bat` to create versioned release folders and
  zips, a `tetris-win-latest.zip` copy, package release notes, and SHA256
  output when `certutil` is available. Updated README packaging instructions.
- Why: Make the project easier to share as a downloadable Windows game instead
  of a loose executable build.
- Risk: The packaging script targets Windows batch/PowerShell and cannot be
  executed in this macOS environment.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/leaderboard.cpp src/position.cpp src/settings.cpp src/sound.cpp -o
  /private/tmp/tetris_core_tests` and ran `/private/tmp/tetris_core_tests`;
  all core tests passed. Windows packaging still needs to be run on Windows.
- Follow-ups: Run `package.bat` on Windows and verify the generated zip,
  latest copy, and checksum file.

### 2026-05-24 - Add default player name setting

- Changed: Added `player_name` to the persisted settings file, normalized
  leaderboard names to uppercase letters and digits, added a `NAME` row to the
  F1 settings overlay, and prefilled qualifying game-over name entry with the
  saved default player name. Esc on the leaderboard name-entry overlay now
  saves using the default player name instead of always using `PLAYER`.
  Updated README, roadmap, and core tests.
- Why: Reduce repeated typing for leaderboard entries and make saved scores
  feel more personal.
- Risk: The settings overlay grew by one row and needs Windows visual
  playtesting. Name editing still accepts only uppercase letters and digits.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/leaderboard.cpp src/position.cpp src/settings.cpp src/sound.cpp -o
  /private/tmp/tetris_core_tests` and ran `/private/tmp/tetris_core_tests`;
  all core tests passed. Windows GUI playtesting was not available in this
  macOS environment.
- Follow-ups: Confirm the larger settings overlay on Windows and consider
  allowing a wider set of name characters later.

### 2026-05-24 - Enhance sound cues

- Changed: Reworked the Windows beep cues into short layered tone sequences
  for movement, soft drop, hard drop, hold, rotation, level-up, pause, and game
  over. Line-clear audio now receives the cleared line count and whether the
  clear was a spin, so singles/doubles/triples/Tetris and spin clears have
  distinct cues. Added a sound API safety test and updated README and roadmap
  notes.
- Why: Make gameplay feedback feel more expressive without adding external
  audio assets or changing the existing sound on/off setting.
- Risk: The exact tone balance can only be judged on Windows speakers; macOS
  core tests only verify that the sound API compiles and remains safe to call.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/leaderboard.cpp src/position.cpp src/settings.cpp src/sound.cpp -o
  /private/tmp/tetris_core_tests` and ran `/private/tmp/tetris_core_tests`;
  all core tests passed. Windows audio playback was not available in this
  macOS environment.
- Follow-ups: Playtest on Windows and replace beep sequences with bundled
  audio assets later if the project needs a fuller sound design.

### 2026-05-23 - Add leaderboard name entry

- Changed: Added leaderboard qualification checks, public name normalization,
  and Win32 game-over name entry for qualifying scores. Players can enter
  3-12 uppercase letters or digits, press Enter to save, or press Esc to save
  as `PLAYER`. Updated README and roadmap notes and expanded core tests for
  qualification and name normalization.
- Why: Give leaderboard scores player identity instead of storing every entry
  under the default name.
- Risk: Name entry is Win32 UI behavior and still needs Windows playtesting.
  The first pass intentionally allows only uppercase letters and digits.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/leaderboard.cpp src/position.cpp src/settings.cpp src/sound.cpp -o
  /private/tmp/tetris_core_tests` and ran `/private/tmp/tetris_core_tests`;
  all core tests passed. Windows GUI playtesting was not available in this
  macOS environment.
- Follow-ups: Add a default player name setting and confirm the name-entry
  overlay layout on Windows.

### 2026-05-23 - Add local leaderboard

- Changed: Added a portable leaderboard module for top-five score sorting,
  file persistence, and best-score lookup. The Win32 game now loads
  `tetris_leaderboard.txt`, migrates an existing single high score into the
  leaderboard, records a positive score once when game over happens, saves the
  leaderboard, and shows the top three entries on the game-over overlay.
  Updated CMake, README, roadmap, git ignores, and core tests.
- Why: Move beyond a single best score and give repeat play a clearer local
  progression target.
- Risk: Entries use the default `PLAYER` name until a later name-entry flow is
  added. The game-over overlay changed layout and still needs Windows visual
  playtesting.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/leaderboard.cpp src/position.cpp src/settings.cpp src/sound.cpp -o
  /private/tmp/tetris_core_tests` and ran `/private/tmp/tetris_core_tests`;
  all core tests passed. Windows GUI playtesting was not available in this
  macOS environment.
- Follow-ups: Add editable player names and confirm the game-over overlay on
  Windows.

### 2026-05-23 - Persist settings

- Changed: Added a portable settings module that reads and writes
  `tetris_settings.txt`, sanitizes DAS/ARR/effect values, and stores the sound
  toggle. The Win32 UI now loads settings on startup and saves them on exit.
  Added settings persistence tests, included the new module in CMake, ignored
  generated settings files, and updated README and roadmap notes.
- Why: Keep F1 tuning choices across launches so Windows playtesting does not
  require retuning every run.
- Risk: Settings save on normal app exit; an abrupt crash or forced close may
  lose the latest in-session tweak. The config format is intentionally simple
  `key=value` text and unknown keys are ignored.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/settings.cpp src/sound.cpp -o
  /private/tmp/tetris_core_tests` and ran `/private/tmp/tetris_core_tests`;
  all core tests passed. Windows GUI playtesting was not available in this
  macOS environment.
- Follow-ups: Playtest the saved settings flow on Windows and decide whether
  settings should be saved immediately when changed instead of only on exit.

### 2026-05-23 - Add in-game settings overlay

- Changed: Added an F1 settings overlay with selectable rows for DAS delay,
  ARR interval, line-clear effect duration, and sound on/off. Converted those
  Win32 timing values from fixed constants into adjustable runtime state,
  paused automatic timers while settings are open, added a sound enable flag,
  documented controls, updated the roadmap, and added a sound toggle core
  test.
- Why: Let the game be tuned without rebuilding, especially for Windows
  playtesting where keyboard feel and clear effect speed are easiest to judge.
- Risk: Settings are runtime-only for now and are not persisted between
  launches. The overlay and key handling touch Win32-only UI code, so real
  keyboard testing on Windows is still needed.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed. Windows GUI
  playtesting was not available in this macOS environment.
- Follow-ups: Persist settings to a config file and tune the default DAS/ARR
  values after Windows playtesting.

### 2026-05-23 - Add counter-clockwise rotation

- Changed: Added counter-clockwise block rotation, wired `Z` as rotate
  counter-clockwise, kept `Up`/`W` and added `X` for clockwise rotation,
  expanded SRS-style kick tables for reverse rotation paths, added core tests
  for counter-clockwise T and I kicks, updated README controls, and added a
  compact roadmap for the next polish tracks.
- Why: Give players the expected two-way rotation control needed for faster
  placement, cleaner spin setup, and more modern Tetris feel.
- Risk: `Z` now performs a gameplay action, so it no longer behaves like a
  generic start/resume key when active play is running. Reverse kick outcomes
  should be close to SRS, but Windows playtesting is still needed for feel.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed. Windows GUI
  playtesting was not available in this macOS environment.
- Follow-ups: Build the settings interface, tune DAS/ARR on Windows, add a
  screenshot or GIF, make a fuller installer/release flow, add a local
  leaderboard, and expand sound effects.

### 2026-05-23 - Add SRS-style wall kicks

- Changed: Replaced the generic wall-kick offsets with clockwise SRS-style
  kick tables, including the I block's special offsets and O block no-op
  handling. Exposed block rotation state for rule evaluation, expanded core
  tests for T-piece and I-piece kicked rotations, and documented the improved
  rotation feel in the README.
- Why: Make rotations near walls, stacks, and spin slots feel closer to modern
  Tetris and support more reliable spin setups.
- Risk: Rotation outcomes can differ from the old generic kick table, so some
  edge-case placements may now choose a different valid kicked position.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Add counter-clockwise rotation later if the controls expand
  beyond the current single rotate key.

### 2026-05-22 - Add DAS/ARR horizontal input

- Changed: Added a Win32 input timer for left/right hold state, immediate
  single-tap movement, DAS-style delayed repeat, ARR-style repeat cadence, and
  key-up handling so the last held horizontal direction wins. Documented
  hold-to-slide controls in the README.
- Why: Make horizontal movement feel smoother and more controllable during
  fast play and while using lock delay near the floor.
- Risk: The repeat timing lives in the Windows UI layer, so portable core tests
  cover rule regressions but not actual keyboard repeat feel. The timing may
  still need tuning on real Windows hardware.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Playtest on Windows and tune `DAS_DELAY_MS` or
  `ARR_INTERVAL_MS` if held movement feels too quick or too sluggish.

### 2026-05-21 - Add lock delay

- Changed: Added grounded lock-delay state, event tracking for Win32 timer
  resets, `Game::FinishLockDelay`, movement/rotation delay refreshes, and a
  Win32 lock-delay timer before grounded pieces are committed to the grid.
  Added core coverage for adjusting a grounded piece before it locks and for
  keeping lock delay paused while the game is paused.
- Why: Make grounded play feel closer to modern Tetris and give players time to
  nudge or rotate pieces after contact.
- Risk: Lock delay changes automatic drop timing and lock timing; hard drop
  still locks immediately, while normal downward movement now waits for the
  short timer.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Add DAS/ARR next so horizontal movement feels consistent while
  lock delay is active.

### 2026-05-17 - Add Windows packaging script

- Changed: Added `package.bat` to build the game, assemble a `dist\tetris-win`
  folder, copy the executable, README, font, and runtime DLLs beside the
  executable, and create `dist\tetris-win.zip` when PowerShell is available.
  Ignored `dist/` and documented packaging in the README.
- Why: Let the game be distributed or run from a packaged folder without asking
  players to rebuild manually.
- Risk: The script targets Windows batch/PowerShell and could not be executed
  in this macOS environment; local verification covers repository syntax and
  portable core tests.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Run `package.bat` on Windows and confirm the zip launches from a
  clean folder.

### 2026-05-16 - Rewrite README for players

- Changed: Rewrote `README.md` into a shorter, more player-focused project
  page with a stronger intro, clearer feature highlights, compact controls,
  scoring tables, and trimmed build/test instructions.
- Why: Make the project look more inviting and remove overly procedural or
  unrelated README content.
- Risk: Some detailed troubleshooting text was removed; the remaining build
  commands should cover the normal paths.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Add a gameplay screenshot or GIF after the next Windows visual
  pass.

### 2026-05-16 - Add style-spin clears

- Changed: Added immobile style-spin detection for L/J/I/S/Z pieces after a
  successful rotation, stored the last spin block id, rendered spin labels such
  as `L-SPIN`, added style-spin bonus scoring, and expanded core tests for an
  L-spin single.
- Why: Let the game recognize satisfying non-T spin clears without pretending
  they are standard T-spin scoring.
- Risk: Style spins use a simple immobility rule and the existing simple kick
  table, so they are a custom all-spin style rather than a full guideline
  implementation.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Add piece-specific SRS kicks and a rules toggle if the game should
  support both strict T-spin-only and all-spin modes.

### 2026-05-16 - Improve pause resume and restart controls

- Changed: Pause now resumes on any key, paused Space is consumed instead of
  hard-dropping immediately, `R` restarts the current game, the pause overlay
  advertises any-key resume plus restart, and the line-clear flash was shortened
  from 220ms to 170ms.
- Why: Make pause easier to escape in a hurry, add an explicit restart action,
  and make line clears feel a little snappier.
- Risk: Any key on the pause screen now resumes play, while `Q` still quits and
  `R` restarts; Win32 GUI build was not available in this environment.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Consider a pause menu later if restart confirmation or settings
  are added.

### 2026-05-16 - Make pause easier to reach

- Changed: Added `Esc` as a Win32 pause/resume key alongside `P`, updated the
  pause overlay action prompt, and documented the shortcut in the controls
  table.
- Why: Pause existed, but it was too easy to miss during play; `Esc` is a more
  natural panic-pause key.
- Risk: `Esc` is ignored on the start and game-over screens to avoid accidental
  starts or restarts; Win32 GUI build was not available in this environment.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Consider adding a small menu pause screen later if settings or
  restart confirmation are added.

### 2026-05-16 - Add T-spin feedback and pixel font

- Changed: Added T-spin single/double/triple bonus scoring, corner-based T-spin
  detection after successful rotation, T-spin status-panel feedback, a faster
  line-clear flash timing, and private loading of `Font/monogram.ttf` for a
  more stylized pixel UI.
- Why: Support a standard advanced Tetris technique, make clears feel snappier,
  and give the window more visual character than the default system font.
- Risk: T-spin detection uses a lightweight three-corner rule with the existing
  simple kick table, not full guideline SRS; Win32 font rendering and animation
  timing still need a Windows play pass.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Add full SRS and optional custom L/J/S/Z spin recognition if the
  project should support non-standard spin bonuses.

### 2026-05-16 - Add level-up feedback

- Changed: Added level-up event tracking, reached-level state, a level-up sound
  cue, a short status-panel level-up flash, and a core test for crossing the
  ten-line level threshold.
- Why: Make speed increases visible and audible instead of only changing the
  small Level number in the side panel.
- Risk: The Win32 flash timing and beep feel need a Windows play pass; local
  verification covers the portable rules and event state.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Tune the status-panel priority if level-up and Tetris feedback
  should be shown together rather than one after the other.

### 2026-05-16 - Add cross-platform CI

- Changed: Added a GitHub Actions workflow that configures CMake, builds the
  project, and runs `ctest` on Ubuntu, macOS, and Windows. Added build
  directory ignores and documented the CI workflow in the README.
- Why: Catch core logic regressions across common development environments and
  make future changes easier to trust.
- Risk: The workflow has not run on GitHub from this environment; local CMake
  and core test verification were used instead.
- Verified: Local `cmake` was not available in this environment. Built with
  `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp src/block.cpp
  src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp src/position.cpp
  src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Check the first GitHub Actions run after pushing and adjust the
  workflow if a hosted runner exposes any generator-specific issue.

### 2026-05-16 - Add three-block next queue

- Changed: Added a three-block upcoming queue behind the existing next-block
  API, advanced the queue through hold/spawn flow, rendered the Next panel as
  three compact previews, and added core tests for queue size and promotion.
- Why: Give players more planning information and make Hold decisions more
  strategic without changing the basic rules.
- Risk: Queue advancement touches block spawning and Hold behavior; Win32 GUI
  build was not available in this environment.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Recheck Next panel spacing on Windows and consider a five-block
  queue if the sidebar is redesigned later.

### 2026-05-16 - Add richer action sounds

- Changed: Added separate lightweight Win32 beep cues for movement, soft drop,
  hard drop, hold, pause, and game over, while keeping rotate and line-clear
  cues. Updated the feature list to describe the broader sound feedback.
- Why: Make controls feel more responsive and give players clearer feedback for
  successful actions and terminal states.
- Risk: Win32 `Beep` is intentionally simple and blocking for very short
  durations; exact feel should be checked on Windows speakers.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Replace beep cues with non-blocking audio files if the project
  later gains an asset pipeline.

### 2026-05-16 - Delay row collapse during line clear

- Changed: Added a `lineClearPending` state, delayed row collapse through
  `Game::FinishLineClear`, paused the Win32 drop timer while rows flash, and
  expanded core tests to verify completed rows stay visible until the animation
  finishes.
- Why: Make line clears feel more intentional and readable, with the board
  showing the completed rows before they collapse.
- Risk: Input is ignored during the short clear delay, and the Win32 GUI build
  was not available in this environment.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Tune the clear delay duration on Windows if it feels too snappy
  or too slow during real play.

### 2026-05-16 - Add line clear flash

- Changed: Added `Grid::GetFullRows`, tracked the last cleared row indices and
  clear event id in `Game`, added a short Win32 clear-flash timer/render pass,
  and expanded core tests for cleared-row/event tracking.
- Why: Make cleared lines visibly pop on the board instead of only updating the
  side-panel text.
- Risk: The flash is a lightweight post-clear overlay, not a delayed clear
  animation; Win32 GUI build was not available in this environment.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: If desired, convert this to a true delayed clear animation that
  holds the completed rows for a few frames before collapsing the board.

### 2026-05-16 - Add game state overlays

- Changed: Added a start-gated game state, `Game::Start`, `Game::IsStarted`,
  start/pause/game-over overlay panels, READY status display, and core tests
  that verify automatic drop is blocked before start.
- Why: Make the game feel like a complete playable experience instead of
  dropping immediately when the window opens.
- Risk: The first gameplay key now starts the game without also moving the
  block; Win32 GUI build was not available in this environment.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Tune overlay spacing and text metrics after another Windows
  visual pass.

### 2026-05-16 - Persist best score

- Changed: Added `src/high_score.h`, `src/high_score.cpp`, high-score loading
  on startup, saving on shutdown, `.gitignore` entries for generated score
  files, CMake source wiring, and core tests for missing/saved/invalid score
  files.
- Why: Keep the Best score across program runs instead of limiting it to one
  session.
- Risk: The score file is a simple text file in the working directory, so
  changing launch directories changes which high-score file is used. Win32 GUI
  build was not available in this environment.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/high_score.cpp
  src/position.cpp src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Save beside the executable or in a platform user-data folder if
  launch-directory independence becomes important.

### 2026-05-16 - Add session best score

- Changed: Added session high-score tracking, a public `Restart()` path that
  preserves the best score, Best score display in the side panel, and core tests
  for restart/high-score behavior and blocked soft-drop scoring.
- Why: Give players a simple score target across restarts without adding file
  persistence yet.
- Risk: High score is in-memory only and resets when the program closes; soft
  drop scoring now only counts successful downward movement. Win32 GUI build was
  not available in this environment.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/position.cpp
  src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Add optional file-backed high-score persistence later.

### 2026-05-16 - Add clear feedback and combo

- Changed: Added last-clear line/score tracking, combo state, a test constructor
  that accepts a starting grid, status-panel clear feedback, and core tests for
  line-clear feedback reset behavior.
- Why: Make line clears feel more responsive and easier to read than score
  changes alone.
- Risk: Locking now resets combo/feedback on non-clearing pieces; Win32 GUI
  build was not available in this environment.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/position.cpp
  src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Add a short visual flash on the board itself once the Win32 build
  can be checked interactively.

### 2026-05-16 - Add rotation kick assist

- Changed: Added simple wall/spawn kick offsets for rotation, a deterministic
  game constructor for core tests, and a test that verifies an I block can
  rotate near the spawn top without leaving the board.
- Why: Make rotation feel more forgiving near walls and the top of the playfield
  instead of immediately rejecting close rotations.
- Risk: This is a simple kick table, not full SRS; some advanced rotation cases
  may still differ from modern guideline Tetris. Win32 GUI build was not
  available in this environment.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/position.cpp
  src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Replace the simple kick table with piece-specific SRS data if the
  project aims for guideline-accurate Tetris behavior.

### 2026-05-16 - Add hold block

- Changed: Added hold-block state, `C`/`Shift` input handling, Hold/Next preview
  panels, a safe default block id, and a core test that verifies hold can only
  be used once before a block lands.
- Why: Bring the controls closer to modern Tetris and give players more
  strategic choice during placement.
- Risk: Current/next/held block swapping touches spawn flow; Win32 GUI build was
  not available in this environment.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/position.cpp
  src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Tune Hold panel spacing on Windows and consider a subtle visual
  lock indicator after Hold is used.

### 2026-05-16 - Polish Win32 game UI

- Changed: Reworked the Win32 drawing style with a wider window, warmer dark
  background, rounded board/sidebar panels, highlighted block cells, dotted
  ghost cells, status panel, and a graphical next-block preview.
- Why: Make the game feel less like a debug sample and more like a finished
  playable window.
- Risk: Main GUI rendering changed substantially and could need spacing/color
  tuning on Windows; Win32 GUI build was not available in this environment.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/position.cpp
  src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Open on Windows to tune exact text metrics and color contrast.

### 2026-05-16 - Add line stats and level speed

- Changed: Added total cleared lines, level calculation, dynamic drop interval,
  side-panel stats for lines/level, and core tests for level/speed formulas.
- Why: Make the game pace evolve over time instead of staying at one fixed
  speed.
- Risk: Timer interval now changes after each tick; Win32 GUI build was not
  available in this environment.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/position.cpp
  src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Tune the speed curve after testing the Windows build by hand.

### 2026-05-16 - Add ghost landing preview

- Changed: Added `Game::GetGhostBlockCells`, const-safe block/grid helpers,
  ghost-cell outline rendering, and a core test that verifies ghost preview does
  not move the current block.
- Why: Make placement easier to read and improve the moment-to-moment feel of
  the game.
- Risk: Rendering order changed so the board is drawn before ghost/current
  pieces; Win32 GUI build was not available in this environment.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/position.cpp
  src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Tune ghost styling on Windows and consider adding a setting to
  hide/show ghost preview later.

### 2026-05-16 - Add pause and resume

- Changed: Added paused game state, `P` input handling, paused UI text, accessors
  for game state, and a core test for paused automatic drop behavior.
- Why: Improve basic playability and start moving direct state access behind
  small query methods.
- Risk: Input flow changed around game-over and paused states; timer still fires
  but gameplay movement is ignored while paused.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/position.cpp
  src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Add a visual overlay for pause/game-over states and consider
  stopping the timer while paused if future frontends need it.

### 2026-05-16 - Make core logic portable

- Changed: Added `src/sound.h`, `src/sound.cpp`, moved Windows `Beep` calls out
  of `Game`, changed CMake to build a `tetris_core` library, and added
  `tests/core_tests.cpp`.
- Why: Let the game rules compile outside the Win32 GUI so future gameplay work
  can be tested on more environments.
- Risk: Windows sound behavior now goes through a wrapper; CMake target layout
  changed.
- Verified: Built with `c++ -std=c++17 -Wall -Wextra -Isrc tests/core_tests.cpp
  src/block.cpp src/blocks.cpp src/game.cpp src/grid.cpp src/position.cpp
  src/sound.cpp -o /private/tmp/tetris_core_tests` and ran
  `/private/tmp/tetris_core_tests`; all core tests passed.
- Follow-ups: Add more tests around locking, game-over, hard drop scoring, and
  future gameplay features.

### 2026-05-16 - Add CMake Windows build

- Changed: Added `CMakeLists.txt` and documented MinGW/Visual Studio CMake
  build commands in `README.md`.
- Why: Support more Windows compiler and IDE setups with one build definition.
- Risk: CMake configuration is new and currently limited to Win32 builds.
- Verified: Tried `cmake -S . -B /private/tmp/cpp-tetris-cmake-check`;
  `cmake` is not installed in this environment. Win32 GUI build not run.
- Follow-ups: Split game logic from Win32 rendering so the core can build and
  be tested on non-Windows platforms.

### 2026-05-16 - Add maintenance devlog

- Changed: Created `DEVLOG.md` and linked it from `README.md`.
- Why: Establish a traceable change history before future fixes and refactors.
- Risk: Documentation-only change; no runtime impact.
- Verified: Not run; documentation-only change.
- Follow-ups: Add a new devlog entry for every future repository change.
