#include "app_config.h"
#include "game.h"
#include "high_score.h"
#include "leaderboard.h"
#include "settings.h"
#include "sound.h"
#include <windows.h>
#include <string>
#include <vector>

enum SettingsOption
{
    SettingsTuningPreset = 0,
    SettingsDasDelay,
    SettingsArrInterval,
    SettingsClearFlash,
    SettingsControlScheme,
    SettingsWindowScale,
    SettingsSound,
    SettingsSoundVolume,
    SettingsPlayerName,
    SettingsOptionCount
};

enum MainMenuOption
{
    MainMenuStart = 0,
    MainMenuSettings,
    MainMenuLeaderboard,
    MainMenuQuit,
    MainMenuOptionCount
};

enum PauseMenuOption
{
    PauseMenuContinue = 0,
    PauseMenuRestart,
    PauseMenuSettings,
    PauseMenuLeaderboard,
    PauseMenuQuit,
    PauseMenuOptionCount
};

Game game;
std::vector<LeaderboardEntry> leaderboard;
int observedClearEventId = 0;
int observedLevelUpEventId = 0;
int observedLockDelayEventId = 0;
DWORD clearFlashStartedAt = 0;
DWORD levelFlashStartedAt = 0;
DWORD lockDelayStartedAt = 0;
int clearFlashDurationMs = SettingsDefaultClearFlashDurationMs;
int dasDelayMs = SettingsDefaultDasDelayMs;
int arrIntervalMs = SettingsDefaultArrIntervalMs;
int tuningPreset = SettingsDefaultTuningPreset;
int controlScheme = SettingsDefaultControlScheme;
int windowScalePercent = SettingsDefaultWindowScalePercent;
int soundVolumePercent = SettingsDefaultSoundVolumePercent;
int selectedSettingIndex = 0;
int selectedMainMenuIndex = MainMenuStart;
int selectedPauseMenuIndex = PauseMenuContinue;
std::string defaultPlayerName = DefaultLeaderboardName;
bool pixelFontLoaded = false;
bool lockDelayTimerRunning = false;
bool inputTimerRunning = false;
bool settingsOpen = false;
bool settingsNameEditActive = false;
bool leaderboardOpen = false;
bool gameOverScoreRecorded = false;
bool leaderboardNameEntryActive = false;
bool leftHeld = false;
bool rightHeld = false;
int horizontalDirection = 0;
int pendingLeaderboardScore = 0;
DWORD horizontalHeldStartedAt = 0;
DWORD lastHorizontalRepeatAt = 0;
std::string settingsNameDraft;
std::string leaderboardNameInput;

const char *GetGameFontName()
{
    return pixelFontLoaded ? AppConfig::PixelFontName : AppConfig::FallbackFontName;
}

DWORD GetGameFontQuality()
{
    return pixelFontLoaded ? NONANTIALIASED_QUALITY : CLEARTYPE_NATURAL_QUALITY;
}

void LoadGameFont()
{
    pixelFontLoaded = AddFontResourceExA(AppConfig::PixelFontFile, FR_PRIVATE, nullptr) > 0;
}

void UnloadGameFont()
{
    if (pixelFontLoaded)
    {
        RemoveFontResourceExA(AppConfig::PixelFontFile, FR_PRIVATE, nullptr);
        pixelFontLoaded = false;
    }
}

COLORREF GetBlockColor(int blockId)
{
    switch (blockId)
    {
    case 1:
        return RGB(226, 116, 17);
    case 2:
        return RGB(13, 64, 216);
    case 3:
        return RGB(21, 204, 209);
    case 4:
        return RGB(237, 234, 4);
    case 5:
        return RGB(47, 230, 23);
    case 6:
        return RGB(166, 0, 247);
    case 7:
        return RGB(232, 18, 18);
    default:
        return RGB(26, 31, 40);
    }
}

COLORREF AdjustColor(COLORREF color, int amount)
{
    int red = GetRValue(color) + amount;
    int green = GetGValue(color) + amount;
    int blue = GetBValue(color) + amount;

    if (red < 0)
    {
        red = 0;
    }
    if (green < 0)
    {
        green = 0;
    }
    if (blue < 0)
    {
        blue = 0;
    }
    if (red > 255)
    {
        red = 255;
    }
    if (green > 255)
    {
        green = 255;
    }
    if (blue > 255)
    {
        blue = 255;
    }
    return RGB(red, green, blue);
}

int ClampInt(int value, int minimum, int maximum)
{
    if (value < minimum)
    {
        return minimum;
    }
    if (value > maximum)
    {
        return maximum;
    }
    return value;
}

int CycleInt(int value, int minimum, int maximum, int direction)
{
    if (direction > 0)
    {
        return value >= maximum ? minimum : value + 1;
    }
    if (direction < 0)
    {
        return value <= minimum ? maximum : value - 1;
    }
    return value;
}

void ApplyTuningPresetValues()
{
    if (tuningPreset == 0)
    {
        dasDelayMs = 185;
        arrIntervalMs = 55;
        clearFlashDurationMs = 190;
    }
    else if (tuningPreset == 1)
    {
        dasDelayMs = SettingsDefaultDasDelayMs;
        arrIntervalMs = SettingsDefaultArrIntervalMs;
        clearFlashDurationMs = SettingsDefaultClearFlashDurationMs;
    }
    else if (tuningPreset == 2)
    {
        dasDelayMs = 105;
        arrIntervalMs = 20;
        clearFlashDurationMs = 110;
    }
}

void MarkCustomTuning()
{
    tuningPreset = 3;
}

int GetScaledWindowWidth()
{
    return AppConfig::WindowWidth * windowScalePercent / 100;
}

int GetScaledWindowHeight()
{
    return AppConfig::WindowHeight * windowScalePercent / 100;
}

bool AllowArrowControls()
{
    return controlScheme != 2;
}

bool AllowWasdControls()
{
    return controlScheme != 1;
}

void ApplyRuntimeSettings(const GameSettings &settings)
{
    GameSettings sanitized = SanitizeSettings(settings);
    tuningPreset = sanitized.tuningPreset;
    dasDelayMs = sanitized.dasDelayMs;
    arrIntervalMs = sanitized.arrIntervalMs;
    clearFlashDurationMs = sanitized.clearFlashDurationMs;
    controlScheme = sanitized.controlScheme;
    windowScalePercent = sanitized.windowScalePercent;
    defaultPlayerName = NormalizeLeaderboardName(sanitized.playerName);
    SetSoundEnabled(sanitized.soundEnabled);
    SetSoundVolumePercent(sanitized.soundVolumePercent);
    soundVolumePercent = GetSoundVolumePercent();
}

GameSettings CollectRuntimeSettings()
{
    return SanitizeSettings(GameSettings{
        dasDelayMs,
        arrIntervalMs,
        clearFlashDurationMs,
        tuningPreset,
        controlScheme,
        windowScalePercent,
        IsSoundEnabled(),
        GetSoundVolumePercent(),
        defaultPlayerName,
    });
}

void RecordGameOverScoreIfNeeded()
{
    if (!game.IsGameOver())
    {
        gameOverScoreRecorded = false;
        leaderboardNameEntryActive = false;
        pendingLeaderboardScore = 0;
        leaderboardNameInput.clear();
        return;
    }

    if (gameOverScoreRecorded || leaderboardNameEntryActive)
    {
        return;
    }

    int score = game.GetScore();
    if (!DoesScoreQualifyForLeaderboard(leaderboard, score))
    {
        gameOverScoreRecorded = true;
        return;
    }

    pendingLeaderboardScore = score;
    leaderboardNameInput = defaultPlayerName;
    leaderboardNameEntryActive = true;
    leftHeld = false;
    rightHeld = false;
    horizontalDirection = 0;
}

void SubmitLeaderboardName(const std::string &name)
{
    if (!leaderboardNameEntryActive)
    {
        return;
    }

    leaderboard = AddLeaderboardScore(leaderboard, pendingLeaderboardScore, name);
    int bestScore = GetBestLeaderboardScore(leaderboard);
    if (bestScore > game.GetHighScore())
    {
        game.SetHighScore(bestScore);
    }
    SaveLeaderboard(AppConfig::LeaderboardFile, leaderboard);
    SaveHighScore(AppConfig::HighScoreFile, game.GetHighScore());

    leaderboardNameEntryActive = false;
    leaderboardNameInput.clear();
    pendingLeaderboardScore = 0;
    gameOverScoreRecorded = true;
}

void DrawTextLine(HDC hdc, int x, int y, const std::string &text, int size = 24,
                  COLORREF color = RGB(238, 238, 228), int weight = FW_NORMAL)
{
    HFONT font = CreateFontA(size, 0, 0, 0, weight, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             GetGameFontQuality(), DEFAULT_PITCH | FF_DONTCARE, GetGameFontName());
    HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, font));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    TextOutA(hdc, x, y, text.c_str(), static_cast<int>(text.size()));
    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

