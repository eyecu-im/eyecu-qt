#include "chatmarkers.h"
#include "chatmarkersoptions.h"

#include <definitions/toolbargroups.h>
#include <definitions/messageeditororders.h>
#include <definitions/messagewriterorders.h>
#include <definitions/messagedataroles.h>
#include <definitions/archivehandlerorders.h>
#include <definitions/optionvalues.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionnodes.h>
#include <definitions/notificationdataroles.h>
#include <definitions/notificationtypeorders.h>
#include <definitions/notificationtypes.h>
#include <definitions/soundfiles.h>
#include <definitions/tabpagenotifypriorities.h>

#include <utils/action.h>
#include <utils/logger.h>

#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QpXhtml>

ChatMarkers::ChatMarkers():
		FMessageProcessor(nullptr),
		FMessageArchiver(nullptr),
		FDiscovery(nullptr),
		FOptionsManager(nullptr),
		FNotifications(nullptr),
		FMessageWidgets(nullptr),
		FReceipts(nullptr),
		FIconStorage(nullptr)
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

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,nullptr);
	if (plugin)
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,nullptr);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IReceipts").value(0,nullptr);
	if (plugin)
	{
		FReceipts = qobject_cast<IReceipts *>(plugin->instance());
		connect(FReceipts->instance(), SIGNAL(messageDelivered(Jid,Jid,QString)),
									   SLOT(onMessageDelivered(Jid,Jid,QString)));
	}

	plugin = APluginManager->pluginInterface("INotifications").value(0,nullptr);
	if (plugin)
	{
		FNotifications = qobject_cast<INotifications *>(plugin->instance());
		if (FNotifications)
			connect(FNotifications->instance(), SIGNAL(notificationActivated(int)), SLOT(onNotificationActivated(int)));
	}

	plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,nullptr);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
		if (FMessageWidgets)
		{
			connect(FMessageWidgets->instance(), SIGNAL(chatWindowCreated(IMessageChatWindow *)),
												 SLOT(onChatWindowCreated(IMessageChatWindow *)));
			connect(FMessageWidgets->instance(), SIGNAL(toolBarWidgetCreated(IMessageToolBarWidget *)),
												 SLOT(onToolBarWidgetCreated(IMessageToolBarWidget *)));
		}
	}

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0,nullptr);
	if (plugin)
	{
		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());
		connect(FPresenceManager->instance(), SIGNAL(presenceOpened(IPresence*)),
											  SLOT(onPresenceOpened(IPresence*)));
		connect(FPresenceManager->instance(), SIGNAL(contactStateChanged(Jid,Jid,bool)),
											  SLOT(onContactStateChanged(Jid,Jid,bool)));
	}

	connect(Options::instance(),SIGNAL(optionsOpened()),
								SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsChanged(OptionsNode)),
								SLOT(onOptionsChanged(OptionsNode)));

	return true;
}

bool ChatMarkers::initSettings()
{
	Options::setDefaultValue(OPV_MARKERS_SHOW_LEVEL, 3);
	Options::setDefaultValue(OPV_MARKERS_SHOW_ACKOWN, true);
	Options::setDefaultValue(OPV_MARKERS_SEND_RECEIVED, true);
	Options::setDefaultValue(OPV_MARKERS_SEND_DISPLAYED, true);
	Options::setDefaultValue(OPV_MARKERS_SEND_ACK, true);
	return true;
}

QMultiMap<int, IOptionsDialogWidget *> ChatMarkers::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_MESSAGES && Options::node(OPV_COMMON_ADVANCED).value().toBool())
	{
		widgets.insertMulti(OHO_MESSAGES_MARKERS,
							FOptionsManager->newOptionsDialogHeader(tr("Chat markers"),
																	AParent));
		widgets.insertMulti(OWO_MESSAGES_MARKERS,
							new ChatMarkersOptions(AParent));
	}
	return widgets;
}

