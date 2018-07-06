#include <definitions/messagedataroles.h>
#include <definitions/mapobjecttyperole.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <definitions/optionnodes.h>
#include <definitions/optionvalues.h>
#include <definitions/resources.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/notificationhandlerorders.h>
#include <definitions/notificationtypes.h>
#include <definitions/notificationdataroles.h>
#include <definitions/namespaces.h>
#include <definitions/messageviewurlhandlerorders.h>
#include <utils/iconstorage.h>
#include <utils/textmanager.h>

#include <MapObject>
#include <QPicture>
#include <QGraphicsItem>
#include <QGraphicsTextItem>
#include <QDesktopServices>
#include <QTextCursor>

#include "bubblepathitem.h"

#define ADR_STREAM_JID          Action::DR_StreamJid
#define ADR_CONTACT_JID         Action::DR_Parametr4

MapMessage::MapMessage():
    FOptionsManager(NULL),
    FMessageProcessor(NULL),
    FMainWindowPlugin(NULL),
    FNotifications(NULL),
    FMapContacts(NULL),
	FMap(NULL),
	FGeoMap(NULL),
    FNetworkAccessManager(NULL),
    FMessageWidget(NULL),
    FOpacityAnimation(500), // Milliseconds
    FCurrentMessageId(0)
{}

MapMessage::~MapMessage()
{}

//-----------------------------
void MapMessage::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Map Message");
	APluginInfo->description = tr("Displays messages on the map");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(MAP_UUID);
    APluginInfo->dependences.append(MAPCONTACTS_UUID);
    APluginInfo->dependences.append(MESSAGEPROCESSOR_UUID);
}

bool MapMessage::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(AInitOrder);

    IPlugin *plugin = APluginManager->pluginInterface("IMessageProcessor").value(0);
    if (plugin)
    {
        FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());
        connect(FMessageProcessor->instance(),SIGNAL(messageNotifyInserted(int)), SLOT(onMessageNotifyInserted(int)));
        connect(FMessageProcessor->instance(),SIGNAL(messageNotifyRemoved(int)), SLOT(onMessageNotifyRemoved(int)));
    }
    else
        return false;

    plugin = APluginManager->pluginInterface("IMap").value(0,NULL);
    if (plugin)
    {
		FGeoMap = (FMap = qobject_cast<IMap *>(plugin->instance()))->geoMap();
		connect(FGeoMap, SIGNAL(mapObjectAboutToRemove(const MapObject *)), SLOT(onMapObjectAboutToRemove(const MapObject*)));
    }
    else
        return false;    

    plugin = APluginManager->pluginInterface("IMapContacts").value(0,NULL);
    if (plugin)
        FMapContacts = qobject_cast<IMapContacts *>(plugin->instance());
    else
        return false;

    plugin = APluginManager->pluginInterface("INotifications").value(0,NULL);
    if (plugin)
        FNotifications = qobject_cast<INotifications *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IMainWindowPlugin").value(0,NULL);
    if (plugin)
        FMainWindowPlugin = qobject_cast<IMainWindowPlugin *>(plugin->instance());

    plugin= APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    IUrlProcessor * urlProcessor = NULL;
    plugin= APluginManager->pluginInterface("IUrlProcessor").value(0,NULL);
    if (plugin)
    {
        QObject *object=plugin->instance();
        urlProcessor = qobject_cast<IUrlProcessor *>(object);
    }
    FNetworkAccessManager = urlProcessor?urlProcessor->networkAccessManager():new QNetworkAccessManager(this);

    connect(Options::instance(), SIGNAL(optionsChanged(OptionsNode)), SLOT(onOptionsChanged(OptionsNode)));

    AInitOrder = 150;   // This one should be initialized AFTER Map
    return true;
}

bool MapMessage::initObjects()
{
	if (FGeoMap)
    {
		FGeoMap->setObjectHandler(MOT_MESSAGE_BUBBLE, this);
		FGeoMap->addObjectStateHandler(MOT_MESSAGE_BUBBLE, this);
		FGeoMap->registerDataType(MDR_MESSAGE_TEXT, MOT_MESSAGE_BUBBLE, 100, MOP_NONE, MOP_CENTER);
		FGeoMap->addDataHolder(MOT_MESSAGE_BUBBLE, this);
		FGeoMap->addObjectStateHandler(MOT_CONTACT, this);
    }
    else
        return false;

    IconStorage *storage=IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
    if (storage)
    {
        FIconCloseInactive=storage->getIcon(MNI_CLOSE_INACTIVE);
        FIconCloseActive=storage->getIcon(MNI_CLOSE_ACTIVE);
        FIconLocationInactive=storage->getIcon(MNI_GEOLOC_OFF);
        FIconLocationActive=storage->getIcon(MNI_GEOLOC);
    }

    if (FNotifications)
        FNotifications->insertNotificationHandler(NHO_DEFAULT, this);

	insertUrlHandler(MVUHO_MAPMESSAGE_DEFAULT, this);

    return true;
}

