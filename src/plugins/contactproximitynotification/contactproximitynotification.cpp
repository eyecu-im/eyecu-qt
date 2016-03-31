#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/notificationtypeorders.h>
#include <definitions/notificationtypes.h>
#include <definitions/notificationdataroles.h>
#include <definitions/rosternotifyorders.h>
#include <definitions/resources.h>
#include <definitions/soundfiles.h>
#include <definitions/menuicons.h>
#include <interfaces/irostersview.h>
#include <utils/logger.h>
#include <utils/iconstorage.h>

#include "contactproximitynotification.h"
#include "contactproximitynotificationoptions.h"

#define ADR_CONTACT_JID         Action::DR_Parametr1

ContactProximityNotification::ContactProximityNotification(QObject *parent):
    QObject(parent),
    FNotifications(NULL),
    FPositioning(NULL),
    FMapContacts(NULL),
    FMessageProcessor(NULL),
    FMessageWidgets(NULL),
    FPresenceManager(NULL),
    FOptionsManager(NULL)
{}

void ContactProximityNotification::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Contact Proximity Notification");
    APluginInfo->description = tr("Notifies user about proximity of contacts in the roster");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(GEOLOC_UUID);
    APluginInfo->dependences.append(POSITIONING_UUID);
    APluginInfo->dependences.append(NOTIFICATIONS_UUID);
    APluginInfo->dependences.append(PRESENCE_UUID);
}

bool ContactProximityNotification::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    IPlugin *plugin = APluginManager->pluginInterface("IGeoloc").value(0,NULL);
    if (plugin)
    {
        connect(plugin->instance(),SIGNAL(locationReceived(Jid,Jid,MercatorCoordinates,bool)),SLOT(onLocationReceived(Jid,Jid,MercatorCoordinates,bool)));
        connect(plugin->instance(),SIGNAL(locationRemoved(Jid,Jid)),SLOT(onLocationRemoved(Jid,Jid)));
    }
    else
        return false;

    plugin = APluginManager->pluginInterface("IPositioning").value(0,NULL);
    if (plugin)
    {
        FPositioning = qobject_cast<IPositioning *>(plugin->instance());
		connect(FPositioning->instance(), SIGNAL(newPositionAvailable(GeolocElement)), SLOT(onNewPositionAvailable(GeolocElement)));
    }
    else
        return false;

    plugin = APluginManager->pluginInterface("INotifications").value(0, NULL);
    if (plugin)
    {
        FNotifications = qobject_cast<INotifications *>(plugin->instance());
        connect(FNotifications->instance(), SIGNAL(notificationActivated(int)), SLOT(onNotificationActivated(int)));
        connect(FNotifications->instance(), SIGNAL(notificationRemoved(int)), SLOT(onNotificationRemoved(int)));
    }
    else
        return false;

    plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0,NULL);
    if (plugin)
		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());
    else
        return false;

    plugin = APluginManager->pluginInterface("IMapContacts").value(0,NULL);
    if (plugin)
    {
        FMapContacts = qobject_cast<IMapContacts *>(plugin->instance());
        connect(FMapContacts->instance(), SIGNAL(contactShowedOnTheMap(QString)), SLOT(onContactShowedOnTheMap(QString)));
    }
    else
    {
        plugin = APluginManager->pluginInterface("IMessageProcessor").value(0);
        if (plugin)
            FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());
        plugin = APluginManager->pluginInterface("IMessageWidgets").value(0);
        if (plugin)
        {
            FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
            connect(FMessageWidgets->instance(),SIGNAL(chatWindowCreated(IMessageChatWindow *)),SLOT(onChatWindowCreated(IMessageChatWindow *)), Qt::QueuedConnection);
        }
    }

    AInitOrder = 150;
    return true;
}

bool ContactProximityNotification::initObjects()
{
    if (FNotifications)
    {
        INotificationType notifyType;
        notifyType.order = NTO_CONTACTPROXIMITY;
        notifyType.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_PROXIMITY);
        notifyType.title = tr("When contact appears nearby");
        notifyType.kindMask = INotification::PopupWindow|INotification::SoundPlay|INotification::RosterNotify|INotification::TrayNotify|INotification::TrayAction;
        notifyType.kindDefs = notifyType.kindMask&~(INotification::TrayNotify|INotification::TrayAction);
        FNotifications->registerNotificationType(NNT_CONTACTPROXIMITY, notifyType);
    }
    return true;
}

