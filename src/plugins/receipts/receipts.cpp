#include "receipts.h"
#include "receiptsoptions.h"

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
#include <QpXhtml>
#include <QCryptographicHash>

Receipts::Receipts():
		FMessageProcessor(nullptr),
		FMessageArchiver(nullptr),
		FDiscovery(nullptr),
		FOptionsManager(nullptr),
		FNotifications(nullptr),
		FMessageWidgets(nullptr),
		FChatMarkers(nullptr),
		FIconStorage(nullptr)
{}

Receipts::~Receipts()
{}

void Receipts::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Message Delivery Receipts");
	APluginInfo->description = tr("Sends, receives and displays message delivery receipts");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(MESSAGEPROCESSOR_UUID);
}

bool Receipts::initConnections(IPluginManager *APluginManager, int & /*AInitOrder*/)
{
	IPlugin *plugin = APluginManager->pluginInterface("IMessageProcessor").value(0);
	if (plugin)
		FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());
	else
		return false;

	plugin = APluginManager->pluginInterface("IMessageArchiver").value(0);
	if (plugin)
		FMessageArchiver = qobject_cast<IMessageArchiver *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0);
	if (plugin)
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IChatMarkers").value(0);
	if (plugin)
		FChatMarkers = qobject_cast<IChatMarkers *>(plugin->instance());

	plugin = APluginManager->pluginInterface("INotifications").value(0);
	if (plugin)
	{
		FNotifications = qobject_cast<INotifications *>(plugin->instance());
		if (FNotifications)
			connect(FNotifications->instance(), SIGNAL(notificationActivated(int)),
												SLOT(onNotificationActivated(int)));
	}

	plugin = APluginManager->pluginInterface("IMessageWidgets").value(0);
	if (plugin)
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

	return true;
}

//---------------------
bool Receipts::initSettings()
{
	if (!FChatMarkers)
		Options::setDefaultValue(OPV_MARKERS_SHOW_LEVEL, 1);
	Options::setDefaultValue(OPV_MARKERS_SEND_RECEIVED, true);
	return true;
}

QMultiMap<int, IOptionsDialogWidget *> Receipts::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_MESSAGES &&
		Options::node(OPV_COMMON_ADVANCED).value().toBool() &&
		!FChatMarkers)
	{
		widgets.insert(OHO_MESSAGES_MARKERS,
					   FOptionsManager->newOptionsDialogHeader(tr("Message delivery receipts"),
															   AParent));
		widgets.insertMulti(OWO_MESSAGES_MARKERS,
							new ReceiptsOptions(AParent));
	}
	return widgets;
}

bool Receipts::initObjects()
{
	FIconStorage = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);

    if (FNotifications)
    {
        INotificationType notifyType;
		notifyType.order = NTO_DELIVERED_NOTIFY;
        if (FIconStorage)
			notifyType.icon = FIconStorage->getIcon(MNI_MESSAGE_RECEIVED);
        notifyType.title = tr("When message delivery notification received");
		notifyType.kindMask = INotification::PopupWindow|INotification::SoundPlay;
		notifyType.kindDefs = notifyType.kindMask;
		FNotifications->registerNotificationType(NNT_DELIVERED, notifyType);
	}

	if (FMessageProcessor)
	{
		FMessageProcessor->insertMessageEditor(MEO_RECEIPTS, this);
		FMessageProcessor->insertMessageWriter(MWO_RECEIPTS, this);
	}

	if (FMessageArchiver)
		FMessageArchiver->insertArchiveHandler(AHO_DEFAULT, this);

	if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);

	return true;
}

void Receipts::registerDiscoFeatures(bool ARegister)
{
	if (ARegister)
	{
		IDiscoFeature dfeature;
		dfeature.active = true;
		dfeature.var = NS_RECEIPTS;
		dfeature.icon = FIconStorage->getIcon(MNI_MESSAGE_RECEIVED);
		dfeature.name = tr("Message Delivery Receipts");
		dfeature.description = tr("Sends/receives Message Delivery Receipts");
		FDiscovery->insertDiscoFeature(dfeature);
	}
	else
		FDiscovery->removeDiscoFeature(NS_RECEIPTS);
}

