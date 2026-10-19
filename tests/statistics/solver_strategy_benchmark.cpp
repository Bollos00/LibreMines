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
 * @file solver_strategy_benchmark.cpp
 * @brief Solvability rate and solve time of each solver strategy combination
 * @author Bruno Bollos Correa
 * @date 2026-10-18
 *
 * Optional tool (built with -DBUILD_SOLVER_STATISTICS=ON) that measures how
 * often each solver strategy combination solves a board and how long it takes.
 *
 * For fairness, one fixed set of boards per difficulty is generated once
 * (random first-click cell) and every strategy is run against that exact same
 * set, so the differences come only from the strategy. Work is distributed
 * across worker threads; each thread owns its own SolverSolveHarness.
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

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <numeric>
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
 * @brief A pre-generated board together with its first-click cell
 */
struct StoredBoard
{
    std::vector<std::vector<LibreMinesGameEngine::CellGameEngine>> board;
    uchar safeX = 0;
    uchar safeY = 0;
};

/**
 * @brief Result of solving one stored board with one strategy
 */
struct Outcome
{
    bool solvable = false;
    double elapsedMs = 0.0;
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

/**
 * @brief Linear-interpolated percentile of an already sorted vector
 *
 * @param sorted Ascending values
 * @param p Fraction in [0, 1]
 * @return Interpolated percentile
 */
double percentile(const std::vector<double>& sorted, double p)
{
    if (sorted.empty())
    {
        return 0.0;
    }
    if (sorted.size() == 1)
    {
        return sorted.front();
    }

    const double position = p * (sorted.size() - 1);
    const size_t low = static_cast<size_t>(std::floor(position));
    const size_t high = static_cast<size_t>(std::ceil(position));
    if (low == high)
    {
        return sorted[low];
    }

    const double fraction = position - low;
    return sorted[low] * (1.0 - fraction) + sorted[high] * fraction;
}

} // namespace