void DrawTextCentered(HDC hdc, int left, int right, int y, const std::string &text,
                      int size, COLORREF color, int weight = FW_NORMAL)
{
    HFONT font = CreateFontA(size, 0, 0, 0, weight, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             GetGameFontQuality(), DEFAULT_PITCH | FF_DONTCARE, GetGameFontName());
    HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, font));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);

    SIZE textSize = {};
    GetTextExtentPoint32A(hdc, text.c_str(), static_cast<int>(text.size()), &textSize);
    int x = left + (right - left - textSize.cx) / 2;
    TextOutA(hdc, x, y, text.c_str(), static_cast<int>(text.size()));

    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

void FillRectColor(HDC hdc, int left, int top, int right, int bottom, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    RECT rect = {left, top, right, bottom};
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
}

void FillVerticalGradient(HDC hdc, int left, int top, int right, int bottom,
                          COLORREF topColor, COLORREF bottomColor)
{
    int height = bottom - top;
    for (int index = 0; index < height; index++)
    {
        int red = GetRValue(topColor) + (GetRValue(bottomColor) - GetRValue(topColor)) * index / height;
        int green = GetGValue(topColor) + (GetGValue(bottomColor) - GetGValue(topColor)) * index / height;
        int blue = GetBValue(topColor) + (GetBValue(bottomColor) - GetBValue(topColor)) * index / height;
        FillRectColor(hdc, left, top + index, right, top + index + 1, RGB(red, green, blue));
    }
}

void FillRoundRectColor(HDC hdc, int left, int top, int right, int bottom, int radius,
                        COLORREF fillColor, COLORREF borderColor)
{
    HBRUSH brush = CreateSolidBrush(fillColor);
    HPEN pen = CreatePen(PS_SOLID, 1, borderColor);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen = SelectObject(hdc, pen);

    RoundRect(hdc, left, top, right, bottom, radius, radius);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
    DeleteObject(brush);
}

void DrawCellAt(HDC hdc, int left, int top, int size, int blockId)
{
    if (blockId == 0)
    {
        FillRoundRectColor(hdc, left + 1, top + 1, left + size - 1, top + size - 1, 5,
                           RGB(17, 20, 20), RGB(27, 33, 31));
        return;
    }

    COLORREF color = GetBlockColor(blockId);
    FillRoundRectColor(hdc, left + 2, top + 2, left + size - 2, top + size - 2, 7,
                       color, AdjustColor(color, -55));
    FillRectColor(hdc, left + 5, top + 5, left + size - 5, top + 9, AdjustColor(color, 38));
    FillRectColor(hdc, left + 5, top + size - 9, left + size - 5, top + size - 5, AdjustColor(color, -45));
}

void DrawCell(HDC hdc, int row, int column, int blockId)
{
    int left = AppConfig::BoardLeft + column * AppConfig::CellSize;
    int top = AppConfig::BoardTop + row * AppConfig::CellSize;
    DrawCellAt(hdc, left, top, AppConfig::CellSize, blockId);
}

void DrawGhostCell(HDC hdc, int row, int column, int blockId)
{
    int left = AppConfig::BoardLeft + column * AppConfig::CellSize;
    int top = AppConfig::BoardTop + row * AppConfig::CellSize;
    COLORREF color = GetBlockColor(blockId);
    HPEN pen = CreatePen(PS_DOT, 1, AdjustColor(color, 30));
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));

    RoundRect(hdc, left + 6, top + 6, left + AppConfig::CellSize - 6, top + AppConfig::CellSize - 6, 6, 6);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

void DrawMetricPanel(HDC hdc, int x, int y, int width, int height,
                     const std::string &label, const std::string &value)
{
    FillRoundRectColor(hdc, x, y, x + width, y + height, 8,
                       RGB(31, 35, 36), RGB(61, 70, 62));
    DrawTextLine(hdc, x + 14, y + 10, label, 16, RGB(170, 178, 158), FW_NORMAL);
    DrawTextLine(hdc, x + 14, y + 32, value, 28, RGB(248, 244, 225), FW_BOLD);
}

void DrawBlockPreview(HDC hdc, int x, int y, int width, int height,
                      const std::string &label, const std::vector<Position> &cells,
                      int blockId, bool dimmed)
{
    FillRoundRectColor(hdc, x, y, x + width, y + height, 8,
                       RGB(31, 35, 36), RGB(61, 70, 62));
    COLORREF labelColor = dimmed ? RGB(115, 122, 108) : RGB(170, 178, 158);
    DrawTextLine(hdc, x + 14, y + 10, label, 16, labelColor, FW_NORMAL);

    if (cells.empty())
    {
        DrawTextLine(hdc, x + 22, y + height / 2 - 8, "EMPTY", 16, RGB(100, 108, 96), FW_BOLD);
        return;
    }

    int minRow = cells[0].row;
    int maxRow = cells[0].row;
    int minColumn = cells[0].column;
    int maxColumn = cells[0].column;
    for (Position cell : cells)
    {
        if (cell.row < minRow)
        {
            minRow = cell.row;
        }
        if (cell.row > maxRow)
        {
            maxRow = cell.row;
        }
        if (cell.column < minColumn)
        {
            minColumn = cell.column;
        }
        if (cell.column > maxColumn)
        {
            maxColumn = cell.column;
        }
    }

    const int previewCellSize = 22;
    int shapeWidth = (maxColumn - minColumn + 1) * previewCellSize;
    int shapeHeight = (maxRow - minRow + 1) * previewCellSize;
    int originX = x + (width - shapeWidth) / 2;
    int originY = y + 44 + (height - 56 - shapeHeight) / 2;

    for (Position cell : cells)
    {
        int cellX = originX + (cell.column - minColumn) * previewCellSize;
        int cellY = originY + (cell.row - minRow) * previewCellSize;
        DrawCellAt(hdc, cellX, cellY, previewCellSize, blockId);
    }
}

void DrawBlockQueuePreview(HDC hdc, int x, int y, int width, int height,
                           const std::string &label,
                           const std::vector<std::vector<Position>> &blocks,
                           const std::vector<int> &blockIds)
{
    FillRoundRectColor(hdc, x, y, x + width, y + height, 8,
                       RGB(31, 35, 36), RGB(61, 70, 62));
    DrawTextLine(hdc, x + 14, y + 10, label, 16, RGB(170, 178, 158), FW_NORMAL);

    int count = static_cast<int>(blocks.size());
    if (static_cast<int>(blockIds.size()) < count)
    {
        count = static_cast<int>(blockIds.size());
    }
    if (count <= 0)
    {
        return;
    }

    const int slotGap = 8;
    const int innerLeft = x + 12;
    const int contentTop = y + 42;
    const int contentHeight = height - 52;
    const int slotWidth = (width - 24 - slotGap * (count - 1)) / count;
    const int previewCellSize = 15;

    for (int index = 0; index < count; index++)
    {
        const std::vector<Position> &cells = blocks[index];
        if (cells.empty())
        {
            continue;
        }

        int slotLeft = innerLeft + index * (slotWidth + slotGap);
        if (index > 0)
        {
            FillRectColor(hdc, slotLeft - slotGap / 2, contentTop + 8,
                          slotLeft - slotGap / 2 + 1, contentTop + contentHeight - 8,
                          RGB(57, 65, 57));
        }

        int minRow = cells[0].row;
        int maxRow = cells[0].row;
        int minColumn = cells[0].column;
        int maxColumn = cells[0].column;
        for (Position cell : cells)
        {
            if (cell.row < minRow)
            {
                minRow = cell.row;
            }
            if (cell.row > maxRow)
            {
                maxRow = cell.row;
            }
            if (cell.column < minColumn)
            {
                minColumn = cell.column;
            }
            if (cell.column > maxColumn)
            {
                maxColumn = cell.column;
            }
        }

        int shapeWidth = (maxColumn - minColumn + 1) * previewCellSize;
        int shapeHeight = (maxRow - minRow + 1) * previewCellSize;
        int originX = slotLeft + (slotWidth - shapeWidth) / 2;
        int originY = contentTop + (contentHeight - shapeHeight) / 2;

        for (Position cell : cells)
        {
            int cellX = originX + (cell.column - minColumn) * previewCellSize;
            int cellY = originY + (cell.row - minRow) * previewCellSize;
            DrawCellAt(hdc, cellX, cellY, previewCellSize, blockIds[index]);
        }
    }
}

std::string GetClearLabel(int clearedLines)
{
    switch (clearedLines)
    {
    case 1:
        return "SINGLE";
    case 2:
        return "DOUBLE";
    case 3:
        return "TRIPLE";
    case 4:
        return "TETRIS";
    default:
        return "";
    }
}

std::string GetSpinLabel(int blockId)
{
    switch (blockId)
    {
    case 1:
        return "L-SPIN";
    case 2:
        return "J-SPIN";
    case 3:
        return "I-SPIN";
    case 5:
        return "S-SPIN";
    case 6:
        return "T-SPIN";
    case 7:
        return "Z-SPIN";
    default:
        return "";
    }
}

