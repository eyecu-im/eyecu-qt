#ifndef MAPMESSAGE_H
#define MAPMESSAGE_H

#include <QLabel>

#include <interfaces/imessageprocessor.h>
#include <interfaces/imapmessage.h>
#include <interfaces/imainwindow.h>
#include <interfaces/imapcontacts.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/igeoloc.h>
#include <interfaces/inotifications.h>
#include <interfaces/imap.h>
#include <interfaces/iurlprocessor.h>
#include <definitions/mapobjectdatarole.h>

#include "opacityanimation.h"

class IBubbleEventListener
{
public:
    typedef enum {
        BE_Click,
        BE_Location,
        BE_Close
    } BubbleEvent;

    virtual QObject *instance() =0;
    virtual void bubbleMouseEvent(BubbleEvent event) = 0;
};

class MapMessage: public QObject,
                  public IPlugin,
                  public IMapMessage,
				  public IOptionsDialogHolder,
				  public MapSceneObjectHandler,
				  public MapSceneObjectStateHandler,
				  public MapObjectDataHolder,
                  public INotificationHandler,                  
				  public IBubbleEventListener,
				  public IBubbleUrlEventHandler
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IMapMessage IOptionsDialogHolder MapSceneObjectHandler MapSceneObjectStateHandler MapObjectDataHolder INotificationHandler IBubbleUrlEventHandler)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapMessage")
#endif
public:
    typedef enum
    {
        IT_CloseInactive,
        IT_CloseActive
    } IconType;

    MapMessage();
    ~MapMessage();
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return MAPMESSAGE_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    //IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

    //IMapDataHolder
	virtual QGraphicsItem *mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement);

    //IMapSceneObjectHandler
	virtual void mouseHoverEnter(SceneObject *ASceneObject) {Q_UNUSED(ASceneObject)}
	virtual void mouseHoverLeave(SceneObject *ASceneObject) {Q_UNUSED(ASceneObject)}
	virtual bool mouseClicked(SceneObject *ASceneObject, Qt::MouseButton AButton) {Q_UNUSED(ASceneObject) Q_UNUSED(AButton) return false;}
	virtual bool mouseDoubleClicked(SceneObject *ASceneObject, Qt::MouseButton AButton) {Q_UNUSED(ASceneObject) Q_UNUSED(AButton) return false;}
	virtual bool contextMenu(SceneObject *ASceneObject, QMenu *AMenu) {Q_UNUSED(ASceneObject) Q_UNUSED(AMenu) return false;}
    virtual bool mouseHit() const {return true; }
    virtual float zValue() const {return 4.0;}
	virtual void objectUpdated(SceneObject *ASceneObject, int ARole=MDR_NONE);
	virtual QString toolTipText(const MapObject *AMapObject, const QHelpEvent *AEvent) const {Q_UNUSED(AMapObject) Q_UNUSED(AEvent) return QString();}
	virtual bool isSticky(const SceneObject *ASceneObject) const {Q_UNUSED(ASceneObject) return true;}

    //IMapSceneObjectStateHandler
	virtual void updatePosition(SceneObject *ASceneObject);
	virtual QVariant itemChange(SceneObject *ASceneObject, QGraphicsItem::GraphicsItemChange change, const QVariant &value);
    //--------

    //INotificationHandler
    virtual bool showNotification(int AOrder, ushort AKind, int ANotifyId, const INotification &ANotification);

    //IBubbleEventListener
    virtual void bubbleMouseEvent(BubbleEvent event);

    //IMapMessage
    virtual void insertUrlHandler(int AOrder, IBubbleUrlEventHandler *ABubbleUrlEventHandler);
    virtual void removeUrlHandler(int AOrder, IBubbleUrlEventHandler *ABubbleUrlEventHandler);

	// IBubbleUrlEventHandler interface
	virtual bool bubbleUrlOpen(int AOrder, const QUrl &AUrl, const Jid &AStreamJid, const Jid &AContactJid);

public slots:
    void onMapObjectInserted(int AType, const QString &AId);        // Map object inserted
    void onMapObjectRemoved(int AType, const QString &AId);         // Map object removed
    void onMapObjectShowed(int AType, const QString &AId){Q_UNUSED(AType) Q_UNUSED(AId)}  // SLOT: Map object showed
	void onMapObjectAboutToRemove(const MapObject *AMapObject);    // Map object is bout to be removed

protected slots:
    void onBubbleAnchorClicked(const QUrl &AUrl);
    void onMessageNotifyInserted(int AMessageId);
    void onMessageNotifyRemoved(int AMessageId);
    void onOptionsChanged(const OptionsNode &ANode);
    void onBubbleDestroyed(QObject *AObject);	

signals:
    //IMapObjectDataHolder
    void mapDataChanged(int AType, const QString &AId, int ARole);
    void enableAnimation(bool);

private:
    IOptionsManager     *FOptionsManager;
    IMessageProcessor   *FMessageProcessor;
    IMainWindowPlugin   *FMainWindowPlugin;
    INotifications      *FNotifications;
    IMapContacts        *FMapContacts;
	IMap				*FMap;
	GeoMap				*FGeoMap;
    QNetworkAccessManager *FNetworkAccessManager;
    QLabel              *FMessageWidget;
    QList<int>          FNotifiedMessages;
    QIcon               FIconCloseInactive;
    QIcon               FIconCloseActive;
    QIcon               FIconLocationInactive;
    QIcon               FIconLocationActive;

    OpacityAnimation    FOpacityAnimation;

    QMultiMap<int, IBubbleUrlEventHandler *> FUrlListeners;
    QHash<QObject *, Jid> FMessageStreamJids;
    QHash<QObject *, Jid> FMessageContactJids;
    int                 FCurrentMessageId;		
};

#endif // MAPMESSAGEE_H
