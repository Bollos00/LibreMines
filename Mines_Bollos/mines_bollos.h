#ifndef MINES_BOLLOS_H
#define MINES_BOLLOS_H

#include "stdlib.h"
#include <cmath>

#include <QVector>
#include <QRandomGenerator>
#include <QLabel>
#include <QObject>
#include <QDebug>
#include <QDesktopWidget>
#include <QSpinBox>
#include <QFont>
#include <QTimer>
#include <QLCDNumber>
#include <QMouseEvent>
#include <QImage>
#include <QPixmap>
#include <QIcon>
#include <QCheckBox>

#include "qpushbutton_adapted.h"

#include <QMainWindow>

enum STATE{
    MINE = -1,
    ZERO = 0,
    ONE = 1,
    TWO = 2,
    THREE = 3,
    FOUR = 4,
    FIVE = 5,
    SIX = 6,
    SEVEN = 7,
    EIGHT = 8
};

enum GAME_DIFFICULTY{
    NONE,
    EASY,
    MEDIUM,
    HARD,
    CUSTOMIZED
};

struct Cell{
    STATE S_Cell;
    QPushButton_adapted *QPB_a;
    QLabel *QL;
    bool bHiddenState;
    bool bFlag;
};

QT_BEGIN_NAMESPACE
namespace Ui { class Mines_Bollos; }
QT_END_NAMESPACE

class Mines_Bollos : public QMainWindow
{
    Q_OBJECT


public:
    Mines_Bollos(QWidget *parent = nullptr);
    ~Mines_Bollos();

    void vNewGame(int _X , int _Y, int i_nMines_, int i_X_Clean = -2, int i_Y_Clean = -2);
    void vResetPrincipal_Matrix();
    void vConfigureInterface();
    void vHideInterface();
    void vShowInterface();
    void vCasaLivre(int _X, int _Y);
    void vAjustInterfaceInGame();
    void vHideInterfaceInGame();
    void vShowInterfaceInGame();
    void vAddOrRemoveFlag(int _X, int _Y);
    void vGameLost(int _X, int _Y);
    void vGameWon();
    void vStartTimer();
    void vGenerateStatics();

private slots:
    void SLOT_Easy();
    void SLOT_Medium();
    void SLOT_Hard();
    void SLOT_Customized();

    void SLOT_Quit();
    void SLOT_Restart();

    void SLOT_UpdateFirstCellClean();

    void SLOT_UpdateTime();

    void SLOT_OnQPushButton_adapteded_Clicked(QMouseEvent *e);

private:
    Ui::Mines_Bollos *ui;

    std::vector<std::vector<Cell>> Principal_Matrix;
    int i_limit_height;
    int i_limit_width;

    int i_X;
    int i_Y;
    int i_nMines;
    int i_TimeInSeconds;
    int i_MinesLeft;
    int i_HiddenCells;
    int fm;

    int i_CellsToUnlock;

    bool bFirst;
    bool bFirstCellClean;

    GAME_DIFFICULTY GAME_STATE;

    QPushButton *QPB_Easy;
    QPushButton *QPB_Medium;
    QPushButton *QPB_Hard;

    QPushButton *QPB_CustomizedNewGame;

    QSpinBox *QSB_Customized_X;
    QSpinBox *QSB_Customized_Y;
    QSpinBox *QSB_Customized_nMines;

    QLabel *QL_Customized_X;
    QLabel *QL_Customized_Y;
    QLabel *QL_Customized_nMines;

    QCheckBox *QCB_FirstCellClean;

    QTimer *QTIMER_TimeInGame;
    QLabel *QL_TimerInGame;
    QLCDNumber *QLCDN_MinesLeft;
    QPushButton *QPB_RestartInGame;
    QPushButton *QPB_QuitInGame;

    QLabel *QL_Mines_Bollos;
    QLabel *QL_YouWon_YouLost;
    QLabel *QL_StatisLastMatch;

    QImage *QI_Zero;
    QImage *QI_One;
    QImage *QI_Two;
    QImage *QI_Three;
    QImage *QI_Four;
    QImage *QI_Five;
    QImage *QI_Six;
    QImage *QI_Seven;
    QImage *QI_Eight;
    QImage *QI_Flag;
    QImage *QI_NoFlag;
    QImage *QI_Mine;
    QImage *QI_Boom;
    QImage *QI_WrongFlag;

};
#endif // MINES_BOLLOS_H
