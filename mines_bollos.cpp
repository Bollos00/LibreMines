#include "mines_bollos.h"

MinesBollos::MinesBollos(QWidget *parent) :
    QMainWindow(parent),
    principalMatrix( std::vector< std::vector<Cell> >(0) ),
    iLimitHeight( 0 ),
    iLimitWidth( 0 ),
    iX( 0 ),
    iY( 0 ),
    nMines( 0 ),
    iTimeInSeconds( 0 ),
    iMinesLeft( 0 ),
    iHiddenCells( 0 ),
    fm( 0 ),
    iCellsToUnlock( 0 ),
    bFirst( 0 ),
    bFirstCellClean( 0 ),
    difficult( NONE ),
    imgZero ( new QImage(":/Media_rsc/Media/Minesweeper_zero.png") ),
    imgOne ( new QImage(":/Media_rsc/Media/Minesweeper_one.png") ),
    imgTwo ( new QImage(":/Media_rsc/Media/Minesweeper_two.png") ),
    imgThree ( new QImage(":/Media_rsc/Media/Minesweeper_three.png") ),
    imgFour( new QImage(":/Media_rsc/Media/Minesweeper_four.png") ),
    imgFive ( new QImage(":/Media_rsc/Media/Minesweeper_five.png") ),
    imgSix( new QImage(":/Media_rsc/Media/Minesweeper_six.png") ),
    imgSeven ( new QImage(":/Media_rsc/Media/Minesweeper_seven.png") ),
    imgEight ( new QImage(":/Media_rsc/Media/Minesweeper_eight.png") ),
    imgFlag ( new QImage(":/Media_rsc/Media/Minesweeper_flag.png") ),
    imgNoFlag ( new QImage(":/Media_rsc/Media/Minesweeper_no_flag.png") ),
    imgMine ( new QImage(":/Media_rsc/Media/Minesweeper_mine.png") ),
    imgBoom ( new QImage(":/Media_rsc/Media/Minesweeper_boom.png") ),
    imgWrongFlag ( new QImage(":/Media_rsc/Media/Minesweeper_wrong_flag.png") )
{
    vConfigureInterface();

    vConfigureDarkMode(true);
}

MinesBollos::~MinesBollos()
{
    vResetPrincipalMatrix();

    delete buttonEasy;
    delete buttonMedium;
    delete buttonHard;

    delete buttonCustomizedNewGame;

    delete sbCustomizedX;
    delete sbCustomizedY;
    delete sbCustomizednMines;

    delete labelCustomizedX;
    delete labelCustomizedY;
    delete labelCustomized_nMines;

    delete cbFirstCellClean;
    delete cbDarkModeEnabled;

    delete timerTimeInGame;
    delete labelTimerInGame;
    delete lcd_numberMinesLeft;
    delete buttonRestartInGame;
    delete buttonQuitInGame;

    delete labelMinesBollos;
    delete labelYouWonYouLost;
    delete labelStatisLastMatch;

    delete imgZero;
    delete imgOne;
    delete imgTwo;
    delete imgThree;
    delete imgFour;
    delete imgFive;
    delete imgSix;
    delete imgSeven;
    delete imgEight;
    delete imgFlag;
    delete imgNoFlag;
    delete imgMine;
    delete imgBoom;
    delete imgWrongFlag;

}

