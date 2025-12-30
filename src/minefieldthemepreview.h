#ifndef MINEFIELDTHEMEPREVIEW_H
#define MINEFIELDTHEMEPREVIEW_H

#include "minefieldtheme.h"
#include "qlabel_adapted.h"

#include <QWidget>

namespace Ui {
class MinefieldThemePreview;
}

class MinefieldThemePreview : public QWidget
{
    Q_OBJECT

public:
    explicit MinefieldThemePreview(QWidget *parent = nullptr);
    ~MinefieldThemePreview();

    void vSetMinefieldTheme(QString theme);

private:
    Ui::MinefieldThemePreview *ui;

    MinefieldTheme minefieldTheme;
    int iCellLength;
    QList<QLabel_adapted*> listLbCells;
};

#endif // MINEFIELDTHEMEPREVIEW_H