void Receipts::onWindowActivated()
{
	IMessageChatWindow *window = qobject_cast<IMessageChatWindow *>(sender());
	if (window)
		removeNotifiedMessages(window);
}

void Receipts::onNotificationActivated(int ANotifyId)
{
	IMessageChatWindow *window=FNotifies.key(ANotifyId);
	if (window)
		window->showTabPage();
}

void Receipts::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_MARKERS_SEND_RECEIVED));
}

void Receipts::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path()==OPV_MARKERS_SEND_RECEIVED)
		registerDiscoFeatures(ANode.value().toBool());
}

bool Receipts::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
	if (FDiscovery && FDiscovery->hasDiscoInfo(AStreamJid, AContactJid))
		return FDiscovery->discoInfo(AStreamJid, AContactJid).features.contains(NS_RECEIPTS);
	else
		return FSupported.contains(AStreamJid) && FSupported[AStreamJid].contains(AContactJid);
}

bool Receipts::isSupportUnknown(const Jid &AStreamJid, const Jid &AContactJid) const
{
	if (FDiscovery && FDiscovery->hasDiscoInfo(AStreamJid, AContactJid))
		return false;
	else
		return !(FSupported.contains(AStreamJid) && FSupported[AStreamJid].contains(AContactJid));


}

void Receipts::removeNotifiedMessages(IMessageChatWindow *AWindow)
{
	if (FNotifies.contains(AWindow))
	{
		for(QHash<IMessageChatWindow *, int>::const_iterator it=FNotifies.constBegin(); it!=FNotifies.constEnd(); it++)
			FNotifications->removeNotification(*it);
		FNotifies.remove(AWindow);
	}
}

static QString calcHash(const Jid &AFrom, const Jid &ATo, const QString &AMessageId)
{
	return QString::fromLatin1(QCryptographicHash::hash(QString("%1|%2|%3").arg(AFrom.full())
																		   .arg(ATo.full())
																		   .arg(AMessageId)
																		   .toUtf8(),
														QCryptographicHash::Md4).toHex());
}

bool Receipts::messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection)
{
	Q_UNUSED(AOrder)

	Stanza stanza=AMessage.stanza();
	if (ADirection==IMessageProcessor::DirectionIn)
	{
		if (!stanza.firstElement("request", NS_RECEIPTS).isNull() &&
			!AMessage.body().isNull())
		{
			if (Options::node(OPV_MARKERS_SEND_RECEIVED).value().toBool()) // Ignore failed messages
			{
				Stanza message("message");
				QString id=AMessage.id();
				message.setTo(AMessage.from()).setId(id); // -- Obsolete revision of XEP-0184 --
				message.addElement("received", NS_RECEIPTS).setAttribute("id", id);
				Message msg(message);
				FMessageProcessor->sendMessage(AStreamJid, msg, IMessageProcessor::DirectionOut);
				if (isSupportUnknown(AStreamJid, AMessage.fromJid()))
					FSupported[AStreamJid].insert(AMessage.fromJid());
			}
		}
		else
		{
			QDomElement rcvd=stanza.firstElement("received", NS_RECEIPTS);
			if(!rcvd.isNull())
			{
				QString id=rcvd.attribute("id");
				if (id.isEmpty())
					id=AMessage.id(); //-- Obsolete revision of XEP-0184 ---
				setDelivered(AStreamJid, AMessage.from(), id);
				if (isSupportUnknown(AStreamJid, AMessage.fromJid()))
					FSupported[AStreamJid].insert(AMessage.fromJid());
			}
		}
	}
	else
	{
		if (Options::node(OPV_MARKERS_SHOW_LEVEL).value().toInt() &&
			(isSupported(AStreamJid, AMessage.toJid()) ||
			 isSupportUnknown(AStreamJid, AMessage.toJid())) &&
			AMessage.stanza().firstElement("received", NS_RECEIPTS).isNull() &&
			!AMessage.body().isNull())
		{
			if(AMessage.id().isEmpty())
				AMessage.setRandomId();
			AMessage.detach();
			AMessage.stanza().addElement("request", NS_RECEIPTS);
			FDeliveryRequestHash[AStreamJid][AMessage.to()].append(AMessage.id());
		}
	}
	return false;
}

