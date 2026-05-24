#include "blocks.h"
#include "game.h"
#include "grid.h"
#include "high_score.h"
#include "leaderboard.h"
#include "settings.h"
#include "sound.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

namespace
{
bool HasCell(const std::vector<Position> &cells, int row, int column)
{
    for (const Position &cell : cells)
    {
        if (cell.row == row && cell.column == column)
        {
            return true;
        }
    }
    return false;
}

bool HasSameCells(const std::vector<Position> &left, const std::vector<Position> &right)
{
    if (left.size() != right.size())
    {
        return false;
    }

    for (const Position &cell : left)
    {
        if (!HasCell(right, cell.row, cell.column))
        {
            return false;
        }
    }
    return true;
}

int MinimumRow(const std::vector<Position> &cells)
{
    int minimum = cells.front().row;
    for (const Position &cell : cells)
    {
        if (cell.row < minimum)
        {
            minimum = cell.row;
        }
    }
    return minimum;
}

void Expect(bool condition, const char *message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << std::endl;
        std::exit(1);
    }
}

void TestGridClearsSingleRow()
{
    Grid grid;
    for (int column = 0; column < 10; column++)
    {
        grid.grid[19][column] = 1;
    }
    grid.grid[18][4] = 7;

    std::vector<int> fullRows = grid.GetFullRows();
    int rowsCleared = grid.ClearFullRows();

    Expect(fullRows.size() == 1 && fullRows[0] == 19, "reports one full row before clearing");
    Expect(rowsCleared == 1, "clears one completed row");
    Expect(grid.grid[19][4] == 7, "moves rows above a cleared row down");
    Expect(grid.grid[18][4] == 0, "clears the original moved row");
}

void TestGridClearsMultipleRows()
{
    Grid grid;
    for (int column = 0; column < 10; column++)
    {
        grid.grid[18][column] = 2;
        grid.grid[19][column] = 3;
    }
    grid.grid[17][0] = 6;

    int rowsCleared = grid.ClearFullRows();

    Expect(rowsCleared == 2, "clears two completed rows");
    Expect(grid.grid[19][0] == 6, "moves rows down by the number of cleared rows");
    Expect(grid.grid[17][0] == 0, "empties moved source rows after multi-clear");
}

void TestBlockRotationCycles()
{
    TBlock block;
    std::vector<Position> initialCells = block.GetCellPositions();

    for (int index = 0; index < 4; index++)
    {
        block.Rotate();
    }

    std::vector<Position> finalCells = block.GetCellPositions();
    Expect(finalCells.size() == initialCells.size(), "rotation preserves cell count");
    for (const Position &cell : initialCells)
    {
        Expect(HasCell(finalCells, cell.row, cell.column), "four rotations return to the original cells");
    }
}

void TestBlockCounterClockwiseRotationCycles()
{
    TBlock block;
    std::vector<Position> initialCells = block.GetCellPositions();

    for (int index = 0; index < 4; index++)
    {
        block.RotateCounterClockwise();
    }

    std::vector<Position> finalCells = block.GetCellPositions();
    Expect(finalCells.size() == initialCells.size(), "counter-clockwise rotation preserves cell count");
    for (const Position &cell : initialCells)
    {
        Expect(HasCell(finalCells, cell.row, cell.column), "four counter-clockwise rotations return to the original cells");
    }
}

void TestRotationKickAllowsIBlockToRotateAtTop()
{
    Game game{IBlock(), OBlock()};
    std::vector<Position> initialCells = game.GetCurrentBlockCells();

    game.Start();
    game.HandleInput('w');
    std::vector<Position> rotatedCells = game.GetCurrentBlockCells();

    Expect(!HasSameCells(initialCells, rotatedCells), "rotation kick allows I block to rotate near the spawn top");
    for (const Position &cell : rotatedCells)
    {
        Expect(cell.row >= 0, "rotation kick keeps rotated cells inside the top of the board");
        Expect(cell.column >= 0 && cell.column < 10, "rotation kick keeps rotated cells inside board columns");
    }
}

