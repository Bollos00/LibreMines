#include "minefieldthemepreview.h"
#include "ui_minefieldthemepreview.h"

MinefieldThemePreview::MinefieldThemePreview(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MinefieldThemePreview)
{
    ui->setupUi(this);

    listLbCells.append(ui->lbNoFlag);
    listLbCells.append(ui->lbFlag);
    listLbCells.append(ui->lbQuestionFlag);
    listLbCells.append(ui->lbZero);
    listLbCells.append(ui->lbZero);
    listLbCells.append(ui->lbZero);
    listLbCells.append(ui->lbOne);
    listLbCells.append(ui->lbTwo);
    listLbCells.append(ui->lbThree);
    listLbCells.append(ui->lbFour);
    listLbCells.append(ui->lbFive);
    listLbCells.append(ui->lbSix);
    listLbCells.append(ui->lbSeven);
    listLbCells.append(ui->lbEight);
    listLbCells.append(ui->lbMine);
    listLbCells.append(ui->lbBoom);
    listLbCells.append(ui->lbWrongFlag);
}

MinefieldThemePreview::~MinefieldThemePreview()
{
    delete ui;
}

void MinefieldThemePreview::vSetMinefieldTheme(const QString& theme, int lenght)
{
    iCellLength = lenght;

    QString themeKey = theme;
    if (theme.compare("Classic Dark", Qt::CaseInsensitive) == 0)
    {
        themeKey = "ClassicDark";
    }
    else if (theme.compare("Classic Light", Qt::CaseInsensitive) == 0)
    {
        themeKey = "ClassicLight";
    }

    minefieldTheme.vSetMinefieldTheme(themeKey, iCellLength);

    setFixedHeight(iCellLength);

    for (QLabel_adapted* lb: listLbCells)
    {
        lb->setFixedSize(iCellLength, iCellLength);
        lb->setText("");
    }

    // ui->lbThemeName->setText(theme);
    ui->lbNoFlag->setPixmapCached(minefieldTheme.getPixmapButton(FlagState::NoFlag));
    ui->lbFlag->setPixmapCached(minefieldTheme.getPixmapButton(FlagState::HasFlag));
    ui->lbQuestionFlag->setPixmapCached(minefieldTheme.getPixmapButton(FlagState::Question));
    ui->lbZero->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::ZERO));
    ui->lbOne->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::ONE));
    ui->lbTwo->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::TWO));
    ui->lbThree->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::THREE));
    ui->lbFour->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::FOUR));
    ui->lbFive->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::FIVE));
    ui->lbSix->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::SIX));
    ui->lbSeven->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::SEVEN));
    ui->lbEight->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::EIGHT));
    ui->lbMine->setPixmapCached(minefieldTheme.getPixmapFromCellValue(CellValue::MINE));
    ui->lbBoom->setPixmapCached(minefieldTheme.getPixmapBoom());
    ui->lbWrongFlag->setPixmapCached(minefieldTheme.getPixmapWrongFlag());
}
