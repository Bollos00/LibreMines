#include "mines_bollos.h"

Mines_Bollos::Mines_Bollos(QWidget *parent)
    : QMainWindow(parent)
{
    QI_Zero = new QImage;
    QI_One = new QImage;
    QI_Two = new QImage;
    QI_Three = new QImage;
    QI_Four= new QImage;
    QI_Five = new QImage;
    QI_Six= new QImage;
    QI_Seven = new QImage;
    QI_Eight = new QImage;
    QI_Flag = new QImage;
    QI_NoFlag = new QImage;
    QI_Mine = new QImage;
    QI_Boom = new QImage;
    QI_WrongFlag = new QImage;

    QI_Zero->load(":/Media_rsc/Media/Minesweeper_zero.png");
    QI_One->load(":/Media_rsc/Media/Minesweeper_one.png");
    QI_Two->load(":/Media_rsc/Media/Minesweeper_two.png");
    QI_Three->load(":/Media_rsc/Media/Minesweeper_three.png");
    QI_Four->load(":/Media_rsc/Media/Minesweeper_four.png");
    QI_Five->load(":/Media_rsc/Media/Minesweeper_five.png");
    QI_Six->load(":/Media_rsc/Media/Minesweeper_six.png");
    QI_Seven->load(":/Media_rsc/Media/Minesweeper_seven.png");
    QI_Eight->load(":/Media_rsc/Media/Minesweeper_eight.png");
    QI_Flag->load(":/Media_rsc/Media/Minesweeper_flag.png");
    QI_NoFlag->load(":/Media_rsc/Media/Minesweeper_no_flag.png");
    QI_Mine->load(":/Media_rsc/Media/Minesweeper_mine.png");
    QI_Boom->load(":/Media_rsc/Media/Minesweeper_boom.png");
    QI_WrongFlag->load(":/Media_rsc/Media/Minesweeper_wrong_flag.png");

    vConfigureInterface();

    vConfigureDarkMode(true);

    bFirst = false;
    bFirstCellClean = false;
    GAME_STATE = NONE;
}

Mines_Bollos::~Mines_Bollos()
{

}

