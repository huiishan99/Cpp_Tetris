#include "game.h"
#include "sound.h"

namespace
{
const int LinesPerLevel = 10;
const int BaseDropIntervalMs = 350;
const int DropIntervalStepMs = 25;
const int MinimumDropIntervalMs = 100;
const int UpcomingPreviewCount = 3;

bool HasNeighborCell(const std::vector<Position> &cells, int row, int column)
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

bool FindTSpinCenter(const std::vector<Position> &cells, int &centerRow, int &centerColumn)
{
    for (const Position &cell : cells)
    {
        int neighbors = 0;
        if (HasNeighborCell(cells, cell.row - 1, cell.column))
        {
            neighbors++;
        }
        if (HasNeighborCell(cells, cell.row + 1, cell.column))
        {
            neighbors++;
        }
        if (HasNeighborCell(cells, cell.row, cell.column - 1))
        {
            neighbors++;
        }
        if (HasNeighborCell(cells, cell.row, cell.column + 1))
        {
            neighbors++;
        }

        if (neighbors == 3)
        {
            centerRow = cell.row;
            centerColumn = cell.column;
            return true;
        }
    }
    return false;
}

std::vector<Position> GetSrsKickOffsets(int blockId, int fromRotationState, int toRotationState)
{
    if (blockId == 4)
    {
        return {Position(0, 0)};
    }

    if (blockId == 3)
    {
        if (fromRotationState == 0 && toRotationState == 1)
        {
            return {Position(0, 0), Position(0, -2), Position(0, 1), Position(1, -2), Position(-2, 1)};
        }
        if (fromRotationState == 1 && toRotationState == 0)
        {
            return {Position(0, 0), Position(0, 2), Position(0, -1), Position(-1, 2), Position(2, -1)};
        }
        if (fromRotationState == 1 && toRotationState == 2)
        {
            return {Position(0, 0), Position(0, -1), Position(0, 2), Position(-2, -1), Position(1, 2)};
        }
        if (fromRotationState == 2 && toRotationState == 1)
        {
            return {Position(0, 0), Position(0, 1), Position(0, -2), Position(2, 1), Position(-1, -2)};
        }
        if (fromRotationState == 2 && toRotationState == 3)
        {
            return {Position(0, 0), Position(0, 2), Position(0, -1), Position(-1, 2), Position(2, -1)};
        }
        if (fromRotationState == 3 && toRotationState == 2)
        {
            return {Position(0, 0), Position(0, -2), Position(0, 1), Position(1, -2), Position(-2, 1)};
        }
        if (fromRotationState == 3 && toRotationState == 0)
        {
            return {Position(0, 0), Position(0, 1), Position(0, -2), Position(2, 1), Position(-1, -2)};
        }
        if (fromRotationState == 0 && toRotationState == 3)
        {
            return {Position(0, 0), Position(0, -1), Position(0, 2), Position(-2, -1), Position(1, 2)};
        }
    }

    if (fromRotationState == 0 && toRotationState == 1)
    {
        return {Position(0, 0), Position(0, -1), Position(-1, -1), Position(2, 0), Position(2, -1)};
    }
    if (fromRotationState == 1 && toRotationState == 0)
    {
        return {Position(0, 0), Position(0, 1), Position(1, 1), Position(-2, 0), Position(-2, 1)};
    }
    if (fromRotationState == 1 && toRotationState == 2)
    {
        return {Position(0, 0), Position(0, 1), Position(1, 1), Position(-2, 0), Position(-2, 1)};
    }
    if (fromRotationState == 2 && toRotationState == 1)
    {
        return {Position(0, 0), Position(0, -1), Position(-1, -1), Position(2, 0), Position(2, -1)};
    }
    if (fromRotationState == 2 && toRotationState == 3)
    {
        return {Position(0, 0), Position(0, 1), Position(-1, 1), Position(2, 0), Position(2, 1)};
    }
    if (fromRotationState == 3 && toRotationState == 2)
    {
        return {Position(0, 0), Position(0, -1), Position(1, -1), Position(-2, 0), Position(-2, -1)};
    }
    if (fromRotationState == 3 && toRotationState == 0)
    {
        return {Position(0, 0), Position(0, -1), Position(1, -1), Position(-2, 0), Position(-2, -1)};
    }
    if (fromRotationState == 0 && toRotationState == 3)
    {
        return {Position(0, 0), Position(0, 1), Position(-1, 1), Position(2, 0), Position(2, 1)};
    }

    return {Position(0, 0)};
}
}

