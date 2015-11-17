#include <QDir>
#include <QClipboard>
#include <definitions/shortcuts.h>
#include "definitions/notificationdataroles.h"
#include "definitions/notificationtypeorders.h"
#include "definitions/notificationtypes.h"
#include "definitions/soundfiles.h"
#include <definitions/rosterindexkinds.h>
#include <definitions/rosterindexroles.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/actiongroups.h>
#include <definitions/menuicons.h>
#include <definitions/namespaces.h>
#include <definitions/rosterlabels.h>
#include <definitions/rosterlabelholderorders.h>
#include <definitions/rostertooltiporders.h>
#include <definitions/rosterdataholderorders.h>
#include <definitions/mapobjecttyperole.h>
#include <definitions/resources.h>
#include <utils/options.h>
#include <utils/menu.h>
#include <utils/qt4qt5compat.h>
#include <MapObject>

#include "activityselect.h"

#define ADR_STREAM_JIDS		Action::DR_StreamJid
#define ADR_CLIPBOARD_NAME  Action::DR_Parametr1
#define ADR_CLIPBOARD_TEXT  Action::DR_Parametr2
#define ADR_CLIPBOARD_IMAGE Action::DR_Parametr3
#define NO_ACTIVITY			"no_activity"
#define ACTIVITIES_DEF		"activities.def.xml"
#define TAG_NAME			"activity"
#define MDR_ACTIVITY_ICON   1000

ActivityData::ActivityData(QString ANameBasic, QString ANameDetailed, QString AText):nameBasic(ANameBasic==NO_ACTIVITY?QString():ANameBasic),nameDetailed(ANameDetailed),text(AText)
{}

ActivityData::ActivityData():nameBasic(),nameDetailed(),text()
{}

QString ActivityData::iconFileName() const
{
	return nameBasic.isEmpty()?NO_ACTIVITY:
		   nameDetailed.isEmpty()?nameBasic:
		  (nameDetailed=="cycling" && nameBasic=="traveling")?"cycling2":
		   nameDetailed;
}

void ActivityData::clear()
{
	nameBasic=QString();
	nameDetailed=QString();
	text=QString();
}

Activity::Activity():
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
	FMapContacts(NULL),
	FNotifications(NULL),
	FIconStorage(NULL),
	FSimpleContactsView(false),
	FRosterLabelId(-1),	
	FRosterIndexKinds(QList<int>() << RIK_CONTACT << RIK_METACONTACT << RIK_METACONTACT_ITEM << RIK_RECENT_ITEM << RIK_MY_RESOURCE << RIK_STREAM_ROOT)
{
	if(false)
	{
		tr("Doing chores");    tr("Drinking");         tr("Eating");        tr("Exercising");
		tr("Grooming");        tr("Having appointment");tr("Inactive");     tr("Relaxing");
		tr("Talking");         tr("Traveling");        tr("Working");       tr("Buying groceries");
		tr("Cleaning");        tr("Cooking");          tr("Doing maintenance"); tr("Writing");
		tr("Doing the dishes");tr("Doing the laundry");tr("Gardening");     tr("Running an errand");
		tr("Walking the dog"); tr("Having a beer");    tr("Having coffee"); tr("Having tea");
		tr("Having a snack");  tr("Having breakfast"); tr("Having dinner"); tr("Having lunch");
		tr("Cycling");         tr("Dancing");          tr("Hiking");        tr("Jogging");
		tr("Playing sports");  tr("Running");          tr("Skiing");        tr("Swimming");
		tr("Working out");     tr("At the spa");       tr("Brushing teeth");tr("Getting a haircut");
		tr("Shaving");         tr("Taking a bath");    tr("Taking a shower");tr("Day off");
		tr("Hanging out");     tr("Hiding");           tr("On vacation");   tr("Praying");
		tr("Scheduled holiday");tr("Sleeping");        tr("Thinking");      tr("Fishing");
		tr("Gaming");          tr("Going out");        tr("Partying");      tr("Reading");
		tr("Rehearsing");      tr("Shopping");         tr("Smoking");       tr("Socializing");
		tr("Sunbathing");      tr("Watching TV");      tr("Watching a movie");tr("In real life");
		tr("On the phone");    tr("On video phone");   tr("Commuting");     tr("Travelling by bicycle");
		tr("Driving");         tr("In a car");         tr("On a bus");      tr("On a plane");
		tr("On a train");      tr("On a trip");        tr("Walking");       tr("Coding");
		tr("In a meeting");    tr("Studying");         tr("No activity");
	}
}

Activity::~Activity()
{}

void Activity::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("User Activity");
	APluginInfo->description = tr("Implements XEP-0108: User Activity");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(PEPMANAGER_UUID);
}

