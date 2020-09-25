#ifndef QPUSHBUTTON_ADAPTED_H
#define QPUSHBUTTON_ADAPTED_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>

/**
 * @brief
 *
 */
class QPushButton_adapted : public QPushButton
{
    Q_OBJECT

public:
    /**
     * @brief
     *
     * @param parent
     */
    explicit QPushButton_adapted(QWidget *parent = nullptr);

private slots:
    /**
     * @brief
     *
     * @param e
     */
    void mousePressEvent(QMouseEvent *e);

signals:
    /**
     * @brief
     *
     * @param
     */
    void SIGNAL_Clicked(QMouseEvent *event);
};

#endif // QPUSHBUTTON_ADAPTED_H