std::string BuildClearHeadline()
{
    std::string clearLabel = GetClearLabel(game.GetLastClearLines());
    if (game.WasLastClearPerfectClear())
    {
        return "PERFECT CLEAR";
    }
    if (game.WasLastClearSpin())
    {
        return GetSpinLabel(game.GetLastClearSpinBlockId()) + " " + clearLabel;
    }
    return clearLabel;
}

std::string BuildClearDetail()
{
    std::string detail = "+" + std::to_string(game.GetLastClearScore());
    if (game.WasLastClearBackToBack())
    {
        detail += "  B2B";
    }
    if (game.GetCombo() > 1)
    {
        detail += "  COMBO x" + std::to_string(game.GetCombo());
    }
    return detail;
}

void DrawStatusPanel(HDC hdc, int x, int y, int width)
{
    std::string status = "READY";
    COLORREF statusColor = RGB(249, 214, 124);
    if (game.IsGameOver())
    {
        status = "GAME OVER";
        statusColor = RGB(240, 101, 95);
    }
    else if (settingsOpen)
    {
        status = "SETTINGS";
        statusColor = RGB(123, 205, 236);
    }
    else if (!game.IsStarted())
    {
        status = "READY";
        statusColor = RGB(249, 214, 124);
    }
    else if (game.IsPaused())
    {
        status = "PAUSED";
        statusColor = RGB(241, 194, 100);
    }
    else
    {
        status = "PLAYING";
        statusColor = RGB(122, 214, 176);
    }

    FillRoundRectColor(hdc, x, y, x + width, y + 78, 8,
                       RGB(31, 35, 36), RGB(61, 70, 62));
    DrawTextLine(hdc, x + 14, y + 10, "Status", 16, RGB(170, 178, 158), FW_NORMAL);
    if (observedLevelUpEventId > 0 &&
        GetTickCount() - levelFlashStartedAt < static_cast<DWORD>(AppConfig::LevelFlashDurationMs) &&
        game.IsStarted() && !game.IsGameOver() && !game.IsPaused() && !settingsOpen)
    {
        DrawTextLine(hdc, x + 14, y + 30, "LEVEL UP", 22, RGB(122, 214, 176), FW_BOLD);
        DrawTextLine(hdc, x + 14, y + 56,
                     "Level " + std::to_string(game.GetLastLevelReached()),
                     16, RGB(170, 178, 158), FW_NORMAL);
    }
    else if (game.GetLastClearLines() > 0 && game.IsStarted() && !game.IsGameOver() &&
             !game.IsPaused() && !settingsOpen)
    {
        DrawTextLine(hdc, x + 14, y + 30, BuildClearHeadline(), 20, RGB(249, 214, 124), FW_BOLD);
        DrawTextLine(hdc, x + 14, y + 56, BuildClearDetail(), 16, RGB(248, 244, 225), FW_BOLD);
    }
    else
    {
        DrawTextLine(hdc, x + 14, y + 34, status, 24, statusColor, FW_BOLD);
    }
}

void DrawOverlayPanel(HDC hdc, const std::string &title, const std::string &action,
                      const std::string &detail, COLORREF accent)
{
    int left = AppConfig::BoardLeft + 22;
    int right = AppConfig::BoardLeft + AppConfig::CellSize * 10 - 22;
    int top = AppConfig::BoardTop + 164;
    int bottom = AppConfig::BoardTop + 394;

    FillRoundRectColor(hdc, left + 5, top + 7, right + 5, bottom + 7, 12,
                       RGB(10, 12, 12), RGB(10, 12, 12));
    FillRoundRectColor(hdc, left, top, right, bottom, 12,
                       RGB(31, 35, 36), AdjustColor(accent, -65));
    FillRectColor(hdc, left + 18, top + 16, right - 18, top + 20, accent);

    DrawTextCentered(hdc, left, right, top + 44, title, 34, RGB(248, 244, 225), FW_BOLD);
    DrawTextCentered(hdc, left, right, top + 96, action, 18, accent, FW_BOLD);
    DrawTextCentered(hdc, left, right, top + 138, detail, 17, RGB(170, 178, 158), FW_NORMAL);
}

void DrawMenuItem(HDC hdc, int left, int right, int top, const std::string &label, bool selected)
{
    COLORREF border = selected ? RGB(249, 214, 124) : RGB(66, 76, 66);
    COLORREF fill = selected ? RGB(47, 49, 39) : RGB(26, 31, 31);
    COLORREF text = selected ? RGB(248, 244, 225) : RGB(178, 187, 168);
    FillRoundRectColor(hdc, left, top, right, top + 42, 8, fill, border);
    if (selected)
    {
        FillRectColor(hdc, left + 12, top + 12, left + 16, top + 30, RGB(249, 214, 124));
    }
    DrawTextCentered(hdc, left, right, top + 10, label, 20, text, selected ? FW_BOLD : FW_NORMAL);
}

void DrawMenuPanel(HDC hdc, const std::string &title, const std::vector<std::string> &items,
                   int selectedIndex, COLORREF accent)
{
    int left = AppConfig::BoardLeft + 18;
    int right = AppConfig::BoardLeft + AppConfig::CellSize * 10 - 18;
    int top = AppConfig::BoardTop + 90;
    int bottom = AppConfig::BoardTop + 466;

    FillRoundRectColor(hdc, left + 5, top + 7, right + 5, bottom + 7, 12,
                       RGB(10, 12, 12), RGB(10, 12, 12));
    FillRoundRectColor(hdc, left, top, right, bottom, 12,
                       RGB(28, 34, 34), AdjustColor(accent, -65));
    FillRectColor(hdc, left + 18, top + 16, right - 18, top + 20, accent);
    DrawTextCentered(hdc, left, right, top + 42, title, 34, RGB(248, 244, 225), FW_BOLD);

    int itemTop = top + 102;
    for (int index = 0; index < static_cast<int>(items.size()); index++)
    {
        DrawMenuItem(hdc, left + 34, right - 34, itemTop + index * 52,
                     items[index], selectedIndex == index);
    }
}

void DrawMainMenuOverlay(HDC hdc)
{
    DrawMenuPanel(hdc, "TETRIS",
                  {"START", "SETTINGS", "LEADERBOARD", "QUIT"},
                  selectedMainMenuIndex, RGB(249, 214, 124));
}

void DrawPauseMenuOverlay(HDC hdc)
{
    DrawMenuPanel(hdc, "PAUSED",
                  {"CONTINUE", "RESTART", "SETTINGS", "LEADERBOARD", "QUIT"},
                  selectedPauseMenuIndex, RGB(241, 194, 100));
}

void DrawLeaderboardOverlay(HDC hdc)
{
    int left = AppConfig::BoardLeft + 18;
    int right = AppConfig::BoardLeft + AppConfig::CellSize * 10 - 18;
    int top = AppConfig::BoardTop + 104;
    int bottom = AppConfig::BoardTop + 484;
    COLORREF accent = RGB(123, 205, 236);

    FillRoundRectColor(hdc, left + 5, top + 7, right + 5, bottom + 7, 12,
                       RGB(10, 12, 12), RGB(10, 12, 12));
    FillRoundRectColor(hdc, left, top, right, bottom, 12,
                       RGB(25, 31, 32), AdjustColor(accent, -65));
    FillRectColor(hdc, left + 18, top + 16, right - 18, top + 20, accent);
    DrawTextCentered(hdc, left, right, top + 42, "LEADERBOARD", 30, RGB(248, 244, 225), FW_BOLD);

    std::vector<LeaderboardEntry> ranked = NormalizeLeaderboard(leaderboard);
    int rowsToDraw = static_cast<int>(ranked.size());
    if (rowsToDraw > LeaderboardMaxEntries)
    {
        rowsToDraw = LeaderboardMaxEntries;
    }

    if (rowsToDraw == 0)
    {
        DrawTextCentered(hdc, left, right, top + 152, "NO SCORES YET", 20,
                         RGB(132, 142, 126), FW_NORMAL);
    }
    else
    {
        for (int index = 0; index < rowsToDraw; index++)
        {
            int rowTop = top + 104 + index * 42;
            COLORREF rowFill = index == 0 ? RGB(47, 49, 39) : RGB(29, 34, 34);
            COLORREF rowBorder = index == 0 ? RGB(249, 214, 124) : RGB(66, 76, 66);
            FillRoundRectColor(hdc, left + 34, rowTop, right - 34, rowTop + 34, 8,
                               rowFill, rowBorder);
            DrawTextLine(hdc, left + 50, rowTop + 8, std::to_string(index + 1) + ".",
                         17, RGB(123, 205, 236), FW_BOLD);
            DrawTextLine(hdc, left + 90, rowTop + 8, ranked[index].name,
                         17, RGB(248, 244, 225), FW_BOLD);
            DrawTextLine(hdc, right - 122, rowTop + 8, std::to_string(ranked[index].score),
                         17, RGB(249, 214, 124), FW_BOLD);
        }
    }

    DrawMenuItem(hdc, left + 52, right - 52, bottom - 64, "BACK", true);
}