bool Activity::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	IPlugin *plugin = APluginManager->pluginInterface("IPEPManager").value(0,NULL);
	if (plugin)
	{
		FPEPManager = qobject_cast<IPEPManager *>(plugin->instance());
		FPEPManager->insertNodeHandler(QString(NS_PEP_ACTIVITY), this);
	} else
		return false;

	//-----------------------
	plugin = APluginManager->pluginInterface("IMessageWidgets").value(0);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
		connect(FMessageWidgets->instance(),SIGNAL(chatWindowCreated(IMessageChatWindow *)),SLOT(onChatWindowCreated(IMessageChatWindow *)), Qt::QueuedConnection);
	}

	plugin = APluginManager->pluginInterface("IMessageProcessor").value(0);
	if (plugin)
		FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());

	plugin = APluginManager->pluginInterface("INotifications").value(0);
	if (plugin)
	{
		FNotifications = qobject_cast<INotifications *>(plugin->instance());
		connect(FNotifications->instance(), SIGNAL(notificationActivated(int)), SLOT(onNotificationActivated(int)));
	}

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0,NULL);
	if (plugin)
		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IMessageStyleManager").value(0,NULL);
	if (plugin)
		FMessageStyleManager = qobject_cast<IMessageStyleManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0,NULL);
	if (plugin)
	{
		FXmppStreamManager = qobject_cast<IXmppStreamManager *>(plugin->instance());
		connect(FXmppStreamManager->instance(),SIGNAL(streamOpened(IXmppStream *)),SLOT(onStreamOpened(IXmppStream *)));
		connect(FXmppStreamManager->instance(),SIGNAL(streamClosed(IXmppStream *)),SLOT(onStreamClosed(IXmppStream *)));
	}

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IRostersModel").value(0,NULL);
	if (plugin)
	{
		FRostersModel = qobject_cast<IRostersModel *>(plugin->instance());
		if (FRostersModel)
			connect(FRostersModel->instance(),SIGNAL(indexInserted(IRosterIndex *)), SLOT(onRosterIndexInserted(IRosterIndex *)));
	}

	plugin = APluginManager->pluginInterface("IMap").value(0,NULL);
	if (plugin)
		FMap = qobject_cast<IMap *>(plugin->instance());

	// This one should be replaced with IMapContacts later
	plugin = APluginManager->pluginInterface("IMapContacts").value(0,NULL);
	if (plugin)
		FMapContacts = qobject_cast<IMapContacts *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IRostersViewPlugin").value(0,NULL);
	if (plugin)
	{
		FRostersViewPlugin = qobject_cast<IRostersViewPlugin *>(plugin->instance());
		if (FRostersViewPlugin)
		{
			connect(FRostersViewPlugin->rostersView()->instance(),SIGNAL(indexContextMenu(QList<IRosterIndex *>, quint32, Menu *)),
					SLOT(onRosterIndexContextMenu(QList<IRosterIndex *>, quint32, Menu *)));
			connect(FRostersViewPlugin->rostersView()->instance(),SIGNAL(indexClipboardMenu(QList<IRosterIndex *>, quint32, Menu *)),
					SLOT(onRosterIndexClipboardMenu(QList<IRosterIndex *>, quint32, Menu *)));
			connect(FRostersViewPlugin->rostersView()->instance(),
					SIGNAL(indexToolTips(IRosterIndex *, quint32, QMap<int,QString> &)),
					SLOT(onRosterIndexToolTips(IRosterIndex *, quint32, QMap<int,QString> &)));
		}
	}

	AInitOrder = 300;   // This one should be initialized AFTER Map Contacts!
	return true;
}

bool Activity::initObjects()
{
	Shortcuts::declareShortcut(SCT_APP_SETACTIVITY, tr("Set activity"), tr("Ctrl+F6", "Set activity (for all accounts)"), Shortcuts::ApplicationShortcut);
	Shortcuts::declareShortcut(SCT_ROSTERVIEW_SETACTIVITY, tr("Set activity"), tr("F6", "Set activity (for an account)"), Shortcuts::WidgetShortcut);

	FIconStorage = IconStorage::staticStorage(RSR_STORAGE_ACTIVITY);

	loadActivityList();

	if (FDiscovery)
		registerDiscoFeatures();

	if (FRostersViewPlugin)
	{
		AdvancedDelegateItem label(RLID_ACTIVITY);
		label.d->kind = AdvancedDelegateItem::CustomData;
		label.d->data = RDR_ACTIVITY_IMAGE;
		FRosterLabelId = FRostersViewPlugin->rostersView()->registerLabel(label);
		FRostersViewPlugin->rostersView()->insertLabelHolder(RLHO_ACTIVITY, this);
		Shortcuts::insertWidgetShortcut(SCT_ROSTERVIEW_SETACTIVITY, FRostersViewPlugin->rostersView()->instance());
		connect(Shortcuts::instance(), SIGNAL(shortcutActivated(QString,QWidget*)), SLOT(onShortcutActivated(QString,QWidget*)));
	}

	if (FRostersModel)
		FRostersModel->insertRosterDataHolder(RDHO_ACTIVITY, this);

	if (FMap)
	{
		FMap->geoMap()->registerDataType(MDR_ACTIVITY_ICON, MOT_CONTACT, 210, MOP_RIGHT_TOP);
		FMap->geoMap()->addDataHolder(MOT_CONTACT, this);
	}

	if (FNotifications)
	{
		INotificationType notifyType;
		notifyType.order = NTO_ACTIVITY_CHANGE;
		if (FIconStorage)
			notifyType.icon = FIconStorage->getIcon(MNI_ACTIVITY);
		notifyType.title = tr("When user activity changed");
		notifyType.kindMask = INotification::PopupWindow|INotification::SoundPlay;
		notifyType.kindDefs = notifyType.kindMask;
		FNotifications->registerNotificationType(NNT_ACTIVITY, notifyType);
	}

	Action *action = FPEPManager->addAction(AG_ACTIVITY);
	action->setText(tr("Activity"));
	action->setIcon(RSR_STORAGE_ACTIVITY, MNI_ACTIVITY);
	action->setShortcutId(SCT_APP_SETACTIVITY);
	connect(action,SIGNAL(triggered(bool)),this,SLOT(menuActivity()));

	return true;
}

