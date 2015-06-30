#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QTextDocument>
#include <QMouseEvent>
#include <definitions/optionvalues.h>
#include "bubblepathitem.h"

//-------------- <<< *** BubbleButtonPixmapItem *** <<< ---------------
BubbleButtonPixmapItem::BubbleButtonPixmapItem(QIcon &icon, IBubbleEventListener::BubbleEvent event, IBubbleEventListener *listener, QGraphicsItem *parent):
    QGraphicsPixmapItem(icon.pixmap(icon.availableSizes().first()), parent), FBubbleEvent(event), FListener(listener), FMousePressPoint()
{}

void BubbleButtonPixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button()==Qt::LeftButton)
        FMousePressPoint=event->pos();
}

void BubbleButtonPixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button()==Qt::LeftButton)
    {
        if (event->pos()==FMousePressPoint)
            FListener->bubbleMouseEvent(FBubbleEvent);
        FMousePressPoint=QPointF();
    }
}

//-------------- >>> *** BubbleButtonPixmapItem *** >>> ---------------

//------------------ <<< *** BubbleTextProxyItem *** <<< -------------------
BubbleTextProxyItem::BubbleTextProxyItem(MapMessage *AMapMessage, QNetworkAccessManager *ANetworkAccessManager, QGraphicsItem *parent):
    QGraphicsProxyWidget(parent), FListener(AMapMessage), FTextWidget(new BubbleTextWidget())
{    
    connect(FTextWidget, SIGNAL(highlighted(QString)), SLOT(onHighlighted(QString)));
    connect(FTextWidget, SIGNAL(resourceLoaded(QUrl)), SLOT(onResourceChanged()));
    connect(FTextWidget, SIGNAL(textChanged()), SLOT(onResourceChanged()));
    connect(FTextWidget, SIGNAL(anchorClicked(QUrl)), SIGNAL(anchorClicked(QUrl)));
    connect(AMapMessage, SIGNAL(enableAnimation(bool)), FTextWidget, SLOT(setAnimated(bool)));

    FTextWidget->setNetworkAccessManager(ANetworkAccessManager);
    FTextWidget->setFocusPolicy(Qt::NoFocus);
    FTextWidget->setFrameShape(QFrame::NoFrame);
    FTextWidget->setMinimumSize(16, 16);
    FTextWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    FTextWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    FTextWidget->setAnimated(Options::node(OPV_MAP_MESSAGE_ANIMATE).value().toBool());
	FTextWidget->setOpenLinks(false);//FALSE
//    FTextWidget->setTextInteractionFlags(Qt::TextBrowserInteraction);
    setWidget(FTextWidget);
}

void BubbleTextProxyItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons()&Qt::RightButton)           // Drag
    {
        QPointF shift=event->pos()-event->lastPos();
        parentItem()->parentItem()->parentItem()->moveBy(shift.x(), shift.y());
    }
    else
        QGraphicsProxyWidget::mouseMoveEvent(event);
}

void BubbleTextProxyItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsProxyWidget::mousePressEvent(event);

    if (event->button()==Qt::RightButton)           // Accepted for drag
        event->accept();
    else if (cursor().shape()!=Qt::PointingHandCursor)   // Not hovered link
        if (FListener)
            if (event->button()==Qt::LeftButton)
                FMousePressPoint=event->pos();       // Send event
}

void BubbleTextProxyItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    bool processed=false;
    if (cursor().shape()!=Qt::PointingHandCursor)
        if (FListener)
            if (event->button()==Qt::LeftButton)
            {
                if (event->pos()==FMousePressPoint)
                {
                    FListener->bubbleMouseEvent(IBubbleEventListener::BE_Click);
                    processed=true;
                }
                FMousePressPoint=QPoint();
            }

    if (!processed)
        QGraphicsProxyWidget::mouseReleaseEvent(event);
    else
        event->accept();
}

void BubbleTextProxyItem::onHighlighted(const QString &ALink)
{
    if (ALink.isEmpty())
        unsetCursor();
    else
        setCursor(Qt::PointingHandCursor);
}

void BubbleTextProxyItem::onResourceChanged()
{
    QSizeF size=document()->size();
    document()->adjustSize();
    resize(document()->size());
    if (document()->size()!=size)
        qgraphicsitem_cast<BubblePathItem *>(parentItem())->updatePath(true);
}
//------------------ >>> *** BubbleTextProxyItem *** >>> -------------------

