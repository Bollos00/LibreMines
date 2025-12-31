#include "gameplaythemechooser.h"
#include "ui_gameplaythemechooser.h"

#include "extrathemes.h"

GameplayThemeChooser::GameplayThemeChooser(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GameplayThemeChooser)
{
    ui->setupUi(this);

    btngMinefield = new QButtonGroup(ui->scrollAreaMinefieldWidgetContents);
    btngFacesReaction = new QButtonGroup(ui->scrollAreaFacesReactionWidgetContents);

    // Set preview sizes
    iFacesReactionPreviewLength = 73;
    iMinefieldPreviewLength = 45;

    // Add begin line separators
    vAddLineSeparatorOnLayout(ui->scrollAreaMinefieldWidgetContentsLayout);
    vAddLineSeparatorOnLayout(ui->scrollAreaFacesReactionWidgetContentsLayout);

    // Add minefield themes
    vAddMinefieldTheme("Classic Dark");
    vAddMinefieldTheme("Classic Light");

    for (const QString& theme: ExtraThemes::getExtraThemes(ExtraThemes::Minefield))
    {
        vAddMinefieldTheme(theme);
    }

    vAddFacesReactionTheme("Disable");
    vAddFacesReactionTheme("Open Emoji Colored");

    for (const QString& theme: ExtraThemes::getExtraThemes(ExtraThemes::FacesReaction))
    {
        vAddFacesReactionTheme(theme);
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

void GameplayThemeChooser::hideEvent(QHideEvent *e)
{
    Q_UNUSED(e);
    Q_EMIT SIGNAL_visibilityChanged(false);
}

void GameplayThemeChooser::showEvent(QShowEvent *e)
{
    Q_UNUSED(e);
    Q_EMIT SIGNAL_visibilityChanged(true);
}

void GameplayThemeChooser::vAddMinefieldTheme(const QString& themeName)
{
    QRadioButton* rb = new QRadioButton(themeName, ui->scrollAreaMinefieldWidgetContents);
    MinefieldThemePreview* themePreview = new MinefieldThemePreview(ui->scrollAreaMinefieldWidgetContents);
    themePreview->vSetMinefieldTheme(themeName, iMinefieldPreviewLength);

    // const int rowCount = ui->scrollAreaMinefieldWidgetContentsLayout->count();
    // ui->scrollAreaMinefieldWidgetContentsLayout->setWidget(rowCount, QFormLayout::ItemRole::LabelRole, rb);
    // ui->scrollAreaMinefieldWidgetContentsLayout->setWidget(rowCount, QFormLayout::ItemRole::FieldRole, themePreview);

    ui->scrollAreaMinefieldWidgetContentsLayout->addRow(rb, themePreview);
    btngMinefield->addButton(rb);

    listMinefieldPreview.append({rb, themePreview});

    // Add line separator below
    vAddLineSeparatorOnLayout(ui->scrollAreaMinefieldWidgetContentsLayout);
}

void GameplayThemeChooser::vAddFacesReactionTheme(const QString& themeName)
{
    QRadioButton* rb = new QRadioButton(themeName);
    FacesReactionPreview* facesPreview = new FacesReactionPreview();
    facesPreview->vSetFacesReactionTheme(themeName, iFacesReactionPreviewLength);

    ui->scrollAreaFacesReactionWidgetContentsLayout->addRow(rb, facesPreview);
    btngFacesReaction->addButton(rb);

    listFacesReactionPreview.append({rb, facesPreview});

    // Add line separator below
    vAddLineSeparatorOnLayout(ui->scrollAreaFacesReactionWidgetContentsLayout);
}

void GameplayThemeChooser::vAddLineSeparatorOnLayout(QFormLayout* layout)
{
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Plain);
    layout->addRow(separator);
}
