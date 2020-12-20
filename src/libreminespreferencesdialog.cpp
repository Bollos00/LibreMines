#include "libreminespreferencesdialog.h"
#include "ui_libreminespreferencesdialog.h"


LibreMinesPreferencesDialog::LibreMinesPreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LibreMinesPreferencesDialog)
{
    ui->setupUi(this);

    ui->comboBoxTheme->addItems({"Dark", "Light"});
}

LibreMinesPreferencesDialog::~LibreMinesPreferencesDialog()
{
    delete ui;
}

bool LibreMinesPreferencesDialog::optionFirstCellClean() const
{
    return ui->cbFirstCellClean->isChecked();
}

bool LibreMinesPreferencesDialog::optionCleanNeighborCellsWhenClickedOnShowedCell() const
{
    return ui->cbCleanNeighborCellsWhenClickedOnShowedCell->isChecked();
}

QString LibreMinesPreferencesDialog::optionTheme() const
{
    return ui->comboBoxTheme->currentText();
}

QString LibreMinesPreferencesDialog::optionUsername() const
{
    return ui->lineEditUsername->text();
}

void LibreMinesPreferencesDialog::setOptionFirstCellClean(const QString &option)
{
    ui->cbFirstCellClean->setChecked(option.compare("On", Qt::CaseInsensitive) == 0);
}

void LibreMinesPreferencesDialog::setOptionCleanNeighborCellsWhenClickedOnShowedCell(const QString &option)
{
    ui->cbCleanNeighborCellsWhenClickedOnShowedCell->setChecked(option.compare("On", Qt::CaseInsensitive) == 0);
}

void LibreMinesPreferencesDialog::setOptionTheme(const QString &theme)
{
    ui->comboBoxTheme->setCurrentText(theme);
}

void LibreMinesPreferencesDialog::setOptionUsername(const QString &username)
{
    ui->lineEditUsername->setText(username);
}

