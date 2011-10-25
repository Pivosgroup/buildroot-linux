#include "qusrgraphicsview.h"
#include <QtGui>

QUsrGraphicsView::QUsrGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
}
QUsrGraphicsView::QUsrGraphicsView(QGraphicsScene *scene, QWidget *parent)
    :QGraphicsView(scene,parent)
{

}

void QUsrGraphicsView::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);
    //qDebug()<<"debugger::"<<status<<"\n";
    status++;
}

void QUsrGraphicsView::reset()
{
    //qDebug()<<"reset\n";
    status=0;
}
