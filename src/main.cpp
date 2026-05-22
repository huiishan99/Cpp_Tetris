#include "game.h"
#include "high_score.h"
#include <windows.h>
#include <string>
#include <vector>

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 700;
const int BOARD_LEFT = 32;
const int BOARD_TOP = 64;
const int CELL_SIZE = 28;
const int TIMER_ID = 1;
const int FLASH_TIMER_ID = 2;
const int LEVEL_TIMER_ID = 3;
const int LOCK_TIMER_ID = 4;
const int INPUT_TIMER_ID = 5;
const int CLEAR_FLASH_DURATION_MS = 170;
const int LEVEL_FLASH_DURATION_MS = 900;
const int LOCK_DELAY_MS = 420;
const int DAS_DELAY_MS = 145;
const int ARR_INTERVAL_MS = 35;
const char *HIGH_SCORE_FILE = "tetris_highscore.txt";
const char *PIXEL_FONT_FILE = "Font\\monogram.ttf";
const char *PIXEL_FONT_NAME = "monogram";
const char *FALLBACK_FONT_NAME = "Segoe UI";

Game game;
int observedClearEventId = 0;
int observedLevelUpEventId = 0;
int observedLockDelayEventId = 0;
DWORD clearFlashStartedAt = 0;
DWORD levelFlashStartedAt = 0;
DWORD lockDelayStartedAt = 0;
bool pixelFontLoaded = false;
bool lockDelayTimerRunning = false;
bool inputTimerRunning = false;
bool leftHeld = false;
bool rightHeld = false;
int horizontalDirection = 0;
DWORD horizontalHeldStartedAt = 0;
DWORD lastHorizontalRepeatAt = 0;

const char *GetGameFontName()
{
    return pixelFontLoaded ? PIXEL_FONT_NAME : FALLBACK_FONT_NAME;
}

DWORD GetGameFontQuality()
{
    return pixelFontLoaded ? NONANTIALIASED_QUALITY : CLEARTYPE_NATURAL_QUALITY;
}

void LoadGameFont()
{
    pixelFontLoaded = AddFontResourceExA(PIXEL_FONT_FILE, FR_PRIVATE, nullptr) > 0;
}

void UnloadGameFont()
{
    if (pixelFontLoaded)
    {
        RemoveFontResourceExA(PIXEL_FONT_FILE, FR_PRIVATE, nullptr);
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
    int left = BOARD_LEFT + column * CELL_SIZE;
    int top = BOARD_TOP + row * CELL_SIZE;
    DrawCellAt(hdc, left, top, CELL_SIZE, blockId);
}

void DrawGhostCell(HDC hdc, int row, int column, int blockId)
{
    int left = BOARD_LEFT + column * CELL_SIZE;
    int top = BOARD_TOP + row * CELL_SIZE;
    COLORREF color = GetBlockColor(blockId);
    HPEN pen = CreatePen(PS_DOT, 1, AdjustColor(color, 30));
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));

    RoundRect(hdc, left + 6, top + 6, left + CELL_SIZE - 6, top + CELL_SIZE - 6, 6, 6);

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

void DrawStatusPanel(HDC hdc, int x, int y, int width)
{
    std::string status = "READY";
    COLORREF statusColor = RGB(249, 214, 124);
    if (game.IsGameOver())
    {
        status = "GAME OVER";
        statusColor = RGB(240, 101, 95);
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
        GetTickCount() - levelFlashStartedAt < static_cast<DWORD>(LEVEL_FLASH_DURATION_MS) &&
        game.IsStarted() && !game.IsGameOver() && !game.IsPaused())
    {
        DrawTextLine(hdc, x + 14, y + 30, "LEVEL UP", 22, RGB(122, 214, 176), FW_BOLD);
        DrawTextLine(hdc, x + 14, y + 56,
                     "Level " + std::to_string(game.GetLastLevelReached()),
                     16, RGB(170, 178, 158), FW_NORMAL);
    }
    else if (game.GetLastClearLines() > 0 && game.IsStarted() && !game.IsGameOver() && !game.IsPaused())
    {
        std::string clearText = game.WasLastClearSpin()
                                    ? GetSpinLabel(game.GetLastClearSpinBlockId())
                                    : GetClearLabel(game.GetLastClearLines());
        std::string scoreText = "+" + std::to_string(game.GetLastClearScore());
        DrawTextLine(hdc, x + 14, y + 30, clearText, 22, RGB(249, 214, 124), FW_BOLD);
        DrawTextLine(hdc, x + 104, y + 33, scoreText, 18, RGB(248, 244, 225), FW_BOLD);
        std::string detailText = game.WasLastClearSpin()
                                     ? GetClearLabel(game.GetLastClearLines()) + "   Combo x" + std::to_string(game.GetCombo())
                                     : "Combo x" + std::to_string(game.GetCombo());
        DrawTextLine(hdc, x + 14, y + 56, detailText, 16, RGB(170, 178, 158), FW_NORMAL);
    }
    else
    {
        DrawTextLine(hdc, x + 14, y + 34, status, 24, statusColor, FW_BOLD);
    }
}

