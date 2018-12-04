#include "receipts.h"
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

Receipts::Receipts():
		FMessageProcessor(NULL),
		FMessageArchiver(NULL),
		FDiscovery(NULL),
		FUrlProcessor(NULL),
		FOptionsManager(NULL),
		FNotifications(NULL),
		FMessageWidgets(NULL),
		FIconStorage(NULL)
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
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

	return true;
}

//---------------------
bool Receipts::initSettings()
{
	Options::setDefaultValue(OPV_RECEIPTS_SHOW, true);
	Options::setDefaultValue(OPV_RECEIPTS_SEND, true);
	return true;
}

QMultiMap<int, IOptionsDialogWidget *> Receipts::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_MESSAGES && Options::node(OPV_COMMON_ADVANCED).value().toBool())
	{
		widgets.insertMulti(OWO_MESSAGES_RECEIPTS_SHOW, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_RECEIPTS_SHOW),tr("Show delivery notifications"), AParent));
		widgets.insertMulti(OWO_MESSAGES_RECEIPTS_SEND, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_RECEIPTS_SEND),tr("Send delivery notifications"), AParent));
	}
	return widgets;
}

bool Receipts::initObjects()
{
	FIconStorage = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
	if (FIconStorage)
	{
		QString fileName=FIconStorage->fileFullName(MNI_MESSAGE_RECEIVED);
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
		INotificationType notifyType;
        notifyType.order = NTO_DELIVERED_NOTIFY;
		if (FIconStorage)
			notifyType.icon = FIconStorage->getIcon(MNI_MESSAGE_RECEIVED);
		notifyType.title = tr("When message delivery notification recieved");
		notifyType.kindMask = INotification::PopupWindow|INotification::SoundPlay;
		notifyType.kindDefs = notifyType.kindMask;
		FNotifications->registerNotificationType(NNT_DELIVERED, notifyType);
	}

	if (FUrlProcessor)
		FUrlProcessor->registerUrlHandler("receipts", this);

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
	onOptionsChanged(Options::node(OPV_RECEIPTS_SEND));
}

void Receipts::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path()==OPV_RECEIPTS_SEND)
		registerDiscoFeatures(ANode.value().toBool());
}

bool Receipts::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
	return FDiscovery && FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_RECEIPTS);
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

bool Receipts::messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection)
{
	Q_UNUSED(AOrder)

	Stanza stanza=AMessage.stanza();
	if (ADirection==IMessageProcessor::DirectionIn)
	{
		if (Options::node(OPV_RECEIPTS_SEND).value().toBool() &&
			!stanza.firstElement("request", NS_RECEIPTS).isNull() &&
			!AMessage.body().isNull() &&
			!AMessage.isDelayed())
		{
			Stanza message("message");
			QString id=AMessage.id();
			message.setTo(AMessage.from()).setId(id); // -- Obsolete revision of XEP-0184 --
			message.addElement("received", NS_RECEIPTS).setAttribute("id", id);
			Message msg(message);
			FMessageProcessor->sendMessage(AStreamJid, msg, IMessageProcessor::DirectionOut);
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
			}
		}
	}
	else
	{
		if (Options::node(OPV_RECEIPTS_SHOW).value().toBool() &&
			isSupported(AStreamJid, AMessage.to()) &&
			AMessage.stanza().firstElement("received", NS_RECEIPTS).isNull() &&
			!AMessage.body().isNull())
		{
			if(AMessage.id().isEmpty())
			{
				uint uTime = QDateTime().currentDateTime().toTime_t();
				AMessage.setId(QString().setNum(uTime,16));
			}
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
			Options::node(OPV_RECEIPTS_SHOW).value().toBool() &&
			!AMessage.stanza().firstElement("request", NS_RECEIPTS).isNull();
}

bool Receipts::writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{
	Q_UNUSED(AOrder)
	Q_UNUSED(ALang)

	if (AMessage.data(MDR_MESSAGE_DIRECTION).toInt() == IMessageProcessor::DirectionOut &&
		Options::node(OPV_RECEIPTS_SHOW).value().toBool() &&
	   !AMessage.stanza().firstElement("request", NS_RECEIPTS).isNull())
	{
		QUrl url(QString("receipts:{%1}{%2}{%3}").arg(AMessage.from().toLower())
												 .arg(AMessage.to().toLower())
												 .arg(AMessage.id()));
		QTextCursor cursor(ADocument);
		cursor.movePosition(QTextCursor::End);
		cursor.insertImage(url.toString());
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

	if (!AMessage.stanza().firstElement("recieved", NS_RECEIPTS).isNull())
		return true;
	if (!AMessage.stanza().firstElement("request", NS_RECEIPTS).isNull())
	{
		AMessage.detach();
		AMessage.stanza().element().removeChild(AMessage.stanza().firstElement("request", NS_RECEIPTS));
	}
	return false;
}

QNetworkReply *Receipts::request(QNetworkAccessManager::Operation op, const QNetworkRequest &ARequest, QIODevice *AOutgoingData)
{
	DelayedImageNetworkReply *reply = new DelayedImageNetworkReply(op, ARequest, AOutgoingData, FImgeData, FUrlProcessor->instance());
	connect(this, SIGNAL(delivered(QString)), reply, SLOT(onReady(QString)), Qt::QueuedConnection);
	if (isDelivered(ARequest.url().path()))
		emit delivered(ARequest.url().path());
	return reply;
}

void Receipts::setDelivered(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId)
{
    if (FDeliveryRequestHash.contains(AStreamJid) &&
            FDeliveryRequestHash[AStreamJid].contains(AContactJid) &&
            FDeliveryRequestHash[AStreamJid][AContactJid].contains(AMessageId))
    {
        QString id = QString("{%1}{%2}{%3}").arg(AStreamJid.full().toLower())
                                            .arg(AContactJid.full().toLower())
                                            .arg(AMessageId);
        FDeliveryHash.insert(id);
        QStringList Ids = FDeliveryRequestHash[AStreamJid][AContactJid];
        FDeliveryRequestHash[AStreamJid][AContactJid] = Ids.mid(Ids.indexOf(AMessageId)+1);
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
    //                notify.data.insert(NDR_POPUP_IMAGE, FNotifications->contactAvatar(AContactJid));

                    notify.data.insert(NDR_SOUND_FILE, SDF_RECEIPTS_DELIVERED);
                    FNotifies.insertMulti(window, FNotifications->appendNotification(notify));
                    connect(window->instance(), SIGNAL(tabPageActivated()), SLOT(onWindowActivated()));
                }
            }
        }

        emit delivered(id);
    }
}

bool Receipts::isDelivered(const QString &AId) const
{
	return FDeliveryHash.contains(AId);
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_receipts, Receipts)
#endif
