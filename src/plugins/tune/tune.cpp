#include <QCryptographicHash>
#include <QDesktopServices>
#include <QMouseEvent>
#include <QClipboard>
#include <QMimeData>
#include <QApplication>
#include <QFile>
#include <MapObject>
#include <definitions/rosterindexkinds.h>
#include <definitions/rosterindexroles.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/toolbargroups.h>
#include <definitions/actiongroups.h>
#include <definitions/menuicons.h>
#include <definitions/namespaces.h>
#include <definitions/rosterlabels.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/rostertooltiporders.h>
#include <definitions/rosterdataholderorders.h>
#include <definitions/rosterlabelholderorders.h>
#include <definitions/rosterclickhookerorders.h>
#include <definitions/mapobjecttyperole.h>
#include <definitions/resources.h>
#include "definitions/notificationdataroles.h"
#include "definitions/notificationtypeorders.h"
#include "definitions/notificationtypes.h"
#include "definitions/soundfiles.h"
#include <definitions/shortcuts.h>
#include <definitions/shortcutgrouporders.h>
#include <utils/qt4qt5compat.h>

#include "tune.h"
#include "tuneoptions.h"
#include "tuneimagehttpquery.h"

#define MDR_TUNE_ICON		1002

#define ADR_URI				Action::DR_Parametr1
#define ADR_CLIPBOARD_INFO	Action::DR_Parametr2
#define ADR_CLIPBOARD_IMAGE	Action::DR_Parametr3

#define TUNE_CACHE_DIR		"tune"
#define TUNE_CACHE_FILE		"tuneinfo.xml"

UrlRequest::UrlRequest(const QUrl &AUrl): FUrl(AUrl)
{
    start();
    connect(this, SIGNAL(finished()), SLOT(deleteLater()));
}

void UrlRequest::run()
{
    QDesktopServices::openUrl(FUrl);
}

Tune::Tune():
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
        FConnectionManager(NULL),
        FNetworkAccessManager(new QNetworkAccessManager(this)),
        FIconStorage(NULL),
        FPollingPlugins(false),
        FListenersFound(false),
		FSimpleContactsView(false),
        FRosterLabelId(-1),
		FRosterIndexKinds(QList<int>() << RIK_CONTACT << RIK_METACONTACT << RIK_METACONTACT_ITEM << RIK_RECENT_ITEM << RIK_MY_RESOURCE << RIK_STREAM_ROOT),
        FCurrentRequester(NULL)
{}

Tune::~Tune()
{}

void Tune::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("User Tune");
    APluginInfo->description = tr("Implements XEP-0118: User Tune");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(PEPMANAGER_UUID);
}

bool Tune::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    FCachePath.setPath(APluginManager->homePath());

    IPlugin *plugin = APluginManager->pluginInterface("IPEPManager").value(0,NULL);
    if (plugin)
    {
        FPEPManager = qobject_cast<IPEPManager *>(plugin->instance());
        FPEPManager->insertNodeHandler(QString(NS_PEP_TUNE), this);
    } else
        return false;

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
	{
		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());
		connect(FPresenceManager->instance(), SIGNAL(presenceOpened(IPresence*)), SLOT(onPresenceOpened(IPresence*)));
	}

    QList<IPlugin *> plugins = APluginManager->pluginInterface("ITuneListener");
    for (QList<IPlugin *>::const_iterator it=plugins.constBegin(); it!=plugins.constEnd(); it++)
    {
        FListenersFound=true;
        QObject *object=(*it)->instance();
		if (qobject_cast<ITuneListener *>(object)->isPollingType())
        {
            FPollingPlugins=true;
            connect(&FPollingTimer, SIGNAL(timeout()), object, SLOT(check()));
        }
        connect(object, SIGNAL(playing(TuneData)), SLOT(onPlaying(TuneData)));
        connect(object, SIGNAL(stopped()), SLOT(onStopped()));
    }

    plugins = APluginManager->pluginInterface("ITuneInfoRequester");
    for (QList<IPlugin *>::const_iterator it=plugins.constBegin(); it!=plugins.constEnd(); it++)
        FRequesters.insert((*it)->pluginUuid(), qobject_cast<ITuneInfoRequester *>((*it)->instance()));

    plugin = APluginManager->pluginInterface("IMessageWidgets").value(0);
    if (plugin)
    {
        FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
        connect(FMessageWidgets->instance(),SIGNAL(chatWindowCreated(IMessageChatWindow *)),SLOT(onChatWindowCreated(IMessageChatWindow *)), Qt::QueuedConnection);
    }

	plugin = APluginManager->pluginInterface("IMessageStyleManager").value(0,NULL);
    if (plugin)
		FMessageStyleManager = qobject_cast<IMessageStyleManager *>(plugin->instance());

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
    connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

    plugin = APluginManager->pluginInterface("IAccountManager").value(0,NULL);
    if (plugin)
        FAccountManager = qobject_cast<IAccountManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IConnectionManager").value(0,NULL);
    if (plugin)
        FConnectionManager = qobject_cast<IConnectionManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
    if (plugin)
        FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IRostersModel").value(0,NULL);
    if (plugin)
    {
        FRostersModel = qobject_cast<IRostersModel *>(plugin->instance());
        if (FRostersModel)
        {
            connect(FRostersModel->instance(),SIGNAL(indexInserted(IRosterIndex *)),
                    SLOT(onRosterIndexInserted(IRosterIndex *)));
        }
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
					SIGNAL(indexClipboardMenu(QList<IRosterIndex *>, quint32, Menu *)),
					SLOT(onRosterIndexClipboardMenu(QList<IRosterIndex *>, quint32, Menu *)));
            connect(FRostersViewPlugin->rostersView()->instance(),
                    SIGNAL(indexToolTips(IRosterIndex *, quint32, QMap<int,QString> &)),
                    SLOT(onRosterIndexToolTips(IRosterIndex *, quint32, QMap<int,QString> &)));
        }
    }
    AInitOrder = 200;   // This one should be initialized AFTER Map Contacts!

    return true; //FMessageWidgets!=NULL
}

