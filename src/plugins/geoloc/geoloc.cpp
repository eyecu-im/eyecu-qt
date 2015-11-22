#include "geoloc.h"
#include <QClipboard>
#include <QApplication>
#include <QMimeData>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/menuicons.h>
#include <definitions/namespaces.h>
#include <definitions/rosterlabels.h>
#include <definitions/rosterlabelholderorders.h>
#include <definitions/rostertooltiporders.h>
#include <definitions/rosterdataholderorders.h>
#include <definitions/resources.h>
#include <definitions/actiongroups.h>
#include <definitions/shortcuts.h>

#include <utils/options.h>
#include <utils/logger.h>

#define ADR_STREAM_JID          Action::DR_StreamJid
#define ADR_CONTACT_JID         Action::DR_Parametr1
#define ADR_CLIPBOARD_DATA      Action::DR_Parametr2

#define SHC_MESSAGE_IN          "/message[@type='chat']/active"
#define SHC_OUT_MESSAGE         "/message[@type='chat']/"

Geoloc::Geoloc():
			FPEPManager(NULL),
			FDiscovery(NULL),
			FXmppStreams(NULL),
			FOptionsManager(NULL),
			FRostersModel(NULL),
			FRostersViewPlugin(NULL),
			FMessageWidgets(NULL),
			FMapContacts(NULL),
			FPositioning(NULL),
			FAccountManager(NULL),
			FIconStorage(NULL),
			FRosterLabelId(-1),
			FSimpleContactsView(false),
			FToggleSend(false),
			FRosterIndexKinds(QList<int>() << RIK_CONTACT << RIK_METACONTACT << RIK_METACONTACT_ITEM << RIK_RECENT_ITEM << RIK_MY_RESOURCE << RIK_STREAM_ROOT)
{}

Geoloc::~Geoloc()
{}

void Geoloc::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("User Location");
	APluginInfo->description = tr("Implements XEP-0080: User Location");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(PEPMANAGER_UUID);
}

bool Geoloc::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	IPlugin *plugin = APluginManager->pluginInterface("IPEPManager").value(0,NULL);
	if (plugin)
	{
		FPEPManager = qobject_cast<IPEPManager *>(plugin->instance());
		FPEPManager->insertNodeHandler(QString(NS_PEP_GEOLOC), this);
	} else
		return false;

	plugin = APluginManager->pluginInterface("IMessageWidgets").value(0);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
		connect(FMessageWidgets->instance(), SIGNAL(chatWindowCreated(IMessageChatWindow *)), SLOT(onChatWindowCreated(IMessageChatWindow *)));
	}

	plugin = APluginManager->pluginInterface("IMapContacts").value(0);
	if (plugin)
		FMapContacts = qobject_cast<IMapContacts *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0,NULL);
	if (plugin)
	{
		FXmppStreams = qobject_cast<IXmppStreamManager *>(plugin->instance());
		if (FXmppStreams)
		{
			connect(FXmppStreams->instance(), SIGNAL(streamOpened(IXmppStream *)), SLOT(onStreamOpened(IXmppStream *)));
			connect(FXmppStreams->instance(), SIGNAL(streamClosed(IXmppStream *)), SLOT(onStreamClosed(IXmppStream *)));
		}
	}

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0,NULL);
	if (plugin)
		connect(plugin->instance(), SIGNAL(presenceActiveChanged(IPresence *, bool)), SLOT(onPresenceActiveChanged(IPresence *,bool)));

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IAccountManager").value(0,NULL);
	if (plugin)
		FAccountManager = qobject_cast<IAccountManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IPositioning").value(0,NULL);
	if (plugin)
	{
		FPositioning = qobject_cast<IPositioning *>(plugin->instance());
		connect(FPositioning->instance(),SIGNAL(newPositionAvailable(GeolocElement)),SLOT(onNewPositionAvailable(GeolocElement)));
	}

	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));
	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IRostersModel").value(0,NULL);
	if (plugin)
	{
		FRostersModel = qobject_cast<IRostersModel *>(plugin->instance());
		if (FRostersModel)
		{
			   connect(FRostersModel->instance(), SIGNAL(indexInserted(IRosterIndex *)),
												  SLOT(onRosterIndexInserted(IRosterIndex *)));
			   connect(FRostersModel->instance(), SIGNAL(indexDataChanged(IRosterIndex *, int)),
												  SLOT(onRosterIndexDataChanged(IRosterIndex*,int)));
		}
	}

