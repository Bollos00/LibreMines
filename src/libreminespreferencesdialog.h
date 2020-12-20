#ifndef LIBREMINESPREFERENCESDIALOG_H
#define LIBREMINESPREFERENCESDIALOG_H

#include <QDialog>

namespace Ui {
class LibreMinesPreferencesDialog;
}

class LibreMinesPreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LibreMinesPreferencesDialog(QWidget *parent = nullptr);
    ~LibreMinesPreferencesDialog();

    bool optionFirstCellClean()const;
    bool optionCleanNeighborCellsWhenClickedOnShowedCell()const;
    QString optionTheme()const;
    QString optionUsername()const;

    void setOptionFirstCellClean(const QString& option);
    void setOptionCleanNeighborCellsWhenClickedOnShowedCell(const QString& option);
    void setOptionTheme(const QString& theme);
    void setOptionUsername(const QString& username);

private:
    Ui::LibreMinesPreferencesDialog *ui;
};

#endif // LIBREMINESPREFERENCESDIALOG_H
