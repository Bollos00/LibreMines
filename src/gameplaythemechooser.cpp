#include "gameplaythemechooser.h"
#include "ui_gameplaythemechooser.h"

#include "extrathemes.h"

GameplayThemeChooser::GameplayThemeChooser(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GameplayThemeChooser)
{
    ui->setupUi(this);

    btngMinefieldThemeGroup = new QButtonGroup(ui->scrollAreaMinefieldWidgetContents);

    // Add minefield themes
    vAddMinefieldTheme("Classic Dark");
    vAddMinefieldTheme("Classic Light");

    for (const QString& theme: ExtraThemes::getExtraThemes(ExtraThemes::Minefield))
    {
        vAddMinefieldTheme(theme);
    }
}

GameplayThemeChooser::~GameplayThemeChooser()
{
    delete ui;
}

void GameplayThemeChooser::vInitialize()
{
    // listMinefieldThemePreview.append(
    //     {});
}

void GameplayThemeChooser::closeEvent(QCloseEvent* e)
{
    Q_UNUSED(e);
    this->hide();
}

void GameplayThemeChooser::vAddMinefieldTheme(const QString& themeName)
{
    QRadioButton* rb = new QRadioButton();
    MinefieldThemePreview* themePreview = new MinefieldThemePreview();
    themePreview->vSetMinefieldTheme(themeName);

    ui->scrollAreaMinefieldWidgetContentsLayout->addRow(rb, themePreview);
    btngMinefieldThemeGroup->addButton(rb);

    listMinefieldThemePreview.append({rb, themePreview});
}
