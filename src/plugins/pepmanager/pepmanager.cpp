#include "pepmanager.h"
#include <definitions/optionwidgetorders.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionnodes.h>
#include <definitions/optionvalues.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>

#include <definitions/namespaces.h>
#include <utils/logger.h>

#define SHC_PUBSUB_EVENT               "/message/event[@xmlns='" NS_PUBSUB_EVENT "']/items"

#define NODE_NOTIFY_SUFFIX             "+notify"

#define DIC_PUBSUB                     "pubsub"
#define DIT_PEP                        "pep"

PEPManager::PEPManager()
{
	FDiscovery = NULL;
	FXmppStreamManager = NULL;
	FStanzaProcessor = NULL;
// *** <<< eyeCU <<< ***
	FOptionsManager = NULL;
	FMainWindowPlugin = NULL;
	FMenu = NULL;
// *** >>> eyeCU >>> ***
}

PEPManager::~PEPManager()
{

}

void PEPManager::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("PEP Manager");
	APluginInfo->description = tr("Allows other plugins to receive and publish PEP events");
	APluginInfo->version = "1.7";
	APluginInfo->author = "Maxim Ignatenko / Road Works Software";
	APluginInfo->homePage = "http://www.vacuum-im.org";
	APluginInfo->dependences.append(SERVICEDISCOVERY_UUID);
	APluginInfo->dependences.append(STANZAPROCESSOR_UUID);
	APluginInfo->dependences.append(XMPPSTREAMS_UUID);
}

bool PEPManager::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);
	IPlugin *plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
	{
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0,NULL);
	if (plugin)
	{
		FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0,NULL);
	if (plugin)
	{
		FXmppStreamManager = qobject_cast<IXmppStreamManager *>(plugin->instance());
		if (FXmppStreamManager)
		{
			connect(FXmppStreamManager->instance(),SIGNAL(streamOpened(IXmppStream *)),SLOT(onXmppStreamOpened(IXmppStream *)));
			connect(FXmppStreamManager->instance(),SIGNAL(streamClosed(IXmppStream *)),SLOT(onXmppStreamClosed(IXmppStream *)));
		}
	}
// *** <<< eyeCU <<< ***
    plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IMainWindowPlugin").value(0,NULL);
	if (plugin)
		FMainWindowPlugin = qobject_cast<IMainWindowPlugin *>(plugin->instance());
// *** >>> eyeCU >>> ***
	return FDiscovery!=NULL && FStanzaProcessor!=NULL && FXmppStreamManager!=NULL;
}

// *** <<< eyeCU <<< ***
bool PEPManager::initSettings()
{
    Options::setDefaultValue(OPV_PEP_DELETE_RETRACT, true);
    Options::setDefaultValue(OPV_PEP_DELETE_PUBLISHEMPTY, true);
    Options::setDefaultValue(OPV_PEP_NOTIFY_IGNOREOFFLINE, true);
    if (FOptionsManager)
    {
		IOptionsDialogNode dnode = {ONO_PEP, OPN_PEP, MNI_PEPMANAGER, tr("Personal events")};
        FOptionsManager->insertOptionsDialogNode(dnode);
		FOptionsManager->insertOptionsDialogHolder(this);
    }
	return true;
}
// *** >>> eyeCU >>> ***

bool PEPManager::stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	if (FStanzaHandles.value(AStreamJid) == AHandleId)
	{
		bool hooked = false;
		QString node = AStanza.firstElement("event",NS_PUBSUB_EVENT).firstChildElement("items").attribute("node",QString::null);

		foreach(int handlerId, FHandlersByNode.values(node))
		{
			if (FHandlersById.contains(handlerId))
				hooked = FHandlersById[handlerId]->processPEPEvent(AStreamJid,AStanza) || hooked;
		}
		AAccept = AAccept || hooked;
	}
	return false;
}
// *** <<< eyeCU <<< ***
QMultiMap<int, IOptionsDialogWidget *> PEPManager::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (FOptionsManager)
	{
		if (ANodeId == OPN_PEP)
		{
			widgets.insertMulti(OHO_PEP_DELETE, FOptionsManager->newOptionsDialogHeader(tr("Personal event removal"), AParent));
			if (Options::node(OPV_COMMON_ADVANCED).value().toBool())
			{
				widgets.insertMulti(OWO_PEP_DELETE_RETRACT, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_PEP_DELETE_RETRACT), tr("Retract item"), AParent));
				widgets.insertMulti(OWO_PEP_DELETE_PUBLISHEMPTY, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_PEP_DELETE_PUBLISHEMPTY), tr("Publish empty item"), AParent));
			}
		}
		else if (ANodeId == OPN_NOTIFICATIONS)
		{
			if (Options::node(OPV_COMMON_ADVANCED).value().toBool())
				widgets.insertMulti(OWO_NOTIFICATIONS_PEP_IGNOREOFFLINE, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_PEP_NOTIFY_IGNOREOFFLINE), tr("Ignore PEP events for offline contacts"), AParent));
		}
		else if (ANodeId == OPN_MESSAGES)
			widgets.insertMulti(OHO_MESSAGES_PEP, FOptionsManager->newOptionsDialogHeader(tr("Personal event notifications"), AParent));
		else
		{
			QStringList nodeTree = ANodeId.split(".", QString::SkipEmptyParts);
			if (nodeTree.count()==3 && nodeTree.at(0)==OPN_ACCOUNTS && nodeTree.at(2)=="Additional")
				widgets.insertMulti(OHO_ACCOUNTS_ADDITIONAL_PEP, FOptionsManager->newOptionsDialogHeader(tr("Personal events"), AParent));
		}
	}
    return widgets;
}
// *** >>> eyeCU >>> ***

