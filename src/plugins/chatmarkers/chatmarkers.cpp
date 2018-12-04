#include "chatmarkers.h"
#include "definitions/toolbargroups.h"
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
        {
            connect(FMessageWidgets->instance(),SIGNAL(chatWindowCreated(IMessageChatWindow *)),SLOT(onChatWindowCreated(IMessageChatWindow *)));
            connect(FMessageWidgets->instance(), SIGNAL(toolBarWidgetCreated(IMessageToolBarWidget *)), SLOT(onToolBarWidgetCreated(IMessageToolBarWidget *)));
        }
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
		QStringList names;
		names << MNI_MESSAGE_RECEIVED
			  << MNI_MESSAGE_DISPLAYED
			  << MNI_MESSAGE_ACKNOWLEDGED;

		int i = 0;
		for (QStringList::ConstIterator it = names.constBegin();
			 it != names.constEnd(); ++it, ++i)
		{
			QString fileName=FIconStorage->fileFullName(*it);
			if (!fileName.isEmpty())
			{
				QFile file(fileName);
				if (file.open(QFile::ReadOnly))
				{
					FImageData[i] = file.readAll();
					file.close();
				}
			}
		}
    }

    if (FNotifications)
    {
        INotificationType recievedType;
        recievedType.order = NTO_DELIVERED_NOTIFY;
        if (FIconStorage)
			recievedType.icon = FIconStorage->getIcon(MNI_MESSAGE_RECEIVED);
        recievedType.title = tr("When message delivery notification recieved");
        recievedType.kindMask = INotification::PopupWindow|INotification::SoundPlay;
        recievedType.kindDefs = recievedType.kindMask;
        FNotifications->registerNotificationType(NNT_DELIVERED, recievedType);

        INotificationType displayedType;
        displayedType.order = NTO_DISPLAYED_NOTIFY;
        if (FIconStorage)
			displayedType.icon = FIconStorage->getIcon(MNI_MESSAGE_DISPLAYED);
        displayedType.title = tr("When message marked with a displayed Chat Marker");
        displayedType.kindMask = INotification::PopupWindow|INotification::SoundPlay;
        displayedType.kindDefs = displayedType.kindMask;
        FNotifications->registerNotificationType(NNT_DISPLAYED, displayedType);

        INotificationType acknowledgedType;
        acknowledgedType.order = NTO_ACKNOWLEDGED_NOTIFY;
        if (FIconStorage)
            acknowledgedType.icon = FIconStorage->getIcon(MNI_MESSAGE_ACKNOWLEDGED);
        acknowledgedType.title = tr("When message marked with a acknowledged Chat Marker");
        acknowledgedType.kindMask = INotification::PopupWindow|INotification::SoundPlay;
        acknowledgedType.kindDefs = acknowledgedType.kindMask;
        FNotifications->registerNotificationType(NNT_ACKNOWLEDGED, acknowledgedType);
    }

    if (FUrlProcessor)
    {
        FUrlProcessor->registerUrlHandler("chatmarkers-received", this);
        FUrlProcessor->registerUrlHandler("chatmarkers-displayed", this);
        FUrlProcessor->registerUrlHandler("chatmarkers-acknowledged", this);
    }

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
		dfeature.icon = FIconStorage->getIcon(MNI_MESSAGE_ACKNOWLEDGED);
		dfeature.name = tr("Chat Markers");
		dfeature.description = tr("Supports marking the last received, displayed and acknowledged message in a chat");
		FDiscovery->insertDiscoFeature(dfeature);
	}
	else
		FDiscovery->removeDiscoFeature(NS_CHATMARKERS);
}

void ChatMarkers::onChatWindowCreated(IMessageChatWindow *AWindow)
{
    connect(AWindow->instance(),SIGNAL(tabPageActivated()),SLOT(onWindowActivated()));
}