//-------------
	plugin = APluginManager->pluginInterface("IRostersViewPlugin").value(0,NULL);
	if (plugin)
	{
		FRostersViewPlugin = qobject_cast<IRostersViewPlugin *>(plugin->instance());
		if (FRostersViewPlugin)
		{
			connect(FRostersViewPlugin->rostersView()->instance(),
					SIGNAL(indexToolTips(IRosterIndex *, quint32, QMap<int,QString> &)),
					SLOT(onRosterIndexToolTips(IRosterIndex *, quint32, QMap<int,QString> &)));
			connect(FRostersViewPlugin->rostersView()->instance(),
					SIGNAL(indexClipboardMenu(QList<IRosterIndex *>, quint32, Menu *)),
					SLOT(onRosterIndexClipboardMenu(QList<IRosterIndex *>, quint32, Menu *)));
		}
	}
	AInitOrder = 100;

	return true;
}

bool Geoloc::initObjects()
{
	FIconStorage = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);

	if (FMapContacts)
		Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_CHAT_SHOWCONTACTONTHEMAP, tr("Show contact on the map"), tr("F10", "Show contact on the map (chat)"), Shortcuts::WindowShortcut);

	if (FDiscovery)
		registerDiscoFeatures();

	if (FRostersViewPlugin)
	{
		AdvancedDelegateItem label(RLID_GEOLOC);
		label.d->kind = AdvancedDelegateItem::CustomData;
		label.d->data = FIconStorage->getIcon(MNI_GEOLOC);
		FRosterLabelId = FRostersViewPlugin->rostersView()->registerLabel(label);
		FRostersViewPlugin->rostersView()->insertLabelHolder(RLHO_GEOLOC, this);
	}

	FTranslated.insert("accuracy", tr("Accuracy"));
	FTranslated.insert("alt", tr("Altitude"));
	FTranslated.insert("area", tr("Area"));
	FTranslated.insert("bearing", tr("Bearing"));
	FTranslated.insert("building", tr("Building"));
	FTranslated.insert("country", tr("Country"));
	FTranslated.insert("countrycode", tr("Country code"));
	FTranslated.insert("datum", tr("Datum"));
	FTranslated.insert("description", tr("Description"));
	FTranslated.insert("error", tr("Error"));
	FTranslated.insert("floor", tr("Floor"));
	FTranslated.insert("lat", tr("Latitude"));
	FTranslated.insert("locality", tr("Locality"));
	FTranslated.insert("lon", tr("Longitude"));
	FTranslated.insert("postalcode", tr("Postal code"));
	FTranslated.insert("region", tr("Region"));
	FTranslated.insert("room", tr("Room"));
	FTranslated.insert("Speed", tr("Speed"));
	FTranslated.insert("street", tr("Street"));
	FTranslated.insert("text", tr("Text"));
	FTranslated.insert("timestamp", tr("Timestamp"));
	FTranslated.insert("tzo", tr("TZO"));
	FTranslated.insert("uri", tr("URI"));

	return true;
}

bool Geoloc::initSettings()
{	
	Options::setDefaultValue(OPV_ROSTER_GEOLOC_SHOW, true);
	Options::setDefaultValue(OPV_MESSAGES_GEOLOC_DISPLAY, true);
	Options::setDefaultValue(OPV_ACCOUNT_PUBLISHUSERLOCATION, true);

	if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);
	return true;
}

QMultiMap<int, IOptionsDialogWidget *> Geoloc::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (FOptionsManager)
	{
		if (ANodeId == OPN_ROSTERVIEW)
		{
			if (Options::node(OPV_COMMON_ADVANCED).value().toBool())
				widgets.insertMulti(OWO_ROSTER_PEP_LOCATION, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ROSTER_GEOLOC_SHOW), tr("Show user location icons"), AParent));
		}
		else if (ANodeId == OPN_MESSAGES)
			widgets.insertMulti(OWO_MESSAGES_INFOBAR_LOCATION, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MESSAGES_GEOLOC_DISPLAY),tr("Display user location icon"),AParent));
		else if (FPositioning)   // Add "Send User Location" option to account settings page
		{
			QStringList nodeTree = ANodeId.split(".", QString::SkipEmptyParts);
			if (nodeTree.count()==3 && nodeTree.at(0)==OPN_ACCOUNTS && nodeTree.at(2)=="Additional")
				widgets.insertMulti(OWO_ACCOUNTS_ADDITIONAL_GEOLOC, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ACCOUNT_ITEM, nodeTree.at(1)).node(OPV_PUBLISHUSERLOCATION), tr("Publish location"), AParent));
		}
	}
	return widgets;
}

QList<quint32> Geoloc::rosterLabels(int AOrder, const IRosterIndex *AIndex) const
{
	QList<quint32> labels;
	if (AOrder==RLHO_GEOLOC && AIndex->kind()==RIK_RECENT_ITEM)
		if (FSimpleContactsView)
			labels.append(RLID_GEOLOC);
	return labels;
}

