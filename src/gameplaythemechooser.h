#ifndef GAMEPLAYTHEMECHOOSER_H
#define GAMEPLAYTHEMECHOOSER_H

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

private:
    void vAddMinefieldTheme(const QString& themeName);

    Ui::GameplayThemeChooser *ui;
    QList<QPair<QRadioButton*, MinefieldThemePreview*>> listMinefieldThemePreview;
    QButtonGroup* btngMinefieldThemeGroup;
};

#endif // GAMEPLAYTHEMECHOOSER_H
