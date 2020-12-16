#ifndef QLABEL_ADAPTED_H
#define QLABEL_ADAPTED_H

#include <QLabel>
#include <QObject>

class QLabel_adapted : public QLabel
{
    Q_OBJECT

public:
    QLabel_adapted(QWidget* parent = nullptr);

protected:
   void mousePressEvent(QMouseEvent* event);

Q_SIGNALS:
    void SIGNAL_clicked(const QMouseEvent *const e);
};

#endif // QLABEL_ADAPTED_H