bool Activity::initSettings()
{
	Options::setDefaultValue(OPV_ROSTER_ACTIVITY_SHOW, true);
	Options::setDefaultValue(OPV_MESSAGES_ACTIVITY_DISPLAY, true);
	Options::setDefaultValue(OPV_MESSAGES_ACTIVITY_NOTIFY, true);
	if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);
	return true;
}

void Activity::menuActivity()
{
	ActivitySelect *activitySelect = new ActivitySelect(this, FActivityList, FActivityTextLists, FCurrentActivity);
	if(activitySelect->exec())
	{
		ActivityData activityData = activitySelect->activityData();
		saveComments(activityData);
		for (QSet<Jid>::ConstIterator it = FStreamsOnline.begin(); it != FStreamsOnline.end(); ++it)
			if (FPEPManager->isSupported(*it))
				sendActivity(activityData, *it);
	}
	activitySelect->deleteLater();
}

void Activity::onWindowActivated()
{
	IMessageChatWindow *window = qobject_cast<IMessageChatWindow *>(sender());
	if (window)
		removeNotifiedMessages(window);
}

void Activity::onNotificationActivated(int ANotifyId)
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

//-------------------
bool Activity::processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza)
{
	if (AStanza.type()!="error")
	{
		Jid contactJid = AStanza.from();
		QDomElement event  = AStanza.firstElement("event", NS_PUBSUB_EVENT);
		QDomElement items  = event.firstChildElement("items");
		if(!items.isNull())
		{
			bool stop=false;
			QDomElement item  = items.firstChildElement("item");
			if(!item.isNull())
			{
				QDomElement activity = item.firstChildElement(TAG_NAME);
				if(!activity.isNull())
				{
					if(contactJid.bare() == AStreamJid.bare())
						FIdHash.insert("AStreamJid.bare()", item.attribute("id"));

					ActivityData activityData;
					for (QDomElement e = activity.firstChildElement(); !e.isNull(); e=e.nextSiblingElement())
						if (e.tagName() == "text")
							activityData.text = e.text();
						else
						{
							activityData.nameBasic = e.tagName();
							QDomElement detailed = e.firstChildElement();
							if (!detailed.isNull())
								activityData.nameDetailed = detailed.tagName();
						}

					if (activityData.isEmpty())
						stop=true;
					else
					{
						if (activityData!=(FActivityHash.value(contactJid.bare())))
						{
							FActivityHash.insert(contactJid.bare(), activityData);
							updateChatWindows(contactJid, AStreamJid);
							updateDataHolder(contactJid);
							displayNotification(AStreamJid, contactJid);
							if(contactJid.bare() == AStreamJid.bare())
								FCurrentActivity = activityData;
						}
						return true;
					}
				}
			}

			if(!stop && event.firstChild().firstChild().nodeName() == "retract")
			{
				if(contactJid.bare() == AStreamJid.bare())
					FIdHash.remove(AStreamJid.bare());
				stop=true;
			}

			if (stop)
			{
				if (contactJid.bare() == AStreamJid.bare())
					FCurrentActivity.clear();
				FActivityHash.remove(contactJid.bare());
				updateChatWindows(contactJid, AStreamJid);
				updateDataHolder(contactJid);
				return true;
			}
		}
	}
	return false;
}

void Activity::registerDiscoFeatures()
{
	IDiscoFeature dfeature;
	dfeature.var = NS_PEP_ACTIVITY;
	dfeature.active = true;
	dfeature.icon = FIconStorage->getIcon(MNI_ACTIVITY);
	dfeature.name = tr("User Activity");
	dfeature.description = tr("Supports publishing of current user activity");
	FDiscovery->insertDiscoFeature(dfeature);

	dfeature.var.append(NODE_NOTIFY_SUFFIX);
	dfeature.name = tr("User Activity Notification");
	dfeature.description = tr("Receives notification of current user activity");
	FDiscovery->insertDiscoFeature(dfeature);
}

bool Activity::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
	return FDiscovery==NULL||!FDiscovery->hasDiscoInfo(AStreamJid,AContactJid)||
		   FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_PEP_ACTIVITY);
}