Game::Game()
{
    grid = Grid();
    randomGenerator = std::mt19937(std::random_device{}());
    blocks = GetAllBlocks();
    upcomingBlocks.clear();
    currentBlock = GetRandomBlock();
    nextBlock = GetRandomBlock();
    FillUpcomingBlocks();
    gameOver = false;
    started = false;
    paused = false;
    lineClearPending = false;
    lockDelayActive = false;
    hasHeldBlock = false;
    holdUsed = false;
    lastSuccessfulActionWasRotate = false;
    lastClearWasTSpin = false;
    lastClearSpinBlockId = 0;
    score = 0;
    highScore = 0;
    linesCleared = 0;
    lastClearLines = 0;
    lastClearScore = 0;
    clearEventId = 0;
    combo = 0;
    levelUpEventId = 0;
    lastLevelReached = 1;
    lockDelayEventId = 0;
}

Game::Game(const Block &startingBlock, const Block &upcomingBlock)
{
    grid = Grid();
    randomGenerator = std::mt19937(0);
    blocks = GetAllBlocks();
    currentBlock = startingBlock;
    nextBlock = upcomingBlock;
    FillUpcomingBlocks();
    gameOver = false;
    started = false;
    paused = false;
    lineClearPending = false;
    lockDelayActive = false;
    hasHeldBlock = false;
    holdUsed = false;
    lastSuccessfulActionWasRotate = false;
    lastClearWasTSpin = false;
    lastClearSpinBlockId = 0;
    score = 0;
    highScore = 0;
    linesCleared = 0;
    lastClearLines = 0;
    lastClearScore = 0;
    clearEventId = 0;
    combo = 0;
    levelUpEventId = 0;
    lastLevelReached = 1;
    lockDelayEventId = 0;
}

Game::Game(const Block &startingBlock, const Block &upcomingBlock, const Grid &initialGrid)
{
    grid = initialGrid;
    randomGenerator = std::mt19937(0);
    blocks = GetAllBlocks();
    currentBlock = startingBlock;
    nextBlock = upcomingBlock;
    FillUpcomingBlocks();
    gameOver = false;
    started = false;
    paused = false;
    lineClearPending = false;
    lockDelayActive = false;
    hasHeldBlock = false;
    holdUsed = false;
    lastSuccessfulActionWasRotate = false;
    lastClearWasTSpin = false;
    lastClearSpinBlockId = 0;
    score = 0;
    highScore = 0;
    linesCleared = 0;
    lastClearLines = 0;
    lastClearScore = 0;
    clearEventId = 0;
    combo = 0;
    levelUpEventId = 0;
    lastLevelReached = CalculateLevel(linesCleared);
    lockDelayEventId = 0;
}

Game::Game(const Block &startingBlock, const Block &upcomingBlock, const Grid &initialGrid, int initialLinesCleared)
{
    if (initialLinesCleared < 0)
    {
        initialLinesCleared = 0;
    }

    grid = initialGrid;
    randomGenerator = std::mt19937(0);
    blocks = GetAllBlocks();
    currentBlock = startingBlock;
    nextBlock = upcomingBlock;
    FillUpcomingBlocks();
    gameOver = false;
    started = false;
    paused = false;
    lineClearPending = false;
    lockDelayActive = false;
    hasHeldBlock = false;
    holdUsed = false;
    lastSuccessfulActionWasRotate = false;
    lastClearWasTSpin = false;
    lastClearSpinBlockId = 0;
    score = 0;
    highScore = 0;
    linesCleared = initialLinesCleared;
    lastClearLines = 0;
    lastClearScore = 0;
    clearEventId = 0;
    combo = 0;
    levelUpEventId = 0;
    lastLevelReached = CalculateLevel(linesCleared);
    lockDelayEventId = 0;
}