void DrawGameOverOverlay(HDC hdc)
{
    int left = AppConfig::BoardLeft + 18;
    int right = AppConfig::BoardLeft + AppConfig::CellSize * 10 - 18;
    int top = AppConfig::BoardTop + 112;
    int bottom = AppConfig::BoardTop + 438;
    COLORREF accent = RGB(240, 101, 95);

    FillRoundRectColor(hdc, left + 5, top + 7, right + 5, bottom + 7, 12,
                       RGB(10, 12, 12), RGB(10, 12, 12));
    FillRoundRectColor(hdc, left, top, right, bottom, 12,
                       RGB(31, 35, 36), AdjustColor(accent, -65));
    FillRectColor(hdc, left + 18, top + 16, right - 18, top + 20, accent);

    if (leaderboardNameEntryActive)
    {
        DrawTextCentered(hdc, left, right, top + 36, "NEW TOP SCORE", 30, RGB(248, 244, 225), FW_BOLD);
        DrawTextCentered(hdc, left, right, top + 80, "TYPE YOUR NAME", 18, accent, FW_BOLD);
        DrawTextCentered(hdc, left, right, top + 112,
                         "Score " + std::to_string(pendingLeaderboardScore),
                         18, RGB(170, 178, 158), FW_NORMAL);

        int inputLeft = left + 38;
        int inputRight = right - 38;
        int inputTop = top + 152;
        FillRoundRectColor(hdc, inputLeft, inputTop, inputRight, inputTop + 52, 8,
                           RGB(18, 22, 22), RGB(123, 205, 236));

        std::string displayName = leaderboardNameInput.empty() ? "___" : leaderboardNameInput + "_";
        DrawTextCentered(hdc, inputLeft, inputRight, inputTop + 13,
                         displayName, 24, RGB(248, 244, 225), FW_BOLD);
        DrawTextCentered(hdc, left, right, top + 228,
                         "A-Z 0-9   ENTER SAVE   ESC PLAYER",
                         14, RGB(170, 178, 158), FW_NORMAL);
        DrawTextCentered(hdc, left, right, top + 258,
                         "3-12 CHARACTERS",
                         14, RGB(123, 205, 236), FW_BOLD);
        return;
    }

    DrawTextCentered(hdc, left, right, top + 38, "GAME OVER", 34, RGB(248, 244, 225), FW_BOLD);
    DrawTextCentered(hdc, left, right, top + 84, "PRESS ANY KEY", 18, accent, FW_BOLD);
    DrawTextCentered(hdc, left, right, top + 118,
                     "Score " + std::to_string(game.GetScore()) + "   Best " + std::to_string(game.GetHighScore()),
                     17, RGB(170, 178, 158), FW_NORMAL);
    DrawTextCentered(hdc, left, right, top + 154, "TOP SCORES", 18, RGB(248, 244, 225), FW_BOLD);

    std::vector<LeaderboardEntry> ranked = NormalizeLeaderboard(leaderboard);
    int rowsToDraw = static_cast<int>(ranked.size());
    if (rowsToDraw > 3)
    {
        rowsToDraw = 3;
    }

    if (rowsToDraw == 0)
    {
        DrawTextCentered(hdc, left, right, top + 190, "NO SCORES YET", 16, RGB(132, 142, 126), FW_NORMAL);
        return;
    }

    for (int index = 0; index < rowsToDraw; index++)
    {
        int rowTop = top + 184 + index * 34;
        std::string rank = std::to_string(index + 1) + ".";
        DrawTextLine(hdc, left + 44, rowTop, rank, 17, RGB(123, 205, 236), FW_BOLD);
        DrawTextLine(hdc, left + 82, rowTop, ranked[index].name, 17, RGB(248, 244, 225), FW_BOLD);
        DrawTextLine(hdc, right - 104, rowTop, std::to_string(ranked[index].score),
                     17, RGB(249, 214, 124), FW_BOLD);
    }
}

void DrawStateOverlay(HDC hdc)
{
    if (leaderboardOpen)
    {
        DrawLeaderboardOverlay(hdc);
    }
    else if (game.IsGameOver())
    {
        DrawGameOverOverlay(hdc);
    }
    else if (!game.IsStarted())
    {
        DrawMainMenuOverlay(hdc);
    }
    else if (game.IsPaused())
    {
        DrawPauseMenuOverlay(hdc);
    }
}

std::string GetSettingLabel(int index)
{
    switch (index)
    {
    case SettingsTuningPreset:
        return "PRESET";
    case SettingsDasDelay:
        return "DAS";
    case SettingsArrInterval:
        return "ARR";
    case SettingsClearFlash:
        return "CLEAR FX";
    case SettingsControlScheme:
        return "CONTROL";
    case SettingsWindowScale:
        return "WINDOW";
    case SettingsSound:
        return "SOUND";
    case SettingsSoundVolume:
        return "VOLUME";
    case SettingsPlayerName:
        return "NAME";
    default:
        return "";
    }
}

std::string GetTuningPresetName()
{
    if (tuningPreset == 0)
    {
        return "BEGINNER";
    }
    if (tuningPreset == 1)
    {
        return "BALANCED";
    }
    if (tuningPreset == 2)
    {
        return "FAST";
    }
    return "CUSTOM";
}

std::string GetControlSchemeName()
{
    if (controlScheme == 1)
    {
        return "ARROWS";
    }
    if (controlScheme == 2)
    {
        return "WASD";
    }
    return "HYBRID";
}

std::string GetSoundModeName()
{
    return IsSoundEnabled() ? "ON" : "OFF";
}

std::string GetSettingValue(int index)
{
    switch (index)
    {
    case SettingsTuningPreset:
        return GetTuningPresetName();
    case SettingsDasDelay:
        return std::to_string(dasDelayMs) + " ms";
    case SettingsArrInterval:
        return std::to_string(arrIntervalMs) + " ms";
    case SettingsClearFlash:
        return std::to_string(clearFlashDurationMs) + " ms";
    case SettingsControlScheme:
        return GetControlSchemeName();
    case SettingsWindowScale:
        return std::to_string(windowScalePercent) + "%";
    case SettingsSound:
        return GetSoundModeName();
    case SettingsSoundVolume:
        return std::to_string(GetSoundVolumePercent()) + "%";
    case SettingsPlayerName:
        return settingsNameEditActive ? settingsNameDraft + "_" : defaultPlayerName;
    default:
        return "";
    }
}

void DrawSettingMeter(HDC hdc, int left, int top, int width, int value, int minimum, int maximum,
                      COLORREF fillColor)
{
    FillRoundRectColor(hdc, left, top, left + width, top + 8, 5,
                       RGB(17, 20, 20), RGB(55, 62, 55));
    int fillWidth = (value - minimum) * width / (maximum - minimum);
    if (fillWidth < 8)
    {
        fillWidth = 8;
    }
    FillRoundRectColor(hdc, left, top, left + fillWidth, top + 8, 5,
                       fillColor, AdjustColor(fillColor, -50));
}

void DrawSettingsRow(HDC hdc, int index, int top)
{
    const int left = AppConfig::BoardLeft + 42;
    const int right = AppConfig::WindowWidth - 42;
    bool selected = selectedSettingIndex == index;
    COLORREF accent = selected ? RGB(123, 205, 236) : RGB(86, 98, 88);
    COLORREF fill = selected ? RGB(39, 48, 49) : RGB(29, 34, 34);
    COLORREF labelColor = selected ? RGB(248, 244, 225) : RGB(178, 187, 168);
    COLORREF valueColor = selected ? RGB(123, 205, 236) : RGB(248, 244, 225);

    FillRoundRectColor(hdc, left, top, right, top + 44, 8, fill, accent);
    DrawTextLine(hdc, left + 16, top + 11, GetSettingLabel(index), 18, labelColor, FW_BOLD);
    if (index != SettingsPlayerName)
    {
        DrawTextLine(hdc, right - 134, top + 11, GetSettingValue(index), 18, valueColor, FW_BOLD);
    }

    if (index == SettingsDasDelay)
    {
        DrawSettingMeter(hdc, left + 156, top + 30, 150, dasDelayMs,
                         SettingsMinDasDelayMs, SettingsMaxDasDelayMs, accent);
    }
    else if (index == SettingsArrInterval)
    {
        DrawSettingMeter(hdc, left + 156, top + 30, 150, arrIntervalMs,
                         SettingsMinArrIntervalMs, SettingsMaxArrIntervalMs, accent);
    }
    else if (index == SettingsClearFlash)
    {
        DrawSettingMeter(hdc, left + 156, top + 30, 150, clearFlashDurationMs,
                         SettingsMinClearFlashDurationMs, SettingsMaxClearFlashDurationMs, accent);
    }
    else if (index == SettingsWindowScale)
    {
        DrawSettingMeter(hdc, left + 156, top + 30, 150, windowScalePercent,
                         SettingsMinWindowScalePercent, SettingsMaxWindowScalePercent, accent);
    }
    else if (index == SettingsSoundVolume)
    {
        DrawSettingMeter(hdc, left + 156, top + 30, 150, GetSoundVolumePercent(),
                         SettingsMinSoundVolumePercent, SettingsMaxSoundVolumePercent, accent);
    }
    else if (index == SettingsPlayerName)
    {
        int inputLeft = left + 156;
        COLORREF inputBorder = settingsNameEditActive ? RGB(123, 205, 236) : RGB(86, 98, 88);
        FillRoundRectColor(hdc, inputLeft, top + 9, inputLeft + 170, top + 35, 8,
                           RGB(18, 22, 22), inputBorder);
        DrawTextLine(hdc, inputLeft + 12, top + 12, GetSettingValue(index), 16, valueColor, FW_BOLD);
    }
    else if (index == SettingsSound)
    {
        int switchLeft = left + 156;
        COLORREF switchColor = IsSoundEnabled() ? RGB(122, 214, 176) : RGB(119, 124, 116);
        FillRoundRectColor(hdc, switchLeft, top + 19, switchLeft + 82, top + 33, 8,
                           switchColor, AdjustColor(switchColor, -55));
        int knobLeft = IsSoundEnabled() ? switchLeft + 56 : switchLeft + 6;
        FillRoundRectColor(hdc, knobLeft, top + 17, knobLeft + 22, top + 35, 9,
                           RGB(248, 244, 225), RGB(210, 204, 184));
    }
}