bool Tune::initObjects()
{
	Shortcuts::declareShortcut(SCT_APP_PUBLISHTUNE, tr("Publish tune"), tr("Ctrl+F5", "Switch Publish Tune"), Shortcuts::ApplicationShortcut);

	FIconStorage = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
    FCachePath.mkpath(TUNE_CACHE_DIR);
    FCachePath.cd(TUNE_CACHE_DIR);

    QFile tuneCacheFile(FCachePath.absoluteFilePath(TUNE_CACHE_FILE));
    if (tuneCacheFile.open(QFile::ReadOnly))
    {
        FTuneInfoCache.setContent(&tuneCacheFile);
        tuneCacheFile.close();
        for (QDomElement artist = FTuneInfoCache.documentElement().firstChildElement("artist"); !artist.isNull(); artist = artist.nextSiblingElement("artist"))
        {
            if (artist.attribute("status") == "requested")
                artist.removeAttribute("status");
            for (QDomElement e = artist.firstChildElement(); !e.isNull(); e = e.nextSiblingElement())
                if (e.attribute("status") == "requested")
                    e.removeAttribute("status");
        }
    }
    else if (tuneCacheFile.open(QFile::WriteOnly))
    {
        QTextStream outptStream(&tuneCacheFile);
        outptStream.setCodec("UTF-8");
        FTuneInfoCache.appendChild(FTuneInfoCache.createElement("tuneinfo"));
        FTuneInfoCache.save(outptStream, 1, QDomNode::EncodingFromTextStream);
        tuneCacheFile.close();
    }

    if (FDiscovery)
        registerDiscoFeatures();

    if (FRostersViewPlugin)
    {
        AdvancedDelegateItem label(RLID_TUNE);
        label.d->kind = AdvancedDelegateItem::CustomData;
        label.d->data = FIconStorage->getIcon(MNI_TUNE);
        FRosterLabelId = FRostersViewPlugin->rostersView()->registerLabel(label);
		FRostersViewPlugin->rostersView()->insertLabelHolder(RLHO_TUNE, this);
        FRostersViewPlugin->rostersView()->insertClickHooker(RCHO_TUNE, this);
    }

    if (FMap)
    {
		FMap->geoMap()->registerDataType(MDR_TUNE_ICON, MOT_CONTACT, 220, MOP_RIGHT_TOP);
		FMap->geoMap()->addDataHolder(MOT_CONTACT, this);
    }

    if (FNotifications)
    {
        INotificationType notifyType;
        notifyType.order = NTO_TUNE_CHANGE;
        if (FIconStorage)
            notifyType.icon = FIconStorage->getIcon(MNI_TUNE);
        notifyType.title = tr("When user starts listening a new tune");
        notifyType.kindMask = INotification::PopupWindow|INotification::SoundPlay;
        notifyType.kindDefs = notifyType.kindMask;
        FNotifications->registerNotificationType(NNT_TUNE, notifyType);
    }

	Action *action = FPEPManager->addAction(AG_TUNE);
	action->setText(tr("Publish Tune"));
	action->setIcon(RSR_STORAGE_MENUICONS, MNI_TUNE);
	action->setCheckable(true);
	action->setShortcutId(SCT_APP_PUBLISHTUNE);
	connect(action, SIGNAL(triggered(bool)), SLOT(onPublishUserTuneTriggered(bool)));
    return true;
}
bool Tune::initSettings()
{
    Options::setDefaultValue(OPV_ROSTER_TUNE_SHOW, true);
    Options::setDefaultValue(OPV_MESSAGES_TUNE_DISPLAY, true);
	Options::setDefaultValue(OPV_MESSAGES_TUNE_NOTIFY, true);
	Options::setDefaultValue(OPV_TUNE_PUBLISH, true);
    Options::setDefaultValue(OPV_TUNE_POLLING_INTERVAL, 1000);
    Options::setDefaultValue(OPV_TUNE_INFOREQUESTER_DISPLAYIMAGE, true);
    Options::setDefaultValue(OPV_TUNE_INFOREQUESTER_QUERYURL, true);
    Options::setDefaultValue(OPV_TUNE_INFOREQUESTER_USED, FRequesters.isEmpty()?QString():FRequesters.keys()[0].toString());
    Options::setDefaultValue(OPV_TUNE_INFOREQUESTER_PROXY, APPLICATION_PROXY_REF_UUID);
    // Account specific options
    Options::setDefaultValue(OPV_ACCOUNT_PUBLISHUSERTUNE, true);    // Enable POI for sp

    if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> Tune::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
    if (FOptionsManager)
    {
		if (ANodeId == OPN_ROSTERVIEW)
		{
			if (Options::node(OPV_COMMON_ADVANCED).value().toBool())
				widgets.insertMulti(OWO_ROSTER_PEP_TUNE, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ROSTER_TUNE_SHOW), tr("Show user tune icons"), AParent));
		}
        else if (ANodeId == OPN_MESSAGES)
		{
			widgets.insertMulti(OWO_MESSAGES_INFOBAR_TUNE, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MESSAGES_TUNE_DISPLAY), tr("Display user tune icon"), AParent));
			widgets.insertMulti(OWO_MESSAGES_PEP_TUNE, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MESSAGES_TUNE_NOTIFY), tr("Tune events in chat"), AParent));
		}
		else if (ANodeId == OPN_PEP)
        {
			widgets.insertMulti(OHO_PEP_USERTUNE, FOptionsManager->newOptionsDialogHeader(tr("User Tune"), AParent));
            if (!FRequesters.isEmpty())
            {
				TuneOptions *tuneOptions = new TuneOptions(FRequesters, AParent);
				connect(tuneOptions, SIGNAL(clearCache()), SLOT(onClearCache()));
				widgets.insertMulti(OWO_PEP_USERTUNE, tuneOptions);
                if (FConnectionManager)
					widgets.insertMulti(OWO_PEP_USERTUNE_INFOREQUESTER_PROXY, FConnectionManager->proxySettingsWidget(Options::node(OPV_TUNE_INFOREQUESTER_PROXY), AParent));
			}
			if (FPollingPlugins)
				widgets.insertMulti(OWO_PEP_USERTUNE_POLLING_INTERVAL, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_TUNE_POLLING_INTERVAL), tr("Polling interval"), AParent));
			widgets.insertMulti(OHO_PEP_USERTUNE_LISTENERS, FOptionsManager->newOptionsDialogHeader(tr("User Tune listeners"), AParent));
			widgets.insertMulti(OHO_PEP_USERTUNE_REQUESTERS, FOptionsManager->newOptionsDialogHeader(tr("User Tune requesters"), AParent));
        }
        else if (FListenersFound)   // Add "Send User Tune" option to account settings page
        {
            QStringList nodeTree = ANodeId.split(".", QString::SkipEmptyParts);
			if (nodeTree.count()==3 && nodeTree.at(0)==OPN_ACCOUNTS && nodeTree.at(2)=="Additional")
				widgets.insertMulti(OWO_ACCOUNTS_ADDITIONAL_USERTUNE, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ACCOUNT_ITEM, nodeTree.at(1)).node(OPV_PUBLISHUSERTUNE), tr("Publish User Tune"), AParent));
        }
    }
    return widgets;
}

bool Tune::rosterIndexSingleClicked(int AOrder, IRosterIndex *AIndex, const QMouseEvent *AEvent)
{
	Q_UNUSED(AOrder)

    QModelIndex index = FRostersViewPlugin->rostersView()->mapFromModel(FRostersModel->modelIndexFromRosterIndex(AIndex));
    if (FRostersViewPlugin->rostersView()->labelAt(AEvent->pos(),index) == FRosterLabelId)
    {
        openUrl(FTuneHash.value(Jid(AIndex->data(RDR_FULL_JID).toString()).bare()).uri);
        return true;
    }
    return false;
}