void MinesBollos::vNewGame(const uchar _X,
                            const uchar _Y,
                            ushort i_nMines_,
                            const uchar i_X_Clean,
                            const uchar i_Y_Clean)
{
    buttonQuitInGame->setEnabled(false);
    buttonRestartInGame->setEnabled(false);

    iX = _X;
    iY = _Y;
    nMines = i_nMines_;
    iMinesLeft = i_nMines_;
    iHiddenCells = 0;

    const bool bRemakingGame = (i_X_Clean != 255 && i_Y_Clean != 255);

    if(!bRemakingGame)
        principalMatrix = std::vector<std::vector<Cell>> (iX, std::vector<Cell>(iY));

    if(iLimitWidth/iX < iLimitHeight/iY)
        fm = iLimitWidth/iX;
    else
        fm = iLimitHeight/iY;

    const QPixmap
            QPM_Zero = QPixmap::fromImage(*imgZero).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_One = QPixmap::fromImage(*imgOne).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Two = QPixmap::fromImage(*imgTwo).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Three = QPixmap::fromImage(*imgThree).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Four = QPixmap::fromImage(*imgFour).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Five = QPixmap::fromImage(*imgFive).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Six = QPixmap::fromImage(*imgSix).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Seven = QPixmap::fromImage(*imgSeven).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Eight = QPixmap::fromImage(*imgEight).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_NoFlag = QPixmap::fromImage(*imgNoFlag).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Mine = QPixmap::fromImage(*imgMine).scaled(fm, fm, Qt::KeepAspectRatio);

    if(bRemakingGame){

        for(std::vector<Cell>& i: principalMatrix)
        {
            for(Cell& j: i)
            {
                j.state = ZERO;
            }
        }
    }
    else
    {
        for(uchar j=0; j<iY; j++)
        {
            for (uchar i=0; i<iX; i++)
            {
                Cell& cell = principalMatrix[i][j];

                cell.label = new QLabel(this);
                cell.button = new QPushButton_adapted(this);

                cell.label->setGeometry(i*fm,j*fm,fm,fm);
                cell.label->show();

                cell.button->setGeometry(i*fm,j*fm,fm,fm);
                cell.button->setIcon(QIcon(QPM_NoFlag));
                cell.button->setIconSize(QSize(fm, fm));
                cell.button->show();
                cell.button->setEnabled(false);

                cell.state = ZERO;
                cell.isHidden = true;
                cell.hasFlag = false;

                connect(cell.button, &QPushButton_adapted::SIGNAL_Clicked,
                        this, &MinesBollos::SLOT_OnButtonClicked);

                qApp->processEvents();
            }
        }
    }


    QVector<Vector2Dshort> vt_vt2d_CleanPoints = QVector<Vector2Dshort>();

    if(bRemakingGame)
    {
        vt_vt2d_CleanPoints.reserve(9);

        for (short i=-1; i<=1; i++)
        {
            for (short j=-1; j<=1; j++)
            {
                vt_vt2d_CleanPoints.append(Vector2Dshort(i_X_Clean + i, i_Y_Clean + j));
            }
        }
    }

    while(i_nMines_ > 0)
    {
        uchar i = QRandomGenerator::global()->bounded(0,iX);
        uchar j = QRandomGenerator::global()->bounded(0,iY);

        bool bPointClean = false;

        if(bRemakingGame)
        {
            for (const Vector2Dshort& n: vt_vt2d_CleanPoints)
            {
                if(n.x == i &&
                   n.y == j)
                {
                    bPointClean = true;
                    break;
                }
            }
        }

        Cell& cell = principalMatrix[i][j];

        if(cell.state == ZERO &&
           !bPointClean)
        {
            i_nMines_--;
            cell.state = MINE;
            cell.label->setPixmap(QPM_Mine);
        }
    }

    for(uchar j=0; j<iY; j++)
    {
        for (uchar i=0; i<iX; i++)
        {
            Cell& cell = principalMatrix[i][j];

            cell.button->setEnabled(true);

            if(cell.state == ZERO){

                iHiddenCells++;

                char minesNeighbors = 0;

                if(i == 0 &&
                   j == 0)
                {
                    if(principalMatrix[i+1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j+1].state == MINE)
                        minesNeighbors++;
                }
                else if(i == 0 &&
                        j == iY-1)
                {
                    if(principalMatrix[i+1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j-1].state == MINE)
                        minesNeighbors++;
                }
                else if(i == iX-1 &&
                        j==0)
                {
                    if(principalMatrix[i-1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j+1].state == MINE)
                        minesNeighbors++;
                }
                else if(i == iX-1 &&
                        j == iY-1)
                {
                    if(principalMatrix[i-1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j-1].state == MINE)
                        minesNeighbors++;
                }
                else if(i == 0 &&
                        j > 0 &&
                        j < iY-1)
                {
                    if(principalMatrix[i+1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j-1].state == MINE)
                        minesNeighbors++;
                }
                else if(i == iX-1 &&
                        j >0 &&
                        j < iY-1)
                {
                    if(principalMatrix[i-1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j-1].state == MINE)
                        minesNeighbors++;
                }
                else if(i > 0 &&
                        i < iX-1 &&
                        j == 0){
                    if(principalMatrix[i-1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j+1].state == MINE)
                        minesNeighbors++;
                }
                else if(i > 0 &&
                        i < iX-1 &&
                        j == iY-1)
                {
                    if(principalMatrix[i+1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j-1].state == MINE)
                        minesNeighbors++;
                }
                else
                {
                    if(principalMatrix[i-1][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i-1][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i][j+1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j-1].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j].state == MINE)
                        minesNeighbors++;
                    if(principalMatrix[i+1][j+1].state == MINE)
                        minesNeighbors++;
                }

                switch (minesNeighbors) {
                    case 0:
                        cell.state = ZERO;
                        cell.label->setPixmap(QPM_Zero);
                        break;

                    case 1:
                        cell.state = ONE;
                        cell.label->setPixmap(QPM_One);
                        break;

                    case 2:
                        cell.state = TWO;
                        cell.label->setPixmap(QPM_Two);
                        break;

                    case 3:
                        cell.state = THREE;
                        cell.label->setPixmap(QPM_Three);
                        break;

                    case 4:
                        cell.state = FOUR;
                        cell.label->setPixmap(QPM_Four);
                        break;

                    case 5:
                        cell.state = FIVE;
                        cell.label->setPixmap(QPM_Five);
                        break;

                    case 6:
                        cell.state = SIX;
                        cell.label->setPixmap(QPM_Six);
                        break;

                    case 7:
                        cell.state = SEVEN;
                        cell.label->setPixmap(QPM_Seven);
                        break;

                    case 8:
                        cell.state = EIGHT;
                        cell.label->setPixmap(QPM_Eight);
                        break;
                }
            }
        }
    }

    iCellsToUnlock = iHiddenCells;

    vAjustInterfaceInGame();
    vShowInterfaceInGame();
    lcd_numberMinesLeft->display(iMinesLeft);
    labelTimerInGame->setNum(0);
    labelYouWonYouLost->setText(" ");

    buttonQuitInGame->setEnabled(true);
    buttonRestartInGame->setEnabled(true);
}



