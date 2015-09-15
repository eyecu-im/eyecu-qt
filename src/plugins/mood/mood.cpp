#include <definitions/rosterlabels.h>
#include <definitions/messagewriterorders.h>
#include <definitions/shortcuts.h>
#include "definitions/notificationdataroles.h"
#include "definitions/notificationtypeorders.h"
#include "definitions/notificationtypes.h"
#include "definitions/soundfiles.h"
#include <definitions/actiongroups.h>
#include <definitions/mapobjecttyperole.h>
#include <definitions/messageeditororders.h>
#include <definitions/menuicons.h>
#include <definitions/namespaces.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/resources.h>
#include <definitions/rosterindexroles.h>
#include <definitions/rosterindexkinds.h>
#include <definitions/rostertooltiporders.h>
#include <definitions/rosterlabelholderorders.h>
#include <definitions/rosterdataholderorders.h>
#include <definitions/toolbargroups.h>
#include <utils/options.h>
#include <utils/iconstorage.h>
#include <utils/menu.h>
#include <utils/shortcuts.h>
#include <utils/qt4qt5compat.h>
#include <MapObject>

#include "moodselect.h"

#define ADR_STREAM_JIDS		Action::DR_StreamJid
#define ADR_CONTACT_JID		Action::DR_Parametr4
#define ADR_GROUP_SHIFT1	Action::DR_Parametr1
#define ADR_MESSAGE_TYPE	Action::DR_UserDefined
#define MT_CHAT     1
#define MT_NORMAL   2

#define TAG_NAME    "mood"

Mood::Mood():
    FOptionsManager(NULL),
    FMessageProcessor(NULL),
    FPEPManager(NULL),
    FDiscovery(NULL),
    FRostersViewPlugin(NULL),
	FPresenceManager(NULL),
    FRostersModel(NULL),
	FXmppStreamManager(NULL),
    FMessageWidgets(NULL),
	FMessageStyleManager(NULL),
    FMap(NULL),    
    FIconStorage(NULL),
    FMapContacts(NULL),
    FNotifications(NULL),
	FSimpleContactsView(false),
    FRosterLabelId(-1),
	FRosterIndexKinds(QList<int>() << RIK_CONTACT << RIK_METACONTACT << RIK_METACONTACT_ITEM << RIK_RECENT_ITEM << RIK_MY_RESOURCE << RIK_STREAM_ROOT)
{

    if(false)
    {
        tr("Afraid");   tr("Amazed");   tr("Amorous");  tr("Angry");
        tr("Annoyed");  tr("Anxious");  tr("Aroused");  tr("Ashamed");
        tr("Bored");    tr("Brave");    tr("Calm");     tr("Cautious");
        tr("Cold");     tr("Confident");tr("Confused"); tr("Contemplative");
        tr("Contented");tr("Cranky");   tr("Crazy");    tr("Creative");
        tr("Curious");  tr("Dejected"); tr("Depressed");tr("Disappointed");
        tr("Disgusted");tr("Dismayed"); tr("Distracted");tr("Embarrassed");
        tr("Envious");  tr("Excited");  tr("Flirtatious");tr("Frustrated");
        tr("Grateful"); tr("Grieving"); tr("Grumpy");   tr("Guilty");
        tr("Happy");    tr("Hopeful");  tr("Hot");      tr("Humbled");
        tr("Humiliated");tr("Hungry");  tr("Hurt");     tr("Impressed");
        tr("In awe");    tr("In love"); tr("Indignant");tr("Interested");
        tr("Intoxicated");tr("Invincible");tr("Jealous");tr("Lonely");
        tr("Lost");     tr("Lucky");    tr("Mean");     tr("Moody");
        tr("Nervous");  tr("Neutral");  tr("Offended"); tr("Outraged");
        tr("Playful");  tr("Proud");    tr("Relaxed");  tr("Relieved");
        tr("Remorseful");tr("Restless");tr("Sad");      tr("Sarcastic");
        tr("Satisfied");tr("Serious");  tr("Shocked");  tr("Shy");
        tr("Sick");     tr("Sleepy");   tr("Spontaneous");tr("Stressed");
        tr("Strong");   tr("Surprised");tr("Thankful"); tr("Thirsty");
        tr("Tired");    tr("Weak");     tr("Worried");  tr("Undefined");
        tr("No mood");
    }
}

Mood::~Mood()
{}

void Mood::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("User Mood");
    APluginInfo->description = tr("Implements XEP-0107: User Mood");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(PEPMANAGER_UUID);
}