Block Game::GetRandomBlock()
{
    if (blocks.empty())
    {
        blocks = GetAllBlocks();
    }
    std::uniform_int_distribution<int> distribution(0, static_cast<int>(blocks.size()) - 1);
    int randomIndex = distribution(randomGenerator);
    Block block = blocks[randomIndex];
    blocks.erase(blocks.begin() + randomIndex);
    return block;
}

std::vector<Block> Game ::GetAllBlocks()
{
    return {IBlock(), JBlock(), LBlock(), OBlock(), SBlock(), TBlock(), ZBlock()};
}

const int (&Game::GetGrid() const)[20][10]
{
    return grid.grid;
}

std::vector<Position> Game::GetCurrentBlockCells() const
{
    if (lineClearPending)
    {
        return {};
    }
    return currentBlock.GetCellPositions();
}

std::vector<Position> Game::GetGhostBlockCells() const
{
    if (lineClearPending)
    {
        return {};
    }

    Block ghostBlock = currentBlock;
    while (true)
    {
        ghostBlock.Move(1, 0);
        if (IsBlockOutside(ghostBlock) || BlockFits(ghostBlock) == false)
        {
            ghostBlock.Move(-1, 0);
            return ghostBlock.GetCellPositions();
        }
    }
}

std::vector<Position> Game::GetNextBlockCells() const
{
    return nextBlock.GetCellPositions();
}

std::vector<std::vector<Position>> Game::GetUpcomingBlockCells() const
{
    std::vector<std::vector<Position>> cells;
    cells.push_back(nextBlock.GetCellPositions());
    for (const Block &block : upcomingBlocks)
    {
        cells.push_back(block.GetCellPositions());
    }
    return cells;
}

std::vector<Position> Game::GetHeldBlockCells() const
{
    if (!hasHeldBlock)
    {
        return {};
    }
    return heldBlock.GetCellPositions();
}

int Game::GetCurrentBlockId() const
{
    return currentBlock.id;
}

int Game::GetNextBlockId() const
{
    return nextBlock.id;
}

std::vector<int> Game::GetUpcomingBlockIds() const
{
    std::vector<int> ids;
    ids.push_back(nextBlock.id);
    for (const Block &block : upcomingBlocks)
    {
        ids.push_back(block.id);
    }
    return ids;
}

int Game::GetHeldBlockId() const
{
    if (!hasHeldBlock)
    {
        return 0;
    }
    return heldBlock.id;
}

int Game::GetScore() const
{
    return score;
}

int Game::GetHighScore() const
{
    return highScore;
}

void Game::SetHighScore(int value)
{
    if (value < score)
    {
        value = score;
    }
    highScore = value;
}

int Game::GetLinesCleared() const
{
    return linesCleared;
}

int Game::GetLastClearLines() const
{
    return lastClearLines;
}

int Game::GetLastClearScore() const
{
    return lastClearScore;
}

bool Game::WasLastClearSpin() const
{
    return lastClearSpinBlockId != 0;
}

bool Game::WasLastClearTSpin() const
{
    return lastClearWasTSpin;
}

int Game::GetLastClearSpinBlockId() const
{
    return lastClearSpinBlockId;
}

const std::vector<int> &Game::GetLastClearedRows() const
{
    return lastClearedRows;
}

int Game::GetClearEventId() const
{
    return clearEventId;
}

int Game::GetCombo() const
{
    return combo;
}

int Game::GetLevelUpEventId() const
{
    return levelUpEventId;
}