void Mines_Bollos::vNewGame(int _X , int _Y, int i_nMines_, int i_X_Clean, int i_Y_Clean){

    QPB_QuitInGame->setEnabled(false);
    QPB_RestartInGame->setEnabled(false);

    i_X =_X;
    i_Y =_Y;
    i_nMines = i_nMines_;
    i_MinesLeft = i_nMines_;
    i_HiddenCells = 0;

    bool bRemakingGame = false;
    if( i_X_Clean != -2 && i_Y_Clean != -2)
        bRemakingGame = true;

    if(!bRemakingGame)
        Principal_Matrix= std::vector<std::vector<Cell>> (i_X, std::vector<Cell>(i_Y));

    if(i_limit_width/i_X < i_limit_height/i_Y)
        fm = i_limit_width/i_X;
    else
        fm = i_limit_height/i_Y;

    QPixmap QPM_Zero = QPixmap::fromImage(*QI_Zero).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_One = QPixmap::fromImage(*QI_One).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Two = QPixmap::fromImage(*QI_Two).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Three = QPixmap::fromImage(*QI_Three).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Four = QPixmap::fromImage(*QI_Four).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Five = QPixmap::fromImage(*QI_Five).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Six = QPixmap::fromImage(*QI_Six).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Seven = QPixmap::fromImage(*QI_Seven).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Eight = QPixmap::fromImage(*QI_Eight).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_NoFlag = QPixmap::fromImage(*QI_NoFlag).scaled(fm, fm, Qt::KeepAspectRatio),
            QPM_Mine = QPixmap::fromImage(*QI_Mine).scaled(fm, fm, Qt::KeepAspectRatio);

    if(bRemakingGame){

        for(int j=0; j<i_Y; j++){
            for (int i=0; i<i_X; i++){

                Principal_Matrix[i][j].S_Cell = ZERO;
            }
        }
    }
    else {

        for(int j=0; j<i_Y; j++){
            for (int i=0; i<i_X; i++){

                Principal_Matrix[i][j].QL = new QLabel(this);
                Principal_Matrix[i][j].QPB_a = new QPushButton_adapted(this);

                Principal_Matrix[i][j].QL->setGeometry(i*fm,j*fm,fm,fm);
                Principal_Matrix[i][j].QL->show();

                Principal_Matrix[i][j].QPB_a->setGeometry(i*fm,j*fm,fm,fm);
                Principal_Matrix[i][j].QPB_a->setIcon(QIcon(QPM_NoFlag));
                Principal_Matrix[i][j].QPB_a->setIconSize(QSize(fm, fm));
                Principal_Matrix[i][j].QPB_a->show();
                Principal_Matrix[i][j].QPB_a->setEnabled(false);

                Principal_Matrix[i][j].S_Cell = ZERO;
                Principal_Matrix[i][j].bHiddenState = true;
                Principal_Matrix[i][j].bFlag = false;

                connect(Principal_Matrix[i][j].QPB_a, SIGNAL(SIGNAL_Clicked(QMouseEvent*)),this,SLOT(SLOT_OnQPushButton_adapteded_Clicked(QMouseEvent*)));
                qApp->processEvents();
            }
        }
    }


    QVector<QVector2D> vt_vt2d_CleanPoints;
    vt_vt2d_CleanPoints.clear();

    if(bRemakingGame){

        for (int i=-1; i<=1; i++){
            for (int j=-1; j<=1; j++){

                vt_vt2d_CleanPoints.append(QVector2D(i_X_Clean + i, i_Y_Clean + j));
            }
        }
    }

    while(i_nMines_>0){

        int i = QRandomGenerator::global()->bounded(0,i_X);
        int j = QRandomGenerator::global()->bounded(0,i_Y);

        bool bPointClean = false;

        if(bRemakingGame){

            for (auto n: vt_vt2d_CleanPoints){
                if(static_cast<int>(n.x()) == i && static_cast<int>(n.y()) == j){
                    bPointClean = true;
                    break;
                }
            }
        }

        if(Principal_Matrix[i][j].S_Cell == ZERO && bPointClean == false){

            i_nMines_--;
            Principal_Matrix[i][j].S_Cell = MINE;
            Principal_Matrix[i][j].QL->setPixmap(QPM_Mine);
        }
    }
    for(int j=0; j<i_Y; j++){
        for (int i=0; i<i_X; i++){

            Principal_Matrix[i][j].QPB_a->setEnabled(true);

            if(Principal_Matrix[i][j].S_Cell == ZERO){

                i_HiddenCells++;

                int auxiliar = 0;
                if(i==0 && j==0){
                    if(Principal_Matrix[i+1][j].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i][j+1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i+1][j+1].S_Cell == MINE)
                        auxiliar++;

                    switch (auxiliar) {
                        case 0:
                            Principal_Matrix[i][j].S_Cell = ZERO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Zero);


                            break;
                        case 1:
                            Principal_Matrix[i][j].S_Cell = ONE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_One);


                            break;
                        case 2:
                            Principal_Matrix[i][j].S_Cell = TWO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Two);


                            break;
                        case 3:
                            Principal_Matrix[i][j].S_Cell = THREE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Three);


                            break;
                        case 4:
                            Principal_Matrix[i][j].S_Cell = FOUR;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Four);


                            break;
                        case 5:
                            Principal_Matrix[i][j].S_Cell = FIVE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Five);


                            break;
                        case 6:
                            Principal_Matrix[i][j].S_Cell = SIX;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Six);


                            break;
                        case 7:
                            Principal_Matrix[i][j].S_Cell = SEVEN;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Seven);


                            break;
                        case 8:
                            Principal_Matrix[i][j].S_Cell = EIGHT;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Eight);


                            break;
                    }
                }
                else if(i==0 && j==(i_Y-1)){
                    if(Principal_Matrix[i+1][j].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i][j-1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i+1][j-1].S_Cell == MINE)
                        auxiliar++;

                    switch (auxiliar) {
                        case 0:
                            Principal_Matrix[i][j].S_Cell = ZERO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Zero);


                            break;
                        case 1:
                            Principal_Matrix[i][j].S_Cell = ONE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_One);


                            break;
                        case 2:
                            Principal_Matrix[i][j].S_Cell = TWO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Two);


                            break;
                        case 3:
                            Principal_Matrix[i][j].S_Cell = THREE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Three);


                            break;
                        case 4:
                            Principal_Matrix[i][j].S_Cell = FOUR;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Four);


                            break;
                        case 5:
                            Principal_Matrix[i][j].S_Cell = FIVE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Five);


                            break;
                        case 6:
                            Principal_Matrix[i][j].S_Cell = SIX;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Six);


                            break;
                        case 7:
                            Principal_Matrix[i][j].S_Cell = SEVEN;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Seven);


                            break;
                        case 8:
                            Principal_Matrix[i][j].S_Cell = EIGHT;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Eight);


                            break;
                    }
                }
                else if(i==(i_X-1) && j==0){
                    if(Principal_Matrix[i-1][j].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i][j+1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i-1][j+1].S_Cell == MINE)
                        auxiliar++;

                    switch (auxiliar) {
                        case 0:
                            Principal_Matrix[i][j].S_Cell = ZERO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Zero);


                            break;
                        case 1:
                            Principal_Matrix[i][j].S_Cell = ONE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_One);


                            break;
                        case 2:
                            Principal_Matrix[i][j].S_Cell = TWO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Two);


                            break;
                        case 3:
                            Principal_Matrix[i][j].S_Cell = THREE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Three);


                            break;
                        case 4:
                            Principal_Matrix[i][j].S_Cell = FOUR;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Four);


                            break;
                        case 5:
                            Principal_Matrix[i][j].S_Cell = FIVE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Five);


                            break;
                        case 6:
                            Principal_Matrix[i][j].S_Cell = SIX;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Six);


                            break;
                        case 7:
                            Principal_Matrix[i][j].S_Cell = SEVEN;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Seven);


                            break;
                        case 8:
                            Principal_Matrix[i][j].S_Cell = EIGHT;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Eight);


                            break;
                    }
                }
                else if(i==(i_X-1) && j==(i_Y-1)){
                    if(Principal_Matrix[i-1][j].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i][j-1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i-1][j-1].S_Cell == MINE)
                        auxiliar++;

                    switch (auxiliar) {
                        case 0:
                            Principal_Matrix[i][j].S_Cell = ZERO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Zero);


                            break;
                        case 1:
                            Principal_Matrix[i][j].S_Cell = ONE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_One);


                            break;
                        case 2:
                            Principal_Matrix[i][j].S_Cell = TWO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Two);


                            break;
                        case 3:
                            Principal_Matrix[i][j].S_Cell = THREE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Three);


                            break;
                        case 4:
                            Principal_Matrix[i][j].S_Cell = FOUR;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Four);


                            break;
                        case 5:
                            Principal_Matrix[i][j].S_Cell = FIVE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Five);


                            break;
                        case 6:
                            Principal_Matrix[i][j].S_Cell = SIX;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Six);


                            break;
                        case 7:
                            Principal_Matrix[i][j].S_Cell = SEVEN;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Seven);


                            break;
                        case 8:
                            Principal_Matrix[i][j].S_Cell = EIGHT;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Eight);


                            break;
                    }
                }
                else if(i==0 && j>0 && j<(i_Y-1)){
                    if(Principal_Matrix[i+1][j].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i][j+1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i+1][j+1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i][j-1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i+1][j-1].S_Cell == MINE)
                        auxiliar++;

                    switch (auxiliar) {
                        case 0:
                            Principal_Matrix[i][j].S_Cell = ZERO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Zero);


                            break;
                        case 1:
                            Principal_Matrix[i][j].S_Cell = ONE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_One);


                            break;
                        case 2:
                            Principal_Matrix[i][j].S_Cell = TWO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Two);


                            break;
                        case 3:
                            Principal_Matrix[i][j].S_Cell = THREE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Three);


                            break;
                        case 4:
                            Principal_Matrix[i][j].S_Cell = FOUR;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Four);


                            break;
                        case 5:
                            Principal_Matrix[i][j].S_Cell = FIVE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Five);


                            break;
                        case 6:
                            Principal_Matrix[i][j].S_Cell = SIX;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Six);


                            break;
                        case 7:
                            Principal_Matrix[i][j].S_Cell = SEVEN;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Seven);


                            break;
                        case 8:
                            Principal_Matrix[i][j].S_Cell = EIGHT;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Eight);


                            break;
                    }
                }
                else if(i==(i_X-1) && j>0 && j<(i_Y-1)){
                    if(Principal_Matrix[i-1][j].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i][j+1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i-1][j+1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i][j-1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i-1][j-1].S_Cell == MINE)
                        auxiliar++;

                    switch (auxiliar) {
                        case 0:
                            Principal_Matrix[i][j].S_Cell = ZERO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Zero);


                            break;
                        case 1:
                            Principal_Matrix[i][j].S_Cell = ONE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_One);


                            break;
                        case 2:
                            Principal_Matrix[i][j].S_Cell = TWO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Two);


                            break;
                        case 3:
                            Principal_Matrix[i][j].S_Cell = THREE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Three);


                            break;
                        case 4:
                            Principal_Matrix[i][j].S_Cell = FOUR;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Four);


                            break;
                        case 5:
                            Principal_Matrix[i][j].S_Cell = FIVE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Five);


                            break;
                        case 6:
                            Principal_Matrix[i][j].S_Cell = SIX;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Six);


                            break;
                        case 7:
                            Principal_Matrix[i][j].S_Cell = SEVEN;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Seven);


                            break;
                        case 8:
                            Principal_Matrix[i][j].S_Cell = EIGHT;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Eight);


                            break;
                    }
                }
                else if(i>0  && i<(i_X-1) && j==0){
                    if(Principal_Matrix[i-1][j].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i+1][j].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i-1][j+1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i][j+1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i+1][j+1].S_Cell == MINE)
                        auxiliar++;

                    switch (auxiliar) {
                        case 0:
                            Principal_Matrix[i][j].S_Cell = ZERO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Zero);


                            break;
                        case 1:
                            Principal_Matrix[i][j].S_Cell = ONE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_One);


                            break;
                        case 2:
                            Principal_Matrix[i][j].S_Cell = TWO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Two);


                            break;
                        case 3:
                            Principal_Matrix[i][j].S_Cell = THREE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Three);


                            break;
                        case 4:
                            Principal_Matrix[i][j].S_Cell = FOUR;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Four);


                            break;
                        case 5:
                            Principal_Matrix[i][j].S_Cell = FIVE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Five);


                            break;
                        case 6:
                            Principal_Matrix[i][j].S_Cell = SIX;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Six);


                            break;
                        case 7:
                            Principal_Matrix[i][j].S_Cell = SEVEN;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Seven);


                            break;
                        case 8:
                            Principal_Matrix[i][j].S_Cell = EIGHT;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Eight);


                            break;
                    }
                }
                else if(i>0 && i<(i_X-1) && j==(i_Y-1)){
                    if(Principal_Matrix[i+1][j].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i-1][j].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i-1][j-1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i][j-1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i+1][j-1].S_Cell == MINE)
                        auxiliar++;

                    switch (auxiliar) {
                        case 0:
                            Principal_Matrix[i][j].S_Cell = ZERO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Zero);


                            break;
                        case 1:
                            Principal_Matrix[i][j].S_Cell = ONE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_One);


                            break;
                        case 2:
                            Principal_Matrix[i][j].S_Cell = TWO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Two);


                            break;
                        case 3:
                            Principal_Matrix[i][j].S_Cell = THREE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Three);


                            break;
                        case 4:
                            Principal_Matrix[i][j].S_Cell = FOUR;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Four);


                            break;
                        case 5:
                            Principal_Matrix[i][j].S_Cell = FIVE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Five);


                            break;
                        case 6:
                            Principal_Matrix[i][j].S_Cell = SIX;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Six);


                            break;
                        case 7:
                            Principal_Matrix[i][j].S_Cell = SEVEN;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Seven);


                            break;
                        case 8:
                            Principal_Matrix[i][j].S_Cell = EIGHT;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Eight);


                            break;
                    }
                }
                else {
                    if(Principal_Matrix[i-1][j-1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i-1][j].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i-1][j+1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i][j-1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i][j+1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i+1][j-1].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i+1][j].S_Cell == MINE)
                        auxiliar++;
                    if(Principal_Matrix[i+1][j+1].S_Cell == MINE)
                        auxiliar++;

                    switch (auxiliar) {
                        case 0:
                            Principal_Matrix[i][j].S_Cell = ZERO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Zero);


                            break;
                        case 1:
                            Principal_Matrix[i][j].S_Cell = ONE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_One);


                            break;
                        case 2:
                            Principal_Matrix[i][j].S_Cell = TWO;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Two);


                            break;
                        case 3:
                            Principal_Matrix[i][j].S_Cell = THREE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Three);


                            break;
                        case 4:
                            Principal_Matrix[i][j].S_Cell = FOUR;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Four);


                            break;
                        case 5:
                            Principal_Matrix[i][j].S_Cell = FIVE;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Five);


                            break;
                        case 6:
                            Principal_Matrix[i][j].S_Cell = SIX;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Six);


                            break;
                        case 7:
                            Principal_Matrix[i][j].S_Cell = SEVEN;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Seven);


                            break;
                        case 8:
                            Principal_Matrix[i][j].S_Cell = EIGHT;
                            Principal_Matrix[i][j].QL->setPixmap(QPM_Eight);


                            break;
                    }
                }
            }
        }
    }

    i_CellsToUnlock = i_HiddenCells;

    vAjustInterfaceInGame();
    vShowInterfaceInGame();
    QLCDN_MinesLeft->display(i_MinesLeft);
    QL_TimerInGame->setNum(0);
    QL_YouWon_YouLost->setText(" ");

    QPB_QuitInGame->setEnabled(true);
    QPB_RestartInGame->setEnabled(true);
}