AdvancedDelegateItem Geoloc::rosterLabel(int AOrder, quint32 ALabelId, const IRosterIndex *AIndex) const
{
	Q_UNUSED(AOrder); Q_UNUSED(ALabelId); Q_UNUSED(AIndex);
	static const AdvancedDelegateItem null = AdvancedDelegateItem();
	return null;
}

bool Geoloc::onNewPositionAvailable(const GeolocElement &APosition)
{
	//---timer public data on server xx sec ----

	if (APosition.isValid())
	{
		sendGeoloc(APosition);
		FToggleSend = false;
	}
	else
		retractGeoloc();
	return true;
}
//---------------------

bool Geoloc::processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza)
{
	if (AStanza.type()!="error")
	{
		Jid userFrom = AStanza.from();
		QString itemId;

		QDomElement event = AStanza.firstElement("event", NS_PUBSUB_EVENT);
		QDomElement items = event.firstChildElement("items");

		bool fromMe=false;
		if (userFrom.isEmpty())
			fromMe = true;
		else if(userFrom.bare()==AStreamJid.bare())
			fromMe = true;

		QDomElement item=items.firstChildElement("item");
		if (!item.isNull()) // We have an item here!
		{
			itemId=item.attribute("id");
			if (!itemId.isNull())
			{
				QStringList k = itemId.split(":");
				if(k[0] == "#resource")
					userFrom.setResource(k[1]);
			}

			if (fromMe)
				FIdHash.insert(AStreamJid.full(), itemId);

			QDomElement geoloc=item.firstChildElement("geoloc");
			if (!geoloc.isNull())
			{
				GeolocElement geolocElement(geoloc);
				if (geolocElement.isEmpty())	// Empty Geoloc! Remove it from the hash!
					removeGeoloc(AStreamJid, userFrom);
				else
					putGeoloc(AStreamJid, userFrom, geolocElement);  // Put location into the list

				updateRosterLabels(userFrom);
				updateChatWindows(userFrom, AStreamJid);
				return true;
			}//--------------
			LOG_ERROR("No <geoloc /> element! WTF?");
		}
		else
		{    // No <item /> element: maybe it's <retract />?
			QDomElement retract=items.firstChildElement("retract");
			if (!retract.isNull())
			{
				if (fromMe)
					FIdHash.remove(AStreamJid);

				itemId=retract.attribute("id");
				if (!itemId.isNull())
				{
					QStringList k = itemId.split(":");
					if(k[0] == "#resource")
						userFrom.setResource(k[1]);
				}

				removeGeoloc(AStreamJid, userFrom);
				updateRosterLabels(userFrom);
				updateChatWindows(userFrom, AStreamJid);
				return true;
			}
		}
	}
	return false;
}

void Geoloc::registerDiscoFeatures()
{
	IDiscoFeature dfeature;
	dfeature.var = NS_PEP_GEOLOC;
	dfeature.active = true;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_GEOLOC);//
	dfeature.name = tr("User Location");
	dfeature.description = tr("Supports user Geolocation");
	FDiscovery->insertDiscoFeature(dfeature);

	dfeature.var.append(NODE_NOTIFY_SUFFIX);
	dfeature.name = tr("User Location Notification");
	dfeature.description = tr("Receives notification of current user Geolocation");
	FDiscovery->insertDiscoFeature(dfeature);

}

void Geoloc::sendGeoloc(const GeolocElement &APosition, const Jid &AStreamJid)
{
	if (FXmppStreams->findXmppStream(AStreamJid)->isConnected() && FPEPManager->isSupported(AStreamJid))
	{
		QDomDocument doc;
		QDomElement item=doc.createElement("item");
		item.setAttribute("id", QString("#resource:").append(AStreamJid.resource()));
		APosition.exportElement(item);
		FPEPManager->publishItem(AStreamJid, NS_PEP_GEOLOC, item);
	}
}

void Geoloc::updateChatWindows()
{
	if (FMessageWidgets)
	{
		QList<IMessageChatWindow *> chatWindows=FMessageWidgets->chatWindows();
		for(QList<IMessageChatWindow *>::const_iterator it=chatWindows.constBegin(); it!=chatWindows.constEnd(); it++)
			updateChatWindow(*it);
	}
}

void Geoloc::updateChatWindows(const Jid &AContactJid, const Jid &AStreamJid)
{
	if (FMessageWidgets)
	{
		IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
		if (window)
			updateChatWindow(window);
	}
}