bool Mood::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    IPlugin *plugin = APluginManager->pluginInterface("IPEPManager").value(0,NULL);
    if (plugin)
    {
        FPEPManager = qobject_cast<IPEPManager *>(plugin->instance());
        FPEPManager->insertNodeHandler(QString(NS_PEP_MOOD), this);
    } else
        return false;

    plugin = APluginManager->pluginInterface("IMessageWidgets").value(0);
    if (plugin)
    {
        FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
        connect(FMessageWidgets->instance(),SIGNAL(chatWindowCreated(IMessageChatWindow *)),SLOT(onChatWindowCreated(IMessageChatWindow *)), Qt::QueuedConnection);
        connect(FMessageWidgets->instance(),SIGNAL(normalWindowCreated(IMessageNormalWindow *)),SLOT(onNormalWindowCreated(IMessageNormalWindow *)));
    }

	plugin = APluginManager->pluginInterface("IMessageStyleManager").value(0,NULL);
    if (plugin)
		FMessageStyleManager = qobject_cast<IMessageStyleManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IMessageProcessor").value(0);
    if (plugin)
        FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());

    plugin = APluginManager->pluginInterface("INotifications").value(0);
    if (plugin)
    {
        FNotifications = qobject_cast<INotifications *>(plugin->instance());
        if (FNotifications)
            connect(FNotifications->instance(), SIGNAL(notificationActivated(int)), SLOT(onNotificationActivated(int)));
    }

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0,NULL);
    if (plugin)
		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0,NULL);
    if (plugin)
    {
		FXmppStreamManager = qobject_cast<IXmppStreamManager *>(plugin->instance());
		if (FXmppStreamManager)
        {
			connect(FXmppStreamManager->instance(),SIGNAL(streamOpened(IXmppStream *)),SLOT(onStreamOpened(IXmppStream *)));
			connect(FXmppStreamManager->instance(),SIGNAL(streamClosed(IXmppStream *)),SLOT(onStreamClosed(IXmppStream *)));
        }
    }

    plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
    connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
    connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

    plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
    if (plugin)
        FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IRostersModel").value(0,NULL);
    if (plugin)
    {
        FRostersModel = qobject_cast<IRostersModel *>(plugin->instance());
        if (FRostersModel)
            connect(FRostersModel->instance(),SIGNAL(indexInserted(IRosterIndex *)),
                    SLOT(onRosterIndexInserted(IRosterIndex *)));
    }

    plugin = APluginManager->pluginInterface("IMap").value(0,NULL);
    if (plugin)
        FMap = qobject_cast<IMap *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IMapContacts").value(0,NULL);
    if (plugin)
        FMapContacts = qobject_cast<IMapContacts *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IRostersViewPlugin").value(0,NULL);
    if (plugin)
    {
        FRostersViewPlugin = qobject_cast<IRostersViewPlugin *>(plugin->instance());
        if (FRostersViewPlugin)
        {
            connect(FRostersViewPlugin->rostersView()->instance(),
                    SIGNAL(indexContextMenu(QList<IRosterIndex *>, quint32, Menu *)),
                    SLOT(onRosterIndexContextMenu(QList<IRosterIndex *>, quint32, Menu *)));
            connect(FRostersViewPlugin->rostersView()->instance(),
                    SIGNAL(indexToolTips(IRosterIndex *, quint32, QMap<int,QString> &)),
                    SLOT(onRosterIndexToolTips(IRosterIndex *, quint32, QMap<int,QString> &)));
        }
    }

    AInitOrder = 200;   // This one should be initialized AFTER Map Contacts!

    return true;
}

bool Mood::initObjects()
{
	Shortcuts::declareShortcut(SCT_APP_SETMOOD, tr("Set mood"), tr("Ctrl+F4", "Set mood (for all accounts)"), Shortcuts::ApplicationShortcut);
    Shortcuts::declareShortcut(SCT_ROSTERVIEW_SETMOOD, tr("Set mood"), tr("F4", "Set mood (for an account)"), Shortcuts::WidgetShortcut);
    Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_SETMOOD, tr("Set mood"), tr("F4", "Set mood (for a message)"), Shortcuts::WindowShortcut);

    if (FDiscovery)
        registerDiscoFeatures();

    if (FRostersViewPlugin)
    {
        AdvancedDelegateItem label(RLID_MOOD);
        label.d->kind = AdvancedDelegateItem::CustomData;
        label.d->data = RDR_MOOD_IMAGE;
        FRosterLabelId = FRostersViewPlugin->rostersView()->registerLabel(label);
		FRostersViewPlugin->rostersView()->insertLabelHolder(RLHO_MOOD, this);
        Shortcuts::insertWidgetShortcut(SCT_ROSTERVIEW_SETMOOD, FRostersViewPlugin->rostersView()->instance());
        connect(Shortcuts::instance(), SIGNAL(shortcutActivated(QString,QWidget*)), SLOT(onShortcutActivated(QString,QWidget*)));
    }

    if (FRostersModel)
        FRostersModel->insertRosterDataHolder(RDHO_MOOD, this);

    FIconStorage = IconStorage::staticStorage(RSR_STORAGE_MOOD);
    FMoodList=FIconStorage->fileFirstKeys();

    QList<QString> keys = FIconStorage->fileKeys();    
    for(QStringList::const_iterator it=keys.constBegin(); it!=keys.constEnd(); it++)
    {
        QString mood=*it;
        it++;
        if (mood != TAG_NAME)
			FMoodKeys.insert(mood, tr((*it).toLatin1().data()));
    }

    if (FMap)
    {
		FMap->geoMap()->registerDataType(MDR_MOOD_ICON, MOT_CONTACT, 200, MOP_RIGHT_TOP);
		FMap->geoMap()->addDataHolder(MOT_CONTACT, this);
    }

    if (FMessageProcessor)
    {
        FMessageProcessor->insertMessageWriter(MWO_MOOD, this);
        FMessageProcessor->insertMessageEditor(MEO_MOOD, this);
    }

    if (FNotifications)
    {
        INotificationType notifyType;
        notifyType.order = NTO_MOOD_CHANGE;
        if (FIconStorage)
            notifyType.icon = FIconStorage->getIcon(MNI_MOOD);
        notifyType.title = tr("When user mood changed");
        notifyType.kindMask = INotification::PopupWindow|INotification::SoundPlay;
        notifyType.kindDefs = notifyType.kindMask;
        FNotifications->registerNotificationType(NNT_MOOD, notifyType);
    }

	Action *action = FPEPManager->addAction(AG_MOOD);
	action->setText(tr("Mood"));
	action->setIcon(RSR_STORAGE_MOOD, MNI_MOOD);
	action->setShortcutId(SCT_APP_SETMOOD);
	connect(action, SIGNAL(triggered(bool)), SLOT(showMoodSelector(bool)));

    return true;
}