void TestSrsKickMovesTBlockUpLeft()
{
    Grid grid;
    grid.grid[7][5] = 7;
    grid.grid[7][4] = 7;

    TBlock block;
    block.Move(5, 1);
    Game game{block, OBlock(), grid};

    game.Start();
    game.HandleInput('w');

    std::vector<Position> rotatedCells = game.GetCurrentBlockCells();
    Expect(HasCell(rotatedCells, 4, 4), "SRS T kick can move a rotated block upward and left");
    Expect(HasCell(rotatedCells, 5, 4), "SRS T kick keeps the kicked block in the expected column");
    Expect(HasCell(rotatedCells, 5, 5), "SRS T kick preserves the rotated shape");
    Expect(HasCell(rotatedCells, 6, 4), "SRS T kick lands in the first valid kicked position");
}

void TestSrsIBlockUsesSpecialKickTable()
{
    Grid grid;
    grid.grid[4][0] = 7;
    grid.grid[4][1] = 7;
    grid.grid[4][2] = 7;
    grid.grid[4][3] = 7;
    grid.grid[4][4] = 7;
    grid.grid[3][2] = 7;
    grid.grid[8][1] = 7;
    grid.grid[8][2] = 7;
    grid.grid[8][3] = 7;

    IBlock block;
    block.Move(5, -3);
    Game game{block, OBlock(), grid};

    game.Start();
    game.HandleInput('w');

    std::vector<Position> rotatedCells = game.GetCurrentBlockCells();
    Expect(HasCell(rotatedCells, 5, 0), "SRS I kick uses the special I offset column");
    Expect(HasCell(rotatedCells, 6, 0), "SRS I kick keeps the vertical I shape");
    Expect(HasCell(rotatedCells, 7, 0), "SRS I kick avoids blocked generic offsets");
    Expect(HasCell(rotatedCells, 8, 0), "SRS I kick lands with the expected downward offset");
}

void TestCounterClockwiseSrsKickMovesTBlockUpRight()
{
    Grid grid;
    grid.grid[7][5] = 7;
    grid.grid[7][6] = 7;

    TBlock block;
    block.Move(5, 1);
    Game game{block, OBlock(), grid};

    game.Start();
    game.HandleInput('z');

    std::vector<Position> rotatedCells = game.GetCurrentBlockCells();
    Expect(HasCell(rotatedCells, 4, 6), "counter-clockwise SRS T kick can move upward and right");
    Expect(HasCell(rotatedCells, 5, 5), "counter-clockwise SRS T kick keeps the left foot in place");
    Expect(HasCell(rotatedCells, 5, 6), "counter-clockwise SRS T kick preserves the rotated center");
    Expect(HasCell(rotatedCells, 6, 6), "counter-clockwise SRS T kick lands in the first valid reverse offset");
}

void TestCounterClockwiseSrsIBlockUsesSpecialKickTable()
{
    Grid grid;
    grid.grid[4][1] = 7;
    grid.grid[4][0] = 7;
    grid.grid[4][3] = 7;

    IBlock block;
    block.Move(5, -3);
    Game game{block, OBlock(), grid};

    game.Start();
    game.HandleInput('z');

    std::vector<Position> rotatedCells = game.GetCurrentBlockCells();
    Expect(HasCell(rotatedCells, 5, 3), "counter-clockwise SRS I kick uses the special reverse I offset column");
    Expect(HasCell(rotatedCells, 6, 3), "counter-clockwise SRS I kick keeps the vertical I shape");
    Expect(HasCell(rotatedCells, 7, 3), "counter-clockwise SRS I kick avoids blocked earlier offsets");
    Expect(HasCell(rotatedCells, 8, 3), "counter-clockwise SRS I kick lands with the expected downward reverse offset");
}

void TestStartGateBlocksMovementUntilInputStarts()
{
    Game game;
    std::vector<Position> initialCells = game.GetCurrentBlockCells();

    Expect(!game.IsStarted(), "new game waits on the start screen");
    Expect(!game.MoveBlockDown(), "automatic drop is blocked before start");
    Expect(HasSameCells(initialCells, game.GetCurrentBlockCells()), "start screen keeps current block still");

    game.HandleInput('a');

    Expect(game.IsStarted(), "first gameplay input starts the game");
    Expect(HasSameCells(initialCells, game.GetCurrentBlockCells()), "first start input does not also move the block");
}

void TestSoftDropScoresOnePoint()
{
    Game game;
    int startingScore = game.GetScore();

    game.Start();
    game.HandleInput('s');

    Expect(game.GetScore() == startingScore + 1, "soft drop scores one point");
}