void Geoloc::updateChatWindow(IMessageChatWindow *AMessageChatWindow)
{
	GeolocElement geoloc;
	if (FGeolocHash.contains(AMessageChatWindow->contactJid()))
		geoloc = FGeolocHash[AMessageChatWindow->contactJid()];
	else if (FGeolocHash.contains(AMessageChatWindow->contactJid().bare()))
		geoloc = FGeolocHash[AMessageChatWindow->contactJid().bare()];

	if (Options::node(OPV_MESSAGES_GEOLOC_DISPLAY).value().toBool() && !geoloc.isEmpty())
	{
		Action *action=NULL;
		QList<QAction *> actions=AMessageChatWindow->infoWidget()->infoToolBarChanger()->groupItems(AG_GEOLOC);
		if (actions.isEmpty())
		{
			action=new Action(AMessageChatWindow->infoWidget()->infoToolBarChanger()->toolBar());
			if (FMapContacts)
			{
				connect(action, SIGNAL(triggered()), FMapContacts->instance(), SLOT(onGeolocActionTriggered()));
				action->setShortcutId(SCT_MESSAGEWINDOWS_CHAT_SHOWCONTACTONTHEMAP);
			}
			AMessageChatWindow->infoWidget()->infoToolBarChanger()->insertAction(action, AG_GEOLOC);
		}
		else
			action=AMessageChatWindow->infoWidget()->infoToolBarChanger()->handleAction(actions[0]);
		action->setIcon(getIcon());
		action->setToolTip(getLabel(geoloc));
		action->setData(ADR_CONTACT_JID, AMessageChatWindow->contactJid().full());
	}
	else
	{
		QList<QAction *> actions=AMessageChatWindow->infoWidget()->infoToolBarChanger()->groupItems(AG_GEOLOC);
		if (!actions.isEmpty())
		{
			AMessageChatWindow->infoWidget()->infoToolBarChanger()->removeItem(actions[0]);
			actions[0]->deleteLater();
		}
	}
}

QString Geoloc::translate(const QString &APropertyName) const
{
	return FTranslated.contains(APropertyName)?FTranslated[APropertyName]:APropertyName;
}

void Geoloc::sendGeoloc(GeolocElement APosition)
{
	if (APosition.hasProperty("course"))	// Extra non-standard property from Positioning Provider Serial Port.
		APosition.removeProperty("course");
	for (QList<Jid>::ConstIterator it = FStreamsOnline.constBegin(); it!=FStreamsOnline.constEnd(); it++)
		if (FAccountManager->findAccountByStream(*it)->optionsNode().value(OPV_PUBLISHUSERLOCATION).toBool())
			sendGeoloc(APosition, *it);  //public data on server
}

void Geoloc::retractGeoloc()
{
	for (int i=0; i<FStreamsOnline.size(); i++)
	{
		QDomDocument doc;
		QDomElement item=doc.createElement("item");
		Jid streamJid=FStreamsOnline.at(i);
		if (FPEPManager->isSupported(streamJid))
		{
			item.setAttribute("id", FIdHash.value(streamJid, QString("#resource:").append(streamJid.resource())));

			if (Options::node(OPV_PEP_DELETE_PUBLISHEMPTY).value().toBool())
			{
				item.appendChild(doc.createElementNS(NS_PEP_GEOLOC, "geoloc"));
				FPEPManager->publishItem(streamJid, NS_PEP_GEOLOC, item);
			}
			if (Options::node(OPV_PEP_DELETE_RETRACT).value().toBool())
				FPEPManager->deleteItem(streamJid, NS_PEP_GEOLOC, item);
		}
	}
}

void Geoloc::updateRosterLabels(const Jid &AContactJid)
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
			if (checkRosterIndex(*it))
				FRostersViewPlugin->rostersView()->insertLabel(FRosterLabelId, *it);
			else
				FRostersViewPlugin->rostersView()->removeLabel(FRosterLabelId, *it);
		}
	}
}

QIcon Geoloc::getIcon() const
{
	return FIconStorage?FIconStorage->getIcon(MNI_GEOLOC):QIcon();
}

QString Geoloc::getIconFileName() const
{
	return FIconStorage?FIconStorage->fileFullName(MNI_GEOLOC):QString();
}

GeolocElement Geoloc::getGeoloc(const Jid &AJid) const
{
	return AJid.resource().isEmpty()?FGeolocBareHash.value(AJid.bare())
									:FGeolocHash.contains(AJid)?FGeolocHash.value(AJid)
															   :FGeolocHash.value(AJid.bare());
}

bool Geoloc::hasGeoloc(const Jid &AJid) const
{
	return AJid.resource().isEmpty()?FGeolocBareHash.contains(AJid.bare()):
									 FGeolocHash.contains(AJid)?true
															   :FGeolocHash.contains(AJid.bare());
}

bool Geoloc::checkRosterIndex(const IRosterIndex *AIndex) const
{
	QStringList resources = AIndex->data(RDR_RESOURCES).toStringList();
	if (resources.isEmpty())
		return hasGeoloc(AIndex->data(RDR_FULL_JID).toString());
	for (QStringList::ConstIterator it=resources.constBegin(); it!=resources.constEnd(); it++)
		if (hasGeoloc(*it))
			return true;
	return false;
}

