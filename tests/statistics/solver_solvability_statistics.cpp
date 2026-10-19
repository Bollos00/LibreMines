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

/**
 * @file solver_solvability_statistics.cpp
 * @brief Percentage of solvable boards from every possible starting point
 * @author Bruno Bollos Correa
 * @date 2026-10-18
 *
 * Optional tool (built with -DBUILD_SOLVER_STATISTICS=ON) that, for every
 * possible starting cell of the Easy, Medium and Hard boards, generates a
 * configurable number of boards with that cell as the first-click cell and
 * counts how many of them the solver can solve without guessing.
 *
 * The work over starting cells is spread across worker threads, each with its
 * own SolverSolveHarness and deterministic random stream. The result is a JSON
 * report that carries both the overall solvability of each level and the
 * per-starting-point rates, ready to be rendered as a heatmap.
 *
 * @since v3.0.0
 */

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <atomic>
#include <cstdio>
#include <random>
#include <thread>
#include <vector>

#include "benchmarkcommon.h"
#include "deterministicboardgenerator.h"
#include "libreminesconfig.h"
#include "solversolveharness.h"

namespace
{

/**
 * @brief One (difficulty, starting cell) unit of work
 */
struct WorkItem
{
    int difficulty; ///< Slot in the selected difficulty list (not the global index)
    uchar x;        ///< Starting cell X coordinate
    uchar y;        ///< Starting cell Y coordinate
};

/**
 * @brief Write a JSON document to disk, creating parent directories
 *
 * @param path Destination file path
 * @param root Document root
 * @return true on success
 */
bool writeJsonFile(const QString& path, const QJsonObject& root)
{
    const QFileInfo info(path);
    if (!info.absolutePath().isEmpty())
    {
        QDir().mkpath(info.absolutePath());
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        std::fprintf(stderr, "Error: could not open '%s' for writing\n",
                     qPrintable(path));
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

} // namespace

int main(int argc, char** argv)
{
    benchmarkUseDeterministicHashSeed();

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("solver_solvability_statistics"));
    QCoreApplication::setApplicationVersion(QStringLiteral(LIBREMINES_PROJECT_VERSION));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Measures the percentage of solvable boards from every "
                       "possible starting point and writes a JSON report."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption boardsOption(
        { QStringLiteral("b"), QStringLiteral("boards") },
        QStringLiteral("Number of boards generated per starting point."),
        QStringLiteral("count"), QStringLiteral("1000"));
    QCommandLineOption threadsOption(
        { QStringLiteral("t"), QStringLiteral("threads") },
        QStringLiteral("Number of worker threads (0 = number of CPU cores)."),
        QStringLiteral("count"), QStringLiteral("0"));
    QCommandLineOption seedOption(
        { QStringLiteral("s"), QStringLiteral("seed") },
        QStringLiteral("Master seed for reproducible runs."),
        QStringLiteral("seed"), QStringLiteral("20260101"));
    QCommandLineOption outputOption(
        { QStringLiteral("o"), QStringLiteral("output") },
        QStringLiteral("Destination JSON file."),
        QStringLiteral("path"), QStringLiteral("solver_solvability_by_start.json"));
    QCommandLineOption difficultiesOption(
        { QStringLiteral("d"), QStringLiteral("difficulties") },
        QStringLiteral("Comma separated list of difficulties (easy, medium, hard)."),
        QStringLiteral("list"), QStringLiteral("easy,medium,hard"));
    QCommandLineOption strategyOption(
        { QStringLiteral("strategy") },
        QStringLiteral("Strategy configuration used for every board."),
        QStringLiteral("id"), QStringLiteral("all_strategies"));

    parser.addOptions({ boardsOption, threadsOption, seedOption, outputOption,
                        difficultiesOption, strategyOption });
    parser.process(app);

    // --- Validate the numeric options ---------------------------------------
    bool ok = false;
    const int boardsPerStart = parser.value(boardsOption).toInt(&ok);
    if (!ok || boardsPerStart <= 0)
    {
        std::fprintf(stderr, "Error: --boards must be a positive integer\n");
        return 2;
    }

    int threadCount = parser.value(threadsOption).toInt(&ok);
    if (!ok || threadCount < 0)
    {
        std::fprintf(stderr, "Error: --threads must be a non-negative integer\n");
        return 2;
    }

    quint32 seed = parser.value(seedOption).toUInt(&ok);
    if (!ok)
    {
        std::fprintf(stderr, "Error: --seed must be an unsigned integer\n");
        return 2;
    }

    const QVector<BenchmarkDifficulty> allDifficulties = benchmarkDifficulties();
    const QVector<BenchmarkStrategyConfig> allStrategies = benchmarkStrategyConfigs();

    // --- Resolve the difficulties -------------------------------------------
    QVector<int> selectedDifficulties;
    for (const QString& token :
         parser.value(difficultiesOption).split(QLatin1Char(','), Qt::SkipEmptyParts))
    {
        bool found = false;
        const int index = benchmarkDifficultyIndex(token, allDifficulties, &found);
        if (!found)
        {
            std::fprintf(stderr, "Error: unknown difficulty '%s'\n", qPrintable(token.trimmed()));
            return 2;
        }
        if (!selectedDifficulties.contains(index))
        {
            selectedDifficulties.append(index);
        }
    }

    if (selectedDifficulties.isEmpty())
    {
        std::fprintf(stderr, "Error: no difficulty selected\n");
        return 2;
    }

    // --- Resolve the strategy -----------------------------------------------
    bool strategyFound = false;
    const int strategyIndex =
        benchmarkStrategyIndex(parser.value(strategyOption), allStrategies, &strategyFound);
    if (!strategyFound)
    {
        std::fprintf(stderr, "Error: unknown strategy '%s'\n",
                     qPrintable(parser.value(strategyOption).trimmed()));
        return 2;
    }
    const BenchmarkStrategyConfig& strategyConfig = allStrategies[strategyIndex];
    const LibreMinesSolver::SolverStrategy strategy =
        static_cast<LibreMinesSolver::SolverStrategy>(strategyConfig.strategies);

    // --- Build the list of starting cells -----------------------------------
    std::vector<WorkItem> items;
    for (int d = 0; d < selectedDifficulties.size(); ++d)
    {
        const BenchmarkDifficulty& diff = allDifficulties[selectedDifficulties[d]];
        for (uchar x = 0; x < diff.cols; ++x)
        {
            for (uchar y = 0; y < diff.rows; ++y)
            {
                items.push_back({ d, x, y });
            }
        }
    }

    // results[difficulty][x * rows + y] = number of solvable boards
    std::vector<std::vector<quint32>> results(selectedDifficulties.size());
    for (int d = 0; d < selectedDifficulties.size(); ++d)
    {
        const BenchmarkDifficulty& diff = allDifficulties[selectedDifficulties[d]];
        results[d].assign(static_cast<size_t>(diff.cols) * diff.rows, 0);
    }

    int requestedThreads = threadCount;
    if (requestedThreads <= 0)
    {
        requestedThreads = static_cast<int>(std::thread::hardware_concurrency());
    }
    if (requestedThreads <= 0)
    {
        requestedThreads = 1;
    }
    if (static_cast<size_t>(requestedThreads) > items.size())
    {
        requestedThreads = static_cast<int>(items.size());
    }

    std::printf("Solving %zu starting points x %d boards with %d thread(s)...\n",
                items.size(), boardsPerStart, requestedThreads);

    // --- Run the workers ----------------------------------------------------
    std::atomic<size_t> nextIndex(0);
    std::atomic<size_t> completed(0);

    const auto worker = [&]()
    {
        SolverSolveHarness harness;

        while (true)
        {
            const size_t index = nextIndex.fetch_add(1);
            if (index >= items.size())
            {
                break;
            }

            const WorkItem& item = items[index];
            const BenchmarkDifficulty& diff =
                allDifficulties[selectedDifficulties[item.difficulty]];

            // One deterministic stream per starting cell, so the results do not
            // depend on the order in which threads pick up the cells. The seed
            // uses the level itself (not its position in the selection), so a
            // level gives the same numbers whichever other levels are selected.
            std::mt19937 rng(benchmarkCombineSeed(
                seed,
                static_cast<quint32>(selectedDifficulties[item.difficulty]),
                static_cast<quint32>(item.x) * 256u + static_cast<quint32>(item.y)));

            quint32 solvable = 0;
            for (int b = 0; b < boardsPerStart; ++b)
            {
                const auto board = DeterministicBoardGenerator::generate(
                    rng, diff.cols, diff.rows, diff.mines, item.x, item.y);

                if (harness.solve(board, diff.cols, diff.rows, diff.mines,
                                  item.x, item.y, strategy,
                                  strategyConfig.maxEnumerationClues))
                {
                    ++solvable;
                }
            }

            results[static_cast<size_t>(item.difficulty)]
                   [static_cast<size_t>(item.x) * diff.rows + item.y] = solvable;

            const size_t done = completed.fetch_add(1) + 1;
            std::fprintf(stderr, "[%zu/%zu] %s (%u,%u): %u/%d solvable (%.2f%%)\n",
                         done, items.size(), diff.name, item.x, item.y,
                         solvable, boardsPerStart,
                         100.0 * solvable / boardsPerStart);
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(requestedThreads);
    for (int t = 0; t < requestedThreads; ++t)
    {
        threads.emplace_back(worker);
    }
    for (std::thread& thread : threads)
    {
        thread.join();
    }

    // --- Build the JSON report ----------------------------------------------
    QJsonArray difficultiesJson;
    for (int d = 0; d < selectedDifficulties.size(); ++d)
    {
        const BenchmarkDifficulty& diff = allDifficulties[selectedDifficulties[d]];
        const std::vector<quint32>& cellResults = results[d];

        quint64 totalSolvable = 0;
        QJsonArray byStartingPoint;
        QJsonArray rateMatrix;

        for (uchar x = 0; x < diff.cols; ++x)
        {
            QJsonArray column;
            for (uchar y = 0; y < diff.rows; ++y)
            {
                const quint32 solvable =
                    cellResults[static_cast<size_t>(x) * diff.rows + y];
                totalSolvable += solvable;

                QJsonObject cell;
                cell.insert(QStringLiteral("x"), static_cast<int>(x));
                cell.insert(QStringLiteral("y"), static_cast<int>(y));
                cell.insert(QStringLiteral("boards"), boardsPerStart);
                cell.insert(QStringLiteral("solvable"), static_cast<qint64>(solvable));
                cell.insert(QStringLiteral("rate"),
                            static_cast<double>(solvable) / boardsPerStart);
                byStartingPoint.append(cell);

                column.append(static_cast<double>(solvable) / boardsPerStart);
            }
            rateMatrix.append(column);
        }

        const quint64 boardsTested =
            static_cast<quint64>(diff.cols) * diff.rows * boardsPerStart;

        QJsonObject difficultyJson;
        difficultyJson.insert(QStringLiteral("name"), QString::fromLatin1(diff.name));
        difficultyJson.insert(QStringLiteral("cols"), static_cast<int>(diff.cols));
        difficultyJson.insert(QStringLiteral("rows"), static_cast<int>(diff.rows));
        difficultyJson.insert(QStringLiteral("mines"), static_cast<int>(diff.mines));
        difficultyJson.insert(QStringLiteral("cells"),
                              static_cast<int>(diff.cols) * diff.rows);
        difficultyJson.insert(QStringLiteral("boardsTested"), static_cast<qint64>(boardsTested));
        difficultyJson.insert(QStringLiteral("solvable"), static_cast<qint64>(totalSolvable));
        difficultyJson.insert(QStringLiteral("solvabilityRate"),
                              static_cast<double>(totalSolvable) / boardsTested);
        difficultyJson.insert(QStringLiteral("byStartingPoint"), byStartingPoint);
        difficultyJson.insert(QStringLiteral("rateMatrixIndexing"), QStringLiteral("[x][y]"));
        difficultyJson.insert(QStringLiteral("rateMatrix"), rateMatrix);

        difficultiesJson.append(difficultyJson);

        std::printf("%-6s %2ux%-2u, %3d mines: %lld/%lld solvable (%.2f%%)\n",
                    diff.name, diff.cols, diff.rows, diff.mines,
                    static_cast<long long>(totalSolvable),
                    static_cast<long long>(boardsTested),
                    100.0 * totalSolvable / boardsTested);
    }

    QJsonObject root;
    root.insert(QStringLiteral("tool"), QStringLiteral("solver_solvability_statistics"));
    root.insert(QStringLiteral("generatedAt"),
                QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    root.insert(QStringLiteral("seed"), static_cast<qint64>(seed));
    root.insert(QStringLiteral("boardsPerStartingPoint"), boardsPerStart);
    root.insert(QStringLiteral("threads"), requestedThreads);
    root.insert(QStringLiteral("strategy"), QString::fromLatin1(strategyConfig.id));
    root.insert(QStringLiteral("strategyLabel"), QString::fromLatin1(strategyConfig.label));
    root.insert(QStringLiteral("strategyMask"), static_cast<qint64>(strategyConfig.strategies));
    root.insert(QStringLiteral("maxEnumerationClues"), strategyConfig.maxEnumerationClues);
    root.insert(QStringLiteral("difficulties"), difficultiesJson);

    const QString outputPath = parser.value(outputOption);
    if (!writeJsonFile(outputPath, root))
    {
        return 1;
    }

    std::printf("Wrote %s\n", qPrintable(outputPath));
    return 0;
}
