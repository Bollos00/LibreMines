#ifndef LIBREMINESVIEWSCORES_H
#define LIBREMINESVIEWSCORES_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QMap>
#include <QTableWidget>

#include "libreminesscore.h"

class LibreMinesViewScores : public QDialog
{
    Q_OBJECT
public:
    LibreMinesViewScores(QWidget *parent);

    void setScores(const QList<LibreMinesScore>& scores);

private:

    void vCreateTab(const QString& tabName);

    QTabWidget* tabWidget;

    struct A{ QWidget* widget; QTableWidget* table; QList<LibreMinesScore> scores; };
    QMap<QString, A> tabs;

    QVBoxLayout* layoutGlobal;

};

#endif // LIBREMINESVIEWSCORES_H
