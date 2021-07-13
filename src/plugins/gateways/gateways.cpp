#include "gateways.h"

#include <QMessageBox>
#include <QTextDocument>
#include <definitions/namespaces.h>
#include <definitions/actiongroups.h>
#include <definitions/toolbargroups.h>
#include <definitions/rosterindexkinds.h>
#include <definitions/rosterindexroles.h>
#include <definitions/discofeaturehandlerorders.h>
#include <definitions/vcardvaluenames.h>
#include <definitions/discoitemdataroles.h>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <utils/xmpperror.h>
#include <utils/stanza.h>
#include <utils/action.h>
#include <utils/logger.h>
#include <utils/qt4qt5compat.h>

#define GATEWAY_TIMEOUT           30000

#define ADR_STREAM_JID            Action::DR_StreamJid
#define ADR_SERVICE_JID           Action::DR_Parametr1
#define ADR_NEW_SERVICE_JID       Action::DR_Parametr2
#define ADR_LOG_IN                Action::DR_Parametr3
#define ADR_ACTION_RESOLVE        Action::DR_Parametr4 /*** <<< eyeCU >>> ***/

#define PSN_GATEWAYS_KEEP         "vacuum:gateways:keep"
#define PSN_GATEWAYS_SUBSCRIBE    "vacuum:gateways:subscribe"
#define PST_GATEWAYS_SERVICES     "services"

#define KEEP_INTERVAL             120000

Gateways::Gateways()
{
	FDiscovery = NULL;
	FStanzaProcessor = NULL;
	FRosterManager = NULL;
	FPresenceManager = NULL;
	FRosterChanger = NULL;
	FRostersViewPlugin = NULL;
	FVCardManager = NULL;
	FPrivateStorage = NULL;
	FStatusIcons = NULL;
	FRegistration = NULL;

	FKeepTimer.setSingleShot(false);
	FKeepTimer.setInterval(KEEP_INTERVAL);
	connect(&FKeepTimer,SIGNAL(timeout()),SLOT(onKeepTimerTimeout()));
}

Gateways::~Gateways()
{

}

void Gateways::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Gateway Interaction");
	APluginInfo->description = tr("Allows to simplify the interaction with transports to other IM systems");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Potapov S.A. aka Lion";
	APluginInfo->homePage = "http://www.vacuum-im.org";
	APluginInfo->dependences.append(STANZAPROCESSOR_UUID);
}

bool Gateways::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);
	IPlugin *plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
	{
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());
		if (FDiscovery)
		{
			connect(FDiscovery->instance(),SIGNAL(discoItemsWindowCreated(IDiscoItemsWindow *)),
				SLOT(onDiscoItemsWindowCreated(IDiscoItemsWindow *)));
		}
	}

	plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0,NULL);
	if (plugin)
	{
		FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IRosterManager").value(0,NULL);
	if (plugin)
	{
		FRosterManager = qobject_cast<IRosterManager *>(plugin->instance());
		if (FRosterManager)
		{
			connect(FRosterManager->instance(),SIGNAL(rosterOpened(IRoster *)),SLOT(onRosterOpened(IRoster *)));
			connect(FRosterManager->instance(),SIGNAL(rosterSubscriptionReceived(IRoster *, const Jid &, int , const QString &)),
				SLOT(onRosterSubscriptionReceived(IRoster *, const Jid &, int , const QString &)));
			connect(FRosterManager->instance(),SIGNAL(rosterStreamJidAboutToBeChanged(IRoster *, const Jid &)),
				SLOT(onRosterStreamJidAboutToBeChanged(IRoster *, const Jid &)));
		}
	}

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0,NULL);
	if (plugin)
	{
		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());
		if (FPresenceManager)
		{
			connect(FPresenceManager->instance(),SIGNAL(presenceOpened(IPresence *)),SLOT(onPresenceOpened(IPresence *)));
			connect(FPresenceManager->instance(),SIGNAL(presenceClosed(IPresence *)),SLOT(onPresenceClosed(IPresence *)));
			connect(FPresenceManager->instance(),SIGNAL(contactStateChanged(const Jid &, const Jid &, bool)),
				SLOT(onContactStateChanged(const Jid &, const Jid &, bool)));
			connect(FPresenceManager->instance(),SIGNAL(presenceActiveChanged(IPresence *, bool)),SLOT(onPresenceActiveChanged(IPresence *, bool)));
		}
	}

	plugin = APluginManager->pluginInterface("IRosterChanger").value(0,NULL);
	if (plugin)
	{
		FRosterChanger = qobject_cast<IRosterChanger *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IRostersViewPlugin").value(0,NULL);
	if (plugin)
	{
		FRostersViewPlugin = qobject_cast<IRostersViewPlugin *>(plugin->instance());
		if (FRostersViewPlugin)
		{
			connect(FRostersViewPlugin->rostersView()->instance(),SIGNAL(indexMultiSelection(const QList<IRosterIndex *> &, bool &)),
				SLOT(onRostersViewIndexMultiSelection(const QList<IRosterIndex *> &, bool &)));
			connect(FRostersViewPlugin->rostersView()->instance(),SIGNAL(indexContextMenu(const QList<IRosterIndex *> &, quint32, Menu *)),
				SLOT(onRostersViewIndexContextMenu(const QList<IRosterIndex *> &, quint32, Menu *)));
		}
	}

	plugin = APluginManager->pluginInterface("IVCardManager").value(0,NULL);
	if (plugin)
	{
		FVCardManager = qobject_cast<IVCardManager *>(plugin->instance());
		if (FVCardManager)
		{
			connect(FVCardManager->instance(),SIGNAL(vcardReceived(const Jid &)),SLOT(onVCardReceived(const Jid &)));
			connect(FVCardManager->instance(),SIGNAL(vcardError(const Jid &, const XmppError &)),SLOT(onVCardError(const Jid &, const XmppError &)));
		}
	}

	plugin = APluginManager->pluginInterface("IPrivateStorage").value(0,NULL);
	if (plugin)
	{
		FPrivateStorage = qobject_cast<IPrivateStorage *>(plugin->instance());
		if (FPrivateStorage)
		{
			connect(FPrivateStorage->instance(),SIGNAL(storageOpened(const Jid &)),SLOT(onPrivateStorateOpened(const Jid &)));
			connect(FPrivateStorage->instance(),SIGNAL(dataLoaded(const QString &, const Jid &, const QDomElement &)),
				SLOT(onPrivateDataLoaded(const QString &, const Jid &, const QDomElement &)));
			connect(FPrivateStorage->instance(),SIGNAL(dataChanged(const Jid &, const QString &, const QString &)),
				SLOT(onPrivateDataChanged(const Jid &, const QString &, const QString &)));
		}
	}

	plugin = APluginManager->pluginInterface("IStatusIcons").value(0,NULL);
	if (plugin)
	{
		FStatusIcons = qobject_cast<IStatusIcons *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IRegistration").value(0,NULL);
	if (plugin)
	{
		FRegistration = qobject_cast<IRegistration *>(plugin->instance());
		if (FRegistration)
		{
			connect(FRegistration->instance(),SIGNAL(registerFields(const QString &, const IRegisterFields &)),
				SLOT(onRegisterFields(const QString &, const IRegisterFields &)));
			connect(FRegistration->instance(),SIGNAL(registerError(const QString &, const XmppError &)),
				SLOT(onRegisterError(const QString &, const XmppError &)));
		}
	}

	return FStanzaProcessor!=NULL;
}

bool Gateways::initObjects()
{
	if (FDiscovery)
	{
		registerDiscoFeatures();
		FDiscovery->insertFeatureHandler(NS_JABBER_GATEWAY,this,DFO_DEFAULT);
	}
	return true;
}

bool Gateways::startPlugin()
{
	FKeepTimer.start(KEEP_INTERVAL);
	return true;
}

void Gateways::stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza)
{
	Q_UNUSED(AStreamJid);
	if (FPromptRequests.contains(AStanza.id()))
	{
		if (AStanza.isResult())
		{
			LOG_STRM_DEBUG(AStreamJid,QString("Legacy user prompt received, id=%1").arg(AStanza.id()));
			QString desc = AStanza.firstElement("query",NS_JABBER_GATEWAY).firstChildElement("desc").text();
			QString prompt = AStanza.firstElement("query",NS_JABBER_GATEWAY).firstChildElement("prompt").text();
			emit promptReceived(AStanza.id(),desc,prompt);
		}
		else
		{
			XmppStanzaError err(AStanza);
			LOG_STRM_WARNING(AStreamJid,QString("Failed to receive legacy user prompt, id=%1: %2").arg(AStanza.id(),err.condition()));
			emit errorReceived(AStanza.id(),err);
		}
		FPromptRequests.removeAll(AStanza.id());
	}
	else if (FUserJidRequests.contains(AStanza.id()))
	{
		if (AStanza.isResult())
		{
			LOG_STRM_DEBUG(AStreamJid,QString("Legacy user JID received, id=%1").arg(AStanza.id()));
			Jid userJid = AStanza.firstElement("query",NS_JABBER_GATEWAY).firstChildElement("jid").text();
			emit userJidReceived(AStanza.id(),userJid);
		}
		else
		{
			XmppStanzaError err(AStanza);
			LOG_STRM_WARNING(AStreamJid,QString("Failed to receive legacy user JID, id=%1: %2").arg(AStanza.id(),err.condition()));
			emit errorReceived(AStanza.id(),err);
		}
		FUserJidRequests.removeAll(AStanza.id());
	}
}

