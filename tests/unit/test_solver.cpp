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

// Test-state diagrams use:
// - "0", "1", "2", "3", "4", "5", "6", "7", "8" for known clue cells
// - "*" for known mine cells
// - '=' for unknown cells
// - "M" for unknown cells that are expected to be marked as mines
// - "S" for unknown cells that are expected to be marked as safe
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

    // Solver cell marking
    void markCellUpdatesStateCountersAndTreatmentSet();
    void markCellSafeStateExpandsZeroCells();

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
    void basicPattern12XResolvesTwoSidedCandidatesViaDirectApply();
    void basicPattern12XResolvesTwoSidedCandidatesViaCleanBasicPatterns();

    // Advanced Patterns
    void advancedPattern23XMarksUniqueCellAsMineViaDirectApply();
    void advancedPattern23XMarksUniqueCellAsMineViaCleanAdvancedPatterns();
    void advancedPattern222CornerResolvesCellsViaDirectApply();
    void advancedPattern222CornerResolvesCellsViaCleanAdvancedPatterns();
    void advancedPattern131CornerResolvesCellsViaDirectApply();
    void advancedPattern131CornerResolvesCellsViaCleanAdvancedPatterns();
    void advancedPattern1X1MarksUniqueCellsSafeViaDirectApply();
    void advancedPattern1X1MarksUniqueCellsSafeViaCleanAdvancedPatterns();
    void advancedPattern22XMarksUniqueCellsSafeViaDirectApply();
    void advancedPattern22XMarksUniqueCellsSafeViaCleanAdvancedPatterns();
    void advancedPattern33XMarksUniqueCellsSafeViaDirectApply();
    void advancedPattern33XMarksUniqueCellsSafeViaCleanAdvancedPatterns();
    void advancedPattern2X2ResolvesCellsViaDirectApply();
    void advancedPattern2X2ResolvesCellsViaCleanAdvancedPatterns();
    void advancedPattern24XMarksUniqueCellsAsMinesViaDirectApply();
    void advancedPattern24XMarksUniqueCellsAsMinesViaCleanAdvancedPatterns();
    void advancedPattern34XMarksUniqueCellsAsMinesViaDirectApply();
    void advancedPattern34XMarksUniqueCellsAsMinesViaCleanAdvancedPatterns();

    // General Pair Rule
    void generalPairRuleMarksUniqueCellsSafeViaDirectApply();
    void generalPairRuleMarksUniqueCellsSafeViaCleanGeneralPairRule();
    void generalPairRuleMarksUniqueCellsAsMinesViaDirectApply();
    void generalPairRuleMarksUniqueCellsAsMinesViaCleanGeneralPairRule();
    void generalPairRuleMarksOnlyBUniqueCellsViaDirectApply();
    void generalPairRuleMarksOnlyBUniqueCellsViaCleanGeneralPairRule();
    void generalPairRuleDiagonal11XMarksUniqueCellsSafeViaDirectApply();
    void generalPairRuleDiagonal11XMarksUniqueCellsSafeViaCleanGeneralPairRule();

    // Enumeration
    void enumerationResolvesCellsAcrossConnectedClues();
    void enumerationRespectsMaxEnumerationCluesWindows();
    void enumerationResolvesThreeConnectedCluesWithoutPairRule();

    // Mine Counting
    void mineCountingMarksAllUnknownCellsSafeWhenNoMinesLeft();
    void mineCountingMarksAllUnknownCellsAsMinesWhenAllRemainingAreMines();
    void mineCountingMarksFreeCellsSafeWhenGroupsUseAllMines();
    void mineCountingRejectsInconsistentMineCount();
    void mineCountingDoesNothingWhenStrategyDisabled();
    void mineCountingResolvesAmbiguousFiftyFiftyWithLastMine();

    // Rejection paths
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
    void runBasic12XTwoSidedCandidates(PatternTestRoute route);
    void runBasic13X(PatternTestRoute route);
    void runBasic14X(PatternTestRoute route);
    void runAdvanced23X(PatternTestRoute route);
    void runAdvanced222Corner(PatternTestRoute route);
    void runAdvanced131Corner(PatternTestRoute route);
    void runAdvanced1X1(PatternTestRoute route);
    void runAdvanced22X(PatternTestRoute route);
    void runAdvanced33X(PatternTestRoute route);
    void runAdvanced2X2(PatternTestRoute route);
    void runAdvanced24X(PatternTestRoute route);
    void runAdvanced34X(PatternTestRoute route);
    void runGeneralPairRuleOnlyASafe(PatternTestRoute route);
    void runGeneralPairRuleOnlyAMine(PatternTestRoute route);
    void runGeneralPairRuleOnlyBMine(PatternTestRoute route);
    void runGeneralPairRuleDiagonal11X(PatternTestRoute route);

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
    solver.nCellsMarkedSafe = 0;
    solver.nCellsMarkedMine = 0;
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
    solver.nCellsMarkedSafe = 0;
    solver.nCellsMarkedMine = 0;
}