void MinesBollos::vGameLost(const uchar _X, const uchar _Y){

    qDebug()<<"You Lost";

    timerTimeInGame->stop();

    switch (difficult) {
        case NONE:
            break;
        case EASY:
            labelYouWonYouLost->setText("You Lost\nDifficulty: EASY");
            break;
        case MEDIUM:
            labelYouWonYouLost->setText("You Lost\nDifficulty: MEDIUM");
            break;
        case HARD:
            labelYouWonYouLost->setText("You Lost\nDifficulty: HARD");
            break;
        case CUSTOMIZED:
            labelYouWonYouLost->setText("You Lost\nDifficulty: CUSTOM\n" +
                                        QString::number(iX) +
                                        "x" +
                                        QString::number(iY) +
                                        " : " +
                                        QString::number(nMines) +
                                        " Mines");
    }
    principalMatrix[_X][_Y].label->setPixmap(QPixmap::fromImage(*imgBoom).scaled(fm, fm, Qt::KeepAspectRatio));

    vGenerateStatics();

    for(uchar j=0; j<iY; j++)
    {
        for (uchar i=0; i<iX; i++)
        {
            Cell& cell = principalMatrix[i][j];

            if(cell.isHidden)
            {
                if(cell.state == MINE &&
                   !cell.hasFlag)
                {
                    cell.button->hide();
                }
                else if (cell.state != MINE &&
                         cell.hasFlag)
                {
                    cell.button->hide();
                    cell.label->setPixmap(QPixmap::fromImage(*imgWrongFlag).scaled(fm, fm, Qt::KeepAspectRatio));
                }
                else
                {
                    cell.button->setEnabled(false);
                }
            }
        }
    }
}

void MinesBollos::vGameWon(){

    qDebug()<<"You won: "<<iTimeInSeconds<<" Seconds";

    timerTimeInGame->stop();

    switch (difficult) {
        case NONE:
            break;
        case EASY:
            labelYouWonYouLost->setText("You Won\nDifficulty: EASY");
            break;
        case MEDIUM:
            labelYouWonYouLost->setText("You Won\nDifficulty: MEDIUM");
            break;
        case HARD:
            labelYouWonYouLost->setText("You Won\nDifficulty: HARD");
            break;
        case CUSTOMIZED:
            labelYouWonYouLost->setText("You Won\nDifficulty: CUSTOM\n" + QString::number(iX) + "x" + QString::number(iY) + " : " + QString::number(nMines) + " Mines");
    }

    vGenerateStatics();
    for(uchar j=0; j<iY; j++)
    {
        for (uchar i=0; i<iX; i++)
        {
            principalMatrix[i][j].button->setEnabled(!principalMatrix[i][j].isHidden);
        }
    }

}

