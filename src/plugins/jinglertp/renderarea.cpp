#include <QDebug>
#include "renderarea.h"

RenderArea::RenderArea(QWidget *parent)
    : QWidget(parent)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    level = 0;
    setMinimumHeight(20);//30
    setMinimumWidth(10);//200
    setMaximumWidth(10);
    addTop    = 1;//10
    addBottom = 2;//20
    addLeft   = 1;//10
    addRight  = 1;//20
}

void RenderArea::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.setPen(Qt::black);
    int pvr=painter.viewport().right();
    int pvl=painter.viewport().left();
    int pvt=painter.viewport().top();
    int pvb=painter.viewport().bottom();
    painter.drawRect(QRect(pvl+addLeft,pvt+addTop,pvr-addRight,pvb-addBottom));
    if (level == 0.0)
        return;

    painter.setPen(Qt::red);
    int pos = (pvb-addRight-pvt+addLeft)*level;

    for (int i = 0; i < 10; ++i) {
        int x1 = pvl+addLeft+i;   int y1 = pvb-addBottom+1;
        int x2 = pvl+1*addLeft+i; int y2 = pvb-addBottom-pos;

        if (x2 < pvl+addLeft)
            x2 = pvl+addLeft;

        painter.drawLine(QPoint(x1, y1),QPoint(x2, y2));
    }
}

void RenderArea::setLevel(qreal value)
{
    level = value;
    repaint();
}

void RenderArea::changeLevel(qreal value)
{
    setLevel(value);
    this->repaint();
}