//------------------ <<< *** BubbleTextWidget *** <<< -------------------
BubbleTextWidget::BubbleTextWidget(QWidget *parent):
    AnimatedTextBrowser(parent)
{
    setTextInteractionFlags(Qt::TextBrowserInteraction);
    setOpenExternalLinks(true);
}

BubbleTextWidget::~BubbleTextWidget()
{}

//------------------ >>> *** BubbleTextWidget *** >>> -------------------

//------------------ <<< *** BubblePathItem *** <<< -------------------
const int BubblePathItem::FCc=RR*C+1;

BubblePathItem::BubblePathItem(QIcon &ACloseIcon, QIcon &ALocationIcon, SceneObject *ASceneObject, SceneObject *AThisObject, IMap *AMap, QNetworkAccessManager *ANetworkAccessManager, MapMessage *AMapMessage, QGraphicsItem *AParent):
    QGraphicsPathItem(AParent),
    FTextWidget(NULL),
    FCloseButtonItem(NULL),
    FLocationButtonItem(NULL),
    FSceneObject(ASceneObject),
    FThisObject(AThisObject),
    FMap(AMap)
{
    if (!AMapMessage)
        setAcceptedMouseButtons(0);

    FTextWidget=new BubbleTextProxyItem(AMapMessage, ANetworkAccessManager, this);
    FCloseButtonItem = new BubbleButtonPixmapItem(ACloseIcon, IBubbleEventListener::BE_Close, AMapMessage, this);
    FLocationButtonItem = new BubbleButtonPixmapItem(ALocationIcon, IBubbleEventListener::BE_Location, AMapMessage, this);
}

void BubblePathItem::updatePath(bool full)
{
    QRectF brect=FTextWidget->boundingRect();
    FTWidth=brect.width()+FCc*2;
    FTHeight=brect.height()+FCc*2+1;
    FOriginX=-FTWidth/2;
    FOriginY=-FTHeight/2;

    QPainterPath path;
    QPointF end;

    if (full)
    {
        QPointF endPos=FSceneObject->position();
        QPointF startPos=FThisObject->position();
        QRect rect=FMap->sceneRect();
        if (endPos.x()>rect.left()&&
            endPos.x()<rect.right()&&
            endPos.y()>rect.top()&&
            endPos.y()<rect.bottom())
            end=endPos-startPos;
    }

    path.moveTo(FOriginX+RR, FOriginY);
    drawPointer(path, TopLeft, end);
    drawPointer(path, Left, end);
    drawPointer(path, BottomLeft, end);
    drawPointer(path, Bottom, end);
    drawPointer(path, BottomRight, end);
    drawPointer(path, Right, end);
    drawPointer(path, TopRight, end);
    drawPointer(path, Top, end);

    setPath(path);
    FTextWidget->setPos(FOriginX+FCc, FOriginY+FCc);
    FCloseButtonItem->setPos(-FOriginX-FCloseButtonItem->boundingRect().width()/2, FOriginY-FCloseButtonItem->boundingRect().height()/2);
    FLocationButtonItem->setPos(FOriginX-FLocationButtonItem->boundingRect().width()/2, -FOriginY-FLocationButtonItem->boundingRect().height()/2);
    FLocationButtonItem->setVisible(end.isNull());
}

void BubblePathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *AOption, QWidget *AWidget)
{
    painter->setRenderHint(QPainter::Antialiasing);
    QGraphicsPathItem::paint(painter, AOption, AWidget);
}