bool ChatMarkers::initObjects()
{
	FIconStorage = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);

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
		displayedType.title = tr("When message marked as displayed");
		displayedType.kindMask = INotification::PopupWindow|INotification::SoundPlay;
		displayedType.kindDefs = displayedType.kindMask;
		FNotifications->registerNotificationType(NNT_DISPLAYED, displayedType);

		INotificationType acknowledgedType;
		acknowledgedType.order = NTO_ACKNOWLEDGED_NOTIFY;
		if (FIconStorage)
			acknowledgedType.icon = FIconStorage->getIcon(MNI_MESSAGE_ACKNOWLEDGED);
		acknowledgedType.title = tr("When message marked as acknowledged");
		acknowledgedType.kindMask = INotification::PopupWindow|INotification::SoundPlay;
		acknowledgedType.kindDefs = acknowledgedType.kindMask;
		FNotifications->registerNotificationType(NNT_ACKNOWLEDGED, acknowledgedType);
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

static QString calcId(const Jid &AFrom, const Jid &ATo, const QString &AMessageId)
{
	return QString::fromLatin1(QCryptographicHash::hash(QString("%1|%2|%3").arg(AFrom.full())
																		   .arg(ATo.full())
																		   .arg(AMessageId)
																		   .toUtf8(),
														QCryptographicHash::Md4).toHex());
}

void ChatMarkers::setImage(IMessageChatWindow *AWindow, const Jid &AStreamJid,
						   const Jid &AContactJid, const QString &AMessageId,
						   const QString &AImage, const QString &ATitle)
{
	QString hash = calcId(AStreamJid, AContactJid, AMessageId);
	AWindow->viewWidget()->setImageUrl(hash, QUrl::fromLocalFile(FIconStorage->fileFullName(AImage)).toString());
	AWindow->viewWidget()->setObjectTitle(hash, ATitle);
}

void ChatMarkers::onChatWindowCreated(IMessageChatWindow *AWindow)
{
	connect(AWindow->instance(),SIGNAL(tabPageActivated()),SLOT(onWindowActivated()));
}

void ChatMarkers::onToolBarWidgetCreated(IMessageToolBarWidget *AWidget)
{
	IMessageChatWindow *chatWindow = qobject_cast<IMessageChatWindow *>(AWidget->messageWindow()->instance());
	if (chatWindow)
	{
		connect(this,SIGNAL(markable(const Jid &,const Jid &)),SLOT(onMarkable(const Jid &,const Jid &)));
		updateToolBarAction(AWidget);
	}
}

void ChatMarkers::onMarkable(const Jid &AStreamJid, const Jid &AContactJid)
{
	IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
	if (window)
		updateToolBarAction(window->toolBarWidget());
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
				if ((*it).resource().isEmpty())
				{
					for (QHash<Jid, QStringList>::Iterator it1 = FLastMarkableDisplayHash[it.key()].begin();
						 it1 != FLastMarkableDisplayHash[it.key()].end(); )
						if (it1.key().bare() == (*it).bare())
						{
							markMessages(it.key(), it1.key(), Displayed, *it1);
							it1 = FLastMarkableDisplayHash[it.key()].erase(it1);
						}
						else
							++it1;
				}
				else
				{
					markMessages(it.key(), *it, Displayed, FLastMarkableDisplayHash[it.key()][*it]);
					FLastMarkableDisplayHash[it.key()].remove(*it);
				}

				if (FLastMarkableDisplayHash[it.key()].isEmpty())
					FLastMarkableDisplayHash.remove(it.key());
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
	if (action!=nullptr)
	{
		IMessageChatWindow *chatWindow = qobject_cast<IMessageChatWindow *>(
					qobject_cast<IMessageToolBarWidget *>(action->parent())
						->messageWindow()->instance());

		QMultiMap<Jid, Jid> addresses = chatWindow->address()->availAddresses();

		for (QMultiMap<Jid, Jid>::ConstIterator it = addresses.constBegin();
			 it != addresses.constEnd(); ++it)
			if (isLastMarkableAcknowledge(it.key(), *it))
			{
				if ((*it).resource().isEmpty())
				{
					for (QHash<Jid, QStringList>::Iterator it1 = FLastMarkableAcknowledgeHash[it.key()].begin();
						 it1 != FLastMarkableAcknowledgeHash[it.key()].end(); )
						if (it1.key().bare() == (*it).bare())
						{
							markMessages(it.key(), it1.key(), Acknowledged, *it1);
							it1 = FLastMarkableAcknowledgeHash[it.key()].erase(it1);
						}
						else
							++it1;
				}
				else
				{
					markMessages(it.key(), *it, Acknowledged, FLastMarkableAcknowledgeHash[it.key()][*it]);
					FLastMarkableAcknowledgeHash[it.key()].remove(*it);
				}

				if (FLastMarkableAcknowledgeHash[it.key()].isEmpty())
					FLastMarkableAcknowledgeHash.remove(it.key());
			}

			updateToolBarAction(chatWindow->toolBarWidget());
	}
}