void Mines_Bollos::vGameLost(int _X, int _Y){

    qDebug()<<"You Lost";

    QTIMER_TimeInGame->stop();

    switch (GAME_STATE) {
        case NONE:
            break;
        case EASY:
            QL_YouWon_YouLost->setText("You Lost\nDifficulty: EASY");
            break;
        case MEDIUM:
            QL_YouWon_YouLost->setText("You Lost\nDifficulty: MEDIUM");
            break;
        case HARD:
            QL_YouWon_YouLost->setText("You Lost\nDifficulty: HARD");
            break;
        case CUSTOMIZED:
            QL_YouWon_YouLost->setText("You Lost\nDifficulty: CUSTOM\n" + QString::number(i_X) + "x" + QString::number(i_Y) + " : " + QString::number(i_nMines) + " Mines");
    }


    Principal_Matrix[_X][_Y].QL->setPixmap(QPixmap::fromImage(*QI_Boom).scaled(fm, fm, Qt::KeepAspectRatio));

    vGenerateStatics();

    for(int j=0; j<i_Y; j++){
        for (int i=0; i<i_X; i++){
            if(Principal_Matrix[i][j].bHiddenState == true){

                if(Principal_Matrix[i][j].S_Cell == MINE && Principal_Matrix[i][j].bFlag == false)
                    Principal_Matrix[i][j].QPB_a->hide();

                else if (Principal_Matrix[i][j].S_Cell != MINE && Principal_Matrix[i][j].bFlag == true){
                    Principal_Matrix[i][j].QPB_a->hide();
                    Principal_Matrix[i][j].QL->setPixmap(QPixmap::fromImage(*QI_WrongFlag).scaled(fm, fm, Qt::KeepAspectRatio));
                }
                else
                    Principal_Matrix[i][j].QPB_a->setEnabled(false);
            }
        }
    }

}

