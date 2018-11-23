#include "chatmarkers.h"
#include "definitions/messageeditororders.h"
#include "definitions/messagewriterorders.h"
#include "definitions/messagedataroles.h"
#include "definitions/archivehandlerorders.h"
#include "definitions/optionvalues.h"
#include "definitions/optionwidgetorders.h"
#include "definitions/optionnodes.h"
#include "definitions/notificationdataroles.h"
#include "definitions/notificationtypeorders.h"
#include "definitions/notificationtypes.h"
#include "definitions/soundfiles.h"
#include "definitions/tabpagenotifypriorities.h"

#include <QDateTime>
#include <QFile>

ChatMarkers::ChatMarkers():
        FMessageProcessor(NULL),
        FMessageArchiver(NULL),
        FDiscovery(NULL),        
        FUrlProcessor(NULL),
        FOptionsManager(NULL),
        FNotifications(NULL),
        FMessageWidgets(NULL),
        FMultiChatManager(NULL),
        FIconStorage(NULL)
{}

ChatMarkers::~ChatMarkers()
{}

void ChatMarkers::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Chat Markers");
	APluginInfo->description = tr("Marking the last received, displayed and acknowledged message in a chat.");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";    
    APluginInfo->dependences.append(MESSAGEPROCESSOR_UUID);
}

bool ChatMarkers::initConnections(IPluginManager *APluginManager, int & /*AInitOrder*/)
{    
    IPlugin *plugin = APluginManager->pluginInterface("IMessageProcessor").value(0);
    if (plugin)
        FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());
    else
        return false;

    plugin = APluginManager->pluginInterface("IMessageArchiver").value(0);
    if (plugin)
        FMessageArchiver = qobject_cast<IMessageArchiver *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IUrlProcessor").value(0,NULL);
    if (plugin)
        FUrlProcessor = qobject_cast<IUrlProcessor *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
    if (plugin)
        FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("INotifications").value(0,NULL);
    if (plugin)
    {
        FNotifications = qobject_cast<INotifications *>(plugin->instance());
        if (FNotifications)
            connect(FNotifications->instance(), SIGNAL(notificationActivated(int)), SLOT(onNotificationActivated(int)));
    }

    plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
    if (plugin)
    {
        FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
        if (FMessageWidgets)
            connect(FMessageWidgets->instance(),SIGNAL(chatWindowCreated(IMessageChatWindow *)),SLOT(onChatWindowCreated(IMessageChatWindow *)));
    }

    plugin = APluginManager->pluginInterface("IMultiUserChatManager").value(0,NULL);
    if (plugin)
    {
        FMultiChatManager = qobject_cast<IMultiUserChatManager *>(plugin->instance());
        if (FMultiChatManager)
            connect(FMultiChatManager->instance(),SIGNAL(multiChatWindowCreated(IMultiUserChatWindow *)), SLOT(onMultiChatWindowCreated(IMultiUserChatWindow *)));
    }

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

    return true;
}

bool ChatMarkers::initSettings()
{
    Options::setDefaultValue(OPV_CHATMARKERS_SHOW, true);
    Options::setDefaultValue(OPV_CHATMARKERS_SEND, true);
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> ChatMarkers::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_MESSAGES && Options::node(OPV_COMMON_ADVANCED).value().toBool())
    {
		widgets.insertMulti(OWO_MESSAGES_CHATMARKERS_SHOW, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_CHATMARKERS_SHOW),tr("Show Chat Marker notifications"), AParent));
		widgets.insertMulti(OWO_MESSAGES_CHATMARKERS_SEND, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_CHATMARKERS_SEND),tr("Send Chat Marker notifications"), AParent));
    }
    return widgets;
}

bool ChatMarkers::initObjects()
{
	FIconStorage = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
    if (FIconStorage)
    {
        QString fileName=FIconStorage->fileFullName(MNI_CHATMARKERS);
        if (!fileName.isEmpty())
        {
            QFile file(fileName);
            if (file.open(QFile::ReadOnly))
            {
                FImgeData=file.readAll();
                file.close();
            }
        }
    }

    if (FNotifications)
    {
        INotificationType recievedType;
        recievedType.order = NTO_DELIVERED_NOTIFY;
        if (FIconStorage)
            recievedType.icon = FIconStorage->getIcon(MNI_DELIVERED);
        recievedType.title = tr("When message delivery notification recieved");
        recievedType.kindMask = INotification::PopupWindow|INotification::SoundPlay;
        recievedType.kindDefs = recievedType.kindMask;
        FNotifications->registerNotificationType(NNT_DELIVERED, recievedType);

        INotificationType displayedType;
        displayedType.order = NTO_CHATMARKERS_NOTIFY;
        if (FIconStorage)
            displayedType.icon = FIconStorage->getIcon(MNI_CHATMARKERS);
        displayedType.title = tr("When message marked with a displayed Chat Marker");
        displayedType.kindMask = INotification::PopupWindow|INotification::SoundPlay;
        displayedType.kindDefs = displayedType.kindMask;
        FNotifications->registerNotificationType(NNT_CHATMARKERS, displayedType);
    }

    if (FUrlProcessor)
        FUrlProcessor->registerUrlHandler("chatmarkers", this);

    if (FMessageProcessor)
    {
        FMessageProcessor->insertMessageEditor(MEO_CHATMARKERS, this);
        FMessageProcessor->insertMessageWriter(MWO_CHATMARKERS, this);
    }

    if (FMessageArchiver)
        FMessageArchiver->insertArchiveHandler(AHO_DEFAULT, this);

	if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);

    return true;
}