void Activity::sendActivity(const ActivityData &AActivityData, const Jid &AStreamJid)
{
	QDomDocument doc;
	QDomElement item=doc.createElement("item");

	item.setAttribute("id", currentItemId(AStreamJid));
	if(AActivityData.isEmpty())
	{
		if (Options::node(OPV_PEP_DELETE_PUBLISHEMPTY).value().toBool())
		{
			item.appendChild(doc.createElementNS(NS_PEP_ACTIVITY, TAG_NAME));
			FPEPManager->publishItem(AStreamJid, NS_PEP_ACTIVITY, item);
		}
		if (Options::node(OPV_PEP_DELETE_RETRACT).value().toBool())
			FPEPManager->deleteItem(AStreamJid, NS_PEP_ACTIVITY, item);
	}
	else
	{
		QDomElement activityNode=doc.createElementNS(NS_PEP_ACTIVITY, TAG_NAME);
		item.appendChild(activityNode);
		QDomElement activityNameBasic=doc.createElement(AActivityData.nameBasic);
		activityNode.appendChild(activityNameBasic);
		if(!AActivityData.nameDetailed.isEmpty())
			activityNameBasic.appendChild(doc.createElement(AActivityData.nameDetailed));

		if(!AActivityData.text.isEmpty())
		{
			QDomElement activityText=doc.createElement("text");
			activityText.appendChild(doc.createTextNode(AActivityData.text));
			activityNode.appendChild(activityText);
		}
		FPEPManager->publishItem(AStreamJid, NS_PEP_ACTIVITY, item);
	}
}

QList<quint32> Activity::rosterLabels(int AOrder, const IRosterIndex *AIndex) const
{
	QList<quint32> labels;
	if (AOrder==RLHO_ACTIVITY && AIndex->kind()==RIK_RECENT_ITEM)
		if (FSimpleContactsView)
			labels.append(RLID_ACTIVITY);
	return labels;
}

AdvancedDelegateItem Activity::rosterLabel(int AOrder, quint32 ALabelId, const IRosterIndex *AIndex) const
{
	Q_UNUSED(AOrder); Q_UNUSED(ALabelId); Q_UNUSED(AIndex);
	static const AdvancedDelegateItem null = AdvancedDelegateItem();
	return null;
}

QList<int> Activity::rosterDataRoles(int AOrder) const
{
	if (AOrder==RDHO_ACTIVITY)
		return QList<int>() << RDR_ACTIVITY_IMAGE;
	return QList<int>();
}

QVariant Activity::rosterData(int AOrder, const IRosterIndex *AIndex, int ARole) const
{
	Q_UNUSED(AOrder);
	if (ARole == RDR_ACTIVITY_IMAGE)
	{
		Jid jid(AIndex->data(RDR_FULL_JID).toString());
		if (FActivityHash.contains(jid.bare()))
			return getIcon(FActivityHash[jid.bare()]);
	}
	return QVariant();
}

bool Activity::setRosterData(int AOrder, const QVariant &AValue, IRosterIndex *AIndex, int ARole)
{
	Q_UNUSED(AOrder);
	Q_UNUSED(AIndex);
	Q_UNUSED(ARole);
	Q_UNUSED(AValue);
	return false;
}

QMultiMap<int, IOptionsDialogWidget *> Activity::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (FOptionsManager)
	{
		if (ANodeId == OPN_ROSTERVIEW)
		{
			if (Options::node(OPV_COMMON_ADVANCED).value().toBool())
				widgets.insertMulti(OWO_ROSTER_PEP_ACTIVITY, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ROSTER_ACTIVITY_SHOW), tr("Show user activity icons"), AParent));
		}
		else if (ANodeId == OPN_MESSAGES)
		{
			widgets.insertMulti(OWO_MESSAGES_INFOBAR_ACTIVITY, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MESSAGES_ACTIVITY_DISPLAY), tr("Display user activity icon"), AParent));
			widgets.insertMulti(OWO_MESSAGES_PEP_ACTIVITY, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MESSAGES_ACTIVITY_NOTIFY), tr("Activity events in chat"), AParent));
		}
	}
	return widgets;
}

//----SLOTS----

void Activity::onStreamOpened(IXmppStream *AXmppStream)
{
	FStreamsOnline.insert(AXmppStream->streamJid());
}

void Activity::onStreamClosed(IXmppStream *AXmppStream)
{
	FStreamsOnline.remove(AXmppStream->streamJid());
}

void Activity::onRosterIndexInserted(IRosterIndex *AIndex)
{
	if (FRostersViewPlugin && FRosterIndexKinds.contains(AIndex->kind()) &&
		(Options::node(OPV_COMMON_ADVANCED).value().toBool()?Options::node(OPV_ROSTER_ACTIVITY_SHOW).value().toBool():Options::node(OPV_ROSTER_VIEWMODE).value().toInt()==IRostersView::ViewFull))
			FRostersViewPlugin->rostersView()->insertLabel(FRosterLabelId, AIndex);
}