//----------------------
void Tune::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_ROSTER_RECENT_SIMPLEITEMSVIEW));
    onOptionsChanged(Options::node(OPV_TUNE_POLLING_INTERVAL));
    onOptionsChanged(Options::node(OPV_TUNE_INFOREQUESTER_USED));
    if (FConnectionManager)
        onOptionsChanged(Options::node(OPV_TUNE_INFOREQUESTER_PROXY));
    if (!FRequesters.isEmpty())
        onOptionsChanged(Options::node(OPV_TUNE_INFOREQUESTER_USED));
    if (FPollingPlugins)
        FPollingTimer.start();
	FPEPManager->groupActions(AG_TUNE).first()->setChecked(Options::node(OPV_TUNE_PUBLISH).value().toBool());
}

void Tune::onOptionsChanged(const OptionsNode &ANode)
{
    if (ANode.path() == OPV_TUNE_POLLING_INTERVAL)
        FPollingTimer.setInterval(ANode.value().toInt());
	else if (ANode.path() == OPV_ROSTER_RECENT_SIMPLEITEMSVIEW)
	{
		FSimpleContactsView = ANode.value().toBool();
		QMultiMap<int, QVariant> findData;
		findData.insert(RDR_KIND, RIK_RECENT_ITEM);
		QList<IRosterIndex *> indexes = FRostersModel->rootIndex()->findChilds(findData, true);
		for (QList<IRosterIndex *>::ConstIterator it = indexes.constBegin(); it!=indexes.constEnd(); it++)
			emit rosterLabelChanged(RLID_TUNE, *it);
	}
    else if(ANode.path() == OPV_TUNE_INFOREQUESTER_PROXY) // Proxy
        FNetworkAccessManager->setProxy(FConnectionManager->proxyById(ANode.value().toString()).proxy);
    else if(ANode.path() == OPV_TUNE_INFOREQUESTER_USED) // Tune info requester
    {
        if (FCurrentRequester)
            FCurrentRequester->instance()->disconnect(SIGNAL(tuneInfoReceived(QString,QString,QString,QHash<QString, QString>)), this, SLOT(onTuneInfoReceived(QString,QString,QString,QHash<QString, QString>)));
		FCurrentRequester = FRequesters.value(Options::node(OPV_TUNE_INFOREQUESTER_USED).value().toString());
		if (FCurrentRequester)
			connect(FCurrentRequester->instance(), SIGNAL(tuneInfoReceived(QString,QString,QString,QHash<QString, QString>)), SLOT(onTuneInfoReceived(QString,QString,QString,QHash<QString, QString>)));
    }
	else if (ANode.path() == OPV_TUNE_PUBLISH)
		publishCurrentTune(!ANode.value().toBool());
	else if (ANode.path() == (Options::node(OPV_COMMON_ADVANCED).value().toBool()?OPV_ROSTER_TUNE_SHOW:OPV_ROSTER_VIEWMODE))
        if (FRostersViewPlugin && FRostersModel)
        {
			if (ANode.path() == OPV_ROSTER_TUNE_SHOW?ANode.value().toBool():ANode.value().toInt() == IRostersView::ViewFull)
            {
                QMultiMap<int,QVariant> findData;
                for (QList<int>::const_iterator it=FRosterIndexKinds.begin(); it!=FRosterIndexKinds.end(); it++)
                    findData.insertMulti(RDR_KIND, *it);
                QList<IRosterIndex *> indexes = FRostersModel->rootIndex()->findChilds(findData,true);
                for (QList<IRosterIndex *>::const_iterator it=indexes.constBegin(); it!=indexes.constEnd(); it++)
                {
                    Jid jid((*it)->data(RDR_FULL_JID).toString());
                    if (FTuneHash.contains(jid.bare()))
                        FRostersViewPlugin->rostersView()->insertLabel(FRosterLabelId, *it);
                }
            }
            else
                FRostersViewPlugin->rostersView()->removeLabel(FRosterLabelId);
        }
}

bool Tune::processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza)
{
	if (AStanza.type()!="error")
	{
		Jid contactJid = AStanza.from();
		QString bareJid = contactJid.bare();

		QDomElement event = AStanza.firstElement("event", NS_PUBSUB_EVENT);
		QDomElement items = event.firstChildElement("items");

		bool stop=false;
		QDomElement item=items.firstChildElement("item");
		if (!item.isNull()) // We have an item here!
		{
			QDomElement tune=item.firstChildElement("tune");
			if (!tune.isNull())
			{
				if(bareJid == AStreamJid.bare())
					FIdHash.insert("AStreamJid.bare()", item.attribute("id"));
				if (tune.hasChildNodes())
				{
					TuneData tuneData;               // User tunes element
					for (QDomElement e=tune.firstChildElement(); !e.isNull(); e=e.nextSiblingElement())
					{
						if (e.tagName()=="artist")
							tuneData.artist = e.text();
						else if (e.tagName()=="source")
							tuneData.source = e.text();
						else if (e.tagName()=="title")
							tuneData.title = e.text();
						else if (e.tagName()=="track")
							tuneData.track = e.text();
						else if (e.tagName()=="rating")
							tuneData.rating = e.text().toInt();
						else if (e.tagName()=="length")
							tuneData.length = e.text().toInt();
						else if (e.tagName()=="uri")
							tuneData.uri URL_ENCODE(e.text().toLatin1());
					}
					if (tuneData!=FTuneHash[bareJid])
					{
						bool newTune = (tuneData.artist != FTuneHash[bareJid].artist) ||
									   (tuneData.title != FTuneHash[bareJid].title);
						FTuneHash.insert(bareJid, tuneData);
						updateRosterLabels(contactJid);
						if (newTune)
							displayNotification(AStreamJid, contactJid);
						updateChatWindows(contactJid, AStreamJid, newTune);
						if (!tuneData.artist.isEmpty()  && Options::node(OPV_TUNE_INFOREQUESTER_DISPLAYIMAGE).value().toBool())
							requsetTuneInfo(tuneData.artist, tuneData.source);
					}
					return true;
				}
				else
					stop=true;
			}//--------------
		}

		if(!stop && event.firstChild().firstChild().nodeName() == "retract")
		{
			if(contactJid.bare() == AStreamJid.bare())
				FIdHash.remove(AStreamJid.bare());
			stop=true;
		}

		if (stop)
		{
			FTuneHash.remove(contactJid.bare());
			updateRosterLabels(contactJid);
			updateChatWindows(contactJid, AStreamJid, false);
			return true;
		}
	}
    return false;
}

QList<quint32> Tune::rosterLabels(int AOrder, const IRosterIndex *AIndex) const
{
	QList<quint32> labels;
	if (AOrder==RLHO_TUNE && AIndex->kind()==RIK_RECENT_ITEM)
		if (FSimpleContactsView)
			labels.append(RLID_TUNE);
	return labels;
}