void MinesBollos::vCellClean(const uchar _X, const uchar _Y)
{
    if(principalMatrix[_X][_Y].isHidden &&
       !principalMatrix[_X][_Y].hasFlag)
    {
        principalMatrix[_X][_Y].isHidden = false;
        principalMatrix[_X][_Y].button->hide();

        if(principalMatrix[_X][_Y].state == ZERO)
        {
            if(_X == 0 &&
               _Y == 0)
            {
                if(principalMatrix[_X+1][_Y].isHidden)
                    vCellClean(_X+1, _Y);
                if(principalMatrix[_X][_Y+1].isHidden)
                    vCellClean(_X, _Y+1);
                if(principalMatrix[_X+1][_Y+1].isHidden)
                    vCellClean(_X+1, _Y+1);
            }
            else if(_X == 0 &&
                    _Y == iY-1)
            {
                if(principalMatrix[_X+1][_Y].isHidden)
                    vCellClean(_X+1, _Y);
                if(principalMatrix[_X][_Y-1].isHidden)
                    vCellClean(_X, _Y-1);
                if(principalMatrix[_X+1][_Y-1].isHidden)
                    vCellClean(_X+1, _Y-1);
            }
            else if(_X == iX-1 &&
                    _Y == 0)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    vCellClean(_X-1, _Y);
                if(principalMatrix[_X][_Y+1].isHidden)
                    vCellClean(_X, _Y+1);
                if(principalMatrix[_X-1][_Y+1].isHidden)
                    vCellClean(_X-1, _Y+1);
            }
            else if(_X == iX-1 &&
                    _Y == iY-1)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    vCellClean(_X-1, _Y);
                if(principalMatrix[_X][_Y-1].isHidden)
                    vCellClean(_X, _Y-1);
                if(principalMatrix[_X-1][_Y-1].isHidden)
                    vCellClean(_X-1, _Y-1);
            }
            else if(_X == iX-1 &&
                    _Y == 0)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    vCellClean(_X-1, _Y);
                if(principalMatrix[_X][_Y+1].isHidden)
                    vCellClean(_X, _Y+1);
                if(principalMatrix[_X-1][_Y+1].isHidden)
                    vCellClean(_X-1, _Y+1);
            }
            else if(_X == iX-1 &&
                    _Y == iY-1)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    vCellClean(_X-1, _Y);
                if(principalMatrix[_X][_Y-1].isHidden)
                    vCellClean(_X, _Y-1);
                if(principalMatrix[_X-1][_Y-1].isHidden)
                    vCellClean(_X-1, _Y-1);
            }
            else if(_X == 0 &&
                    _Y > 0 &&
                    _Y < iY-1)
            {
                if(principalMatrix[_X+1][_Y].isHidden)
                    vCellClean(_X+1, _Y);
                if(principalMatrix[_X][_Y+1].isHidden)
                    vCellClean(_X, _Y+1);
                if(principalMatrix[_X+1][_Y+1].isHidden)
                    vCellClean(_X+1, _Y+1);
                if(principalMatrix[_X][_Y-1].isHidden)
                    vCellClean(_X, _Y-1);
                if(principalMatrix[_X+1][_Y-1].isHidden)
                    vCellClean(_X+1, _Y-1);
            }
            else if(_X == iX-1 &&
                    _Y > 0 &&
                    _Y < iY-1)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    vCellClean(_X-1, _Y);
                if(principalMatrix[_X][_Y+1].isHidden)
                    vCellClean(_X, _Y+1);
                if(principalMatrix[_X-1][_Y+1].isHidden)
                    vCellClean(_X-1, _Y+1);
                if(principalMatrix[_X][_Y-1].isHidden)
                    vCellClean(_X, _Y-1);
                if(principalMatrix[_X-1][_Y-1].isHidden)
                    vCellClean(_X-1, _Y-1);
            }
            else if(_X > 0 &&
                    _X < iX-1 &&
                    _Y == 0)
            {
                if(principalMatrix[_X-1][_Y].isHidden)
                    vCellClean(_X-1, _Y);
                if(principalMatrix[_X+1][_Y].isHidden)
                    vCellClean(_X+1, _Y);
                if(principalMatrix[_X-1][_Y+1].isHidden)
                    vCellClean(_X-1, _Y+1);
                if(principalMatrix[_X][_Y+1].isHidden)
                    vCellClean(_X, _Y+1);
                if(principalMatrix[_X+1][_Y+1].isHidden)
                    vCellClean(_X+1, _Y+1);
            }
            else if(_X > 0 &&
                    _X < iX-1 &&
                    _Y == iY-1)
            {
                if(principalMatrix[_X+1][_Y].isHidden)
                    vCellClean(_X+1, _Y);
                if(principalMatrix[_X-1][_Y].isHidden)
                    vCellClean(_X-1, _Y);
                if(principalMatrix[_X-1][_Y-1].isHidden)
                    vCellClean(_X-1, _Y-1);
                if(principalMatrix[_X][_Y-1].isHidden)
                    vCellClean(_X, _Y-1);
                if(principalMatrix[_X+1][_Y-1].isHidden)
                    vCellClean(_X+1, _Y-1);
            }
            else
            {
                if(principalMatrix[_X-1][_Y-1].isHidden)
                    vCellClean(_X-1, _Y-1);
                if(principalMatrix[_X-1][_Y].isHidden)
                    vCellClean(_X-1, _Y);
                if(principalMatrix[_X-1][_Y+1].isHidden)
                    vCellClean(_X-1, _Y+1);
                if(principalMatrix[_X][_Y-1].isHidden)
                    vCellClean(_X, _Y-1);
                if(principalMatrix[_X][_Y+1].isHidden)
                    vCellClean(_X, _Y+1);
                if(principalMatrix[_X+1][_Y-1].isHidden)
                    vCellClean(_X+1, _Y-1);
                if(principalMatrix[_X+1][_Y].isHidden)
                    vCellClean(_X+1, _Y);
                if(principalMatrix[_X+1][_Y+1].isHidden)
                    vCellClean(_X+1, _Y+1);
            }
        }
        else
        {
            principalMatrix[_X][_Y].isHidden = false;
            principalMatrix[_X][_Y].button->hide();
        }
        iHiddenCells--;
        if(iHiddenCells == 0)
            vGameWon();
    }

}