void ChatMarkers::onPresenceOpened(IPresence *APresence)
{
	if (FPendingMarkers.contains(APresence->streamJid()))
	{
		for (QHash<Jid, QHash<QString, int> >::ConstIterator it = FPendingMarkers[APresence->streamJid()].constBegin();
			 it != FPendingMarkers[APresence->streamJid()].constEnd(); ++it)
			for (QHash<QString, int>::ConstIterator it1 = (*it).constBegin();
				 it1 != (*it).constEnd(); ++it1)
				sendMessageMarked(APresence->streamJid(), it.key(), Type(*it1), it1.key());

		FPendingMarkers.remove(APresence->streamJid());
	}
}

void ChatMarkers::onContactStateChanged(const Jid &AStreamJid, const Jid &AContactJid, bool AStateOnline)
{
	if (AStateOnline || isReceiptsSupported(AStreamJid, AContactJid))
		return;

	if (FLastMarkableDisplayHash.contains(AStreamJid) &&
		FLastMarkableDisplayHash[AStreamJid].contains(AContactJid) &&
		!FLastMarkableDisplayHash[AStreamJid][AContactJid].isEmpty() &&
		!FLastMarkableDisplayHash[AStreamJid][AContactJid].last().isEmpty())
		FLastMarkableDisplayHash[AStreamJid][AContactJid].append(QString());

	if (FLastMarkableAcknowledgeHash.contains(AStreamJid) &&
		FLastMarkableAcknowledgeHash[AStreamJid].contains(AContactJid) &&
		!FLastMarkableAcknowledgeHash[AStreamJid][AContactJid].isEmpty() &&
		!FLastMarkableAcknowledgeHash[AStreamJid][AContactJid].last().isEmpty())
		FLastMarkableAcknowledgeHash[AStreamJid][AContactJid].append(QString());

	if (FRequestHash.contains(AStreamJid) &&
		FRequestHash[AStreamJid].contains(AContactJid) &&
		!FRequestHash[AStreamJid][AContactJid].isEmpty() &&
		!FRequestHash[AStreamJid][AContactJid].last().isEmpty())
		FRequestHash[AStreamJid][AContactJid].append(QString());
}