bool Mood::initSettings()
{
    Options::setDefaultValue(OPV_ROSTER_MOOD_SHOW, true);
    Options::setDefaultValue(OPV_MESSAGES_MOOD_DISPLAY, true);
	Options::setDefaultValue(OPV_MESSAGES_MOOD_NOTIFY, true);
    if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);
    return true;
}

void Mood::showMoodSelector(bool)
{    
    MoodSelect *myMood = new MoodSelect(this, FMoodList, FMoodTextLists, FMoodKeys, FCurrentMood);
    if(myMood->exec())
    {
        MoodData moodData = myMood->moodData();
        if (!moodData.isEmpty())
            saveComments(moodData);
		for (QSet<Jid>::ConstIterator it = FStreamsOnline.begin(); it != FStreamsOnline.end(); ++it)
			if (FPEPManager->isSupported(*it))
				sendMood(moodData, *it);
    }
    myMood->deleteLater();
}

void Mood::onShortcutActivated(const QString &AString, QWidget *AWidget)
{
	Q_UNUSED(AWidget)

    if (AString==SCT_ROSTERVIEW_SETMOOD)
    {
        QList<IRosterIndex *>indexes=FRostersViewPlugin->rostersView()->selectedRosterIndexes();
        for (QList<IRosterIndex *>::const_iterator it=indexes.constBegin(); it!=indexes.constEnd(); it++)
            if ((*it)->kind()==RIK_STREAM_ROOT)
            {
                Jid streamJid((*it)->data(RDR_STREAM_JID).toString());
				if (FStreamsOnline.contains(streamJid) && FPEPManager->isSupported(streamJid))
                    setMoodForAccount(streamJid);
            }
    }
}

bool Mood::processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza)
{
	if (AStanza.type()!="error")
	{
		Jid contactJid = AStanza.from();
		QDomElement event = AStanza.firstElement("event", NS_PUBSUB_EVENT);
		QDomElement items = event.firstChildElement("items");

		if(!items.isNull())
		{
			bool stop=false;
			QDomElement item = items.firstChildElement("item");
			if(!item.isNull())
			{
				QDomElement mood = item.firstChildElement(TAG_NAME);
				if(!mood.isNull())
				{
					if(contactJid.bare() == AStreamJid.bare())
						FIdHash.insert(AStreamJid.bare(), item.attribute("id"));
					QString moodName, moodText;
					for (QDomElement e=mood.firstChildElement(); !e.isNull(); e=e.nextSiblingElement())
						if (e.tagName()=="text")
							moodText=e.text();
						else
							moodName=e.tagName();

					if (moodName.isNull())
						stop=true;
					else
					{
						MoodData moodData(moodName, moodText);
						if (moodData != FMoodHash.value(contactJid.bare()))
						{
							if (contactJid.bare() == AStreamJid.bare())
								FCurrentMood = moodData;
							FMoodHash.insert(contactJid.bare(), moodData);
							updateDataHolder(contactJid);
							updateChatWindows(AStreamJid, contactJid);
							displayNotification(AStreamJid, contactJid);
						}
						return true;
					}
				}
			}

			if(!stop && event.firstChild().firstChild().nodeName() == "retract")
			{
				FIdHash.remove(AStreamJid.bare());
				stop=true;
			}

			if (stop)
			{
				if (contactJid.bare() == AStreamJid.bare())
					FCurrentMood.clear();
				FMoodHash.remove(contactJid.bare());
				updateDataHolder(contactJid);
				updateChatWindows(AStreamJid, contactJid);
				return true;
			}
		}
	}
    return false;
}
//---------------------

void Mood::registerDiscoFeatures()
{
    IDiscoFeature dfeature;
    dfeature.var = NS_PEP_MOOD;
    dfeature.active = true;
    dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MOOD)->getIcon(MNI_MOOD);
    dfeature.name = tr("User Mood");
    dfeature.description = tr("Supports User Mood");
    FDiscovery->insertDiscoFeature(dfeature);

    dfeature.var.append(NODE_NOTIFY_SUFFIX);
    dfeature.name = tr("User Mood Notification");
    dfeature.description = tr("Receives User Mood notifications");
	FDiscovery->insertDiscoFeature(dfeature);
}

bool Mood::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
    return FDiscovery==NULL || !FDiscovery->hasDiscoInfo(AStreamJid,AContactJid)
                            || FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_PEP_MOOD);
}


void Mood::sendMood(const MoodData &AMoodData, const Jid &AStreamJid)
{
    QDomDocument doc;
    QDomElement item=doc.createElement("item");
    item.setAttribute("id", currentItemId(AStreamJid));
    if(!AMoodData.isEmpty())
    {
        QDomElement moodNode=doc.createElementNS(NS_PEP_MOOD, TAG_NAME);
        item.appendChild(moodNode);
        QDomElement moodName=doc.createElement(AMoodData.name);
        moodNode.appendChild(moodName);
        if(!AMoodData.text.isEmpty())
        {
            QDomElement moodText=doc.createElement("text");
            moodNode.appendChild(moodText);
            moodText.appendChild(doc.createTextNode(AMoodData.text));
        }
        FPEPManager->publishItem(AStreamJid, NS_PEP_MOOD, item);
    }
    else
    {
        if (Options::node(OPV_PEP_DELETE_PUBLISHEMPTY).value().toBool())
        {
            item.appendChild(doc.createElementNS(NS_PEP_MOOD, TAG_NAME));
            FPEPManager->publishItem(AStreamJid, NS_PEP_MOOD, item);
        }
        if (Options::node(OPV_PEP_DELETE_RETRACT).value().toBool())
            FPEPManager->deleteItem(AStreamJid, NS_PEP_MOOD, item);
    }
}

QList<quint32> Mood::rosterLabels(int AOrder, const IRosterIndex *AIndex) const
{
	QList<quint32> labels;
	if (AOrder==RLHO_MOOD && AIndex->kind()==RIK_RECENT_ITEM)
		if (FSimpleContactsView)
			labels.append(RLID_MOOD);
	return labels;
}

