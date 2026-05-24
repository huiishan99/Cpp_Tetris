#pragma once

namespace AppConfig
{
constexpr int WindowWidth = 600;
constexpr int WindowHeight = 700;
constexpr int BoardLeft = 32;
constexpr int BoardTop = 64;
constexpr int CellSize = 28;

constexpr int DropTimerId = 1;
constexpr int ClearFlashTimerId = 2;
constexpr int LevelFlashTimerId = 3;
constexpr int LockDelayTimerId = 4;
constexpr int InputTimerId = 5;

constexpr int LevelFlashDurationMs = 900;
constexpr int LockDelayMs = 420;

inline constexpr const char *HighScoreFile = "tetris_highscore.txt";
inline constexpr const char *LeaderboardFile = "tetris_leaderboard.txt";
inline constexpr const char *SettingsFile = "tetris_settings.txt";
inline constexpr const char *PixelFontFile = "Font\\monogram.ttf";
inline constexpr const char *PixelFontName = "monogram";
inline constexpr const char *FallbackFontName = "Segoe UI";
}