QString Geoloc::getLabel(const Jid &AContactJid) const
{
	return getLabel(getGeoloc(AContactJid));
}

QString Geoloc::getLabel(const GeolocElement &AGeoloc) const
{
	qreal lat=Q_SNAN;
	qreal lon=Q_SNAN;
	QString latd;
	QString lond;
	QColor  color;

	if (AGeoloc.isEmpty())
		return QString();

	switch (AGeoloc.reliability())
	{
		case GeolocElement::Reliable:
			color.setNamedColor("green");
			break;
		case GeolocElement::WasReliable:
			color.setNamedColor("red");
			break;
		case GeolocElement::NotReliable:
			color.setNamedColor("brown");
			break;
		case GeolocElement::Unknown:
			color.setNamedColor("black");
			break;
	}

	if(AGeoloc.hasProperty(GeolocElement::Lat))
	{
		lat=AGeoloc.lat();
		if (lat<0)
		{
			latd=tr("S");
			lat=-lat;
		}
		else
			latd=tr("N");
	}

	if(AGeoloc.hasProperty(GeolocElement::Lon))
	{
		lon=AGeoloc.lon();
		if (lon<0)
		{
			lond=tr("W");
			lon=-lon;
		}
		else
			lond=tr("E");
	}

	QString locationString = QString("%1%2;&nbsp;%3%4").arg(lat,0,'f',6)
													   .arg(latd)
													   .arg(lon,0,'f',6)
													   .arg(lond);

	if(AGeoloc.hasProperty(GeolocElement::Alt))
		locationString.append(QString(";&nbsp;%1%2")
							  .arg(AGeoloc.alt())
							  .arg(tr("m", "Short for \"meters\"")));

	QString textLine=QString("<img src='%1' />&nbsp;<font color=%2>%3</font>")
			.arg(FIconStorage->fileFullName(MNI_GEOLOC))
			.arg(color.name())
			.arg(locationString);

	if(AGeoloc.hasProperty(GeolocElement::Text))
		textLine.append(QString("<br><img src=\"%1\" /> %2")
						.arg(FIconStorage->fileFullName(MNI_DESCRIPTION))
						.arg(AGeoloc.text()));

	if(AGeoloc.hasProperty(GeolocElement::Description))
		textLine.append(QString("<br><img src=\"%1\" /> %2")
						.arg(FIconStorage->fileFullName(MNI_DESCRIPTION))
						.arg(AGeoloc.description()));

	if (AGeoloc.hasProperty(GeolocElement::Uri))
		textLine.append(QString("<br><img src=\"%1\" /> <a href=\"%2\">%2</a>")
						.arg(FIconStorage->fileFullName(MNI_LINK))
						.arg(AGeoloc.uri().toString()));

	if (AGeoloc.hasProperty(GeolocElement::CountryCode))
		if (AGeoloc.hasProperty(GeolocElement::Country))
			textLine.append(QString("<br>%1: %2 (%3)").arg(translate("country")).arg(AGeoloc.propertyAsString(GeolocElement::CountryCode)).arg(AGeoloc.propertyAsString(GeolocElement::Country)));
		else
			textLine.append(QString("<br>%1: %2").arg(translate(translate("country"))).arg(AGeoloc.propertyAsString(GeolocElement::CountryCode)));
	else
		if (AGeoloc.hasProperty(GeolocElement::Country))
			textLine.append(QString("<br>%1: %2").arg(translate(translate("country"))).arg(AGeoloc.propertyAsString(GeolocElement::Country)));

	const QStringList properties = AGeoloc.propertyNames();
	for(QStringList::ConstIterator it=properties.constBegin(); it != properties.constEnd(); it++)
	{
		if(!((*it=="lat") || (*it=="lon") || (*it=="alt") ||
			 (*it=="timestamp") || (*it=="uri") || (*it=="description") ||
			 (*it=="text") || (*it=="country") || (*it=="countrycode")))
			textLine.append(QString("<br>%1: %2").arg(translate(*it)).arg(AGeoloc.propertyAsString(*it)));
	}

	if (AGeoloc.hasProperty(GeolocElement::TimeStamp))
		textLine.append(QString("<br><img src=\"%1\" />%2")
						.arg(FIconStorage->fileFullName(MNI_CLIENTINFO_TIME))
						.arg(AGeoloc.timeStamp().toLocalTime().toString(Qt::DefaultLocaleShortDate)));

	return QString("<strong>%1:</strong><br>%2").arg(tr("Location")).arg(textLine);
}

