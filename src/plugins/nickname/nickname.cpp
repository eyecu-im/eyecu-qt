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
#include <definitions/toolbargroups.h>
#include <definitions/actiongroups.h>
#include <definitions/menuicons.h>
#include <definitions/namespaces.h>
#include <definitions/rosterlabels.h>
#include <definitions/rostertooltiporders.h>
#include <definitions/rosterdataholderorders.h>
#include <definitions/mapobjecttyperole.h>
#include <definitions/resources.h>

#include <utils/advanceditemdelegate.h>
#include <utils/logger.h>

#include "nickname.h"

#define SHC_MESSAGE "/message"
#define SHC_PRESENCE "/presence"

#define ADR_STREAM_JID  Action::DR_StreamJid
#define ADR_CONTACT_JID Action::DR_Parametr1
#define ADR_SERVICE_JID   Action::DR_Parametr1
#define TAG_NAME        "nick"
#define SHO_NICKNAME    500

Nickname::Nickname():
    FOptionsManager(NULL),
    FStanzaProcessor(NULL),
    FPEPManager(NULL),
    FDiscovery(NULL),
    FPresenceManager(NULL),
    FRosterManager(NULL),
    FRostersModel(NULL),
    FRostersViewPlugin(NULL),
    FAccountManager(NULL),
    FRosterIndexTypes(QList<int>() << RIK_CONTACT << RIK_RECENT_ITEM << RIK_AGENT << RIK_STREAM_ROOT)
{}

Nickname::~Nickname()
{}

void Nickname::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Nickname");
    APluginInfo->description = tr("Implements XEP-0172: User Nickname");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
//    APluginInfo->dependences.append(PEPMANAGER_UUID);
}

bool Nickname::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder)

    IPlugin *plugin = APluginManager->pluginInterface("IPEPManager").value(0,NULL);
    if (plugin)
    {
        FPEPManager = qobject_cast<IPEPManager *>(plugin->instance());
        FPEPManager->insertNodeHandler(QString(NS_PEP_NICK), this);
    }

    plugin = APluginManager->pluginInterface("IAccountManager").value(0);
    if (plugin)
        FAccountManager = qobject_cast<IAccountManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0);
    if (plugin)
        FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0,NULL);
    if (plugin)
    {
		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());
        connect(FPresenceManager->instance(), SIGNAL(presenceOpened(IPresence*)), SLOT(onPresenceOpened(IPresence*)));
    }

	plugin = APluginManager->pluginInterface("IRosterManager").value(0,NULL);
    if (plugin)
    {
		FRosterManager = qobject_cast<IRosterManager *>(plugin->instance());
        connect(FRosterManager->instance(), SIGNAL(rosterItemReceived(IRoster *, IRosterItem, IRosterItem)), SLOT(onRosterItemReceived(IRoster*,IRosterItem,IRosterItem)));
    }

    plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
    if (plugin)
        FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IRostersModel").value(0,NULL);
    if (plugin)
        FRostersModel = qobject_cast<IRostersModel *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IRostersViewPlugin").value(0,NULL);
    if (plugin)
    {
        FRostersViewPlugin = qobject_cast<IRostersViewPlugin *>(plugin->instance());
        if (FRostersViewPlugin)
        {
            connect(FRostersViewPlugin->rostersView()->instance(),
                    SIGNAL(indexToolTips(IRosterIndex *, quint32, QMap<int,QString> &)),
                    SLOT(onRosterIndexToolTips(IRosterIndex *, quint32, QMap<int,QString> &)));
        }
    }

    connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
    connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
    connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

    return true;
}

bool Nickname::initObjects()
{
    if (FDiscovery)
        registerDiscoFeatures();

    if (FRostersModel)
        FRostersModel->insertRosterDataHolder(RDHO_NICK, this);

    if (FStanzaProcessor)
    {                       // Insert stanza handles
        IStanzaHandle handle;
        handle.handler = this;
        handle.order = SHO_NICKNAME;
        handle.direction = IStanzaHandle::DirectionIn;
        handle.conditions.append(SHC_MESSAGE);
        FSHIMessage = FStanzaProcessor->insertStanzaHandle(handle);

        handle.direction = IStanzaHandle::DirectionOut;
        FSHOMessage = FStanzaProcessor->insertStanzaHandle(handle);

        handle.conditions.clear();
        handle.conditions.append(SHC_PRESENCE);
        FSHOPresence = FStanzaProcessor->insertStanzaHandle(handle);

        handle.direction = IStanzaHandle::DirectionIn;
        FSHIPresence = FStanzaProcessor->insertStanzaHandle(handle);
    }
    return true;
}

