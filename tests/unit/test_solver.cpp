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

class TestLibreMinesSolver : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void cleanZeroClearsConnectedRegionAndKeepsMinesUnknown();
    void cleanZeroDoesNothingForNonZeroOrKnownCell();

    void cleanBasicMarksUnknownNeighborsSafeWhenMinesAreAccountedFor();
    void cleanBasicMarksUnknownNeighborsAsMinesWhenAllMustBeMines();
    void cleanBasicLeavesAmbiguousNeighborsUnknown();
    void cleanBasicDoesNothingForUnknownClueCell();

    void cleanBasicPatternsDetects11X();
    void apply11XPatternLogicClearsTopRowForTriangle();
    void apply11XPatternLogicClearsTopRowForSingleSharedRow();
    void apply11XPatternLogicClearsTopRowForTwoSharedRows();
    void cleanBasicPatternsDetects12X();
    void apply1NXPatternLogicMarksUniqueCellAsMineFor12XTriangle();
    void cleanBasicPatternsDetects13X();
    void cleanBasicPatternsDetects14X();
    void cleanBasicPatternsDetects23XForProvidedBoard();
    void solveResolvesProvided23XBoard();
    void apply11XPatternLogicRejectsNonSubset();
    void apply1NXPatternLogicRejectsWrongUniqueNeighborCount();

private:
    void setBoard(uchar width, uchar height, short mines, const QString& description);
    void prepare11XPattern();
    void prepare1NXPattern(int remainingMines);
    void prepare23XPattern();

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

void TestLibreMinesSolver::prepare11XPattern()
{
    // The mine at (2, 0) is shared by the two clue cells at (1, 1) and (2, 1).
    setBoard(4, 3, 1, QString::fromLatin1(
        R"(01*1
           0111
           0000)"));

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][1] = LibreMinesSolver::SolverCellState::Safe;

    // The right-hand clue's neighbors outside the shared area are already known safe.
    for (uchar y = 0; y < solver.rows; ++y)
    {
        solver.solverBoard[3][y] = LibreMinesSolver::SolverCellState::Safe;
    }
}

void TestLibreMinesSolver::prepare1NXPattern(int remainingMines)
{
    QString description;
    switch (remainingMines)
    {
        case 2:
            // One mine is unique to the N-clue and one is shared with the 1-clue.
            description = QString::fromLatin1(
                R"(2*10
                   *210
                   1100)");
            break;
        case 3:
            description = QString::fromLatin1(
                R"(**10
                   *310
                   1100)");
            break;
        case 4:
            description = QString::fromLatin1(
                R"(**10
                   *410
                   *200)");
            break;
        default:
            QFAIL("Test fixture only supports N values 2 through 4");
            return;
    }

    setBoard(4, 3, static_cast<short>(remainingMines), description);
    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][1] = LibreMinesSolver::SolverCellState::Safe;

    // The three cells in column zero are unique to the N-clue. Leave N-1 of
    // them unknown and mark the rest safe to isolate the intended deduction.
    for (uchar y = 0; y < solver.rows; ++y)
    {
        const bool containsUniqueMine = remainingMines == 4 ||
            (remainingMines == 3 && y < 2) ||
            (remainingMines == 2 && y == 1);
        if (containsUniqueMine)
        {
            continue;
        }
        solver.solverBoard[0][y] = LibreMinesSolver::SolverCellState::Safe;
    }

    // The cells to the right of the 1-clue are unique to that clue and known safe.
    for (uchar y = 0; y < solver.rows; ++y)
    {
        solver.solverBoard[3][y] = LibreMinesSolver::SolverCellState::Safe;
    }
}