void ChatMarkers::onMultiChatWindowCreated(IMultiUserChatWindow *AWindow)
{
    connect(AWindow->instance(),SIGNAL(tabPageActivated()),SLOT(onWindowActivated()));
}

void ChatMarkers::onToolBarWidgetCreated(IMessageToolBarWidget *AWidget)
{
    IMessageChatWindow *chatWindow = qobject_cast<IMessageChatWindow *>(AWidget->messageWindow()->instance());
    if (chatWindow)
        connect(this,SIGNAL(markable(const Jid &,const Jid &)),SLOT(onMarkable(const Jid &,const Jid &)));
    connect(AWidget->instance(),SIGNAL(destroyed(QObject *)),SLOT(onToolBarWidgetDestroyed(QObject *)));

    updateToolBarAction(AWidget);
}

void ChatMarkers::onMarkable(const Jid &AStreamJid, const Jid &AContactJid)
{
    IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);

    foreach(IMessageToolBarWidget *widget, FToolBarActions.keys())
            if (widget->messageWindow()->instance() == window->instance())
            updateToolBarAction(widget);
}

void ChatMarkers::onToolBarWidgetDestroyed(QObject *AObject)
{
    foreach(IMessageToolBarWidget *widget, FToolBarActions.keys())
        if (qobject_cast<QObject *>(widget->instance()) == AObject)
            FToolBarActions.remove(widget);
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
            if (isLastMarkableDisplay(it.key(), *it))
            {
                QString AId = FLastMarkableDisplayHash[it.key()][*it];
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

void ChatMarkers::onAcknowledgedByAction(bool)
{
    Action *action = qobject_cast<Action *>(sender());
    if (action!=NULL)
    {
        IMessageToolBarWidget *widget = FToolBarActions.key(action);
        IMessageChatWindow *chatWindow = qobject_cast<IMessageChatWindow *>(widget->messageWindow()->instance());

        QMultiMap<Jid, Jid> addresses = chatWindow->address()->availAddresses();

        for (QMultiMap<Jid, Jid>::ConstIterator it = addresses.constBegin();
             it != addresses.constEnd(); ++it)
            if (isLastMarkableAcknowledge(it.key(), *it))
                markAcknowledged(it.key(), *it);

        foreach(IMessageToolBarWidget *widget, FToolBarActions.keys())
            if (widget->messageWindow()->instance() == chatWindow->instance())
                updateToolBarAction(widget);
    }
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

void ChatMarkers::updateToolBarAction(IMessageToolBarWidget *AWidget)
{
    Action *acknowledgedAction = FToolBarActions.value(AWidget);
    IMessageChatWindow *chatWindow = qobject_cast<IMessageChatWindow *>(AWidget->messageWindow()->instance());
    if (chatWindow != NULL)
    {
        if (acknowledgedAction == NULL)
        {
            acknowledgedAction = new Action(AWidget->toolBarChanger()->toolBar());
			acknowledgedAction->setIcon(RSR_STORAGE_MENUICONS, MNI_MESSAGE_ACKNOWLEDGED);
			acknowledgedAction->setText(tr("Acknowledge"));
            //acknowledgedAction->setShortcutId();
            connect(acknowledgedAction,SIGNAL(triggered(bool)),SLOT(onAcknowledgedByAction(bool)));
            AWidget->toolBarChanger()->insertAction(acknowledgedAction,TBG_MWTBW_ACKNOWLEDGEMENT);
            FToolBarActions.insert(AWidget,acknowledgedAction);
        }

        QMultiMap<Jid, Jid> addresses = chatWindow->address()->availAddresses();
        bool mrkd = false;

        for (QMultiMap<Jid, Jid>::ConstIterator it = addresses.constBegin();
             it != addresses.constEnd(); ++it)
            if (isLastMarkableAcknowledge(it.key(), *it))
            {
                mrkd = true;
                break;
            }

        acknowledgedAction->setEnabled(mrkd);
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
            bool isMarkedBefore = isLastMarkableAcknowledge(AStreamJid, AMessage.from());
            if (window && window->isActiveTabPage())
                markDisplayed(AStreamJid, AMessage.from(), AMessage.id());
            else
                FLastMarkableDisplayHash[AStreamJid].insert(AMessage.from(), AMessage.id());
            FLastMarkableAcknowledgeHash[AStreamJid].insert(AMessage.from(), AMessage.id());
            if (!isMarkedBefore)
                emit markable(AStreamJid, AMessage.from());
        }
        else
        {
            QDomElement received=stanza.firstElement("received", NS_CHATMARKERS);
            if (!received.isNull())
				setReceived(AStreamJid, AMessage.from(), received.attribute("id"));
            QDomElement displayed=stanza.firstElement("displayed", NS_CHATMARKERS);
            if(!displayed.isNull())
				setDisplayed(AStreamJid, AMessage.from(), displayed.attribute("id"));
            QDomElement acknowledged=stanza.firstElement("acknowledged", NS_CHATMARKERS);
            if (!acknowledged.isNull())
			   setAcknowledged(AStreamJid, AMessage.from(), acknowledged.attribute("id"));
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
            FReceivedRequestHash[AStreamJid][AMessage.to()].append(AMessage.id());
            FDisplayedRequestHash[AStreamJid][AMessage.to()].append(AMessage.id());
            FAcknowledgedRequestHash[AStreamJid][AMessage.to()].append(AMessage.id());
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
		QUrl url(QString("scheme:{%1}{%2}{%3}")
				  .arg(AMessage.from().toLower())
				  .arg(AMessage.to().toLower())
				  .arg(AMessage.id()));

		QTextCursor cursor(ADocument);
		cursor.movePosition(QTextCursor::End);

		QTextImageFormat image;
        if (AMessage.stanza().firstElement("request", NS_RECEIPTS).isNull())
		{
			url.setScheme("chatmarkers-received");
			image.setName(url.toString());
			image.setToolTip(tr("Received"));
			cursor.insertImage(image);
        }
		url.setScheme("chatmarkers-displayed");
		image.setName(url.toString());
		image.setToolTip(tr("Displayed"));
		cursor.insertImage(image);

		url.setScheme("chatmarkers-acknowledged");
		image.setName(url.toString());
		image.setToolTip(tr("Acknowledged"));
		cursor.insertImage(image);

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
	Type type = ARequest.url().scheme() == "chatmarkers-received"	  ?	Received:
				ARequest.url().scheme() == "chatmarkers-displayed"	  ?	Displayed:
				ARequest.url().scheme() == "chatmarkers-acknowledged" ?	Acknowledged:
																		Unknown;

	Q_ASSERT(type != Unknown);
	if (type == Unknown)
		return nullptr;

	DelayedImageNetworkReply *reply = new DelayedImageNetworkReply(op, ARequest, AOutgoingData, FImageData[type-1], FUrlProcessor->instance());

    if (type == Received)
		connect(this, SIGNAL(received(QString)), reply, SLOT(onReady(QString)), Qt::QueuedConnection);
    if (type == Displayed)
        connect(this, SIGNAL(displayed(QString)), reply, SLOT(onReady(QString)), Qt::QueuedConnection);
    if (type == Acknowledged)
        connect(this, SIGNAL(acknowledged(QString)), reply, SLOT(onReady(QString)), Qt::QueuedConnection);

    if (isReceived(ARequest.url().path()))
        emit received(ARequest.url().path());
    if(isDisplayed(ARequest.url().path()))
        emit displayed(ARequest.url().path());
    if (isAcknowledged(ARequest.url().path()))
        emit acknowledged(ARequest.url().path());
    return reply;
}

void ChatMarkers::setReceived(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId)
{
    if (FDisplayedRequestHash.contains(AStreamJid) &&
            FDisplayedRequestHash[AStreamJid].contains(AContactJid) &&
            FDisplayedRequestHash[AStreamJid][AContactJid].contains(AMessageId))
    {
        int IdsNum = 0;
        QStringList Ids = FReceivedRequestHash[AStreamJid][AContactJid];
        for (int i=0; i<=Ids.indexOf(AMessageId); ++i)
        {
            QString id = QString("{%1}{%2}{%3}").arg(AStreamJid.full().toLower())
                                                .arg(AContactJid.full().toLower())
                                                .arg(Ids[i]);
            IdsNum++;
            FReceivedHash.insert(id);
            emit received(id);
        }

        FReceivedRequestHash[AStreamJid][AContactJid] = Ids.mid(Ids.indexOf(AMessageId)+1);

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
                    notify.data.insert(NDR_ICON,FIconStorage->getIcon(MNI_MESSAGE_RECEIVED));
                    notify.data.insert(NDR_POPUP_CAPTION, tr("%n message(s) delivered", "", IdsNum));
                    notify.data.insert(NDR_POPUP_TITLE, FNotifications->contactName(AStreamJid, AContactJid));
    //                notify.data.insert(NDR_POPUP_IMAGE, FNotifications->contactAvatar(AContactJid));

                    notify.data.insert(NDR_SOUND_FILE, SDF_RECEIPTS_DELIVERED);
                    FNotifies.insertMulti(window, FNotifications->appendNotification(notify));
                    connect(window->instance(), SIGNAL(tabPageActivated()), SLOT(onWindowActivated()));
                }
            }
        }
    }
}

void ChatMarkers::setDisplayed(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId)
{
    if (FDisplayedRequestHash.contains(AStreamJid) &&
            FDisplayedRequestHash[AStreamJid].contains(AContactJid) &&
            FDisplayedRequestHash[AStreamJid][AContactJid].contains(AMessageId))
    {
        int IdsNum = 0;
        QStringList Ids = FDisplayedRequestHash[AStreamJid][AContactJid];
        for (int i=0; i<=Ids.indexOf(AMessageId); ++i)
        {
            QString id = QString("{%1}{%2}{%3}").arg(AStreamJid.full().toLower())
                                                .arg(AContactJid.full().toLower())
                                                .arg(Ids[i]);
            IdsNum++;
            FDisplayedHash.insert(id);
            emit displayed(id);
        }

        FDisplayedRequestHash[AStreamJid][AContactJid] = Ids.mid(Ids.indexOf(AMessageId)+1);

        if (FMessageWidgets)
        {
            IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
            if (window && !window->isActiveTabPage())
            {
                INotification notify;
                notify.kinds = FNotifications->enabledTypeNotificationKinds(NNT_DISPLAYED);
                if (notify.kinds & (INotification::PopupWindow|INotification::SoundPlay))
                {
                    notify.typeId = NNT_DISPLAYED;
                    notify.data.insert(NDR_ICON, FIconStorage->getIcon(MNI_MESSAGE_DISPLAYED));
                    notify.data.insert(NDR_POPUP_CAPTION, tr("%n message(s) displayed", "", IdsNum));
                    notify.data.insert(NDR_POPUP_TITLE, FNotifications->contactName(AStreamJid, AContactJid));
    //                notify.data.insert(NDR_POPUP_IMAGE, FNotifications->contactAvatar(AContactJid));

                    notify.data.insert(NDR_SOUND_FILE, SDF_CHATMARKERS_MARKED);
                    FNotifies.insertMulti(window, FNotifications->appendNotification(notify));
                    connect(window->instance(), SIGNAL(tabPageActivated()), SLOT(onWindowActivated()));
                }
            }
        }
    }
}

void ChatMarkers::setAcknowledged(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId)
{
    if (FAcknowledgedRequestHash.contains(AStreamJid) &&
            FAcknowledgedRequestHash[AStreamJid].contains(AContactJid) &&
            FAcknowledgedRequestHash[AStreamJid][AContactJid].contains(AMessageId))
    {
        int IdsNum = 0;
        QStringList Ids = FAcknowledgedRequestHash[AStreamJid][AContactJid];
        for (int i=0; i<=Ids.indexOf(AMessageId); ++i)
        {
            QString id = QString("{%1}{%2}{%3}").arg(AStreamJid.full().toLower())
                                                .arg(AContactJid.full().toLower())
                                                .arg(Ids[i]);
            IdsNum++;
            FAcknowledgedHash.insert(id);

            emit acknowledged(id);
        }

        FAcknowledgedRequestHash[AStreamJid][AContactJid] = Ids.mid(Ids.indexOf(AMessageId)+1);

        if (FMessageWidgets)
        {
            IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
            if (window && !window->isActiveTabPage())
            {
                INotification notify;
                notify.kinds = FNotifications->enabledTypeNotificationKinds(NNT_ACKNOWLEDGED);
                if (notify.kinds & (INotification::PopupWindow|INotification::SoundPlay))
                {
                    notify.typeId = NNT_ACKNOWLEDGED;
                    notify.data.insert(NDR_ICON, FIconStorage->getIcon(MNI_MESSAGE_ACKNOWLEDGED));
                    notify.data.insert(NDR_POPUP_CAPTION, tr("User acknowledged %n message(s) are read", "", IdsNum));
                    notify.data.insert(NDR_POPUP_TITLE, FNotifications->contactName(AStreamJid, AContactJid));
    //                notify.data.insert(NDR_POPUP_IMAGE, FNotifications->contactAvatar(AContactJid));

                    notify.data.insert(NDR_SOUND_FILE, SDF_CHATMARKERS_MARKED);
                    FNotifies.insertMulti(window, FNotifications->appendNotification(notify));
                    connect(window->instance(), SIGNAL(tabPageActivated()), SLOT(onWindowActivated()));
                }
            }
        }
    }
}

void ChatMarkers::markDisplayed(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId)
{
    Stanza message("message");
    message.setTo(AContactJid.bare()).setUniqueId();
    message.addElement("displayed", NS_CHATMARKERS).setAttribute("id", AMessageId);
    Message msg(message);
    FMessageProcessor->sendMessage(AStreamJid, msg, IMessageProcessor::DirectionOut);
    FLastMarkableDisplayHash[AStreamJid].remove(AContactJid);
}

void ChatMarkers::markAcknowledged(const Jid &AStreamJid, const Jid &AContactJid)
{
    LOG_STRM_INFO(AStreamJid,QString("ChatMarkers Acknowledged, contact=%1").arg(AContactJid.bare()));
    QString Id = FLastMarkableAcknowledgeHash[AStreamJid][AContactJid];
    Stanza message("message");
    message.setTo(AContactJid.bare()).setUniqueId();
    message.addElement("acknowledged", NS_CHATMARKERS).setAttribute("id", Id);
    Message msg(message);
    FMessageProcessor->sendMessage(AStreamJid, msg, IMessageProcessor::DirectionOut);
    FLastMarkableAcknowledgeHash[AStreamJid].remove(AContactJid);
}

bool ChatMarkers::isLastMarkableDisplay(const Jid &AStreamJid, const Jid &AContactJid) const
{
    return FLastMarkableDisplayHash[AStreamJid].contains(AContactJid);
}

bool ChatMarkers::isLastMarkableAcknowledge(const Jid &AStreamJid, const Jid &AContactJid) const
{
    return FLastMarkableAcknowledgeHash[AStreamJid].contains(AContactJid);
}

bool ChatMarkers::isReceived(const QString &AId) const
{
    return FReceivedHash.contains(AId);
}

bool ChatMarkers::isDisplayed(const QString &AId) const
{
    return FDisplayedHash.contains(AId);
}

bool ChatMarkers::isAcknowledged(const QString &AId) const
{
    return FAcknowledgedHash.contains(AId);
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_chatmarkers, ChatMarkers)
#endif