void DrawSettingsOverlay(HDC hdc)
{
    if (!settingsOpen)
    {
        return;
    }

    const int left = AppConfig::BoardLeft + 34;
    const int right = AppConfig::WindowWidth - 34;
    const int top = AppConfig::BoardTop + 42;
    const int bottom = AppConfig::BoardTop + 592;

    FillRoundRectColor(hdc, left + 6, top + 8, right + 6, bottom + 8, 12,
                       RGB(9, 11, 11), RGB(9, 11, 11));
    FillRoundRectColor(hdc, left, top, right, bottom, 12,
                       RGB(25, 31, 32), RGB(65, 129, 146));
    FillRectColor(hdc, left + 22, top + 18, right - 22, top + 22, RGB(123, 205, 236));

    DrawTextCentered(hdc, left, right, top + 36, "SETTINGS", 30,
                     RGB(248, 244, 225), FW_BOLD);

    for (int index = 0; index < SettingsOptionCount; index++)
    {
        DrawSettingsRow(hdc, index, top + 78 + index * 50);
    }
}

bool IsClearFlashActive()
{
    if (!game.IsLineClearPending() || observedClearEventId == 0 || game.GetLastClearedRows().empty())
    {
        return false;
    }

    DWORD elapsed = GetTickCount() - clearFlashStartedAt;
    return elapsed < static_cast<DWORD>(clearFlashDurationMs);
}

bool IsClearFlashReadyToFinish()
{
    if (settingsOpen)
    {
        return false;
    }

    if (!game.IsLineClearPending() || observedClearEventId == 0)
    {
        return false;
    }

    DWORD elapsed = GetTickCount() - clearFlashStartedAt;
    return elapsed >= static_cast<DWORD>(clearFlashDurationMs);
}

bool IsLevelFlashActive()
{
    if (observedLevelUpEventId == 0)
    {
        return false;
    }

    DWORD elapsed = GetTickCount() - levelFlashStartedAt;
    return elapsed < static_cast<DWORD>(AppConfig::LevelFlashDurationMs);
}

bool IsLockDelayReadyToFinish()
{
    if (settingsOpen)
    {
        return false;
    }

    if (!game.IsLockDelayActive())
    {
        return false;
    }

    DWORD elapsed = GetTickCount() - lockDelayStartedAt;
    return elapsed >= static_cast<DWORD>(AppConfig::LockDelayMs);
}

void UpdateClearFlash(HWND hwnd)
{
    int clearEventId = game.GetClearEventId();
    if (clearEventId != observedClearEventId)
    {
        observedClearEventId = clearEventId;
        if (clearEventId > 0 && game.IsLineClearPending())
        {
            clearFlashStartedAt = GetTickCount();
            KillTimer(hwnd, AppConfig::DropTimerId);
            SetTimer(hwnd, AppConfig::ClearFlashTimerId, 24, nullptr);
        }
    }
}

void UpdateLevelFlash(HWND hwnd)
{
    int levelUpEventId = game.GetLevelUpEventId();
    if (levelUpEventId != observedLevelUpEventId)
    {
        observedLevelUpEventId = levelUpEventId;
        if (levelUpEventId > 0)
        {
            levelFlashStartedAt = GetTickCount();
            SetTimer(hwnd, AppConfig::LevelFlashTimerId, 45, nullptr);
        }
    }
}

void UpdateLockDelay(HWND hwnd)
{
    if (!game.IsLockDelayActive())
    {
        KillTimer(hwnd, AppConfig::LockDelayTimerId);
        lockDelayTimerRunning = false;
        return;
    }

    if (game.IsPaused() || settingsOpen)
    {
        KillTimer(hwnd, AppConfig::LockDelayTimerId);
        lockDelayTimerRunning = false;
        return;
    }

    int lockDelayEventId = game.GetLockDelayEventId();
    if (lockDelayEventId != observedLockDelayEventId || !lockDelayTimerRunning)
    {
        observedLockDelayEventId = lockDelayEventId;
        lockDelayStartedAt = GetTickCount();
        KillTimer(hwnd, AppConfig::DropTimerId);
        SetTimer(hwnd, AppConfig::LockDelayTimerId, 30, nullptr);
        lockDelayTimerRunning = true;
    }
}

void FinishClearFlash(HWND hwnd)
{
    game.FinishLineClear();
    RecordGameOverScoreIfNeeded();
    KillTimer(hwnd, AppConfig::ClearFlashTimerId);
    SetTimer(hwnd, AppConfig::DropTimerId, game.GetDropIntervalMs(), nullptr);
}

void FinishLockDelay(HWND hwnd)
{
    game.FinishLockDelay();
    RecordGameOverScoreIfNeeded();
    KillTimer(hwnd, AppConfig::LockDelayTimerId);
    lockDelayTimerRunning = false;
    UpdateClearFlash(hwnd);
    UpdateLevelFlash(hwnd);
    if (!game.IsLineClearPending() && !game.IsLockDelayActive() && !game.IsPaused() && !settingsOpen)
    {
        SetTimer(hwnd, AppConfig::DropTimerId, game.GetDropIntervalMs(), nullptr);
    }
}

bool IsHorizontalMoveKey(WPARAM key)
{
    return (AllowArrowControls() && (key == VK_LEFT || key == VK_RIGHT)) ||
           (AllowWasdControls() && (key == 'A' || key == 'D'));
}

int GetHorizontalDirection(WPARAM key)
{
    if ((AllowArrowControls() && key == VK_LEFT) || (AllowWasdControls() && key == 'A'))
    {
        return -1;
    }
    if ((AllowArrowControls() && key == VK_RIGHT) || (AllowWasdControls() && key == 'D'))
    {
        return 1;
    }
    return 0;
}

int GetHorizontalInput(int direction)
{
    if (direction < 0)
    {
        return 'a';
    }
    if (direction > 0)
    {
        return 'd';
    }
    return 0;
}

bool CanRepeatHorizontalInput()
{
    return game.IsStarted() && !game.IsGameOver() && !game.IsPaused() && !settingsOpen &&
           !game.IsLineClearPending();
}

void StartHorizontalInputTimer(HWND hwnd)
{
    if (!inputTimerRunning)
    {
        SetTimer(hwnd, AppConfig::InputTimerId, 12, nullptr);
        inputTimerRunning = true;
    }
}

void StopHorizontalInputTimer(HWND hwnd)
{
    KillTimer(hwnd, AppConfig::InputTimerId);
    inputTimerRunning = false;
}

void ResetHorizontalInput(HWND hwnd)
{
    leftHeld = false;
    rightHeld = false;
    horizontalDirection = 0;
    StopHorizontalInputTimer(hwnd);
}

void ResetRunUiState(HWND hwnd)
{
    observedClearEventId = 0;
    observedLevelUpEventId = 0;
    observedLockDelayEventId = 0;
    clearFlashStartedAt = 0;
    levelFlashStartedAt = 0;
    lockDelayStartedAt = 0;
    leaderboardNameEntryActive = false;
    gameOverScoreRecorded = false;
    pendingLeaderboardScore = 0;
    leaderboardNameInput.clear();
    KillTimer(hwnd, AppConfig::ClearFlashTimerId);
    KillTimer(hwnd, AppConfig::LevelFlashTimerId);
    KillTimer(hwnd, AppConfig::LockDelayTimerId);
    lockDelayTimerRunning = false;
    ResetHorizontalInput(hwnd);
}