void ChatMarkers::registerDiscoFeatures(bool ARegister)
{
	if (ARegister)
	{
		IDiscoFeature dfeature;
		dfeature.active = true;
		dfeature.var = NS_CHATMARKERS;
		dfeature.icon = FIconStorage->getIcon(MNI_CHATMARKERS);
		dfeature.name = tr("Chat Markers");
		dfeature.description = tr("Supports marking the last received, displayed and acknowledged message in a chat");
		FDiscovery->insertDiscoFeature(dfeature);
	}
	else
		FDiscovery->removeDiscoFeature(NS_CHATMARKERS);
}

void ChatMarkers::onChatWindowCreated(IMessageChatWindow *AWindow)
{
    connect(AWindow->instance(),SIGNAL(tabPageActivated()),SLOT(onChatWindowActivated()));
}

void ChatMarkers::onMultiChatWindowCreated(IMultiUserChatWindow *AWindow)
{
    connect(AWindow->instance(),SIGNAL(tabPageActivated()),SLOT(onWindowActivated()));
}

void ChatMarkers::onWindowActivated()
{
    IMessageChatWindow *window = qobject_cast<IMessageChatWindow *>(sender());
    if (window)
    {
        QMultiMap<Jid, Jid> addresses = window->address()->availAddresses();
        removeNotifiedMessages(window);

        for (QMultiMap<Jid, Jid>::ConstIterator it = addresses.constBegin();
             it != addresses.constEnd(); ++it)
            if (isMarked(it.key(), *it))
            {
                QString AId = FMarkedHash[it.key()][*it];
                markDisplayed(it.key(), *it, AId);
            }
    }
}

void ChatMarkers::onNotificationActivated(int ANotifyId)
{
    IMessageChatWindow *window=FNotifies.key(ANotifyId);
    if (window)
		window->showTabPage();
}

void ChatMarkers::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_CHATMARKERS_SEND));
}

void ChatMarkers::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path()==OPV_CHATMARKERS_SEND)
		registerDiscoFeatures(ANode.value().toBool() ||
							  Options::node(OPV_CHATMARKERS_SHOW).value().toBool());
	else if (ANode.path()==OPV_CHATMARKERS_SHOW)
		registerDiscoFeatures(ANode.value().toBool() ||
							  Options::node(OPV_CHATMARKERS_SEND).value().toBool());
}

bool ChatMarkers::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
    return FDiscovery && FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_CHATMARKERS);
}

void ChatMarkers::removeNotifiedMessages(IMessageChatWindow *AWindow)
{
    if (FNotifies.contains(AWindow))
    {
        for(QHash<IMessageChatWindow *, int>::const_iterator it=FNotifies.constBegin(); it!=FNotifies.constEnd(); it++)
            FNotifications->removeNotification(*it);
        FNotifies.remove(AWindow);
    }
}