void TestBlockedSoftDropDoesNotScore()
{
    OBlock block;
    block.Move(18, 0);
    Game game{block, IBlock()};

    game.Start();
    game.HandleInput('s');

    Expect(game.GetScore() == 0, "blocked soft drop does not score");
    Expect(game.GetHighScore() == 0, "blocked soft drop does not update high score");
}

void TestLockDelayAllowsGroundAdjustment()
{
    OBlock block;
    block.Move(18, 0);
    Game game{block, IBlock()};

    game.Start();
    int lockDelayEvent = game.GetLockDelayEventId();
    game.MoveBlockDown();

    Expect(game.IsLockDelayActive(), "blocked downward movement starts lock delay");
    Expect(game.GetCurrentBlockId() == 4, "lock delay keeps the grounded block active");
    Expect(game.GetGrid()[19][4] == 0, "lock delay does not write the block into the grid immediately");

    game.HandleInput('a');

    Expect(game.IsLockDelayActive(), "ground movement keeps lock delay active");
    Expect(game.GetLockDelayEventId() > lockDelayEvent, "ground movement refreshes lock delay timing");

    game.FinishLockDelay();

    Expect(!game.IsLockDelayActive(), "finishing lock delay clears the pending lock state");
    Expect(game.GetGrid()[19][3] == 4, "finishing lock delay writes the adjusted block position");
    Expect(game.GetCurrentBlockId() == 3, "finishing lock delay spawns the next block");
}

void TestPauseStopsLockDelay()
{
    OBlock block;
    block.Move(18, 0);
    Game game{block, IBlock()};

    game.Start();
    game.MoveBlockDown();
    game.HandleInput('p');
    game.FinishLockDelay();

    Expect(game.IsPaused(), "pause stays active during pending lock delay");
    Expect(game.IsLockDelayActive(), "paused game keeps lock delay pending");
    Expect(game.GetGrid()[19][4] == 0, "paused lock delay does not write the block into the grid");

    game.HandleInput(' ');
    game.FinishLockDelay();

    Expect(!game.IsPaused(), "any key resumes before lock delay can finish");
    Expect(!game.IsLockDelayActive(), "lock delay can finish after resume");
    Expect(game.GetGrid()[19][4] == 4, "resumed lock delay writes the block into the grid");
}

void TestHighScoreSurvivesRestart()
{
    Game game;

    game.Start();
    game.HandleInput('s');
    Expect(game.GetScore() == 1, "soft drop creates a score for high score tracking");
    Expect(game.GetHighScore() == 1, "high score updates when score increases");

    game.Restart();

    Expect(game.IsStarted(), "restart immediately starts a new game");
    Expect(game.GetScore() == 0, "restart clears current score");
    Expect(game.GetHighScore() == 1, "restart keeps session high score");
}

void TestHighScoreFilePersistence()
{
    const char *path = "tetris_highscore_test.tmp";
    std::remove(path);

    Expect(LoadHighScore(path) == 0, "missing high score file loads as zero");
    Expect(SaveHighScore(path, 1234), "high score file saves successfully");
    Expect(LoadHighScore(path) == 1234, "saved high score can be loaded");

    {
        std::ofstream file(path, std::ios::trunc);
        file << "not-a-score\n";
    }
    Expect(LoadHighScore(path) == 0, "invalid high score file loads as zero");

    std::remove(path);
}

void TestSoundToggleState()
{
    SetSoundEnabled(false);
    Expect(!IsSoundEnabled(), "sound can be disabled");
    SetSoundEnabled(true);
    Expect(IsSoundEnabled(), "sound can be re-enabled");
}

void TestSoundCueCallsAreSafe()
{
    SetSoundEnabled(false);
    PlayMoveSound();
    PlaySoftDropSound();
    PlayHardDropSound();
    PlayHoldSound();
    PlayRotateSound();
    PlayLineClearSound(1, false);
    PlayLineClearSound(4, false);
    PlayLineClearSound(1, true);
    PlayLevelUpSound();
    PlayPauseSound();
    PlayGameOverSound();
    SetSoundEnabled(true);
    Expect(IsSoundEnabled(), "sound cue safety test restores sound");
}