AdvancedDelegateItem Mood::rosterLabel(int AOrder, quint32 ALabelId, const IRosterIndex *AIndex) const
{
	Q_UNUSED(AOrder); Q_UNUSED(ALabelId); Q_UNUSED(AIndex);
	static const AdvancedDelegateItem null = AdvancedDelegateItem();
	return null;
}

QList<int> Mood::rosterDataRoles(int AOrder) const
{
    if (AOrder==RDHO_MOOD)
        return QList<int>() << RDR_MOOD_IMAGE;
    return QList<int>();
}

QVariant Mood::rosterData(int AOrder, const IRosterIndex *AIndex, int ARole) const
{
    Q_UNUSED(AOrder);
    if (ARole == RDR_MOOD_IMAGE)
    {
        Jid jid(AIndex->data(RDR_FULL_JID).toString());
        if (FMoodHash.contains(jid.bare()))
            return getIcon(FMoodHash[jid.bare()].name);
    }
    return QVariant();
}

bool Mood::setRosterData(int AOrder, const QVariant &AValue, IRosterIndex *AIndex, int ARole)
{
    Q_UNUSED(AOrder);
    Q_UNUSED(AIndex);
    Q_UNUSED(ARole);
    Q_UNUSED(AValue);
    return false;
}

void Mood::updateDataHolder(const Jid &AContactJid)
{
    if (FRostersModel)
    {
        QMultiMap<int,QVariant> findData;
        for(QList<int>::const_iterator it=FRosterIndexKinds.constBegin(); it!=FRosterIndexKinds.constEnd(); it++)
            findData.insert(RDR_KIND, *it);
        if (!AContactJid.isEmpty())
            findData.insert(RDR_PREP_BARE_JID, AContactJid.pBare());
        QList<IRosterIndex *> indexes = FRostersModel->rootIndex()->findChilds(findData, true);
        for (QList<IRosterIndex *>::const_iterator it=indexes.constBegin(); it!=indexes.constEnd(); it++)
            emit rosterDataChanged(*it, RDR_MOOD_IMAGE);
    }

    if (FMap && FMapContacts)
    {
        QString bareJid=AContactJid.bare();
        QStringList jids=FMapContacts->getFullJidList(bareJid);
        if (!jids.isEmpty())    // There are such contacts on the map!!!
        {
            if (FMoodHash.contains(bareJid))
                for (QStringList::const_iterator it=jids.constBegin(); it!=jids.constEnd(); it++)
                    emit mapDataChanged(MOT_CONTACT, *it, MDR_MOOD_ICON); // Activity either added or removed!
            else
                for (QStringList::const_iterator it=jids.constBegin(); it!=jids.constEnd(); it++)
                    emit mapDataChanged(MOT_CONTACT, *it, MDR_MOOD_ICON); // Activity either added or removed!
        }
    }
}

QMultiMap<int, IOptionsDialogWidget *> Mood::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
    if (FOptionsManager)
    {
		if (ANodeId == OPN_ROSTERVIEW)
		{
			if (Options::node(OPV_COMMON_ADVANCED).value().toBool())
				widgets.insertMulti(OWO_ROSTER_PEP_MOOD, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ROSTER_MOOD_SHOW), tr("Show user mood icons"), AParent));
		}
        else if (ANodeId == OPN_MESSAGES)
		{
			widgets.insertMulti(OWO_MESSAGES_INFOBAR_MOOD, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MESSAGES_MOOD_DISPLAY), tr("Display user mood icon"), AParent));
			widgets.insertMulti(OWO_MESSAGES_PEP_MOOD, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MESSAGES_MOOD_NOTIFY), tr("Mood events in chat"), AParent));
		}
    }
    return widgets;
}


void Mood::onStreamOpened(IXmppStream *AXmppStream)
{
	FStreamsOnline.insert(AXmppStream->streamJid());
}

void Mood::onStreamClosed(IXmppStream *AXmppStream)
{
	FStreamsOnline.remove(AXmppStream->streamJid());
}

void Mood::onRosterIndexInserted(IRosterIndex *AIndex)
{
	if (FRostersViewPlugin &&  FRosterIndexKinds.contains(AIndex->kind()))
		if (Options::node(OPV_COMMON_ADVANCED).value().toBool()?Options::node(OPV_ROSTER_MOOD_SHOW).value().toBool():Options::node(OPV_ROSTER_VIEWMODE).value().toInt()==IRostersView::ViewFull)
            FRostersViewPlugin->rostersView()->insertLabel(FRosterLabelId, AIndex);
}

void Mood::onRosterIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
	QStringList streamJids;
    if (ALabelId == AdvancedDelegateItem::DisplayId || ALabelId == FRosterLabelId)
        for (QList<IRosterIndex *>::const_iterator it=AIndexes.constBegin(); it!=AIndexes.constEnd(); it++)
			if ((*it)->kind() == RIK_STREAM_ROOT)
			{
				Jid streamJid((*it)->data(RDR_STREAM_JID).toString());
				if (FStreamsOnline.contains(streamJid) && FPEPManager->isSupported(streamJid))
					streamJids.append((*it)->data(RDR_STREAM_JID).toString());
			}

	if (!streamJids.isEmpty())
	{
		Action *action = new Action(AMenu);
		action->setText(tr("Mood"));
		action->setIcon(RSR_STORAGE_MOOD, MNI_MOOD);
		action->setData(ADR_STREAM_JIDS, streamJids);
		connect(action, SIGNAL(triggered(bool)), SLOT(onSetMoodByAction(bool)));
		AMenu->addAction(action, AG_RVCM_ACTIVITY, true);
	}
}

