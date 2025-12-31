#ifndef GAMEPLAYTHEMECHOOSER_H
#define GAMEPLAYTHEMECHOOSER_H

#include "facesreactionpreview.h"
#include "minefieldthemepreview.h"

#include <QDialog>
#include <QRadioButton>
#include <QButtonGroup>

namespace Ui {
class GameplayThemeChooser;
}

class GameplayThemeChooser : public QDialog
{
    Q_OBJECT

public:
    explicit GameplayThemeChooser(QWidget *parent = nullptr);
    ~GameplayThemeChooser();

    void vInitialize();

protected:
    void closeEvent(QCloseEvent* e);
    void hideEvent(QHideEvent* e);
    void showEvent(QShowEvent* e);

private:
    void vAddMinefieldTheme(const QString& themeName);
    void vAddFacesReactionTheme(const QString& themeName);
    static void vAddLineSeparatorOnLayout(QFormLayout* layout);

    Ui::GameplayThemeChooser *ui;
    QList<QPair<QRadioButton*, MinefieldThemePreview*>> listMinefieldPreview;
    QButtonGroup* btngMinefield;

    QList<QPair<QRadioButton*, FacesReactionPreview*>> listFacesReactionPreview;
    QButtonGroup* btngFacesReaction;

    int iFacesReactionPreviewLength;
    int iMinefieldPreviewLength;

Q_SIGNALS:
    void SIGNAL_visibilityChanged(const bool visible);

};

#endif // GAMEPLAYTHEMECHOOSER_H