void Activity::onRosterIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
	QStringList streamJids;
	if (ALabelId == AdvancedDelegateItem::DisplayId || ALabelId == FRosterLabelId)
		for (QList<IRosterIndex *>::const_iterator it=AIndexes.constBegin(); it!=AIndexes.constEnd(); it++)
		{
			if ((*it)->kind() == RIK_STREAM_ROOT)
			{
				Jid streamJid((*it)->data(RDR_STREAM_JID).toString());
				if (FStreamsOnline.contains(streamJid) && FPEPManager->isSupported(streamJid))
					streamJids.append((*it)->data(RDR_STREAM_JID).toString());
			}
		}

//	Jid streamJid = (*it)->data(RDR_STREAM_JID).toString();
	if (!streamJids.isEmpty())
	{
		Action *action = new Action(AMenu);
		action->setText(tr("Activity"));
		action->setIcon(RSR_STORAGE_ACTIVITY, MNI_ACTIVITY);
		action->setData(ADR_STREAM_JIDS, streamJids);
		connect(action, SIGNAL(triggered(bool)), SLOT(onSetActivityByAction(bool)));
		AMenu->addAction(action, AG_RVCM_ACTIVITY, true);
	}
}

void Activity::onRosterIndexClipboardMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
	if (ALabelId == AdvancedDelegateItem::DisplayId || ALabelId == FRosterLabelId)
		for (QList<IRosterIndex *>::const_iterator it=AIndexes.constBegin(); it!=AIndexes.constEnd(); it++)
		{
			Jid jid((*it)->data(RDR_FULL_JID).toString());
			if (FActivityHash.contains(jid.bare()))
			{
				Action *action = new Action(AMenu);
				ActivityData data = FActivityHash[jid.bare()];
				if (!data.text.isEmpty())
				{
					action->setText(data.text);
					action->setData(ADR_CLIPBOARD_TEXT, data.text);
				}
				action->setIcon(getIcon(jid));
				QString fileName = getIconFileName(jid);
				if (!fileName.isEmpty())
					action->setData(ADR_CLIPBOARD_IMAGE, fileName);
				action->setData(ADR_CLIPBOARD_NAME, data.nameDetailed.isEmpty()?data.nameBasic:tr("%1: %2").arg(data.nameBasic).arg(data.nameDetailed));
				connect(action, SIGNAL(triggered()), SLOT(onCopyToClipboard()));
				AMenu->addAction(action, AG_RVCBM_PEP, true);
			}
		}
}

void Activity::onRosterIndexToolTips(IRosterIndex *AIndex, quint32 ALabelId, QMap<int, QString> &AToolTips)
{
	if ((ALabelId == AdvancedDelegateItem::DisplayId || ALabelId == FRosterLabelId) && FRosterIndexKinds.contains(AIndex->kind()))
	{
		 QString label=getLabel(AIndex->data(RDR_FULL_JID).toString());
		 if (!label.isEmpty())
			 AToolTips.insert(RTTO_ACTIVITY, label);
	}
}

void Activity::onCopyToClipboard()
{
	QString text=qobject_cast<Action *>(sender())->data(ADR_CLIPBOARD_TEXT).toString();
	QString name=qobject_cast<Action *>(sender())->data(ADR_CLIPBOARD_NAME).toString();
	QString fileName = qobject_cast<Action *>(sender())->data(ADR_CLIPBOARD_IMAGE).toString();
	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();

	mime->setText(text.isEmpty()?name:QString("%1 (%2)").arg(name).arg(text));

	if (!fileName.isEmpty())
	{
		QFile file(fileName);
		if (file.open(QIODevice::ReadOnly))
		{
			QByteArray format(QImageReader::imageFormat(&file));
			QByteArray data(file.readAll());
			if (!data.isEmpty())
			{
				QImage image = QImage::fromData(data);
				if (!image.isNull())
				{
					mime->setImageData(image);
					QUrl url;
					url.setScheme("data");
					url.setPath(QString("image/%1;base64,%2").arg(QString::fromLatin1(format)).arg(QString::fromLatin1(data.toBase64())));
					mime->setHtml(QString("<body><img src=\"%1\" alt=\"%2\" title=\"%2\" /> %3</body>").arg(url.toString()).arg(FTranslatedNames.value(name)).arg(text));
				}
			}
		}
	}

	clipboard->setMimeData(mime);
}

void Activity::onSetActivityByAction(bool)
{
	QStringList streamJids = qobject_cast<Action *>(sender())->data(ADR_STREAM_JIDS).toStringList();
	for (QStringList::ConstIterator it = streamJids.constBegin(); it != streamJids.constEnd(); ++it)
		setActivityForAccount(*it);
}

void Activity::setActivityForAccount(Jid AStreamJid)
{
	ActivityData data = FActivityHash.value(AStreamJid.bare());
	ActivitySelect *activitySelect = new ActivitySelect(this, FActivityList, FActivityTextLists, data);
	if(activitySelect->exec())
	{
		data = activitySelect->activityData();
		sendActivity(data, AStreamJid);
		saveComments(data);
	}
	activitySelect->deleteLater();
}