void ChatMarkers::onMessageDelivered(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId)
{
	FDeliveredHash[AStreamJid][AContactJid].insert(AMessageId);

	// Not sure he code below is necessary
	Jid contactJid;
	QStringList *ids;
	if (FRequestHash[AStreamJid].contains(AContactJid) &&
		FRequestHash[AStreamJid][AContactJid].contains(AMessageId))
	{
		contactJid = AContactJid;
		ids = &FRequestHash[AStreamJid][AContactJid];
	}
	else if (FRequestHash[AStreamJid].contains(AContactJid.bare()) &&
			 FRequestHash[AStreamJid][AContactJid.bare()].contains(AMessageId))
	{
		contactJid = AContactJid.bare();
		ids = &FRequestHash[AStreamJid][AContactJid.bare()];
	}
	else
		return;

	QString lastDisplayed = FLastDisplayed[AStreamJid][AContactJid];
	QString lastAcknowledged = FLastAcknowledged[AStreamJid][AContactJid];

	QStringListIterator it(*ids);

	bool isDisplayed=false;
	bool isAcknowledged=false;

	for (it.toBack(); it.hasPrevious();)
	{
		QString id = it.previous();
		if (id == lastAcknowledged)
			isAcknowledged = true;
		if (id == lastDisplayed)
			isDisplayed = true;
		if (id == AMessageId)
			break;
	}

	int level = Options::node(OPV_MARKERS_SHOW_LEVEL).value().toInt();

	QString image, title;
	if (isAcknowledged && level >= Acknowledged)
	{
		image = MNI_MESSAGE_ACKNOWLEDGED;
		title = tr("Acknowledged");
	}
	else if (isDisplayed && level >= Displayed)
	{
		image = MNI_MESSAGE_DISPLAYED;
		title = tr("Displayed");
	}
	else
		return;

	IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
	setImage(window, AStreamJid, AContactJid, AMessageId, image, title);
}

void ChatMarkers::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_MARKERS_SEND_RECEIVED));
	onOptionsChanged(Options::node(OPV_MARKERS_SEND_DISPLAYED));
	onOptionsChanged(Options::node(OPV_MARKERS_SEND_ACK));
}

void ChatMarkers::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path()==OPV_MARKERS_SEND_RECEIVED)
		registerDiscoFeatures(ANode.value().toBool() ||
							  Options::node(OPV_MARKERS_SEND_DISPLAYED).value().toBool() ||
							  Options::node(OPV_MARKERS_SEND_ACK).value().toBool());
	else if (ANode.path()==OPV_MARKERS_SEND_DISPLAYED)
		registerDiscoFeatures(ANode.value().toBool() ||
							  Options::node(OPV_MARKERS_SEND_RECEIVED).value().toBool() ||
							  Options::node(OPV_MARKERS_SEND_ACK).value().toBool());
	else if (ANode.path()==OPV_MARKERS_SEND_ACK)
	{
		registerDiscoFeatures(ANode.value().toBool() ||
							  Options::node(OPV_MARKERS_SEND_RECEIVED).value().toBool() ||
							  Options::node(OPV_MARKERS_SEND_DISPLAYED).value().toBool());
		updateToolBarAction();
	}
}

bool ChatMarkers::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
	if (FDiscovery && FDiscovery->hasDiscoInfo(AStreamJid,AContactJid) &&
		!FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_CHATMARKERS))
		return false;
	else
		return true;
}

bool ChatMarkers::isReceiptsSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
	return FReceipts && FReceipts->isSupported(AStreamJid, AContactJid);
}

void ChatMarkers::removeNotifiedMessages(IMessageChatWindow *AWindow)
{
	if (FNotifies.contains(AWindow))
	{
		for(QHash<IMessageChatWindow *, int>::const_iterator it=FNotifies.constBegin();
			it!=FNotifies.constEnd(); it++)
			FNotifications->removeNotification(*it);
		FNotifies.remove(AWindow);
	}
}

void ChatMarkers::updateToolBarAction()
{
	QList<IMessageChatWindow*> windows = FMessageWidgets->chatWindows();
	for (QList<IMessageChatWindow*>::ConstIterator it = windows.constBegin();
		 it != windows.constEnd(); ++it)
		updateToolBarAction((*it)->toolBarWidget());
}