bool MapMessage::initSettings()
{
    Options::setDefaultValue(OPV_MAP_MESSAGE_SHOW, true);
    Options::setDefaultValue(OPV_MAP_MESSAGE_AUTOFOCUS, true);
    Options::setDefaultValue(OPV_MAP_MESSAGE_ANIMATE, true);

    if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> MapMessage::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
    if (ANodeId == OPN_MAP)
    {
		widgets.insertMulti(OHO_MAP_MESSAGE, FOptionsManager->newOptionsDialogHeader(tr("Messages"), AParent));
		widgets.insertMulti(OWO_MAP_MESSAGE, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MAP_MESSAGE_AUTOFOCUS),tr("Autofocus on the map"), AParent));
		widgets.insertMulti(OWO_MAP_MESSAGE, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MAP_MESSAGE_SHOW),tr("Show messages on the map"), AParent));
		widgets.insertMulti(OWO_MAP_MESSAGE, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MAP_MESSAGE_ANIMATE),tr("Enable animation in the messages on the map"), AParent));
    }
    return widgets;
}

void MapMessage::onMessageNotifyInserted(int AMessageId)
{
    Message message=FMessageProcessor->notifiedMessage(AMessageId);
    QString from=message.from();
	if (!FGeoMap->getObject(MOT_MESSAGE_BUBBLE, "MessageBubble") &&    // No bubble displayed
		FGeoMap->getObject(MOT_CONTACT, from))                         // There is a contact on the map
    {
        if(Options::node(OPV_MAP_MESSAGE_AUTOFOCUS).value().toBool())
            FMapContacts->showContact(from, false);
        FCurrentMessageId=AMessageId;
		FGeoMap->addObject(MOT_MESSAGE_BUBBLE, "MessageBubble", 0, 0); // Create bubble
        FNotifiedMessages.append(FMessageProcessor->notifyByMessage(AMessageId));   // Add notification to the list
    }
}

void MapMessage::onMessageNotifyRemoved(int AMessageId)
{
    FOpacityAnimation.stop();
    Message message;
    QString from;
	MapObject *contactMapObject=NULL;
    FCurrentMessageId=0;

    FNotifiedMessages.removeOne(FMessageProcessor->notifyByMessage(AMessageId));   // Remove notification from the list
    QList<int> notifiedMessages=FMessageProcessor->notifiedMessages();
    for (QList<int>::const_iterator it=notifiedMessages.constBegin(); it!=notifiedMessages.constEnd(); it++)
    {
        message=FMessageProcessor->notifiedMessage(*it);
        from=message.from();
		contactMapObject=FGeoMap->getObject(MOT_CONTACT, from);
        if (contactMapObject)   // Object found
        {
            FCurrentMessageId=*it;
            break;
        }
    }

    if (FCurrentMessageId)   // There is a message to display
    {
		MapObject *object=FGeoMap->getObject(MOT_MESSAGE_BUBBLE, "MessageBubble");
        if (object)
        {
			BubblePathItem *bubble= qgraphicsitem_cast<BubblePathItem *>(FGeoMap->getScene()->sceneObject(object)->getElementByRole(MDR_MESSAGE_TEXT)->childItems().first());
            if (contactMapObject == bubble->targetObject())
            {
                emit mapDataChanged(MOT_MESSAGE_BUBBLE, "MessageBubble", MDR_MESSAGE_TEXT);
                bubble->updatePath(true);
                return;
            }
        }

        if(Options::node(OPV_MAP_MESSAGE_AUTOFOCUS).value().toBool())
            FMapContacts->showContact(from, false);

		FGeoMap->removeObject(MOT_MESSAGE_BUBBLE, "MessageBubble");    // Remove bubble
		FGeoMap->addObject(MOT_MESSAGE_BUBBLE, "MessageBubble", 0, 0); // Create bubble
    }
    else
		FGeoMap->removeObject(MOT_MESSAGE_BUBBLE, "MessageBubble");    // Remove bubble
}