bool Gateways::execDiscoFeature(const Jid &AStreamJid, const QString &AFeature, const IDiscoInfo &ADiscoInfo)
{
	if (AFeature == NS_JABBER_GATEWAY)
		return showAddLegacyContactDialog(AStreamJid,ADiscoInfo.contactJid)!=NULL;
	return false;
}

Action *Gateways::createDiscoFeatureAction(const Jid &AStreamJid, const QString &AFeature, const IDiscoInfo &ADiscoInfo, QWidget *AParent)
{
	IPresence *presence = FPresenceManager!=NULL ? FPresenceManager->findPresence(AStreamJid) : NULL;
	if (presence && presence->isOpen() && AFeature==NS_JABBER_GATEWAY)
	{
		Action *action = new Action(AParent);
		action->setText(tr("Add Legacy User"));
		action->setIcon(RSR_STORAGE_MENUICONS,MNI_GATEWAYS_ADD_CONTACT);
		action->setData(ADR_STREAM_JID,AStreamJid.full());
		action->setData(ADR_SERVICE_JID,ADiscoInfo.contactJid.full());
		connect(action,SIGNAL(triggered(bool)),SLOT(onAddLegacyUserActionTriggered(bool)));
		return action;
	}
	return NULL;
}

void Gateways::resolveNickName(const Jid &AStreamJid, const Jid &AContactJid)
{
	IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(AStreamJid) : NULL;
	IRosterItem ritem = roster!=NULL ? roster->findItem(AContactJid) : IRosterItem();
	if (!ritem.isNull() && roster->isOpen())
	{
		if (FVCardManager->hasVCard(ritem.itemJid))
		{
			static const QList<QString> nickFields = QList<QString>() << VVN_NICKNAME << VVN_FULL_NAME << VVN_GIVEN_NAME << VVN_FAMILY_NAME;
			LOG_STRM_INFO(AStreamJid,QString("Resolving contact nick name from vCard, jid=%1").arg(AContactJid.bare()));

			IVCard *vcard = FVCardManager->getVCard(ritem.itemJid);
			foreach(const QString &field, nickFields)
			{
				QString nick = vcard->value(field);
				if (!nick.isEmpty())
				{
					if (ritem.name != nick)
						roster->renameItem(ritem.itemJid,nick);
					break;
				}
			}
			vcard->unlock();
		}
		else
		{
			LOG_STRM_INFO(AStreamJid,QString("Requesting contact vCard to resolve nick name, jid=%1").arg(AContactJid.bare()));
			if (!FResolveNicks.contains(ritem.itemJid))
				FVCardManager->requestVCard(AStreamJid,ritem.itemJid);
			FResolveNicks.insertMulti(ritem.itemJid,AStreamJid);
		}
	}
	else if (ritem.isNull())
	{
		LOG_STRM_ERROR(AStreamJid,QString("Failed to resolve contact nick name, jid=%1: Contact not found").arg(AContactJid.bare()));
	}
}

