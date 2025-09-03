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

/**
 * @file libreminesscore.h
 * @brief Score management and tracking for LibreMines games
 * @author Bruno Bollos Correa
 * @date 2020-12-12
 * 
 * This file defines the LibreMinesScore class which encapsulates all
 * information about a completed game including timing, difficulty,
 * completion status, and player information. It provides functionality
 * for score comparison, sorting, and serialization.
 */

#ifndef LIBREMINESSCORE_H
#define LIBREMINESSCORE_H

#include "common.h"

/**
 * @brief Represents a single game score with complete game statistics
 * 
 * LibreMinesScore encapsulates all relevant information about a completed
 * minesweeper game including timing data, game parameters, completion status,
 * and player information. This class supports comparison operations for
 * ranking scores and provides serialization capabilities for saving and
 * loading high scores.
 * 
 * The score system considers both completion time and completion percentage,
 * allowing for meaningful comparison of both completed and partially
 * completed games.
 * 
 * @since v1.0
 */
class LibreMinesScore
{
public:
    /**
     * @brief Default constructor
     * 
     * Initializes a score object with default values. Sets time to 0,
     * difficulty to NONE, dimensions to 0, and completion status to false.
     */
    LibreMinesScore();

    /**
     * @brief Convert score to string representation
     * 
     * Provides a human-readable string representation of the score
     * including key statistics and game parameters.
     * 
     * @return QString containing formatted score information
     */
    operator QString() const;

    qint64 iTimeInNs;                    ///< Game duration in nanoseconds
    GameDifficulty gameDifficulty;       ///< Difficulty level of the game
    uchar width;                         ///< Game board width (columns)
    uchar height;                        ///< Game board height (rows) 
    short mines;                         ///< Total number of mines in the game
    bool bGameCompleted;                 ///< Whether the game was successfully completed
    double dPercentageGameCompleted;     ///< Percentage of game completed (0.0 to 1.0)
    QString username;                    ///< Player username associated with this score

    /**
     * @brief Compare two scores to determine which is better
     * 
     * Compares scores based on completion status, time, and completion
     * percentage. Completed games are always considered better than
     * incomplete games. Among completed games, faster times are better.
     * Among incomplete games, higher completion percentages are better.
     * 
     * @param first First score to compare
     * @param second Second score to compare
     * @return true if first score is better than second, false otherwise
     */
    static bool bFirstIsBetter(const LibreMinesScore& first, const LibreMinesScore& second);
    
    /**
     * @brief Sort a list of scores from best to worst
     * 
     * Sorts the provided list of scores in descending order of quality
     * using the bFirstIsBetter comparison function.
     * 
     * @param l List of scores to sort (modified in place)
     */
    static void sort(QList<LibreMinesScore> &l);
};

/**
 * @brief Serialize a score to a data stream
 * 
 * Writes all score data to a QDataStream for saving to file or
 * network transmission.
 * 
 * @param stream Target data stream
 * @param score Score object to serialize
 * @return Reference to the data stream for chaining
 */
QDataStream& operator<< (QDataStream& stream, const LibreMinesScore& score);

/**
 * @brief Deserialize a score from a data stream
 * 
 * Reads score data from a QDataStream for loading from file or
 * network reception.
 * 
 * @param stream Source data stream
 * @param score Score object to populate with data
 * @return Reference to the data stream for chaining
 */
QDataStream& operator>> (QDataStream& stream, LibreMinesScore& score);

/**
 * @brief Debug output operator for scores
 * 
 * Provides formatted debug output for score objects when using
 * Qt's debug system.
 * 
 * @param debug Debug output stream
 * @param score Score object to output
 * @return Reference to the debug stream for chaining
 */
QDebug operator<<(QDebug debug, const LibreMinesScore& score);

// Register LibreMinesScore with Qt's meta-object system for QVariant support
Q_DECLARE_METATYPE(LibreMinesScore)

#endif // LIBREMINESSCORE_H