void DrawOverlayPanel(HDC hdc, const std::string &title, const std::string &action,
                      const std::string &detail, COLORREF accent)
{
    int left = BOARD_LEFT + 22;
    int right = BOARD_LEFT + CELL_SIZE * 10 - 22;
    int top = BOARD_TOP + 164;
    int bottom = BOARD_TOP + 394;

    FillRoundRectColor(hdc, left + 5, top + 7, right + 5, bottom + 7, 12,
                       RGB(10, 12, 12), RGB(10, 12, 12));
    FillRoundRectColor(hdc, left, top, right, bottom, 12,
                       RGB(31, 35, 36), AdjustColor(accent, -65));
    FillRectColor(hdc, left + 18, top + 16, right - 18, top + 20, accent);

    DrawTextCentered(hdc, left, right, top + 44, title, 34, RGB(248, 244, 225), FW_BOLD);
    DrawTextCentered(hdc, left, right, top + 96, action, 18, accent, FW_BOLD);
    DrawTextCentered(hdc, left, right, top + 138, detail, 17, RGB(170, 178, 158), FW_NORMAL);
}

void DrawStateOverlay(HDC hdc)
{
    if (game.IsGameOver())
    {
        DrawOverlayPanel(hdc, "GAME OVER", "PRESS ANY KEY",
                         "Score " + std::to_string(game.GetScore()) + "   Best " + std::to_string(game.GetHighScore()),
                         RGB(240, 101, 95));
    }
    else if (!game.IsStarted())
    {
        DrawOverlayPanel(hdc, "READY", "PRESS ANY KEY",
                         "Best " + std::to_string(game.GetHighScore()),
                         RGB(249, 214, 124));
    }
    else if (game.IsPaused())
    {
        DrawOverlayPanel(hdc, "PAUSED", "PRESS ANY KEY",
                         "R restarts   Score " + std::to_string(game.GetScore()),
                         RGB(241, 194, 100));
    }
}

bool IsClearFlashActive()
{
    if (!game.IsLineClearPending() || observedClearEventId == 0 || game.GetLastClearedRows().empty())
    {
        return false;
    }

    DWORD elapsed = GetTickCount() - clearFlashStartedAt;
    return elapsed < static_cast<DWORD>(CLEAR_FLASH_DURATION_MS);
}

bool IsClearFlashReadyToFinish()
{
    if (!game.IsLineClearPending() || observedClearEventId == 0)
    {
        return false;
    }

    DWORD elapsed = GetTickCount() - clearFlashStartedAt;
    return elapsed >= static_cast<DWORD>(CLEAR_FLASH_DURATION_MS);
}

bool IsLevelFlashActive()
{
    if (observedLevelUpEventId == 0)
    {
        return false;
    }

    DWORD elapsed = GetTickCount() - levelFlashStartedAt;
    return elapsed < static_cast<DWORD>(LEVEL_FLASH_DURATION_MS);
}

bool IsLockDelayReadyToFinish()
{
    if (!game.IsLockDelayActive())
    {
        return false;
    }

    DWORD elapsed = GetTickCount() - lockDelayStartedAt;
    return elapsed >= static_cast<DWORD>(LOCK_DELAY_MS);
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
            KillTimer(hwnd, TIMER_ID);
            SetTimer(hwnd, FLASH_TIMER_ID, 24, nullptr);
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
            SetTimer(hwnd, LEVEL_TIMER_ID, 45, nullptr);
        }
    }
}