bool Nickname::initSettings()
{
    Options::setDefaultValue(OPV_ACCOUNT_NICKNAME, QString());
    Options::setDefaultValue(OPV_ACCOUNT_NICKNAMECHANGE, true);
    Options::setDefaultValue(OPV_ACCOUNT_NICKNAMESET, true);
    Options::setDefaultValue(OPV_ACCOUNT_NICKNAMEBROADCAST, false);
	Options::setDefaultValue(OPV_ACCOUNT_NICKNAME_MUC_DEFAULT, false);
    if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);
    return true;
}

bool Nickname::stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	Q_UNUSED(AAccept)

	IAccount *account = FAccountManager->findAccountByStream(AStreamJid);
	if (account)
	{
		OptionsNode node = account->optionsNode();
		if (AHandleId==FSHIMessage || AHandleId==FSHIPresence)
		{
			QDomElement nick = AStanza.firstElement(TAG_NAME, NS_PEP_NICK);
			if (!nick.isNull())
			{
				QString nickName = nick.text();
				Jid contactJid(AStanza.from());
				if (nickName!=(FNicknameHash.value(contactJid.bare())))
				{
					if (nickName.isEmpty())
						FNicknameHash.remove(contactJid.bare());
					else
						FNicknameHash.insert(contactJid.bare(), nickName);
					updateDataHolder(contactJid);

					if(contactJid.bare() == AStreamJid.bare() && node.value(OPV_NICKNAMECHANGE).toBool())  // My resource
						node.setValue(nickName, OPV_NICKNAME);
				}
			}
		}
		else if ((node.value(OPV_NICKNAMEBROADCAST).toBool() && AHandleId==FSHOPresence && AStanza.to().isEmpty()) ||
				(!node.value(OPV_NICKNAME).toString().isEmpty() && ((!FNotifiedContacts.value(AStreamJid.bare()).contains(AStanza.to()) && AHandleId==FSHOMessage) || AStanza.type()=="subscribe")))
		{
			AStanza.addElement(TAG_NAME, NS_PEP_NICK).appendChild(AStanza.createTextNode(node.value(OPV_NICKNAME).toString()));
			FNotifiedContacts[AStreamJid.bare()].append(AStanza.to());
		}
	}
    return false;
}

//-------------------
bool Nickname::processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza)
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
				QDomElement nick = item.firstChildElement(TAG_NAME);
				if(!nick.isNull())
				{
					if(contactJid.bare() == AStreamJid.bare())
						FIdHash.insert("AStreamJid.bare()", item.attribute("id"));

					QString nickName = nick.text();

					if (nickName.isEmpty())
						stop=true;
					else
					{
						if (nickName!=(FNicknameHash.value(contactJid.bare())))
						{
							FNicknameHash.insert(contactJid.bare(), nickName);
							updateDataHolder(contactJid);
							if(contactJid.bare() == AStreamJid.bare())
							{
								OptionsNode node = FAccountManager->findAccountByStream(AStreamJid)->optionsNode();
								if (node.value(OPV_NICKNAMECHANGE).toBool())
									node.setValue(nickName, OPV_NICKNAME);
							}
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
				FNicknameHash.remove(contactJid.bare());
				updateDataHolder(contactJid);
				return true;
			}
		}
	}
    return false;
}

void Nickname::registerDiscoFeatures()
{
    IDiscoFeature dfeature;
    dfeature.var = NS_PEP_NICK;
    dfeature.active = true;
    dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_NICKNAME);
    dfeature.name = tr("User Nickname");
    dfeature.description = tr("Supports user nickname");
    FDiscovery->insertDiscoFeature(dfeature);

    dfeature.var.append(NODE_NOTIFY_SUFFIX);
    dfeature.name = tr("User Nickname Notification");
    dfeature.description = tr("Receives notification of user nickname change");
    FDiscovery->insertDiscoFeature(dfeature);
}