/*** <<< eyeCU <<< ***/
void Gateways::resetNickName(const Jid &AStreamJid, const Jid &AContactJid)
{
	IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(AStreamJid) : NULL;
	IRosterItem ritem = roster!=NULL ? roster->findItem(AContactJid) : IRosterItem();
	if (!ritem.isNull() && !ritem.name.isEmpty() && roster->isOpen())
	{
		roster->renameItem(ritem.itemJid,QString());
	}
	else if (ritem.isNull())
	{
		LOG_STRM_ERROR(AStreamJid,QString("Failed to reset contact nick name, jid=%1: Contact not found").arg(AContactJid.bare()));
	}
}
/*** >>> eyeCU >>> ***/

void Gateways::sendLogPresence(const Jid &AStreamJid, const Jid &AServiceJid, bool ALogIn)
{
	IPresence *presence = FPresenceManager!=NULL ? FPresenceManager->findPresence(AStreamJid) : NULL;
	if (presence && presence->isOpen())
	{
		if (ALogIn)
			presence->sendPresence(AServiceJid,presence->show(),presence->status(),presence->priority());
		else
			presence->sendPresence(AServiceJid,IPresence::Offline,tr("Log Out"),0);
	}
}

QList<Jid> Gateways::keepConnections(const Jid &AStreamJid) const
{
	return FKeepConnections.values(AStreamJid);
}

void Gateways::setKeepConnection(const Jid &AStreamJid, const Jid &AServiceJid, bool AEnabled)
{
	IPresence *presence = FPresenceManager!=NULL ? FPresenceManager->findPresence(AStreamJid) : NULL;
	if (presence)
	{
		if (AEnabled)
			FKeepConnections.insertMulti(presence->streamJid(),AServiceJid);
		else
			FKeepConnections.remove(presence->streamJid(),AServiceJid);
	}
}

QList<Jid> Gateways::streamServices(const Jid &AStreamJid, const IDiscoIdentity &AIdentity) const
{
	QList<Jid> services;
	IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(AStreamJid) : NULL;
	QList<IRosterItem> ritems = roster!=NULL ? roster->items() : QList<IRosterItem>();
	foreach(const IRosterItem &ritem, ritems)
	{
		if (!ritem.itemJid.hasNode())
		{
			if (FDiscovery && (!AIdentity.category.isEmpty() || !AIdentity.type.isEmpty()))
			{
				IDiscoInfo dinfo = FDiscovery->discoInfo(AStreamJid, ritem.itemJid);
				foreach(const IDiscoIdentity &identity, dinfo.identity)
				{
					if ((AIdentity.category.isEmpty() || AIdentity.category == identity.category) && (AIdentity.type.isEmpty() || AIdentity.type == identity.type))
					{
						services.append(ritem.itemJid);
						break;
					}
				}
			}
			else
			{
				services.append(ritem.itemJid);
			}
		}
	}
	return services;
}

QList<Jid> Gateways::serviceContacts(const Jid &AStreamJid, const Jid &AServiceJid) const
{
	QList<Jid> contacts;
	IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(AStreamJid) : NULL;
	QList<IRosterItem> ritems = roster!=NULL ? roster->items() : QList<IRosterItem>();
	foreach(const IRosterItem &ritem, ritems)
		if (ritem.itemJid.hasNode() && ritem.itemJid.pDomain()==AServiceJid.pDomain())
			contacts.append(ritem.itemJid);
	return contacts;
}

bool Gateways::removeService(const Jid &AStreamJid, const Jid &AServiceJid, bool AWithContacts)
{
	IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(AStreamJid) : NULL;
	if (roster && roster->isOpen())
	{
		LOG_STRM_INFO(AStreamJid,QString("Removing service=%1 with contacts=%2").arg(AServiceJid.bare()).arg(AWithContacts));
		sendLogPresence(AStreamJid,AServiceJid,false);

		if (FRosterChanger)
			FRosterChanger->insertAutoSubscribe(AStreamJid, AServiceJid, true, false, true);
		if (FRegistration)
			FRegistration->sendUnregisterRequest(AStreamJid,AServiceJid);
		roster->removeItem(AServiceJid);

		if (AWithContacts)
		{
			foreach(const Jid &contactJid, serviceContacts(AStreamJid,AServiceJid))
			{
				if (FRosterChanger)
					FRosterChanger->insertAutoSubscribe(AStreamJid, contactJid, true, false, true);
				roster->removeItem(contactJid);
			}
		}
		return true;
	}
	else if (roster)
	{
		LOG_STRM_ERROR(AStreamJid,QString("Failed to remove service=%1: Roster not opened").arg(AServiceJid.bare()));
	}
	return false;
}

bool Gateways::changeService(const Jid &AStreamJid, const Jid &AServiceFrom, const Jid &AServiceTo, bool ARemove, bool ASubscribe)
{
	IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(AStreamJid) : NULL;
	IPresence *presence = FPresenceManager!=NULL ? FPresenceManager->findPresence(AStreamJid) : NULL;
	if (FRosterChanger && roster && presence &&  presence->isOpen() && AServiceFrom.isValid() && AServiceTo.isValid() && AServiceFrom.pDomain()!=AServiceTo.pDomain())
	{
		LOG_STRM_INFO(AStreamJid,QString("Changing service from=%1 to=%2, remove=%2, subscribe=%4").arg(AServiceFrom.bare(),AServiceTo.bare()).arg(ARemove).arg(ASubscribe));

		IRosterItem ritemOld = roster->findItem(AServiceFrom);
		IRosterItem ritemNew = roster->findItem(AServiceTo);

		//Logout on old service
		if (!presence->findItems(AServiceFrom).isEmpty())
			sendLogPresence(AStreamJid,AServiceFrom,false);

		//Remove registration on old service
		if (FRegistration && ARemove)
			FRegistration->sendUnregisterRequest(AStreamJid,AServiceFrom);

		//Remove subscription from old service
		if (!ritemOld.isNull() && !ARemove)
			FRosterChanger->unsubscribeContact(AStreamJid,AServiceFrom,QString(),true);

		//Adding contact of old service to new
		QList<IRosterItem> newItems, oldItems, curItems;
		foreach(const IRosterItem &ritem, roster->items())
		{
			if (ritem.itemJid.pDomain() == AServiceFrom.pDomain())
			{
				IRosterItem newItem = ritem;
				newItem.itemJid.setDomain(AServiceTo.domain());
				if (roster->findItem(newItem.itemJid).isNull())
					newItems.append(newItem);
				else
					curItems += newItem;
				if (ARemove)
				{
					oldItems.append(ritem);
					FRosterChanger->insertAutoSubscribe(AStreamJid, ritem.itemJid, true, false, true);
				}
			}
		}
		roster->removeItems(oldItems);
		roster->setItems(newItems);

		//Request subscription from new service and its contacts
		if (ASubscribe)
		{
			FSubscribeServices.remove(AStreamJid,AServiceFrom.bare());
			FSubscribeServices.insertMulti(AStreamJid,AServiceTo.bare());
			savePrivateStorageSubscribe(AStreamJid);

			curItems+=newItems;
			foreach(const IRosterItem &ritem, curItems)
				FRosterChanger->insertAutoSubscribe(AStreamJid,ritem.itemJid, true, true, false);
			FRosterChanger->insertAutoSubscribe(AStreamJid,AServiceTo,true,true,false);
			roster->sendSubscription(AServiceTo,IRoster::Subscribe);
		}
		else if (FSubscribeServices.contains(AStreamJid,AServiceFrom.bare()))
		{
			FSubscribeServices.remove(AStreamJid,AServiceFrom.bare());
			savePrivateStorageSubscribe(AStreamJid);
		}

		return true;
	}
	else if (FRosterChanger && roster && presence && presence->isOpen())
	{
		LOG_STRM_ERROR(AStreamJid,QString("Failed to change service from=%1 to=%2: Invalid params").arg(AServiceFrom.bare(),AServiceTo.bare()));
	}
	return false;
}