AdvancedDelegateItem Tune::rosterLabel(int AOrder, quint32 ALabelId, const IRosterIndex *AIndex) const
{
	Q_UNUSED(AOrder); Q_UNUSED(ALabelId); Q_UNUSED(AIndex);
	static const AdvancedDelegateItem null = AdvancedDelegateItem();
	return null;
}



//---------------------

void Tune::registerDiscoFeatures()
{
    IDiscoFeature dfeature;
    dfeature.var = NS_PEP_TUNE;
    dfeature.active = true;
    dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_TUNE);
    dfeature.name = tr("User Tune");
    dfeature.description = tr("Supports User Tune");
    FDiscovery->insertDiscoFeature(dfeature);

    dfeature.var = QString(NS_PEP_TUNE).append(NODE_NOTIFY_SUFFIX);
    dfeature.active = true;
    dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_TUNE);
    dfeature.name = tr("User Tune Notification");
    dfeature.description = tr("Receives User Tune notifications");
	FDiscovery->insertDiscoFeature(dfeature);
}

bool Tune::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
    return !FDiscovery||!FDiscovery->hasDiscoInfo(AStreamJid,AContactJid)||
            FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_PEP_TUNE);
}

void Tune::sendTune(const TuneData &ATuneData, const Jid &AStreamJid) const
{
    QDomDocument doc;
    QDomElement item=doc.createElement("item");
    item.setAttribute("id", currentItemId(AStreamJid));
    if(!ATuneData.isEmpty())
    {
        QDomElement tuneElement = doc.createElementNS(NS_PEP_TUNE, "tune");        
        QDomElement e;
        QDomText    t;
        if (ATuneData.rating)
        {
            e = doc.createElement("rating");
            t = doc.createTextNode(QString::number(ATuneData.rating));
            e.appendChild(t);
            tuneElement.appendChild(e);
        }
        if (ATuneData.length)
        {
            e = doc.createElement("length");
            t = doc.createTextNode(QString::number(ATuneData.length));
            e.appendChild(t);
            tuneElement.appendChild(e);
        }
        if (!ATuneData.source.isEmpty())
        {
            e = doc.createElement("source");
            t = doc.createTextNode(ATuneData.source);
            e.appendChild(t);
            tuneElement.appendChild(e);
        }
        if (!ATuneData.track.isEmpty())
        {
            e = doc.createElement("track");
            t = doc.createTextNode(ATuneData.track);
            e.appendChild(t);
            tuneElement.appendChild(e);
        }
        if (!ATuneData.artist.isEmpty())
        {
            e = doc.createElement("artist");
            t = doc.createTextNode(ATuneData.artist);
            e.appendChild(t);
            tuneElement.appendChild(e);
        }
        if (!ATuneData.title.isEmpty())
        {
            e = doc.createElement("title");
            t = doc.createTextNode(ATuneData.title);
            e.appendChild(t);
            tuneElement.appendChild(e);
        }
        if (!ATuneData.uri.isEmpty())
        {
            e = doc.createElement("uri");
            t = doc.createTextNode(QString::fromLatin1(ATuneData.uri.toEncoded()));
            e.appendChild(t);
            tuneElement.appendChild(e);
        }
        item.appendChild(tuneElement);
        FPEPManager->publishItem(AStreamJid, NS_PEP_TUNE, item);
    }
    else
    {
        if (Options::node(OPV_PEP_DELETE_PUBLISHEMPTY).value().toBool())
        {
            item.appendChild(doc.createElementNS(NS_PEP_TUNE, "tune"));
            FPEPManager->publishItem(AStreamJid, NS_PEP_TUNE, item);
        }
        if (Options::node(OPV_PEP_DELETE_RETRACT).value().toBool())
            FPEPManager->deleteItem(AStreamJid, NS_PEP_TUNE, item);
    }
}

void Tune::updateRosterLabels(const Jid &AContactJid)
{
    if (FRostersModel)
    {
        QMultiMap<int,QVariant> findData;
        for (QList<int>::const_iterator it=FRosterIndexKinds.begin(); it!=FRosterIndexKinds.end(); it++)
            findData.insertMulti(RDR_KIND, *it);
        findData.insert(RDR_PREP_BARE_JID, AContactJid.pBare());
        QList<IRosterIndex *> indexes = FRostersModel->rootIndex()->findChilds(findData,true);
        for (QList<IRosterIndex *>::const_iterator it=indexes.constBegin(); it!=indexes.constEnd(); it++)
        {
            Jid jid((*it)->data(RDR_FULL_JID).toString());
            if (FTuneHash.contains(jid.bare()))
                FRostersViewPlugin->rostersView()->insertLabel(FRosterLabelId, *it);
            else
                FRostersViewPlugin->rostersView()->removeLabel(FRosterLabelId, *it);
        }
    }

    if (FMapContacts)
    {
        QString bareJid=AContactJid.bare();
        QStringList jids=FMapContacts->getFullJidList(bareJid);
        if (!jids.isEmpty())    // There are such contacts on the map!!!
            for(QStringList::const_iterator it=jids.constBegin(); it!=jids.constEnd(); it++)
                emit mapDataChanged(MOT_CONTACT, *it, MDR_TUNE_ICON); // User Tune either added or removed!
    }
}

void Tune::onStreamOpened(IXmppStream *AXmppStream)
{    
	FStreamsOnline.insert(AXmppStream->streamJid());
}

void Tune::onStreamClosed(IXmppStream *AXmppStream)
{
	FStreamsOnline.remove(AXmppStream->streamJid());
}

void Tune::onRosterIndexInserted(IRosterIndex *AIndex)
{
    if (FRostersViewPlugin && FRosterIndexKinds.contains(AIndex->kind()) &&
		(Options::node(OPV_COMMON_ADVANCED).value().toBool()?Options::node(OPV_ROSTER_TUNE_SHOW).value().toBool():Options::node(OPV_ROSTER_VIEWMODE).value().toInt()==IRostersView::ViewFull))
    {
        Jid jid(AIndex->data(RDR_FULL_JID).toString());
        if (FTuneHash.contains(jid.bare()))
            FRostersViewPlugin->rostersView()->insertLabel(FRosterLabelId, AIndex);
    }
}

void Tune::onRosterIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{Q_UNUSED(AIndexes) Q_UNUSED(ALabelId) Q_UNUSED(AMenu)}