void Activity::onShortcutActivated(const QString &AString, QWidget *AWidget)
{
	Q_UNUSED(AWidget)

	if (AString==SCT_ROSTERVIEW_SETACTIVITY)
	{
		QList<IRosterIndex *>indexes=FRostersViewPlugin->rostersView()->selectedRosterIndexes();
		for (QList<IRosterIndex *>::const_iterator it=indexes.constBegin(); it!=indexes.constEnd(); it++)
			if ((*it)->kind()==RIK_STREAM_ROOT)
			{
				Jid streamJid((*it)->data(RDR_STREAM_JID).toString());
				if (FStreamsOnline.contains(streamJid) && FPEPManager->isSupported(streamJid))
					setActivityForAccount(streamJid);
			}
	}
}

void Activity::updateChatWindows(bool AInfoBar)
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

void Activity::updateChatWindows(const Jid &AContactJid, const Jid &AStreamJid)
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

void Activity::updateChatWindow(IMessageChatWindow *AMessageChatWindow)
{
	if (Options::node(OPV_MESSAGES_ACTIVITY_NOTIFY).value().toBool() && FActivityHash.contains(AMessageChatWindow->contactJid().bare()))
	{
		ActivityData activity=FActivityHash[AMessageChatWindow->contactJid().bare()];
		IMessageStyleContentOptions options;
		options.time = QDateTime::currentDateTime();
		options.timeFormat = FMessageStyleManager->timeFormat(options.time);
		options.type = IMessageStyleContentOptions::TypeEvent;
		options.kind = IMessageStyleContentOptions::KindStatus;
		options.direction = IMessageStyleContentOptions::DirectionIn;
		options.senderId = AMessageChatWindow->contactJid().full();
		options.senderName   = HTML_ESCAPE(FMessageStyleManager->contactName(AMessageChatWindow->streamJid(), AMessageChatWindow->contactJid()));
		options.senderAvatar = FMessageStyleManager->contactAvatar(AMessageChatWindow->contactJid());

		QString title = FTranslatedNames.value(activity.nameBasic);
		if (!activity.nameDetailed.isEmpty())
			title.append(": ").append(FTranslatedNames.value(activity.nameDetailed));

		QUrl iconUrl = QUrl::fromLocalFile(getIconFileName(activity));
		QString pic = QString("<img src=\"%1\" title=\"%2\" alt=\"%2\" />")
			  .arg(iconUrl.toString()).arg(title);

		if (!activity.text.isEmpty())
			pic.append(" (").append(HTML_ESCAPE(activity.text)).append(")");
		QString longMessage = QString("<i>")
						   .append(tr("%1 changed activity to %2")
						   .arg(AMessageChatWindow->infoWidget()->fieldValue(IMessageInfoWidget::Name).toString())
						   .arg(pic))
						   .append("</i>");

		AMessageChatWindow->viewWidget()->appendHtml(longMessage, options);
	}
}

void Activity::updateChatWindowInfo(IMessageChatWindow *AMessageChatWindow)
{
	if (Options::node(OPV_MESSAGES_ACTIVITY_DISPLAY).value().toBool() && FActivityHash.contains(AMessageChatWindow->contactJid().bare()))
	{
		ActivityData activity=FActivityHash[AMessageChatWindow->contactJid().bare()];
		QLabel *label = NULL;
		QList<QAction *> actions=AMessageChatWindow->infoWidget()->infoToolBarChanger()->groupItems(AG_ACTIVITY);
		if (!actions.isEmpty())
			label = qobject_cast<QLabel*>(AMessageChatWindow->infoWidget()->infoToolBarChanger()->toolBar()->widgetForAction(actions.first()));
		if (!label)
		{
			label = new QLabel(AMessageChatWindow->infoWidget()->infoToolBarChanger()->toolBar());
			label->setMargin(3);
			AMessageChatWindow->infoWidget()->infoToolBarChanger()->insertWidget(label, AG_ACTIVITY);
		}
		label->setPixmap(getIcon(activity).pixmap(16));
		label->setToolTip(getLabel(activity));
	}
	else
	{
		QList<QAction *> actions=AMessageChatWindow->infoWidget()->infoToolBarChanger()->groupItems(AG_ACTIVITY);
		if (!actions.isEmpty())
		{
			AMessageChatWindow->infoWidget()->infoToolBarChanger()->handleWidget(actions.first())->deleteLater();
			AMessageChatWindow->infoWidget()->infoToolBarChanger()->removeItem(actions.first());
		}
	}
}

void Activity::loadActivityList()
{
	QDir dir(FIconStorage->resourcesDirs().first());
	dir.cd(FIconStorage->storage());
	dir.cd(FIconStorage->subStorage());

	QFile file(dir.absoluteFilePath(ACTIVITIES_DEF));
	if(file.open(QFile::ReadOnly))
	{
		QDomDocument doc;
		if (doc.setContent(file.readAll(), false))
			for (QDomElement e = doc.documentElement().firstChildElement("icon");
				 !e.isNull();
				 e = e.nextSiblingElement("icon"))
			{
				QString key = e.firstChildElement("key").text();    //--icon--
				QString txt = e.firstChildElement("text").text();   //--dir---
				FTranslatedNames.insert(key, tr(e.firstChildElement("name").text().toUtf8().data()));
				if (txt.isEmpty())
				{
					if (!FActivityList.contains(key))
						FActivityList.insert(key, QStringList());
				}
				else
				{
					if (key=="cycling2")
						key="cycling";
					FActivityList[txt].append(key);
				}
			}
		file.close();
	}
}