QString Gateways::sendPromptRequest(const Jid &AStreamJid, const Jid &AServiceJid)
{
	Stanza request(STANZA_KIND_IQ);
	request.setType(STANZA_TYPE_GET).setTo(AServiceJid.full()).setUniqueId();
	request.addElement("query",NS_JABBER_GATEWAY);
	if (FStanzaProcessor->sendStanzaRequest(this,AStreamJid,request,GATEWAY_TIMEOUT))
	{
		LOG_STRM_DEBUG(AStreamJid,QString("Legacy user prompt request sent to=%1, id=%2").arg(AServiceJid.full(),request.id()));
		FPromptRequests.append(request.id());
		return request.id();
	}
	else
	{
		LOG_STRM_WARNING(AStreamJid,QString("Failed to send legacy user prompt request to=%1").arg(AServiceJid.full()));
	}
	return QString();
}

QString Gateways::sendUserJidRequest(const Jid &AStreamJid, const Jid &AServiceJid, const QString &AContactID)
{
	Stanza request(STANZA_KIND_IQ);
	request.setType(STANZA_TYPE_SET).setTo(AServiceJid.full()).setUniqueId();
	QDomElement elem = request.addElement("query",NS_JABBER_GATEWAY);
	elem.appendChild(request.createElement("prompt")).appendChild(request.createTextNode(AContactID));
	if (FStanzaProcessor->sendStanzaRequest(this,AStreamJid,request,GATEWAY_TIMEOUT))
	{
		LOG_STRM_DEBUG(AStreamJid,QString("Legacy user JID request sent to=%1, id=%2").arg(AServiceJid.full(),request.id()));
		FUserJidRequests.append(request.id());
		return request.id();
	}
	else
	{
		LOG_STRM_WARNING(AStreamJid,QString("Failed to send legacy user JID request to=%1").arg(AServiceJid.full()));
	}
	return QString();
}

QDialog *Gateways::showAddLegacyContactDialog(const Jid &AStreamJid, const Jid &AServiceJid, QWidget *AParent)
{
	IPresence *presence = FPresenceManager!=NULL ? FPresenceManager->findPresence(AStreamJid) : NULL;
	if (presence && presence->isOpen())
	{
		AddLegacyContactDialog *dialog = new AddLegacyContactDialog(this,FRosterChanger,AStreamJid,AServiceJid,AParent);
		connect(presence->instance(),SIGNAL(closed()),dialog,SLOT(reject()));
		dialog->show();
		return dialog;
	}
	else
	{
		LOG_STRM_WARNING(AStreamJid,"Failed to open add legacy contact dialog: Presence not opened");
	}
	return NULL;
}

void Gateways::registerDiscoFeatures()
{
	IDiscoFeature dfeature;
	dfeature.active = false;
	dfeature.var = NS_JABBER_GATEWAY;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_GATEWAYS);
	dfeature.name = tr("Gateway Interaction");
	dfeature.description = tr("Supports the adding of the contact by the username of the legacy system");
	FDiscovery->insertDiscoFeature(dfeature);
}

void Gateways::savePrivateStorageKeep(const Jid &AStreamJid)
{
	if (FPrivateStorage && FPrivateStorageKeep.contains(AStreamJid))
	{
		QDomDocument doc;
		doc.appendChild(doc.createElement("services"));
		QDomElement elem = doc.documentElement().appendChild(doc.createElementNS(PSN_GATEWAYS_KEEP,PST_GATEWAYS_SERVICES)).toElement();
		QSet<Jid> services = FPrivateStorageKeep.value(AStreamJid);
		foreach(const Jid &service, services)
			elem.appendChild(doc.createElement("service")).appendChild(doc.createTextNode(service.bare()));
		if (!FPrivateStorage->saveData(AStreamJid,elem).isEmpty())
			LOG_STRM_INFO(AStreamJid,QString("Save gateways with keep connection request sent"));
		else
			LOG_STRM_WARNING(AStreamJid,QString("Failed to send save gateways with keep connection request"));
	}
}

void Gateways::savePrivateStorageSubscribe(const Jid &AStreamJid)
{
	if (FPrivateStorage)
	{
		QDomDocument doc;
		doc.appendChild(doc.createElement("services"));
		QDomElement elem = doc.documentElement().appendChild(doc.createElementNS(PSN_GATEWAYS_SUBSCRIBE,PST_GATEWAYS_SERVICES)).toElement();
		foreach(const Jid &service, FSubscribeServices.values(AStreamJid))
			elem.appendChild(doc.createElement("service")).appendChild(doc.createTextNode(service.bare()));
		if (!FPrivateStorage->saveData(AStreamJid,elem).isEmpty())
			LOG_STRM_INFO(AStreamJid,QString("Save gateways with auto subscribe request sent"));
		else
			LOG_STRM_WARNING(AStreamJid,QString("Failed to send save gateways with auto subscribe request"));
	}
}