void Geoloc::putGeoloc(const Jid &AStreamJid, const Jid &AContactJid, const GeolocElement &AGeolocElement)
{
	double lat = AGeolocElement.lat();
	if(lat < -90  || lat > 90 ) return;
	double lon = AGeolocElement.lon();
	if(lon < -180  || lon > 180 ) return;

	GeolocElement::Reliability reliable = GeolocElement::Unknown;
	if (FGeolocHash.contains(AContactJid))
		reliable=FGeolocHash[AContactJid].reliability();
	FGeolocHash.insert(AContactJid, AGeolocElement);
	FGeolocBareHash.insert(AContactJid.bare(), AGeolocElement);
	MercatorCoordinates coords(lat, lon);
	emit locationReceived(AStreamJid, AContactJid, coords, AGeolocElement.reliability()!=reliable);
}

void Geoloc::removeGeoloc(const Jid &AStreamJid, const Jid &AContactJid)
{
	FGeolocHash.remove(AContactJid);
	emit locationRemoved(AStreamJid, AContactJid);
}

//----SLOTS----
void Geoloc::onStreamOpened(IXmppStream *AXmppStream)
{
	FStreamsOnline.append(AXmppStream->streamJid());
}

void Geoloc::onStreamClosed(IXmppStream *AXmppStream)
{
	FStreamsOnline.removeOne(AXmppStream->streamJid());
}

void Geoloc::onRosterIndexInserted(IRosterIndex *AIndex)
{
	if (FRostersViewPlugin &&  FRosterIndexKinds.contains(AIndex->kind()) &&
		(Options::node(OPV_COMMON_ADVANCED).value().toBool()?Options::node(OPV_ROSTER_GEOLOC_SHOW).value().toBool():Options::node(OPV_ROSTER_VIEWMODE).value().toInt()==IRostersView::ViewFull) &&
		checkRosterIndex(AIndex))
	FRostersViewPlugin->rostersView()->insertLabel(FRosterLabelId, AIndex);
}

void Geoloc::onRosterIndexDataChanged(IRosterIndex *AIndex, int ARole)
{
	if (ARole==RDR_FULL_JID && Options::node(OPV_ROSTER_GEOLOC_SHOW).value().toBool() && FRostersViewPlugin &&  FRosterIndexKinds.contains(AIndex->kind()))
	{
		if (checkRosterIndex(AIndex))
			FRostersViewPlugin->rostersView()->insertLabel(FRosterLabelId, AIndex);
		else
			FRostersViewPlugin->rostersView()->removeLabel(FRosterLabelId, AIndex);
	}
}

void Geoloc::onRosterIndexToolTips(IRosterIndex *AIndex, quint32 ALabelId, QMap<int, QString> &AToolTips)
{
	if ((ALabelId == AdvancedDelegateItem::DisplayId || ALabelId == FRosterLabelId)
		 && FRosterIndexKinds.contains(AIndex->kind()))
	{
		Jid jid;
		QStringList resources = AIndex->data(RDR_RESOURCES).toStringList();
		if (resources.isEmpty())
			jid=AIndex->data(RDR_FULL_JID).toString();
		else
			for (QStringList::ConstIterator it=resources.constBegin(); it!=resources.constEnd(); it++)
				if (hasGeoloc(*it))
				{
					jid=*it;
					break;
				}

		if (!jid.isEmpty())
		{
			AToolTips.insert(RTTO_GEOLOC_SEPARATOR, "<hr>");
			AToolTips.insert(RTTO_GEOLOC, getLabel(jid));
		}
	}
}

void Geoloc::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_ROSTER_RECENT_SIMPLEITEMSVIEW));
}



void Geoloc::onRosterIndexClipboardMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
	if (ALabelId == AdvancedDelegateItem::DisplayId || ALabelId == FRosterLabelId)
		for (QList<IRosterIndex *>::const_iterator it=AIndexes.constBegin(); it!=AIndexes.constEnd(); it++)
		{
			Jid jid((*it)->data(RDR_FULL_JID).toString());
			if (hasGeoloc(jid))
			{
				Action *action = new Action(AMenu);
				GeolocElement position = getGeoloc(jid);

				qreal lat, lon;
				QString latd, lond;
				if(position.hasProperty(GeolocElement::Lat))
				{
					lat=position.lat();
					if (lat<0)
					{
						latd=tr("S");
						lat=-lat;
					}
					else
						latd=tr("N");
				}

				if(position.hasProperty(GeolocElement::Lon))
				{
					lon=position.lon();
					if (lon<0)
					{
						lond=tr("W");
						lon=-lon;
					}
					else
						lond=tr("E");
				}

				action->setText(QString("%1%2; %3%4").arg(lat,0,'f',6).arg(latd)
														  .arg(lon,0,'f',6).arg(lond));
				action->setIcon(getIcon());

				QHash<QString, QVariant> data = position.properties();
				if (position.reliability()!=GeolocElement::Unknown)
					data.insert("reliability", position.reliability());
				action->setData(ADR_CLIPBOARD_DATA, data);

				connect(action, SIGNAL(triggered()), SLOT(onCopyToClipboard()));
				AMenu->addAction(action, AG_RVCBM_PEP, true);
			}
		}
}