void TestSettingsDefaultsAndPersistence()
{
    const char *path = "tetris_settings_test.tmp";
    std::remove(path);

    GameSettings defaults = LoadSettings(path);
    Expect(defaults.dasDelayMs == SettingsDefaultDasDelayMs, "missing settings file uses default DAS");
    Expect(defaults.arrIntervalMs == SettingsDefaultArrIntervalMs, "missing settings file uses default ARR");
    Expect(defaults.clearFlashDurationMs == SettingsDefaultClearFlashDurationMs, "missing settings file uses default clear flash");
    Expect(defaults.soundEnabled, "missing settings file enables sound by default");

    GameSettings settings{
        120,
        25,
        110,
        false,
    };
    Expect(SaveSettings(path, settings), "settings file saves successfully");

    GameSettings loaded = LoadSettings(path);
    Expect(loaded.dasDelayMs == 120, "saved DAS loads successfully");
    Expect(loaded.arrIntervalMs == 25, "saved ARR loads successfully");
    Expect(loaded.clearFlashDurationMs == 110, "saved clear flash loads successfully");
    Expect(!loaded.soundEnabled, "saved sound flag loads successfully");

    std::remove(path);
}

void TestSettingsSanitizeInvalidValues()
{
    GameSettings settings{
        -10,
        999,
        20,
        true,
    };

    GameSettings sanitized = SanitizeSettings(settings);
    Expect(sanitized.dasDelayMs == SettingsMinDasDelayMs, "DAS setting clamps to minimum");
    Expect(sanitized.arrIntervalMs == SettingsMaxArrIntervalMs, "ARR setting clamps to maximum");
    Expect(sanitized.clearFlashDurationMs == SettingsMinClearFlashDurationMs, "clear flash setting clamps to minimum");
}

void TestLeaderboardSortsAndLimitsEntries()
{
    std::vector<LeaderboardEntry> entries = {
        {"LOW", 100},
        {"HIGH", 900},
        {"MID", 500},
        {"NEG", -1},
        {"FOUR", 400},
        {"FIVE", 300},
        {"SIX", 200},
    };

    std::vector<LeaderboardEntry> ranked = NormalizeLeaderboard(entries);
    Expect(ranked.size() == LeaderboardMaxEntries, "leaderboard keeps the configured number of entries");
    Expect(ranked[0].score == 900, "leaderboard sorts highest score first");
    Expect(ranked[1].score == 500, "leaderboard keeps descending order");
    Expect(ranked[4].score == 200, "leaderboard trims lower scores after sorting");
}

void TestLeaderboardAddsScoreAndBestScore()
{
    std::vector<LeaderboardEntry> entries = {
        {"AAA", 100},
        {"BBB", 200},
    };

    std::vector<LeaderboardEntry> ranked = AddLeaderboardScore(entries, 350, "NEW");
    Expect(ranked[0].name == "NEW", "added leaderboard score stores the player name");
    Expect(ranked[0].score == 350, "added leaderboard score becomes first when highest");
    Expect(GetBestLeaderboardScore(ranked) == 350, "best leaderboard score reports the top score");
}

void TestLeaderboardQualificationAndNameNormalization()
{
    std::vector<LeaderboardEntry> entries = {
        {"AAA", 500},
        {"BBB", 400},
        {"CCC", 300},
        {"DDD", 200},
        {"EEE", 100},
    };

    Expect(DoesScoreQualifyForLeaderboard(entries, 100), "score equal to the last slot qualifies");
    Expect(!DoesScoreQualifyForLeaderboard(entries, 99), "score below a full leaderboard does not qualify");
    Expect(!DoesScoreQualifyForLeaderboard(entries, 0), "zero score does not qualify");
    Expect(DoesScoreQualifyForLeaderboard({{"AAA", 500}}, 10), "positive score qualifies when leaderboard has room");
    Expect(NormalizeLeaderboardName("") == DefaultLeaderboardName, "empty leaderboard name uses the default");
    Expect(NormalizeLeaderboardName("ABCDEFGHIJKLMNO") == "ABCDEFGHIJKL", "long leaderboard name is trimmed");
}