void Tune::onRosterIndexClipboardMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
	if (ALabelId == AdvancedDelegateItem::DisplayId || ALabelId == FRosterLabelId)
		for (QList<IRosterIndex *>::const_iterator it=AIndexes.constBegin(); it!=AIndexes.constEnd(); it++)
		{
			Jid jid((*it)->data(RDR_FULL_JID).toString());
			if (FTuneHash.contains(jid.bare()))
			{
				QHash<QString, QVariant> info;
				Action *action = new Action(AMenu);
				TuneData data = FTuneHash[jid.bare()];
				QString longMessage;
				if (!data.track.isEmpty())
				{
					longMessage.append(data.track).append(". ");
					info.insert("track", data.track);
				}
				if (!data.artist.isEmpty())
				{
					longMessage.append(HTML_ESCAPE(data.artist)).append(" - ");
					info.insert("artist", data.artist);
				}
				if (!data.source.isEmpty())
					info.insert("source", data.source);
				if (!data.title.isEmpty())
				{
					longMessage.append(HTML_ESCAPE(data.title));
					info.insert("title", data.title);
				}
				if (data.length)
				{
					longMessage.append(" (").append(lengthString(data.length)).append(")");
					info.insert("length", data.length);
				}
				if (data.rating)
					info.insert("rating", data.rating);
				if (data.uri.isValid())
					info.insert("uri", data.uri);

				action->setText(longMessage);
				action->setIcon(getIcon());

				action->setData(ADR_CLIPBOARD_INFO, info);

				if (Options::node(OPV_TUNE_INFOREQUESTER_DISPLAYIMAGE).value().toBool())
				{
					QString imageFileName = tuneInfo("image", data.artist, data.source);
					if (!imageFileName.isEmpty())
						action->setData(ADR_CLIPBOARD_IMAGE, imageFileName);
				}

				connect(action, SIGNAL(triggered()), SLOT(onCopyToClipboard()));
				AMenu->addAction(action, AG_RVCBM_PEP, true);
			}
		}
}

QString Tune::getLabel(const Jid &AContactJid) const
{
    return FTuneHash.contains(AContactJid.bare())?getLabel(FTuneHash[AContactJid.bare()]):QString();
}

QString Tune::getLabel(const TuneData &ATuneData) const
{
    QString label=QString("<strong>%1:</strong> ").arg(tr("Now playing"));
    if (Options::node(OPV_TUNE_INFOREQUESTER_DISPLAYIMAGE).value().toBool())
    {
        QString imageFileName = tuneInfo("image", ATuneData.artist, ATuneData.source);
        if (!imageFileName.isEmpty())
			label.append(QString("<div style=\"text-indent: 15px\"><strong><img src=\"%1\" /></div>").arg(QUrl::fromLocalFile(FCachePath.absoluteFilePath(imageFileName)).toString()));
    }
    if (!ATuneData.title.isEmpty())
        label.append("<div style=\"text-indent: 15px\"><strong>").append(tr("Title")).append(":</strong> ").append(ATuneData.title).append("</div>");
    if (!ATuneData.artist.isEmpty())
        label.append("<div style=\"text-indent: 15px\"><strong>").append(tr("Artist")).append(":</strong> ").append(ATuneData.artist).append("</div>");
    if (!ATuneData.source.isEmpty())
        label.append("<div style=\"text-indent: 15px\"><strong>").append(tr("Source")).append(":</strong> ").append(ATuneData.source).append("</div>");
    if (!ATuneData.track.isEmpty())
        label.append("<div style=\"text-indent: 15px\"><strong>").append(tr("Track")).append(":</strong> ").append(ATuneData.track).append("</div>");
    if (ATuneData.length)
        label.append("<div style=\"text-indent: 15px\"><strong>").append(tr("Length")).append(":</strong> ").append(lengthString(ATuneData.length).append("</div>"));
    if (ATuneData.rating)
        label.append("<div style=\"text-indent: 15px\"><strong>").append(tr("Rating")).append(":</strong> ").append(QString::number(ATuneData.rating)).append("</div>");
    if (ATuneData.uri.isValid())
        label.append("<div style=\"text-indent: 15px\"><strong>").append(tr("URL")).append(":</strong> <u><font color=blue>").append(ATuneData.uri.toString()).append("</font></u></div>");
    return label;
}

QString Tune::lengthString(const quint16 &ASeconds)
{
    return QTime().addSecs(ASeconds).toString("m:ss");
}

void Tune::displayNotification(const Jid &AStreamJid, const Jid &AContactJid)
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
    notify.kinds = FNotifications->enabledTypeNotificationKinds(NNT_TUNE);

    if (FMessageWidgets)
    {
        IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
        if (window && window->isActiveTabPage())    // The window is existing and is an active tab page!
            notify.kinds = 0;                       // So, do not need to notify!
    }

    if (notify.kinds)
    {
        TuneData tuneData=FTuneHash.value(AContactJid.bare());
        QString tune;
        if (!tuneData.track.isEmpty())
            tune.append(tuneData.track).append(". ");
        if (!tuneData.artist.isEmpty())
            tune.append(tuneData.artist).append(" - ");
        tune.append(tuneData.title);

        QString html=tr("Now listening: %1").arg(QString("<b>").append(tune).append("</b>"));

        notify.typeId = NNT_TUNE;
        notify.data.insert(NDR_STREAM_JID, AStreamJid.full());
        notify.data.insert(NDR_CONTACT_JID, AContactJid.full());
        notify.data.insert(NDR_ICON, getIcon());
        notify.data.insert(NDR_POPUP_HTML, html);
        notify.data.insert(NDR_POPUP_TITLE, FNotifications->contactName(AStreamJid, AContactJid));
        notify.data.insert(NDR_POPUP_IMAGE, FNotifications->contactAvatar(AContactJid));
        notify.data.insert(NDR_SOUND_FILE, SDF_PEP_EVENT);
        if (FNotifies[AStreamJid].contains(AContactJid))
            FNotifications->removeNotification(FNotifies[AStreamJid].value(AContactJid));
        FNotifies[AStreamJid].insert(AContactJid, FNotifications->appendNotification(notify));
    }

    //    emit received(AStreamJid, AContactJid, AMessageId);
}

void Tune::removeNotifiedMessages(IMessageChatWindow *AWindow)
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

IPresenceItem Tune::presenceItemForBareJid(const Jid &AStreamJid, const Jid &AContactJid) const
{
    IPresence *presence = FPresenceManager ? FPresenceManager->findPresence(AStreamJid) : NULL;
    return presence ? FPresenceManager->sortPresenceItems(presence->findItems(AContactJid)).value(0) : IPresenceItem();
}

QString Tune::currentItemId(const Jid &AStreamJid) const
{
    return FIdHash.contains(AStreamJid.bare())?FIdHash[AStreamJid.bare()]:"current";
}