int Game::GetLastLevelReached() const
{
    return lastLevelReached;
}

int Game::GetLevel() const
{
    return CalculateLevel(linesCleared);
}

int Game::GetDropIntervalMs() const
{
    return CalculateDropIntervalMs(GetLevel());
}

bool Game::IsStarted() const
{
    return started;
}

bool Game::IsGameOver() const
{
    return gameOver;
}

bool Game::IsPaused() const
{
    return paused;
}

bool Game::IsLineClearPending() const
{
    return lineClearPending;
}

bool Game::IsLockDelayActive() const
{
    return lockDelayActive;
}

int Game::GetLockDelayEventId() const
{
    return lockDelayEventId;
}

bool Game::HasHeldBlock() const
{
    return hasHeldBlock;
}

bool Game::CanHold() const
{
    return started && !gameOver && !paused && !lineClearPending && !holdUsed;
}

int Game::CalculateLineClearScore(int completedLines)
{
    switch (completedLines)
    {
    case 1:
        return 100;
    case 2:
        return 300;
    case 3:
        return 500;
    case 4:
        return 800;
    default:
        return 0;
    }
}

int Game::CalculateTSpinScore(int completedLines)
{
    switch (completedLines)
    {
    case 0:
        return 400;
    case 1:
        return 800;
    case 2:
        return 1200;
    case 3:
        return 1600;
    default:
        return 0;
    }
}

int Game::CalculateStyleSpinScore(int completedLines)
{
    switch (completedLines)
    {
    case 1:
        return 400;
    case 2:
        return 700;
    case 3:
        return 1000;
    case 4:
        return 1200;
    default:
        return 0;
    }
}

int Game::CalculateLevel(int completedLines)
{
    if (completedLines <= 0)
    {
        return 1;
    }
    return completedLines / LinesPerLevel + 1;
}

int Game::CalculateDropIntervalMs(int level)
{
    if (level < 1)
    {
        level = 1;
    }

    int interval = BaseDropIntervalMs - (level - 1) * DropIntervalStepMs;
    if (interval < MinimumDropIntervalMs)
    {
        return MinimumDropIntervalMs;
    }
    return interval;
}

void Game::HandleInput(int key)
{
    if (key == 'r' || key == 'R')
    {
        Restart();
        return;
    }

    if (gameOver && key != 0)
    {
        Restart();
        return;
    }

    if (!started && key != 0)
    {
        Start();
        return;
    }

    if (lineClearPending)
    {
        return;
    }

    if (key == 'p' || key == 'P')
    {
        TogglePause();
        return;
    }

    if (paused)
    {
        TogglePause();
        return;
    }

    switch (key)
    {
    case 'c':
    case 'C':
        HoldBlock();
        break;
    case 'a':
    case 'A':
        MoveBlockLeft();
        break;
    case 'd':
    case 'D':
        MoveBlockRight();
        break;
    case 's':
    case 'S':
        if (MoveBlockDown())
        {
            PlaySoftDropSound();
            UpdateScore(0, 1);
        }
        break;
    case 'w':
    case 'W':
    case 'x':
    case 'X':
        RotateBlock();
        break;
    case 'z':
    case 'Z':
        RotateBlockCounterClockwise();
        break;
    case ' ':
        DropBlock();
        break;
    }
}

void Game::Start()
{
    if (!gameOver)
    {
        started = true;
        paused = false;
    }
}

void Game::Restart()
{
    Reset();
    Start();
}

void Game::MoveBlockLeft()
{
    if (started && !gameOver && !paused && !lineClearPending)
    {
        currentBlock.Move(0, -1);
        if (IsBlockOutside() || BlockFits() == false)
        {
            currentBlock.Move(0, 1);
        }
        else
        {
            lastSuccessfulActionWasRotate = false;
            RefreshLockDelay();
            PlayMoveSound();
        }
    }
}

