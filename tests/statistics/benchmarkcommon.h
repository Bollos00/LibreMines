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
 * @file benchmarkcommon.h
 * @brief Shared definitions for the optional solver statistics tools
 * @author Bruno Bollos Correa
 * @date 2026-10-18
 *
 * This header defines the difficulty presets and the solver strategy
 * configurations used by the optional statistics/benchmark executables in
 * tests/statistics/. Keeping them in one place guarantees that the
 * per-starting-point solvability tool and the strategy comparison tool report
 * the exact same levels and strategy combinations.
 *
 * @since v3.0.0
 */

#ifndef BENCHMARKCOMMON_H
#define BENCHMARKCOMMON_H

#include "libreminessolver.h"

#include <QHash>
#include <QString>
#include <QVector>

/**
 * @brief Board parameters of one game difficulty
 *
 * The dimensions and mine counts match the values used by LibreMinesGui for
 * the Easy, Medium and Hard menu entries.
 *
 * @since v3.0.0
 */
struct BenchmarkDifficulty
{
    const char* name;   ///< Display name ("Easy", "Medium", "Hard")
    uchar cols;         ///< Board width (number of columns)
    uchar rows;         ///< Board height (number of rows)
    short mines;        ///< Total number of mines on the board
};

/**
 * @brief One solver strategy combination to benchmark
 *
 * @since v3.0.0
 */
struct BenchmarkStrategyConfig
{
    const char* id;             ///< Stable identifier used on the command line and in JSON
    const char* label;          ///< Human readable description
    quint64 strategies;         ///< Bit mask of LibreMinesSolver::SolverStrategy values
    int maxEnumerationClues;    ///< Clue limit used by the enumeration strategy (0 = unlimited)
};

/**
 * @brief The three standard difficulties used by the statistics tools
 * @return Vector with Easy, Medium and Hard in that order
 */
inline QVector<BenchmarkDifficulty> benchmarkDifficulties()
{
    return {
        { "Easy",   8,  8,  10 },
        { "Medium", 16, 16, 40 },
        { "Hard",   30, 16, 99 }
    };
}

/**
 * @brief The ordered list of strategy combinations tested by the benchmark tool
 *
 * The order defines the "strategies" array in the JSON report.
 *
 * @return Vector with every supported strategy configuration
 */
inline QVector<BenchmarkStrategyConfig> benchmarkStrategyConfigs()
{
    const quint64 none          = static_cast<quint64>(LibreMinesSolver::SolverStrategy::None);
    const quint64 basic         = static_cast<quint64>(LibreMinesSolver::SolverStrategy::BasicPatterns);
    const quint64 basicAdvanced = static_cast<quint64>(LibreMinesSolver::SolverStrategy::BasicAndAdvancedPatterns);
    const quint64 pairRule      = static_cast<quint64>(LibreMinesSolver::SolverStrategy::PairRule);
    const quint64 enumeration   = static_cast<quint64>(LibreMinesSolver::SolverStrategy::Enumeration);
    const quint64 all           = static_cast<quint64>(LibreMinesSolver::SolverStrategy::AllStrategies);

    return {
        { "none",
          "No strategy (basic and zero clearing only)",
          none, 8 },
        { "basic_patterns",
          "Basic patterns",
          basic, 8 },
        { "basic_advanced",
          "Basic and advanced patterns",
          basicAdvanced, 8 },
        { "pair_rule",
          "Pair rule only",
          pairRule, 8 },
        { "basic_advanced_pair",
          "Basic, advanced and pair rule",
          basicAdvanced | pairRule, 8 },
        { "enumeration_3",
          "Enumeration (max clues = 3) only",
          enumeration, 3 },
        { "enumeration_8",
          "Enumeration (max clues = 8) only",
          enumeration, 8 },
        { "enumeration_unlimited",
          "Enumeration (max clues = unlimited) only",
          enumeration, 0 },
        { "basic_advanced_pair_enum3",
          "Basic, advanced, pair rule and enumeration (max clues = 3)",
          basicAdvanced | pairRule | enumeration, 3 },
        { "basic_advanced_pair_enum8",
          "Basic, advanced, pair rule and enumeration (max clues = 8)",
          basicAdvanced | pairRule | enumeration, 8 },
        { "basic_advanced_pair_enum_unlimited",
          "Basic, advanced, pair rule and enumeration (max clues = unlimited)",
          basicAdvanced | pairRule | enumeration, 0 },
        { "all_strategies",
          "All strategies (basic, advanced, pair rule, enumeration max clues = 8 and mine counting)",
          all, 8 }
    };
}

