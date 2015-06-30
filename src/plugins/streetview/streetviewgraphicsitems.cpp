#include <QDebug>
#include "streetviewgraphicsitems.h"

void StreetViewGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *AOption, QWidget *AWidget)
{
    painter->setRenderHint(QPainter::Antialiasing);
    QGraphicsPathItem::paint(painter, AOption, AWidget);
}

/**********************************************/

StreetViewPoint::StreetViewPoint(StreetView *AStreetView):
    FDragging(false),
    FStreetView(AStreetView)
{
    QPainterPath path;
    path.addEllipse(3, 3, 6, 6);
    setPath(path);
    setBrush(QColor(Qt::cyan));
}

QRectF StreetViewPoint::boundingRect() const
{
    return QRectF(0, 0, 12, 12);
}

void StreetViewPoint::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsPathItem::mousePressEvent(event);

    if (event->button()==Qt::RightButton)           // Accepted for drag
        event->accept();
}

void StreetViewPoint::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "StreetViewPoint::mouseReleaseEvent(" << event << ")";
    if (event->button()==Qt::RightButton)           // Accepted for drag
    {
        if (FDragging)
        {
            FDragging=false;
            FStreetView->dropped();
        }
        event->accept();
    }
    else
        QGraphicsPathItem::mouseReleaseEvent(event);
}

void StreetViewPoint::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons()&Qt::RightButton)           // Drag
    {
        QPointF shift=event->pos()-event->lastPos();
        parentItem()->moveBy(shift.x(), shift.y());
        FDragging=true;
    }
    else
        QGraphicsPathItem::mouseMoveEvent(event);
}

void StreetViewPoint::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    event->accept(); // Disable context menu
}

/**********************************************/

StreetViewMarker::StreetViewMarker()
{
    QPainterPath path;
    path.moveTo(5, 0);
    path.lineTo(11, 7);
    path.lineTo(0, 7);
    path.lineTo(5, 0);

    setPath(path);
    setBrush(QColor(Qt::red));
}