QString Tune::tuneInfo(const QString &AKey, const QString &AArtist, const QString &AAlbum, const QString &ATrack, bool FDetailed) const
{
    if (!AArtist.isNull())
    {
        QDomElement tuneinfo=FTuneInfoCache.documentElement();
        if (!tuneinfo.isNull())
        {
            for (QDomElement artist=tuneinfo.firstChildElement("artist"); !artist.isNull(); artist=artist.nextSiblingElement("artist"))
                if (artist.attribute("name") == AArtist)
                {
                    if (!ATrack.isEmpty())
                    {
                        for (QDomElement track=artist.firstChildElement("track"); !track.isNull(); track=track.nextSiblingElement("track"))
                            if (track.attribute("name") == ATrack)
                                if (track.hasAttribute(AKey))
                                    return track.attribute(AKey);
                        if (FDetailed)
                            return QString();
                    }

                    if (!AAlbum.isEmpty())
                    {
                        for (QDomElement album=artist.firstChildElement("album"); !album.isNull(); album=album.nextSiblingElement("album"))
                            if (album.attribute("name") == AAlbum)
                                if (album.hasAttribute(AKey))
                                    return album.attribute(AKey);
                        if (FDetailed)
                            return QString();
                    }

                    return artist.attribute(AKey);
                }
        }
    }
    return QString();
}

Tune::TuneInfoStatus Tune::tuneInfoStatus(const QString &AArtist, const QString &AAlbum, const QString &ATrack) const
{
    if (!AArtist.isNull())
    {
        QDomElement tuneinfo=FTuneInfoCache.documentElement();
        if (!tuneinfo.isNull())
        {
            for (QDomElement artist=tuneinfo.firstChildElement("artist"); !artist.isNull(); artist=artist.nextSiblingElement("artist"))
                if (artist.attribute("name") == AArtist)
                {
                    if (!ATrack.isEmpty())
                    {
                        for (QDomElement track=artist.firstChildElement("track"); !track.isNull(); track=track.nextSiblingElement("track"))
                            if (track.attribute("name") == ATrack && track.attribute("requested") == "1")
                            {
                                if (track.attribute("status") == "requested")
                                    return Requested;
                                else if (track.attribute("status") == "recieved")
                                    return Recieved;
                                else
                                    break;
                            }
                        return NotRequested;
                    }
                    if (!AAlbum.isEmpty())
                    {
                        for (QDomElement album=artist.firstChildElement("album"); !album.isNull(); album=album.nextSiblingElement("album"))
                            if (album.attribute("name") == AAlbum)
                            {
                                if (album.attribute("status") == "requested")
                                    return Requested;
                                else if (album.attribute("status") == "recieved")
                                    return Recieved;
                                else
                                    break;
                            }
                        return NotRequested;
                    }
                    if (artist.attribute("status") == "requested")
                        return Requested;
                    else if (artist.attribute("status") == "recieved")
                        return Recieved;
                    break;
                }
            return NotRequested;
        }
    }
    return Error;
}

void Tune::requsetTuneInfo(const QString &AArtist, const QString &AAlbum, const QString &ATrack)
{
    if (tuneInfoStatus(AArtist, AAlbum, ATrack)==NotRequested && // Have to request it
        FCurrentRequester &&
        FCurrentRequester->requestTuneInfo(FNetworkAccessManager, AArtist, AAlbum, ATrack))
        createElement(AArtist, AAlbum, ATrack).setAttribute("status", "requested");
}

QDomElement Tune::findElement(const QString &AArtist, const QString &AAlbum, const QString &ATrack) const
{
    if (!AArtist.isNull())
    {
        QDomElement tuneinfo=FTuneInfoCache.documentElement();
        if (!tuneinfo.isNull())
        {
            for (QDomElement artist=tuneinfo.firstChildElement("artist"); !artist.isNull(); artist=artist.nextSiblingElement("artist"))
                if (artist.attribute("name") == AArtist)
                {
                    if (AAlbum.isEmpty() && ATrack.isEmpty())
                        return artist;
                    else
                    {
                        if (!ATrack.isEmpty())
                        {
                            for (QDomElement track=artist.firstChildElement("track"); !track.isNull(); track=track.nextSiblingElement("track"))
                                if (track.attribute("name") == ATrack)
                                    return track;
                            return QDomElement();
                        }
                        if (!AAlbum.isEmpty())
                        {
                            for (QDomElement album=artist.firstChildElement("album"); !album.isNull(); album=album.nextSiblingElement("album"))
                                if (album.attribute("name") == AAlbum)
                                    return album;
                            return QDomElement();
                        }
                    }
                }
        }
    }
    return QDomElement();
}

QDomElement Tune::createElement(const QString &AArtist, const QString &AAlbum, const QString &ATrack)
{
    if (!AArtist.isEmpty())
    {
        QDomElement artist = findElement(AArtist);
        if (artist.isNull())
        {
            artist = FTuneInfoCache.createElement("artist");
            artist.setAttribute("name", AArtist);
            FTuneInfoCache.documentElement().appendChild(artist);
        }
        if (!ATrack.isEmpty())
        {            
            QDomElement track = findElement(AArtist, QString(), ATrack);
            if (track.isNull())
            {
                track = FTuneInfoCache.createElement("track");
                track.setAttribute("name", ATrack);
                artist.appendChild(track);                
            }            
            return track;
        }
        if (!AAlbum.isEmpty())
        {
            QDomElement album = findElement(AArtist, AAlbum);
            if (album.isNull())
            {
                album = FTuneInfoCache.createElement("album");
                album.setAttribute("name", AAlbum);
                artist.appendChild(album);
            }            
            return album;
        }
        return artist;
    }
    return QDomElement();
}

void Tune::saveTuneInfoCache() const
{
    QFile tuneCacheFile(FCachePath.absoluteFilePath(TUNE_CACHE_FILE));
    if (tuneCacheFile.open(QFile::WriteOnly|QFile::Truncate))
    {
        QTextStream outptStream(&tuneCacheFile);
        outptStream.setCodec("UTF-8");
        FTuneInfoCache.save(outptStream, 1, QDomNode::EncodingFromTextStream);
        tuneCacheFile.close();
    }
}

/**
 * @brief Tune::openUrl opens specified URL asynchronously
 * @param AUrl URL to be opened
 */
void Tune::openUrl(const QUrl &AUrl) const
{
    if (AUrl.isValid())
        new UrlRequest(AUrl);
}

void Tune::onRosterIndexToolTips(IRosterIndex *AIndex, quint32 ALabelId, QMap<int, QString> &AToolTips)
{    
    if ((ALabelId == AdvancedDelegateItem::DisplayId || ALabelId == FRosterLabelId)
         && FRosterIndexKinds.contains(AIndex->kind()))
    {
        Jid jid(AIndex->data(RDR_FULL_JID).toString());
        if (FTuneHash.contains(jid.bare())) //.bare()-----------------
        {
			AToolTips.insert(RTTO_ROSTERSVIEW_TUNE_SEPARATOR, "<hr>");
			AToolTips.insert(RTTO_ROSTERSVIEW_TUNE, getLabel(FTuneHash[jid.bare()]));
        }
	}
}