bool Nickname::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
    return FDiscovery==NULL||!FDiscovery->hasDiscoInfo(AStreamJid,AContactJid)||
            FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_PEP_ACTIVITY);
}

void Nickname::sendNick(const QString &ANickname, const Jid &AStreamJid)
{
	if (FPEPManager && FPEPManager->isSupported(AStreamJid))
	{
		QDomDocument doc;
		QDomElement item=doc.createElement("item");

		item.setAttribute("id", currentItemId(AStreamJid));
		if(ANickname.isEmpty())
		{
			if (Options::node(OPV_PEP_DELETE_PUBLISHEMPTY).value().toBool())
			{
				item.appendChild(doc.createElementNS(NS_PEP_NICK, "nick"));
				FPEPManager->publishItem(AStreamJid, NS_PEP_NICK, item);
			}
			if (Options::node(OPV_PEP_DELETE_RETRACT).value().toBool())
				FPEPManager->deleteItem(AStreamJid, NS_PEP_NICK, item);
		}
		else
		{
			QDomElement nick=doc.createElementNS(NS_PEP_NICK, "nick");
			nick.appendChild(doc.createTextNode(ANickname));
			item.appendChild(nick);
			FPEPManager->publishItem(AStreamJid, NS_PEP_NICK, item);
		}
	}
}

QList<int> Nickname::rosterDataRoles(int AOrder) const
{
    if (AOrder==RDHO_NICK)
        return QList<int>() << RDR_NAME;
    return QList<int>();
}

QVariant Nickname::rosterData(int AOrder, const IRosterIndex *AIndex, int ARole) const
{
    if (AOrder==RDHO_NICK && ARole == RDR_NAME && FRosterIndexTypes.contains(AIndex->data(RDR_KIND).toInt()))
    {
        Jid jid(AIndex->data(RDR_FULL_JID).toString());

        if (FNicknameHash.contains(jid.bare()))
        {
            Jid streamJid(AIndex->data(RDR_STREAM_JID).toString());
            IRoster *roster = FRosterManager->findRoster(streamJid);
            if (roster)
            {
				IRosterItem item = roster->findItem(jid);
				if (!item.isNull() && !item.name.isEmpty())
                    return QVariant();  // The contact name overriden in the roster

            }
            return FNicknameHash[jid.bare()];
        }
    }
    return QVariant();
}

bool Nickname::setRosterData(int AOrder, const QVariant &AValue, IRosterIndex *AIndex, int ARole)
{
    Q_UNUSED(AOrder);
    Q_UNUSED(AIndex);
    Q_UNUSED(ARole);
    Q_UNUSED(AValue);
    return false;
}

QMultiMap<int, IOptionsDialogWidget *> Nickname::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
    QStringList nodeTree = ANodeId.split(".", QString::SkipEmptyParts);
	if (FOptionsManager && nodeTree.count()==3 && nodeTree.at(0)==OPN_ACCOUNTS && nodeTree.at(2)=="Additional")
	{		
		widgets.insertMulti(OHO_ACCOUNTS_ADDITIONAL_NICKNAME, FOptionsManager->newOptionsDialogHeader(tr("User nickname"), AParent));
		OptionsNode node =  FAccountManager->findAccountById(nodeTree.at(1))->optionsNode();
		widgets.insertMulti(OWO_ACCOUNTS_ADDITIONAL_NICKNAME, FOptionsManager->newOptionsDialogWidget(node.node(OPV_NICKNAME), tr("Nickname:"), AParent));
		widgets.insertMulti(OWO_ACCOUNTS_ADDITIONAL_NICKNAME_MUC_DEFAULT, FOptionsManager->newOptionsDialogWidget(node.node(OPV_NICKNAME_MUC_DEFAULT), tr("Set as default for conferences"), AParent));
		if (Options::node(OPV_COMMON_ADVANCED).value().toBool())
		{
			widgets.insertMulti(OWO_ACCOUNTS_ADDITIONAL_NICKNAME_SET, FOptionsManager->newOptionsDialogWidget(node.node(OPV_NICKNAMESET), tr("Set contact name when added"), AParent));
			widgets.insertMulti(OWO_ACCOUNTS_ADDITIONAL_NICKNAME_CHANGE, FOptionsManager->newOptionsDialogWidget(node.node(OPV_NICKNAMECHANGE), tr("Change automatically"), AParent));
			widgets.insertMulti(OWO_ACCOUNTS_ADDITIONAL_NICKNAME_BROADCAST, FOptionsManager->newOptionsDialogWidget(node.node(OPV_NICKNAMEBROADCAST), tr("Broadcast within presence (violates XEP-0172!)"), AParent));
		}
	}
    return widgets;
}