bool Gateways::isSelectionAccepted(const QList<IRosterIndex *> &ASelected) const
{
	int singleKind = -1;
	foreach(IRosterIndex *index, ASelected)
	{
		int indexKind = index->kind();
		if (indexKind!=RIK_STREAM_ROOT && indexKind!=RIK_CONTACT && indexKind!=RIK_AGENT)
			return false;
		else if (singleKind!=-1 && singleKind!=indexKind)
			return false;
		singleKind = indexKind;

		IPresence *presence = FPresenceManager!=NULL ? FPresenceManager->findPresence(index->data(RDR_STREAM_JID).toString()) : NULL;
		if (presence==NULL || !presence->isOpen())
			return false;
	}
	return !ASelected.isEmpty();
}

void Gateways::onAddLegacyUserActionTriggered(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		Jid streamJid = action->data(ADR_STREAM_JID).toString();
		Jid serviceJid = action->data(ADR_SERVICE_JID).toString();
		showAddLegacyContactDialog(streamJid,serviceJid);
	}
}

void Gateways::onLogActionTriggered(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		bool logIn = action->data(ADR_LOG_IN).toBool();
		QStringList streams = action->data(ADR_STREAM_JID).toStringList();
		QStringList services = action->data(ADR_SERVICE_JID).toStringList();
		for (int i=0; i<streams.count(); i++)
		{
			if (FPrivateStorageKeep.value(streams.at(i)).contains(services.at(i)))
				setKeepConnection(streams.at(i),services.at(i),logIn);
			sendLogPresence(streams.at(i),services.at(i),logIn);
		}
	}
}

void Gateways::onResolveActionTriggered(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		QStringList streams = action->data(ADR_STREAM_JID).toStringList();
		QStringList services = action->data(ADR_SERVICE_JID).toStringList();
		bool resolve = action->data(ADR_ACTION_RESOLVE).toBool(); /*** <<< eyeCU >>> ***/
		for (int i=0; i<streams.count(); i++)
		{
			Jid serviceJid = services.at(i);
			if (!serviceJid.hasNode())
			{
				IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(streams.at(i)) : NULL;
				foreach(const Jid &contactJid, serviceContacts(streams.at(i),serviceJid))
				{
					IRosterItem ritem = roster!=NULL ? roster->findItem(contactJid) : IRosterItem();
					/*** <<< eyeCU <<< ***/
					if (!ritem.isNull())
					{
						if (resolve)
						{
							if (ritem.name.trimmed().isEmpty())
								resolveNickName(streams.at(i),contactJid);
						}
						else
							if (!ritem.name.trimmed().isEmpty())
								resetNickName(streams.at(i),contactJid);
					}
					/*** >>> eyeCU >>> ***/
				}
			}
			else
			{
				/*** <<< eyeCU <<< ***/
				if (resolve)
					resolveNickName(streams.at(i),serviceJid);
				else
					resetNickName(streams.at(i),serviceJid);
				/*** >>> eyeCU >>> ***/
			}
		}
	}
}

void Gateways::onKeepActionTriggered(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		QSet<Jid> saveKeepStorages;

		QStringList streams = action->data(ADR_STREAM_JID).toStringList();
		QStringList services = action->data(ADR_SERVICE_JID).toStringList();
		for (int i=0; i<streams.count(); i++)
		{
			if (FPrivateStorageKeep.contains(streams.at(i)) && FPrivateStorageKeep.value(streams.at(i)).contains(services.at(i))!=action->isChecked())
			{
				if (action->isChecked())
					FPrivateStorageKeep[streams.at(i)] += services.at(i);
				else
					FPrivateStorageKeep[streams.at(i)] -= services.at(i);
				saveKeepStorages += streams.at(i);
			}
			setKeepConnection(streams.at(i),services.at(i),action->isChecked());
		}

		foreach(const Jid &streamJid, saveKeepStorages)
			savePrivateStorageKeep(streamJid);
	}
}

void Gateways::onChangeActionTriggered(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		Jid streamJid = action->data(ADR_STREAM_JID).toString();
		Jid serviceFrom = action->data(ADR_SERVICE_JID).toString();
		Jid serviceTo = action->data(ADR_NEW_SERVICE_JID).toString();
		if (changeService(streamJid,serviceFrom,serviceTo,true,true))
		{
			QString id = FRegistration!=NULL ?  FRegistration->sendRegisterRequest(streamJid,serviceTo) : QString();
			if (!id.isEmpty())
				FShowRegisterRequests.insert(id,streamJid);
		}
	}
}

void Gateways::onRemoveActionTriggered(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		QStringList streams = action->data(ADR_STREAM_JID).toStringList();
		QStringList services = action->data(ADR_SERVICE_JID).toStringList();

		int button = QMessageBox::No;
		if (services.count() == 1)
		{
			Jid serviceJid = services.first();
			button = QMessageBox::question(NULL,tr("Remove transport and its contacts"),
				tr("Are you sure you wish to remove transport '<b>%1</b>' and its <b>%n contact(s)</b> from the roster?","",serviceContacts(streams.first(),serviceJid).count())
					.arg(HTML_ESCAPE(serviceJid.domain())),
				QMessageBox::Yes | QMessageBox::No);
		}
		else if (services.count() > 1)
		{
			button = QMessageBox::question(NULL,tr("Remove transports and their contacts"),
				tr("Are you sure you wish to remove <b>%n transport(s)</b> and their contacts from the roster?","",services.count()),
				QMessageBox::Yes | QMessageBox::No);
		}

		if (button == QMessageBox::Yes)
		{
			for (int i=0; i<streams.count(); i++)
				removeService(streams.at(i),services.at(i));
		}
	}
}

void Gateways::onRostersViewIndexMultiSelection(const QList<IRosterIndex *> &ASelected, bool &AAccepted)
{
	AAccepted = AAccepted || isSelectionAccepted(ASelected);
}

