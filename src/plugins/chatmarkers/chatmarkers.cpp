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
        FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());

    return true;
}

bool ChatMarkers::initSettings()
{
    Options::setDefaultValue(OPV_CHATMARKERS_SHOW, true);
    Options::setDefaultValue(OPV_CHATMARKERS_SEND, true);
    if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);
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
        INotificationType notifyType;
        notifyType.order = NTO_CHATMARKERS_NOTIFY;
        if (FIconStorage)
            notifyType.icon = FIconStorage->getIcon(MNI_CHATMARKERS);
        notifyType.title = tr("When message marked with a recieved Chat Marker");
        notifyType.kindMask = INotification::PopupWindow|INotification::SoundPlay;
        notifyType.kindDefs = notifyType.kindMask;
        FNotifications->registerNotificationType(NNT_CHATMARKERS, notifyType);
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

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

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

void ChatMarkers::onWindowActivated()
{
    IMessageChatWindow *window = qobject_cast<IMessageChatWindow *>(sender());
    if (window)
        removeNotifiedMessages(window);
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
		registerDiscoFeatures(ANode.value().toBool());
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
            !stanza.firstElement("markable", NS_CHATMARKERS).isNull() &&
            !AMessage.body().isNull() &&
            !AMessage.isDelayed())
        {
            Stanza message("message");
            QString id=AMessage.id();
            message.setTo(AMessage.from()).setId(id); //-- Obsolete revision of XEP-0184 ---
            message.addElement("received", NS_CHATMARKERS).setAttribute("id", id);
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
                setMarked(AStreamJid, AMessage.from(), id);
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
        }
    }
	return false;
}

bool ChatMarkers::writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang)
{
	Q_UNUSED(AOrder) Q_UNUSED(ALang)
	return AMessage.data(MDR_MESSAGE_DIRECTION).toInt() == IMessageProcessor::DirectionOut &&
			Options::node(OPV_RECEIPTS_SHOW).value().toBool() &&
			!AMessage.stanza().firstElement("markable", NS_CHATMARKERS).isNull();
}

bool ChatMarkers::writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{    
	Q_UNUSED(AOrder)
	Q_UNUSED(ALang)

    if (AMessage.data(MDR_MESSAGE_DIRECTION).toInt() == IMessageProcessor::DirectionOut &&
        Options::node(OPV_RECEIPTS_SHOW).value().toBool() &&
       !AMessage.stanza().firstElement("markable", NS_CHATMARKERS).isNull())
    {
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

//QNetworkReply *Receipts::request(QNetworkAccessManager::Operation op, const QNetworkRequest &ARequest, QIODevice *AOutgoingData)
//{
//    return new NetworkReplyReceipts(op, ARequest, AOutgoingData, this, &FImgeData, FUrlProcessor->instance());
//}

void ChatMarkers::setMarked(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId)
{
    QString id = AStreamJid.full()+"/"+AContactJid.full()+"/"+AMessageId;
    FMarkedHash.insert(id);
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
                notify.data.insert(NDR_POPUP_CAPTION, tr("Message marked"));
                notify.data.insert(NDR_POPUP_TITLE, FNotifications->contactName(AStreamJid, AContactJid));
//                notify.data.insert(NDR_POPUP_IMAGE, FNotifications->contactAvatar(AContactJid));

                notify.data.insert(NDR_SOUND_FILE, SDF_CHATMARKERS_MARKED);
                FNotifies.insertMulti(window, FNotifications->appendNotification(notify));
                connect(window->instance(), SIGNAL(tabPageActivated()), SLOT(onWindowActivated()));
            }
        }
    }

    emit marked(id);
}

bool ChatMarkers::isMarked(const QString &AId) const
{
    return FMarkedHash.contains(AId);
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_chatmarkers, ChatMarkers)
#endif
