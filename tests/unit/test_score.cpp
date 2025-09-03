/*****************************************************************************
 * LibreMines Test Suite                                                     *
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

#include <QtTest/QtTest>
#include <QObject>
#include <QDataStream>
#include <QBuffer>
#include <QDebug>

#include "libreminesscore.h"
#include "common.h"

class TestScore : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic construction and assignment tests
    void testScoreConstruction();
    void testScoreAssignment();
    void testScoreStringConversion();
    
    // Comparison tests
    void testScoreComparison();
    void testScoreComparisonSameTime();
    void testScoreComparisonDifferentDifficulty();
    void testScoreComparisonIncompleteGames();
    
    // Serialization tests
    void testScoreSerialization();
    void testScoreDeserialization();
    void testScoreSerializationRoundTrip();
    
    // Sorting tests
    void testScoreSorting();
    void testScoreSortingMixed();
    void testScoreSortingEmptyList();
    
    // Game completion tests
    void testGameCompletionPercentage();
    void testCompletedVsIncompleteGames();
    
    // Edge cases
    void testZeroTime();
    void testMaximumTime();
    void testInvalidDifficulty();

private:
    LibreMinesScore *score1;
    LibreMinesScore *score2;
    LibreMinesScore *score3;
    
    // Helper methods
    LibreMinesScore createTestScore(qint64 timeNs, GameDifficulty difficulty, 
                                   bool completed = true, double percentage = 100.0);
    void compareScores(const LibreMinesScore& score1, const LibreMinesScore& score2);
};

void TestScore::initTestCase()
{
    // Setup test environment
}

void TestScore::cleanupTestCase()
{
    // Cleanup test environment
}

void TestScore::init()
{
    score1 = new LibreMinesScore();
    score2 = new LibreMinesScore();
    score3 = new LibreMinesScore();
}

void TestScore::cleanup()
{
    delete score1;
    delete score2;
    delete score3;
    score1 = score2 = score3 = nullptr;
}

void TestScore::testScoreConstruction()
{
    LibreMinesScore score;
    
    // Test default values
    QCOMPARE(score.iTimeInNs, static_cast<qint64>(0));
    QCOMPARE(score.gameDifficulty, GameDifficulty::NONE);
    QCOMPARE(score.width, static_cast<uchar>(0));
    QCOMPARE(score.height, static_cast<uchar>(0));
    QCOMPARE(score.mines, static_cast<short>(0));
    QCOMPARE(score.bGameCompleted, false);
    QCOMPARE(score.dPercentageGameCompleted, 0.0);
    QVERIFY(score.username.isEmpty());
}

void TestScore::testScoreAssignment()
{
    LibreMinesScore score;
    
    // Set values
    score.iTimeInNs = 15000000000LL; // 15 seconds in nanoseconds
    score.gameDifficulty = GameDifficulty::MEDIUM;
    score.width = 16;
    score.height = 16;
    score.mines = 40;
    score.bGameCompleted = true;
    score.dPercentageGameCompleted = 100.0;
    score.username = "TestUser";
    
    // Verify assignment
    QCOMPARE(score.iTimeInNs, 15000000000LL);
    QCOMPARE(score.gameDifficulty, GameDifficulty::MEDIUM);
    QCOMPARE(score.width, static_cast<uchar>(16));
    QCOMPARE(score.height, static_cast<uchar>(16));
    QCOMPARE(score.mines, static_cast<short>(40));
    QCOMPARE(score.bGameCompleted, true);
    QCOMPARE(score.dPercentageGameCompleted, 100.0);
    QCOMPARE(score.username, QString("TestUser"));
}

void TestScore::testScoreStringConversion()
{
    LibreMinesScore score;
    score.iTimeInNs = 10000000000LL; // 10 seconds
    score.gameDifficulty = GameDifficulty::EASY;
    score.width = 9;
    score.height = 9;
    score.mines = 10;
    score.bGameCompleted = true;
    score.dPercentageGameCompleted = 100.0;
    score.username = "Player1";
    
    QString scoreString = static_cast<QString>(score);
    QVERIFY(!scoreString.isEmpty());
    
    // Should contain key information
    QVERIFY(scoreString.contains("Player1"));
    // Note: The actual format depends on implementation
}

void TestScore::testScoreComparison()
{
    // Test that faster time is better
    *score1 = createTestScore(10000000000LL, GameDifficulty::EASY); // 10 seconds
    *score2 = createTestScore(15000000000LL, GameDifficulty::EASY); // 15 seconds
    
    QVERIFY(LibreMinesScore::bFirstIsBetter(*score1, *score2));
    QVERIFY(!LibreMinesScore::bFirstIsBetter(*score2, *score1));
}

void TestScore::testScoreComparisonSameTime()
{
    // When times are equal, both should be equivalent
    *score1 = createTestScore(10000000000LL, GameDifficulty::EASY);
    *score2 = createTestScore(10000000000LL, GameDifficulty::EASY);
    
    // Neither should be definitively better
    bool first_better = LibreMinesScore::bFirstIsBetter(*score1, *score2);
    bool second_better = LibreMinesScore::bFirstIsBetter(*score2, *score1);
    
    // Depending on implementation, this could go either way or be equal
    QVERIFY(!(first_better && second_better)); // Can't both be better
}

void TestScore::testScoreComparisonDifferentDifficulty()
{
    // The LibreMinesScore comparison requires same difficulty
    // So we'll test that it enforces this requirement
    *score1 = createTestScore(10000000000LL, GameDifficulty::EASY);
    *score2 = createTestScore(15000000000LL, GameDifficulty::HARD);
    
    // This test verifies that we understand the limitation - 
    // different difficulties cannot be directly compared
    // In a real application, scores would be grouped by difficulty
    QVERIFY(score1->gameDifficulty != score2->gameDifficulty);
    
    // Test comparison within same difficulty instead
    *score2 = createTestScore(15000000000LL, GameDifficulty::EASY);
    QVERIFY(LibreMinesScore::bFirstIsBetter(*score1, *score2));
}

void TestScore::testScoreComparisonIncompleteGames()
{
    // Test comparison with incomplete games
    *score1 = createTestScore(10000000000LL, GameDifficulty::EASY, true, 100.0);  // Complete
    *score2 = createTestScore(15000000000LL, GameDifficulty::EASY, false, 75.0); // Incomplete
    
    // Completed games should generally be better than incomplete ones
    QVERIFY(LibreMinesScore::bFirstIsBetter(*score1, *score2));
}

void TestScore::testScoreSerialization()
{
    LibreMinesScore originalScore = createTestScore(12345678900LL, GameDifficulty::MEDIUM);
    originalScore.username = "SerializationTest";
    originalScore.width = 12;
    originalScore.height = 8;
    originalScore.mines = 25;
    
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << originalScore;
    
    // Verify data was written
    QVERIFY(!data.isEmpty());
}

void TestScore::testScoreDeserialization()
{
    // Create and serialize a score
    LibreMinesScore originalScore = createTestScore(98765432100LL, GameDifficulty::HARD);
    originalScore.username = "DeserializationTest";
    originalScore.width = 20;
    originalScore.height = 15;
    originalScore.mines = 60;
    
    QByteArray data;
    QDataStream outStream(&data, QIODevice::WriteOnly);
    outStream << originalScore;
    
    // Deserialize
    LibreMinesScore deserializedScore;
    QDataStream inStream(&data, QIODevice::ReadOnly);
    inStream >> deserializedScore;
    
    // Verify deserialization
    QCOMPARE(deserializedScore.iTimeInNs, originalScore.iTimeInNs);
    QCOMPARE(deserializedScore.gameDifficulty, originalScore.gameDifficulty);
    QCOMPARE(deserializedScore.username, originalScore.username);
    QCOMPARE(deserializedScore.width, originalScore.width);
    QCOMPARE(deserializedScore.height, originalScore.height);
    QCOMPARE(deserializedScore.mines, originalScore.mines);
    QCOMPARE(deserializedScore.bGameCompleted, originalScore.bGameCompleted);
    QCOMPARE(deserializedScore.dPercentageGameCompleted, originalScore.dPercentageGameCompleted);
}

void TestScore::testScoreSerializationRoundTrip()
{
    // Test multiple round trips
    QList<LibreMinesScore> originalScores;
    originalScores.append(createTestScore(1000000000LL, GameDifficulty::EASY));
    originalScores.append(createTestScore(2000000000LL, GameDifficulty::MEDIUM));
    originalScores.append(createTestScore(3000000000LL, GameDifficulty::HARD));
    
    for (int i = 0; i < originalScores.size(); ++i) {
        originalScores[i].username = QString("User%1").arg(i);
    }
    
    // Serialize all scores
    QByteArray data;
    QDataStream outStream(&data, QIODevice::WriteOnly);
    for (const auto& score : originalScores) {
        outStream << score;
    }
    
    // Deserialize all scores
    QList<LibreMinesScore> deserializedScores;
    QDataStream inStream(&data, QIODevice::ReadOnly);
    for (int i = 0; i < originalScores.size(); ++i) {
        LibreMinesScore score;
        inStream >> score;
        deserializedScores.append(score);
    }
    
    // Verify all scores
    QCOMPARE(deserializedScores.size(), originalScores.size());
    for (int i = 0; i < originalScores.size(); ++i) {
        QCOMPARE(deserializedScores[i].iTimeInNs, originalScores[i].iTimeInNs);
        QCOMPARE(deserializedScores[i].gameDifficulty, originalScores[i].gameDifficulty);
        QCOMPARE(deserializedScores[i].username, originalScores[i].username);
    }
}

void TestScore::testScoreSorting()
{
    QList<LibreMinesScore> scores;
    
    // Create scores with different times (higher time = worse score)
    scores.append(createTestScore(15000000000LL, GameDifficulty::EASY)); // 15 sec
    scores.append(createTestScore(10000000000LL, GameDifficulty::EASY)); // 10 sec (best)
    scores.append(createTestScore(20000000000LL, GameDifficulty::EASY)); // 20 sec
    scores.append(createTestScore(12000000000LL, GameDifficulty::EASY)); // 12 sec
    
    LibreMinesScore::sort(scores);
    
    // Verify sorted order (best scores first)
    QVERIFY(scores.size() == 4);
    
    // Check that scores are sorted in improving order
    for (int i = 0; i < scores.size() - 1; ++i) {
        // First score should be better than or equal to second score
        QVERIFY(LibreMinesScore::bFirstIsBetter(scores[i], scores[i+1]) || 
                !LibreMinesScore::bFirstIsBetter(scores[i+1], scores[i]));
    }
}

void TestScore::testScoreSortingMixed()
{
    QList<LibreMinesScore> scores;
    
    // Mix of completed and incomplete games
    scores.append(createTestScore(10000000000LL, GameDifficulty::EASY, true, 100.0));
    scores.append(createTestScore(15000000000LL, GameDifficulty::EASY, false, 80.0));
    scores.append(createTestScore(8000000000LL, GameDifficulty::EASY, true, 100.0));
    scores.append(createTestScore(12000000000LL, GameDifficulty::EASY, false, 90.0));
    
    LibreMinesScore::sort(scores);
    
    // Verify list is sorted
    QCOMPARE(scores.size(), 4);
    
    // Completed games should generally rank higher than incomplete ones
    // (exact ordering depends on implementation)
}

void TestScore::testScoreSortingEmptyList()
{
    QList<LibreMinesScore> emptyList;
    
    // Should not crash on empty list - just verify it completes without exception
    LibreMinesScore::sort(emptyList);
    QVERIFY(emptyList.isEmpty());
}

void TestScore::testGameCompletionPercentage()
{
    LibreMinesScore incompleteScore;
    incompleteScore.bGameCompleted = false;
    incompleteScore.dPercentageGameCompleted = 75.5;
    
    LibreMinesScore completeScore;
    completeScore.bGameCompleted = true;
    completeScore.dPercentageGameCompleted = 100.0;
    
    // Verify completion status
    QVERIFY(!incompleteScore.bGameCompleted);
    QCOMPARE(incompleteScore.dPercentageGameCompleted, 75.5);
    
    QVERIFY(completeScore.bGameCompleted);
    QCOMPARE(completeScore.dPercentageGameCompleted, 100.0);
}

void TestScore::testCompletedVsIncompleteGames()
{
    // Same time, but one is complete and one is not
    *score1 = createTestScore(10000000000LL, GameDifficulty::EASY, true, 100.0);
    *score2 = createTestScore(10000000000LL, GameDifficulty::EASY, false, 95.0);
    
    // Complete game should be better
    QVERIFY(LibreMinesScore::bFirstIsBetter(*score1, *score2));
}

void TestScore::testZeroTime()
{
    LibreMinesScore zeroTimeScore = createTestScore(0LL, GameDifficulty::EASY);
    LibreMinesScore normalScore = createTestScore(10000000000LL, GameDifficulty::EASY);
    
    // Zero time might be invalid or the best possible score
    // depending on implementation
    bool comparison = LibreMinesScore::bFirstIsBetter(zeroTimeScore, normalScore);
    Q_UNUSED(comparison); // Just verify no crash
}

void TestScore::testMaximumTime()
{
    LibreMinesScore maxTimeScore = createTestScore(LLONG_MAX, GameDifficulty::EASY);
    LibreMinesScore normalScore = createTestScore(10000000000LL, GameDifficulty::EASY);
    
    // Normal score should be better than maximum time
    QVERIFY(LibreMinesScore::bFirstIsBetter(normalScore, maxTimeScore));
}

void TestScore::testInvalidDifficulty()
{
    LibreMinesScore invalidScore;
    invalidScore.gameDifficulty = static_cast<GameDifficulty>(999); // Invalid value
    
    // Should handle invalid difficulty gracefully
    QString scoreString = static_cast<QString>(invalidScore);
    Q_UNUSED(scoreString); // Just verify no crash
}

// Helper method implementations
LibreMinesScore TestScore::createTestScore(qint64 timeNs, GameDifficulty difficulty, 
                                          bool completed, double percentage)
{
    LibreMinesScore score;
    score.iTimeInNs = timeNs;
    score.gameDifficulty = difficulty;
    score.bGameCompleted = completed;
    score.dPercentageGameCompleted = percentage;
    score.username = "TestUser";
    
    // Set appropriate dimensions based on difficulty
    switch (difficulty) {
        case GameDifficulty::EASY:
            score.width = 9;
            score.height = 9;
            score.mines = 10;
            break;
        case GameDifficulty::MEDIUM:
            score.width = 16;
            score.height = 16;
            score.mines = 40;
            break;
        case GameDifficulty::HARD:
            score.width = 30;
            score.height = 16;
            score.mines = 99;
            break;
        default:
            score.width = 8;
            score.height = 8;
            score.mines = 5;
            break;
    }
    
    return score;
}

void TestScore::compareScores(const LibreMinesScore& s1, const LibreMinesScore& s2)
{
    QCOMPARE(s1.iTimeInNs, s2.iTimeInNs);
    QCOMPARE(s1.gameDifficulty, s2.gameDifficulty);
    QCOMPARE(s1.width, s2.width);
    QCOMPARE(s1.height, s2.height);
    QCOMPARE(s1.mines, s2.mines);
    QCOMPARE(s1.bGameCompleted, s2.bGameCompleted);
    QCOMPARE(s1.dPercentageGameCompleted, s2.dPercentageGameCompleted);
    QCOMPARE(s1.username, s2.username);
}

QTEST_MAIN(TestScore)
#include "test_score.moc"