void TestLeaderboardFilePersistence()
{
    const char *path = "tetris_leaderboard_test.tmp";
    std::remove(path);

    std::vector<LeaderboardEntry> missing = LoadLeaderboard(path);
    Expect(missing.empty(), "missing leaderboard file loads as empty");

    std::vector<LeaderboardEntry> entries = {
        {"AAA", 120},
        {"BBB", 300},
        {"CCC", 220},
    };
    Expect(SaveLeaderboard(path, entries), "leaderboard file saves successfully");

    std::vector<LeaderboardEntry> loaded = LoadLeaderboard(path);
    Expect(loaded.size() == 3, "saved leaderboard entry count loads successfully");
    Expect(loaded[0].name == "BBB" && loaded[0].score == 300, "loaded leaderboard is sorted");
    Expect(loaded[2].name == "AAA" && loaded[2].score == 120, "loaded leaderboard keeps lower entries");

    std::remove(path);
}

void TestPauseStopsAutomaticDrop()
{
    Game game;
    std::vector<Position> initialCells = game.GetCurrentBlockCells();

    game.Start();
    game.HandleInput('p');
    game.MoveBlockDown();

    Expect(game.IsPaused(), "pause input toggles paused state on");
    Expect(HasSameCells(initialCells, game.GetCurrentBlockCells()), "paused game does not auto-drop");

    game.HandleInput(' ');

    Expect(!game.IsPaused(), "any gameplay key resumes from pause");
    Expect(HasSameCells(initialCells, game.GetCurrentBlockCells()), "resume key is consumed instead of moving immediately");

    game.MoveBlockDown();

    Expect(!HasSameCells(initialCells, game.GetCurrentBlockCells()), "unpaused game resumes auto-drop");
}

void TestRestartInputResetsRunningGame()
{
    Game game;

    game.Start();
    game.HandleInput('s');
    Expect(game.GetScore() == 1, "soft drop changes score before restart");

    game.HandleInput('r');

    Expect(game.IsStarted(), "restart input starts a fresh game immediately");
    Expect(game.GetScore() == 0, "restart input clears current score");
    Expect(game.GetLinesCleared() == 0, "restart input clears line count");
}

void TestGhostBlockPreviewIsBelowCurrentBlock()
{
    Game game;
    std::vector<Position> currentCells = game.GetCurrentBlockCells();
    std::vector<Position> ghostCells = game.GetGhostBlockCells();

    Expect(ghostCells.size() == currentCells.size(), "ghost block keeps the same number of cells");
    Expect(MinimumRow(ghostCells) > MinimumRow(currentCells), "ghost block lands below the current block");
    Expect(HasSameCells(currentCells, game.GetCurrentBlockCells()), "ghost preview does not move the current block");
}

void TestNextBlockPreviewHasFourCells()
{
    Game game;
    Expect(game.GetNextBlockCells().size() == 4, "next block preview exposes four cells");
}

void TestUpcomingBlockQueueAdvances()
{
    Game game;
    std::vector<int> startingIds = game.GetUpcomingBlockIds();
    std::vector<std::vector<Position>> startingCells = game.GetUpcomingBlockCells();

    Expect(startingIds.size() == 3, "upcoming queue exposes three block ids");
    Expect(startingCells.size() == 3, "upcoming queue exposes three block previews");
    Expect(startingIds[0] == game.GetNextBlockId(), "upcoming queue starts with the next block");
    for (const std::vector<Position> &cells : startingCells)
    {
        Expect(cells.size() == 4, "each upcoming preview has four cells");
    }

    game.Start();
    game.HandleInput('c');

    Expect(game.GetCurrentBlockId() == startingIds[0], "hold advances the first queued block into play");
    Expect(game.GetNextBlockId() == startingIds[1], "hold promotes the second queued block to next");
    Expect(game.GetUpcomingBlockIds().size() == 3, "upcoming queue refills after advancing");
}

void TestHoldStoresCurrentBlockOncePerDrop()
{
    Game game;
    int startingBlockId = game.GetCurrentBlockId();
    int startingNextBlockId = game.GetNextBlockId();

    game.Start();
    Expect(game.CanHold(), "hold is available before use");
    game.HandleInput('c');

    Expect(game.HasHeldBlock(), "hold stores a block");
    Expect(game.GetHeldBlockId() == startingBlockId, "hold stores the original current block");
    Expect(game.GetCurrentBlockId() == startingNextBlockId, "first hold advances the next block");
    Expect(!game.CanHold(), "hold is locked until the block lands");

    int currentAfterHold = game.GetCurrentBlockId();
    game.HandleInput('c');
    Expect(game.GetCurrentBlockId() == currentAfterHold, "second hold before landing is ignored");

    game.HandleInput(' ');
    Expect(game.CanHold(), "hold unlocks after the active block lands");
}