void BubblePathItem::drawPointer(QPainterPath &APath, BubblePathItem::Direction ADirection, const QPointF &AEnd)
{
    switch (ADirection)
    {
        case TopLeft:
            if (!AEnd.isNull() && AEnd.x()<FOriginX && AEnd.y()<FOriginY)
            {
                APath.lineTo(AEnd);
                APath.lineTo(FOriginX, FOriginY+RR);
            }
            else
                APath.arcTo(FOriginX, FOriginY, RR*2, RR*2, 90, 90);
            break;

        case Left:
            if (!AEnd.isNull() && AEnd.x()<FOriginX && AEnd.y()>FOriginY && AEnd.y()<-FOriginY)
            {
                QPointF cp=APath.currentPosition();
                // qreal cy=(cp.y()-origY-RR)/2;
                qreal cy;
                if (-FOriginY-RR<5)
                    cy=0;
                else
                {
                    cy=AEnd.y()*cp.x()/AEnd.x();
                    if (cy-cp.y()<5)
                        cy=cp.y()+5;
                    else if (-FOriginY-RR-cy<5)
                        cy=-FOriginY-RR-5;
                }
                if (cy-cp.y()>5)
                {
                    cp.setY(cy-5);
                    APath.lineTo(cp);
                }
                APath.lineTo(AEnd);
                if (-FOriginY-RR-cy>5)
                {
                    cp.setY(cy+5);
                    APath.lineTo(cp);
                }
            }
            APath.lineTo(FOriginX, -FOriginY-RR);
            break;

        case BottomLeft:
            if (!AEnd.isNull() && AEnd.x()<FOriginX && AEnd.y()>-FOriginY)
            {
                APath.lineTo(AEnd);
                APath.lineTo(FOriginX+RR, -FOriginY);
            }
            else
                APath.arcTo(FOriginX, -FOriginY-RR*2, RR*2, RR*2, 180, 90);
            break;

        case Bottom:
            if (!AEnd.isNull() && AEnd.y()>-FOriginY && AEnd.x()>FOriginX && AEnd.x()<-FOriginX)
            {
                QPointF cp=APath.currentPosition();
                qreal cx;
                if (-FOriginX-RR<5)
                    cx=0;
                else
                {
                    cx=AEnd.x()*cp.y()/AEnd.y();
                    if (cx-cp.x()<5)
                        cx=5+cp.x();
                    else if (-FOriginX-RR-cx<5)
                        cx=-FOriginX-RR-5;
                }
                if (cx-cp.x()>5)
                {
                    cp.setX(cx-5);
                    APath.lineTo(cp);
                }
                APath.lineTo(AEnd);
                if (-FOriginX-RR-cx>5)
                {
                    cp.setX(cx+5);
                    APath.lineTo(cp);
                }
            }
            APath.lineTo(-FOriginX-RR, -FOriginY);
            break;

        case BottomRight:
            if (!AEnd.isNull() && AEnd.x()>-FOriginX && AEnd.y()>-FOriginY)
            {
                APath.lineTo(AEnd);
                APath.lineTo(-FOriginX, -FOriginY-RR);
            }
            else
                APath.arcTo(-FOriginX-RR*2, -FOriginY-RR*2, RR*2, RR*2, 270, 90);
            break;

        case Right:
            if (!AEnd.isNull() && AEnd.x()>-FOriginX && AEnd.y()>FOriginY && AEnd.y()<-FOriginY)
            {
                QPointF cp=APath.currentPosition();
                qreal cy;
                if (-FOriginY-RR<5)
                    cy=0;
                else
                {
                    cy=AEnd.y()*cp.x()/AEnd.x();
                    if (cp.y()-cy<5)
                        cy=cp.y()-5;
                    else if (cy-FOriginY-RR<5)
                        cy=5+FOriginY+RR;
                }
                if (cp.y()-cy>5)
                {
                    cp.setY(cy+5);
                    APath.lineTo(cp);
                }
                APath.lineTo(AEnd);
                if (cy-FOriginY-RR>5)
                {
                    cp.setY(cy-5);
                    APath.lineTo(cp);
                }
            }
            APath.lineTo(-FOriginX, FOriginY+RR);
            break;

        case TopRight:
            if (!AEnd.isNull() && AEnd.x()>-FOriginX && AEnd.y()<FOriginY)
            {
                APath.lineTo(AEnd);
                APath.lineTo(-FOriginX-RR, FOriginY);
            }
            else
                APath.arcTo(-FOriginX-RR*2, FOriginY, RR*2, RR*2, 0, 90);
            break;

        case Top:
            if (!AEnd.isNull() && AEnd.y()<FOriginY && AEnd.x()>FOriginX && AEnd.x()<-FOriginX)
            {
                QPointF cp=APath.currentPosition();
                qreal cx;
                if (-FOriginX-RR<5)
                    cx=0;
                else
                {
                    cx=AEnd.x()*cp.y()/AEnd.y();
                    if (cp.x()-cx<5)
                        cx=cp.x()-5;
                    else if (cx-FOriginX-RR<5)
                        cx=5+FOriginX+RR;
                }
                if (cp.x()-cx>5)
                {
                    cp.setX(cx+5);
                    APath.lineTo(cp);
                }
                APath.lineTo(AEnd);
                if (-FOriginX-RR-cx>5)
                {
                    cp.setX(cx-5);
                    APath.lineTo(cp);
                }
            }
            APath.closeSubpath();
            break;
    }
}
//------------------ >>> *** BubblePathItem *** >>> -------------------
