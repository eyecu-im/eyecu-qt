#ifndef BUBBLEPATHITEM_H
#define BUBBLEPATHITEM_H

#include <QGraphicsPathItem>
#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>
#include <QTextBrowser>
#include <QGraphicsSceneMouseEvent>
#include <QIcon>
#include <utils/animatedtextbrowser.h>
#include "mapmessage.h"

#define RR                10
#define C                 0.29289321881345247559915563789515

class BubbleProxyItem: public QGraphicsItem
{
public:
    // Enable the use of qgraphicsitem_cast with this item.
    enum { Type = UserType + 10};
    virtual int type() const { return Type; }

    BubbleProxyItem(QGraphicsItem *parent = NULL): QGraphicsItem(parent){setFlags(ItemHasNoContents);}
	void paint(QPainter *APainter, const QStyleOptionGraphicsItem *AOption, QWidget *AWidget=0) {Q_UNUSED(APainter) Q_UNUSED(AOption) Q_UNUSED(AWidget)} // Nothing to paint
    QRectF boundingRect() const {return QRectF(0, 0, 1, 1);}
};

class BubbleButtonPixmapItem : public QGraphicsPixmapItem
{
public:
    BubbleButtonPixmapItem(QIcon &icon, IBubbleEventListener::BubbleEvent event, IBubbleEventListener *FListener=NULL, QGraphicsItem *parent=NULL);
    // Enable the use of qgraphicsitem_cast with this item.
    enum { Type = UserType + 12};
    virtual int type() const { return Type; }

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    const IBubbleEventListener::BubbleEvent FBubbleEvent;
    IBubbleEventListener    *FListener;
    QPointF                 FMousePressPoint;
    QTimer                  FTimer;
};


class BubbleTextWidget : public AnimatedTextBrowser
{
public:
    BubbleTextWidget(QWidget *parent=NULL);
    ~BubbleTextWidget();
};

class BubbleTextProxyItem : public QGraphicsProxyWidget
{
    Q_OBJECT

public:
    BubbleTextProxyItem(MapMessage *AMapMessage, QNetworkAccessManager *ANetworkAccessManager, QGraphicsItem *parent);
    // Enable the use of qgraphicsitem_cast with this item.
    enum { Type = UserType + 13};
    virtual int type() const { return Type; }
    QTextDocument *document() {return FTextWidget->document();}

public slots:
    virtual void onResourceChanged();

protected:
	// QGraphicsItem interface
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

protected slots:
    void onHighlighted(const QString &ALink);

signals:
    void anchorClicked(const QUrl &AUrl);

private:
    IBubbleEventListener    *FListener;
    BubbleTextWidget        *FTextWidget;
    QPointF                 FMousePressPoint;
	bool					FSuppressContextMenu;
};

class BubblePathItem : public QGraphicsPathItem
{
    enum Direction {
        Top,
        Bottom,
        Left,
        Right,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

public:
	BubblePathItem(QIcon &ACloseIcon, QIcon &ALocationIcon, SceneObject *ASceneObject, SceneObject *AThisObject, IMap *AMap, QNetworkAccessManager *ANetworkAccessManager, MapMessage *AMapMessage=NULL, QGraphicsItem *AParent=NULL);
    // Enable the use of qgraphicsitem_cast with this item.
    enum { Type = UserType + 11};


    BubbleTextProxyItem *textItem() {return FTextWidget;}

    void updatePath(bool full);
	const MapObject * targetObject() const {return FSceneObject->mapObject(); }
	const SceneObject * targetSceneObject() const {return FSceneObject; }

	bool initialized() const;
	void setInitialized(bool AInitialized);

	// QGraphicsItem interface
	virtual int type() const { return Type; }
	// QGraphicsPathItem interface
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *AOption, QWidget *AWidget);

protected:
    virtual void drawPointer(QPainterPath &APath, Direction ADirection, const QPointF &AEnd);

private:
    static const   int      FCc;
    BubbleTextProxyItem     *FTextWidget;
    BubbleButtonPixmapItem  *FCloseButtonItem;
    BubbleButtonPixmapItem  *FLocationButtonItem;
	SceneObject				*FSceneObject;
	SceneObject				*FThisObject;
	IMap					*FMap;

    int                     FTWidth;
    int                     FTHeight;
    int                     FOriginX;
    int                     FOriginY;

	bool					FInitialized;
};

#endif // BUBBLEPATHITEM_H