bool ChatMarkers::messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection)
{
	Q_UNUSED(AOrder)

    Stanza stanza=AMessage.stanza();
    if (ADirection==IMessageProcessor::DirectionIn)
    {
        if (Options::node(OPV_CHATMARKERS_SEND).value().toBool() &&
                isSupported(AStreamJid, AMessage.from()) &&
                !stanza.firstElement("markable", NS_CHATMARKERS).isNull() &&
                !AMessage.body().isNull() &&
                !AMessage.isDelayed())
        {
            Stanza message("message");
            QString id=AMessage.id();
            message.setTo(AMessage.from()).setUniqueId();
            message.addElement("received", NS_CHATMARKERS).setAttribute("id", id);
            Message msg(message);
            FMessageProcessor->sendMessage(AStreamJid, msg, IMessageProcessor::DirectionOut);
            IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AMessage.from());
            if (window && window->isActiveTabPage())
                markDisplayed(AStreamJid, AMessage.from(), AMessage.id());
            else
                FMarkedHash[AStreamJid].insert(AMessage.from(), AMessage.id());
        }
        else
        {
            QDomElement received=stanza.firstElement("received", NS_CHATMARKERS);
            if (!received.isNull())
            {
                QString id=received.attribute("id");
                if (id.isEmpty())
                    id=AMessage.id();
                setReceived(AStreamJid, AMessage.from(), id);
            }
            QDomElement displayed=stanza.firstElement("displayed", NS_CHATMARKERS);
            if(!displayed.isNull())
            {
                QString id=displayed.attribute("id");
                if (id.isEmpty())
                    id=AMessage.id(); //-- Obsolete revision of XEP-0184 ---
                setDisplayed(AStreamJid, AMessage.from(), id);
            }
        }
    }
    else
    {
        if (Options::node(OPV_CHATMARKERS_SHOW).value().toBool() &&
            isSupported(AStreamJid, AMessage.to()) &&
            AMessage.stanza().firstElement("markable", NS_CHATMARKERS).isNull() &&
            !AMessage.body().isNull())
        {
            if(AMessage.id().isEmpty())
            {
                uint uTime = QDateTime().currentDateTime().toTime_t();
                AMessage.setId(QString().setNum(uTime,16));
            }
            AMessage.detach();
            AMessage.stanza().addElement("markable", NS_CHATMARKERS);
            FMarkableHash[AStreamJid][AMessage.to()].append(AMessage.id());
        }
    }
	return false;
}

bool ChatMarkers::writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang)
{
	Q_UNUSED(AOrder) Q_UNUSED(ALang)
	return AMessage.data(MDR_MESSAGE_DIRECTION).toInt() == IMessageProcessor::DirectionOut &&
			Options::node(OPV_CHATMARKERS_SHOW).value().toBool() &&
			!AMessage.stanza().firstElement("markable", NS_CHATMARKERS).isNull();
}

bool ChatMarkers::writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{    
	Q_UNUSED(AOrder)
	Q_UNUSED(ALang)

    if (AMessage.data(MDR_MESSAGE_DIRECTION).toInt() == IMessageProcessor::DirectionOut &&
        Options::node(OPV_CHATMARKERS_SHOW).value().toBool() &&
       !AMessage.stanza().firstElement("markable", NS_CHATMARKERS).isNull())
    {
        if (AMessage.stanza().firstElement("request", NS_RECEIPTS).isNull())
        {
            QUrl url(QString("received:%1/%2/%3").arg(AMessage.from())
                                                 .arg(AMessage.to())
                                                 .arg(AMessage.id()));
            QTextCursor cursor(ADocument);
            cursor.movePosition(QTextCursor::End);
            cursor.insertImage(url.toString());
        }
        QUrl url(QString("chatmarkers:%1/%2/%3").arg(AMessage.from())
												.arg(AMessage.to())
												.arg(AMessage.id()));
        QTextCursor cursor(ADocument);
        cursor.movePosition(QTextCursor::End);
        cursor.insertImage(url.toString());
		return true;
    }
	return false;
}

bool ChatMarkers::writeTextToMessage(int AOrder, QTextDocument *ADocument, Message &AMessage, const QString &ALang)
{
	Q_UNUSED(AOrder) Q_UNUSED(AMessage) Q_UNUSED(ADocument) Q_UNUSED(ALang)
	return false;
}  // Nothing to do right now

//  Clients SHOULD use Message Archiving (XEP-0136) [7] or Message Archive Management (XEP-0313) [8]
//  to support offline updating of Chat Markers. Chat Markers SHOULD be archived, so they can be fetched
//  and set regardless of whether the other users in a chat are online.
bool ChatMarkers::archiveMessageEdit(int AOrder, const Jid &AStreamJid, Message &AMessage, bool ADirectionIn)
{
    Q_UNUSED(AOrder)
    Q_UNUSED(AStreamJid)
    Q_UNUSED(ADirectionIn)

//    if (!AMessage.stanza().firstElement("received", NS_CHATMARKERS).isNull())
//        return true;
//    if (!AMessage.stanza().firstElement("displayed", NS_CHATMARKERS).isNull())
//        return true;
//    if (!AMessage.Stanza().firstElement("acknowledged", NS_CHATMARKERS).isNull())
//        return true;
//    if (!AMessage.stanza().firstElement("markable", NS_CHATMARKERS).isNull())
//    {
//        AMessage.detach();
//        AMessage.stanza().element().removeChild(AMessage.stanza().firstElement("markable", NS_CHATMARKERS));
//    }
	return false;
}