void TestLibreMinesSolver::prepare23XPattern()
{
    // The 2-clue and 3-clue share three candidates. The '?' candidate is
    // unique to the 3-clue and must therefore be a mine.
    // =2==
    // =311
    // ?200
    // =200
    setBoard(4, 4, 4, QString::fromLatin1(
        R"(*2*1
           2311
           *200
           *200)"));

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

    // The bottom-right zero is already revealed in the diagram and is a
    // suitable safe starting point for the full solve() pass.
    solver.safeX = 3;
    solver.safeY = 3;
}

void TestLibreMinesSolver::cleanZeroClearsConnectedRegionAndKeepsMinesUnknown()
{
    // The zero region opens every safe cell on this board:
    // *10
    // 110
    // 000
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(*10
          110
          000)"));

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
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(111
          1*1
          111)"));

    QVERIFY(!solver.cleanZero(0, 0));
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Unknown);

    setBoard(3, 3, 1, QString::fromLatin1(
        R"(*10
           110
           000)"));
    solver.solverBoard[2][0] = LibreMinesSolver::SolverCellState::Safe;

    QVERIFY(!solver.cleanZero(2, 0));
    QVERIFY(solver.cellsNeedingTreatment.isEmpty());
    QVERIFY(solver.solverBoard[2][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[2][1] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::cleanBasicMarksUnknownNeighborsSafeWhenMinesAreAccountedFor()
{
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(111
           1*1
           111)"));

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
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(111
           1*1
           111)"));

    solver.solverBoard[0][0] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][0] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][1] = LibreMinesSolver::SolverCellState::Safe;

    QVERIFY(solver.cleanBasic(0, 0));
    QVERIFY(solver.solverBoard[1][1] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.cellsNeedingTreatment.isEmpty());
}

void TestLibreMinesSolver::cleanBasicLeavesAmbiguousNeighborsUnknown()
{
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(111
           1*1
           111)"));

    solver.solverBoard[0][0] = LibreMinesSolver::SolverCellState::Safe;

    QVERIFY(!solver.cleanBasic(0, 0));
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[1][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.cellsNeedingTreatment.isEmpty());
}

void TestLibreMinesSolver::cleanBasicDoesNothingForUnknownClueCell()
{
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(111
           1*1
           111)"));

    QVERIFY(!solver.cleanBasic(0, 0));
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.cellsNeedingTreatment.isEmpty());
}

void TestLibreMinesSolver::cleanBasicPatternsDetects11X()
{
    prepare11XPattern();

    QVERIFY(solver.cleanBasicPatterns(1, 1));
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[2][0] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.cellsNeedingTreatment.contains({0, 0}));
    QVERIFY(solver.cellsNeedingTreatment.contains({0, 1}));
    QVERIFY(solver.cellsNeedingTreatment.contains({0, 2}));
}