int main(int argc, char** argv)
{
    benchmarkUseDeterministicHashSeed();

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("solver_strategy_benchmark"));
    QCoreApplication::setApplicationVersion(QStringLiteral(LIBREMINES_PROJECT_VERSION));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Measures the solvability rate and solve time of every "
                       "solver strategy combination and writes a JSON report."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption boardsOption(
        { QStringLiteral("b"), QStringLiteral("boards") },
        QStringLiteral("Number of boards generated per difficulty."),
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
        QStringLiteral("path"), QStringLiteral("solver_strategy_benchmark.json"));
    QCommandLineOption difficultiesOption(
        { QStringLiteral("d"), QStringLiteral("difficulties") },
        QStringLiteral("Comma separated list of difficulties (easy, medium, hard)."),
        QStringLiteral("list"), QStringLiteral("easy,medium,hard"));

    parser.addOptions({ boardsOption, threadsOption, seedOption, outputOption,
                        difficultiesOption });
    parser.process(app);

    // --- Validate the numeric options ---------------------------------------
    bool ok = false;
    const int boardsPerStrategy = parser.value(boardsOption).toInt(&ok);
    if (!ok || boardsPerStrategy <= 0)
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

    const quint32 seed = parser.value(seedOption).toUInt(&ok);
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

    const int nDifficulties = selectedDifficulties.size();
    const int nStrategies = allStrategies.size();

    // --- Generate the shared board set for every difficulty -----------------
    std::vector<std::vector<StoredBoard>> boardSets(nDifficulties);

    for (int d = 0; d < nDifficulties; ++d)
    {
        const BenchmarkDifficulty& diff = allDifficulties[selectedDifficulties[d]];
        std::mt19937 rng(benchmarkCombineSeed(
            seed, static_cast<quint32>(selectedDifficulties[d]), 0xB0A2D5E1u));
        std::uniform_int_distribution<int> distX(0, static_cast<int>(diff.cols) - 1);
        std::uniform_int_distribution<int> distY(0, static_cast<int>(diff.rows) - 1);

        boardSets[d].reserve(boardsPerStrategy);
        for (int b = 0; b < boardsPerStrategy; ++b)
        {
            StoredBoard stored;
            stored.safeX = static_cast<uchar>(distX(rng));
            stored.safeY = static_cast<uchar>(distY(rng));
            stored.board = DeterministicBoardGenerator::generate(
                rng, diff.cols, diff.rows, diff.mines, stored.safeX, stored.safeY);
            boardSets[d].push_back(std::move(stored));
        }
    }

    // --- Flatten all (difficulty, strategy, board) tasks --------------------
    const size_t boardsPerDifficulty =
        static_cast<size_t>(nStrategies) * boardsPerStrategy;
    const size_t totalTasks = static_cast<size_t>(nDifficulties) * boardsPerDifficulty;
    std::vector<Outcome> outcomes(totalTasks);

    int requestedThreads = threadCount;
    if (requestedThreads <= 0)
    {
        requestedThreads = static_cast<int>(std::thread::hardware_concurrency());
    }
    if (requestedThreads <= 0)
    {
        requestedThreads = 1;
    }
    if (static_cast<size_t>(requestedThreads) > totalTasks)
    {
        requestedThreads = static_cast<int>(totalTasks);
    }

    std::printf("Testing %d strategies on %d difficulty(ies) x %d boards "
                "(%zu solves) with %d thread(s)...\n",
                nStrategies, nDifficulties, boardsPerStrategy, totalTasks,
                requestedThreads);

    // --- Run the workers ----------------------------------------------------
    std::atomic<size_t> nextIndex(0);
    std::atomic<size_t> completed(0);
    const size_t progressStep = std::max<size_t>(1, totalTasks / 100);

    const auto worker = [&]()
    {
        SolverSolveHarness harness;

        while (true)
        {
            const size_t index = nextIndex.fetch_add(1);
            if (index >= totalTasks)
            {
                break;
            }

            const size_t difficultySlot = index / boardsPerDifficulty;
            const size_t remainder = index % boardsPerDifficulty;
            const size_t strategySlot = remainder / boardsPerStrategy;
            const size_t boardSlot = remainder % boardsPerStrategy;

            const BenchmarkDifficulty& diff =
                allDifficulties[selectedDifficulties[static_cast<int>(difficultySlot)]];
            const BenchmarkStrategyConfig& config = allStrategies[static_cast<int>(strategySlot)];
            const StoredBoard& stored = boardSets[difficultySlot][boardSlot];

            const auto start = std::chrono::steady_clock::now();
            const bool solvable = harness.solve(
                stored.board, diff.cols, diff.rows, diff.mines,
                stored.safeX, stored.safeY,
                static_cast<LibreMinesSolver::SolverStrategy>(config.strategies),
                config.maxEnumerationClues);
            const auto end = std::chrono::steady_clock::now();

            outcomes[index].solvable = solvable;
            outcomes[index].elapsedMs =
                std::chrono::duration<double, std::milli>(end - start).count();

            const size_t done = completed.fetch_add(1) + 1;
            if (done % progressStep == 0 || done == totalTasks)
            {
                std::fprintf(stderr, "\rProgress: %zu/%zu (%.1f%%)",
                             done, totalTasks, 100.0 * done / totalTasks);
            }
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
    std::fprintf(stderr, "\n");

    // --- Aggregate the outcomes --------------------------------------------
    std::vector<std::vector<quint64>> solvableCount(
        nDifficulties, std::vector<quint64>(nStrategies, 0));
    std::vector<std::vector<std::vector<double>>> times(
        nDifficulties, std::vector<std::vector<double>>(nStrategies));

    for (size_t index = 0; index < totalTasks; ++index)
    {
        const size_t difficultySlot = index / boardsPerDifficulty;
        const size_t strategySlot = (index % boardsPerDifficulty) / boardsPerStrategy;

        if (outcomes[index].solvable)
        {
            ++solvableCount[difficultySlot][strategySlot];
        }
        times[difficultySlot][strategySlot].push_back(outcomes[index].elapsedMs);
    }

    // --- Build the JSON report ----------------------------------------------
    QJsonArray strategiesJson;
    for (const BenchmarkStrategyConfig& config : allStrategies)
    {
        QJsonObject strategyJson;
        strategyJson.insert(QStringLiteral("id"), QString::fromLatin1(config.id));
        strategyJson.insert(QStringLiteral("label"), QString::fromLatin1(config.label));
        strategyJson.insert(QStringLiteral("strategiesMask"),
                            static_cast<qint64>(config.strategies));
        strategyJson.insert(QStringLiteral("maxEnumerationClues"),
                            config.maxEnumerationClues);
        strategiesJson.append(strategyJson);
    }

    QJsonArray difficultiesJson;
    for (int d = 0; d < nDifficulties; ++d)
    {
        const BenchmarkDifficulty& diff = allDifficulties[selectedDifficulties[d]];
        QJsonObject difficultyJson;
        difficultyJson.insert(QStringLiteral("name"), QString::fromLatin1(diff.name));
        difficultyJson.insert(QStringLiteral("cols"), static_cast<int>(diff.cols));
        difficultyJson.insert(QStringLiteral("rows"), static_cast<int>(diff.rows));
        difficultyJson.insert(QStringLiteral("mines"), static_cast<int>(diff.mines));
        difficultiesJson.append(difficultyJson);
    }

    QJsonArray resultsJson;
    std::printf("\n%-6s %-38s %10s %8s %12s %12s\n",
                "Level", "Strategy", "Solvable", "Rate", "Total (ms)", "Avg (ms)");
    for (int d = 0; d < nDifficulties; ++d)
    {
        const BenchmarkDifficulty& diff = allDifficulties[selectedDifficulties[d]];

        for (int s = 0; s < nStrategies; ++s)
        {
            const BenchmarkStrategyConfig& config = allStrategies[s];
            std::vector<double>& strategyTimes = times[d][s];
            std::sort(strategyTimes.begin(), strategyTimes.end());

            const quint64 solvable = solvableCount[d][s];
            const double totalMs =
                std::accumulate(strategyTimes.begin(), strategyTimes.end(), 0.0);
            const double avgMs = strategyTimes.empty()
                ? 0.0
                : totalMs / strategyTimes.size();

            QJsonObject timing;
            timing.insert(QStringLiteral("totalMs"), totalMs);
            timing.insert(QStringLiteral("avgMs"), avgMs);
            timing.insert(QStringLiteral("minMs"),
                          strategyTimes.empty() ? 0.0 : strategyTimes.front());
            timing.insert(QStringLiteral("maxMs"),
                          strategyTimes.empty() ? 0.0 : strategyTimes.back());
            timing.insert(QStringLiteral("medianMs"), percentile(strategyTimes, 0.5));
            timing.insert(QStringLiteral("p95Ms"), percentile(strategyTimes, 0.95));

            QJsonObject result;
            result.insert(QStringLiteral("difficulty"), QString::fromLatin1(diff.name));
            result.insert(QStringLiteral("strategyId"), QString::fromLatin1(config.id));
            result.insert(QStringLiteral("boards"), boardsPerStrategy);
            result.insert(QStringLiteral("solvable"), static_cast<qint64>(solvable));
            result.insert(QStringLiteral("solvabilityRate"),
                          static_cast<double>(solvable) / boardsPerStrategy);
            result.insert(QStringLiteral("timing"), timing);
            resultsJson.append(result);

            std::printf("%-6s %-38s %10lld %7.2f%% %12.1f %12.4f\n",
                        diff.name, config.id,
                        static_cast<long long>(solvable),
                        100.0 * solvable / boardsPerStrategy,
                        totalMs, avgMs);
        }
    }

    QJsonObject root;
    root.insert(QStringLiteral("tool"), QStringLiteral("solver_strategy_benchmark"));
    root.insert(QStringLiteral("generatedAt"),
                QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    root.insert(QStringLiteral("seed"), static_cast<qint64>(seed));
    root.insert(QStringLiteral("boardsPerStrategy"), boardsPerStrategy);
    root.insert(QStringLiteral("threads"), requestedThreads);
    root.insert(QStringLiteral("startingPoint"), QStringLiteral("random-clean"));
    root.insert(QStringLiteral("strategies"), strategiesJson);
    root.insert(QStringLiteral("difficulties"), difficultiesJson);
    root.insert(QStringLiteral("results"), resultsJson);

    const QString outputPath = parser.value(outputOption);
    if (!writeJsonFile(outputPath, root))
    {
        return 1;
    }

    std::printf("Wrote %s\n", qPrintable(outputPath));
    return 0;
}