void TestLineClearFeedbackAndComboReset()
{
    Grid grid;
    for (int column = 0; column < 10; column++)
    {
        if (column < 3 || column > 6)
        {
            grid.grid[19][column] = 7;
        }
    }

    Game game{IBlock(), OBlock(), grid};
    game.Start();
    int startingClearEvent = game.GetClearEventId();
    game.HandleInput(' ');

    const int (&pendingGrid)[20][10] = game.GetGrid();
    bool pendingRowStillFilled = true;
    for (int column = 0; column < 10; column++)
    {
        if (pendingGrid[19][column] == 0)
        {
            pendingRowStillFilled = false;
        }
    }

    Expect(game.IsLineClearPending(), "line clear waits for animation before collapsing rows");
    Expect(pendingRowStillFilled, "line clear keeps the completed row visible during animation");
    Expect(game.GetLastClearLines() == 1, "line clear feedback stores cleared line count");
    Expect(game.GetLastClearScore() == 100, "line clear feedback stores clear score");
    Expect(!game.WasLastClearSpin(), "normal line clear is not marked as a spin");
    Expect(!game.WasLastClearTSpin(), "normal line clear is not marked as T-spin");
    Expect(game.GetLastClearedRows().size() == 1 && game.GetLastClearedRows()[0] == 19, "line clear feedback stores cleared row index");
    Expect(game.GetClearEventId() == startingClearEvent + 1, "line clear increments clear event id");
    Expect(game.GetCombo() == 1, "first clear starts combo at one");
    Expect(game.GetLinesCleared() == 1, "game tracks total cleared lines after lock");

    game.FinishLineClear();

    const int (&clearedGrid)[20][10] = game.GetGrid();
    bool clearedRowIsEmpty = true;
    for (int column = 0; column < 10; column++)
    {
        if (clearedGrid[19][column] != 0)
        {
            clearedRowIsEmpty = false;
        }
    }

    Expect(!game.IsLineClearPending(), "finishing line clear resumes normal play");
    Expect(clearedRowIsEmpty, "finishing line clear collapses the completed row");

    game.HandleInput(' ');

    Expect(game.GetLastClearLines() == 0, "non-clear lock resets clear line feedback");
    Expect(game.GetLastClearScore() == 0, "non-clear lock resets clear score feedback");
    Expect(game.GetCombo() == 0, "non-clear lock resets combo");
}

void TestTSpinSingleFeedbackAndScore()
{
    Grid grid;
    for (int column = 0; column < 10; column++)
    {
        if (column < 3 || column > 5)
        {
            grid.grid[19][column] = 7;
        }
    }
    grid.grid[18][3] = 7;
    grid.grid[18][5] = 7;

    TBlock block;
    block.Rotate();
    block.Rotate();
    block.Rotate();
    block.Move(18, 0);

    Game game{block, OBlock(), grid};
    game.Start();
    game.HandleInput('w');
    game.HandleInput(' ');

    Expect(game.IsLineClearPending(), "T-spin single waits for clear animation");
    Expect(game.WasLastClearSpin(), "T-spin single is marked as a spin");
    Expect(game.WasLastClearTSpin(), "T-spin single is marked as T-spin");
    Expect(game.GetLastClearSpinBlockId() == 6, "T-spin single stores the T block id");
    Expect(game.GetLastClearLines() == 1, "T-spin single clears one line");
    Expect(game.GetLastClearScore() == 800, "T-spin single uses T-spin score");
    Expect(game.GetScore() >= 800, "T-spin single adds the bonus score");
}

void TestStyleSpinSingleFeedbackAndScore()
{
    Grid grid;
    for (int column = 0; column < 10; column++)
    {
        if (column < 3 || column > 5)
        {
            grid.grid[19][column] = 7;
        }
    }

    LBlock block;
    block.Rotate();
    block.Rotate();
    block.Rotate();
    block.Move(18, 0);

    Game game{block, OBlock(), grid};
    game.Start();
    game.HandleInput('w');
    game.HandleInput(' ');

    Expect(game.IsLineClearPending(), "L-spin single waits for clear animation");
    Expect(game.WasLastClearSpin(), "L-spin single is marked as a spin");
    Expect(!game.WasLastClearTSpin(), "L-spin single is not marked as T-spin");
    Expect(game.GetLastClearSpinBlockId() == 1, "L-spin single stores the L block id");
    Expect(game.GetLastClearLines() == 1, "L-spin single clears one line");
    Expect(game.GetLastClearScore() == 400, "L-spin single uses style-spin score");
}

