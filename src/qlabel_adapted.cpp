#include "qlabel_adapted.h"

QLabel_adapted::QLabel_adapted(QWidget *parent):
    QLabel(parent)
{

}

void QLabel_adapted::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    emit SIGNAL_clicked(event);
}