void Mood::onRosterIndexToolTips(IRosterIndex * AIndex, quint32 ALabelId, QMap<int, QString> &AToolTips)
{    
	if ((ALabelId == AdvancedDelegateItem::DisplayId || ALabelId == FRosterLabelId) && FRosterIndexKinds.contains(AIndex->kind()))
    {
        Jid jid(AIndex->data(RDR_FULL_JID).toString());
        QString label=getLabel(jid);
        if (!label.isEmpty())
            AToolTips.insert(RTTO_MOOD, label);
    }
}

void Mood::setMoodForAccount(Jid AStreamJid)
{
    MoodData moodData = FMoodHash.value(AStreamJid.bare());
    MoodSelect *myMood= new MoodSelect(this, FMoodList, FMoodTextLists, FMoodKeys, moodData);
    if(myMood->exec())
    {
        moodData = myMood->moodData();
        sendMood(moodData, AStreamJid);
        if (!moodData.isEmpty())
            saveComments(moodData);
    }
    myMood->deleteLater();
}


void Mood::onSetMoodByAction(bool)
{
	QStringList streamJids = qobject_cast<Action *>(sender())->data(ADR_STREAM_JIDS).toStringList();
	for (QStringList::ConstIterator it = streamJids.constBegin(); it !=  streamJids.constEnd(); ++it)
		setMoodForAccount(*it);
}

void Mood::updateChatWindows(bool AInfoBar)
{
	if (FMessageWidgets)
	{
		QList<IMessageChatWindow *> chatWindows=FMessageWidgets->chatWindows();
		for(QList<IMessageChatWindow *>::const_iterator it=chatWindows.constBegin(); it!=chatWindows.constEnd(); it++)
			if (AInfoBar)
				updateChatWindowInfo(*it);
			else
				updateChatWindow(*it);
	}
}

void Mood::updateChatWindows(const Jid &AStreamJid, const Jid &AContactJid)
{
	if (FMessageWidgets)
	{
		IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
		if (window)
		{
			updateChatWindow(window);
			updateChatWindowInfo(window);
		}
	}
}

void Mood::updateChatWindow(IMessageChatWindow *AMessageChatWindow)
{
    Jid contactJid=AMessageChatWindow->contactJid();
	if (Options::node(OPV_MESSAGES_MOOD_NOTIFY).value().toBool() && FMoodHash.contains(contactJid.bare()))
    {
        MoodData mood=FMoodHash[contactJid.bare()];
		QUrl iconUrl = QUrl::fromLocalFile(getIconFileName(mood.name));
		QString pic = QString("<img src=\"%1\" title=\"%2\" alt=\"%2\" />")
			.arg(iconUrl.toString()).arg(FMoodKeys.value(mood.name));

		QString message;
		if (!mood.text.isEmpty())
			message=QString(" (%1)").arg(HTML_ESCAPE(mood.text));

		QString longMessage = QString("<i>")
							  .append(tr("%1 changed mood to %2").arg(AMessageChatWindow->infoWidget()->fieldValue(IMessageInfoWidget::Name).toString())
																 .arg(pic.append(message)))
							  .append("</i>");

		IMessageStyleContentOptions options;
		options.time = QDateTime::currentDateTime();
		options.timeFormat = FMessageStyleManager->timeFormat(options.time);
		options.type = IMessageStyleContentOptions::TypeEvent;
		options.kind = IMessageStyleContentOptions::KindStatus;
		options.direction = IMessageStyleContentOptions::DirectionIn;
		options.senderId  = contactJid.full();
		options.senderName   = HTML_ESCAPE(FMessageStyleManager->contactName(AMessageChatWindow->streamJid(), AMessageChatWindow->contactJid()));
		options.senderAvatar = FMessageStyleManager->contactAvatar(contactJid);
		AMessageChatWindow->viewWidget()->appendHtml(longMessage, options);
    }	
}

void Mood::updateChatWindowInfo(IMessageChatWindow *AMessageChatWindow)
{
	Jid contactJid=AMessageChatWindow->contactJid();
	if (Options::node(OPV_MESSAGES_MOOD_DISPLAY).value().toBool() && FMoodHash.contains(contactJid.bare()))
	{
		QLabel *label = NULL;
		QList<QAction *> actions=AMessageChatWindow->infoWidget()->infoToolBarChanger()->groupItems(AG_MOOD);
		if (!actions.isEmpty())
			label = qobject_cast<QLabel*>(AMessageChatWindow->infoWidget()->infoToolBarChanger()->toolBar()->widgetForAction(actions.first()));
		if (!label)
		{
			label = new QLabel(AMessageChatWindow->infoWidget()->infoToolBarChanger()->toolBar());
			label->setMargin(3);
			AMessageChatWindow->infoWidget()->infoToolBarChanger()->insertWidget(label, AG_MOOD);
		}
		label->setPixmap(getIcon(FMoodHash[contactJid.bare()].name).pixmap(16));
		label->setToolTip(getLabel(contactJid));
	}
	else
	{
		QList<QAction *> actions=AMessageChatWindow->infoWidget()->infoToolBarChanger()->groupItems(AG_MOOD);
		if (!actions.isEmpty())
		{
			AMessageChatWindow->infoWidget()->infoToolBarChanger()->handleWidget(actions[0])->deleteLater();
			AMessageChatWindow->infoWidget()->infoToolBarChanger()->removeItem(actions[0]);
			actions[0]->deleteLater();
		}
	}
}

QIcon Mood::getIcon(const QString &moodName) const
{
    return FIconStorage->getIcon(moodName);
}

QString Mood::getIconFileName(const QString &moodName) const
{
    return FIconStorage->fileFullName(moodName);
}

QString Mood::getIconName(const Jid &AContactJid) const
{
    return  FMoodHash.contains(AContactJid.bare())?getIconFileName(FMoodHash.value(AContactJid.bare()).name):QString();
}