void Game::MoveBlockRight()
{
    if (started && !gameOver && !paused && !lineClearPending)
    {
        currentBlock.Move(0, 1);
        if (IsBlockOutside() || BlockFits() == false)
        {
            currentBlock.Move(0, -1);
        }
        else
        {
            lastSuccessfulActionWasRotate = false;
            RefreshLockDelay();
            PlayMoveSound();
        }
    }
}

bool Game::MoveBlockDown()
{
    if (started && !gameOver && !paused && !lineClearPending)
    {
        currentBlock.Move(1, 0);
        if (IsBlockOutside() || BlockFits() == false)
        {
            currentBlock.Move(-1, 0);
            StartLockDelay();
            return false;
        }
        CancelLockDelay();
        return true;
    }
    return false;
}

void Game::DropBlock()
{
    if (started && !gameOver && !paused && !lineClearPending)
    {
        CancelLockDelay();
        int rowsDropped = 0;
        while (true)
        {
            currentBlock.Move(1, 0);
            if (IsBlockOutside() || BlockFits() == false)
            {
                currentBlock.Move(-1, 0);
                if (rowsDropped > 0)
                {
                    PlayHardDropSound();
                }
                LockBlock();
                break;
            }
            rowsDropped++;
        }
        UpdateScore(0, rowsDropped * 2);
    }
}

void Game::HoldBlock()
{
    if (!CanHold())
    {
        return;
    }

    Block previousCurrent = CreateBlockById(currentBlock.id);
    if (!hasHeldBlock)
    {
        heldBlock = previousCurrent;
        hasHeldBlock = true;
        currentBlock = AdvanceNextBlock();
    }
    else
    {
        Block previousHeld = heldBlock;
        heldBlock = previousCurrent;
        currentBlock = previousHeld;
    }

    holdUsed = true;
    lastSuccessfulActionWasRotate = false;
    CancelLockDelay();
    if (BlockFits() == false)
    {
        gameOver = true;
        paused = false;
        PlayGameOverSound();
        return;
    }
    PlayHoldSound();
}

bool Game::IsBlockOutside() const
{
    return IsBlockOutside(currentBlock);
}

bool Game::IsBlockOutside(const Block &block) const
{
    std::vector<Position> tiles = block.GetCellPositions();
    for (Position item : tiles)
    {
        if (grid.IsCellOutside(item.row, item.column))
        {
            return true;
        }
    }
    return false;
}

void Game::RotateBlock()
{
    if (started && !gameOver && !paused && !lineClearPending)
    {
        int fromRotationState = currentBlock.GetRotationState();
        currentBlock.Rotate();
        int toRotationState = currentBlock.GetRotationState();
        if (!TryWallKick(fromRotationState, toRotationState))
        {
            currentBlock.UndoRotation();
        }
        else
        {
            lastSuccessfulActionWasRotate = true;
            RefreshLockDelay();
            PlayRotateSound();
        }
    }
}

void Game::RotateBlockCounterClockwise()
{
    if (started && !gameOver && !paused && !lineClearPending)
    {
        int fromRotationState = currentBlock.GetRotationState();
        currentBlock.RotateCounterClockwise();
        int toRotationState = currentBlock.GetRotationState();
        if (!TryWallKick(fromRotationState, toRotationState))
        {
            currentBlock.Rotate();
        }
        else
        {
            lastSuccessfulActionWasRotate = true;
            RefreshLockDelay();
            PlayRotateSound();
        }
    }
}

bool Game::TryWallKick(int fromRotationState, int toRotationState)
{
    std::vector<Position> kickOffsets = GetSrsKickOffsets(currentBlock.id, fromRotationState, toRotationState);
    for (Position offset : kickOffsets)
    {
        currentBlock.Move(offset.row, offset.column);
        if (!IsBlockOutside() && BlockFits())
        {
            return true;
        }
        currentBlock.Move(-offset.row, -offset.column);
    }
    return false;
}