void StartDropTimer(HWND hwnd)
{
    KillTimer(hwnd, AppConfig::DropTimerId);
    SetTimer(hwnd, AppConfig::DropTimerId, game.GetDropIntervalMs(), nullptr);
}

void ApplyWindowScale(HWND hwnd)
{
    RECT windowRect = {0, 0, GetScaledWindowWidth(), GetScaledWindowHeight()};
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
    SetWindowPos(hwnd, nullptr, 0, 0,
                 windowRect.right - windowRect.left,
                 windowRect.bottom - windowRect.top,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    InvalidateRect(hwnd, nullptr, FALSE);
}

void StartGameFromMenu(HWND hwnd)
{
    leaderboardOpen = false;
    settingsOpen = false;
    settingsNameEditActive = false;
    game.Start();
    ResetRunUiState(hwnd);
    StartDropTimer(hwnd);
    InvalidateRect(hwnd, nullptr, FALSE);
}

void RestartGameFromMenu(HWND hwnd)
{
    leaderboardOpen = false;
    settingsOpen = false;
    settingsNameEditActive = false;
    game.Restart();
    ResetRunUiState(hwnd);
    StartDropTimer(hwnd);
    InvalidateRect(hwnd, nullptr, FALSE);
}

void ResumeGameFromMenu(HWND hwnd)
{
    leaderboardOpen = false;
    if (game.IsPaused())
    {
        game.HandleInput('p');
    }
    UpdateLockDelay(hwnd);
    if (!game.IsLineClearPending() && !game.IsLockDelayActive())
    {
        StartDropTimer(hwnd);
    }
    InvalidateRect(hwnd, nullptr, FALSE);
}

void OpenLeaderboard(HWND hwnd)
{
    leaderboardOpen = true;
    settingsOpen = false;
    settingsNameEditActive = false;
    ResetHorizontalInput(hwnd);
    InvalidateRect(hwnd, nullptr, FALSE);
}

void CloseLeaderboard(HWND hwnd)
{
    leaderboardOpen = false;
    InvalidateRect(hwnd, nullptr, FALSE);
}

void ToggleSettings(HWND hwnd)
{
    leaderboardOpen = false;
    settingsOpen = !settingsOpen;
    settingsNameEditActive = false;
    settingsNameDraft.clear();
    ResetHorizontalInput(hwnd);

    if (settingsOpen)
    {
        KillTimer(hwnd, AppConfig::DropTimerId);
        KillTimer(hwnd, AppConfig::LockDelayTimerId);
        lockDelayTimerRunning = false;
    }
    else
    {
        UpdateLockDelay(hwnd);
        if (!game.IsLineClearPending() && !game.IsLockDelayActive() && !game.IsPaused())
        {
            SetTimer(hwnd, AppConfig::DropTimerId, game.GetDropIntervalMs(), nullptr);
        }
    }

    InvalidateRect(hwnd, nullptr, FALSE);
}

void AdjustSelectedSetting(HWND hwnd, int direction)
{
    if (selectedSettingIndex == SettingsTuningPreset)
    {
        tuningPreset = CycleInt(tuningPreset, SettingsMinTuningPreset, SettingsMaxTuningPreset, direction);
        ApplyTuningPresetValues();
    }
    else if (selectedSettingIndex == SettingsDasDelay)
    {
        dasDelayMs = ClampInt(dasDelayMs + direction * 10, SettingsMinDasDelayMs, SettingsMaxDasDelayMs);
        MarkCustomTuning();
    }
    else if (selectedSettingIndex == SettingsArrInterval)
    {
        arrIntervalMs = ClampInt(arrIntervalMs + direction * 5, SettingsMinArrIntervalMs, SettingsMaxArrIntervalMs);
        MarkCustomTuning();
    }
    else if (selectedSettingIndex == SettingsClearFlash)
    {
        clearFlashDurationMs = ClampInt(clearFlashDurationMs + direction * 10,
                                        SettingsMinClearFlashDurationMs, SettingsMaxClearFlashDurationMs);
        MarkCustomTuning();
    }
    else if (selectedSettingIndex == SettingsControlScheme)
    {
        controlScheme = CycleInt(controlScheme, SettingsMinControlScheme, SettingsMaxControlScheme, direction);
        ResetHorizontalInput(hwnd);
    }
    else if (selectedSettingIndex == SettingsWindowScale)
    {
        windowScalePercent = ClampInt(windowScalePercent + direction * 25,
                                      SettingsMinWindowScalePercent,
                                      SettingsMaxWindowScalePercent);
        ApplyWindowScale(hwnd);
    }
    else if (selectedSettingIndex == SettingsSound)
    {
        if (IsSoundEnabled())
        {
            SetSoundEnabled(false);
            soundVolumePercent = GetSoundVolumePercent();
        }
        else
        {
            SetSoundEnabled(true);
            SetSoundVolumePercent(soundVolumePercent <= 0 ? SettingsDefaultSoundVolumePercent : soundVolumePercent);
            soundVolumePercent = GetSoundVolumePercent();
        }
    }
    else if (selectedSettingIndex == SettingsSoundVolume)
    {
        int volume = ClampInt(GetSoundVolumePercent() + direction * 10,
                              SettingsMinSoundVolumePercent,
                              SettingsMaxSoundVolumePercent);
        SetSoundVolumePercent(volume);
        soundVolumePercent = GetSoundVolumePercent();
    }
}

void StartSettingsNameEdit()
{
    settingsNameDraft = defaultPlayerName;
    settingsNameEditActive = true;
}

void CommitSettingsNameEdit()
{
    if (static_cast<int>(settingsNameDraft.size()) >= LeaderboardMinNameLength)
    {
        defaultPlayerName = NormalizeLeaderboardName(settingsNameDraft);
        settingsNameEditActive = false;
        settingsNameDraft.clear();
    }
}

void CancelSettingsNameEdit()
{
    settingsNameEditActive = false;
    settingsNameDraft.clear();
}

void HandleSettingsNameEditKey(WPARAM key)
{
    if (key == VK_ESCAPE)
    {
        CancelSettingsNameEdit();
        return;
    }

    if (key == VK_BACK)
    {
        if (!settingsNameDraft.empty())
        {
            settingsNameDraft.pop_back();
        }
        return;
    }

    if (key == VK_RETURN)
    {
        CommitSettingsNameEdit();
        return;
    }

    if (static_cast<int>(settingsNameDraft.size()) >= LeaderboardMaxNameLength)
    {
        return;
    }

    if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9'))
    {
        settingsNameDraft.push_back(static_cast<char>(key));
    }
}

void HandleSettingsKey(HWND hwnd, WPARAM key)
{
    if (settingsNameEditActive)
    {
        HandleSettingsNameEditKey(key);
        InvalidateRect(hwnd, nullptr, FALSE);
        return;
    }

    switch (key)
    {
    case VK_ESCAPE:
        ToggleSettings(hwnd);
        return;
    case VK_UP:
        selectedSettingIndex = (selectedSettingIndex + SettingsOptionCount - 1) % SettingsOptionCount;
        break;
    case VK_DOWN:
        selectedSettingIndex = (selectedSettingIndex + 1) % SettingsOptionCount;
        break;
    case VK_LEFT:
        AdjustSelectedSetting(hwnd, -1);
        break;
    case VK_RIGHT:
        AdjustSelectedSetting(hwnd, 1);
        break;
    case VK_RETURN:
    case VK_SPACE:
        if (selectedSettingIndex == SettingsPlayerName)
        {
            StartSettingsNameEdit();
        }
        else if (selectedSettingIndex == SettingsSound)
        {
            AdjustSelectedSetting(hwnd, 1);
        }
        else if (selectedSettingIndex == SettingsTuningPreset ||
                 selectedSettingIndex == SettingsControlScheme ||
                 selectedSettingIndex == SettingsWindowScale)
        {
            AdjustSelectedSetting(hwnd, 1);
        }
        break;
    default:
        break;
    }

    InvalidateRect(hwnd, nullptr, FALSE);
}

void ActivateMainMenuSelection(HWND hwnd)
{
    switch (selectedMainMenuIndex)
    {
    case MainMenuStart:
        StartGameFromMenu(hwnd);
        break;
    case MainMenuSettings:
        ToggleSettings(hwnd);
        break;
    case MainMenuLeaderboard:
        OpenLeaderboard(hwnd);
        break;
    case MainMenuQuit:
        PostQuitMessage(0);
        break;
    default:
        break;
    }
}

void ActivatePauseMenuSelection(HWND hwnd)
{
    switch (selectedPauseMenuIndex)
    {
    case PauseMenuContinue:
        ResumeGameFromMenu(hwnd);
        break;
    case PauseMenuRestart:
        RestartGameFromMenu(hwnd);
        break;
    case PauseMenuSettings:
        ToggleSettings(hwnd);
        break;
    case PauseMenuLeaderboard:
        OpenLeaderboard(hwnd);
        break;
    case PauseMenuQuit:
        PostQuitMessage(0);
        break;
    default:
        break;
    }
}

void HandleMainMenuKey(HWND hwnd, WPARAM key)
{
    switch (key)
    {
    case VK_UP:
    case 'W':
        selectedMainMenuIndex = (selectedMainMenuIndex + MainMenuOptionCount - 1) % MainMenuOptionCount;
        break;
    case VK_DOWN:
    case 'S':
        selectedMainMenuIndex = (selectedMainMenuIndex + 1) % MainMenuOptionCount;
        break;
    case VK_RETURN:
    case VK_SPACE:
        ActivateMainMenuSelection(hwnd);
        return;
    case VK_ESCAPE:
    case 'Q':
        PostQuitMessage(0);
        return;
    default:
        break;
    }

    InvalidateRect(hwnd, nullptr, FALSE);
}

void HandlePauseMenuKey(HWND hwnd, WPARAM key)
{
    switch (key)
    {
    case VK_UP:
    case 'W':
        selectedPauseMenuIndex = (selectedPauseMenuIndex + PauseMenuOptionCount - 1) % PauseMenuOptionCount;
        break;
    case VK_DOWN:
    case 'S':
        selectedPauseMenuIndex = (selectedPauseMenuIndex + 1) % PauseMenuOptionCount;
        break;
    case VK_RETURN:
    case VK_SPACE:
        ActivatePauseMenuSelection(hwnd);
        return;
    case VK_ESCAPE:
    case 'P':
        ResumeGameFromMenu(hwnd);
        return;
    case 'R':
        RestartGameFromMenu(hwnd);
        return;
    case 'Q':
        PostQuitMessage(0);
        return;
    default:
        break;
    }

    InvalidateRect(hwnd, nullptr, FALSE);
}

void HandleLeaderboardPanelKey(HWND hwnd, WPARAM key)
{
    if (key == VK_ESCAPE || key == VK_RETURN || key == VK_SPACE || key == VK_BACK)
    {
        CloseLeaderboard(hwnd);
    }
}

void HandleLeaderboardNameKey(HWND hwnd, WPARAM key)
{
    if (!leaderboardNameEntryActive)
    {
        return;
    }

    if (key == VK_ESCAPE)
    {
        SubmitLeaderboardName(defaultPlayerName);
        InvalidateRect(hwnd, nullptr, FALSE);
        return;
    }

    if (key == VK_BACK)
    {
        if (!leaderboardNameInput.empty())
        {
            leaderboardNameInput.pop_back();
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        return;
    }

    if (key == VK_RETURN)
    {
        if (static_cast<int>(leaderboardNameInput.size()) >= LeaderboardMinNameLength)
        {
            SubmitLeaderboardName(leaderboardNameInput);
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        return;
    }

    if (static_cast<int>(leaderboardNameInput.size()) >= LeaderboardMaxNameLength)
    {
        return;
    }

    if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9'))
    {
        leaderboardNameInput.push_back(static_cast<char>(key));
        InvalidateRect(hwnd, nullptr, FALSE);
    }
}

void ApplyGameInput(HWND hwnd, int input)
{
    if (input == 0)
    {
        return;
    }

    game.HandleInput(input);
    UpdateLockDelay(hwnd);
    UpdateClearFlash(hwnd);
    UpdateLevelFlash(hwnd);
    RecordGameOverScoreIfNeeded();
    InvalidateRect(hwnd, nullptr, FALSE);
}

void HandleHorizontalKeyDown(HWND hwnd, WPARAM key, LPARAM)
{
    int direction = GetHorizontalDirection(key);
    if (direction == 0)
    {
        return;
    }

    bool wasHeld = direction < 0 ? leftHeld : rightHeld;
    if (direction < 0)
    {
        leftHeld = true;
    }
    else
    {
        rightHeld = true;
    }

    if (!wasHeld || horizontalDirection != direction)
    {
        horizontalDirection = direction;
        horizontalHeldStartedAt = GetTickCount();
        lastHorizontalRepeatAt = horizontalHeldStartedAt;
        ApplyGameInput(hwnd, GetHorizontalInput(direction));
    }

    StartHorizontalInputTimer(hwnd);
}

void HandleHorizontalKeyUp(HWND hwnd, WPARAM key)
{
    int direction = GetHorizontalDirection(key);
    if (direction == 0)
    {
        return;
    }

    if (direction < 0)
    {
        leftHeld = false;
    }
    else
    {
        rightHeld = false;
    }

    if (horizontalDirection == direction)
    {
        horizontalDirection = rightHeld ? 1 : (leftHeld ? -1 : 0);
        horizontalHeldStartedAt = GetTickCount();
        lastHorizontalRepeatAt = horizontalHeldStartedAt;

        if (horizontalDirection == 0)
        {
            StopHorizontalInputTimer(hwnd);
            return;
        }

        ApplyGameInput(hwnd, GetHorizontalInput(horizontalDirection));
    }
    else if (!leftHeld && !rightHeld)
    {
        horizontalDirection = 0;
        StopHorizontalInputTimer(hwnd);
    }
}

void TickHorizontalInput(HWND hwnd)
{
    if (horizontalDirection == 0 || (!leftHeld && !rightHeld))
    {
        horizontalDirection = 0;
        StopHorizontalInputTimer(hwnd);
        return;
    }

    if (!CanRepeatHorizontalInput())
    {
        return;
    }

    DWORD now = GetTickCount();
    if (now - horizontalHeldStartedAt < static_cast<DWORD>(dasDelayMs) ||
        now - lastHorizontalRepeatAt < static_cast<DWORD>(arrIntervalMs))
    {
        return;
    }

    lastHorizontalRepeatAt = now;
    ApplyGameInput(hwnd, GetHorizontalInput(horizontalDirection));
}

void DrawLineClearFlash(HDC hdc)
{
    if (!IsClearFlashActive())
    {
        return;
    }

    DWORD elapsed = GetTickCount() - clearFlashStartedAt;
    COLORREF flashColor = ((elapsed / 42) % 2 == 0) ? RGB(249, 214, 124) : RGB(248, 244, 225);

    for (int row : game.GetLastClearedRows())
    {
        int top = AppConfig::BoardTop + row * AppConfig::CellSize;
        FillRoundRectColor(hdc, AppConfig::BoardLeft + 4, top + 5,
                           AppConfig::BoardLeft + AppConfig::CellSize * 10 - 4, top + AppConfig::CellSize - 5,
                           6, flashColor, AdjustColor(flashColor, -55));
        FillRectColor(hdc, AppConfig::BoardLeft + 12, top + 12,
                      AppConfig::BoardLeft + AppConfig::CellSize * 10 - 12, top + 16,
                      RGB(255, 255, 245));
    }
}

void DrawGame(HDC hdc)
{
    FillVerticalGradient(hdc, 0, 0, AppConfig::WindowWidth, AppConfig::WindowHeight,
                         RGB(18, 22, 21), RGB(35, 38, 32));
    DrawTextLine(hdc, AppConfig::BoardLeft, 20, "TETRIS", 30, RGB(248, 244, 225), FW_BOLD);
    DrawTextLine(hdc, AppConfig::BoardLeft + 118, 28, "native C++", 14, RGB(154, 164, 145), FW_NORMAL);

    FillRoundRectColor(hdc, AppConfig::BoardLeft - 8, AppConfig::BoardTop - 8,
                       AppConfig::BoardLeft + AppConfig::CellSize * 10 + 8, AppConfig::BoardTop + AppConfig::CellSize * 20 + 8,
                       10, RGB(12, 15, 15), RGB(67, 76, 66));

    int display[20][10];
    const int (&grid)[20][10] = game.GetGrid();
    for (int row = 0; row < 20; row++)
    {
        for (int column = 0; column < 10; column++)
        {
            display[row][column] = grid[row][column];
        }
    }

    for (int row = 0; row < 20; row++)
    {
        for (int column = 0; column < 10; column++)
        {
            DrawCell(hdc, row, column, display[row][column]);
        }
    }

    for (Position item : game.GetGhostBlockCells())
    {
        if (item.row >= 0 && item.row < 20 && item.column >= 0 && item.column < 10 && display[item.row][item.column] == 0)
        {
            DrawGhostCell(hdc, item.row, item.column, game.GetCurrentBlockId());
        }
    }

    for (Position item : game.GetCurrentBlockCells())
    {
        if (item.row >= 0 && item.row < 20 && item.column >= 0 && item.column < 10)
        {
            DrawCell(hdc, item.row, item.column, game.GetCurrentBlockId());
        }
    }

    DrawLineClearFlash(hdc);

    int panelX = AppConfig::BoardLeft + AppConfig::CellSize * 10 + 34;
    int panelWidth = AppConfig::WindowWidth - panelX - 32;
    FillRoundRectColor(hdc, panelX - 12, AppConfig::BoardTop - 8,
                       AppConfig::WindowWidth - 24, AppConfig::BoardTop + AppConfig::CellSize * 20 + 8,
                       10, RGB(24, 28, 28), RGB(58, 68, 58));

    DrawMetricPanel(hdc, panelX, AppConfig::BoardTop + 10, panelWidth, 86,
                    "Score", std::to_string(game.GetScore()));
    DrawTextLine(hdc, panelX + 14, AppConfig::BoardTop + 72,
                 "Best " + std::to_string(game.GetHighScore()), 15, RGB(170, 178, 158), FW_NORMAL);
    DrawMetricPanel(hdc, panelX, AppConfig::BoardTop + 112, (panelWidth - 12) / 2, 78,
                    "Lines", std::to_string(game.GetLinesCleared()));
    DrawMetricPanel(hdc, panelX + (panelWidth + 12) / 2, AppConfig::BoardTop + 112,
                    (panelWidth - 12) / 2, 78, "Level", std::to_string(game.GetLevel()));
    DrawBlockPreview(hdc, panelX, AppConfig::BoardTop + 208, panelWidth, 126,
                     "Hold", game.GetHeldBlockCells(), game.GetHeldBlockId(), !game.CanHold());
    DrawBlockQueuePreview(hdc, panelX, AppConfig::BoardTop + 350, panelWidth, 126,
                          "Next", game.GetUpcomingBlockCells(), game.GetUpcomingBlockIds());
    DrawStatusPanel(hdc, panelX, AppConfig::BoardTop + 486, panelWidth);
    DrawStateOverlay(hdc);
    DrawSettingsOverlay(hdc);
}

void HandleGameKey(HWND hwnd, WPARAM key)
{
    if (key == 'Q')
    {
        PostQuitMessage(0);
        return;
    }

    if (game.IsGameOver())
    {
        RestartGameFromMenu(hwnd);
        return;
    }

    if (key == 'R')
    {
        RestartGameFromMenu(hwnd);
        return;
    }

    int input = 0;
    switch (key)
    {
    case VK_LEFT:
        input = AllowArrowControls() ? 'a' : 0;
        break;
    case VK_RIGHT:
        input = AllowArrowControls() ? 'd' : 0;
        break;
    case VK_DOWN:
        input = AllowArrowControls() ? 's' : 0;
        break;
    case VK_UP:
        input = AllowArrowControls() ? 'w' : 0;
        break;
    case VK_SPACE:
        input = ' ';
        break;
    case VK_SHIFT:
        input = 'c';
        break;
    case VK_ESCAPE:
        input = (game.IsStarted() && !game.IsGameOver()) ? 'p' : 0;
        break;
    case 'C':
        input = static_cast<int>(key);
        break;
    case 'A':
        input = AllowWasdControls() ? static_cast<int>(key) : 0;
        break;
    case 'D':
        input = AllowWasdControls() ? static_cast<int>(key) : 0;
        break;
    case 'S':
        input = AllowWasdControls() ? static_cast<int>(key) : 0;
        break;
    case 'W':
        input = AllowWasdControls() ? static_cast<int>(key) : 0;
        break;
    case 'X':
    case 'Z':
    case 'P':
        input = static_cast<int>(key);
        break;
    default:
        input = (game.IsPaused() || !game.IsStarted() || game.IsGameOver()) ? 'p' : 0;
        break;
    }

    if (input != 0)
    {
        ApplyGameInput(hwnd, input);
        if (game.IsPaused() || game.IsGameOver() || !game.IsStarted())
        {
            ResetHorizontalInput(hwnd);
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hwnd, AppConfig::DropTimerId, game.GetDropIntervalMs(), nullptr);
        return 0;
    case WM_TIMER:
        if (wParam == AppConfig::ClearFlashTimerId)
        {
            if (IsClearFlashReadyToFinish())
            {
                FinishClearFlash(hwnd);
            }
            else if (!game.IsLineClearPending())
            {
                KillTimer(hwnd, AppConfig::ClearFlashTimerId);
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        if (wParam == AppConfig::LevelFlashTimerId)
        {
            if (!IsLevelFlashActive())
            {
                KillTimer(hwnd, AppConfig::LevelFlashTimerId);
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        if (wParam == AppConfig::LockDelayTimerId)
        {
            if (IsLockDelayReadyToFinish())
            {
                FinishLockDelay(hwnd);
            }
            else if (!game.IsLockDelayActive())
            {
                KillTimer(hwnd, AppConfig::LockDelayTimerId);
                lockDelayTimerRunning = false;
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        if (wParam == AppConfig::InputTimerId)
        {
            TickHorizontalInput(hwnd);
            return 0;
        }

        if (settingsOpen)
        {
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        game.MoveBlockDown();
        UpdateLockDelay(hwnd);
        UpdateClearFlash(hwnd);
        UpdateLevelFlash(hwnd);
        RecordGameOverScoreIfNeeded();
        if (!game.IsLineClearPending() && !game.IsLockDelayActive())
        {
            SetTimer(hwnd, AppConfig::DropTimerId, game.GetDropIntervalMs(), nullptr);
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    case WM_KEYDOWN:
        if (leaderboardNameEntryActive)
        {
            HandleLeaderboardNameKey(hwnd, wParam);
            return 0;
        }
        if (leaderboardOpen)
        {
            HandleLeaderboardPanelKey(hwnd, wParam);
            return 0;
        }
        if (wParam == VK_F1)
        {
            ToggleSettings(hwnd);
            return 0;
        }
        if (settingsOpen)
        {
            HandleSettingsKey(hwnd, wParam);
            return 0;
        }
        if (!game.IsStarted())
        {
            HandleMainMenuKey(hwnd, wParam);
            return 0;
        }
        if (game.IsPaused())
        {
            HandlePauseMenuKey(hwnd, wParam);
            return 0;
        }
        if (IsHorizontalMoveKey(wParam))
        {
            HandleHorizontalKeyDown(hwnd, wParam, lParam);
            return 0;
        }
        HandleGameKey(hwnd, wParam);
        return 0;
    case WM_KEYUP:
        if (settingsOpen)
        {
            return 0;
        }
        if (IsHorizontalMoveKey(wParam))
        {
            HandleHorizontalKeyUp(hwnd, wParam);
            return 0;
        }
        return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT paint;
        HDC hdc = BeginPaint(hwnd, &paint);
        HDC memoryDc = CreateCompatibleDC(hdc);
        HBITMAP bitmap = CreateCompatibleBitmap(hdc, AppConfig::WindowWidth, AppConfig::WindowHeight);
        HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(memoryDc, bitmap));
        RECT clientRect = {};
        GetClientRect(hwnd, &clientRect);

        DrawGame(memoryDc);
        SetStretchBltMode(hdc, COLORONCOLOR);
        StretchBlt(hdc, 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top,
                   memoryDc, 0, 0, AppConfig::WindowWidth, AppConfig::WindowHeight, SRCCOPY);

        SelectObject(memoryDc, oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(memoryDc);
        EndPaint(hwnd, &paint);
        return 0;
    }
    case WM_DESTROY:
        KillTimer(hwnd, AppConfig::DropTimerId);
        KillTimer(hwnd, AppConfig::ClearFlashTimerId);
        KillTimer(hwnd, AppConfig::LevelFlashTimerId);
        KillTimer(hwnd, AppConfig::LockDelayTimerId);
        KillTimer(hwnd, AppConfig::InputTimerId);
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCommand)
{
    LoadGameFont();
    leaderboard = LoadLeaderboard(AppConfig::LeaderboardFile);
    int savedHighScore = LoadHighScore(AppConfig::HighScoreFile);
    if (leaderboard.empty() && savedHighScore > 0)
    {
        leaderboard = AddLeaderboardScore(leaderboard, savedHighScore);
        SaveLeaderboard(AppConfig::LeaderboardFile, leaderboard);
    }
    int leaderboardHighScore = GetBestLeaderboardScore(leaderboard);
    game.SetHighScore(savedHighScore > leaderboardHighScore ? savedHighScore : leaderboardHighScore);
    ApplyRuntimeSettings(LoadSettings(AppConfig::SettingsFile));

    const char className[] = "HuiShanTetrisWindow";

    WNDCLASSA windowClass = {};
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    RegisterClassA(&windowClass);

    RECT windowRect = {0, 0, GetScaledWindowWidth(), GetScaledWindowHeight()};
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowExA(0, className, "Tetris - By HuiShan",
                                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                windowRect.right - windowRect.left,
                                windowRect.bottom - windowRect.top,
                                nullptr, nullptr, instance, nullptr);

    if (hwnd == nullptr)
    {
        UnloadGameFont();
        return 0;
    }

    ShowWindow(hwnd, showCommand);
    UpdateWindow(hwnd);

    MSG message = {};
    while (GetMessage(&message, nullptr, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    SaveHighScore(AppConfig::HighScoreFile, game.GetHighScore());
    SaveLeaderboard(AppConfig::LeaderboardFile, leaderboard);
    SaveSettings(AppConfig::SettingsFile, CollectRuntimeSettings());
    UnloadGameFont();

    return static_cast<int>(message.wParam);
}