void ChatMarkers::updateToolBarAction(IMessageToolBarWidget *AWidget)
{
	bool acknowledge = Options::node(OPV_MARKERS_SEND_ACK).value().toBool();

	IMessageChatWindow *chatWindow = qobject_cast<IMessageChatWindow *>(
				AWidget->messageWindow()->instance());

	ToolBarChanger *changer = AWidget->toolBarChanger();

	QList<QAction*> actions = changer->groupItems(TBG_MWTBW_ACKNOWLEDGEMENT);

	if (acknowledge)
	{
		Action *acknowledgedAction =  actions.isEmpty()?
					nullptr : changer->handleAction(actions.first());
		if (!acknowledgedAction)
		{
			acknowledgedAction = new Action(changer->toolBar());
			acknowledgedAction->setIcon(RSR_STORAGE_MENUICONS, MNI_MESSAGE_ACKNOWLEDGE);
			acknowledgedAction->setText(tr("Acknowledge"));
			//acknowledgedAction->setShortcutId();
			connect(acknowledgedAction,SIGNAL(triggered(bool)),SLOT(onAcknowledgedByAction(bool)));
			AWidget->toolBarChanger()->insertAction(acknowledgedAction,TBG_MWTBW_ACKNOWLEDGEMENT);
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
	else
	{
		if (!actions.isEmpty())
			changer->removeItem(actions.first());
	}
}

bool ChatMarkers::messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection)
{
	Q_UNUSED(AOrder)

	Stanza stanza=AMessage.stanza();
	if (ADirection==IMessageProcessor::DirectionIn)
	{
		if (isSupported(AStreamJid, AMessage.from()) &&
			!stanza.firstElement("markable", NS_CHATMARKERS).isNull() &&
			!AMessage.body().isNull())
		{
			if (Options::node(OPV_MARKERS_SEND_RECEIVED).value().toBool())
				markMessage(AStreamJid, AMessage.from(), Received, AMessage.id());

			if (Options::node(OPV_MARKERS_SEND_DISPLAYED).value().toBool())
			{
				IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AMessage.from());
				if (window && window->isActiveTabPage())
					markMessage(AStreamJid, AMessage.from(), Displayed, AMessage.id());
				else
				{
					if (FLastMarkableDisplayHash[AStreamJid][AMessage.from()].isEmpty())
						FLastMarkableDisplayHash[AStreamJid][AMessage.from()].append(AMessage.id());
					else
						FLastMarkableDisplayHash[AStreamJid][AMessage.from()].last() = AMessage.id();
				}
			}

			bool isMarkedBefore = isLastMarkableAcknowledge(AStreamJid, AMessage.from());

			if (FLastMarkableAcknowledgeHash[AStreamJid][AMessage.from()].isEmpty())
				FLastMarkableAcknowledgeHash[AStreamJid][AMessage.from()].append(AMessage.id());
			else
				FLastMarkableAcknowledgeHash[AStreamJid][AMessage.from()].last() = AMessage.id();

			if (Options::node(OPV_MARKERS_SEND_ACK).value().toBool() && !isMarkedBefore)
				emit markable(AStreamJid, AMessage.from());

			if (Options::node(OPV_MARKERS_SHOW_ACKOWN).value().toBool() &&
				isSupported(AStreamJid, AMessage.from()) &&
				!AMessage.stanza().firstElement("markable", NS_CHATMARKERS).isNull() &&
				!AMessage.body().isNull())
				FAcknowledgeHash[AMessage.from()][AMessage.to()].append(AMessage.id());
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
		if ((Options::node(OPV_MARKERS_SHOW_LEVEL).value().toInt()) &&
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
			if (Options::node(OPV_MARKERS_SHOW_LEVEL).value().toInt())
				FRequestHash[AStreamJid][AMessage.to()].append(AMessage.id());
		}
	}
	return false;
}

bool ChatMarkers::writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang)
{
	Q_UNUSED(AOrder) Q_UNUSED(ALang)
	return AMessage.data(MDR_MESSAGE_DIRECTION).toInt() == IMessageProcessor::DirectionOut &&
			(Options::node(OPV_MARKERS_SHOW_LEVEL).value().toInt()) &&
			!AMessage.stanza().firstElement("markable", NS_CHATMARKERS).isNull();
}