void Gateways::onRostersViewIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
	if (ALabelId==AdvancedDelegateItem::DisplayId && isSelectionAccepted(AIndexes))
	{
		bool isMultiSelection = AIndexes.count()>1;
		int indexKind = AIndexes.first()->kind();

		if (FDiscovery && indexKind==RIK_STREAM_ROOT)
		{
			Menu *addUserMenu = new Menu(AMenu);
			addUserMenu->setTitle(tr("Add Legacy User"));
			addUserMenu->setIcon(RSR_STORAGE_MENUICONS,MNI_GATEWAYS_ADD_CONTACT);

			int streamGroup = AG_DEFAULT;
			foreach(IRosterIndex *index, AIndexes)
			{
				IPresence *presence = FPresenceManager!=NULL ? FPresenceManager->findPresence(index->data(RDR_STREAM_JID).toString()) : NULL;
				if (presence && presence->isOpen())
				{
					foreach(const IPresenceItem &pitem, presence->items())
					{
						if (pitem.show!=IPresence::Error && !pitem.itemJid.hasNode() && FDiscovery->discoInfo(presence->streamJid(),pitem.itemJid).features.contains(NS_JABBER_GATEWAY))
						{
							Action *action = new Action(addUserMenu);
							action->setText(pitem.itemJid.uFull());
							action->setIcon(FStatusIcons!=NULL ? FStatusIcons->iconByJid(presence->streamJid(),pitem.itemJid) : QIcon());
							action->setData(ADR_STREAM_JID,presence->streamJid().full());
							action->setData(ADR_SERVICE_JID,pitem.itemJid.full());
							connect(action,SIGNAL(triggered(bool)),SLOT(onAddLegacyUserActionTriggered(bool)));
							addUserMenu->addAction(action,streamGroup,true);
						}
					}
					streamGroup++;
				}
			}

			if (!addUserMenu->isEmpty())
				AMenu->addAction(addUserMenu->menuAction(), AG_RVCM_GATEWAYS_ADD_LEGACY_USER, true);
			else
				delete addUserMenu;
		}
		else if (indexKind==RIK_CONTACT || indexKind==RIK_AGENT)
		{
			QMap<int, QStringList> rolesMap = FRostersViewPlugin->rostersView()->indexesRolesMap(AIndexes,QList<int>()<<RDR_STREAM_JID<<RDR_PREP_BARE_JID,RDR_PREP_BARE_JID,RDR_STREAM_JID);

			bool showResolve = FVCardManager!=NULL;
			for(int i=0; showResolve && i<AIndexes.count(); i++)
			{
				IRosterIndex *index = AIndexes.at(i);
				if (indexKind == RIK_CONTACT)
				{
					IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(index->data(RDR_STREAM_JID).toString()) : NULL;
					IRosterItem ritem = roster!=NULL ? roster->findItem(index->data(RDR_PREP_BARE_JID).toString()) : IRosterItem();
					showResolve = !ritem.isNull() && (ritem.name.trimmed().isEmpty() || streamServices(roster->streamJid()).contains(ritem.itemJid.domain()));
				}
				else
				{
					showResolve = !serviceContacts(index->data(RDR_STREAM_JID).toString(),index->data(RDR_PREP_BARE_JID).toString()).isEmpty();
				}
			}

			if (showResolve)
			{
				Action *action = new Action(AMenu);
				action->setText(indexKind==RIK_AGENT ? tr("Resolve nick names") : tr("Resolve nick name"));
				action->setIcon(RSR_STORAGE_MENUICONS,MNI_GATEWAYS_RESOLVE); // *** <<< eyeCU >>> ***
				action->setData(ADR_STREAM_JID,rolesMap.value(RDR_STREAM_JID));
				action->setData(ADR_SERVICE_JID,rolesMap.value(RDR_PREP_BARE_JID));
				action->setData(ADR_ACTION_RESOLVE, true);
				connect(action,SIGNAL(triggered(bool)),SLOT(onResolveActionTriggered(bool)));
				AMenu->addAction(action,AG_RVCM_GATEWAYS_RESOLVE);
			}

			/*** <<< eyeCU <<< ***/
			QStringList jids = rolesMap.value(RDR_PREP_BARE_JID);
			QStringList streamJids = rolesMap.value(RDR_STREAM_JID);

			bool disabled=true;
			if (indexKind==RIK_CONTACT)
				for (QStringList::ConstIterator its=streamJids.constBegin(); its!=streamJids.constEnd() && disabled; its++)
				{
					IRoster *roster = FRosterManager->findRoster(*its);
					if (roster)
						for (QStringList::ConstIterator it=jids.constBegin(); it!=jids.constEnd() && disabled; it++)
							if (!roster->findItem(*it).name.isEmpty())
								disabled=false;
				}
			else
				disabled=false;
			Action *action = new Action(AMenu);
			action->setText(indexKind==RIK_AGENT ? tr("Reset nick names") : tr("Reset nick name"));
			action->setIcon(RSR_STORAGE_MENUICONS,MNI_GATEWAYS_RESET);
			action->setData(ADR_STREAM_JID,rolesMap.value(RDR_STREAM_JID));
			action->setData(ADR_SERVICE_JID,rolesMap.value(RDR_PREP_BARE_JID));
			action->setData(ADR_ACTION_RESOLVE, false);
			action->setDisabled(disabled);
			connect(action,SIGNAL(triggered(bool)),SLOT(onResolveActionTriggered(bool)));
			AMenu->addAction(action,AG_RVCM_GATEWAYS_RESOLVE);
			/*** >>> eyeCU >>> ***/

			if (indexKind == RIK_AGENT)
			{
				Action *action = new Action(AMenu);
				action->setText(tr("Login on transport"));
				action->setIcon(RSR_STORAGE_MENUICONS,MNI_GATEWAYS_LOG_IN);
				action->setData(ADR_STREAM_JID,rolesMap.value(RDR_STREAM_JID));
				action->setData(ADR_SERVICE_JID,rolesMap.value(RDR_PREP_BARE_JID));
				action->setData(ADR_LOG_IN,true);
				connect(action,SIGNAL(triggered(bool)),SLOT(onLogActionTriggered(bool)));
				AMenu->addAction(action,AG_RVCM_GATEWAYS_LOGIN);

				action = new Action(AMenu);
				action->setText(tr("Logout from transport"));
				action->setIcon(RSR_STORAGE_MENUICONS,MNI_GATEWAYS_LOG_OUT);
				action->setData(ADR_STREAM_JID,rolesMap.value(RDR_STREAM_JID));
				action->setData(ADR_SERVICE_JID,rolesMap.value(RDR_PREP_BARE_JID));
				action->setData(ADR_LOG_IN,false);
				connect(action,SIGNAL(triggered(bool)),SLOT(onLogActionTriggered(bool)));
				AMenu->addAction(action,AG_RVCM_GATEWAYS_LOGIN);

				int keepChecks = 0;
				foreach(IRosterIndex *index, AIndexes)
				{
					if (!FPrivateStorageKeep.contains(index->data(RDR_STREAM_JID).toString()))
						keepChecks |= 0x03;
					else if (FKeepConnections.contains(index->data(RDR_STREAM_JID).toString(),index->data(RDR_PREP_BARE_JID).toString()))
						keepChecks |= 0x01;
					else
						keepChecks |= 0x02;
				}

				action = new Action(AMenu);
				action->setText(tr("Keep connection"));
				action->setIcon(RSR_STORAGE_MENUICONS,MNI_GATEWAYS_KEEP_CONNECTION);
				action->setData(ADR_STREAM_JID,rolesMap.value(RDR_STREAM_JID));
				action->setData(ADR_SERVICE_JID,rolesMap.value(RDR_PREP_BARE_JID));
				action->setCheckable(true);
				action->setChecked(keepChecks == 0x01);
				action->setVisible(keepChecks != 0x03);
				connect(action,SIGNAL(triggered(bool)),SLOT(onKeepActionTriggered(bool)));
				AMenu->addAction(action,AG_RVCM_GATEWAYS_LOGIN);

				if (isMultiSelection || !serviceContacts(rolesMap.value(RDR_STREAM_JID).value(0),rolesMap.value(RDR_PREP_BARE_JID).value(0)).isEmpty())
				{
					action = new Action(AMenu);
					action->setText(tr("Remove transport and its contacts"));
					action->setIcon(RSR_STORAGE_MENUICONS,MNI_GATEWAYS_REMOVE);
					action->setData(ADR_STREAM_JID,rolesMap.value(RDR_STREAM_JID));
					action->setData(ADR_SERVICE_JID,rolesMap.value(RDR_PREP_BARE_JID));
					connect(action,SIGNAL(triggered(bool)),SLOT(onRemoveActionTriggered(bool)));
					AMenu->addAction(action,AG_RVCM_GATEWAYS_REMOVE);
				}
			}
		}
	}
}

