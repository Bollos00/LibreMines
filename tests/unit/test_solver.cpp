/*****************************************************************************
 * LibreMines                                                                *
 * Copyright (C) 2020-2026  Bruno Bollos Correa                              *
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
 *****************************************************************************/

#include <QtTest/QtTest>

#include "libreminesgameengine.h"
#include "libreminessolver.h"

// Test-state diagrams use '?' for unknown cells, '=' for shared unknown
// candidates, clue digits for known-safe cells, 'M' for identified mines, and
// '@' for a guaranteed safe starting cell not yet processed by the solver.
class TestLibreMinesSolver : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Zero Clear
    void cleanZeroClearsConnectedRegionAndKeepsMinesUnknown();
    void cleanZeroDoesNothingForNonZeroOrKnownCell();

    // Basic Clear
    void cleanBasicMarksUnknownNeighborsSafeWhenMinesAreAccountedFor();
    void cleanBasicMarksUnknownNeighborsAsMinesWhenAllMustBeMines();
    void cleanBasicLeavesAmbiguousNeighborsUnknown();
    void cleanBasicDoesNothingForUnknownClueCell();

    // Basic Patterns
    void basicPattern11XMarksUniqueCellsSafeViaDirectApply();
    void basicPattern11XMarksUniqueCellsSafeViaCleanBasicPatterns();
    void basicPattern11XTriangleMarksUniqueSafeCellViaDirectApply();
    void basicPattern11XTriangleMarksUniqueSafeCellViaCleanBasicPatterns();
    void basicPattern11XSingleSharedRowMarksUniqueSafeCellViaDirectApply();
    void basicPattern11XSingleSharedRowMarksUniqueSafeCellViaCleanBasicPatterns();
    void basicPattern11XTwoSharedRowsMarksTopRowSafeViaDirectApply();
    void basicPattern11XTwoSharedRowsMarksTopRowSafeViaCleanBasicPatterns();
    void basicPattern12XMarksUniqueCellAsMineViaDirectApply();
    void basicPattern12XMarksUniqueCellAsMineViaCleanBasicPatterns();
    void basicPattern12XTriangleMarksUniqueCellAsMineViaDirectApply();
    void basicPattern12XTriangleMarksUniqueCellAsMineViaCleanBasicPatterns();
    void basicPattern13XMarksUniqueCellsAsMinesViaDirectApply();
    void basicPattern13XMarksUniqueCellsAsMinesViaCleanBasicPatterns();
    void basicPattern14XMarksUniqueCellsAsMinesViaDirectApply();
    void basicPattern14XMarksUniqueCellsAsMinesViaCleanBasicPatterns();

    // Advanced Patterns
    void advancedPattern23XMarksUniqueCellAsMineViaDirectApply();
    void advancedPattern23XMarksUniqueCellAsMineViaCleanAdvancedPatterns();
    void advancedPattern222CornerResolvesCellsViaDirectApply();
    void advancedPattern222CornerResolvesCellsViaCleanAdvancedPatterns();
    void advancedPattern131CornerResolvesCellsViaDirectApply();
    void advancedPattern131CornerResolvesCellsViaCleanAdvancedPatterns();
    void advancedPattern1X1MarksUniqueCellsSafeViaDirectApply();
    void advancedPattern1X1MarksUniqueCellsSafeViaCleanAdvancedPatterns();
    void basicPattern11XRejectsNonSubsetViaDirectApply();
    void basicPattern12XRejectsWrongUniqueNeighborCountViaDirectApply();

private:
    enum class PatternTestRoute {
        DirectApply,
        Cleaner
    };

    void setBoard(uchar width, uchar height, short mines, const QString& description);
    void runBasic11XBoard(PatternTestRoute route);
    void runBasic11XTriangle(PatternTestRoute route);
    void runBasic11XSingleSharedRow(PatternTestRoute route);
    void runBasic11XTwoSharedRows(PatternTestRoute route);
    void runBasic12XBoard(PatternTestRoute route);
    void runBasic12XTriangle(PatternTestRoute route);
    void runBasic13X(PatternTestRoute route);
    void runBasic14X(PatternTestRoute route);
    void runAdvanced23X(PatternTestRoute route);
    void runAdvanced222Corner(PatternTestRoute route);
    void runAdvanced131Corner(PatternTestRoute route);
    void runAdvanced1X1(PatternTestRoute route);

    LibreMinesGameEngine gameEngine;
    LibreMinesSolver solver;
};