bool Game::IsCurrentTSpin() const
{
    if (currentBlock.id != 6 || !lastSuccessfulActionWasRotate)
    {
        return false;
    }

    std::vector<Position> tiles = currentBlock.GetCellPositions();
    int centerRow = 0;
    int centerColumn = 0;
    if (!FindTSpinCenter(tiles, centerRow, centerColumn))
    {
        return false;
    }

    const Position corners[] = {
        Position(centerRow - 1, centerColumn - 1),
        Position(centerRow - 1, centerColumn + 1),
        Position(centerRow + 1, centerColumn - 1),
        Position(centerRow + 1, centerColumn + 1),
    };

    int blockedCorners = 0;
    for (Position corner : corners)
    {
        if (grid.IsCellOutside(corner.row, corner.column) || !grid.IsCellEmpty(corner.row, corner.column))
        {
            blockedCorners++;
        }
    }
    return blockedCorners >= 3;
}

bool Game::IsCurrentStyleSpin() const
{
    if (!lastSuccessfulActionWasRotate || currentBlock.id == 0 || currentBlock.id == 4 || currentBlock.id == 6)
    {
        return false;
    }

    return !CanMoveBlock(currentBlock, 0, -1) &&
           !CanMoveBlock(currentBlock, 0, 1) &&
           !CanMoveBlock(currentBlock, 1, 0);
}

bool Game::CanMoveBlock(const Block &block, int rowOffset, int columnOffset) const
{
    Block movedBlock = block;
    movedBlock.Move(rowOffset, columnOffset);
    return !IsBlockOutside(movedBlock) && BlockFits(movedBlock);
}

void Game::StartLockDelay()
{
    if (!lockDelayActive)
    {
        lockDelayActive = true;
        lockDelayEventId++;
    }
}

void Game::RefreshLockDelay()
{
    if (!lockDelayActive)
    {
        return;
    }

    if (CanMoveBlock(currentBlock, 1, 0))
    {
        CancelLockDelay();
        return;
    }

    lockDelayEventId++;
}

void Game::CancelLockDelay()
{
    lockDelayActive = false;
}

void Game::LockBlock()
{
    CancelLockDelay();
    bool tSpin = IsCurrentTSpin();
    bool styleSpin = !tSpin && IsCurrentStyleSpin();
    int spinBlockId = tSpin || styleSpin ? currentBlock.id : 0;
    std::vector<Position> tiles = currentBlock.GetCellPositions();
    for (Position item : tiles)
    {
        grid.grid[item.row][item.column] = currentBlock.id;
    }
    holdUsed = false;
    lastSuccessfulActionWasRotate = false;

    std::vector<int> fullRows = grid.GetFullRows();
    if (!fullRows.empty())
    {
        lastClearWasTSpin = tSpin;
        lastClearSpinBlockId = spinBlockId;
        StartLineClear(fullRows);
        return;
    }

    combo = 0;
    lastClearLines = 0;
    lastClearScore = 0;
    lastClearWasTSpin = false;
    lastClearSpinBlockId = 0;
    lastClearedRows.clear();
    SpawnNextBlock();
}

void Game::StartLineClear(const std::vector<int> &fullRows)
{
    int rowsCleared = static_cast<int>(fullRows.size());
    lineClearPending = true;
    combo++;
    lastClearLines = rowsCleared;
    if (lastClearWasTSpin)
    {
        lastClearScore = CalculateTSpinScore(rowsCleared);
    }
    else if (lastClearSpinBlockId != 0)
    {
        lastClearScore = CalculateStyleSpinScore(rowsCleared);
    }
    else
    {
        lastClearScore = CalculateLineClearScore(rowsCleared);
    }
    lastClearedRows = fullRows;
    clearEventId++;
    PlayLineClearSound(rowsCleared, lastClearSpinBlockId != 0);
    UpdateScore(rowsCleared, 0, lastClearSpinBlockId);
}