void TestLibreMinesSolver::markCellUpdatesStateCountersAndTreatmentSet()
{
    setBoard(2, 2, 1, QString::fromLatin1(
        R"(1*
           11)"));

    const QPair<uchar, uchar> cell(0, 0);
    solver.markCell(cell, LibreMinesSolver::SolverCellState::Safe);
    QCOMPARE(solver.nCellsMarkedSafe, quint32(1));
    QCOMPARE(solver.nCellsMarkedMine, quint32(0));
    QVERIFY(solver.cellsNeedingTreatment.contains({0, 0}));

    // Repeating a mark updates the treatment set without counting the cell twice.
    solver.markCell(cell, LibreMinesSolver::SolverCellState::Safe);
    QCOMPARE(solver.nCellsMarkedSafe, quint32(1));

    solver.markCell(cell, LibreMinesSolver::SolverCellState::Mine);
    QCOMPARE(solver.nCellsMarkedSafe, quint32(0));
    QCOMPARE(solver.nCellsMarkedMine, quint32(1));
    QVERIFY(!solver.cellsNeedingTreatment.contains({0, 0}));
}

void TestLibreMinesSolver::markCellSafeStateExpandsZeroCells()
{
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(* 1 0
           1 1 0
           0 0 0)"));

    solver.markCell({2, 0}, LibreMinesSolver::SolverCellState::Safe);

    QCOMPARE(solver.nCellsMarkedSafe, quint32(8));
    QCOMPARE(solver.nCellsMarkedMine, quint32(0));
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(!solver.cellsNeedingTreatment.contains({2, 0}));
}

void TestLibreMinesSolver::cleanZeroClearsConnectedRegionAndKeepsMinesUnknown()
{
    // All cells are initially unknown. The zero at (2, 0) shall clear the
    // connected safe region while the actual mine remains unknown to the solver:
    // = S S
    // = S S
    // S S S
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
    // = = =
    // = = =
    // = = =
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(1 1 1
           1 * 1
           1 1 1)"));

    QVERIFY(!solver.cleanZero(0, 0));
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Unknown);

    // The zero at (2, 0) is already known safe. Other cells remain unknown:
    // = = 0
    // = = =
    // = = =
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
    // 1 S =
    // S * =
    // = = =
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
    // 1 1 =
    // 1 M =
    // = = =
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
    // 1 = =
    // = = =
    // = = =
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
    // = = =
    // = = =
    // = = =
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
    // S = = 1
    // S 1 1 1
    // S = = 0
    setBoard(4, 3, 1, QString::fromLatin1(
        R"(0 1 * 1
           0 1 1 1
           0 0 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern11X);

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
    // = S = =
    // = 1 = =
    // = 1 1 =
    setBoard(4, 3, 2, QString::fromLatin1(
        R"(1 1 0 0
           * 1 1 1
           1 1 1 *)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern11X);

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][2] = LibreMinesSolver::SolverCellState::Safe;

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyNNXPatternLogic(1, 1, 1, 2, 1)
        : solver.cleanBasicPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Safe);
}

void TestLibreMinesSolver::runBasic11XSingleSharedRow(PatternTestRoute route)
{
    // The lower clue's unique neighbors are known safe, leaving one shared row:
    // = S = =
    // = 1 = =
    // 1 1 1 =
    setBoard(4, 3, 2, QString::fromLatin1(
        R"(1 1 0 0
           * 1 1 1
           1 1 1 *)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern11X);

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][2] = LibreMinesSolver::SolverCellState::Safe;

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyNNXPatternLogic(1, 1, 1, 2, 1)
        : solver.cleanBasicPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Safe);
}

void TestLibreMinesSolver::runBasic11XTwoSharedRows(PatternTestRoute route)
{
    // Two known 1 clues share candidates in both rows below the top:
    // S S S
    // = 1 =
    // = 1 =
    // 1 1 1
    // = = =
    setBoard(3, 5, 2, QString::fromLatin1(
        R"(1 1 0
           * 1 0
           1 1 0
           1 1 1
           1 * 1)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern11X);

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][2] = LibreMinesSolver::SolverCellState::Safe;
    for (uchar x = 0; x < solver.cols; ++x)
    {
        solver.solverBoard[x][3] = LibreMinesSolver::SolverCellState::Safe;
    }

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyNNXPatternLogic(1, 1, 1, 2, 1)
        : solver.cleanBasicPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[2][0] == LibreMinesSolver::SolverCellState::Safe);
}

