#include "libreminesviewscores.h"


LibreMinesViewScores::LibreMinesViewScores(QWidget *parent):
    QDialog(parent)
{
    this->setWindowTitle("High Scores");
    this->resize(500, 800);

    layoutGlobal = new QVBoxLayout(this);
    tabWidget = new QTabWidget(this);

    for(QString n : {"Easy", "Medium", "Hard"})
        vCreateTab(n);

    layoutGlobal->addWidget(tabWidget);
}

void LibreMinesViewScores::setScores(const QList<LibreMinesScore>& scores)
{
    for(const LibreMinesScore& s : scores)
    {
        if(s.gameDifficulty == EASY)
        {
            tabs["Easy"].scores.append(s);
        }
        else if(s.gameDifficulty == MEDIUM)
        {
            tabs["Medium"].scores.append(s);
        }
        else if(s.gameDifficulty == HARD)
        {
            tabs["Hard"].scores.append(s);
        }
        else
        {
            QString strDifficulty;
            strDifficulty += QString::number(s.width) + " x ";
            strDifficulty += QString::number(s.heigth) + " : ";
            strDifficulty += QString::number(s.mines);

            if(!tabs.contains(strDifficulty))
            {
                vCreateTab(strDifficulty);
            }

            tabs[strDifficulty].scores.append(s);
        }
    }

    QStringList tableHorizontalHeader;
    tableHorizontalHeader << "User" << "Game Completed (%)" << "Time (secs)";

    for(const QString& key : tabs.keys())
    {
        LibreMinesScore::sort(tabs[key].scores);

        A& tab = tabs[key];
        tab.table->setRowCount(tab.scores.size());
        tab.table->setColumnCount(3);
        tab.table->setHorizontalHeaderLabels(tableHorizontalHeader);
        tab.table->setColumnWidth(0, 100);
        tab.table->setColumnWidth(1, 200);
        tab.table->setColumnWidth(2, 100);


        QStringList tableVerticalHeader;
        tableVerticalHeader.append("1st");

        if(tab.scores.size() >= 2)
            tableVerticalHeader.append("2nd");
        if(tab.scores.size() >= 3)
            tableVerticalHeader.append("3rd");

        for(int i=4; i<=tab.scores.size(); ++i)
        {
            tableVerticalHeader.append(QString::number(i) + "th");
        }
        tab.table->setVerticalHeaderLabels(tableVerticalHeader);

        for(int i=0; i<tab.scores.size(); ++i)
        {
            QTableWidgetItem* user = new QTableWidgetItem(tab.scores.at(i).username);
            QTableWidgetItem* gameCompleted = new QTableWidgetItem( QString::number(tab.scores.at(i).dPercentageGameCompleted, 'f', 2) );
            QTableWidgetItem* time = new QTableWidgetItem( QString::number(tab.scores.at(i).iTimeInNs*1e-9, 'f', 3) );

            user->setFlags(Qt::NoItemFlags);
            gameCompleted->setFlags(Qt::NoItemFlags);
            time->setFlags(Qt::NoItemFlags);

            tab.table->setItem(i, 0, user);
            tab.table->setItem(i, 1, gameCompleted);
            tab.table->setItem(i, 2, time);
        }
    }
}

void LibreMinesViewScores::vCreateTab(const QString &tabName)
{
    tabs[tabName] = {new QWidget(), nullptr, {}};
    tabs[tabName].table = new QTableWidget(tabs[tabName].widget);
    tabWidget->addTab(tabs[tabName].widget, tabName);
    QVBoxLayout* layoutTab = new QVBoxLayout(tabs[tabName].widget);
    layoutTab->addWidget(tabs[tabName].table);

}