void Tune::onCopyToClipboard()
{
	QHash<QString, QVariant> info = qobject_cast<Action *>(sender())->data(ADR_CLIPBOARD_INFO).toHash();
	QString fileName = qobject_cast<Action *>(sender())->data(ADR_CLIPBOARD_IMAGE).toString();

	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime;
	mime = new QMimeData();

	QString text, html;

	if (!fileName.isEmpty())
	{
		QFile file(FCachePath.absoluteFilePath(fileName));
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
					html=info.contains("source")?QString("<img src=\"%1\" alt=\"%2\" title=\"%2\" />").arg(url.toString()).arg(info["source"].toString()):QString("<img src=\"%1\" />").arg(url.toString());
				}
			}
		}
	}

	if (info.contains("title"))
	{
		text.append(tr("%1: %2").arg(tr("Title")).arg(info["title"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b> %2").arg(tr("Title")).arg(info["title"].toString()));
	}
	if (info.contains("artist"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(tr("Artist")).arg(info["artist"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b> %2").arg(tr("Artist")).arg(info["artist"].toString()));
	}
	if (info.contains("source"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(tr("Source")).arg(info["source"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b> %2").arg(tr("Source")).arg(info["source"].toString()));
	}
	if (info.contains("track"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(tr("Track")).arg(info["track"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b> %2").arg(tr("Track")).arg(info["track"].toString()));
	}
	if (info.contains("length"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(tr("Length")).arg(lengthString(info["length"].toInt())));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b> %2").arg(tr("Length")).arg(lengthString(info["length"].toInt())));
	}
	if (info.contains("rating"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(tr("Rating")).arg(info["rating"].toInt()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b> %2").arg(tr("Rating")).arg(info["rating"].toInt()));
	}
	if (info.contains("uri"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(tr("URL")).arg(info["uri"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b> <a href=\"%2\">%2</a>").arg(tr("URL")).arg(info["uri"].toString()));
		mime->setUrls(QList<QUrl>() << info["uri"].toUrl());
	}

	if (!text.isEmpty())
		mime->setText(text);
	if (!html.isEmpty())
		mime->setHtml(QString("<body>%1</body>").arg(html));

	clipboard->setMimeData(mime);
}

void Tune::updateChatWindows(bool AInfoBar)
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

void Tune::updateChatWindows(const Jid &AContactJid, const Jid &AStreamJid, bool AAddMessage)//
{
	if (FMessageWidgets)
	{
		IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
		if (window)
		{
			updateChatWindowInfo(window);
			if (AAddMessage)
				updateChatWindow(window);
		}
	}
}

void Tune::updateChatWindow(IMessageChatWindow *AMessageChatWindow)
{
    QString contactJid = AMessageChatWindow->contactJid().bare();
	if (Options::node(OPV_MESSAGES_TUNE_NOTIFY).value().toBool() && FTuneHash.contains(contactJid))
    {        
        TuneData tuneData=FTuneHash[contactJid];
		QString longMessage;
		if (!tuneData.track.isEmpty())
			longMessage.append(tuneData.track).append(". ");
		if (!tuneData.artist.isEmpty())
			longMessage.append(HTML_ESCAPE(tuneData.artist)).append(" - ");
		if (!tuneData.title.isEmpty())
			longMessage.append(HTML_ESCAPE(tuneData.title));
		if (tuneData.length)
			longMessage.append(" (").append(lengthString(tuneData.length)).append(")");
		IMessageStyleContentOptions options;
		options.time = QDateTime::currentDateTime();
		options.timeFormat = FMessageStyleManager->timeFormat(options.time);
		options.kind = IMessageStyleContentOptions::KindStatus;
		options.type = IMessageStyleContentOptions::TypeEvent;
		options.direction = IMessageStyleContentOptions::DirectionIn;
		options.senderId = AMessageChatWindow->contactJid().full();
		options.senderName   = HTML_ESCAPE(FMessageStyleManager->contactName(AMessageChatWindow->streamJid(), AMessageChatWindow->contactJid()));
		options.senderAvatar = FMessageStyleManager->contactAvatar(AMessageChatWindow->contactJid());

		QString html = QString("<img src=\"%1\" alt=\"%2\" title=\"%2\" /> %3")
						.arg(QUrl::fromLocalFile(getIconFileName()).toString())
						.arg(tr("Now playing"))
						.arg(longMessage);
		AMessageChatWindow->viewWidget()->appendHtml(html, options);
    }
}

void Tune::updateChatWindowInfo(IMessageChatWindow *AMessageChatWindow)
{
	QString contactJid = AMessageChatWindow->contactJid().bare();
	if (Options::node(OPV_MESSAGES_TUNE_DISPLAY).value().toBool() && FTuneHash.contains(contactJid))
	{
		TuneData tuneData=FTuneHash[contactJid];
		Action *action = NULL;
		QList<QAction *> actions=AMessageChatWindow->infoWidget()->infoToolBarChanger()->groupItems(AG_TUNE);
		if (actions.isEmpty())
		{
			action=new Action(AMessageChatWindow->infoWidget()->infoToolBarChanger()->toolBar());
			action->setIcon(getIcon());
			connect(action, SIGNAL(triggered()), SLOT(onTuneActionTriggered()));
//            action->setShortcutId(SCT_MESSAGEWINDOWS_CHAT_OPENTUNEURL);
			AMessageChatWindow->infoWidget()->infoToolBarChanger()->insertAction(action, AG_TUNE);
		}
		else
			action=AMessageChatWindow->infoWidget()->infoToolBarChanger()->handleAction(actions[0]);

		action->setToolTip(getLabel(tuneData));
		if (tuneData.uri.isValid())
			action->setData(ADR_URI, tuneData.uri);
		else
			action->setData(ADR_URI, QVariant());
	}
	else
	{
		QList<QAction *> actions=AMessageChatWindow->infoWidget()->infoToolBarChanger()->groupItems(AG_TUNE);
		if (!actions.isEmpty())
		{
			actions[0]->disconnect(SIGNAL(triggered()));
			AMessageChatWindow->infoWidget()->infoToolBarChanger()->handleWidget(actions[0])->deleteLater();
			AMessageChatWindow->infoWidget()->infoToolBarChanger()->removeItem(actions[0]);
		}
	}
}

QIcon Tune::getIcon() const
{
    return FIconStorage->getIcon(MNI_TUNE);
}

QString Tune::getIconFileName() const
{
    return FIconStorage->fileFullName(MNI_TUNE);
}

//------------- <<< Map Data Holder <<< -----------------
void Tune::onMapObjectInserted(int AType, const QString &AId)
{
    if (AType==MOT_CONTACT)
        if (FTuneHash.contains(Jid(AId).bare()))
            emit mapDataChanged(AType, AId, MDR_TUNE_ICON);
}

void Tune::onMapObjectRemoved(int AType, const QString &AId)
{
    Q_UNUSED(AType)
    Q_UNUSED(AId)
}

QGraphicsItem * Tune::mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement)
{
    if (ARole == MDR_TUNE_ICON)
    {
        Jid jid(ASceneObject->mapObject()->id());
        if (FTuneHash.contains(jid.bare()))
        {
            QIcon icon=getIcon();
            if (icon.isNull())
                return NULL;
            else
                if (ACurrentElement)
                    qgraphicsitem_cast<QGraphicsPixmapItem *>(ACurrentElement)->setPixmap(icon.pixmap(icon.availableSizes().first()));
                else
                    ACurrentElement = new QGraphicsPixmapItem(icon.pixmap(icon.availableSizes().first()));
        }
    }
    return ACurrentElement;
}

void Tune::onChatWindowCreated(IMessageChatWindow *AWindow)
{
	updateChatWindowInfo(AWindow);
	updateChatWindow(AWindow);
    connect(AWindow->instance(), SIGNAL(tabPageActivated()), SLOT(onWindowActivated()));
}

void Tune::onWindowActivated()
{
    IMessageChatWindow *window = qobject_cast<IMessageChatWindow *>(sender());
    if (window)
        removeNotifiedMessages(window);
}

void Tune::onNotificationActivated(int ANotifyId)
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
					FMessageProcessor->getMessageWindow(its.key(), contactJid, Message::Chat, IMessageProcessor::ActionAssign);
                    window = FMessageWidgets->findChatWindow(its.key(), contactJid);
                }
                if (window)
                {
                    window->showTabPage();
                    return;
                }
            }
}

void Tune::onTuneInfoReceived(const QString &AArtist, const QString &AAlbum, const QString &ATrack, const QHash<QString, QString> &ATuneInfo)
{
    if (!ATuneInfo.isEmpty())
    {
        QDomElement e = findElement(AArtist, AAlbum, ATrack);
        e.setAttribute("status", "received");
        if (ATuneInfo.contains("image"))
        {
            TuneImageHttpQuery *httpRequest=new TuneImageHttpQuery(ATuneInfo["image"], AArtist, AAlbum);
            connect(httpRequest, SIGNAL(resultReceived(QByteArray,QString,QString)), SLOT(onResultReceived(QByteArray,QString,QString)));
            httpRequest->sendRequest(FNetworkAccessManager);
        }
        else    // No image found
            if (!AAlbum.isEmpty())  // If album image was requested, let's try to request artist image instead
                requsetTuneInfo(AArtist, QString());

        if (Options::node(OPV_TUNE_INFOREQUESTER_QUERYURL).value().toBool())
        {
            if (ATuneInfo.contains("url"))
            {
                e.setAttribute("uri", ATuneInfo["url"]);
                if (FCurrentTuneData.artist == AArtist &&
                    FCurrentTuneData.source == AAlbum &&
                    FCurrentTuneData.title == ATrack &&
                    !FCurrentTuneData.uri.isValid())
                {
					FCurrentTuneData.uri URL_ENCODE(ATuneInfo.value("url").toLatin1());
					notifyCurrentTune();
                }
            }
            else    // No URI found
                if (!(ATrack.isEmpty()))                 // If track URI was requested,
                    requsetTuneInfo(AArtist, AAlbum);    // Let's try to request album URI instead
                else if (!(AAlbum.isEmpty()))            // If album URI was requested,
                    requsetTuneInfo(AArtist, QString()); // Let's try to request artist URI instead
        }
        saveTuneInfoCache();
    }
}

void Tune::onResultReceived(const QByteArray &AResult, const QString &AArtist, const QString &AAlbum)
{
    QString fileName = QCryptographicHash::hash(AResult, QCryptographicHash::Sha1).toHex();
    QString path(FCachePath.absoluteFilePath(fileName));
    QFile file(path);
    if (file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        file.write(AResult);
        file.close();
        QDomElement e = findElement(AArtist, AAlbum, QString());
        e.setAttribute("image", fileName);
        saveTuneInfoCache();
        QList<IMessageChatWindow *> windows = FMessageWidgets->chatWindows();
        for (QList<IMessageChatWindow *>::const_iterator it=windows.constBegin(); it!=windows.constEnd(); it++)
			updateChatWindowInfo(*it);
    }
}

void Tune::onTuneActionTriggered() const
{
    Action *action = qobject_cast<Action *>(sender());
    if (action)
		openUrl(action->data(ADR_URI).toUrl());
}

void Tune::onPublishUserTuneTriggered(bool APublish) const
{
	Options::node(OPV_TUNE_PUBLISH).setValue(APublish);
}

void Tune::onPresenceOpened(IPresence *APresence)
{
	if (Options::node(OPV_TUNE_PUBLISH).value().toBool() &&
		FAccountManager->findAccountByStream(APresence->streamJid())->optionsNode().value(OPV_PUBLISHUSERTUNE).toBool()
		&& FPEPManager->isSupported(APresence->streamJid()))
		sendTune(FCurrentTuneData, APresence->streamJid());
}

void Tune::onPlaying(const TuneData &ATuneData)
{
    FCurrentTuneData = ATuneData;
    if (!ATuneData.uri.isValid() &&                                      // No valid URI received from the listener and
        Options::node(OPV_TUNE_INFOREQUESTER_QUERYURL).value().toBool()) // Option for requesting URL is on
    {                                                                    // So, let's try to obtain URL from tune info requester
        QString uri = tuneInfo("uri", ATuneData.artist, ATuneData.source, ATuneData.title, true);
        if (!uri.isEmpty())
			FCurrentTuneData.uri URL_ENCODE(uri.toLatin1());
        else
            requsetTuneInfo(ATuneData.artist, ATuneData.source, ATuneData.title);
    }
	notifyCurrentTune();
}

void Tune::onStopped()
{
    FCurrentTuneData.clear();
	notifyCurrentTune();
}

void Tune::publishCurrentTune(bool ARetract) const
{
	for (QSet<Jid>::const_iterator it=FStreamsOnline.constBegin(); it!=FStreamsOnline.constEnd(); it++)
		if (FAccountManager->findAccountByStream(*it)->optionsNode().value(OPV_PUBLISHUSERTUNE).toBool() && FPEPManager->isSupported(*it))
			sendTune(ARetract?TuneData():FCurrentTuneData, *it);
}

void Tune::notifyCurrentTune() const
{
	if (Options::node(OPV_TUNE_PUBLISH).value().toBool())
		publishCurrentTune();
}

void Tune::onClearCache()
{
    QStringList cachedFiles=FCachePath.entryList();
    for (QStringList::const_iterator it=cachedFiles.constBegin(); it!=cachedFiles.constEnd(); it++)
        QFile(FCachePath.absoluteFilePath(*it)).remove();
    FTuneInfoCache.clear();
    FTuneInfoCache.appendChild(FTuneInfoCache.createElement("tuneinfo"));
    saveTuneInfoCache();
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_tune, Tune)
#endif