bool ContactProximityNotification::initSettings()
{
    if (FOptionsManager)
    {
		IOptionsDialogNode dnode = {ONO_CONTACTPROXIMITY, OPN_CONTACTPROXIMITY, MNI_PROXIMITY, tr("Contact proximity")};
        FOptionsManager->insertOptionsDialogNode(dnode);
		FOptionsManager->insertOptionsDialogHolder(this);
    }
    Options::setDefaultValue(OPV_CONTACTPROXIMITYNOTIFICATIONS_DISTANCE, 1000);
    Options::setDefaultValue(OPV_CONTACTPROXIMITYNOTIFICATIONS_TRESHOLD, 200);
    Options::setDefaultValue(OPV_CONTACTPROXIMITYNOTIFICATIONS_IGNOREOWN, true);
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> ContactProximityNotification::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
    if (ANodeId == OPN_CONTACTPROXIMITY)
        widgets.insertMulti(OWO_CONTACTPROXIMITYNOTIFICATION, new ContactProximityNotificationOptions(AParent));
    return widgets;
}

void ContactProximityNotification::displayNotification(const Jid &AStreamJid, const Jid &AContactJid)
{
    if (AStreamJid.bare()==AContactJid.bare())
        return;

    INotification notify;
    notify.kinds = FNotifications->enabledTypeNotificationKinds(NNT_CONTACTPROXIMITY);
/*
    if (FMessageWidgets)
    {
        IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
        if (window && window->isActiveTabPage())    // The window is existing and is an active tab page!
            notify.kinds = 0;                       // So, do not have to notify!
    }
*/
    if (notify.kinds)
    {
        QString html=tr("Appeared nearby!");
        notify.typeId = NNT_CONTACTPROXIMITY;
        notify.data.insert(NDR_STREAM_JID, AStreamJid.full());
        notify.data.insert(NDR_CONTACT_JID, AContactJid.full());
        notify.data.insert(NDR_ICON, IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_PROXIMITY));
        notify.data.insert(NDR_ROSTER_ORDER, RNO_CONTACTPROXIMITY);
        notify.data.insert(NDR_ROSTER_FLAGS, IRostersNotify::Blink|IRostersNotify::AllwaysVisible|IRostersNotify::HookClicks);
        notify.data.insert(NDR_POPUP_HTML, html);
        notify.data.insert(NDR_POPUP_TITLE, FNotifications->contactName(AStreamJid, AContactJid));
        notify.data.insert(NDR_POPUP_IMAGE, FNotifications->contactAvatar(AContactJid));
        notify.data.insert(NDR_SOUND_FILE, SDF_PROXIMITY_EVENT);
        if (FNotifies[AStreamJid].contains(AContactJid))
            FNotifications->removeNotification(FNotifies[AStreamJid].value(AContactJid));
        FNotifies[AStreamJid].insert(AContactJid, FNotifications->appendNotification(notify));
    }
}

void ContactProximityNotification::checkContactProximity(const Jid &AStreamJid, const Jid &AContactJid, const MercatorCoordinates &ACoordinates, const GeolocElement &ACurrentPosition)
{
    // Check, if contact should be ignored in the first place
    if (Options::node(OPV_CONTACTPROXIMITYNOTIFICATIONS_IGNOREOWN).value().toBool() && (AStreamJid.bare() == AContactJid.bare()))
        return;

    IPresence *presence = FPresenceManager ? FPresenceManager->findPresence(AStreamJid) : NULL;
    if (presence)
    {
        IPresenceItem item =presence->findItem(AContactJid);
        if (item.show != IPresence::Offline && item.show != IPresence::Error) // Check, if contact is online
        {
			MercatorCoordinates coordinates(ACurrentPosition.coordinates());
            int distance = Options::node(OPV_CONTACTPROXIMITYNOTIFICATIONS_DISTANCE).value().toInt();
            int treshold = Options::node(OPV_CONTACTPROXIMITYNOTIFICATIONS_TRESHOLD).value().toInt();

            // Check, if contact is within range
            if (ACoordinates.distance(coordinates)<(distance-treshold))
                if (!FNotifiedContacts.contains(AContactJid))
                {
                    FNotifiedContacts.append(AContactJid);
                    displayNotification(AStreamJid, AContactJid);
                }

            // Check, if contact is out of range
            if (ACoordinates.distance(coordinates)>(distance+treshold))
                if (FNotifiedContacts.contains(AContactJid))
                    FNotifiedContacts.removeAll(AContactJid);
        }
    }
}

void ContactProximityNotification::checkContactsProximity(const GeolocElement &ACurrentPosition)
{
    for (QHash<Jid, QPair<Jid, MercatorCoordinates> >::ConstIterator it=FContactCoordinates.constBegin(); it!=FContactCoordinates.constEnd(); it++)
    {
        QPair<Jid, MercatorCoordinates> pair=*it;
		checkContactProximity(pair.first, it.key(), pair.second, ACurrentPosition);
    }
}