void TestLibreMinesSolver::initTestCase()
{
    // The solver test cases need no one-time application setup.
}

void TestLibreMinesSolver::cleanupTestCase()
{
    // Per-test state is released by cleanup().
}

void TestLibreMinesSolver::init()
{
    
}

void TestLibreMinesSolver::cleanup()
{
    solver.board.clear();
    solver.solverBoard.clear();
    solver.cellsNeedingTreatment.clear();
    solver.cols = 0;
    solver.rows = 0;
}

void TestLibreMinesSolver::setBoard(uchar width,
                                    uchar height,
                                    short mines,
                                    const QString& description)
{
    gameEngine.vNewGameForTest(width, height, mines, description);
    solver.board = gameEngine.getPrincipalMatrix();
    solver.cols = width;
    solver.rows = height;
    solver.solverBoard.assign(
        width,
        std::vector<LibreMinesSolver::SolverCellState>(
            height, LibreMinesSolver::SolverCellState::Unknown));
    solver.cellsNeedingTreatment.clear();
}

void TestLibreMinesSolver::cleanZeroClearsConnectedRegionAndKeepsMinesUnknown()
{
    // All cells are initially unknown. The zero at (2, 0) shall clear the
    // connected safe region while the actual mine remains unknown to the solver:
    // ? ? 0
    // ? ? ?
    // ? ? ?
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(* 1 0
           1 1 0
           0 0 0)"));

    QVERIFY(solver.cleanZero(2, 0));

    for (uchar x = 0; x < solver.cols; ++x)
    {
        for (uchar y = 0; y < solver.rows; ++y)
        {
            const auto expectedState = solver.board[x][y].value == CellValue::MINE
                ? LibreMinesSolver::SolverCellState::Unknown
                : LibreMinesSolver::SolverCellState::Safe;
            QVERIFY(solver.solverBoard[x][y] == expectedState);
        }
    }

    QVERIFY(!solver.cellsNeedingTreatment.contains({2, 0}));
    QVERIFY(solver.cellsNeedingTreatment.contains({1, 0}));
    QVERIFY(solver.cellsNeedingTreatment.contains({1, 1}));
    QVERIFY(solver.cellsNeedingTreatment.contains({0, 1}));
}

void TestLibreMinesSolver::cleanZeroDoesNothingForNonZeroOrKnownCell()
{
    // No cells are known; the clue at (0, 0) is 1, so cleanZero shall do nothing:
    // ? ? ?
    // ? ? ?
    // ? ? ?
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(1 1 1
           1 * 1
           1 1 1)"));

    QVERIFY(!solver.cleanZero(0, 0));
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Unknown);

    // The zero at (2, 0) is already known safe. Other cells remain unknown:
    // ? ? 0
    // ? ? ?
    // ? ? ?
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(* 1 0
           1 1 0
           0 0 0)"));
    solver.solverBoard[2][0] = LibreMinesSolver::SolverCellState::Safe;

    QVERIFY(!solver.cleanZero(2, 0));
    QVERIFY(solver.cellsNeedingTreatment.isEmpty());
    QVERIFY(solver.solverBoard[2][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[2][1] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::cleanBasicMarksUnknownNeighborsSafeWhenMinesAreAccountedFor()
{
    // A safe 1 clue and its mine are known; its other unknown neighbors shall
    // become safe:
    // 1 ? ?
    // ? M ?
    // ? ? ?
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(1 1 1
           1 * 1
           1 1 1)"));

    solver.solverBoard[0][0] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Mine;

    QVERIFY(solver.cleanBasic(0, 0));
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.cellsNeedingTreatment.contains({1, 0}));
    QVERIFY(solver.cellsNeedingTreatment.contains({0, 1}));
    QVERIFY(solver.solverBoard[1][1] == LibreMinesSolver::SolverCellState::Mine);
}

void TestLibreMinesSolver::cleanBasicMarksUnknownNeighborsAsMinesWhenAllMustBeMines()
{
    // Three neighboring 1 clues are known safe. The remaining unknown neighbor
    // of the upper-left clue shall be identified as a mine:
    // 1 1 ?
    // 1 ? ?
    // ? ? ?
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(1 1 1
           1 * 1
           1 1 1)"));

    solver.solverBoard[0][0] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][0] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][1] = LibreMinesSolver::SolverCellState::Safe;

    QVERIFY(solver.cleanBasic(0, 0));
    QVERIFY(solver.solverBoard[1][1] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.cellsNeedingTreatment.isEmpty());
}

