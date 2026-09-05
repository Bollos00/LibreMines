/*****************************************************************************
 * LibreMines                                                                *
 * Copyright (C) 2020-2025  Bruno Bollos Correa                              *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************
 */

#include "gameboardrandomgenerator.h"
#include <QRandomGenerator>

std::vector<std::vector<LibreMinesGameEngine::CellGameEngine>> GameBoardRandomGenerator::vGenerate(const uchar cols, const uchar rows, short nMines, const uchar xClean, const uchar yClean)
{
    std::vector<std::vector<LibreMinesGameEngine::CellGameEngine>> 
    result(cols, std::vector<LibreMinesGameEngine::CellGameEngine>(rows));
    
    // Check if we need to avoid a clean area
    const bool bHasCleanArea = (xClean != 255 && yClean != 255);
    
    // Place mines randomly
    short minesToPlace = nMines;
    while (minesToPlace > 0)
    {
        uchar i = QRandomGenerator::global()->bounded(0, cols);
        uchar j = QRandomGenerator::global()->bounded(0, rows);
        
        // Avoid cells neighboring the clean cell when specified
        if (bHasCleanArea && i <= xClean + 1 && i >= xClean - 1 && j <= yClean + 1 && j >= yClean - 1)
        {
            continue;
        }
        
        LibreMinesGameEngine::CellGameEngine& cell = result[i][j];
        
        // Only place mine if cell is empty
        if (cell.value == CellValue::ZERO)
        {
            minesToPlace--;
            cell.value = CellValue::MINE;
        }
    }
    
    // Calculate adjacent mine counts for all non-mine cells
    for (uchar j = 0; j < rows; j++)
    {
        for (uchar i = 0; i < cols; i++)
        {
            LibreMinesGameEngine::CellGameEngine& cell = result[i][j];
            
            if (cell.value == CellValue::ZERO)
            {
                uchar minesCount = countAdjacentMines(result, i, j, cols - 1, rows - 1);
                cell.value = static_cast<CellValue>(minesCount);
            }
        }
    }
    
    return result;
}

uchar GameBoardRandomGenerator::countAdjacentMines(const std::vector<std::vector<LibreMinesGameEngine::CellGameEngine>>& matrix, uchar x, uchar y, uchar maxX, uchar maxY)
{
    uchar minesNeighbors = 0;
    
    // Check all 8 adjacent cells
    for (short i = x - 1; i <= x + 1; i++)
    {
        if (i < 0 || i > maxX)
            continue;
            
        for (short j = y - 1; j <= y + 1; j++)
        {
            if (j < 0 || j > maxY)
                continue;
                
            // Skip the center cell
            if (i == x && j == y)
                continue;
                
            if (matrix[i][j].value == CellValue::MINE)
                minesNeighbors++;
        }
    }
    
    return minesNeighbors;
}