/**
 * @brief Resolve a difficulty name or index to its position in the list
 *
 * Accepts either the case-insensitive name ("easy") or the 0-based index ("0").
 *
 * @param token Text supplied on the command line
 * @param difficulties Difficulty list to search
 * @param[out] ok Set to true when the token was resolved
 * @return Index in the list, or -1 when the token is unknown
 */
inline int benchmarkDifficultyIndex(const QString& token,
                                    const QVector<BenchmarkDifficulty>& difficulties,
                                    bool* ok)
{
    const QString trimmed = token.trimmed();
    bool intOk = false;
    const int asInt = trimmed.toInt(&intOk);
    if (intOk && asInt >= 0 && asInt < difficulties.size())
    {
        if (ok) *ok = true;
        return asInt;
    }

    for (int i = 0; i < difficulties.size(); ++i)
    {
        if (QString::fromLatin1(difficulties[i].name).compare(trimmed, Qt::CaseInsensitive) == 0)
        {
            if (ok) *ok = true;
            return i;
        }
    }

    if (ok) *ok = false;
    return -1;
}

/**
 * @brief Resolve a strategy id to its position in the configuration list
 *
 * @param token Strategy id supplied on the command line
 * @param configs Strategy list to search
 * @param[out] ok Set to true when the token was resolved
 * @return Index in the list, or -1 when the token is unknown
 */
inline int benchmarkStrategyIndex(const QString& token,
                                  const QVector<BenchmarkStrategyConfig>& configs,
                                  bool* ok)
{
    const QString trimmed = token.trimmed();
    for (int i = 0; i < configs.size(); ++i)
    {
        if (QString::fromLatin1(configs[i].id).compare(trimmed, Qt::CaseInsensitive) == 0)
        {
            if (ok) *ok = true;
            return i;
        }
    }

    if (ok) *ok = false;
    return -1;
}

/**
 * @brief Derive a reproducible 32-bit seed from three values
 *
 * Uses the FNV-1a hash so every (difficulty, starting cell, ...) combination
 * gets its own deterministic stream regardless of the order in which the
 * worker threads pick up their tasks.
 *
 * @param a First value
 * @param b Second value
 * @param c Third value
 * @return Deterministic 32-bit seed
 */
inline quint32 benchmarkCombineSeed(quint32 a, quint32 b, quint32 c)
{
    quint64 hash = 1469598103934665603ULL;
    const quint64 prime = 1099511628211ULL;
    const auto mix = [&hash, prime](quint64 value)
    {
        hash ^= value;
        hash *= prime;
    };

    mix(a);
    mix(b);
    mix(c);

    return static_cast<quint32>(hash ^ (hash >> 32));
}

/**
 * @brief Force a deterministic QHash seed for the current process
 *
 * The solver iterates over QSet/QHash containers, and Qt randomises their
 * iteration order per process by default. Pinning the seed makes a run with a
 * fixed --seed fully reproducible across invocations.
 */
inline void benchmarkUseDeterministicHashSeed()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QHashSeed::setDeterministicGlobalSeed();
#else
    qSetGlobalQHashSeed(0);
#endif
}

#endif // BENCHMARKCOMMON_H