void MinesBollos::vResetPrincipalMatrix()
{
    for(std::vector<Cell>& i: principalMatrix)
    {
        for (Cell& j: i){
            delete j.label;
            delete j.button;
        }
    }
    principalMatrix.clear();
}

void MinesBollos::vConfigureInterface()
{
    labelTimerInGame = new QLabel (this);
    lcd_numberMinesLeft = new QLCDNumber (this);
    buttonRestartInGame = new QPushButton (this);
    buttonQuitInGame = new QPushButton (this);
    labelYouWonYouLost = new QLabel(this);
    labelStatisLastMatch = new QLabel(this);

    labelTimerInGame->setFont(QFont("Ubuntu", 40));
    labelTimerInGame->setNum(0);
    lcd_numberMinesLeft->setDecMode();
    lcd_numberMinesLeft->display(0);
    buttonQuitInGame->setText("Quit");
    buttonRestartInGame->setText("Restart");
    labelYouWonYouLost->setFont(QFont("Ubuntu", 15));

    vHideInterfaceInGame();

    QRect rec = QApplication::desktop()->screenGeometry();
    int height  = rec.height();
    int width = rec.width();
    this->setGeometry(0,0,width,height);
    iLimitWidth = 8*width/10;
    iLimitHeight = 9*height/10;


    buttonEasy = new QPushButton(this);
    buttonEasy->setText("Easy\n\n8x8\n\n10 mines");
    buttonEasy->setFont(QFont("Ubuntu",20));

    buttonMedium= new QPushButton(this);
    buttonMedium->setText("Medium\n\n16x16\n\n40mines");
    buttonMedium->setFont(QFont("Ubuntu",20));


    buttonHard = new QPushButton(this);
    buttonHard->setText("Hard\n\n30x16\n\n99Mines");
    buttonHard->setFont(QFont("Ubuntu",20));

    buttonCustomizedNewGame = new QPushButton(this);
    buttonCustomizedNewGame->setText("Customized New Game");
    buttonCustomizedNewGame->setFont(QFont("Ubuntu",20));

    sbCustomizedX = new QSpinBox(this);
    sbCustomizedX->setMinimum(10);
    sbCustomizedX->setMaximum(100);
    sbCustomizedX->setValue(20);

    sbCustomizedY = new QSpinBox(this);
    sbCustomizedY->setMinimum(10);
    sbCustomizedY->setMaximum(100);
    sbCustomizedY->setValue(20);


    sbCustomizednMines = new QSpinBox(this);
    sbCustomizednMines->setMinimum(0);
    sbCustomizednMines->setMaximum(100);
    sbCustomizednMines->setValue(20);


    labelCustomizedX = new QLabel(this);
    labelCustomizedX->setText("Width: ");

    labelCustomizedY = new QLabel(this);
    labelCustomizedY->setText("Height: ");

    labelCustomized_nMines = new QLabel(this);
    labelCustomized_nMines->setText("Percent of Mines_Bollos: ");

    cbFirstCellClean = new QCheckBox(this);
    cbFirstCellClean->setText("First Cell Clean");
    cbFirstCellClean->setChecked(false);

    cbDarkModeEnabled = new QCheckBox(this);
    cbDarkModeEnabled->setText("Disable Dark Mode");
    cbDarkModeEnabled->setChecked(true);

    buttonEasy->setGeometry(width/20, height/20,
                            width/2 - 2*width/20, 4*(height/2 - 2*height/20)/5 );

    buttonMedium->setGeometry(buttonEasy->x() + buttonEasy->width() + width/20, height/20,
                              width/2 - 2*width/20, 4*(height/2 - 2*height/20)/5 );

    buttonHard->setGeometry(width/20, buttonEasy->y() + buttonEasy->height() + height/20,
                            width/2 - 2*width/20,  4*(height/2 - 2*height/20)/5 );

    cbFirstCellClean->setGeometry(width/20, buttonHard->y() + buttonHard->height() + height/20,
                                  width/2 - 2*width/20,  (height/2 - 2*height/20)/10 );

    cbDarkModeEnabled->setGeometry(width/20, cbFirstCellClean->y() + cbFirstCellClean->height(),
                                   width/2 - 2*width/20,  (height/2 - 2*height/20)/10);

    buttonCustomizedNewGame->setGeometry(buttonEasy->x() + buttonEasy->width() + width/20, buttonEasy->y() + buttonEasy->height() + height/20,
                                         width/2 - 2*width/20, 2*(height/2 - 2*height/20)/5 );

    labelCustomized_nMines->setGeometry(buttonCustomizedNewGame->x(), buttonCustomizedNewGame->y()+buttonCustomizedNewGame->height(),
                                        buttonCustomizedNewGame->width()/2,2*(height/2 - 2*height/20)/(3*5));

    labelCustomizedX->setGeometry(labelCustomized_nMines->x(), labelCustomized_nMines->y()+labelCustomized_nMines->height(),
                                  labelCustomized_nMines->width(),labelCustomized_nMines->height());

    labelCustomizedY->setGeometry(labelCustomizedX->x(), labelCustomizedX->y()+labelCustomizedX->height(),
                                  labelCustomizedX->width(),labelCustomizedX->height());

    sbCustomizednMines->setGeometry(labelCustomized_nMines->x()+labelCustomized_nMines->width(), labelCustomized_nMines->y(),
                                    labelCustomized_nMines->width(), labelCustomized_nMines->height());

    sbCustomizedX->setGeometry(labelCustomizedX->x()+labelCustomizedX->width(), labelCustomizedX->y(),
                               labelCustomizedX->width(), labelCustomizedX->height());

    sbCustomizedY->setGeometry(labelCustomizedY->x()+labelCustomizedY->width(), labelCustomizedY->y(),
                               labelCustomizedY->width(), labelCustomizedY->height());


    connect(buttonEasy, &QPushButton::pressed,
            this, &MinesBollos::SLOT_Easy);

    connect(buttonMedium, &QPushButton::pressed, this,
            &MinesBollos::SLOT_Medium);

    connect(buttonHard, &QPushButton::pressed,
            this, &MinesBollos::SLOT_Hard);

    connect(buttonCustomizedNewGame, &QPushButton::pressed,
            this, &MinesBollos::SLOT_Customized);

    connect(buttonRestartInGame, &QPushButton::pressed,
            this, &MinesBollos::SLOT_Restart);

    connect(buttonQuitInGame, &QPushButton::pressed,
            this, &MinesBollos::SLOT_Quit);

    connect(cbFirstCellClean, &QPushButton::released,
            this, &MinesBollos::SLOT_UpdateFirstCellClean);

    connect(cbDarkModeEnabled, &QPushButton::released,
            this, &MinesBollos::SLOT_DarkMode);

    labelMinesBollos = new QLabel(this);
    labelMinesBollos->setText("Mines\nBollos");
    labelMinesBollos->setFont(QFont("Ubuntu", 15));
    labelMinesBollos->setGeometry(9*width/10, 85*height/100, 2*width/10, height/10);
    labelMinesBollos->show();


}