QString Nickname::currentItemId(const Jid &AStreamJid) const
{
    return FIdHash.contains(AStreamJid.bare())?FIdHash[AStreamJid.bare()]:"current";
}

void Nickname::updateDataHolder(const Jid &AContactJid)
{
    if (FRostersModel)
    {
        QMultiMap<int,QVariant> findData;
        for(QList<int>::const_iterator it=FRosterIndexTypes.constBegin(); it!=FRosterIndexTypes.constEnd(); it++)
            findData.insert(RDR_KIND, *it);
        if (!AContactJid.isEmpty())
            findData.insert(RDR_PREP_BARE_JID, AContactJid.pBare());
        QList<IRosterIndex *> indexes = FRostersModel->rootIndex()->findChilds(findData,true);
        for (QList<IRosterIndex *>::const_iterator it=indexes.constBegin(); it!=indexes.constEnd(); it++)
            emit rosterDataChanged(*it, RDR_NAME);
    }
}


void Nickname::onOptionsOpened()
{}

void Nickname::onOptionsClosed()
{}

void Nickname::onOptionsChanged(const OptionsNode &ANode)
{
    if (ANode.name()==OPV_NICKNAME &&
        ANode.parent().name()=="account" &&
        ANode.parent().parent().name()=="accounts")
    {
        QString AccountID=ANode.parent().nspace();
		IAccount *account=FAccountManager->findAccountById(AccountID);
        if (account)
        {
            if (ANode.value().isValid())
                FNicknameHash.insert(account->streamJid().bare(), ANode.value().toString());
            else
                FNicknameHash.remove(account->streamJid().bare());
            updateDataHolder(account->streamJid());
            sendNick(ANode.value().toString(), account->streamJid());
        }
    }
}

void Nickname::onRosterIndexToolTips(IRosterIndex *AIndex, quint32 ALabelId, QMap<int, QString> &AToolTips)
{
    if ((ALabelId == AdvancedDelegateItem::DisplayId) && rosterDataTypes().contains(AIndex->kind()))
    {
        QString bareJid = Jid(AIndex->data(RDR_FULL_JID).toString()).bare();
        if (FNicknameHash.contains(bareJid))
			AToolTips.insert(RTTO_ROSTERSVIEW_NICKNAME, QString("<b>%1: %2</b>").arg(tr("Nickname")).arg(FNicknameHash[bareJid]));
    }
}

void Nickname::onRosterItemReceived(IRoster *ARoster, const IRosterItem &AItem, const IRosterItem &ABefore)
{
	OptionsNode node = FAccountManager->findAccountByStream(ARoster->streamJid())->optionsNode();
	if (node.value(OPV_NICKNAMESET).toBool() && ARoster->isOpen() && !AItem.isNull() && ABefore.isNull() && FNicknameHash.contains(AItem.itemJid.bare()))
        ARoster->renameItem(AItem.itemJid, FNicknameHash[AItem.itemJid.bare()]);
}

void Nickname::onPresenceOpened(IPresence *APresence)
{
    if (!FNicknameHash.contains(APresence->streamJid().bare()))
    {
		QString nickname = FAccountManager->findAccountByStream(APresence->streamJid())->optionsNode().value(OPV_NICKNAME).toString();
        if (!nickname.isEmpty())
            sendNick(nickname, APresence->streamJid());
    }
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_nickname, Nickname)
#endif