QString Mood::getText(const Jid &AContactJid) const
{
    return  FMoodHash.value(AContactJid.bare()).text;
}

QString Mood::getLabel(const Jid &AContactJid) const
{
    return FMoodHash.contains(AContactJid.bare())?getLabel(FMoodHash[AContactJid.bare()])
                                                 :QString();
}

QString Mood::getLabel(const MoodData &AMoodData) const
{
    QString label=QString("<strong>%1:</strong> %2").arg(tr("Mood")).arg(FMoodKeys.value(AMoodData.name));
    if (!AMoodData.text.isEmpty())
        label.append(QString(" <i>(%1)</i>").arg(AMoodData.text));
    return label;
}

void Mood::displayNotification(const Jid &AStreamJid, const Jid &AContactJid)
{
    if (AStreamJid.bare()==AContactJid.bare())
        return;

    if (Options::node(OPV_PEP_NOTIFY_IGNOREOFFLINE).value().toBool())
    {
        int show = presenceItemForBareJid(AStreamJid, AContactJid).show;
        if (show==IPresence::Offline || show==IPresence::Error)
            return;
    }

    INotification notify;
    notify.kinds = FNotifications->enabledTypeNotificationKinds(NNT_MOOD);

    if (FMessageWidgets)
    {
        IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
        if (window && window->isActiveTabPage())    // The window is existing and is an active tab page!
            notify.kinds = 0;                       // So, do not to notify!
    }

    if (notify.kinds)
    {
        MoodData moodData=FMoodHash.value(AContactJid.bare());
        QString html=tr("Changed mood to: %1").arg(QString("<b>").append(FMoodKeys.value(moodData.name)).append("</b>"));
        if (!moodData.text.isEmpty())
            html.append(QString("<br><i>%1</i>").arg(moodData.text));        

        notify.typeId = NNT_MOOD;
        notify.data.insert(NDR_STREAM_JID, AStreamJid.full());
        notify.data.insert(NDR_CONTACT_JID, AContactJid.full());
        notify.data.insert(NDR_ICON, getIcon(moodData.name));
        notify.data.insert(NDR_POPUP_HTML, html);
        notify.data.insert(NDR_POPUP_TITLE, FNotifications->contactName(AStreamJid, AContactJid));
        notify.data.insert(NDR_POPUP_IMAGE, FNotifications->contactAvatar(AContactJid));
        notify.data.insert(NDR_SOUND_FILE, SDF_PEP_EVENT);
        if (FNotifies[AStreamJid].contains(AContactJid))
            FNotifications->removeNotification(FNotifies[AStreamJid].value(AContactJid));
        FNotifies[AStreamJid].insert(AContactJid, FNotifications->appendNotification(notify));
    }
}

void Mood::updateChatWindowActions(IMessageChatWindow *AChatWindow)
{
    QList<QAction *> actions = AChatWindow->toolBarWidget()->toolBarChanger()->groupItems(TBG_MWTBW_MOOD_VIEW);
    QAction *handle=actions.value(0, NULL);
    if(isSupported(AChatWindow->streamJid(), AChatWindow->contactJid()))
    {
        if (!handle)
        {
            IMessageAddress *address=AChatWindow->address();
            Action *action = new Action(AChatWindow->toolBarWidget()->instance());
            action->setText(tr("Add mood"));
            action->setIcon(RSR_STORAGE_MOOD, MNI_MOOD);
            action->setData(ADR_MESSAGE_TYPE, MT_CHAT);
            action->setShortcutId(SCT_MESSAGEWINDOWS_SETMOOD);
            action->setData(ADR_CONTACT_JID, address->contactJid().full());
			action->setData(ADR_STREAM_JIDS, address->streamJid().full());
            connect(action,SIGNAL(triggered(bool)),SLOT(onAddMood(bool)));			
			AChatWindow->toolBarWidget()->toolBarChanger()->insertAction(action, TBG_MWTBW_MOOD_VIEW);
        }
    }
    else
    {
        if (handle)
        {
            AChatWindow->toolBarWidget()->toolBarChanger()->removeItem(handle);
            handle->deleteLater();
        }
    }
}

void Mood::removeNotifiedMessages(IMessageChatWindow *AWindow)
{
    if (FNotifies.contains(AWindow->streamJid()) &&
        FNotifies[AWindow->streamJid()].contains(AWindow->contactJid().bare()))
    {
        FNotifications->removeNotification(FNotifies[AWindow->streamJid()][AWindow->contactJid().bare()]);
        FNotifies[AWindow->streamJid()].remove(AWindow->contactJid().bare());
        if (FNotifies[AWindow->streamJid()].isEmpty())
            FNotifies.remove(AWindow->streamJid());
    }
}

IPresenceItem Mood::presenceItemForBareJid(const Jid &AStreamJid, const Jid &AContactJid) const
{
	IPresence *presence = FPresenceManager ? FPresenceManager->findPresence(AStreamJid) : NULL;
	return presence ? FPresenceManager->sortPresenceItems(presence->findItems(AContactJid)).value(0) : IPresenceItem();
}

void Mood::saveComments(const MoodData &AMoodData)
{
    if (!AMoodData.text.isEmpty())
    {
        if (FMoodTextLists[AMoodData.name].contains(AMoodData.text))
            FMoodTextLists[AMoodData.name].removeAll(AMoodData.text);
        FMoodTextLists[AMoodData.name].prepend(AMoodData.text);
        Options::node(OPV_MOOD_COMMENT).setValue(FMoodTextLists[AMoodData.name], AMoodData.name);
    }
}

//------------------------------