void MapMessage::onMapObjectInserted(int AType, const QString &AId)
{
    if (AType==MOT_MESSAGE_BUBBLE)
        emit mapDataChanged(AType, AId, MDR_MESSAGE_TEXT);
}

void MapMessage::onMapObjectRemoved(int AType, const QString &AId)
{
    Q_UNUSED(AType)
    Q_UNUSED(AId)
}

void MapMessage::onMapObjectAboutToRemove(const MapObject *AMapObject)
{
	MapObject *bubbleObject = FGeoMap->getObject(MOT_MESSAGE_BUBBLE, "MessageBubble");
    if (bubbleObject)
    {
		BubblePathItem *bubble= qgraphicsitem_cast<BubblePathItem *>(FGeoMap->getScene()->sceneObject(bubbleObject)->getElementByRole(MDR_MESSAGE_TEXT)->childItems().first());
        if (bubble->targetObject() == AMapObject)
            onMessageNotifyRemoved(FCurrentMessageId);
    }
}

void MapMessage::onBubbleAnchorClicked(const QUrl &AUrl)
{
	Jid streamJid = FMessageStreamJids.value(sender());
    Jid contactJid = FMessageContactJids.value(sender());
    for (QMap<int, IBubbleUrlEventHandler*>::ConstIterator it=FUrlListeners.constBegin(); it!=FUrlListeners.constEnd(); it++)
        if ((*it)->bubbleUrlOpen(it.key(), AUrl, streamJid, contactJid))
            break;
}

QGraphicsItem * MapMessage::mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement)
{
    if (ARole==MDR_MESSAGE_TEXT)
    {
        Message message=FMessageProcessor->notifiedMessage(FCurrentMessageId);
        BubblePathItem *bubble;
        if (ACurrentElement)
            bubble=qgraphicsitem_cast<BubblePathItem *>(ACurrentElement->childItems().first());
        else
        {
            ACurrentElement = new BubbleProxyItem();
			bubble = new BubblePathItem(FIconCloseActive, FIconLocationActive, FGeoMap->getScene()->sceneObject(FGeoMap->getObject(MOT_CONTACT, message.from())), ASceneObject, FMap, FNetworkAccessManager, this, ACurrentElement);
            bubble->setBrush(QBrush(Qt::white));
            FOpacityAnimation.setGraphicsItem(bubble);
            FOpacityAnimation.start();
        }
        BubbleTextProxyItem *textWidget=bubble->textItem();
        FMessageStreamJids.insert(textWidget, message.to());
        FMessageContactJids.insert(textWidget, message.from());
        connect(textWidget, SIGNAL(anchorClicked(QUrl)), SLOT(onBubbleAnchorClicked(QUrl)));
        connect(textWidget, SIGNAL(destroyed(QObject*)), SLOT(onBubbleDestroyed(QObject*)));

        QTextDocument doc;
		FMessageProcessor->messageToText(message, &doc);
        QTextDocument *d=textWidget->document();
        d->setHtml(doc.toHtml());

        QString subject=message.subject();
        if (!subject.isEmpty()) // Message has subject element
        {
            // Create HTML code for subject
            QString html(tr("Subject"));
            html.append(": <b>").append(message.subject()).append("</b>");

            // Insert subject HTML block
            QTextCursor cursor(d);
            cursor.movePosition(QTextCursor::Start);
            cursor.insertHtml(html);

            // Add horizontal ruler (<hr>) element
            cursor.insertBlock();
            cursor.movePosition(QTextCursor::PreviousBlock);
            QTextBlockFormat format;
            format.setProperty(QTextFormat::BlockTrailingHorizontalRulerWidth, 1);
            cursor.setBlockFormat(format);
        }
        d->adjustSize();
        textWidget->resize(d->size());
        bubble->updatePath(false);
    }
    return ACurrentElement;
}

void MapMessage::updatePosition(SceneObject *ASceneObject)
{
    switch (ASceneObject->mapObject()->objectType())
    {
        case MOT_MESSAGE_BUBBLE:
        {
            QGraphicsItem *messageText=ASceneObject->getElementByRole(MDR_MESSAGE_TEXT);
            if (messageText)
            {
                QList<QGraphicsItem *> children=messageText->childItems();
                if (!children.isEmpty())
                {                    
					BubblePathItem *bubble=qgraphicsitem_cast<BubblePathItem *>(children.first());
					if (bubble->initialized())
                        bubble->updatePath(true);
					else
					{
						bubble->setInitialized(true);
						QPointF newPosition(0, -bubble->boundingRect().height()/2-40);
						ASceneObject->setPosition(newPosition);
					}
                }
            }
            break;
        }

        case MOT_CONTACT:
        {
			MapObject *object=FGeoMap->getObject(MOT_MESSAGE_BUBBLE, "MessageBubble");
            if (object)
            {
				BubblePathItem *bubble= qgraphicsitem_cast<BubblePathItem *>(FGeoMap->getScene()->sceneObject(object)->getElementByRole(MDR_MESSAGE_TEXT)->childItems().first());
                if (ASceneObject == bubble->targetSceneObject())
                    bubble->updatePath(true);
            }
        }
    }
}

