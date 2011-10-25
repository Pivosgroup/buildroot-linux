#ifndef QUSRGRAPHICSVIEW_H
#define QUSRGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>

class QUsrGraphicsView : public QGraphicsView
{
Q_OBJECT
public:
    explicit QUsrGraphicsView(QWidget *parent = 0);
    QUsrGraphicsView(QGraphicsScene *scene, QWidget *parent = 0);
protected:
    void paintEvent(QPaintEvent *event);
public:
    void reset();
public:
    int status;
};

#endif // QUSRGRAPHICSVIEW_H