void Mood::onMapObjectInserted(int AType, const QString &AId)
{
    if (AType==MOT_CONTACT)
        if (FMoodHash.contains(Jid(AId).bare()))
            emit mapDataChanged(AType, AId, MDR_MOOD_ICON);
}

void Mood::onMapObjectRemoved(int AType, const QString &AId)
{
    Q_UNUSED(AType)
    Q_UNUSED(AId)
}

QGraphicsItem * Mood::mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement)
{
    if (ARole == MDR_MOOD_ICON)
    {
        Jid jid(ASceneObject->mapObject()->id());
        if (FMoodHash.contains(jid.bare()))
        {
            QIcon icon=getIcon(FMoodHash[jid.bare()].name);
            if (icon.isNull())
                return NULL;
            else
            {
                if (ACurrentElement)
                    qgraphicsitem_cast<QGraphicsPixmapItem *>(ACurrentElement)->setPixmap(icon.pixmap(icon.availableSizes().first()));
                else
                    ACurrentElement = new QGraphicsPixmapItem(icon.pixmap(icon.availableSizes().first()));
            }
        }
    }
    return ACurrentElement;
}

void Mood::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_ROSTER_RECENT_SIMPLEITEMSVIEW));
    for(QStringList::const_iterator it=FMoodList.constBegin(); it!=FMoodList.constEnd(); it++)
    {
        QString com=OPV_MOOD_COMMENT;
        com.append(".").append(*it);
        QStringList tmp=Options::node(com).value().toStringList();
        if(!tmp.isEmpty())
            FMoodTextLists.insert(*it, tmp);
    }
}

void Mood::onOptionsClosed()
{}

void Mood::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_MESSAGES_MOOD_DISPLAY)
		updateChatWindows(true);
	else if (ANode.path() == OPV_ROSTER_RECENT_SIMPLEITEMSVIEW)
	{
		FSimpleContactsView = ANode.value().toBool();
		QMultiMap<int, QVariant> findData;
		findData.insert(RDR_KIND, RIK_RECENT_ITEM);
		QList<IRosterIndex *> indexes = FRostersModel->rootIndex()->findChilds(findData, true);
		for (QList<IRosterIndex *>::ConstIterator it = indexes.constBegin(); it!=indexes.constEnd(); it++)
			emit rosterLabelChanged(RLID_MOOD, *it);
	}
	else if (ANode.path() == OPV_MESSAGES_MOOD_NOTIFY)
		updateChatWindows(false);
	else if (ANode.path() == (Options::node(OPV_COMMON_ADVANCED).value().toBool()?OPV_ROSTER_MOOD_SHOW:OPV_ROSTER_VIEWMODE))
	{
		if (FRostersViewPlugin && FRostersModel)
		{
			if (ANode.path() == OPV_ROSTER_MOOD_SHOW?ANode.value().toBool():ANode.value().toInt() == IRostersView::ViewFull)
			{
				QMultiMap<int,QVariant> findData;
				for(QList<int>::const_iterator it=FRosterIndexKinds.constBegin(); it!=FRosterIndexKinds.constEnd(); it++)
					findData.insert(RDR_KIND, *it);
				QList<IRosterIndex *> indexes = FRostersModel->rootIndex()->findChilds(findData, true);
				for (QList<IRosterIndex *>::const_iterator it=indexes.constBegin(); it!=indexes.constEnd(); it++)
					FRostersViewPlugin->rostersView()->insertLabel(FRosterLabelId, *it);
			}
			else
				FRostersViewPlugin->rostersView()->removeLabel(FRosterLabelId);
		}
	}
}

void Mood::onChatWindowCreated(IMessageChatWindow *AWindow)
{
	updateChatWindowActions(AWindow);
	updateChatWindowInfo(AWindow);
    updateChatWindow(AWindow);
    connect(AWindow->instance(), SIGNAL(tabPageActivated()), SLOT(onWindowActivated()));
    connect(AWindow->address()->instance(), SIGNAL(addressChanged(Jid, Jid)), SLOT(onAddressChanged(Jid,Jid)));
}

void Mood::onNormalWindowCreated(IMessageNormalWindow *AWindow)
{
    if(AWindow->mode()==IMessageNormalWindow::WriteMode && isSupported(AWindow->streamJid(), AWindow->contactJid()))
    {
        Action *action = new Action();
        action->setText(tr("Add mood"));
        action->setIcon(RSR_STORAGE_MOOD, MNI_MOOD);
        action->setData(ADR_MESSAGE_TYPE, MT_NORMAL);
        action->setShortcutId(SCT_MESSAGEWINDOWS_SETMOOD);
        action->setData(ADR_CONTACT_JID, AWindow->contactJid().full());
		action->setData(ADR_STREAM_JIDS, AWindow->streamJid().full());
        connect(action, SIGNAL(triggered(bool)), SLOT(onAddMood(bool)));//----
        AWindow->toolBarWidget()->toolBarChanger()->insertAction(action,TBG_MWTBW_MOOD_VIEW);
    }
}