QIcon Activity::getIcon(const QString &AName) const
{
	return FIconStorage->getIcon(AName);
}

QIcon Activity::getIcon(const Jid &AContactJid) const
{
	return  FActivityHash.contains(AContactJid.bare())?getIcon(FActivityHash.value(AContactJid.bare())):QIcon();
}

QIcon Activity::getIcon(const ActivityData &AActivity) const
{
	return getIcon(AActivity.iconFileName());
}

QString Activity::getIconFileName(const ActivityData &AActivity) const
{
	return getIconFileName(AActivity.nameDetailed.isEmpty()?AActivity.nameBasic:AActivity.nameDetailed);
}

QString Activity::getIconFileName(const QString &AActivityName) const
{
	return FIconStorage->fileFullName(AActivityName);
}

QString Activity::getIconFileName(const Jid &AContactJid) const
{
	return  FActivityHash.contains(AContactJid.bare())?getIconFileName(FActivityHash.value(AContactJid.bare())):QString();
}

QString Activity::getIconName(const Jid &AContactJid) const
{
	ActivityData a=FActivityHash.value(AContactJid.bare());
	return  getIconFileName(a.nameDetailed.isEmpty()?a.nameBasic:a.nameDetailed);
}

QString Activity::getText(const Jid &AContactJid) const
{
	return  FActivityHash.value(AContactJid.bare()).text;
}

QString Activity::getLabel(const Jid &AContactJid) const
{
	return FActivityHash.contains(AContactJid.bare())?getLabel(FActivityHash[AContactJid.bare()])
													 :QString();

}

QString Activity::getLabel(const ActivityData &AActivityData) const
{
	QString label=QString("<strong>%1:</strong> %2").arg(tr("Activity")).arg(FTranslatedNames[AActivityData.nameBasic]);
	if (!AActivityData.nameDetailed.isEmpty())
		 label.append(": ")
			  .append(FTranslatedNames[AActivityData.nameDetailed]);
	if (!AActivityData.text.isEmpty())
		label.append(QString(" (<i>%1)</i>").arg(AActivityData.text));
	return label;
}

void Activity::displayNotification(const Jid &AStreamJid, const Jid &AContactJid)
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
	notify.kinds = FNotifications->enabledTypeNotificationKinds(NNT_ACTIVITY);

	if (FMessageWidgets)
	{
		IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
		if (window && window->isActiveTabPage())    // The window is existing and is an active tab page!
			notify.kinds = 0;                       // So, do not have to notify!
	}

	if (notify.kinds)
	{
		ActivityData activityData=FActivityHash.value(AContactJid.bare());
		QString activity=FTranslatedNames.value(activityData.nameBasic);
		if (!activityData.nameDetailed.isEmpty())
			activity.append(": ").append(FTranslatedNames.value(activityData.nameDetailed));
		QString html=tr("Changed activity to: %1").arg(QString("<b>").append(activity).append("</b>"));
		if (!activityData.text.isEmpty())
			html.append(QString("<br><i>%1</i>").arg(activityData.text));

		notify.typeId = NNT_ACTIVITY;
		notify.data.insert(NDR_STREAM_JID, AStreamJid.full());
		notify.data.insert(NDR_CONTACT_JID, AContactJid.full());
		notify.data.insert(NDR_ICON, getIcon(activityData));
		notify.data.insert(NDR_POPUP_HTML, html);
		notify.data.insert(NDR_POPUP_TITLE, FNotifications->contactName(AStreamJid, AContactJid));
		notify.data.insert(NDR_POPUP_IMAGE, FNotifications->contactAvatar(AContactJid));
		notify.data.insert(NDR_SOUND_FILE, SDF_PEP_EVENT);
		if (FNotifies[AStreamJid].contains(AContactJid))
			FNotifications->removeNotification(FNotifies[AStreamJid].value(AContactJid));
		FNotifies[AStreamJid].insert(AContactJid, FNotifications->appendNotification(notify));
	}
}

void Activity::removeNotifiedMessages(IMessageChatWindow *AWindow)
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

IPresenceItem Activity::presenceItemForBareJid(const Jid &AStreamJid, const Jid &AContactJid) const
{
	IPresence *presence = FPresenceManager ? FPresenceManager->findPresence(AStreamJid) : NULL;
	return presence ? FPresenceManager->sortPresenceItems(presence->findItems(AContactJid)).value(0) : IPresenceItem();
}

QString Activity::currentItemId(const Jid &AStreamJid) const
{
	return FIdHash.contains(AStreamJid.bare())?FIdHash[AStreamJid.bare()]:"current";
}

void Activity::onMapObjectInserted(int AType, const QString &AId)
{
	if (AType==MOT_CONTACT)
		if (FActivityHash.contains(Jid(AId).bare()))
			emit mapDataChanged(AType, AId, MDR_ACTIVITY_ICON);
}