void Mines_Bollos::vGameWon(){

    qDebug()<<"You won: "<<i_TimeInSeconds<<" Seconds";

    QTIMER_TimeInGame->stop();

    switch (GAME_STATE) {
        case NONE:
            break;
        case EASY:
            QL_YouWon_YouLost->setText("You Won\nDifficulty: EASY");
            break;
        case MEDIUM:
            QL_YouWon_YouLost->setText("You Won\nDifficulty: MEDIUM");
            break;
        case HARD:
            QL_YouWon_YouLost->setText("You Won\nDifficulty: HARD");
            break;
        case CUSTOMIZED:
            QL_YouWon_YouLost->setText("You Won\nDifficulty: CUSTOM\n" + QString::number(i_X) + "x" + QString::number(i_Y) + " : " + QString::number(i_nMines) + " Mines");
    }

    vGenerateStatics();
    for(int j=0; j<i_Y; j++){
        for (int i=0; i<i_X; i++){
            if(Principal_Matrix[i][j].bHiddenState == true)
                Principal_Matrix[i][j].QPB_a->setEnabled(false);
        }
    }

}

void Mines_Bollos::vCasaLivre(int _X, int _Y){

    if(Principal_Matrix[_X][_Y] .bHiddenState == true && Principal_Matrix[_X][_Y].bFlag == false){
        Principal_Matrix[_X][_Y].bHiddenState = false;
        Principal_Matrix[_X][_Y].QPB_a->hide();

        if(Principal_Matrix[_X][_Y].S_Cell == ZERO){
            if (_X==0 && _Y==0){
                if(Principal_Matrix[_X+1][_Y].bHiddenState == true)
                    vCasaLivre(_X+1, _Y);
                if(Principal_Matrix[_X][_Y+1].bHiddenState == true)
                    vCasaLivre(_X, _Y+1);
                if(Principal_Matrix[_X+1][_Y+1].bHiddenState == true)
                    vCasaLivre(_X+1, _Y+1);
            }

            else if (_X==0 && _Y==(i_Y-1)){
                if(Principal_Matrix[_X+1][_Y].bHiddenState == true)
                    vCasaLivre(_X+1, _Y);
                if(Principal_Matrix[_X][_Y-1].bHiddenState == true)
                    vCasaLivre(_X, _Y-1);
                if(Principal_Matrix[_X+1][_Y-1].bHiddenState == true)
                    vCasaLivre(_X+1, _Y-1);


            }
            else if (_X==(i_X-1) && _Y==0){
                if(Principal_Matrix[_X-1][_Y].bHiddenState == true)
                    vCasaLivre(_X-1, _Y);
                if(Principal_Matrix[_X][_Y+1].bHiddenState == true)
                    vCasaLivre(_X, _Y+1);
                if(Principal_Matrix[_X-1][_Y+1].bHiddenState == true)
                    vCasaLivre(_X-1, _Y+1);
            }
            else if (_X==(i_X-1) && _Y==(i_Y-1)){
                if(Principal_Matrix[_X-1][_Y].bHiddenState == true)
                    vCasaLivre(_X-1, _Y);
                if(Principal_Matrix[_X][_Y-1].bHiddenState == true)
                    vCasaLivre(_X, _Y-1);
                if(Principal_Matrix[_X-1][_Y-1].bHiddenState == true)
                    vCasaLivre(_X-1, _Y-1);
            }
            else if(_X==(i_X-1) && _Y==0){
                if(Principal_Matrix[_X-1][_Y].bHiddenState == true)
                    vCasaLivre(_X-1, _Y);
                if(Principal_Matrix[_X][_Y+1].bHiddenState == true)
                    vCasaLivre(_X, _Y+1);
                if(Principal_Matrix[_X-1][_Y+1].bHiddenState == true)
                    vCasaLivre(_X-1, _Y+1);
            }
            else if (_X==(i_X-1) && _Y==(i_Y-1)){
                if(Principal_Matrix[_X-1][_Y].bHiddenState == true)
                    vCasaLivre(_X-1, _Y);
                if(Principal_Matrix[_X][_Y-1].bHiddenState == true)
                    vCasaLivre(_X, _Y-1);
                if(Principal_Matrix[_X-1][_Y-1].bHiddenState == true)
                    vCasaLivre(_X-1, _Y-1);
            }
            else if( _X==0 && _Y>0 && _Y<(i_Y-1)){
                if(Principal_Matrix[_X+1][_Y].bHiddenState == true)
                    vCasaLivre(_X+1, _Y);
                if(Principal_Matrix[_X][_Y+1].bHiddenState == true)
                    vCasaLivre(_X, _Y+1);
                if(Principal_Matrix[_X+1][_Y+1].bHiddenState == true)
                    vCasaLivre(_X+1, _Y+1);
                if(Principal_Matrix[_X][_Y-1].bHiddenState == true)
                    vCasaLivre(_X, _Y-1);
                if(Principal_Matrix[_X+1][_Y-1].bHiddenState == true)
                    vCasaLivre(_X+1, _Y-1);
            }
            else if (_X==(i_X-1) && _Y>0 && _Y<(i_Y-1)){
                if(Principal_Matrix[_X-1][_Y].bHiddenState == true)
                    vCasaLivre(_X-1, _Y);
                if(Principal_Matrix[_X][_Y+1].bHiddenState == true)
                    vCasaLivre(_X, _Y+1);
                if(Principal_Matrix[_X-1][_Y+1].bHiddenState == true)
                    vCasaLivre(_X-1, _Y+1);
                if(Principal_Matrix[_X][_Y-1].bHiddenState == true)
                    vCasaLivre(_X, _Y-1);
                if(Principal_Matrix[_X-1][_Y-1].bHiddenState == true)
                    vCasaLivre(_X-1, _Y-1);
            }
            else if (_X>0  && _X<(i_X-1) && _Y==0){
                if(Principal_Matrix[_X-1][_Y].bHiddenState == true)
                    vCasaLivre(_X-1, _Y);
                if(Principal_Matrix[_X+1][_Y].bHiddenState == true)
                    vCasaLivre(_X+1, _Y);
                if(Principal_Matrix[_X-1][_Y+1].bHiddenState == true)
                    vCasaLivre(_X-1, _Y+1);
                if(Principal_Matrix[_X][_Y+1].bHiddenState == true)
                    vCasaLivre(_X, _Y+1);
                if(Principal_Matrix[_X+1][_Y+1].bHiddenState == true)
                    vCasaLivre(_X+1, _Y+1);
            }
            else if(_X>0 && _X<(i_X-1) && _Y==(i_Y-1)){
                if(Principal_Matrix[_X+1][_Y].bHiddenState == true)
                    vCasaLivre(_X+1, _Y);
                if(Principal_Matrix[_X-1][_Y].bHiddenState == true)
                    vCasaLivre(_X-1, _Y);
                if(Principal_Matrix[_X-1][_Y-1].bHiddenState == true)
                    vCasaLivre(_X-1, _Y-1);
                if(Principal_Matrix[_X][_Y-1].bHiddenState == true)
                    vCasaLivre(_X, _Y-1);
                if(Principal_Matrix[_X+1][_Y-1].bHiddenState == true)
                    vCasaLivre(_X+1, _Y-1);
            }
            else {
                if(Principal_Matrix[_X-1][_Y-1].bHiddenState == true)
                    vCasaLivre(_X-1, _Y-1);
                if(Principal_Matrix[_X-1][_Y].bHiddenState == true)
                    vCasaLivre(_X-1, _Y);
                if(Principal_Matrix[_X-1][_Y+1].bHiddenState == true)
                    vCasaLivre(_X-1, _Y+1);
                if(Principal_Matrix[_X][_Y-1].bHiddenState == true)
                    vCasaLivre(_X, _Y-1);
                if(Principal_Matrix[_X][_Y+1].bHiddenState == true)
                    vCasaLivre(_X, _Y+1);
                if(Principal_Matrix[_X+1][_Y-1].bHiddenState == true)
                    vCasaLivre(_X+1, _Y-1);
                if(Principal_Matrix[_X+1][_Y].bHiddenState == true)
                    vCasaLivre(_X+1, _Y);
                if(Principal_Matrix[_X+1][_Y+1].bHiddenState == true)
                    vCasaLivre(_X+1, _Y+1);
            }

        }
        else {
            Principal_Matrix[_X][_Y].bHiddenState = false;
            Principal_Matrix[_X][_Y].QPB_a->hide();
        }
        i_HiddenCells--;
        if(i_HiddenCells ==0)
            vGameWon();
    }

}

