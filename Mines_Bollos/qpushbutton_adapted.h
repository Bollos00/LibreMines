#ifndef QPUSHBUTTON_ADAPTED_H
#define QPUSHBUTTON_ADAPTED_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>

class QPushButton_adapted : public QPushButton
{
    Q_OBJECT

public:
    explicit QPushButton_adapted(QWidget *parent = nullptr);

private slots:
    void mousePressEvent(QMouseEvent *e);

signals:
    void SIGNAL_Clicked(QMouseEvent *);
};

#endif // QPUSHBUTTON_ADAPTED_H