void MinesBollos::vHideInterface()
{
    buttonEasy->hide();
    buttonMedium->hide();
    buttonHard->hide();

    buttonCustomizedNewGame->hide();

    sbCustomizedX->hide();
    sbCustomizedY->hide();
    sbCustomizednMines->hide();

    labelCustomizedX->hide();
    labelCustomizedY->hide();
    labelCustomized_nMines->hide();

    cbFirstCellClean->hide();
    cbDarkModeEnabled->hide();
}

void MinesBollos::vShowInterface()
{
    buttonEasy->show();
    buttonMedium->show();
    buttonHard->show();

    buttonCustomizedNewGame->show();

    sbCustomizedX->show();
    sbCustomizedY->show();
    sbCustomizednMines->show();

    labelCustomizedX->show();
    labelCustomizedY->show();
    labelCustomized_nMines->show();

    cbFirstCellClean->show();
    cbDarkModeEnabled->show();
}

void MinesBollos::SLOT_Easy()
{
    vHideInterface();

    timerTimeInGame = new QTimer(this);

    vNewGame(8, 8, 10);

    difficult = EASY;
}

void MinesBollos::SLOT_Medium()
{
    vHideInterface();

    timerTimeInGame = new QTimer(this);

    bFirst = true;

    vNewGame(16, 16, 40);

    difficult = MEDIUM;

}

void MinesBollos::SLOT_Hard()
{
    vHideInterface();

    timerTimeInGame = new QTimer(this);

    bFirst = true;

    vNewGame(30, 16, 99);

    difficult = HARD;

}

void MinesBollos::SLOT_Customized()
{
    vHideInterface();

    timerTimeInGame = new QTimer(this);

    bFirst = true;

    int x = sbCustomizedX->value();
    int y = sbCustomizedY->value();
    int Mines_Bollos = static_cast<int> (round(x*y*sbCustomizednMines->value()/100));
    vNewGame(x,y,Mines_Bollos);

    difficult = CUSTOMIZED;

}