bool PEPManager::isSupported(const Jid &AStreamJid) const
{
	bool supported = false;
	IDiscoInfo dinfo = FDiscovery!=NULL ? FDiscovery->discoInfo(AStreamJid, AStreamJid.domain()) : IDiscoInfo();
	for (int i=0; !supported && i<dinfo.identity.count(); i++)
	{
		const IDiscoIdentity &ident = dinfo.identity.at(i);
		supported = ident.category==DIC_PUBSUB && ident.type==DIT_PEP;
	}
	return supported;
}

bool PEPManager::publishItem(const Jid &AStreamJid, const QString &ANode, const QDomElement &AItem)
{
	if (FStanzaProcessor && isSupported(AStreamJid))
	{
		Stanza iq("iq");
		iq.setType("set").setId(FStanzaProcessor->newId());
		QDomElement publish = iq.addElement("pubsub", NS_PUBSUB).appendChild(iq.createElement("publish")).toElement();
		publish.setAttribute("node", ANode);
		publish.appendChild(AItem.cloneNode(true));
		if (FStanzaProcessor->sendStanzaOut(AStreamJid,iq))
		{
			LOG_STRM_INFO(AStreamJid,QString("PEP item publish request sent, node=%1, id=%2").arg(ANode,iq.id()));
			return true;
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,QString("Failed to send PEP item publish request, node=%1").arg(ANode));
		}
	}
	else if (FStanzaProcessor)
	{
		LOG_STRM_ERROR(AStreamJid,QString("Failed to publish PEP item, node=%1: Not supported").arg(ANode));
	}
	return false;
}

// *** <<< eyeCU <<< ***
bool PEPManager::deleteItem(const Jid &AStreamJid, const QString &ANode, const QDomElement &AItem)
{
    if (FStanzaProcessor && isSupported(AStreamJid))
    {
        Stanza iq("iq");
        iq.setType("set").setId(FStanzaProcessor->newId());
        QDomElement delItem = iq.addElement("pubsub", NS_PUBSUB)
                              .appendChild(iq.createElement("retract"))
                              .toElement();
        delItem.setAttribute("node", ANode);
        delItem.setAttribute("notify", 1);
        delItem.appendChild(AItem.cloneNode(true));
        return FStanzaProcessor->sendStanzaOut(AStreamJid, iq);
    }
    return false;
}

Action * PEPManager::addAction(int AGroup, bool ASort)
{
	if (!FMenu)
	{		
		FMenu = new Menu(FMainWindowPlugin->mainWindow()->bottomToolBarChanger()->toolBar());
		FMenu->setIcon(RSR_STORAGE_MENUICONS, MNI_PEPMANAGER);
		QToolButton *button = FMainWindowPlugin->mainWindow()->bottomToolBarChanger()->insertAction(FMenu->menuAction());
		button->setPopupMode(QToolButton::InstantPopup);
		button->setToolTip(tr("Extended Status"));
	}
	Action *action = new Action(FMainWindowPlugin->mainWindow()->topToolBarChanger()->toolBar());
	FMenu->addAction(action, AGroup, ASort);
	return action;
}

QList<Action *> PEPManager::groupActions(int AGroup)
{
	return FMenu->actions(AGroup);
}
// *** >>> eyeCU >>> ***

IPEPHandler *PEPManager::nodeHandler(int AHandleId) const
{
	return FHandlersById.value(AHandleId, NULL);
}

int PEPManager::insertNodeHandler(const QString &ANode, IPEPHandler *AHandle)
{
	static int handleId = 0;

	handleId++;
	while(handleId <= 0 || FHandlersById.contains(handleId))
		handleId = (handleId > 0) ? handleId+1 : 1;

	FHandlersById.insert(handleId, AHandle);
	FHandlersByNode.insertMulti(ANode, handleId);
	connect(AHandle->instance(),SIGNAL(destroyed(QObject *)),SLOT(onPEPHandlerDestroyed(QObject *)));

	return handleId;
}

bool PEPManager::removeNodeHandler(int AHandleId)
{
	if (FHandlersById.contains(AHandleId))
	{
		QList<QString> nodes = FHandlersByNode.keys(AHandleId);
		foreach(const QString &node, nodes)
			FHandlersByNode.remove(node, AHandleId);
		FHandlersById.remove(AHandleId);
		return true;
	}
	return false;
}

void PEPManager::onXmppStreamOpened(IXmppStream *AXmppStream)
{
	if (FStanzaProcessor)
	{
		IStanzaHandle shandle;
		shandle.handler = this;
		shandle.order = SHO_DEFAULT;
		shandle.direction = IStanzaHandle::DirectionIn;
		shandle.streamJid = AXmppStream->streamJid();
		shandle.conditions.append(SHC_PUBSUB_EVENT);
		FStanzaHandles.insert(AXmppStream->streamJid(), FStanzaProcessor->insertStanzaHandle(shandle));
	}
}

void PEPManager::onXmppStreamClosed(IXmppStream *AXmppStream)
{
	if (FStanzaProcessor)
		FStanzaProcessor->removeStanzaHandle(FStanzaHandles.take(AXmppStream->streamJid()));
}

void PEPManager::onPEPHandlerDestroyed(QObject *AHandler)
{
	foreach(int id, FHandlersById.keys())
	{
		IPEPHandler *handler = FHandlersById.value(id);
		if (handler->instance() == AHandler)
		{
			removeNodeHandler(id);
			break;
		}
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_pepmanager, PEPManager)
#endif