void Gateways::onPresenceOpened(IPresence *APresence)
{
	if (FPrivateStorage)
	{
		if (!FPrivateStorage->loadData(APresence->streamJid(),PST_GATEWAYS_SERVICES,PSN_GATEWAYS_KEEP).isEmpty())
			LOG_STRM_INFO(APresence->streamJid(),QString("Gateways with keep connection load request sent"));
		else
			LOG_STRM_WARNING(APresence->streamJid(),QString("Failed to send load gateways with keep connection request"));
	}
}

void Gateways::onContactStateChanged(const Jid &AStreamJid, const Jid &AContactJid, bool AStateOnline)
{
	if (AStateOnline && FSubscribeServices.contains(AStreamJid,AContactJid.bare()))
	{
		IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(AStreamJid) : NULL;
		if (roster)
		{
			FSubscribeServices.remove(AStreamJid,AContactJid.bare());
			savePrivateStorageSubscribe(AStreamJid);

			LOG_STRM_DEBUG(AStreamJid,QString("Automatically requesting subscription from service contacts=%1").arg(AContactJid.domain()));
			foreach(const IRosterItem &ritem, roster->items())
			{
				if (ritem.itemJid.pDomain() == AContactJid.pDomain())
					if (ritem.subscription!=SUBSCRIPTION_BOTH && ritem.subscription!=SUBSCRIPTION_TO && ritem.subscriptionAsk!=SUBSCRIPTION_SUBSCRIBE)
						roster->sendSubscription(ritem.itemJid,IRoster::Subscribe);
			}
		}
	}
}

void Gateways::onPresenceClosed(IPresence *APresence)
{
	FSubscribeServices.remove(APresence->streamJid());
}

void Gateways::onPresenceActiveChanged(IPresence *APresence, bool AActive)
{
	if (!AActive)
	{
		FKeepConnections.remove(APresence->streamJid());
		FPrivateStorageKeep.remove(APresence->streamJid());
	}
}

void Gateways::onRosterOpened(IRoster *ARoster)
{
	if (FRosterChanger)
	{
		foreach(const Jid &serviceJid, FSubscribeServices.values(ARoster->streamJid()))
			foreach(const Jid &contactJid, serviceContacts(ARoster->streamJid(),serviceJid))
				FRosterChanger->insertAutoSubscribe(ARoster->streamJid(),contactJid,true,true,false);
	}
}

void Gateways::onRosterSubscriptionReceived(IRoster *ARoster, const Jid &AItemJid, int ASubsType, const QString &AText)
{
	Q_UNUSED(AText);
	if (ASubsType==IRoster::Subscribed && FSubscribeServices.contains(ARoster->streamJid(),AItemJid))
		sendLogPresence(ARoster->streamJid(),AItemJid,true);
}

void Gateways::onRosterStreamJidAboutToBeChanged(IRoster *ARoster, const Jid &AAfter)
{
	Q_UNUSED(AAfter);
	FKeepConnections.remove(ARoster->streamJid());
	FPrivateStorageKeep.remove(ARoster->streamJid());
}

void Gateways::onPrivateStorateOpened(const Jid &AStreamJid)
{
	if (!FPrivateStorage->loadData(AStreamJid,PST_GATEWAYS_SERVICES,PSN_GATEWAYS_SUBSCRIBE).isEmpty())
		LOG_STRM_INFO(AStreamJid,QString("Load gateways with auto subscribe request sent"));
	else
		LOG_STRM_WARNING(AStreamJid,QString("Failed to send load gateways with auto subscribe request"));
}