void MinesBollos::vAjustInterfaceInGame()
{
    int width = this->width();
    int height = this->height();

    labelTimerInGame->setGeometry(85*width/100, height/20, 15*width/100, height/8);
    lcd_numberMinesLeft->setGeometry(labelTimerInGame->x(), labelTimerInGame->y()+labelTimerInGame->height(), labelTimerInGame->width(), height/7);
    buttonRestartInGame->setGeometry(lcd_numberMinesLeft->x(), lcd_numberMinesLeft->y()+lcd_numberMinesLeft->height(), lcd_numberMinesLeft->width()/2, height/20);
    buttonQuitInGame->setGeometry(buttonRestartInGame->x()+buttonRestartInGame->width(), buttonRestartInGame->y(), buttonRestartInGame->width(), buttonRestartInGame->height());
    labelYouWonYouLost->setGeometry(lcd_numberMinesLeft->x(), buttonRestartInGame->y()+buttonRestartInGame->height()+height/10, lcd_numberMinesLeft->width(), lcd_numberMinesLeft->height());
    labelStatisLastMatch->setGeometry(labelYouWonYouLost->x(), labelYouWonYouLost->y() + labelYouWonYouLost->height(),
                                      labelYouWonYouLost->width(), height/5);
}


void MinesBollos::vHideInterfaceInGame()
{
    labelTimerInGame->hide();
    lcd_numberMinesLeft->hide();
    buttonRestartInGame->hide();
    buttonQuitInGame->hide();
    labelYouWonYouLost->hide();
    labelStatisLastMatch->hide();

}

void MinesBollos::vShowInterfaceInGame()
{
    labelTimerInGame->show();
    lcd_numberMinesLeft->show();
    buttonRestartInGame->show();
    buttonQuitInGame->show();
    labelYouWonYouLost->show();
    labelStatisLastMatch->show();

}

void MinesBollos::SLOT_Restart()
{
    if(timerTimeInGame->isActive()
       )
        timerTimeInGame->stop();

    delete timerTimeInGame;

    timerTimeInGame = new QTimer (this);

    vResetPrincipalMatrix();

    labelStatisLastMatch->setText(" ");

    bFirst = true;

    vNewGame(iX, iY, nMines);
}

void MinesBollos::SLOT_Quit(){

    if (timerTimeInGame->isActive())
        timerTimeInGame->stop();

    delete timerTimeInGame;

    labelStatisLastMatch->setText(" ");

    vResetPrincipalMatrix();
    vHideInterfaceInGame();
    vShowInterface();

    difficult = NONE;
}

void MinesBollos::SLOT_UpdateTime()
{

    iTimeInSeconds++;
    labelTimerInGame->setNum(iTimeInSeconds);
}



void MinesBollos::vAddOrRemoveFlag(const uchar& _X, const uchar& _Y)
{
    switch (principalMatrix[_X][_Y].hasFlag){
        case false:
            principalMatrix[_X][_Y].button->setIcon(QIcon(QPixmap::fromImage(*imgFlag).scaled(fm, fm, Qt::KeepAspectRatio)));
            principalMatrix[_X][_Y].button->setIconSize(QSize(fm, fm));
            principalMatrix[_X][_Y].hasFlag = true;
            iMinesLeft--;
            lcd_numberMinesLeft->display(iMinesLeft);
            break;

        case true:
            principalMatrix[_X][_Y].button->setIcon(QIcon(QPixmap::fromImage(*imgNoFlag).scaled(fm, fm, Qt::KeepAspectRatio)));
            principalMatrix[_X][_Y].button->setIconSize(QSize(fm, fm));
            principalMatrix[_X][_Y].hasFlag = false;
            iMinesLeft++;
            lcd_numberMinesLeft->display(iMinesLeft);
            break;
    }
}

void MinesBollos::SLOT_OnButtonClicked(const QMouseEvent *e)
{
    QPushButton_adapted *buttonClicked = (QPushButton_adapted *) sender();

    for(uchar j=0; j<iY; j++)
    {
        for (uchar i=0; i<iX; i++)
        {
            if(buttonClicked == principalMatrix[i][j].button)
            {
                switch (e->button())
                {
                    case Qt::RightButton:

                        vAddOrRemoveFlag(i, j);
                        return;

                    case Qt::LeftButton:

                        if(bFirst)
                        {
                            if(bFirstCellClean  &&  principalMatrix[i][j].state != ZERO)
                            {
                                vNewGame(iX, iY, nMines, i, j);
                            }
                            bFirst = false;
                            vStartTimer();
                        }

                        if(principalMatrix[i][j].state == MINE)
                        {
                            if(!principalMatrix[i][j].hasFlag)
                                vGameLost(i, j);
                        }
                        else
                            vCellClean(i, j);

                        return;

                    default:
                        return;
                }
            }
        }
    }
}

void MinesBollos::vStartTimer()
{
    iTimeInSeconds = 0;
    timerTimeInGame->start(1e3);

    connect(timerTimeInGame, &QTimer::timeout,
            this, &MinesBollos::SLOT_UpdateTime);
}

void MinesBollos::SLOT_UpdateFirstCellClean()
{
    bFirstCellClean = cbFirstCellClean->isChecked();
}