void Mines_Bollos::vResetPrincipal_Matrix(){
    for(int j=0; j<i_Y; j++){
        for (int i=0; i<i_X; i++){
            if(Principal_Matrix[i][j].QL != nullptr)
                delete Principal_Matrix[i][j].QL;
            if(Principal_Matrix[i][j].QPB_a != nullptr)
                delete Principal_Matrix[i][j].QPB_a;
        }
    }
    Principal_Matrix.clear();

}

void Mines_Bollos::vConfigureInterface(){

    QL_TimerInGame = new QLabel (this);
    QLCDN_MinesLeft = new QLCDNumber (this);
    QPB_RestartInGame = new QPushButton (this);
    QPB_QuitInGame = new QPushButton (this);
    QL_YouWon_YouLost = new QLabel(this);
    QL_StatisLastMatch = new QLabel(this);

    QL_TimerInGame->setFont(QFont("Ubuntu", 40));
    QL_TimerInGame->setNum(0);
    QLCDN_MinesLeft->setDecMode();
    QLCDN_MinesLeft->display(0);
    QPB_QuitInGame->setText("Quit");
    QPB_RestartInGame->setText("Restart");
    QL_YouWon_YouLost->setFont(QFont("Ubuntu", 15));

    vHideInterfaceInGame();

    QRect rec = QApplication::desktop()->screenGeometry();
    int height  = rec.height();
    int width = rec.width();
    this->setGeometry(0,0,width,height);
    i_limit_width = 8*width/10;
    i_limit_height = 9*height/10;


    QPB_Easy = new QPushButton(this);
    QPB_Easy->setText("Easy\n\n8x8\n\n10 mines");
    QPB_Easy->setFont(QFont("Ubuntu",20));

    QPB_Medium= new QPushButton(this);
    QPB_Medium->setText("Medium\n\n16x16\n\n40mines");
    QPB_Medium->setFont(QFont("Ubuntu",20));


    QPB_Hard = new QPushButton(this);
    QPB_Hard->setText("Hard\n\n30x16\n\n99Mines");
    QPB_Hard->setFont(QFont("Ubuntu",20));

    QPB_CustomizedNewGame = new QPushButton(this);
    QPB_CustomizedNewGame->setText("Customized New Game");
    QPB_CustomizedNewGame->setFont(QFont("Ubuntu",20));

    QSB_Customized_X = new QSpinBox(this);
    QSB_Customized_X->setMinimum(10);
    QSB_Customized_X->setMaximum(40);
    QSB_Customized_X->setValue(20);

    QSB_Customized_Y = new QSpinBox(this);
    QSB_Customized_Y->setMinimum(10);
    QSB_Customized_Y->setMaximum(40);
    QSB_Customized_Y->setValue(20);


    QSB_Customized_nMines = new QSpinBox(this);
    QSB_Customized_nMines->setMinimum(0);
    QSB_Customized_nMines->setMaximum(100);
    QSB_Customized_nMines->setValue(20);


    QL_Customized_X = new QLabel(this);
    QL_Customized_X->setText("Width: ");

    QL_Customized_Y = new QLabel(this);
    QL_Customized_Y->setText("Height: ");

    QL_Customized_nMines = new QLabel(this);
    QL_Customized_nMines->setText("Percent of Mines_Bollos: ");

    QCB_FirstCellClean = new QCheckBox(this);
    QCB_FirstCellClean->setText("First Cell Clean");
    QCB_FirstCellClean->setChecked(false);

    QCB_DarkModeEnabled = new QCheckBox(this);
    QCB_DarkModeEnabled->setText("Disable Dark Mode");
    QCB_DarkModeEnabled->setChecked(true);

    QPB_Easy->setGeometry(width/20, height/20,
                          width/2 - 2*width/20, 4*(height/2 - 2*height/20)/5 );

    QPB_Medium->setGeometry(QPB_Easy->x() + QPB_Easy->width() + width/20, height/20,
                            width/2 - 2*width/20, 4*(height/2 - 2*height/20)/5 );

    QPB_Hard->setGeometry(width/20, QPB_Easy->y() + QPB_Easy->height() + height/20,
                          width/2 - 2*width/20,  4*(height/2 - 2*height/20)/5 );

    QCB_FirstCellClean->setGeometry(width/20, QPB_Hard->y() + QPB_Hard->height() + height/20,
                                    width/2 - 2*width/20,  (height/2 - 2*height/20)/10 );

    QCB_DarkModeEnabled->setGeometry(width/20, QCB_FirstCellClean->y() + QCB_FirstCellClean->height(),
                                     width/2 - 2*width/20,  (height/2 - 2*height/20)/10);

    QPB_CustomizedNewGame->setGeometry(QPB_Easy->x() + QPB_Easy->width() + width/20, QPB_Easy->y() + QPB_Easy->height() + height/20,
                                       width/2 - 2*width/20, 2*(height/2 - 2*height/20)/5 );

    QL_Customized_nMines->setGeometry(QPB_CustomizedNewGame->x(), QPB_CustomizedNewGame->y()+QPB_CustomizedNewGame->height(),
                                      QPB_CustomizedNewGame->width()/2,2*(height/2 - 2*height/20)/(3*5));

    QL_Customized_X->setGeometry(QL_Customized_nMines->x(), QL_Customized_nMines->y()+QL_Customized_nMines->height(),
                                 QL_Customized_nMines->width(),QL_Customized_nMines->height());

    QL_Customized_Y->setGeometry(QL_Customized_X->x(), QL_Customized_X->y()+QL_Customized_X->height(),
                                 QL_Customized_X->width(),QL_Customized_X->height());

    QSB_Customized_nMines->setGeometry(QL_Customized_nMines->x()+QL_Customized_nMines->width(), QL_Customized_nMines->y(),
                                       QL_Customized_nMines->width(), QL_Customized_nMines->height());

    QSB_Customized_X->setGeometry(QL_Customized_X->x()+QL_Customized_X->width(), QL_Customized_X->y(),
                                  QL_Customized_X->width(), QL_Customized_X->height());

    QSB_Customized_Y->setGeometry(QL_Customized_Y->x()+QL_Customized_Y->width(), QL_Customized_Y->y(),
                                  QL_Customized_Y->width(), QL_Customized_Y->height());


    connect(QPB_Easy, &QPushButton::pressed,
            this, &Mines_Bollos::SLOT_Easy);

    connect(QPB_Medium, &QPushButton::pressed, this,
            &Mines_Bollos::SLOT_Medium);

    connect(QPB_Hard, &QPushButton::pressed,
            this, &Mines_Bollos::SLOT_Hard);

    connect(QPB_CustomizedNewGame, &QPushButton::pressed,
            this, &Mines_Bollos::SLOT_Customized);

    connect(QPB_RestartInGame, &QPushButton::pressed,
            this, &Mines_Bollos::SLOT_Restart);

    connect(QPB_QuitInGame, &QPushButton::pressed,
            this, &Mines_Bollos::SLOT_Quit);

    connect(QCB_FirstCellClean, &QPushButton::released,
            this, &Mines_Bollos::SLOT_UpdateFirstCellClean);

    connect(QCB_DarkModeEnabled, &QPushButton::released,
            this, &Mines_Bollos::SLOT_DarkMode);

    QL_Mines_Bollos = new QLabel(this);
    QL_Mines_Bollos->setText("Mines\nBollos");
    QL_Mines_Bollos->setFont(QFont("Ubuntu", 15));
    QL_Mines_Bollos->setGeometry(9*width/10, 85*height/100, 2*width/10, height/10);
    QL_Mines_Bollos->show();


}

