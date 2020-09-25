#include "qpushbutton_adapted.h"

QPushButton_adapted::QPushButton_adapted(QWidget *parent) : QPushButton(parent)
{

}

void QPushButton_adapted::mousePressEvent(QMouseEvent *e){
    emit SIGNAL_Clicked(e);
}