bool Receipts::writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang)
{
	Q_UNUSED(AOrder) Q_UNUSED(ALang)
	return AMessage.data(MDR_MESSAGE_DIRECTION).toInt() == IMessageProcessor::DirectionOut &&
						 Options::node(OPV_MARKERS_SHOW_LEVEL).value().toInt() &&
						 !AMessage.stanza().firstElement("request", NS_RECEIPTS).isNull();
}

bool Receipts::writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{
	Q_UNUSED(AOrder)
	Q_UNUSED(ALang)

	if (AMessage.data(MDR_MESSAGE_DIRECTION).toInt() == IMessageProcessor::DirectionOut &&
		Options::node(OPV_MARKERS_SHOW_LEVEL).value().toBool() &&
	   !AMessage.stanza().firstElement("request", NS_RECEIPTS).isNull())
	{	
		QTextCursor cursor(ADocument);
		cursor.movePosition(QTextCursor::End);
		QTextImageFormat image;
		QString name = QUrl::fromLocalFile(FIconStorage->fileFullName(MNI_EMPTY_BOX)).toString();
		image.setName(name);
		image.setProperty(QpXhtml::ObjectId, calcHash(AMessage.from(), AMessage.to(), AMessage.id()));
		cursor.insertImage(image);
		return true;
	}
	return false;
}

bool Receipts::writeTextToMessage(int AOrder, QTextDocument *ADocument, Message &AMessage, const QString &ALang)
{
	Q_UNUSED(AOrder) Q_UNUSED(AMessage) Q_UNUSED(ADocument) Q_UNUSED(ALang)
	return false;
}  // Nothing to do right now

bool Receipts::archiveMessageEdit(int AOrder, const Jid &AStreamJid, Message &AMessage, bool ADirectionIn)
{
	Q_UNUSED(AOrder)
	Q_UNUSED(AStreamJid)
	Q_UNUSED(ADirectionIn)

    if (!AMessage.stanza().firstElement("received", NS_RECEIPTS).isNull())
        return true;
    if (!AMessage.stanza().firstElement("request", NS_RECEIPTS).isNull())
    {
        AMessage.detach();
        AMessage.stanza().element().removeChild(AMessage.stanza().firstElement("request", NS_RECEIPTS));
    }
    return false;
}

void Receipts::setDelivered(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId)
{
	if (FDeliveryRequestHash.contains(AStreamJid))
	{
		Jid contactJid;
		if (FDeliveryRequestHash[AStreamJid].contains(AContactJid) &&
			FDeliveryRequestHash[AStreamJid][AContactJid].contains(AMessageId))
			contactJid = AContactJid;
		else if (FDeliveryRequestHash[AStreamJid].contains(AContactJid.bare()) &&
				 FDeliveryRequestHash[AStreamJid][AContactJid.bare()].contains(AMessageId))
			contactJid = AContactJid.bare();
		if (!contactJid.isEmpty())
		{
			QStringList ids = FDeliveryRequestHash[AStreamJid][contactJid];
			FDeliveryRequestHash[AStreamJid][contactJid] = ids.mid(ids.indexOf(AMessageId)+1);
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
						notify.data.insert(NDR_POPUP_CAPTION, tr("Message delivered"));
						notify.data.insert(NDR_POPUP_TITLE, FNotifications->contactName(AStreamJid, AContactJid));
	//					notify.data.insert(NDR_POPUP_IMAGE, FNotifications->contactAvatar(AContactJid));

						notify.data.insert(NDR_SOUND_FILE, SDF_RECEIPTS_DELIVERED);
						FNotifies.insertMulti(window, FNotifications->appendNotification(notify));
						connect(window->instance(), SIGNAL(tabPageActivated()), SLOT(onWindowActivated()));
					}
				}

				QString hash = calcHash(AStreamJid, contactJid, AMessageId);
				window->viewWidget()->setImageUrl(hash, QUrl::fromLocalFile(FIconStorage->fileFullName(MNI_MESSAGE_RECEIVED)).toString());
				window->viewWidget()->setObjectTitle(hash, tr("Received"));
			}
		}
	}

	emit messageDelivered(AStreamJid, AContactJid, AMessageId);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_receipts, Receipts)
#endif