void Mines_Bollos::vHideInterface(){

    QPB_Easy->hide();
    QPB_Medium->hide();
    QPB_Hard->hide();

    QPB_CustomizedNewGame->hide();

    QSB_Customized_X->hide();
    QSB_Customized_Y->hide();
    QSB_Customized_nMines->hide();

    QL_Customized_X->hide();
    QL_Customized_Y->hide();
    QL_Customized_nMines->hide();

    QCB_FirstCellClean->hide();
}

void Mines_Bollos::vShowInterface(){

    QPB_Easy->show();
    QPB_Medium->show();
    QPB_Hard->show();

    QPB_CustomizedNewGame->show();

    QSB_Customized_X->show();
    QSB_Customized_Y->show();
    QSB_Customized_nMines->show();

    QL_Customized_X->show();
    QL_Customized_Y->show();
    QL_Customized_nMines->show();

    QCB_FirstCellClean->show();
}

void Mines_Bollos::SLOT_Easy(){

    vHideInterface();

    QTIMER_TimeInGame = new QTimer (this);

    vNewGame(8,8,10);

    GAME_STATE = EASY;
}

void Mines_Bollos::SLOT_Medium(){

    vHideInterface();

    QTIMER_TimeInGame = new QTimer (this);

    bFirst =true;

    vNewGame(16,16,40);

    GAME_STATE = MEDIUM;

}

