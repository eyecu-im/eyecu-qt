#include "volumecontrol.h"

#include <QPainter>
#include <QPaintEngine>
#include <QStyle>
#include <QDebug>

VolumeControl::VolumeControl(QWidget *parent) :
    QSlider(parent)
{
}

VolumeControl::VolumeControl(Qt::Orientation orientation, QWidget *parent) :
    QSlider(orientation, parent)
{
}

void VolumeControl::paintEvent(QPaintEvent *ev)
{
	Q_UNUSED(ev)

    QPainter painter;
    QPalette palette;

    painter.begin(this);

//    int sliderSpace=style()->pixelMetric(QStyle::PM_SliderSpaceAvailable);
    int sliderLength=style()->pixelMetric(QStyle::PM_SliderLength);
//    int sliderThickness=style()->pixelMetric(QStyle::PM_SliderThickness);
//    int sliderControlThickness=style()->pixelMetric(QStyle::PM_SliderControlThickness);
//    int sliderTickmarkOffset=style()->pixelMetric(QStyle::PM_SliderTickmarkOffset);

    int left=sliderLength/2;
    int right=width()-sliderLength/2;
    float length=width()-sliderLength;

    int sliderWidth=height()*2/3;
    if (sliderWidth<12)
        sliderWidth=12;
    int tickLength = height()-sliderWidth;
    int sliderPos;
    switch (tickPosition())
    {
        case TicksAbove:
            sliderPos=tickLength+1;
            break;
        case TicksBothSides:
            sliderPos=tickLength/2+1;
            break;
        default:
            sliderPos=0;
            break;
    }

/*
    qDebug() << "sliderThickness=" << sliderThickness;
    qDebug() << "sliderControlThickness=" << sliderControlThickness;
    qDebug() << "sliderTickmarkOffset=" << sliderTickmarkOffset;
*/

    QLinearGradient gradient(QPointF(left, 0), QPointF(right-1, 0));
    QColor pen(Qt::black);

    const QPointF points[3] = {
         QPointF(left, sliderPos+sliderWidth+1),
         QPointF(right-1, sliderPos+sliderWidth+1),
         QPointF(right-1, sliderPos),
     };

    int position=QStyle::sliderPositionFromValue(minimum(), maximum(), value(), width()-sliderLength, false);

    painter.setPen(pen);

    // Draw filled band
	if (isEnabled())
	{
		gradient.setColorAt(0, Qt::darkBlue);
		gradient.setColorAt(1, Qt::magenta);
	}
	else
	{
		gradient.setColorAt(0, "#2A2A2A");
		gradient.setColorAt(1, "#AAAAAA");
	}
    painter.setBrush(gradient);
    painter.setClipRect(left, sliderPos, left+position, sliderPos+sliderWidth+2);
    painter.drawPolygon(points, 3);

    // Draw empty band
    gradient.setColorAt(0, Qt::darkGray);
    gradient.setColorAt(1, Qt::lightGray);
    painter.setBrush(gradient);
    painter.setClipRect(left+position, sliderPos, right, sliderPos+sliderWidth+2);
    painter.drawPolygon(points, 3);


    if (tickPosition()==TicksAbove||
        tickPosition()==TicksBelow)
    {
        int total=maximum()-minimum();
        if (total)
        {
            int interval=tickInterval();
            if (!interval)
            {
                interval=singleStep();
                if (total/interval+1>length)
                {
                    interval=pageStep();
                    if (total/interval+1>length)
                        interval=0;
                }
            }

            if (interval)
            {
                painter.setClipRect(left, height()-5, right, height());
                painter.setPen(palette.color(QPalette::WindowText));
                int numTicks=total/interval+1;
                for (int i=0; i<numTicks; i++)
                {
                    int pos=left+length*i/(numTicks-1);
                    if (tickPosition()==TicksAbove)
                        painter.drawLine(pos, 0, pos, tickLength);
                    else
                        painter.drawLine(pos, height()-1, pos, height()-tickLength);
                }
            }
        }
    }

    // Draw slider handle
    painter.setClipRect(position, 0, sliderLength+position, height());            

    switch (tickPosition())
    {
        case NoTicks:
        case TicksBothSides:
        {
            QPoint pts[11]={QPoint(position, sliderPos+sliderWidth), QPoint(position, sliderPos), QPoint(position+sliderLength-1, sliderPos),
                           QPoint(position+sliderLength-1, sliderPos), QPoint(position+sliderLength-1, sliderPos+sliderWidth), QPoint(position, sliderPos+sliderWidth),
                           QPoint(position+sliderLength-2, sliderPos+1), QPoint(position+sliderLength-2, sliderPos+sliderWidth-1), QPoint(position+1, sliderPos+sliderWidth-1)};
            painter.setPen(palette.color(QPalette::Light));
            painter.drawPolyline(pts, 3);
            painter.setPen(palette.color(QPalette::Shadow));
            painter.drawPolyline(pts+3, 3);
            painter.setPen(palette.color(QPalette::Dark));
            painter.drawPolyline(pts+6, 3);
            painter.setPen(Qt::NoPen);
            painter.setBrush(palette.color(QPalette::Background));
            painter.drawRect(position+1, sliderPos+1, sliderLength-3, sliderPos+sliderWidth-2);
            break;
        }

        case TicksAbove:
        {
            QPoint pts[19]={QPoint(position, sliderPos+sliderWidth-1), QPoint(position, sliderPos+left), QPoint(position+left-1, sliderPos+1),
                            QPoint(position+left-1, sliderPos+2), QPoint(position+1, sliderPos+left), QPoint(position+1, sliderPos+sliderWidth-2),
                            QPoint(position+left, sliderPos), QPoint(position+sliderLength-1, sliderPos+left), QPoint(position+sliderLength-1, sliderPos+sliderWidth), QPoint(position, sliderPos+sliderWidth),
                            QPoint(position+left, sliderPos+1), QPoint(position+sliderLength-2, sliderPos+left), QPoint(position+sliderLength-2, sliderPos+sliderWidth-1), QPoint(position+1, sliderPos+sliderWidth-1),
                            QPoint(position+2, sliderPos+sliderWidth-2), QPoint(position+2, sliderPos+left), QPoint(position+left, sliderPos+1),
                            QPoint(position+sliderLength-2, left), QPoint(position+sliderLength-2, height()-2)};
            painter.setPen(palette.color(QPalette::Light));
            painter.drawPolyline(pts, 6);
            painter.setPen(palette.color(QPalette::Shadow));
            painter.drawPolyline(pts+6, 4);
            painter.setPen(palette.color(QPalette::Dark));
            painter.drawPolyline(pts+10, 4);
            painter.setPen(Qt::NoPen);
            painter.setBrush(palette.color(QPalette::Background));
            painter.drawPolygon(pts+14, 5);
            break;
        }

        case TicksBelow:
        {
            QPoint pts[19]={QPoint(position+sliderLength-2, sliderPos+1),QPoint(position, sliderPos+1), QPoint(position, sliderPos+sliderWidth-left), QPoint(position+left-1, sliderPos+sliderWidth-1),
                            QPoint(position+sliderLength-3, sliderPos+2),QPoint(position+1, sliderPos+2), QPoint(position+1, sliderPos+sliderWidth-left), QPoint(position+left-1, sliderPos+sliderWidth-2),
                            QPoint(position+sliderLength-1, sliderPos+1), QPoint(position+sliderLength-1, sliderPos+sliderWidth-left), QPoint(position+left, sliderPos+sliderWidth),
                            QPoint(position+sliderLength-2, 2), QPoint(position+sliderLength-2, sliderPos+sliderWidth-left), QPoint(position+left, sliderPos+sliderWidth-1),
                            QPoint(position+2, sliderPos+2), QPoint(position+2, sliderPos+sliderWidth-left), QPoint(position+left, sliderPos+sliderWidth-1),
                            QPoint(position+sliderLength-2, sliderPos+sliderWidth-left), QPoint(position+sliderLength-2, sliderPos+1)};
            painter.setPen(palette.color(QPalette::Light));
            painter.drawPolyline(pts, 4);
            painter.setPen(palette.color(QPalette::Midlight));
            painter.drawPolyline(pts+4, 4);
            painter.setPen(palette.color(QPalette::Shadow));
            painter.drawPolyline(pts+8, 3);
            painter.setPen(palette.color(QPalette::Dark));
            painter.drawPolyline(pts+11, 3);
            painter.setPen(Qt::NoPen);
            painter.setBrush(palette.color(QPalette::Background));
            painter.drawPolygon(pts+14, 5);
            break;
        }

        default:
            break;
    }
    painter.end();
//    QSlider::paintEvent(ev);
}
