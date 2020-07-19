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
#include <QStyleFactory>
#include <QApplication>

#include "qpushbutton_adapted.h"

#include <QMainWindow>

/**
 * @brief
 *
 */
enum CELL_STATE: char{
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
enum GAME_DIFFICULTY: uchar{
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
    CELL_STATE state; /**< TODO: describe */
    QPushButton_adapted *button; /**< TODO: describe */
    QLabel *label; /**< TODO: describe */
    bool isHidden; /**< TODO: describe */
    bool hasFlag; /**< TODO: describe */

    Cell():
        state(ZERO),
        button(nullptr),
        label(nullptr),
        isHidden(true),
        hasFlag(false)
    {

    }
};

struct Vector2Dshort {


    short x;
    short y;

    Vector2Dshort():
        x( 0 ),
        y( 0 )
    {

    }

    Vector2Dshort(const qint16& _x, const qint16& _y):
        x(_x),
        y(_y)
    {

    }

    float length() const{
        return sqrtf(static_cast<float>(x*x + y*y));
    }


    float distanceToPoint(const Vector2Dshort& point) const{
        return Vector2Dshort(point.x - x, point.y - y).length();
    }

};

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

private:
    void vNewGame(const uchar _X,
                  const uchar _Y,
                  ushort i_nMines_,
                  const uchar i_X_Clean = 255,
                  const uchar i_Y_Clean = 255);
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
    void vCasaLivre(const uchar _X, const uchar _Y);
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
    void vAddOrRemoveFlag(const uchar& _X, const uchar& _Y);
    /**
     * @brief
     *
     * @param _X
     * @param _Y
     */
    void vGameLost(const uchar _X, const uchar _Y);
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

    void vConfigureDarkMode(const bool bDark);

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

    ///
    /// \brief SLOT_DarkMode
    ///
    void SLOT_DarkMode();

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
    void SLOT_OnButtonClicked(const QMouseEvent* e);

private:

    std::vector< std::vector<Cell> > PrincipalMatrix; /**< TODO: describe */
    int iLimitHeight; /**< TODO: describe */
    int iLimitWidth; /**< TODO: describe */

    uchar iX; /**< TODO: describe */
    uchar iY; /**< TODO: describe */
    ushort nMines; /**< TODO: describe */
    ushort iTimeInSeconds; /**< TODO: describe */
    ushort iMinesLeft; /**< TODO: describe */
    ushort iHiddenCells; /**< TODO: describe */
    int fm; /**< TODO: describe */

    ushort iCellsToUnlock; /**< TODO: describe */

    bool bFirst; /**< TODO: describe */
    bool bFirstCellClean; /**< TODO: describe */

    GAME_DIFFICULTY difficult; /**< TODO: describe */

    QPushButton *buttonEasy; /**< TODO: describe */
    QPushButton *buttonMedium; /**< TODO: describe */
    QPushButton *buttonHard; /**< TODO: describe */

    QPushButton *buttonCustomizedNewGame; /**< TODO: describe */

    QSpinBox *sbCustomizedX; /**< TODO: describe */
    QSpinBox *sbCustomizedY; /**< TODO: describe */
    QSpinBox *sbCustomizednMines; /**< TODO: describe */

    QLabel *labelCustomizedX; /**< TODO: describe */
    QLabel *labelCustomizedY; /**< TODO: describe */
    QLabel *labelCustomized_nMines; /**< TODO: describe */

    QCheckBox *cbFirstCellClean; /**< TODO: describe */
    QCheckBox *cbDarkModeEnabled;

    QTimer *timerTimeInGame; /**< TODO: describe */
    QLabel *labelTimerInGame; /**< TODO: describe */
    QLCDNumber *lcd_numberMinesLeft; /**< TODO: describe */
    QPushButton *buttonRestartInGame; /**< TODO: describe */
    QPushButton *buttonQuitInGame; /**< TODO: describe */

    QLabel *labelMinesBollos; /**< TODO: describe */
    QLabel *labelYouWonYouLost; /**< TODO: describe */
    QLabel *labelStatisLastMatch; /**< TODO: describe */

    QImage *imgZero; /**< TODO: describe */
    QImage *imgOne; /**< TODO: describe */
    QImage *imgTwo; /**< TODO: describe */
    QImage *imgThree; /**< TODO: describe */
    QImage *imgFour; /**< TODO: describe */
    QImage *imgFive; /**< TODO: describe */
    QImage *imgSix; /**< TODO: describe */
    QImage *imgSeven; /**< TODO: describe */
    QImage *imgEight; /**< TODO: describe */
    QImage *imgFlag; /**< TODO: describe */
    QImage *imgNoFlag; /**< TODO: describe */
    QImage *imgMine; /**< TODO: describe */
    QImage *imgBoom; /**< TODO: describe */
    QImage *imgWrongFlag; /**< TODO: describe */

};
#endif // MINES_BOLLOS_H
