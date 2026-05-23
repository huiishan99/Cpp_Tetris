#pragma once
#include <vector>
#include <map>
#include "position.h"

class Block
{
public:
    Block();
    void Move(int row, int columns);
    std::vector<Position> GetCellPositions() const;
    void Rotate();
    void RotateCounterClockwise();
    void UndoRotation();
    int GetRotationState() const;
    int id;
    std::map<int, std::vector<Position>> cells;

private:
    int rotationState;
    int rowOffset;
    int columnOffset;
};
