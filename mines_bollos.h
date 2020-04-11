#ifndef MINES_BOLLOS_H
#define MINES_BOLLOS_H

#include "stdlib.h"
#include <cmath>

#include <QVector>
#include <QtCore/QRandomGenerator>
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

/**
 * @brief
 *
 */
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

/**
 * @brief
 *
 */
enum GAME_DIFFICULTY{
    NONE,
    EASY,
    MEDIUM,
    HARD,
    CUSTOMIZED
};

/**
 * @brief
 *
 */
struct Cell{
    STATE S_Cell; /**< TODO: describe */
    QPushButton_adapted *QPB_a; /**< TODO: describe */
    QLabel *QL; /**< TODO: describe */
    bool bHiddenState; /**< TODO: describe */
    bool bFlag; /**< TODO: describe */
};

QT_BEGIN_NAMESPACE
namespace Ui { class Mines_Bollos; }
QT_END_NAMESPACE

/**
 * @brief
 *
 */
class Mines_Bollos : public QMainWindow
{
    Q_OBJECT


public:
    /**
     * @brief
     *
     * @param parent
     */
    Mines_Bollos(QWidget *parent = nullptr);
    /**
     * @brief
     *
     */
    ~Mines_Bollos();

    /**
     * @brief
     *
     * @param _X
     * @param _Y
     * @param i_nMines_
     * @param i_X_Clean
     * @param i_Y_Clean
     */
    void vNewGame(int _X , int _Y, int i_nMines_, int i_X_Clean = -2, int i_Y_Clean = -2);
    /**
     * @brief
     *
     */
    void vResetPrincipal_Matrix();
    /**
     * @brief
     *
     */
    void vConfigureInterface();
    /**
     * @brief
     *
     */
    void vHideInterface();
    /**
     * @brief
     *
     */
    void vShowInterface();
    /**
     * @brief
     *
     * @param _X
     * @param _Y
     */
    void vCasaLivre(int _X, int _Y);
    /**
     * @brief
     *
     */
    void vAjustInterfaceInGame();
    /**
     * @brief
     *
     */
    void vHideInterfaceInGame();
    /**
     * @brief
     *
     */
    void vShowInterfaceInGame();
    /**
     * @brief
     *
     * @param _X
     * @param _Y
     */
    void vAddOrRemoveFlag(int _X, int _Y);
    /**
     * @brief
     *
     * @param _X
     * @param _Y
     */
    void vGameLost(int _X, int _Y);
    /**
     * @brief
     *
     */
    void vGameWon();
    /**
     * @brief
     *
     */
    void vStartTimer();
    /**
     * @brief
     *
     */
    void vGenerateStatics();

private slots:
    /**
     * @brief
     *
     */
    void SLOT_Easy();
    /**
     * @brief
     *
     */
    void SLOT_Medium();
    /**
     * @brief
     *
     */
    void SLOT_Hard();
    /**
     * @brief
     *
     */
    void SLOT_Customized();

    /**
     * @brief
     *
     */
    void SLOT_Quit();
    /**
     * @brief
     *
     */
    void SLOT_Restart();

    /**
     * @brief
     *
     */
    void SLOT_UpdateFirstCellClean();

    /**
     * @brief
     *
     */
    void SLOT_UpdateTime();

    /**
     * @brief
     *
     * @param e
     */
    void SLOT_OnQPushButton_adapteded_Clicked(QMouseEvent *e);

private:
    Ui::Mines_Bollos *ui; /**< TODO: describe */

    std::vector<std::vector<Cell>> Principal_Matrix; /**< TODO: describe */
    int i_limit_height; /**< TODO: describe */
    int i_limit_width; /**< TODO: describe */

    int i_X; /**< TODO: describe */
    int i_Y; /**< TODO: describe */
    int i_nMines; /**< TODO: describe */
    int i_TimeInSeconds; /**< TODO: describe */
    int i_MinesLeft; /**< TODO: describe */
    int i_HiddenCells; /**< TODO: describe */
    int fm; /**< TODO: describe */

    int i_CellsToUnlock; /**< TODO: describe */

    bool bFirst; /**< TODO: describe */
    bool bFirstCellClean; /**< TODO: describe */

    GAME_DIFFICULTY GAME_STATE; /**< TODO: describe */

    QPushButton *QPB_Easy; /**< TODO: describe */
    QPushButton *QPB_Medium; /**< TODO: describe */
    QPushButton *QPB_Hard; /**< TODO: describe */

    QPushButton *QPB_CustomizedNewGame; /**< TODO: describe */

    QSpinBox *QSB_Customized_X; /**< TODO: describe */
    QSpinBox *QSB_Customized_Y; /**< TODO: describe */
    QSpinBox *QSB_Customized_nMines; /**< TODO: describe */

    QLabel *QL_Customized_X; /**< TODO: describe */
    QLabel *QL_Customized_Y; /**< TODO: describe */
    QLabel *QL_Customized_nMines; /**< TODO: describe */

    QCheckBox *QCB_FirstCellClean; /**< TODO: describe */

    QTimer *QTIMER_TimeInGame; /**< TODO: describe */
    QLabel *QL_TimerInGame; /**< TODO: describe */
    QLCDNumber *QLCDN_MinesLeft; /**< TODO: describe */
    QPushButton *QPB_RestartInGame; /**< TODO: describe */
    QPushButton *QPB_QuitInGame; /**< TODO: describe */

    QLabel *QL_Mines_Bollos; /**< TODO: describe */
    QLabel *QL_YouWon_YouLost; /**< TODO: describe */
    QLabel *QL_StatisLastMatch; /**< TODO: describe */

    QImage *QI_Zero; /**< TODO: describe */
    QImage *QI_One; /**< TODO: describe */
    QImage *QI_Two; /**< TODO: describe */
    QImage *QI_Three; /**< TODO: describe */
    QImage *QI_Four; /**< TODO: describe */
    QImage *QI_Five; /**< TODO: describe */
    QImage *QI_Six; /**< TODO: describe */
    QImage *QI_Seven; /**< TODO: describe */
    QImage *QI_Eight; /**< TODO: describe */
    QImage *QI_Flag; /**< TODO: describe */
    QImage *QI_NoFlag; /**< TODO: describe */
    QImage *QI_Mine; /**< TODO: describe */
    QImage *QI_Boom; /**< TODO: describe */
    QImage *QI_WrongFlag; /**< TODO: describe */

};
#endif // MINES_BOLLOS_H