void Mines_Bollos::SLOT_Hard(){

    vHideInterface();

    QTIMER_TimeInGame = new QTimer (this);

    bFirst =true;

    vNewGame(30,16,99);

    GAME_STATE = HARD;

}

void Mines_Bollos::SLOT_Customized(){

    vHideInterface();

    QTIMER_TimeInGame = new QTimer (this);

    bFirst = true;

    int x = QSB_Customized_X->value();
    int y = QSB_Customized_Y->value();
    int Mines_Bollos = static_cast<int> (round(x*y*QSB_Customized_nMines->value()/100));
    vNewGame(x,y,Mines_Bollos);

    GAME_STATE = CUSTOMIZED;

}

void Mines_Bollos::vAjustInterfaceInGame(){

    int width = this->width();
    int height = this->height();

    QL_TimerInGame->setGeometry(85*width/100, height/20, 15*width/100, height/8);
    QLCDN_MinesLeft->setGeometry(QL_TimerInGame->x(), QL_TimerInGame->y()+QL_TimerInGame->height(), QL_TimerInGame->width(), height/7);
    QPB_RestartInGame->setGeometry(QLCDN_MinesLeft->x(), QLCDN_MinesLeft->y()+QLCDN_MinesLeft->height(), QLCDN_MinesLeft->width()/2, height/20);
    QPB_QuitInGame->setGeometry(QPB_RestartInGame->x()+QPB_RestartInGame->width(), QPB_RestartInGame->y(), QPB_RestartInGame->width(), QPB_RestartInGame->height());
    QL_YouWon_YouLost->setGeometry(QLCDN_MinesLeft->x(), QPB_RestartInGame->y()+QPB_RestartInGame->height()+height/10, QLCDN_MinesLeft->width(), QLCDN_MinesLeft->height());
    QL_StatisLastMatch->setGeometry(QL_YouWon_YouLost->x(), QL_YouWon_YouLost->y() + QL_YouWon_YouLost->height(),
                                    QL_YouWon_YouLost->width(), height/5);
}


void Mines_Bollos::vHideInterfaceInGame(){

    QL_TimerInGame->hide();
    QLCDN_MinesLeft->hide();
    QPB_RestartInGame->hide();
    QPB_QuitInGame->hide();
    QL_YouWon_YouLost->hide();
    QL_StatisLastMatch->hide();

}

void Mines_Bollos::vShowInterfaceInGame(){

    QL_TimerInGame->show();
    QLCDN_MinesLeft->show();
    QPB_RestartInGame->show();
    QPB_QuitInGame->show();
    QL_YouWon_YouLost->show();
    QL_StatisLastMatch->show();

}

void Mines_Bollos::SLOT_Restart(){
    if(QTIMER_TimeInGame->isActive()
       )
        QTIMER_TimeInGame->stop();

    delete QTIMER_TimeInGame;

    QTIMER_TimeInGame = new QTimer (this);

    vResetPrincipal_Matrix();

    QL_StatisLastMatch->setText(" ");

    bFirst = true;

    vNewGame(i_X, i_Y, i_nMines);
}