void TestLibreMinesSolver::apply11XPatternLogicClearsTopRowForTriangle()
{
    // Pattern under test; the three '?' cells are safe, and '=' cells remain
    // shared unknown candidates for the two vertically adjacent 1-clues:
    // ???
    // =1=
    // =11
    setBoard(4, 3, 2, QString::fromLatin1(
        R"(1100
           *111
           111*)"));

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][2] = LibreMinesSolver::SolverCellState::Safe;

    QVERIFY(solver.applyNNXPatternLogic(1, 1, 1, 2, 1));

    for (uchar x = 0; x < 3; ++x)
    {
        QVERIFY(solver.solverBoard[x][0] == LibreMinesSolver::SolverCellState::Safe);
        QVERIFY(solver.cellsNeedingTreatment.contains({x, 0}));
    }
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[2][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::apply11XPatternLogicClearsTopRowForSingleSharedRow()
{
    // Pattern under test (the two '=' cells are shared unknown candidates):
    // ???
    // =1=
    // 111
    setBoard(4, 3, 2, QString::fromLatin1(
        R"(1100
           *111
           111*)"));

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][2] = LibreMinesSolver::SolverCellState::Safe;

    QVERIFY(solver.applyNNXPatternLogic(1, 1, 1, 2, 1));

    for (uchar x = 0; x < 3; ++x)
    {
        QVERIFY(solver.solverBoard[x][0] == LibreMinesSolver::SolverCellState::Safe);
        QVERIFY(solver.cellsNeedingTreatment.contains({x, 0}));
    }
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[2][1] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::apply11XPatternLogicClearsTopRowForTwoSharedRows()
{
    // Pattern under test (the '=' cells remain shared unknown candidates):
    // ???
    // =1=
    // =1=
    // 111
    setBoard(3, 5, 2, QString::fromLatin1(
        R"(110
           *10
           110
           111
           1*1)"));

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][2] = LibreMinesSolver::SolverCellState::Safe;
    for (uchar x = 0; x < solver.cols; ++x)
    {
        solver.solverBoard[x][3] = LibreMinesSolver::SolverCellState::Safe;
    }

    QVERIFY(solver.applyNNXPatternLogic(1, 1, 1, 2, 1));

    for (uchar x = 0; x < solver.cols; ++x)
    {
        QVERIFY(solver.solverBoard[x][0] == LibreMinesSolver::SolverCellState::Safe);
        QVERIFY(solver.cellsNeedingTreatment.contains({x, 0}));
    }
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[2][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[2][2] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::cleanBasicPatternsDetects12X()
{
    prepare1NXPattern(2);

    QVERIFY(solver.cleanBasicPatterns(1, 1));
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::apply1NXPatternLogicMarksUniqueCellAsMineFor12XTriangle()
{
    // The 2-clue shares one candidate with the adjacent 1-clue; '?' is its
    // only unique candidate and must therefore be a mine.
    // ===
    // =1=
    // =21
    // ?10
    // ===
    setBoard(3, 5, 2, QString::fromLatin1(
        R"(011
           01*
           121
           *10
           110)"));

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][3] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][3] = LibreMinesSolver::SolverCellState::Safe;

    QVERIFY(solver.applyMNXPatternLogic(1, 2, 2, 2, 1, 2));
    QVERIFY(solver.solverBoard[0][3] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[2][1] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::cleanBasicPatternsDetects13X()
{
    prepare1NXPattern(3);

    QVERIFY(solver.cleanBasicPatterns(1, 1));
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::cleanBasicPatternsDetects14X()
{
    prepare1NXPattern(4);

    QVERIFY(solver.cleanBasicPatterns(1, 1));
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::cleanBasicPatternsDetects23XForProvidedBoard()
{
    prepare23XPattern();

    QVERIFY(solver.cleanBasicPatterns(1, 1));
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Mine);
}

void TestLibreMinesSolver::solveResolvesProvided23XBoard()
{
    prepare23XPattern();

    QVERIFY(solver.solve());
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Mine);

    for (uchar x = 0; x < solver.cols; ++x)
    {
        for (uchar y = 0; y < solver.rows; ++y)
        {
            const auto expectedState = solver.board[x][y].value == CellValue::MINE
                ? LibreMinesSolver::SolverCellState::Mine
                : LibreMinesSolver::SolverCellState::Safe;
            QVERIFY(solver.solverBoard[x][y] == expectedState);
        }
    }
}

void TestLibreMinesSolver::apply11XPatternLogicRejectsNonSubset()
{
    prepare11XPattern();
    solver.solverBoard[3][0] = LibreMinesSolver::SolverCellState::Unknown;

    QVERIFY(!solver.applyNNXPatternLogic(1, 1, 2, 1, 1));
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.cellsNeedingTreatment.isEmpty());
}

void TestLibreMinesSolver::apply1NXPatternLogicRejectsWrongUniqueNeighborCount()
{
    prepare1NXPattern(2);
    solver.solverBoard[0][0] = LibreMinesSolver::SolverCellState::Unknown;

    QVERIFY(!solver.applyMNXPatternLogic(1, 1, 2, 1, 1, 2));
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.cellsNeedingTreatment.isEmpty());
}

QTEST_GUILESS_MAIN(TestLibreMinesSolver)
#include "test_solver.moc"