void TestLibreMinesSolver::runBasic12XBoard(PatternTestRoute route)
{
    // The 2 and 1 clues share candidates; (0, 1) is a unique mine:
    // 2 = = 0
    // M 2 1 0
    // 1 = = 0
    setBoard(4, 3, 2, QString::fromLatin1(
        R"(2 * 1 0
           * 2 1 0
           1 1 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern12X);

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
    // The 2 and 1 clues share candidates; (0, 3) is a unique mine, while the cells unique to the 1 clue are known safe:
    // = = =
    // 0 1 M
    // = 2 1
    // = 1 =
    // S S S
    setBoard(3, 5, 2, QString::fromLatin1(
        R"(0 1 1
           0 1 *
           1 2 1
           * 1 0
           1 1 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern12X);

    solver.solverBoard[0][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][2] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[1][3] = LibreMinesSolver::SolverCellState::Safe;

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyMNXPatternLogic(1, 2, 1, 3, 1, 2)
        : solver.cleanBasicPatterns(1, 2);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[2][1] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[0][3] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[0][4] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[1][4] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[2][4] == LibreMinesSolver::SolverCellState::Safe);
}

void TestLibreMinesSolver::runBasic13X(PatternTestRoute route)
{
    // The 3 and 1 clues leave two unique mine candidates for the 3.
    // M = = S
    // M 3 1 S
    // 1 = = S
    setBoard(4, 3, 3, QString::fromLatin1(
        R"(* * 1 0
           * 3 1 0
           1 1 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern13X);

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][2] = LibreMinesSolver::SolverCellState::Safe;


    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyMNXPatternLogic(1, 1, 2, 1, 1, 3)
        : solver.cleanBasicPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[3][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[3][1] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[3][2] == LibreMinesSolver::SolverCellState::Safe);
}

void TestLibreMinesSolver::runBasic14X(PatternTestRoute route)
{
    // The 4 and 1 clues leave three unique mine candidates for the 4:
    // M = = 0
    // M 4 1 0
    // M = = 0
    setBoard(4, 3, 4, QString::fromLatin1(
        R"(* * 1 0
           * 4 1 0
           * 2 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern14X);

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
    // = 2 = =
    // = 3 1 1
    // M 2 0 0
    // = 2 0 0
    // = = = =
    setBoard(4, 5, 4, QString::fromLatin1(
        R"(* 2 * 1
           2 3 1 1
           * 2 0 0
           * 2 0 0
           1 1 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern23X);

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
    // S = = M = =
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
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern222Corner);

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
    // M = = S =
    // = 3 1 1 =
    // = 1 0 1 =
    // S 1 1 1 =
    // = = = = =
    setBoard(5, 5, 6, QString::fromLatin1(
        R"(* * 1 1 1
           * 3 1 1 *
           1 1 0 1 1
           1 1 1 1 1
           * 1 1 * 1)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern131Corner);

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
    // = = * *
    // S = = 3
    // S 1 = 1
    // S 2 2 2
    // S 1 = 1
    setBoard(4, 5, 4, QString::fromLatin1(
        R"(0 1 * *
           0 2 3 3
           0 1 * 1
           0 2 2 2
           0 1 * 1)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern1X1);

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

void TestLibreMinesSolver::advancedPattern22XMarksUniqueCellsSafeViaDirectApply()
{
    runAdvanced22X(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::advancedPattern22XMarksUniqueCellsSafeViaCleanAdvancedPatterns()
{
    runAdvanced22X(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::advancedPattern33XMarksUniqueCellsSafeViaDirectApply()
{
    runAdvanced33X(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::advancedPattern33XMarksUniqueCellsSafeViaCleanAdvancedPatterns()
{
    runAdvanced33X(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::advancedPattern2X2ResolvesCellsViaDirectApply()
{
    runAdvanced2X2(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::advancedPattern2X2ResolvesCellsViaCleanAdvancedPatterns()
{
    runAdvanced2X2(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::advancedPattern24XMarksUniqueCellsAsMinesViaDirectApply()
{
    runAdvanced24X(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::advancedPattern24XMarksUniqueCellsAsMinesViaCleanAdvancedPatterns()
{
    runAdvanced24X(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::advancedPattern34XMarksUniqueCellsAsMinesViaDirectApply()
{
    runAdvanced34X(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::advancedPattern34XMarksUniqueCellsAsMinesViaCleanAdvancedPatterns()
{
    runAdvanced34X(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::runAdvanced22X(PatternTestRoute route)
{
    // Two adjacent 2 clues share candidates; the unique cells of the first are safe:
    // S = = 0 =
    // S 2 2 0 =
    // S = = 0 =
    setBoard(5, 3, 2, QString::fromLatin1(
        R"(1 * 1 0 0
           2 2 2 0 0
           1 * 1 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern22X);

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][1] = LibreMinesSolver::SolverCellState::Safe;
    for (uchar y = 0; y < solver.rows; ++y)
    {
        solver.solverBoard[3][y] = LibreMinesSolver::SolverCellState::Safe;
    }

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyNNXPatternLogic(1, 1, 2, 1, 2)
        : solver.cleanAdvancedPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::runAdvanced33X(PatternTestRoute route)
{
    // Two adjacent 3 clues share candidates; the unique cells of the first are safe:
    // S = = 1 =
    // S 3 3 1 =
    // S = = 0 =
    setBoard(5, 3, 3, QString::fromLatin1(
        R"(1 * * 1 0
           2 3 3 1 0
           1 * 1 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern33X);

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][1] = LibreMinesSolver::SolverCellState::Safe;
    for (uchar y = 0; y < solver.rows; ++y)
    {
        solver.solverBoard[3][y] = LibreMinesSolver::SolverCellState::Safe;
    }

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyNNXPatternLogic(1, 1, 2, 1, 3)
        : solver.cleanAdvancedPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::runAdvanced2X2(PatternTestRoute route)
{
    // The two 2 clues are two cells apart and share candidates; the unique cells of
    // the left clue are safe:
    // S S = 1 0
    // S 2 = 2 0
    // S S = 1 0
    setBoard(5, 3, 2, QString::fromLatin1(
        R"(0 1 * 1 0
           0 2 2 2 0
           0 1 * 1 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern2X2);

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[3][1] = LibreMinesSolver::SolverCellState::Safe;
    const QPair<uchar, uchar> knownSafeCells[] = {
        {3, 0}, {4, 0}, {4, 1}, {3, 2}, {4, 2}
    };
    for (const QPair<uchar, uchar>& cell : knownSafeCells)
    {
        solver.solverBoard[cell.first][cell.second] = LibreMinesSolver::SolverCellState::Safe;
    }

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyNNXPatternLogic(1, 1, 3, 1, 2)
        : solver.cleanAdvancedPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[1][2] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[2][1] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::runAdvanced24X(PatternTestRoute route)
{
    // The 4 and 2 clues share candidates; the two unique cells of the 4 are mines:
    // M = = 1 =
    // 3 4 2 1 =
    // M = = 0 =
    setBoard(5, 3, 4, QString::fromLatin1(
        R"(* * * 1 0
           3 4 2 1 0
           * 1 0 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern24X);

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][1] = LibreMinesSolver::SolverCellState::Safe;
    for (uchar y = 0; y < solver.rows; ++y)
    {
        solver.solverBoard[3][y] = LibreMinesSolver::SolverCellState::Safe;
    }

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyMNXPatternLogic(1, 1, 2, 1, 2, 4)
        : solver.cleanAdvancedPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[2][2] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::runAdvanced34X(PatternTestRoute route)
{
    // The 4 and 3 clues share candidates; the sole unique cell of the 4 is a mine:
    // 2 = = 1 =
    // M 4 3 1 =
    // 2 = = 0 =
    setBoard(5, 3, 4, QString::fromLatin1(
        R"(2 * * 1 0
           * 4 3 1 0
           2 * 1 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern34X);

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][0] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][2] = LibreMinesSolver::SolverCellState::Safe;
    for (uchar y = 0; y < solver.rows; ++y)
    {
        solver.solverBoard[3][y] = LibreMinesSolver::SolverCellState::Safe;
    }

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyMNXPatternLogic(1, 1, 2, 1, 3, 4)
        : solver.cleanAdvancedPatterns(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[2][2] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::generalPairRuleMarksUniqueCellsSafeViaDirectApply()
{
    runGeneralPairRuleOnlyASafe(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::generalPairRuleMarksUniqueCellsSafeViaCleanGeneralPairRule()
{
    runGeneralPairRuleOnlyASafe(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::generalPairRuleMarksUniqueCellsAsMinesViaDirectApply()
{
    runGeneralPairRuleOnlyAMine(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::generalPairRuleMarksUniqueCellsAsMinesViaCleanGeneralPairRule()
{
    runGeneralPairRuleOnlyAMine(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::generalPairRuleMarksOnlyBUniqueCellsViaDirectApply()
{
    runGeneralPairRuleOnlyBMine(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::generalPairRuleMarksOnlyBUniqueCellsViaCleanGeneralPairRule()
{
    runGeneralPairRuleOnlyBMine(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::generalPairRuleDiagonal11XMarksUniqueCellsSafeViaDirectApply()
{
    runGeneralPairRuleDiagonal11X(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::generalPairRuleDiagonal11XMarksUniqueCellsSafeViaCleanGeneralPairRule()
{
    runGeneralPairRuleDiagonal11X(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::runGeneralPairRuleOnlyASafe(PatternTestRoute route)
{
    // Both 2 clues need two more mines. Every unknown neighbor of the second clue is
    // shared with the first, so the first clue's unique cells must be safe:
    // S = = 0 =
    // S 2 2 0 =
    // S = = 0 =
    setBoard(5, 3, 2, QString::fromLatin1(
        R"(1 * 1 0 0
           2 2 2 0 0
           1 * 1 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::PairRule);

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][1] = LibreMinesSolver::SolverCellState::Safe;
    for (uchar y = 0; y < solver.rows; ++y)
    {
        solver.solverBoard[3][y] = LibreMinesSolver::SolverCellState::Safe;
    }

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyPairRule(1, 1, 2, 1)
        : solver.cleanGeneralPairRule(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Safe);
}

void TestLibreMinesSolver::runGeneralPairRuleOnlyAMine(PatternTestRoute route)
{
    // The 4 clue needs two more mines than the 2 clue and has exactly two unique
    // unknown neighbors, so they must both be mines:
    // M = = 1 =
    // 3 4 2 1 =
    // M = = 0 =
    setBoard(5, 3, 4, QString::fromLatin1(
        R"(* * * 1 0
           3 4 2 1 0
           * 1 0 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::PairRule);

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[0][1] = LibreMinesSolver::SolverCellState::Safe;
    for (uchar y = 0; y < solver.rows; ++y)
    {
        solver.solverBoard[3][y] = LibreMinesSolver::SolverCellState::Safe;
    }

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyPairRule(1, 1, 2, 1)
        : solver.cleanGeneralPairRule(1, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Mine);
}

void TestLibreMinesSolver::runGeneralPairRuleOnlyBMine(PatternTestRoute route)
{
    // The 3 clue needs two more mines than the 1 clue and has exactly two unique
    // unknown neighbors, so they must both be mines while the 1 clue's unique cells
    // stay safe:
    // S = = M =
    // S 1 3 2 =
    // S = = M =
    setBoard(5, 3, 3, QString::fromLatin1(
        R"(1 * 2 * 1
           1 1 3 2 2
           0 0 1 * 1)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::PairRule);

    solver.solverBoard[1][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[2][1] = LibreMinesSolver::SolverCellState::Safe;
    solver.solverBoard[3][1] = LibreMinesSolver::SolverCellState::Safe;

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyPairRule(2, 1, 1, 1)
        : solver.cleanGeneralPairRule(2, 1);
    QVERIFY(changed);
    QVERIFY(solver.solverBoard[3][0] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[3][2] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][2] == LibreMinesSolver::SolverCellState::Safe);
}

void TestLibreMinesSolver::runGeneralPairRuleDiagonal11X(PatternTestRoute route)
{
    // The two 1 clues are placed diagonally at (0, 0) and (1, 1).
    // Clue (0, 0) has unknown candidates {(1, 0), (0, 1)}, which are a subset of
    // clue (1, 1)'s unknown candidates.
    // A second mine at (3, 1) ensures all cells have value 1 so no zero-expansion occurs.
    // Neither basic nor advanced patterns check diagonal pairs, but the pair rule
    // resolves the candidates unique to (1, 1) as safe:
    // 1 = S =
    // = 1 S =
    // S S S =
    setBoard(4, 3, 2, QString::fromLatin1(
        R"(1 1 1 1
           * 1 1 *
           1 1 1 1)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::PairRule);

    solver.updateCellState({0, 0}, LibreMinesSolver::SolverCellState::Safe);
    solver.updateCellState({1, 1}, LibreMinesSolver::SolverCellState::Safe);

    // Basic and advanced pattern cleaners do not detect diagonal 1-1 pairs
    QVERIFY(!solver.cleanBasicPatterns(1, 1));
    QVERIFY(!solver.cleanAdvancedPatterns(1, 1));

    const bool changed = route == PatternTestRoute::DirectApply
        ? solver.applyPairRule(1, 1, 0, 0)
        : solver.cleanGeneralPairRule(1, 1);
    QVERIFY(changed);
    const QPair<uchar, uchar> safeCells[] = {
        {2, 0}, {2, 1}, {0, 2}, {1, 2}, {2, 2}
    };
    for (const QPair<uchar, uchar>& cell : safeCells)
    {
        QCOMPARE(solver.solverBoard[cell.first][cell.second],
                 LibreMinesSolver::SolverCellState::Safe);
    }
    QCOMPARE(solver.solverBoard[1][0], LibreMinesSolver::SolverCellState::Unknown);
    QCOMPARE(solver.solverBoard[0][1], LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::enumerationResolvesCellsAcrossConnectedClues()
{
    // The connected 1-2-1 clues leave (1, 0) and (1, 2) safe in every valid layout:
    // = S =
    // 1 2 1
    // = S =
    setBoard(3, 3, 2, QString::fromLatin1(
        R"(* 2 *
           1 2 1
           0 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Enumeration);

    solver.updateCellState({0, 1}, LibreMinesSolver::SolverCellState::Safe);
    solver.updateCellState({1, 1}, LibreMinesSolver::SolverCellState::Safe);
    solver.updateCellState({2, 1}, LibreMinesSolver::SolverCellState::Safe);

    QVERIFY(solver.cleanEnumeration());
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[1][2] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[2][0] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::enumerationRespectsMaxEnumerationCluesWindows()
{
    // The same 1-2-1 board: limiting one enumeration to a single clue breaks the
    // group into windows that each deduce nothing, so no cell is resolved until the
    // limit is lifted:
    // = = =
    // 1 2 1
    // = = =
    setBoard(3, 3, 2, QString::fromLatin1(
        R"(* 2 *
           1 2 1
           0 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Enumeration);
    solver.setMaxEnumerationClues(1);

    solver.updateCellState({0, 1}, LibreMinesSolver::SolverCellState::Safe);
    solver.updateCellState({1, 1}, LibreMinesSolver::SolverCellState::Safe);
    solver.updateCellState({2, 1}, LibreMinesSolver::SolverCellState::Safe);

    QVERIFY(!solver.cleanEnumeration());
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[1][2] == LibreMinesSolver::SolverCellState::Unknown);

    solver.setMaxEnumerationClues(0);

    // Lifting the limit restores the whole-group deduction:
    // = S =
    // 1 2 1
    // = S =
    QVERIFY(solver.cleanEnumeration());
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[1][2] == LibreMinesSolver::SolverCellState::Safe);
}

void TestLibreMinesSolver::enumerationResolvesThreeConnectedCluesWithoutPairRule()
{
    // Three connected clues (1-2-1) share 6 unknown candidates.
    // Mines at (0, 0) and (6, 0) are already flagged so outer candidates (0, 1) and (6, 1)
    // have non-zero clue values and do not trigger cascading zero-clearing.
    // Clue A at (1, 1) touches { (0, 1), (2, 0), (2, 1) } (needs 1 mine)
    // Clue B at (3, 1) touches { (2, 0), (2, 1), (4, 0), (4, 1) } (needs 2 mines)
    // Clue C at (5, 1) touches { (4, 0), (4, 1), (6, 1) } (needs 1 mine)
    // No pair of clues collapses via pair-rule or patterns, but enumerating all 3
    // forces (0, 1) and (6, 1) to be safe:
    // * 2 = 2 = 2 *
    // S 2 = 2 = 2 S
    setBoard(7, 2, 4, QString::fromLatin1(
        R"(* 2 * 2 * 2 *
           1 2 1 2 1 2 1)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Enumeration);

    solver.updateCellState({0, 0}, LibreMinesSolver::SolverCellState::Mine);
    solver.updateCellState({6, 0}, LibreMinesSolver::SolverCellState::Mine);

    const QPair<uchar, uchar> safeCells[] = {
        {1, 0}, {3, 0}, {5, 0},
        {1, 1}, {3, 1}, {5, 1}
    };
    for (const QPair<uchar, uchar>& cell : safeCells)
    {
        solver.updateCellState(cell, LibreMinesSolver::SolverCellState::Safe);
    }

    // Verify simpler strategies cannot make progress on these clues
    for (uchar x : {1, 3, 5})
    {
        QVERIFY(!solver.cleanBasicPatterns(x, 1));
        QVERIFY(!solver.cleanAdvancedPatterns(x, 1));
        QVERIFY(!solver.cleanGeneralPairRule(x, 1));
    }

    QVERIFY(solver.cleanEnumeration());
    QCOMPARE(solver.solverBoard[0][1], LibreMinesSolver::SolverCellState::Safe);
    QCOMPARE(solver.solverBoard[6][1], LibreMinesSolver::SolverCellState::Safe);
    QCOMPARE(solver.solverBoard[2][0], LibreMinesSolver::SolverCellState::Unknown);
    QCOMPARE(solver.solverBoard[2][1], LibreMinesSolver::SolverCellState::Unknown);
    QCOMPARE(solver.solverBoard[4][0], LibreMinesSolver::SolverCellState::Unknown);
    QCOMPARE(solver.solverBoard[4][1], LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::mineCountingMarksAllUnknownCellsSafeWhenNoMinesLeft()
{
    // Every mine is already flagged, so every remaining unknown cell must be safe:
    // * S S
    // S S S
    // S S S
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(* 1 0
           1 1 0
           0 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::MineCounting);
    solver.nMines = 1;
    solver.updateCellState({0, 0}, LibreMinesSolver::SolverCellState::Mine);

    QVERIFY(solver.cleanMineCounting());
    QCOMPARE(solver.nCellsMarkedSafe, quint32(8));
    QCOMPARE(solver.nCellsMarkedMine, quint32(1));
    for (uchar x = 0; x < solver.cols; ++x)
    {
        for (uchar y = 0; y < solver.rows; ++y)
        {
            const auto expected = (x == 0 && y == 0)
                ? LibreMinesSolver::SolverCellState::Mine
                : LibreMinesSolver::SolverCellState::Safe;
            QVERIFY(solver.solverBoard[x][y] == expected);
        }
    }
}

void TestLibreMinesSolver::mineCountingMarksAllUnknownCellsAsMinesWhenAllRemainingAreMines()
{
    // The two unknown cells are exactly the two mines still hidden:
    // M M 1
    // 2 2 1
    // 0 0 0
    setBoard(3, 3, 2, QString::fromLatin1(
        R"(* * 1
           2 2 1
           0 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::MineCounting);
    solver.nMines = 2;

    for (uchar x = 0; x < solver.cols; ++x)
    {
        for (uchar y = 0; y < solver.rows; ++y)
        {
            if ((x == 0 && y == 0) || (x == 1 && y == 0))
            {
                continue;
            }
            solver.solverBoard[x][y] = LibreMinesSolver::SolverCellState::Safe;
        }
    }
    solver.nCellsMarkedSafe = 7;

    QVERIFY(solver.cleanMineCounting());
    QVERIFY(solver.solverBoard[0][0] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Mine);
    QCOMPARE(solver.nCellsMarkedMine, quint32(2));
}

void TestLibreMinesSolver::mineCountingMarksFreeCellsSafeWhenGroupsUseAllMines()
{
    // The single clue must contain the only mine, so the cells that touch no clue
    // ("free" cells) cannot contain any and are all safe:
    // 1 = S
    // = = S
    // S S S
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(1 1 1
           1 * 1
           1 1 1)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::MineCounting);
    solver.nMines = 1;
    solver.updateCellState({0, 0}, LibreMinesSolver::SolverCellState::Safe);

    QVERIFY(solver.cleanMineCounting());

    const QPair<uchar, uchar> freeCells[] = {
        {2, 0}, {2, 1}, {0, 2}, {1, 2}, {2, 2}
    };
    for (const QPair<uchar, uchar>& cell : freeCells)
    {
        QVERIFY(solver.solverBoard[cell.first][cell.second] == LibreMinesSolver::SolverCellState::Safe);
    }
    QVERIFY(solver.solverBoard[1][0] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[0][1] == LibreMinesSolver::SolverCellState::Unknown);
    QVERIFY(solver.solverBoard[1][1] == LibreMinesSolver::SolverCellState::Unknown);
}

void TestLibreMinesSolver::mineCountingRejectsInconsistentMineCount()
{
    // More mines are flagged than the board contains, so the mine counter is
    // inconsistent and nothing may be deduced:
    // * * =
    // = = =
    // = = =
    setBoard(3, 3, 2, QString::fromLatin1(
        R"(* * 1
           2 2 1
           0 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::MineCounting);
    solver.nMines = 1;
    solver.updateCellState({0, 0}, LibreMinesSolver::SolverCellState::Mine);
    solver.updateCellState({1, 0}, LibreMinesSolver::SolverCellState::Mine);

    QTest::ignoreMessage(QtWarningMsg,
        "LibreMinesSolver: mine count is inconsistent with the solver board");
    QVERIFY(!solver.cleanMineCounting());
    QCOMPARE(solver.nCellsMarkedMine, quint32(2));
}

void TestLibreMinesSolver::mineCountingDoesNothingWhenStrategyDisabled()
{
    // The strategy is disabled, so the board must be left untouched:
    // * = =
    // = = =
    // = = =
    setBoard(3, 3, 1, QString::fromLatin1(
        R"(* 1 0
           1 1 0
           0 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::None);
    solver.nMines = 1;
    solver.updateCellState({0, 0}, LibreMinesSolver::SolverCellState::Mine);

    QVERIFY(!solver.cleanMineCounting());
    QCOMPARE(solver.nCellsMarkedSafe, quint32(0));
    QCOMPARE(solver.nCellsMarkedMine, quint32(1));
}

void TestLibreMinesSolver::mineCountingResolvesAmbiguousFiftyFiftyWithLastMine()
{
    // End-of-game scenario with 1 remaining mine:
    // Clues (0, 1) and (2, 1) touch unknown candidates (0, 0), (1, 0), (2, 0).
    // Locally this admits two layouts:
    //   - Layout 1: (1, 0) is mine, (0, 0) and (2, 0) are safe (1 mine)
    //   - Layout 2: (0, 0) and (2, 0) are mines, (1, 0) is safe (2 mines)
    // Neither patterns, pair-rule, nor enumeration alone can resolve the 50-50 choice.
    // Mine counting discards Layout 2 because only 1 mine remains, fixing the last mine:
    // S M S
    // 2 * 2
    // 1 1 1
    setBoard(3, 3, 2, QString::fromLatin1(
        R"(2 * 2
           2 * 2
           1 1 1)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::MineCounting);
    solver.nMines = 2;

    solver.updateCellState({1, 1}, LibreMinesSolver::SolverCellState::Mine);
    const QPair<uchar, uchar> safeCells[] = {
        {0, 1}, {2, 1},
        {0, 2}, {1, 2}, {2, 2}
    };
    for (const QPair<uchar, uchar>& cell : safeCells)
    {
        solver.updateCellState(cell, LibreMinesSolver::SolverCellState::Safe);
    }

    // Verify simpler strategies and enumeration alone cannot resolve this ambiguity
    QVERIFY(!solver.cleanBasicPatterns(0, 1));
    QVERIFY(!solver.cleanBasicPatterns(2, 1));
    QVERIFY(!solver.cleanAdvancedPatterns(0, 1));
    QVERIFY(!solver.cleanAdvancedPatterns(2, 1));
    QVERIFY(!solver.cleanGeneralPairRule(0, 1));
    QVERIFY(!solver.cleanGeneralPairRule(2, 1));
    QVERIFY(!solver.cleanEnumeration());

    QVERIFY(solver.cleanMineCounting());
    QCOMPARE(solver.solverBoard[1][0], LibreMinesSolver::SolverCellState::Mine);
    QCOMPARE(solver.solverBoard[0][0], LibreMinesSolver::SolverCellState::Safe);
    QCOMPARE(solver.solverBoard[2][0], LibreMinesSolver::SolverCellState::Safe);
    QCOMPARE(solver.nCellsMarkedMine, quint32(2));
    QCOMPARE(solver.nCellsMarkedSafe, quint32(7));
}

void TestLibreMinesSolver::basicPattern11XRejectsNonSubsetViaDirectApply()
{
    // Reopening (3, 0) breaks the subset relation; unique cells stay unknown:
    // = = = =
    // = 1 1 1
    // = = = 0
    setBoard(4, 3, 1, QString::fromLatin1(
        R"(0 1 * 1
           0 1 1 1
           0 0 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern11X);

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
    // = = = 0
    // = 2 1 0
    // 1 = = 0
    setBoard(4, 3, 2, QString::fromLatin1(
        R"(2 * 1 0
           * 2 1 0
           1 1 0 0)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern12X);

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

void TestLibreMinesSolver::basicPattern12XResolvesTwoSidedCandidatesViaDirectApply()
{
    runBasic12XTwoSidedCandidates(PatternTestRoute::DirectApply);
}

void TestLibreMinesSolver::basicPattern12XResolvesTwoSidedCandidatesViaCleanBasicPatterns()
{
    runBasic12XTwoSidedCandidates(PatternTestRoute::Cleaner);
}

void TestLibreMinesSolver::runBasic12XTwoSidedCandidates(PatternTestRoute route)
{
    // The two 2 clues share the 1; each pair has one unique candidate on both sides.
    // Known mines reduce the 3 and 5 clues to 2 remaining mines each:
    // 2 * * M 1
    // 2 * 5 S 2
    // 1 1 2 = 1
    // 1 1 3 S 2
    // 1 * 2 M 1
    // 1 1 2 1 1
    setBoard(5, 6, 7, QString::fromLatin1(
        R"(2***1
           2*532
           112*1
           11322
           1*2*1
           11211)"));
    solver.setStrategies(LibreMinesSolver::SolverStrategy::Pattern12X);

    const QPair<uchar, uchar> unknownCells[] = {
        {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}
    };
    for (uchar x = 0; x < solver.cols; ++x)
    {
        for (uchar y = 0; y < solver.rows; ++y)
        {
            const QPair<uchar, uchar> cell(x, y);
            bool isUnknown = false;
            for (const QPair<uchar, uchar>& unknownCell : unknownCells)
            {
                if (cell == unknownCell)
                {
                    isUnknown = true;
                    break;
                }
            }

            if (isUnknown)
            {
                continue;
            }

            solver.updateCellState(cell,
                (x == 1 && (y == 0 || y == 1 || y == 4)) ||
                (x == 2 && y == 0)
                    ? LibreMinesSolver::SolverCellState::Mine
                    : LibreMinesSolver::SolverCellState::Safe);
        }
    }

    const bool upperChanged = route == PatternTestRoute::DirectApply
        ? solver.applyMNXPatternLogic(2, 3, 2, 2, 1, 2)
        : solver.cleanBasicPatterns(2, 3);
    QVERIFY(upperChanged);
    QVERIFY(solver.solverBoard[3][4] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[3][1] == LibreMinesSolver::SolverCellState::Safe);

    const bool lowerChanged = route == PatternTestRoute::DirectApply
        ? solver.applyMNXPatternLogic(2, 1, 2, 2, 1, 2)
        : solver.cleanBasicPatterns(2, 1);
    QVERIFY(lowerChanged);
    QVERIFY(solver.solverBoard[3][0] == LibreMinesSolver::SolverCellState::Mine);
    QVERIFY(solver.solverBoard[3][3] == LibreMinesSolver::SolverCellState::Safe);
    QVERIFY(solver.solverBoard[3][2] == LibreMinesSolver::SolverCellState::Unknown);
}

QTEST_GUILESS_MAIN(TestLibreMinesSolver)
#include "test_solver.moc"