void MinesBollos::SLOT_DarkMode()
{
    vConfigureDarkMode(cbDarkModeEnabled->isChecked());

    if(cbDarkModeEnabled->isChecked())
        cbDarkModeEnabled->setText("Disable dark mode");

    else
        cbDarkModeEnabled->setText("Enable dark mode");
}

void MinesBollos::vGenerateStatics()
{
    int iCorrectFlags = 0,
            iWrongFlags = 0,
            iUnlockedCells = iCellsToUnlock - iHiddenCells;

    for (int i=0; i<(iX-1); i++){
        for (int j=0; j<=(iY-1); j++){
            if(principalMatrix[i][j].hasFlag == true){
                if (principalMatrix[i][j].state == MINE)
                    iCorrectFlags++;
                else
                    iWrongFlags++;
            }
        }
    }
    if(iTimeInSeconds == 0)
        iTimeInSeconds = 1;

    double dFlagsPerSecond = (double)iCorrectFlags/iTimeInSeconds,
            dCellsPerSecond = (double)iUnlockedCells/iTimeInSeconds,
            dPercentageGameComplete = (double)100*iUnlockedCells/iCellsToUnlock;

    QString QS_Statics = "Correct Flags: " + QString::number(iCorrectFlags)
                         +"\nWrongFlags: " + QString::number(iWrongFlags)
                         +"\nUnlocked Cells: " + QString::number(iUnlockedCells)
                         +"\n"
                         +"\nFlags/s: " + QString::number(dFlagsPerSecond, 'f', 3)
                         +"\nCells/s: " + QString::number(dCellsPerSecond, 'f', 3)
                         +"\n"
                         +"\nGame Complete: " + QString::number(dPercentageGameComplete, 'f', 2) + " %";

    labelStatisLastMatch->setText(QS_Statics);
}

void MinesBollos::vConfigureDarkMode(const bool bDark)
{
    if(bDark){
        qApp->setStyle (QStyleFactory::create ("Fusion"));
        QPalette darkPalette;
        darkPalette.setColor (QPalette::BrightText,      Qt::red);
        darkPalette.setColor (QPalette::WindowText,      Qt::white);
        darkPalette.setColor (QPalette::ToolTipBase,     Qt::white);
        darkPalette.setColor (QPalette::ToolTipText,     Qt::white);
        darkPalette.setColor (QPalette::Text,            Qt::white);
        darkPalette.setColor (QPalette::ButtonText,      Qt::white);
        darkPalette.setColor (QPalette::HighlightedText, Qt::black);
        darkPalette.setColor (QPalette::Window,          QColor (53, 53, 53));
        darkPalette.setColor (QPalette::Base,            QColor (25, 25, 25));
        darkPalette.setColor (QPalette::AlternateBase,   QColor (53, 53, 53));
        darkPalette.setColor (QPalette::Button,          QColor (53, 53, 53));
        darkPalette.setColor (QPalette::Link,            QColor (42, 130, 218));
        darkPalette.setColor (QPalette::Highlight,       QColor (42, 130, 218));

        qApp->setPalette(darkPalette);

    }

    else {

        qApp->setStyle (QStyleFactory::create ("Fusion"));
        QPalette lightPalette;
        lightPalette.setColor (QPalette::BrightText,      Qt::cyan);
        lightPalette.setColor (QPalette::WindowText,      Qt::black);
        lightPalette.setColor (QPalette::ToolTipBase,     Qt::black);
        lightPalette.setColor (QPalette::ToolTipText,     Qt::black);
        lightPalette.setColor (QPalette::Text,            Qt::black);
        lightPalette.setColor (QPalette::ButtonText,      Qt::black);
        lightPalette.setColor (QPalette::HighlightedText, Qt::white);
        lightPalette.setColor (QPalette::Window,          QColor (202, 202, 202));
        lightPalette.setColor (QPalette::Base,            QColor (228, 228, 228));
        lightPalette.setColor (QPalette::AlternateBase,   QColor (202, 202, 202));
        lightPalette.setColor (QPalette::Button,          QColor (202, 202, 202));
        lightPalette.setColor (QPalette::Link,            QColor (213, 125, 37));
        lightPalette.setColor (QPalette::Highlight,       QColor (213, 225, 37));

        qApp->setPalette(lightPalette);

    }

    imgZero->invertPixels();
    imgOne->invertPixels();
    imgTwo->invertPixels();
    imgThree->invertPixels();
    imgFour->invertPixels();
    imgFive->invertPixels();
    imgSix->invertPixels();
    imgSeven->invertPixels();
    imgEight->invertPixels();
    imgFlag->invertPixels();
    imgNoFlag->invertPixels();
    imgMine->invertPixels();
    imgBoom->invertPixels();
    imgWrongFlag->invertPixels();
}