QNetworkReply *ChatMarkers::request(QNetworkAccessManager::Operation op, const QNetworkRequest &ARequest, QIODevice *AOutgoingData)
{
    DelayedImageNetworkReply *reply = new DelayedImageNetworkReply(op, ARequest, AOutgoingData, &FImgeData, FUrlProcessor->instance());

    if (ARequest.url().scheme().contains("received"))
            connect(this, SIGNAL(received(QString)), reply, SLOT(onReady(QString)), Qt::QueuedConnection);
    if (ARequest.url().scheme().contains("chatmarkers"))
        connect(this, SIGNAL(displayed(QString)), reply, SLOT(onReady(QString)), Qt::QueuedConnection);

    if (isReceived(ARequest.url().path()))
        emit received(ARequest.url().path());
    if(isDisplayed(ARequest.url().path()))
        emit displayed(ARequest.url().path());
    return reply;
}

void ChatMarkers::setReceived(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId)
{
    QString id = AStreamJid.full()+"/"+AContactJid.full()+"/"+AMessageId;
    FReceivedHash.insert(id);
    if (FMessageWidgets)
    {
        IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
        if (window && !window->isActiveTabPage())
        {
            INotification notify;
            notify.kinds = FNotifications->enabledTypeNotificationKinds(NNT_DELIVERED);
            if (notify.kinds & (INotification::PopupWindow|INotification::SoundPlay))
            {
                notify.typeId = NNT_DELIVERED;
                notify.data.insert(NDR_ICON,FIconStorage->getIcon(MNI_DELIVERED));
                notify.data.insert(NDR_POPUP_CAPTION, tr("Message delivered"));
                notify.data.insert(NDR_POPUP_TITLE, FNotifications->contactName(AStreamJid, AContactJid));
//                notify.data.insert(NDR_POPUP_IMAGE, FNotifications->contactAvatar(AContactJid));

                notify.data.insert(NDR_SOUND_FILE, SDF_RECEIPTS_DELIVERED);
                FNotifies.insertMulti(window, FNotifications->appendNotification(notify));
                connect(window->instance(), SIGNAL(tabPageActivated()), SLOT(onWindowActivated()));
            }
        }
    }

    emit received(id);
}

void ChatMarkers::setDisplayed(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId)
{
    QString id = AStreamJid.full()+"/"+AContactJid.full()+"/"+AMessageId;
    FDisplayedHash.insert(id);
    if (FMessageWidgets)
    {
        IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
        if (window && !window->isActiveTabPage())
        {
            INotification notify;
            notify.kinds = FNotifications->enabledTypeNotificationKinds(NNT_CHATMARKERS);
            if (notify.kinds & (INotification::PopupWindow|INotification::SoundPlay))
            {
                notify.typeId = NNT_CHATMARKERS;
                notify.data.insert(NDR_ICON,FIconStorage->getIcon(MNI_CHATMARKERS));
                notify.data.insert(NDR_POPUP_CAPTION, tr("Message displayed"));
                notify.data.insert(NDR_POPUP_TITLE, FNotifications->contactName(AStreamJid, AContactJid));
//                notify.data.insert(NDR_POPUP_IMAGE, FNotifications->contactAvatar(AContactJid));

                notify.data.insert(NDR_SOUND_FILE, SDF_CHATMARKERS_MARKED);
                FNotifies.insertMulti(window, FNotifications->appendNotification(notify));
                connect(window->instance(), SIGNAL(tabPageActivated()), SLOT(onWindowActivated()));
            }
        }
    }

    emit displayed(id);
}

void ChatMarkers::markDisplayed(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId)
{
        Stanza message("message");
        message.setTo(AContactJid.bare()).setUniqueId();
        message.addElement("displayed", NS_CHATMARKERS).setAttribute("id", AMessageId);
        Message msg(message);
        FMessageProcessor->sendMessage(AStreamJid, msg, IMessageProcessor::DirectionOut);
        if (isMarked(AStreamJid, AContactJid))
            FMarkedHash[AStreamJid].remove(AContactJid);
}

bool ChatMarkers::isMarkable(const Jid &AStreamJid, const Jid &AContactJid) const
{
    return FMarkableHash[AStreamJid].contains(AContactJid);
}

bool ChatMarkers::isMarked(const Jid &AStreamJid, const Jid &AContactJid) const
{
    return FMarkedHash[AStreamJid].contains(AContactJid);
}

bool ChatMarkers::isReceived(const QString &AId) const
{
    return FReceivedHash.contains(AId);
}

bool ChatMarkers::isDisplayed(const QString &AId) const
{
    return FDisplayedHash.contains(AId);
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_chatmarkers, ChatMarkers)
#endif