void Geoloc::onCopyToClipboard()
{
	QHash<QString, QVariant> position = qobject_cast<Action *>(sender())->data(ADR_CLIPBOARD_DATA).toHash();

	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime;
	mime = new QMimeData();

	QString text, html;
	QString color;

	if (position.contains("reliability"))
		switch (position["reliability"].toInt())
		{
			case GeolocElement::Reliable:
				color="green";
				break;
			case GeolocElement::WasReliable:
				color="red";
				break;
			case GeolocElement::NotReliable:
				color="brown";
				break;
			case GeolocElement::Unknown:
				break;
		}

	if (position.contains("lat"))
	{
		qreal lat=position["lat"].toDouble();
		QString latd;
		if (lat<0)
		{
			latd=tr("S");
			lat=-lat;
		}
		else
			latd=tr("N");
		text.append(tr("%1: %2%3").arg(translate("lat")).arg(lat).arg(latd));
		html.append(tr("<b>%1:</b><span %4>&nbsp;%2%3</span>").arg(translate("lat")).arg(lat).arg(latd).arg(color.isEmpty()?QString(""):QString("style=\"color:%1;\"").arg(color)));
	}
	if (position.contains("lon"))
	{
		qreal lon=position["lon"].toDouble();
		QString lond;
		if (lon<0)
		{
			lond=tr("W");
			lon=-lon;
		}
		else
			lond=tr("E");
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2%3").arg(translate("lon")).arg(lon).arg(lond));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b><span %4>&nbsp;%2%3</span>").arg(translate("lon")).arg(lon).arg(lond).arg(color.isEmpty()?QString(""):QString("style=\"color:%1;\"").arg(color)));
	}
	if (position.contains("alt"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(translate("alt")).arg(position["alt"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b><span %3>&nbsp;%2</span>").arg(translate("alt")).arg(position["alt"].toFloat()).arg(color.isEmpty()?QString(""):QString("style=\"color:%1;\"").arg(color)));
	}
	if (position.contains("text"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(translate("text")).arg(position["text"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b> %2").arg(translate("text")).arg(position["text"].toString()));
	}
	if (position.contains("description"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(translate("description")).arg(position["description"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b>&nbsp;%2").arg(translate("description")).arg(position["description"].toString()));
	}
	if (position.contains("postalcode"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(translate("postalcode")).arg(position["postalcode"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b>&nbsp;%2").arg(translate("postalcode")).arg(position["postalcode"].toString()));
	}
	if (position.contains("countrycode"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(translate("countrycode")).arg(position["countrycode"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b>&nbsp;%2").arg(translate("countrycode")).arg(position["countrycode"].toString()));
	}
	if (position.contains("country"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(translate("country")).arg(position["country"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b>&nbsp;%2").arg(translate("country")).arg(position["country"].toString()));
	}
	if (position.contains("region"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(translate("region")).arg(position["region"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b>&nbsp;%2").arg(translate("region")).arg(position["region"].toString()));
	}
	if (position.contains("locality"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(translate("locality")).arg(position["locality"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b>&nbsp;%2").arg(translate("locality")).arg(position["locality"].toString()));
	}
	if (position.contains("area"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(translate("area")).arg(position["area"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b>&nbsp;%2").arg(translate("area")).arg(position["area"].toString()));
	}
	if (position.contains("street"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(translate("street")).arg(position["street"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b>&nbsp;%2").arg(translate("street")).arg(position["street"].toString()));
	}
	if (position.contains("building"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(translate("building")).arg(position["building"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b>&nbsp;%2").arg(translate("building")).arg(position["building"].toString()));
	}
	if (position.contains("floor"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(translate("floor")).arg(position["floor"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b>&nbsp;%2").arg(translate("floor")).arg(position["floor"].toString()));
	}
	if (position.contains("room"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(translate("room")).arg(position["room"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b>&nbsp;%2").arg(translate("room")).arg(position["room"].toString()));
	}
	for(GeolocElement::ConstIterator it=position.constBegin(); it != position.constEnd(); it++)
	{
		if(!((it.key()=="lat") || (it.key()=="lon") || (it.key()=="alt") ||
			 (it.key()=="timestamp") || (it.key()=="uri") || (it.key()=="description") ||
			 (it.key()=="text") || (it.key()=="countrycode") || (it.key()=="country") ||
			 (it.key()=="region") || (it.key()=="locality") || (it.key()=="street") ||
			 (it.key()=="area") || (it.key()=="building") || (it.key()=="floor") ||
			 (it.key()=="room") || (it.key()=="reliability")))
		{
			if (!text.isEmpty())
				text.append(QChar::LineSeparator);
			text.append(QString("<br>%1: %2").arg(translate(it.key())).arg((*it).toString()));
			if (!html.isEmpty())
				html.append("<br/>");
			html.append(tr("<b>%1:</b>&nbsp;%2").arg(translate(it.key())).arg((*it).toString()));
		}
	}
	if (position.contains("uri"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		text.append(tr("%1: %2").arg(translate("uri")).arg(position["uri"].toString()));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b>&nbsp;<a href=\"%2\">%2</a>").arg(translate("uri")).arg(position["uri"].toString()));
		mime->setUrls(QList<QUrl>() << position["uri"].toUrl());
	}
	if (position.contains("timestamp"))
	{
		if (!text.isEmpty())
			text.append(QChar::LineSeparator);
		QString timestamp = position["timestamp"].toDateTime().toLocalTime().toString(Qt::DefaultLocaleShortDate);
		text.append(tr("%1: %2").arg(translate("timestamp")).arg(timestamp));
		if (!html.isEmpty())
			html.append("<br/>");
		html.append(tr("<b>%1:</b>&nbsp;%2").arg(translate("timestamp")).arg(timestamp));
	}

	if (!text.isEmpty())
		mime->setText(text);
	if (!html.isEmpty())
		mime->setHtml(QString("<body>%1</body>").arg(html));

	clipboard->setMimeData(mime);
}

// Options Manager
void Geoloc::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_MESSAGES_GEOLOC_DISPLAY)
		updateChatWindows();
	else if (ANode.path() == OPV_ROSTER_RECENT_SIMPLEITEMSVIEW)
	{
		FSimpleContactsView = ANode.value().toBool();
		QMultiMap<int, QVariant> findData;
		findData.insert(RDR_KIND, RIK_RECENT_ITEM);
		QList<IRosterIndex *> indexes = FRostersModel->rootIndex()->findChilds(findData, true);
		for (QList<IRosterIndex *>::ConstIterator it = indexes.constBegin(); it!=indexes.constEnd(); it++)
			emit rosterLabelChanged(RLID_GEOLOC, *it);
	}
	else if (ANode.path() == (Options::node(OPV_COMMON_ADVANCED).value().toBool()?OPV_ROSTER_GEOLOC_SHOW:OPV_ROSTER_VIEWMODE))
	{
		if (FRostersViewPlugin && FRostersModel)
		{
			if (ANode.path() == OPV_ROSTER_GEOLOC_SHOW?ANode.value().toBool():ANode.value().toInt() == IRostersView::ViewFull)
			{
				QMultiMap<int,QVariant> findData;
				for (QList<int>::const_iterator it=FRosterIndexKinds.begin(); it!=FRosterIndexKinds.end(); it++)
					findData.insertMulti(RDR_KIND, *it);
				QList<IRosterIndex *> indexes = FRostersModel->rootIndex()->findChilds(findData,true);
				for (QList<IRosterIndex *>::const_iterator it=indexes.constBegin(); it!=indexes.constEnd(); it++)
					if (checkRosterIndex(*it))
					   FRostersViewPlugin->rostersView()->insertLabel(FRosterLabelId, *it);
			}
			else
				FRostersViewPlugin->rostersView()->removeLabel(FRosterLabelId);
		}
	}
}

void Geoloc::onChatWindowCreated(IMessageChatWindow *AWindow)
{
	updateChatWindow(AWindow);
	connect(AWindow->address()->instance(), SIGNAL(addressChanged(Jid, Jid)), SLOT(onAddressChanged(Jid,Jid)));
}

void Geoloc::onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore)
{
	Q_UNUSED(AStreamBefore)
	Q_UNUSED(AContactBefore)

	IMessageAddress *address=qobject_cast<IMessageAddress *>(sender());
	IMessageChatWindow *window=FMessageWidgets->findChatWindow(address->streamJid(), address->contactJid());
	if (window)
		updateChatWindow(window);
}

void Geoloc::onPresenceActiveChanged(IPresence *APresence, bool AActive)
{
	if (FPositioning && AActive)
	{
		GeolocElement position = FPositioning->currentPosition();
		if (position.isValid())
			if (FAccountManager->findAccountByStream(APresence->streamJid())->optionsNode().value(OPV_PUBLISHUSERLOCATION).toBool())
				sendGeoloc(position, APresence->streamJid());
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_geoloc, Geoloc)
#endif