void TestLibreMinesSolver::cleanBasicLeavesAmbiguousNeighborsUnknown()
{
    // Only the upper-left 1 clue is known safe, so its neighbors remain ambiguous:
    // 1 ? ?
    // ? ? ?
    // ? ? ?
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(1 1 1
           1 * 1
           1 1 1)"));

    solver.solverBoard[0][0] = LibreMinesSolver::SolverCellState::Safe;

    QVERIFY(!solver.cleanBasic(0, 0));
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[1][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.cellsNeedingTreatment.isEmpty());
}

void TestLibreMinesSolver::cleanBasicDoesNothingForUnknownClueCell()
{
    // Every cell, including the upper-left 1 clue, is unknown to the solver:
    // ? ? ?
    // ? ? ?
    // ? ? ?
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(1 1 1
           1 * 1
           1 1 1)"));

    QVERIFY(!solver.cleanBasic(0, 0));
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.cellsNeedingTreatment.isEmpty());
}

void TestLibreMinesSolver::basicPattern11XMarksUniqueCellsSafeViaDirectApply()
{
    runBasic11XBoard(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::basicPattern11XMarksUniqueCellsSafeViaCleanBasicPatterns()
{
    runBasic11XBoard(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::basicPattern11XTriangleMarksUniqueSafeCellViaDirectApply()
{
    runBasic11XTriangle(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::basicPattern11XTriangleMarksUniqueSafeCellViaCleanBasicPatterns()
{
    runBasic11XTriangle(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::basicPattern11XSingleSharedRowMarksUniqueSafeCellViaDirectApply()
{
    runBasic11XSingleSharedRow(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::basicPattern11XSingleSharedRowMarksUniqueSafeCellViaCleanBasicPatterns()
{
    runBasic11XSingleSharedRow(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::basicPattern11XTwoSharedRowsMarksTopRowSafeViaDirectApply()
{
    runBasic11XTwoSharedRows(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::basicPattern11XTwoSharedRowsMarksTopRowSafeViaCleanBasicPatterns()
{
    runBasic11XTwoSharedRows(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::basicPattern12XMarksUniqueCellAsMineViaDirectApply()
{
    runBasic12XBoard(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::basicPattern12XMarksUniqueCellAsMineViaCleanBasicPatterns()
{
    runBasic12XBoard(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::basicPattern12XTriangleMarksUniqueCellAsMineViaDirectApply()
{
    runBasic12XTriangle(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::basicPattern12XTriangleMarksUniqueCellAsMineViaCleanBasicPatterns()
{
    runBasic12XTriangle(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::basicPattern13XMarksUniqueCellsAsMinesViaDirectApply()
{
    runBasic13X(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::basicPattern13XMarksUniqueCellsAsMinesViaCleanBasicPatterns()
{
    runBasic13X(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::basicPattern14XMarksUniqueCellsAsMinesViaDirectApply()
{
    runBasic14X(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::basicPattern14XMarksUniqueCellsAsMinesViaCleanBasicPatterns()
{
    runBasic14X(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::advancedPattern23XMarksUniqueCellAsMineViaDirectApply()
{
    runAdvanced23X(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::advancedPattern23XMarksUniqueCellAsMineViaCleanAdvancedPatterns()
{
    runAdvanced23X(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::advancedPattern222CornerResolvesCellsViaDirectApply()
{
    runAdvanced222Corner(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::advancedPattern222CornerResolvesCellsViaCleanAdvancedPatterns()
{
    runAdvanced222Corner(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::advancedPattern131CornerResolvesCellsViaDirectApply()
{
    runAdvanced131Corner(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::advancedPattern131CornerResolvesCellsViaCleanAdvancedPatterns()
{
    runAdvanced131Corner(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::advancedPattern1X1MarksUniqueCellsSafeViaDirectApply()
{
    runAdvanced1X1(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::advancedPattern1X1MarksUniqueCellsSafeViaCleanAdvancedPatterns()
{
    runAdvanced1X1(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::runBasic11XBoard(PatternTestRoute route)
{
    // Unknown cells in the left column are unique to the first known 1 clue:
    // ? = = 1
    // ? 1 1 1
    // ? = = 0
    setBoard(4, 3, 1, QString::fromLatin1(
        R"(0 1 * 1
           0 1 1 1
           0 0 0 0)"));

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][1] = LibreMinesSolver::SolverCellState::Safe;
    for (uchar y = 0; y < solver.rows; ++y)
    {
        solver.solverBoard[3][y] = LibreMinesSolver::SolverCellState::Safe;
    }

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyNNXPatternLogic(1, 1, 2, 1, 1)
        : solver.cleanBasicPatterns(1, 1);
    QVERIFY(changed);
    for (uchar y = 0; y < solver.rows; ++y)
    {
        QVERIFY(solver.solverBoard[0][y] == LibreMinesSolver::SolverCellState::Safe);
    }
}

void TestLibreMinesSolver::runBasic11XTriangle(PatternTestRoute route)
{
    // The two known 1 clues share candidates; the top row becomes safe:
    // ? ? ? ?
    // = 1 = ?
    // = 1 1 ?
    setBoard(4, 3, 2, QString::fromLatin1(
        R"(1 1 0 0
           * 1 1 1
           1 1 1 *)"));

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][2] = LibreMinesSolver::SolverCellState::Safe;

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyNNXPatternLogic(1, 1, 0, 1, 1)
        : solver.cleanBasicPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Safe);
}

void TestLibreMinesSolver::runBasic11XSingleSharedRow(PatternTestRoute route)
{
    // The lower clue's unique neighbors are known safe, leaving one shared row:
    // ? ? ? ?
    // = 1 = ?
    // 1 1 1 ?
    setBoard(4, 3, 2, QString::fromLatin1(
        R"(1 1 0 0
           * 1 1 1
           1 1 1 *)"));

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][2] = LibreMinesSolver::SolverCellState::Safe;

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyNNXPatternLogic(1, 1, 0, 1, 1)
        : solver.cleanBasicPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Safe);
}

void TestLibreMinesSolver::runBasic11XTwoSharedRows(PatternTestRoute route)
{
    // Two known 1 clues share candidates in both rows below the top:
    // ? ? ?
    // = 1 =
    // = 1 =
    // 1 1 1
    // ? ? ?
    setBoard(3, 5, 2, QString::fromLatin1(
        R"(1 1 0
           * 1 0
           1 1 0
           1 1 1
           1 * 1)"));

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][2] = LibreMinesSolver::SolverCellState::Safe;
    for (uchar x = 0; x < solver.cols; ++x)
    {
        solver.solverBoard[x][3] = LibreMinesSolver::SolverCellState::Safe;
    }

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyNNXPatternLogic(1, 1, 1, 0, 1)
        : solver.cleanBasicPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[2][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[2][1] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[2][2] == LibreMinesSolver::SolverCellState::Safe);
}

void TestLibreMinesSolver::runBasic12XBoard(PatternTestRoute route)
{
    // The 2 and 1 clues share candidates; (0, 1) is a unique mine:
    // 2 = = 0
    // ? 2 1 0
    // 1 = = 0
    setBoard(4, 3, 2, QString::fromLatin1(
        R"(2 * 1 0
           * 2 1 0
           1 1 0 0)"));

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][0] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][2] = LibreMinesSolver::SolverCellState::Safe;
    for (uchar y = 0; y < solver.rows; ++y)
    {
        solver.solverBoard[3][y] = LibreMinesSolver::SolverCellState::Safe;
    }

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyMNXPatternLogic(1, 1, 2, 1, 1, 2)
        : solver.cleanBasicPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::runBasic12XTriangle(PatternTestRoute route)
{
    // The 2 and 1 clues share candidates; (0, 3) is a unique mine:
    // ? ? ?
    // 0 1 =
    // 1 2 1
    // ? 1 0
    // ? ? ?
    setBoard(3, 5, 2, QString::fromLatin1(
        R"(0 1 1
           0 1 *
           1 2 1
           * 1 0
           1 1 0)"));

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][3] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][3] = LibreMinesSolver::SolverCellState::Safe;

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyMNXPatternLogic(1, 2, 0, 2, 1, 2)
        : solver.cleanBasicPatterns(1, 2);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[2][1] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[0][3] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::runBasic13X(PatternTestRoute route)
{
    // The 3 and 1 clues leave two unique mine candidates for the 3:
    // ? = = 0
    // ? 3 1 0
    // 1 = = 0
    setBoard(4, 3, 3, QString::fromLatin1(
        R"(* * 1 0
           * 3 1 0
           1 1 0 0)"));

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][2] = LibreMinesSolver::SolverCellState::Safe;
    for (uchar y = 0; y < solver.rows; ++y)
    {
        solver.solverBoard[3][y] = LibreMinesSolver::SolverCellState::Safe;
    }

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyMNXPatternLogic(1, 1, 2, 1, 1, 3)
        : solver.cleanBasicPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::runBasic14X(PatternTestRoute route)
{
    // The 4 and 1 clues leave three unique mine candidates for the 4:
    // ? = = 0
    // ? 4 1 0
    // ? = = 0
    setBoard(4, 3, 4, QString::fromLatin1(
        R"(* * 1 0
           * 4 1 0
           * 2 0 0)"));

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][1] = LibreMinesSolver::SolverCellState::Safe;
    for (uchar y = 0; y < solver.rows; ++y)
    {
        solver.solverBoard[3][y] = LibreMinesSolver::SolverCellState::Safe;
    }

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyMNXPatternLogic(1, 1, 2, 1, 1, 4)
        : solver.cleanBasicPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::runAdvanced23X(PatternTestRoute route)
{
    // The 2 and 3 clues share candidates; (0, 2) is a unique mine candidate:
    // = 2 = ?
    // = 3 1 1
    // ? 2 0 0
    // ? 2 0 @
    // ? ? ? ?
    setBoard(4, 5, 4, QString::fromLatin1(
        R"(* 2 * 1
           2 3 1 1
           * 2 0 0
           * 2 0 0
           1 1 0 0)"));

    const QPair<uchar, uchar> revealedCells[] = {
        {1, 0},
        {1, 1}, {2, 1}, {3, 1},
        {1, 2}, {2, 2}, {3, 2},
        {1, 3}, {2, 3}, {3, 3}
    };
    for (const QPair<uchar, uchar>& cell : revealedCells)
    {
        solver.solverBoard[cell.first][cell.second] =
            LibreMinesSolver::SolverCellState::Safe;
        solver.cellsNeedingTreatment.insert(cell);
    }
    solver.safeX = 3;
    solver.safeY = 3;

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyMNXPatternLogic(1, 1, 1, 0, 2, 3)
        : solver.cleanAdvancedPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Mine);
}

void TestLibreMinesSolver::runAdvanced222Corner(PatternTestRoute route)
{
    // Clearing the middle zero reveals a 2-ring with a 2-2-2 corner:
    // ? = = M = =
    // = 2 2 2 2 =
    // = 2 0 0 2 =
    // M 2 0 0 2 =
    // = 2 2 2 2 =
    // = = = = = =
    setBoard(6, 6, 8, QString::fromLatin1(
        R"(0 1 * * 1 0
           1 2 2 2 2 1
           * 2 0 0 2 *
           * 2 0 0 2 *
           1 2 2 2 2 1
           0 1 * * 1 0)"));

    QVERIFY(solver.cleanZero(2, 2));
    for (uchar x = 1; x <= 4; ++x)
    {
        for (uchar y = 1; y <= 4; ++y)
        {
            QVERIFY(solver.solverBoard[x][y] ==
                    LibreMinesSolver::SolverCellState::Safe);
        }
    }

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.apply222CornerPatternLogic(1, 1, 2, 1, 1, 2)
        : solver.cleanAdvancedPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][3] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[3][0] == LibreMinesSolver::SolverCellState::Mine);
}

void TestLibreMinesSolver::runAdvanced131Corner(PatternTestRoute route)
{
    // The 3 and two perpendicular 1 clues leave (0, 0) as the unique middle
    // mine and one unique safe cell for each 1:
    // ? = = ? =
    // = 3 1 1 =
    // = 1 0 1 =
    // ? 1 1 1 =
    // = = = = =
    setBoard(5, 5, 6, QString::fromLatin1(
        R"(* * 1 1 1
           * 3 1 1 *
           1 1 0 1 1
           1 1 1 1 1
           * 1 1 * 1)"));

    QVERIFY(solver.cleanZero(2, 2));

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.apply131CornerPatternLogic(1, 1, 2, 1, 1, 2)
        : solver.cleanAdvancedPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[3][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][3] == LibreMinesSolver::SolverCellState::Safe);
}

void TestLibreMinesSolver::runAdvanced1X1(PatternTestRoute route)
{
    // The 1 clues share unknown candidates; unique cells of the first become safe:
    // ? ? * *
    // ? ? = 3
    // ? 1 = 1
    // ? 2 2 2
    // ? 1 = 1
    setBoard(4, 5, 4, QString::fromLatin1(
        R"(0 1 * *
           0 2 3 3
           0 1 * 1
           0 2 2 2
           0 1 * 1)"));

    solver.solverBoard[2][0] = LibreMinesSolver::SolverCellState::Mine;
    solver.solverBoard[3][0] = LibreMinesSolver::SolverCellState::Mine;
    const QPair<uchar, uchar> knownSafeCells[] = {
        {3, 1},
        {1, 2}, {3, 2},
        {1, 3}, {2, 3}, {3, 3},
        {1, 4}, {3, 4}
    };
    for (const QPair<uchar, uchar>& cell : knownSafeCells)
    {
        solver.solverBoard[cell.first][cell.second] =
            LibreMinesSolver::SolverCellState::Safe;
    }

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyNNXPatternLogic(1, 2, 3, 2, 1)
        : solver.cleanAdvancedPatterns(1, 2);
    QVERIFY(changed);
    const QPair<uchar, uchar> uniqueCells[] = {
        {0, 1}, {1, 1}, {0, 2}, {0, 3}
    };
    for (const QPair<uchar, uchar>& cell : uniqueCells)
    {
        QVERIFY(solver.solverBoard[cell.first][cell.second] ==
                LibreMinesSolver::SolverCellState::Safe);
    }
    QVERIFY(solver.solverBoard[2][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[2][2] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::basicPattern11XRejectsNonSubsetViaDirectApply()
{
    // Reopening (3, 0) breaks the subset relation; unique cells stay unknown:
    // ? = = ?
    // ? 1 1 1
    // ? = = 0
    setBoard(4, 3, 1, QString::fromLatin1(
        R"(0 1 * 1
           0 1 1 1
           0 0 0 0)"));

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][1] = LibreMinesSolver::SolverCellState::Safe;
    for (uchar y = 0; y < solver.rows; ++y)
    {
        solver.solverBoard[3][y] = LibreMinesSolver::SolverCellState::Safe;
    }
    solver.solverBoard[3][0] = LibreMinesSolver::SolverCellState::Unknown;

    QVERIFY(!solver.applyNNXPatternLogic(1, 1, 2, 1, 1));
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.cellsNeedingTreatment.isEmpty());
}

void TestLibreMinesSolver::basicPattern12XRejectsWrongUniqueNeighborCountViaDirectApply()
{
    // The unknown unique candidate count does not match the 1-2-X deduction:
    // ? = = 0
    // ? 2 1 0
    // 1 = = 0
    setBoard(4, 3, 2, QString::fromLatin1(
        R"(2 * 1 0
           * 2 1 0
           1 1 0 0)"));

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][0] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][2] = LibreMinesSolver::SolverCellState::Safe;
    for (uchar y = 0; y < solver.rows; ++y)
    {
        solver.solverBoard[3][y] = LibreMinesSolver::SolverCellState::Safe;
    }
    solver.solverBoard[0][0] = LibreMinesSolver::SolverCellState::Unknown;

    QVERIFY(!solver.applyMNXPatternLogic(1, 1, 2, 1, 1, 2));
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.cellsNeedingTreatment.isEmpty());
}

QTEST_GUILESS_MAIN(TestLibreMinesSolver)
#include "test_solver.moc"
