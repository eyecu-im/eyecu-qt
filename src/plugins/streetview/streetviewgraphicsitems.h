#ifndef STREETVIEWGRAPHICSITEMS_H
#define STREETVIEWGRAPHICSITEMS_H

#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include "streetview.h"

class StreetViewGraphicsItem : public QGraphicsPathItem
{
public:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *AOption, QWidget *AWidget);
};

class StreetViewPoint: public StreetViewGraphicsItem
{
public:
    StreetViewPoint(StreetView *AStreetView);
    // Enable the use of qgraphicsitem_cast with this item.
    enum { Type = UserType + 20};
    virtual int type() const { return Type; }

    // QGraphicsItem interface
    virtual QRectF boundingRect() const;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    bool        FDragging;
    StreetView *FStreetView;
};

class StreetViewMarker : public StreetViewGraphicsItem
{
public:
    StreetViewMarker();
    // Enable the use of qgraphicsitem_cast with this item.
    enum { Type = UserType + 21};
    virtual int type() const { return Type; }
};

#endif // STREETVIEWGRAPHICSITEMS_H