void UpdateLockDelay(HWND hwnd)
{
    if (!game.IsLockDelayActive())
    {
        KillTimer(hwnd, LOCK_TIMER_ID);
        lockDelayTimerRunning = false;
        return;
    }

    if (game.IsPaused())
    {
        KillTimer(hwnd, LOCK_TIMER_ID);
        lockDelayTimerRunning = false;
        return;
    }

    int lockDelayEventId = game.GetLockDelayEventId();
    if (lockDelayEventId != observedLockDelayEventId || !lockDelayTimerRunning)
    {
        observedLockDelayEventId = lockDelayEventId;
        lockDelayStartedAt = GetTickCount();
        KillTimer(hwnd, TIMER_ID);
        SetTimer(hwnd, LOCK_TIMER_ID, 30, nullptr);
        lockDelayTimerRunning = true;
    }
}

void FinishClearFlash(HWND hwnd)
{
    game.FinishLineClear();
    KillTimer(hwnd, FLASH_TIMER_ID);
    SetTimer(hwnd, TIMER_ID, game.GetDropIntervalMs(), nullptr);
}

void FinishLockDelay(HWND hwnd)
{
    game.FinishLockDelay();
    KillTimer(hwnd, LOCK_TIMER_ID);
    lockDelayTimerRunning = false;
    UpdateClearFlash(hwnd);
    UpdateLevelFlash(hwnd);
    if (!game.IsLineClearPending() && !game.IsLockDelayActive() && !game.IsPaused())
    {
        SetTimer(hwnd, TIMER_ID, game.GetDropIntervalMs(), nullptr);
    }
}

bool IsHorizontalMoveKey(WPARAM key)
{
    return key == VK_LEFT || key == VK_RIGHT || key == 'A' || key == 'D';
}

int GetHorizontalDirection(WPARAM key)
{
    if (key == VK_LEFT || key == 'A')
    {
        return -1;
    }
    if (key == VK_RIGHT || key == 'D')
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
    return game.IsStarted() && !game.IsGameOver() && !game.IsPaused() &&
           !game.IsLineClearPending();
}

void StartHorizontalInputTimer(HWND hwnd)
{
    if (!inputTimerRunning)
    {
        SetTimer(hwnd, INPUT_TIMER_ID, 12, nullptr);
        inputTimerRunning = true;
    }
}

void StopHorizontalInputTimer(HWND hwnd)
{
    KillTimer(hwnd, INPUT_TIMER_ID);
    inputTimerRunning = false;
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
    if (now - horizontalHeldStartedAt < static_cast<DWORD>(DAS_DELAY_MS) ||
        now - lastHorizontalRepeatAt < static_cast<DWORD>(ARR_INTERVAL_MS))
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
        int top = BOARD_TOP + row * CELL_SIZE;
        FillRoundRectColor(hdc, BOARD_LEFT + 4, top + 5,
                           BOARD_LEFT + CELL_SIZE * 10 - 4, top + CELL_SIZE - 5,
                           6, flashColor, AdjustColor(flashColor, -55));
        FillRectColor(hdc, BOARD_LEFT + 12, top + 12,
                      BOARD_LEFT + CELL_SIZE * 10 - 12, top + 16,
                      RGB(255, 255, 245));
    }
}