void Mood::onAddMood(bool)
{
    Action *action = qobject_cast<Action *>(sender());
    if (action)
    {
		Jid streamJid = action->data(ADR_STREAM_JIDS).toString();
        Jid contactJid = action->data(ADR_CONTACT_JID).toString();
        int messageType = action->data(ADR_MESSAGE_TYPE).toInt();

        MoodData moodData = FCurrentChatMood.value(streamJid).value(contactJid.bare());

        if (FCurrentChatMood.contains(streamJid))
            if (FCurrentChatMood[streamJid].contains(contactJid.bare()))
                moodData = FCurrentChatMood[streamJid][contactJid.bare()];

        MoodSelect *moodDialog= new MoodSelect(this, FMoodList, FMoodTextLists, FMoodKeys, moodData);
        if(moodDialog->exec())
        {
            moodData = moodDialog->moodData();

            if (moodData.isEmpty())
            {
                if (FCurrentChatMood.contains(streamJid))
                {
                    FCurrentChatMood[streamJid].remove(contactJid.bare());
                    if (FCurrentChatMood[streamJid].isEmpty())
                        FCurrentChatMood.remove(streamJid);
                }
            }
            else
            {
                FCurrentChatMood[streamJid].insert(contactJid.bare(), moodData);
                saveComments(moodData);
            }

            action->setIcon(RSR_STORAGE_MOOD, moodData.isEmpty()?MNI_MOOD:moodData.name);

            if(messageType==MT_CHAT)
            {
                FMoodChat[streamJid].insert(contactJid, moodData);
                FMoodChatActions[streamJid].insert(contactJid, action);
            }
            else if(messageType==MT_NORMAL)
            {
                FMoodMess[streamJid].insert(contactJid, moodData);
                FMoodMessActions[streamJid].insert(contactJid, action);
            }
        }
        moodDialog->deleteLater();
    }
}

QString Mood::currentItemId(const Jid &AStreamJid) const
{
    return FIdHash.contains(AStreamJid.bare())?FIdHash[AStreamJid.bare()]:"current";
}

void Mood::onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore)
{
	Q_UNUSED(AContactBefore)
	Q_UNUSED(AStreamBefore)

    IMessageAddress *address=qobject_cast<IMessageAddress *>(sender());
    IMessageChatWindow *window=FMessageWidgets->findChatWindow(address->streamJid(), address->contactJid());
    if (window)
		updateChatWindowActions(window);
}

void Mood::onWindowActivated()
{
    IMessageChatWindow *window = qobject_cast<IMessageChatWindow *>(sender());
    if (window)
        removeNotifiedMessages(window);
}

void Mood::onNotificationActivated(int ANotifyId)
{
    for (QHash<Jid, QHash<Jid, int> >::const_iterator its=FNotifies.constBegin(); its!=FNotifies.constEnd(); its++)
        for (QHash<Jid, int>::const_iterator itc=(*its).constBegin(); itc!=(*its).constEnd(); itc++)
            if (itc.value()==ANotifyId) // Notification found! Activate window!
            {
                Jid contactJid=itc.key().full();

                IPresenceItem pitem = presenceItemForBareJid(its.key(), itc.key());
                if (!pitem.itemJid.isEmpty())
                    contactJid=pitem.itemJid;

                IMessageChatWindow *window=FMessageWidgets->findChatWindow(its.key(), contactJid);
                if (!window)
                {
                    FMessageProcessor->createMessageWindow(its.key(), contactJid, Message::Chat, IMessageHandler::SM_ASSIGN);
                    window = FMessageWidgets->findChatWindow(its.key(), contactJid);
                }
                if (window)
                {                    
                    window->showTabPage();
                    return;
                }
            }
}

void Mood::writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{
    Q_UNUSED(ALang);
    if (AOrder == MWO_MOOD)
    {
        QDomElement mood=AMessage.stanza().firstElement(TAG_NAME, NS_PEP_MOOD);
        if(!mood.isNull())
        {
            QString icon = mood.firstChild().toElement().tagName();
            QString comment = mood.firstChildElement("text").text();
			QUrl uri=QUrl::fromLocalFile(FIconStorage->fileFullName(icon));
            QString comText = QString(" (%1)").arg(comment);

            QTextImageFormat imageFormat;
			imageFormat.setName(uri.toString());
			imageFormat.setToolTip(FMoodKeys.value(icon));

            QTextCursor cursor(ADocument);
            QTextCharFormat format = cursor.charFormat();
            format.setFontItalic(true);
            format.setFontFamily("Courier");
            format.setFontPointSize(10);
            format.setForeground(Qt::blue);

            cursor.movePosition(QTextCursor::End);
            cursor.insertBlock();
            cursor.insertImage(imageFormat);
            if(!comment.isEmpty())
                cursor.insertText(comText,format);
        }
    }
}

void Mood::writeTextToMessage(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{
    Q_UNUSED(AOrder); Q_UNUSED(AMessage); Q_UNUSED(ADocument); Q_UNUSED(ALang);
}

bool Mood::messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection)
{
    if (AOrder==MEO_MOOD && ADirection==IMessageProcessor::DirectionOut)
    {
        Jid contactJid = AMessage.to();
        MoodElement *moodElement;
        if(AMessage.type()==Message::Chat)
        {
            moodElement = &FMoodChat[AStreamJid];
            MoodData moodData = moodElement->value(contactJid);
            if(!moodData.isEmpty())
            {
                setMoodForMessage(moodData,AMessage);
                moodElement->remove(contactJid);

                Action *act =FMoodChatActions[AStreamJid].value(contactJid);
                if(act)
                    act->setIcon(RSR_STORAGE_MOOD, MNI_MOOD);
            }
        }
        else if(AMessage.type()==Message::Normal)
        {
            moodElement = &FMoodMess[AStreamJid];
            MoodData moodData = moodElement->value(contactJid);
            if(!moodData.isEmpty())
            {
                setMoodForMessage(moodData,AMessage);
                moodElement->remove(contactJid);
            }
        }
    }
    return false;
}


void Mood::setMoodForMessage(const MoodData &AMoodData, Message &AMessage)
{
    QDomElement mood = AMessage.stanza().addElement(TAG_NAME, NS_PEP_MOOD);
    mood.appendChild(AMessage.stanza().document().createElement(AMoodData.name));
    if(!AMoodData.text.isEmpty())
    {
        QDomNode comment = AMessage.stanza().document().createElement("text");
        comment.appendChild(AMessage.stanza().document().createTextNode(AMoodData.text));
        mood.appendChild(comment);
    }
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_moods, Mood)
#endif