bool ChatMarkers::writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{		
	Q_UNUSED(AOrder)
	Q_UNUSED(ALang)

	int direction = AMessage.data(MDR_MESSAGE_DIRECTION).toInt();

	if (((direction == IMessageProcessor::DirectionOut &&
		  (Options::node(OPV_MARKERS_SHOW_LEVEL).value().toInt()) &&
		  AMessage.stanza().firstElement("request", NS_RECEIPTS).isNull()) ||
		 (direction == IMessageProcessor::DirectionIn &&
		  Options::node(OPV_MARKERS_SHOW_ACKOWN).value().toBool())) &&
		!AMessage.stanza().firstElement("markable", NS_CHATMARKERS).isNull())
	{
		QTextCursor cursor(ADocument);
		cursor.movePosition(QTextCursor::End);
		QTextImageFormat image;
		QString name = QUrl::fromLocalFile(FIconStorage->fileFullName(MNI_EMPTY_BOX)).toString();
		image.setName(name);
		image.setProperty(QpXhtml::ObjectId, calcId(AMessage.from(), AMessage.to(), AMessage.id()));
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

//	qDebug() << "ChatMarkers::archiveMessageEdit(" << AOrder
//			 << "," << AStreamJid.full()
//			 << "," << AMessage.stanza().toString()
//			 << "," << ADirectionIn << ")";

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

void ChatMarkers::setMessageMarker(const Jid &AStreamJid, const Jid &AContactJid,
								   const QString &AMessageId,
								   ChatMarkers::Type AType)
{
	if (FRequestHash.contains(AStreamJid))
	{
		Jid contactJid;
		QStringList *ids;
		if (FRequestHash[AStreamJid].contains(AContactJid) &&
			FRequestHash[AStreamJid][AContactJid].contains(AMessageId))
		{
			contactJid = AContactJid;
			ids = &FRequestHash[AStreamJid][AContactJid];
		}
		else if (FRequestHash[AStreamJid].contains(AContactJid.bare()) &&
				 FRequestHash[AStreamJid][AContactJid.bare()].contains(AMessageId))
		{
			contactJid = AContactJid.bare();
			ids = &FRequestHash[AStreamJid][AContactJid.bare()];
		}
		else
			return;

		bool receiptsSupported = isReceiptsSupported(AStreamJid, AContactJid);

		if (receiptsSupported && (
			!FDeliveredHash.contains(AStreamJid) ||
			!FDeliveredHash[AStreamJid].contains(AContactJid)))
			return;

		if (!receiptsSupported || AType!=Received)
		{
			int index(ids->indexOf(AMessageId)), i(index), idsNum(0);

			IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);

			if (!window)
			{
				REPORT_ERROR("No chat window found for specified Stream JID/Contact JID pair!");
				return;
			}

			if (AType == Acknowledge)
			{
				if (!Options::node(OPV_MARKERS_SHOW_ACKOWN).value().toBool())
					return;
			}
			else
			{
				int level = Options::node(OPV_MARKERS_SHOW_LEVEL).value().toInt();
				if (AType > level)
					AType = Type(level);
			}

			QString lastId = AType == Received  ? FLastReceived[AStreamJid][contactJid]
						   : AType == Displayed ? FLastDisplayed[AStreamJid][contactJid]
						   :					  FLastAcknowledged[AStreamJid][contactJid];

			for (; i>=0 && !ids->at(i).isEmpty() && ids->at(i) != lastId; --i)
			{
				if (receiptsSupported &&
					!FDeliveredHash[AStreamJid][AContactJid].contains(AMessageId))
					continue;

				idsNum++;

				QString image, title;

				switch (AType)
				{
					case Received:
						image = MNI_MESSAGE_RECEIVED;
						title = tr("Received");
						break;
					case Displayed:
						image = MNI_MESSAGE_DISPLAYED;
						title = tr("Displayed");
						break;
					case Acknowledged:
						image = MNI_MESSAGE_ACKNOWLEDGED;
						title = tr("Acknowledged");
						break;
					case Acknowledge:
						image = MNI_MESSAGE_ACKNOWLEDGE;
						title = tr("Acknowledged");
						break;
					default:
						break;
				}

				setImage(window, AStreamJid, AContactJid, ids->at(i), image, title);
			}

			if (idsNum)
				showNotification(AStreamJid, AContactJid, AType, idsNum);
		}

		switch (AType)
		{
			case Acknowledged:
				FLastAcknowledged[AStreamJid][contactJid] = AMessageId;
			case Displayed:
				FLastDisplayed[AStreamJid][contactJid] = AMessageId;
			case Received:
				FLastReceived[AStreamJid][contactJid] = AMessageId;
			default:;
		}
	}
}

void ChatMarkers::setAcknowledgedMarker(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId)
{
	if (FAcknowledgeHash.contains(AStreamJid) &&
		FAcknowledgeHash[AStreamJid].contains(AContactJid))
	{
		QStringList &ids = FAcknowledgeHash[AStreamJid][AContactJid];
		if (ids.contains(AMessageId))
		{
			int index(ids.indexOf(AMessageId)), i(index);

			IMessageChatWindow *window = FMessageWidgets->findChatWindow(AContactJid, AStreamJid);

			if (!window)
			{
				REPORT_ERROR("No chat window found for specified Stream JID/Contact JID pair!");
				return;
			}

			for (; i>=0 && !ids.at(i).isEmpty(); --i)
			{
				QString image = MNI_MESSAGE_ACKNOWLEDGE;
				QString title = tr("Acknowledged");

				setImage(window, AStreamJid, AContactJid, ids.at(i), image, title);

				ids.removeAt(i);
			}
		}
	}
}

void ChatMarkers::setReceived(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId)
{
	setMessageMarker(AStreamJid, AContactJid, AMessageId, Received);
}

void ChatMarkers::setDisplayed(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId)
{
	setMessageMarker(AStreamJid, AContactJid, AMessageId, Displayed);
}

void ChatMarkers::setAcknowledged(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId)
{
	setMessageMarker(AStreamJid, AContactJid, AMessageId, Acknowledged);
}

void ChatMarkers::showNotification(const Jid &AStreamJid, const Jid &AContactJid, const Type &AType, int IdsNum)
{
	if (FMessageWidgets)
	{
		IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
		if (window && !window->isActiveTabPage())
		{
			INotification notify;
			switch (AType) {
			case Received:
				notify.kinds = FNotifications->enabledTypeNotificationKinds(NNT_DELIVERED);
				if (notify.kinds & (INotification::PopupWindow|INotification::SoundPlay))
				{
					notify.typeId = NNT_DELIVERED;
					notify.data.insert(NDR_ICON,FIconStorage->getIcon(MNI_MESSAGE_RECEIVED));
					notify.data.insert(NDR_POPUP_CAPTION, tr("%n message(s) delivered", "", IdsNum));
					notify.data.insert(NDR_POPUP_TITLE, FNotifications->contactName(AStreamJid, AContactJid));
				}
				break;
			case Displayed:
				notify.kinds = FNotifications->enabledTypeNotificationKinds(NNT_DISPLAYED);
				if (notify.kinds & (INotification::PopupWindow|INotification::SoundPlay))
				{
					notify.typeId = NNT_DISPLAYED;
					notify.data.insert(NDR_ICON, FIconStorage->getIcon(MNI_MESSAGE_DISPLAYED));
					notify.data.insert(NDR_POPUP_CAPTION, tr("%n message(s) displayed", "", IdsNum));
					notify.data.insert(NDR_POPUP_TITLE, FNotifications->contactName(AStreamJid, AContactJid));
				}
				break;
			case Acknowledged:
				notify.kinds = FNotifications->enabledTypeNotificationKinds(NNT_ACKNOWLEDGED);
				if (notify.kinds & (INotification::PopupWindow|INotification::SoundPlay))
				{
					notify.typeId = NNT_ACKNOWLEDGED;
					notify.data.insert(NDR_ICON, FIconStorage->getIcon(MNI_MESSAGE_ACKNOWLEDGED));
					notify.data.insert(NDR_POPUP_CAPTION, tr("User acknowledged %n message(s) are read", "", IdsNum));
					notify.data.insert(NDR_POPUP_TITLE, FNotifications->contactName(AStreamJid, AContactJid));
				}
				break;
			default:
				notify.kinds = 0;
				break;
			}

			if (notify.kinds > 0)
			{
				notify.data.insert(NDR_SOUND_FILE, SDF_CHATMARKERS_MARKED);
				FNotifies.insertMulti(window, FNotifications->appendNotification(notify));
				connect(window->instance(), SIGNAL(tabPageActivated()), SLOT(onWindowActivated()));
			}
		}
	}
}

void ChatMarkers::markMessages(const Jid &AStreamJid, const Jid &AContactJid, const ChatMarkers::Type &AType, const QStringList &AMessageIds)
{
	for (QStringList::ConstIterator it = AMessageIds.constBegin();
		 it != AMessageIds.constEnd(); ++it)
		if (!it->isEmpty())
			markMessage(AStreamJid, AContactJid, AType, *it);
}


void ChatMarkers::markMessage(const Jid &AStreamJid, const Jid &AContactJid, const Type &AType, const QString &AMessageId)
{
	if (FPresenceManager->findPresence(AStreamJid)->isOpen())
		sendMessageMarked(AStreamJid, AContactJid, AType, AMessageId);
	else
		FPendingMarkers[AStreamJid][AContactJid].insertMulti(AMessageId, AType);

	if (AType == Acknowledged &&
		Options::node(OPV_MARKERS_SHOW_ACKOWN).value().toBool())
		setAcknowledgedMarker(AContactJid, AStreamJid, AMessageId);
}

void ChatMarkers::sendMessageMarked(const Jid &AStreamJid, const Jid &AContactJid, const ChatMarkers::Type &AType, const QString &AMessageId)
{
	Stanza message("message");
	message.setTo(AContactJid.bare()).setUniqueId();
	switch (AType) {
		case Received:
			message.addElement("received", NS_CHATMARKERS).setAttribute("id", AMessageId);
			break;
		case Displayed:
			message.addElement("displayed", NS_CHATMARKERS).setAttribute("id", AMessageId);
			break;
		case Acknowledged:
			message.addElement("acknowledged", NS_CHATMARKERS).setAttribute("id", AMessageId);
			break;
		default:
			break;
	}
	Message msg(message);
	FMessageProcessor->sendMessage(AStreamJid, msg, IMessageProcessor::DirectionOut);
}

bool ChatMarkers::isLastMarkableDisplay(const Jid &AStreamJid, const Jid &AContactJid) const
{
	if (FLastMarkableDisplayHash[AStreamJid].contains(AContactJid))
		return true;

	if (AContactJid.resource().isEmpty()) // Bare JID
		for (QHash<Jid, QStringList>::ConstIterator it = FLastMarkableDisplayHash[AStreamJid].constBegin();
			 it != FLastMarkableDisplayHash[AStreamJid].constEnd(); ++it)
			if (it.key().bare() == AContactJid.bare())
				return true;

	return false;
}

bool ChatMarkers::isLastMarkableAcknowledge(const Jid &AStreamJid, const Jid &AContactJid) const
{
	if (FLastMarkableAcknowledgeHash[AStreamJid].contains(AContactJid))
		return true;

	if (AContactJid.resource().isEmpty()) // Bare JID
		for (QHash<Jid, QStringList>::ConstIterator it = FLastMarkableAcknowledgeHash[AStreamJid].constBegin();
			 it != FLastMarkableAcknowledgeHash[AStreamJid].constEnd(); ++it)
			if (it.key().bare() == AContactJid.bare())
				return true;

	return false;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_chatmarkers, ChatMarkers)
#endif