void TestLevelUpEventOnLineThreshold()
{
    Grid grid;
    for (int column = 0; column < 10; column++)
    {
        if (column < 3 || column > 6)
        {
            grid.grid[19][column] = 7;
        }
    }

    Game game{IBlock(), OBlock(), grid, 9};
    game.Start();
    int startingLevelEvent = game.GetLevelUpEventId();
    game.HandleInput(' ');

    Expect(game.GetLinesCleared() == 10, "line clear can cross a level threshold");
    Expect(game.GetLevel() == 2, "level increases after ten cleared lines");
    Expect(game.GetLastLevelReached() == 2, "level-up feedback stores the reached level");
    Expect(game.GetLevelUpEventId() == startingLevelEvent + 1, "level-up feedback increments event id");
}

void TestLevelAndDropSpeedProgression()
{
    Expect(Game::CalculateLineClearScore(0) == 0, "zero lines score zero line-clear points");
    Expect(Game::CalculateLineClearScore(4) == 800, "four-line clear uses Tetris score");
    Expect(Game::CalculateTSpinScore(1) == 800, "T-spin single uses its bonus score");
    Expect(Game::CalculateTSpinScore(3) == 1600, "T-spin triple uses its bonus score");
    Expect(Game::CalculateStyleSpinScore(1) == 400, "style-spin single uses its bonus score");
    Expect(Game::CalculateStyleSpinScore(4) == 1200, "style-spin four-line clear uses its bonus score");
    Expect(Game::CalculateLevel(0) == 1, "level starts at one");
    Expect(Game::CalculateLevel(9) == 1, "level stays one before ten lines");
    Expect(Game::CalculateLevel(10) == 2, "level increases every ten lines");
    Expect(Game::CalculateDropIntervalMs(1) == 350, "level one uses the base drop interval");
    Expect(Game::CalculateDropIntervalMs(3) == 300, "drop interval gets faster as level rises");
    Expect(Game::CalculateDropIntervalMs(99) == 100, "drop interval has a minimum speed cap");
}
}

int main()
{
    TestGridClearsSingleRow();
    TestGridClearsMultipleRows();
    TestBlockRotationCycles();
    TestBlockCounterClockwiseRotationCycles();
    TestRotationKickAllowsIBlockToRotateAtTop();
    TestSrsKickMovesTBlockUpLeft();
    TestSrsIBlockUsesSpecialKickTable();
    TestCounterClockwiseSrsKickMovesTBlockUpRight();
    TestCounterClockwiseSrsIBlockUsesSpecialKickTable();
    TestStartGateBlocksMovementUntilInputStarts();
    TestSoftDropScoresOnePoint();
    TestBlockedSoftDropDoesNotScore();
    TestLockDelayAllowsGroundAdjustment();
    TestPauseStopsLockDelay();
    TestHighScoreSurvivesRestart();
    TestHighScoreFilePersistence();
    TestSoundToggleState();
    TestSoundCueCallsAreSafe();
    TestSettingsDefaultsAndPersistence();
    TestSettingsSanitizeInvalidValues();
    TestLeaderboardSortsAndLimitsEntries();
    TestLeaderboardAddsScoreAndBestScore();
    TestLeaderboardQualificationAndNameNormalization();
    TestLeaderboardFilePersistence();
    TestPauseStopsAutomaticDrop();
    TestRestartInputResetsRunningGame();
    TestGhostBlockPreviewIsBelowCurrentBlock();
    TestNextBlockPreviewHasFourCells();
    TestUpcomingBlockQueueAdvances();
    TestHoldStoresCurrentBlockOncePerDrop();
    TestLineClearFeedbackAndComboReset();
    TestTSpinSingleFeedbackAndScore();
    TestStyleSpinSingleFeedbackAndScore();
    TestLevelUpEventOnLineThreshold();
    TestLevelAndDropSpeedProgression();

    std::cout << "All core tests passed." << std::endl;
    return 0;
}