void Mines_Bollos::SLOT_Quit(){

    if (QTIMER_TimeInGame->isActive())
        QTIMER_TimeInGame->stop();

    delete QTIMER_TimeInGame;

    QL_StatisLastMatch->setText(" ");

    vResetPrincipal_Matrix();
    vHideInterfaceInGame();
    vShowInterface();

    GAME_STATE = NONE;
}

void Mines_Bollos::SLOT_UpdateTime(){

    i_TimeInSeconds++;
    QL_TimerInGame->setNum(i_TimeInSeconds);
}



void Mines_Bollos::vAddOrRemoveFlag(int _X, int _Y){

    switch (Principal_Matrix[_X][_Y].bFlag){
        case false:
            Principal_Matrix[_X][_Y].QPB_a->setIcon(QIcon(QPixmap::fromImage(*QI_Flag).scaled(fm, fm, Qt::KeepAspectRatio)));
            Principal_Matrix[_X][_Y].QPB_a->setIconSize(QSize(fm, fm));
            Principal_Matrix[_X][_Y].bFlag = true;
            i_MinesLeft--;
            QLCDN_MinesLeft->display(i_MinesLeft);
            break;

        case true:
            Principal_Matrix[_X][_Y].QPB_a->setIcon(QIcon(QPixmap::fromImage(*QI_NoFlag).scaled(fm, fm, Qt::KeepAspectRatio)));
            Principal_Matrix[_X][_Y].QPB_a->setIconSize(QSize(fm, fm));
            Principal_Matrix[_X][_Y].bFlag = false;
            i_MinesLeft++;
            QLCDN_MinesLeft->display(i_MinesLeft);
            break;

    }
}

void Mines_Bollos::SLOT_OnQPushButton_adapteded_Clicked(QMouseEvent *e){

    QPushButton_adapted *_QPB = (QPushButton_adapted *) sender();
    bool bEncontrouPai = false;
    int _X, _Y;

    for(int j=0; j<i_Y; j++){
        for (int i=0; i<i_X; i++){
            if(_QPB == Principal_Matrix[i][j].QPB_a){
                bEncontrouPai = true;
                _X = i;
                _Y = j;
            }
            if(bEncontrouPai == true)
                break;
        }
        if (bEncontrouPai == true)
            break;
    }
    switch (e->button()) {
        case Qt::RightButton:

            vAddOrRemoveFlag(_X, _Y);
            break;

        case Qt::LeftButton:
            if(bFirst){
                if(bFirstCellClean  &&  Principal_Matrix[_X][_Y].S_Cell != ZERO){
                    vNewGame(i_X, i_Y, i_nMines, _X, _Y);
                }
                bFirst =false;
                vStartTimer();
            }

            if(Principal_Matrix[_X][_Y].S_Cell == MINE){
                if(Principal_Matrix[_X][_Y].bFlag == false)
                    vGameLost(_X, _Y);
            }
            else

                vCasaLivre(_X, _Y);
            break;

        default:
            break;

    }
}

void Mines_Bollos::vStartTimer(){

    i_TimeInSeconds = 0;
    QTIMER_TimeInGame->start(1e3);

    connect(QTIMER_TimeInGame, &QTimer::timeout, this, &Mines_Bollos::SLOT_UpdateTime);

}

void Mines_Bollos::SLOT_UpdateFirstCellClean(){

    bFirstCellClean = QCB_FirstCellClean->isChecked();
}

void Mines_Bollos::SLOT_DarkMode()
{
    vConfigureDarkMode(QCB_DarkModeEnabled->isChecked());

    if(QCB_DarkModeEnabled->isChecked())
        QCB_DarkModeEnabled->setText("Disable dark mode");

    else
        QCB_DarkModeEnabled->setText("Enable dark mode");

}

void Mines_Bollos::vGenerateStatics(){

    int iCorrectFlags = 0,
            iWrongFlags = 0,
            iUnlockedCells = i_CellsToUnlock - i_HiddenCells;

    for (int i=0; i<(i_X-1); i++){
        for (int j=0; j<=(i_Y-1); j++){
            if(Principal_Matrix[i][j].bFlag == true){
                if (Principal_Matrix[i][j].S_Cell == MINE)
                    iCorrectFlags++;
                else
                    iWrongFlags++;
            }
        }
    }
    if(i_TimeInSeconds == 0)
        i_TimeInSeconds = 1;

    double dFlagsPerSecond = (double)iCorrectFlags/i_TimeInSeconds,
            dCellsPerSecond = (double)iUnlockedCells/i_TimeInSeconds,
            dPercentageGameComplete = (double)100*iUnlockedCells/i_CellsToUnlock;

    QString QS_Statics = "Correct Flags: " + QString::number(iCorrectFlags)
            +"\nWrongFlags: " + QString::number(iWrongFlags)
            +"\nUnlocked Cells: " + QString::number(iUnlockedCells)
            +"\n"
            +"\nFlags/s: " + QString::number(dFlagsPerSecond, 'f', 3)
            +"\nCells/s: " + QString::number(dCellsPerSecond, 'f', 3)
            +"\n"
            +"\nGame Complete: " + QString::number(dPercentageGameComplete, 'f', 2) + " %";

    QL_StatisLastMatch->setText(QS_Statics);
}

void Mines_Bollos::vConfigureDarkMode(const bool bDark)
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

        qApp->setPalette (darkPalette);

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

        qApp->setPalette (lightPalette);

    }

    QI_Zero->invertPixels();
    QI_One->invertPixels();
    QI_Two->invertPixels();
    QI_Three->invertPixels();
    QI_Four->invertPixels();
    QI_Five->invertPixels();
    QI_Six->invertPixels();
    QI_Seven->invertPixels();
    QI_Eight->invertPixels();
    QI_Flag->invertPixels();
    QI_NoFlag->invertPixels();
    QI_Mine->invertPixels();
    QI_Boom->invertPixels();
    QI_WrongFlag->invertPixels();

}