void ContactProximityNotification::removeProximityNotification(const Jid &AStreamJid, const Jid &AContactJid)
{
    if (FNotifies.contains(AStreamJid) && FNotifies[AStreamJid].contains(AContactJid))
        FNotifications->removeNotification(FNotifies[AStreamJid][AContactJid]);
}

IPresenceItem ContactProximityNotification::presenceItemForBareJid(const Jid &AStreamJid, const Jid &AContactJid) const
{
    IPresence *presence = FPresenceManager ? FPresenceManager->findPresence(AStreamJid) : NULL;
    return presence ? FPresenceManager->sortPresenceItems(presence->findItems(AContactJid)).value(0) : IPresenceItem();
}

void ContactProximityNotification::onLocationReceived(const Jid &AStreamJid, const Jid &AContactJid, const MercatorCoordinates &ACoordinates, bool AReliabilityChanged)
{
	Q_UNUSED(AReliabilityChanged)

    if (AStreamJid!=AContactJid)
    {
        QPair<Jid, MercatorCoordinates> pair(AStreamJid, ACoordinates);
        FContactCoordinates.insert(AContactJid, pair);
		GeolocElement position = FPositioning->currentPosition();
        if (position.isValid())
            checkContactProximity(AStreamJid, AContactJid, ACoordinates, position);
    }
}

void ContactProximityNotification::onLocationRemoved(const Jid &AStreamJid, const Jid &AContactJid)
{
	Q_UNUSED(AStreamJid)

    if (FContactCoordinates.contains(AContactJid))
        FContactCoordinates.remove(AContactJid);
    if (FNotifiedContacts.contains(AContactJid))
        FNotifiedContacts.removeAll(AContactJid);
}

void ContactProximityNotification::onNewPositionAvailable(const GeolocElement &APosition)
{
    if (APosition.isValid())
        checkContactsProximity(APosition);
}

void ContactProximityNotification::onNotificationActivated(int ANotifyId)
{
    for (QHash<Jid, QHash<Jid, int> >::const_iterator its=FNotifies.constBegin(); its!=FNotifies.constEnd(); its++)
        for (QHash<Jid, int>::const_iterator itc=(*its).constBegin(); itc!=(*its).constEnd(); itc++)
            if (itc.value()==ANotifyId) // Notification found! Activate window!
            {
                Jid contactJid = itc.key();
                Jid streamJid  = its.key();
                if (FMapContacts)
                {
                    FMapContacts->showContact(contactJid.full());
                    removeProximityNotification(streamJid, contactJid);
                }
                else
                {
                    IPresenceItem pitem = presenceItemForBareJid(its.key(), itc.key());
                    if (!pitem.itemJid.isEmpty())
                        contactJid=pitem.itemJid;

                    IMessageChatWindow *window=FMessageWidgets->findChatWindow(its.key(), contactJid);
                    if (!window)
                    {
						FMessageProcessor->getMessageWindow(its.key(), contactJid, Message::Chat, IMessageProcessor::ActionAssign);
                        window = FMessageWidgets->findChatWindow(its.key(), contactJid);
                    }
                    if (window)
                        window->showTabPage();
                }
                return;
            }
}

void ContactProximityNotification::onNotificationRemoved(int ANotifyId)
{
    for (QHash<Jid, QHash<Jid, int> >::iterator its=FNotifies.begin(); its!=FNotifies.end(); its++)
        for (QHash<Jid, int>::iterator itc=(*its).begin(); itc!=(*its).end(); itc++)
            if (*itc == ANotifyId)
            {
                (*its).remove(itc.key());
                if ((*its).isEmpty())
                    FNotifies.remove(its.key());
                return;
            }
}

void ContactProximityNotification::onChatWindowCreated(IMessageChatWindow *AWindow)
{
    connect(AWindow->instance(), SIGNAL(tabPageActivated()), SLOT(onWindowActivated()));
}

void ContactProximityNotification::onWindowActivated()
{
    IMessageChatWindow *window = qobject_cast<IMessageChatWindow *>(sender());
    if (window)
        removeProximityNotification(window->streamJid(), window->contactJid());
}

void ContactProximityNotification::onContactShowedOnTheMap(const QString &AId)
{
    for (QHash<Jid, QHash<Jid, int> >::iterator its=FNotifies.begin(); its!=FNotifies.end(); its++)
        for (QHash<Jid, int>::iterator itc=(*its).begin(); itc!=(*its).end(); itc++)
            if (itc.key()== AId)
            {
                FNotifications->removeNotification(*itc);
                return;
            }
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_contactproximitynotification, ContactProximityNotification)
#endif