void Activity::onMapObjectRemoved(int AType, const QString &AId)
{
	Q_UNUSED(AType)
	Q_UNUSED(AId)
}

void Activity::updateDataHolder(const Jid &AContactJid)
{
	if (FRostersModel)
	{
		QMultiMap<int,QVariant> findData;
		for(QList<int>::const_iterator it=FRosterIndexKinds.constBegin(); it!=FRosterIndexKinds.constEnd(); it++)
			findData.insert(RDR_KIND, *it);
		if (!AContactJid.isEmpty())
			findData.insert(RDR_PREP_BARE_JID, AContactJid.pBare());
		QList<IRosterIndex *> indexes = FRostersModel->rootIndex()->findChilds(findData,true);
		for (QList<IRosterIndex *>::const_iterator it=indexes.constBegin(); it!=indexes.constEnd(); it++)
			emit rosterDataChanged(*it, RDR_ACTIVITY_IMAGE);
	}

	if (FMapContacts)
	{
		QString bareJid=AContactJid.bare();
		QStringList jids=FMapContacts->getFullJidList(bareJid);
		if (!jids.isEmpty())    // There are such contacts on the map!!!
			for (QStringList::const_iterator it=jids.constBegin(); it!=jids.constEnd(); it++)
				emit mapDataChanged(MOT_CONTACT, *it, MDR_ACTIVITY_ICON); // Activity either added or removed!
	}
}

//------------------------------
QGraphicsItem * Activity::mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement)
{
	if (ARole == MDR_ACTIVITY_ICON)
	{
		Jid jid(ASceneObject->mapObject()->id());
		if (FActivityHash.contains(jid.bare()))
		{
			QIcon icon=getIcon(FActivityHash[jid.bare()]);
			if (!icon.isNull())
			{
				if (ACurrentElement)
				{
					QGraphicsPixmapItem *item=qgraphicsitem_cast<QGraphicsPixmapItem *>(ACurrentElement);
					if (item)
						item->setPixmap(icon.pixmap(icon.availableSizes().first()));
					return item;
				}
				else
					return new QGraphicsPixmapItem(icon.pixmap(icon.availableSizes().first()));
			}
		}
	}
	return NULL;
}

void Activity::loadTextList(const QString &AActivityName)
{
	QStringList tmp=Options::node(OPV_ACTIVITY_COMMENT).value(AActivityName).toStringList();
	if(!tmp.isEmpty())
		FActivityTextLists.insert(AActivityName, tmp);
}

void Activity::saveComments(const ActivityData &AActivityData)
{
	if (!AActivityData.isEmpty() && !AActivityData.text.isEmpty())
	{
		if (FActivityTextLists[AActivityData.nameDetailed].contains(AActivityData.text))
			FActivityTextLists[AActivityData.nameDetailed].removeAll(AActivityData.text);
		FActivityTextLists[AActivityData.nameDetailed].prepend(AActivityData.text);
		Options::node(OPV_ACTIVITY_COMMENT).setValue(FActivityTextLists[AActivityData.nameDetailed], AActivityData.nameDetailed);
	}
}

void Activity::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_ROSTER_RECENT_SIMPLEITEMSVIEW));
//TODO: Move this code into to ActivitySelect dialog	
	for(QHash<QString, QStringList>::const_iterator it=FActivityList.constBegin(); it!=FActivityList.constEnd(); it++)
	{
		loadTextList(it.key());
		for (QStringList::const_iterator itl=(*it).constBegin(); itl!=(*it).constEnd(); itl++)
			loadTextList(*itl);
	}
}

void Activity::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_MESSAGES_ACTIVITY_DISPLAY)
		updateChatWindows(true);
	else if (ANode.path() == OPV_ROSTER_RECENT_SIMPLEITEMSVIEW)
	{
		FSimpleContactsView = ANode.value().toBool();
		QMultiMap<int, QVariant> findData;
		findData.insert(RDR_KIND, RIK_RECENT_ITEM);
		QList<IRosterIndex *> indexes = FRostersModel->rootIndex()->findChilds(findData, true);
		for (QList<IRosterIndex *>::ConstIterator it = indexes.constBegin(); it!=indexes.constEnd(); it++)
			emit rosterLabelChanged(RLID_ACTIVITY, *it);
	}
	else if (ANode.path() == OPV_MESSAGES_ACTIVITY_NOTIFY)
		updateChatWindows(false);
	else if (ANode.path() == (Options::node(OPV_COMMON_ADVANCED).value().toBool()?OPV_ROSTER_ACTIVITY_SHOW:OPV_ROSTER_VIEWMODE))
	{
		if (FRostersViewPlugin && FRostersModel)
		{
			if (ANode.path() == OPV_ROSTER_ACTIVITY_SHOW?ANode.value().toBool():ANode.value().toInt() == IRostersView::ViewFull)
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

void Activity::onChatWindowCreated(IMessageChatWindow *AWindow)
{
	updateChatWindow(AWindow);
	updateChatWindowInfo(AWindow);
	connect(AWindow->instance(), SIGNAL(tabPageActivated()), SLOT(onWindowActivated()));
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_activity, Activity)
#endif