void DrawGame(HDC hdc)
{
    FillVerticalGradient(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                         RGB(18, 22, 21), RGB(35, 38, 32));
    DrawTextLine(hdc, BOARD_LEFT, 20, "TETRIS", 30, RGB(248, 244, 225), FW_BOLD);
    DrawTextLine(hdc, BOARD_LEFT + 118, 28, "native C++", 14, RGB(154, 164, 145), FW_NORMAL);

    FillRoundRectColor(hdc, BOARD_LEFT - 8, BOARD_TOP - 8,
                       BOARD_LEFT + CELL_SIZE * 10 + 8, BOARD_TOP + CELL_SIZE * 20 + 8,
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

    int panelX = BOARD_LEFT + CELL_SIZE * 10 + 34;
    int panelWidth = WINDOW_WIDTH - panelX - 32;
    FillRoundRectColor(hdc, panelX - 12, BOARD_TOP - 8,
                       WINDOW_WIDTH - 24, BOARD_TOP + CELL_SIZE * 20 + 8,
                       10, RGB(24, 28, 28), RGB(58, 68, 58));

    DrawMetricPanel(hdc, panelX, BOARD_TOP + 10, panelWidth, 86,
                    "Score", std::to_string(game.GetScore()));
    DrawTextLine(hdc, panelX + 14, BOARD_TOP + 72,
                 "Best " + std::to_string(game.GetHighScore()), 15, RGB(170, 178, 158), FW_NORMAL);
    DrawMetricPanel(hdc, panelX, BOARD_TOP + 112, (panelWidth - 12) / 2, 78,
                    "Lines", std::to_string(game.GetLinesCleared()));
    DrawMetricPanel(hdc, panelX + (panelWidth + 12) / 2, BOARD_TOP + 112,
                    (panelWidth - 12) / 2, 78, "Level", std::to_string(game.GetLevel()));
    DrawBlockPreview(hdc, panelX, BOARD_TOP + 208, panelWidth, 126,
                     "Hold", game.GetHeldBlockCells(), game.GetHeldBlockId(), !game.CanHold());
    DrawBlockQueuePreview(hdc, panelX, BOARD_TOP + 350, panelWidth, 126,
                          "Next", game.GetUpcomingBlockCells(), game.GetUpcomingBlockIds());
    DrawStatusPanel(hdc, panelX, BOARD_TOP + 486, panelWidth);
    DrawStateOverlay(hdc);
}

void HandleGameKey(HWND hwnd, WPARAM key)
{
    if (key == 'Q')
    {
        PostQuitMessage(0);
        return;
    }

    int input = 0;
    switch (key)
    {
    case VK_LEFT:
        input = 'a';
        break;
    case VK_RIGHT:
        input = 'd';
        break;
    case VK_DOWN:
        input = 's';
        break;
    case VK_UP:
        input = 'w';
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
    case 'A':
    case 'D':
    case 'S':
    case 'W':
    case 'P':
    case 'R':
        input = static_cast<int>(key);
        break;
    default:
        input = (game.IsPaused() || !game.IsStarted() || game.IsGameOver()) ? 'p' : 0;
        break;
    }

    if (input != 0)
    {
        ApplyGameInput(hwnd, input);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hwnd, TIMER_ID, game.GetDropIntervalMs(), nullptr);
        return 0;
    case WM_TIMER:
        if (wParam == FLASH_TIMER_ID)
        {
            if (IsClearFlashReadyToFinish())
            {
                FinishClearFlash(hwnd);
            }
            else if (!game.IsLineClearPending())
            {
                KillTimer(hwnd, FLASH_TIMER_ID);
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        if (wParam == LEVEL_TIMER_ID)
        {
            if (!IsLevelFlashActive())
            {
                KillTimer(hwnd, LEVEL_TIMER_ID);
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        if (wParam == LOCK_TIMER_ID)
        {
            if (IsLockDelayReadyToFinish())
            {
                FinishLockDelay(hwnd);
            }
            else if (!game.IsLockDelayActive())
            {
                KillTimer(hwnd, LOCK_TIMER_ID);
                lockDelayTimerRunning = false;
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        if (wParam == INPUT_TIMER_ID)
        {
            TickHorizontalInput(hwnd);
            return 0;
        }

        game.MoveBlockDown();
        UpdateLockDelay(hwnd);
        UpdateClearFlash(hwnd);
        UpdateLevelFlash(hwnd);
        if (!game.IsLineClearPending() && !game.IsLockDelayActive())
        {
            SetTimer(hwnd, TIMER_ID, game.GetDropIntervalMs(), nullptr);
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    case WM_KEYDOWN:
        if (IsHorizontalMoveKey(wParam))
        {
            HandleHorizontalKeyDown(hwnd, wParam, lParam);
            return 0;
        }
        HandleGameKey(hwnd, wParam);
        return 0;
    case WM_KEYUP:
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
        HBITMAP bitmap = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
        HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(memoryDc, bitmap));

        DrawGame(memoryDc);
        BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, memoryDc, 0, 0, SRCCOPY);

        SelectObject(memoryDc, oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(memoryDc);
        EndPaint(hwnd, &paint);
        return 0;
    }
    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ID);
        KillTimer(hwnd, FLASH_TIMER_ID);
        KillTimer(hwnd, LEVEL_TIMER_ID);
        KillTimer(hwnd, LOCK_TIMER_ID);
        KillTimer(hwnd, INPUT_TIMER_ID);
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCommand)
{
    LoadGameFont();
    game.SetHighScore(LoadHighScore(HIGH_SCORE_FILE));

    const char className[] = "HuiShanTetrisWindow";

    WNDCLASSA windowClass = {};
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    RegisterClassA(&windowClass);

    RECT windowRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
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

    SaveHighScore(HIGH_SCORE_FILE, game.GetHighScore());
    UnloadGameFont();

    return static_cast<int>(message.wParam);
}