void Game::FinishLockDelay()
{
    if (!lockDelayActive || paused)
    {
        return;
    }

    if (CanMoveBlock(currentBlock, 1, 0))
    {
        CancelLockDelay();
        return;
    }

    LockBlock();
}

void Game::FinishLineClear()
{
    if (!lineClearPending)
    {
        return;
    }

    grid.ClearFullRows();
    lineClearPending = false;
    SpawnNextBlock();
}

void Game::SpawnNextBlock()
{
    currentBlock = AdvanceNextBlock();
    if (BlockFits() == false)
    {
        gameOver = true;
        started = true;
        paused = false;
        PlayGameOverSound();
    }
}

Block Game::AdvanceNextBlock()
{
    Block advancedBlock = nextBlock;
    if (upcomingBlocks.empty())
    {
        FillUpcomingBlocks();
    }

    if (!upcomingBlocks.empty())
    {
        nextBlock = upcomingBlocks.front();
        upcomingBlocks.erase(upcomingBlocks.begin());
    }
    else
    {
        nextBlock = GetRandomBlock();
    }

    FillUpcomingBlocks();
    return advancedBlock;
}

void Game::FillUpcomingBlocks()
{
    while (static_cast<int>(upcomingBlocks.size()) < UpcomingPreviewCount - 1)
    {
        upcomingBlocks.push_back(GetRandomBlock());
    }
}

bool Game::BlockFits() const
{
    return BlockFits(currentBlock);
}

bool Game::BlockFits(const Block &block) const
{
    std::vector<Position> tiles = block.GetCellPositions();
    for (Position item : tiles)
    {
        if (grid.IsCellEmpty(item.row, item.column) == false)
        {
            return false;
        }
    }
    return true;
}

Block Game::CreateBlockById(int blockId) const
{
    switch (blockId)
    {
    case 1:
        return LBlock();
    case 2:
        return JBlock();
    case 3:
        return IBlock();
    case 4:
        return OBlock();
    case 5:
        return SBlock();
    case 6:
        return TBlock();
    case 7:
        return ZBlock();
    default:
        return Block();
    }
}

void Game::Reset()
{
    grid.Initialize();
    blocks = GetAllBlocks();
    upcomingBlocks.clear();
    currentBlock = GetRandomBlock();
    nextBlock = GetRandomBlock();
    FillUpcomingBlocks();
    gameOver = false;
    started = false;
    paused = false;
    lineClearPending = false;
    lockDelayActive = false;
    hasHeldBlock = false;
    holdUsed = false;
    lastSuccessfulActionWasRotate = false;
    lastClearWasTSpin = false;
    lastClearSpinBlockId = 0;
    score = 0;
    linesCleared = 0;
    lastClearLines = 0;
    lastClearScore = 0;
    lastClearedRows.clear();
    clearEventId = 0;
    combo = 0;
    levelUpEventId = 0;
    lastLevelReached = 1;
    lockDelayEventId = 0;
}

void Game::UpdateScore(int linesCompleted, int moveDownPoints, int spinBlockId)
{
    int previousLevel = GetLevel();
    if (spinBlockId == 6)
    {
        score += CalculateTSpinScore(linesCompleted);
    }
    else if (spinBlockId != 0)
    {
        score += CalculateStyleSpinScore(linesCompleted);
    }
    else
    {
        score += CalculateLineClearScore(linesCompleted);
    }
    linesCleared += linesCompleted;
    score += moveDownPoints;
    int currentLevel = GetLevel();
    if (currentLevel > previousLevel)
    {
        lastLevelReached = currentLevel;
        levelUpEventId++;
        PlayLevelUpSound();
    }
    UpdateHighScore();
}

void Game::UpdateHighScore()
{
    if (score > highScore)
    {
        highScore = score;
    }
}

void Game::TogglePause()
{
    if (!gameOver)
    {
        paused = !paused;
        PlayPauseSound();
    }
}