void Gateways::onPrivateDataLoaded(const QString &AId, const Jid &AStreamJid, const QDomElement &AElement)
{
	Q_UNUSED(AId);
	if (AElement.tagName()==PST_GATEWAYS_SERVICES && AElement.namespaceURI()==PSN_GATEWAYS_KEEP)
	{
		LOG_STRM_INFO(AStreamJid,"Gateways with keep connection loaded");
		IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(AStreamJid) : NULL;
		if (roster)
		{
			QSet<Jid> newServices;
			bool changed = false;
			QDomElement elem = AElement.firstChildElement("service");
			while (!elem.isNull())
			{
				Jid service = elem.text();
				IRosterItem ritem = roster->findItem(service);
				if (!ritem.isNull())
				{
					newServices += service;
					if (ritem.subscription!=SUBSCRIPTION_BOTH && ritem.subscription!=SUBSCRIPTION_FROM)
						sendLogPresence(AStreamJid,service,true);
					setKeepConnection(AStreamJid,service,true);
				}
				else
				{
					changed = true;
				}
				elem = elem.nextSiblingElement("service");
			}

			QSet<Jid> oldServices = FPrivateStorageKeep.value(AStreamJid) - newServices;
			foreach(const Jid &service, oldServices)
				setKeepConnection(AStreamJid,service,false);
			FPrivateStorageKeep[AStreamJid] = newServices;

			if (changed)
				savePrivateStorageKeep(AStreamJid);
		}
	}
	else if (AElement.tagName()==PST_GATEWAYS_SERVICES && AElement.namespaceURI()==PSN_GATEWAYS_SUBSCRIBE)
	{
		LOG_STRM_INFO(AStreamJid,"Gateways with auto subscribe loaded");
		QDomElement elem = AElement.firstChildElement("service");
		while (!elem.isNull())
		{
			Jid serviceJid = elem.text();
			FSubscribeServices.insertMulti(AStreamJid,serviceJid);
			QString id = FRegistration!=NULL ? FRegistration->sendRegisterRequest(AStreamJid,serviceJid) : QString();
			if (!id.isEmpty())
				FShowRegisterRequests.insert(id,AStreamJid);
			elem = elem.nextSiblingElement("service");
		}
	}
}

void Gateways::onPrivateDataChanged(const Jid &AStreamJid, const QString &ATagName, const QString &ANamespace)
{
	if (ATagName==PST_GATEWAYS_SERVICES && ANamespace==PSN_GATEWAYS_KEEP)
		FPrivateStorage->loadData(AStreamJid,PST_GATEWAYS_SERVICES,PSN_GATEWAYS_KEEP);
}

void Gateways::onKeepTimerTimeout()
{
	foreach(const Jid &streamJid, FKeepConnections.uniqueKeys())
	{
		IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(streamJid) : NULL;
		IPresence *presence = FPresenceManager!=NULL ? FPresenceManager->findPresence(streamJid) : NULL;
		if (roster && presence && presence->isOpen())
		{
			QList<Jid> services = FKeepConnections.values(streamJid);
			foreach(const Jid &service, services)
			{
				if (roster->hasItem(service))
				{
					const QList<IPresenceItem> pitems = presence->findItems(service);
					if (pitems.isEmpty() || pitems.at(0).show==IPresence::Error)
					{
						presence->sendPresence(service,IPresence::Offline,QString(),0);
						presence->sendPresence(service,presence->show(),presence->status(),presence->priority());
					}
				}
			}
		}
	}
}

void Gateways::onVCardReceived(const Jid &AContactJid)
{
	if (FResolveNicks.contains(AContactJid))
	{
		QList<Jid> streamJids = FResolveNicks.values(AContactJid);
		foreach(const Jid &streamJid, streamJids)
			resolveNickName(streamJid,AContactJid);
		FResolveNicks.remove(AContactJid);
	}
}

void Gateways::onVCardError(const Jid &AContactJid, const XmppError &AError)
{
	Q_UNUSED(AError);
	FResolveNicks.remove(AContactJid);
}

void Gateways::onDiscoItemsWindowCreated(IDiscoItemsWindow *AWindow)
{
	connect(AWindow->instance(),SIGNAL(indexContextMenu(const QModelIndex &, Menu *)),SLOT(onDiscoItemContextMenu(const QModelIndex &, Menu *)));
}

void Gateways::onDiscoItemContextMenu(const QModelIndex &AIndex, Menu *AMenu)
{
	Jid itemJid = AIndex.data(DIDR_JID).toString();
	QString itemNode = AIndex.data(DIDR_NODE).toString();
	if (!itemJid.hasNode() && itemNode.isEmpty())
	{
		Jid streamJid = AIndex.data(DIDR_STREAM_JID).toString();
		IDiscoInfo dinfo = FDiscovery->discoInfo(streamJid,itemJid,itemNode);
		if (dinfo.error.isNull() && !dinfo.identity.isEmpty())
		{
			QList<Jid> services;
			foreach(const IDiscoIdentity &ident, dinfo.identity)
				services += streamServices(streamJid,ident);

			foreach(const Jid &service, streamServices(streamJid))
				if (!services.contains(service) && FDiscovery->discoInfo(streamJid,service).identity.isEmpty())
					services.append(service);

			if (!services.isEmpty() && !services.contains(itemJid))
			{
				Menu *change = new Menu(AMenu);
				change->setTitle(tr("Use instead of"));
				change->setIcon(RSR_STORAGE_MENUICONS,MNI_GATEWAYS_CHANGE);
				foreach(const Jid &service, services)
				{
					Action *action = new Action(change);
					action->setText(service.uFull());
					if (FStatusIcons!=NULL)
						action->setIcon(FStatusIcons->iconByJid(streamJid,service));
					else
						action->setIcon(RSR_STORAGE_MENUICONS,MNI_GATEWAYS_LOG_IN);
					action->setData(ADR_STREAM_JID,streamJid.full());
					action->setData(ADR_SERVICE_JID,service.full());
					action->setData(ADR_NEW_SERVICE_JID,itemJid.full());
					connect(action,SIGNAL(triggered(bool)),SLOT(onChangeActionTriggered(bool)));
					change->addAction(action,AG_DEFAULT,true);
				}
				AMenu->addAction(change->menuAction(),TBG_DIWT_DISCOVERY_ACTIONS,true);
			}
		}
	}
}

void Gateways::onRegisterFields(const QString &AId, const IRegisterFields &AFields)
{
	if (FShowRegisterRequests.contains(AId))
	{
		Jid streamJid = FShowRegisterRequests.take(AId);
		if (!AFields.registered && FSubscribeServices.contains(streamJid,AFields.serviceJid))
			FRegistration->showRegisterDialog(streamJid,AFields.serviceJid,IRegistration::Register);
	}
}

void Gateways::onRegisterError(const QString &AId, const XmppError &AError)
{
	Q_UNUSED(AError);
	FShowRegisterRequests.remove(AId);
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_gateways, Gateways)
#endif