void MapMessage::onOptionsChanged(const OptionsNode &ANode)
{
    if (ANode.path()==OPV_MAP_MESSAGE_ANIMATE)
        emit enableAnimation(ANode.value().toBool());
}

void MapMessage::onBubbleDestroyed(QObject *AObject)
{
    FMessageStreamJids.remove(AObject);
    FMessageContactJids.remove(AObject);
}

void MapMessage::objectUpdated(SceneObject *ASceneObject, int ARole)
{
    Q_UNUSED(ASceneObject)
    Q_UNUSED(ARole)
}

QVariant MapMessage::itemChange(SceneObject *ASceneObject, QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    switch (change)
    {
        case QGraphicsItem::ItemPositionHasChanged:
        {
			MapObject *mapObject=ASceneObject->mapObject();
            switch (mapObject->objectType())
            {
                case MOT_MESSAGE_BUBBLE:
                {
                    QGraphicsItem *item=ASceneObject->getElementByRole(MDR_MESSAGE_TEXT);
                    if (item)
                        qgraphicsitem_cast<BubblePathItem *>(item->childItems().first())->updatePath(true);
                    break;
                }

                case MOT_CONTACT:
                {
					MapObject *object=FGeoMap->getObject(MOT_MESSAGE_BUBBLE, "MessageBubble");
                    if (object)
                    {
                        QGraphicsItem *messageText=ASceneObject->getElementByRole(MDR_MESSAGE_TEXT);
                        if (messageText)
                        {
                            BubblePathItem *bubble=qgraphicsitem_cast<BubblePathItem *>(messageText->childItems().first());
                            if (mapObject == bubble->targetObject())
                                bubble->updatePath(true);
                        }
                    }
                    break;
                }
            }
        }
        default:
            break;
    }
    return value;
}

//INotificationHandler
bool MapMessage::showNotification(int AOrder, ushort AKind, int ANotifyId, const INotification &ANotification)
{
	Q_UNUSED(AOrder)
	Q_UNUSED(ANotifyId)

    if (FMainWindowPlugin->mainWindow()->instance()->isActiveWindow())
		if (FMap->isVisible())
            if (ANotification.typeId==NNT_CHAT_MESSAGE || ANotification.typeId==NNT_NORMAL_MESSAGE)     // Only chat and normal messages are in interest
				if (FGeoMap->getObject(MOT_CONTACT, ANotification.data.value(NDR_CONTACT_JID).toString())) // Map contains contact
                    if (AKind==INotification::PopupWindow)
                        return true;
    return false;
}

void MapMessage::bubbleMouseEvent(IBubbleEventListener::BubbleEvent event)
{
    switch (event)
    {
        case IBubbleEventListener::BE_Click:
            FMessageProcessor->showNotifiedMessage(FCurrentMessageId);
            break;

        case IBubbleEventListener::BE_Location:
            FMapContacts->showContact(FMessageProcessor->notifiedMessage(FCurrentMessageId).from(), false);
            break;

        case IBubbleEventListener::BE_Close:
            FMessageProcessor->removeMessageNotify(FCurrentMessageId);
            break;
    }
}

void MapMessage::insertUrlHandler(int AOrder, IBubbleUrlEventHandler *ABubbleUrlEventHandler)
{
    FUrlListeners.insert(AOrder, ABubbleUrlEventHandler);
}

void MapMessage::removeUrlHandler(int AOrder, IBubbleUrlEventHandler *ABubbleUrlEventHandler)
{
    if (FUrlListeners.contains(AOrder, ABubbleUrlEventHandler))
		FUrlListeners.remove(AOrder, ABubbleUrlEventHandler);
}

bool MapMessage::bubbleUrlOpen(int AOrder, const QUrl &AUrl, const Jid &AStreamJid, const Jid &AContactJid)
{
	Q_UNUSED(AOrder)
	Q_UNUSED(AStreamJid)
	Q_UNUSED(AContactJid)

	return QDesktopServices::openUrl(AUrl);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapmessage,MapMessage)
#endif
