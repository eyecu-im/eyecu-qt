#include "commands.h"

#include <definitions/namespaces.h>
#include <definitions/discofeaturehandlerorders.h>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/xmpperrors.h>
#include <definitions/xmppurihandlerorders.h>
#include <definitions/stanzahandlerorders.h>
#include <utils/logger.h>
#include <utils/menu.h>

#define COMMAND_TAG_NAME              "command"
#define COMMANDS_TIMEOUT              60000

#define SHC_COMMANDS                  "/iq[@type='set']/command[@xmlns='" NS_COMMANDS "']"

#define ADR_STREAM_JID                Action::DR_StreamJid
#define ADR_COMMAND_JID               Action::DR_Parametr1
#define ADR_COMMAND_NODE              Action::DR_Parametr2

#define DIC_CLIENT                    "client"
#define DIC_AUTOMATION                "automation"
#define DIT_COMMAND_NODE              "command-node"
#define DIT_COMMAND_LIST              "command-list"

Commands::Commands()
{
	FDataForms = NULL;
	FXmppStreamManager = NULL;
	FStanzaProcessor = NULL;
	FDiscovery = NULL;
	FPresenceManager = NULL;
	FXmppUriQueries = NULL;
}

Commands::~Commands()
{

}

void Commands::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Ad-Hoc Commands");
	APluginInfo->description = tr("Allows to perform special commands provided by various services");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Potapov S.A. aka Lion";
	APluginInfo->homePage = "http://www.vacuum-im.org";
	APluginInfo->dependences.append(DATAFORMS_UUID);
	APluginInfo->dependences.append(XMPPSTREAMS_UUID);
	APluginInfo->dependences.append(STANZAPROCESSOR_UUID);
}

bool Commands::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);
	IPlugin *plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
	{
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());
		if (FDiscovery)
		{
			connect(FDiscovery->instance(),SIGNAL(discoInfoReceived(const IDiscoInfo &)),SLOT(onDiscoInfoReceived(const IDiscoInfo &)));
			connect(FDiscovery->instance(),SIGNAL(discoInfoRemoved(const IDiscoInfo &)),SLOT(onDiscoInfoRemoved(const IDiscoInfo &)));
			connect(FDiscovery->instance(),SIGNAL(discoItemsReceived(const IDiscoItems &)),SLOT(onDiscoItemsReceived(const IDiscoItems &)));
		}
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

	plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0,NULL);
	if (plugin)
	{
		FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IDataForms").value(0,NULL);
	if (plugin)
	{
		FDataForms = qobject_cast<IDataForms *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0,NULL);
	if (plugin)
	{
		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());
		if (FPresenceManager)
		{
			connect(FPresenceManager->instance(),SIGNAL(presenceItemReceived(IPresence *, const IPresenceItem &, const IPresenceItem &)),
				SLOT(onPresenceItemReceived(IPresence *, const IPresenceItem &, const IPresenceItem &)));
		}
	}

	plugin = APluginManager->pluginInterface("IXmppUriQueries").value(0,NULL);
	if (plugin)
	{
		FXmppUriQueries = qobject_cast<IXmppUriQueries *>(plugin->instance());
	}

	return FXmppStreamManager!=NULL && FStanzaProcessor!=NULL && FDataForms!=NULL;
}

bool Commands::initObjects()
{
	XmppError::registerError(NS_COMMANDS,XERR_COMMANDS_MALFORMED_ACTION,tr("Can not understand the specified action"));
	XmppError::registerError(NS_COMMANDS,XERR_COMMANDS_BAD_ACTION,tr("Can not accept the specified action"));
	XmppError::registerError(NS_COMMANDS,XERR_COMMANDS_BAD_LOCALE,tr("Can not accept the specified language/locale"));
	XmppError::registerError(NS_COMMANDS,XERR_COMMANDS_BAD_PAYLOAD,tr("The data form did not provide one or more required fields"));
	XmppError::registerError(NS_COMMANDS,XERR_COMMANDS_BAD_SESSIONID,tr("Specified session not present"));
	XmppError::registerError(NS_COMMANDS,XERR_COMMANDS_SESSION_EXPIRED,tr("Specified session is no longer active"));

	if (FDiscovery)
	{
		registerDiscoFeatures();
		FDiscovery->insertDiscoHandler(this);
		FDiscovery->insertFeatureHandler(NS_COMMANDS,this,DFO_DEFAULT);
	}
	if (FXmppUriQueries)
	{
		FXmppUriQueries->insertUriHandler(XUHO_DEFAULT,this);
	}
	return true;
}

bool Commands::stanzaReadWrite(int AHandlerId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	if (FSHICommands.value(AStreamJid) == AHandlerId)
	{
		AAccept = true;

		ICommandRequest request;
		request.streamJid = AStreamJid;
		request.contactJid = AStanza.from();
		request.stanzaId = AStanza.id();

		QDomElement cmdElem = AStanza.firstElement(COMMAND_TAG_NAME,NS_COMMANDS);
		request.sessionId = cmdElem.attribute("sessionid");
		request.node = cmdElem.attribute("node");
		request.action = cmdElem.attribute("action",COMMAND_ACTION_EXECUTE);

		if (FDataForms)
		{
			QDomElement formElem = cmdElem.firstChildElement("x");
			while (!formElem.isNull() && formElem.namespaceURI()!=NS_JABBER_DATA)
				formElem = formElem.nextSiblingElement("x");
			if (!formElem.isNull())
				request.form = FDataForms->dataForm(formElem);
		}

		ICommandServer *server = FServers.value(request.node);
		if (server && !server->isCommandPermitted(request.streamJid,request.contactJid,request.node))
		{
			Stanza error = FStanzaProcessor->makeReplyError(AStanza,XmppStanzaError::EC_FORBIDDEN);
			FStanzaProcessor->sendStanzaOut(AStreamJid,error);
			LOG_STRM_WARNING(AStreamJid,QString("Regected forbidden command from=%1, node=%2").arg(AStanza.from(),request.node));
		}
		else if (!server || !server->receiveCommandRequest(request))
		{
			XmppStanzaError err(XmppStanzaError::EC_BAD_REQUEST);
			err.setAppCondition(NS_COMMANDS,XERR_COMMANDS_MALFORMED_ACTION);
			Stanza error = FStanzaProcessor->makeReplyError(AStanza,err);
			FStanzaProcessor->sendStanzaOut(AStreamJid,error);
			LOG_STRM_WARNING(AStreamJid,QString("Regected bad command from=%1, node=%2").arg(AStanza.from(),request.node));
		}
		else
		{
			LOG_STRM_INFO(AStreamJid,QString("Accepted command request from=%1, id=%2, node=%3").arg(AStanza.from(),request.stanzaId,request.node));
		}
	}
	else
	{
		REPORT_ERROR("Received unexpected stanza");
	}
	return false;
}

void Commands::stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza)
{
	if (FRequests.contains(AStanza.id()))
	{
		FRequests.removeAt(FRequests.indexOf(AStanza.id()));
		if (AStanza.isResult())
		{
			ICommandResult result;
			result.streamJid = AStreamJid;
			result.contactJid = AStanza.from();
			result.stanzaId = AStanza.id();

			QDomElement cmdElem = AStanza.firstElement(COMMAND_TAG_NAME,NS_COMMANDS);
			result.sessionId = cmdElem.attribute("sessionid");
			result.node = cmdElem.attribute("node");
			result.status = cmdElem.attribute("status");

			QDomElement actElem = cmdElem.firstChildElement("actions");
			result.execute = actElem.attribute("execute");
			actElem = actElem.firstChildElement();
			while (!actElem.isNull())
			{
				result.actions.append(actElem.tagName());
				actElem = actElem.nextSiblingElement();
			}

			QDomElement noteElem = cmdElem.firstChildElement("note");
			while (!noteElem.isNull())
			{
				ICommandNote note;
				note.type = noteElem.attribute("type",COMMAND_NOTE_INFO);
				note.message = noteElem.text();
				result.notes.append(note);
				noteElem = noteElem.nextSiblingElement("note");
			}

			if (FDataForms)
			{
				QDomElement formElem = cmdElem.firstChildElement("x");
				while (!formElem.isNull() && formElem.namespaceURI()!=NS_JABBER_DATA)
					formElem = formElem.nextSiblingElement("x");
				if (!formElem.isNull())
					result.form = FDataForms->dataForm(formElem);
			}

			foreach(ICommandClient *client, FClients)
				if (client->receiveCommandResult(result))
					break;

			LOG_STRM_INFO(AStreamJid,QString("Received command request answer from=%1, id=%2, node=%3").arg(AStanza.from(),AStanza.id(),result.node));
		}
		else
		{
			ICommandError err;
			err.stanzaId = AStanza.id();
			err.error = XmppStanzaError(AStanza);
			foreach(ICommandClient *client, FClients)
				if (client->receiveCommandError(err))
					break;

			LOG_STRM_WARNING(AStreamJid,QString("Failed to received command request answer from=%1, id=%2: %3").arg(AStanza.from(),AStanza.id(),err.error.condition()));
		}
	}
}

bool Commands::xmppUriOpen(const Jid &AStreamJid, const Jid &AContactJid, const QString &AAction, const QMultiMap<QString, QString> &AParams)
{
	if (AAction == "command")
	{
		QString node = AParams.value("node");
		if (!node.isEmpty())
		{
			QString action = AParams.value("action","execute");
			if (action == "execute")
				executeCommand(AStreamJid, AContactJid, node);
		}
		return true;
	}
	return false;
}

void Commands::fillDiscoInfo(IDiscoInfo &ADiscoInfo)
{
	if (ADiscoInfo.node == NS_COMMANDS)
	{
		IDiscoIdentity identity;
		identity.category = DIC_AUTOMATION;
		identity.type = DIT_COMMAND_LIST;
		identity.name = "Commands";
		ADiscoInfo.identity.append(identity);

		if (!ADiscoInfo.features.contains(NS_COMMANDS))
			ADiscoInfo.features.append(NS_COMMANDS);
	}
	else if (FServers.contains(ADiscoInfo.node))
	{
		ICommandServer *server = FServers.value(ADiscoInfo.node);
		if (server && server->isCommandPermitted(ADiscoInfo.streamJid,ADiscoInfo.contactJid,ADiscoInfo.node))
		{
			IDiscoIdentity identity;
			identity.category = DIC_AUTOMATION;
			identity.type = DIT_COMMAND_NODE;
			identity.name = server->commandName(ADiscoInfo.node);
			ADiscoInfo.identity.append(identity);

			if (!ADiscoInfo.features.contains(NS_COMMANDS))
				ADiscoInfo.features.append(NS_COMMANDS);
			if (!ADiscoInfo.features.contains(NS_JABBER_DATA))
				ADiscoInfo.features.append(NS_JABBER_DATA);
		}
	}
}

void Commands::fillDiscoItems(IDiscoItems &ADiscoItems)
{
	if (ADiscoItems.node == NS_COMMANDS)
	{
		foreach(const QString &node, FServers.keys())
		{
			ICommandServer *server = FServers.value(node);
			if (server && server->isCommandPermitted(ADiscoItems.streamJid,ADiscoItems.contactJid,node))
			{
				IDiscoItem ditem;
				ditem.itemJid = ADiscoItems.streamJid;
				ditem.node = node;
				ditem.name = server->commandName(node);
				ADiscoItems.items.append(ditem);
			}
		}
	}
	else if (ADiscoItems.node.isEmpty() && !FServers.isEmpty())
	{
		IDiscoItem ditem;
		ditem.itemJid = ADiscoItems.streamJid;
		ditem.node = NS_COMMANDS;
		ditem.name = "Commands";
		ADiscoItems.items.append(ditem);
	}
}

bool Commands::execDiscoFeature(const Jid &AStreamJid, const QString &AFeature, const IDiscoInfo &ADiscoInfo)
{
	if (AFeature==NS_COMMANDS && !ADiscoInfo.node.isEmpty() && FDiscovery->findIdentity(ADiscoInfo.identity,DIC_AUTOMATION,DIT_COMMAND_NODE)>=0)
	{
		executeCommand(AStreamJid,ADiscoInfo.contactJid,ADiscoInfo.node);
		return true;
	}
	return false;
}

Action *Commands::createDiscoFeatureAction(const Jid &AStreamJid, const QString &AFeature, const IDiscoInfo &ADiscoInfo, QWidget *AParent)
{
	if (FSHICommands.contains(AStreamJid) && AFeature==NS_COMMANDS)
	{
		if (FDiscovery->findIdentity(ADiscoInfo.identity,DIC_AUTOMATION,DIT_COMMAND_NODE)>=0)
		{
			if (!ADiscoInfo.node.isEmpty())
			{
				Action *action = new Action(AParent);
				action->setText(tr("Execute"));
				action->setIcon(RSR_STORAGE_MENUICONS,MNI_COMMANDS);
				action->setData(ADR_STREAM_JID,AStreamJid.full());
				action->setData(ADR_COMMAND_JID,ADiscoInfo.contactJid.full());
				action->setData(ADR_COMMAND_NODE,ADiscoInfo.node);
				connect(action,SIGNAL(triggered(bool)),SLOT(onExecuteActionTriggered(bool)));
				return action;
			}
		}
		else if (FCommands.value(AStreamJid).contains(ADiscoInfo.contactJid))
		{
			QList<ICommand> commands = FCommands.value(AStreamJid).value(ADiscoInfo.contactJid);
			if (!commands.isEmpty())
			{
				Menu *execMenu = new Menu(AParent);
				execMenu->setTitle(tr("Commands"));
				execMenu->setIcon(RSR_STORAGE_MENUICONS,MNI_COMMANDS);
				foreach (const ICommand &command, commands)
				{
					Action *action = new Action(execMenu);
					action->setText(command.name);
					action->setData(ADR_STREAM_JID,AStreamJid.full());
					action->setData(ADR_COMMAND_JID,command.itemJid.full());
					action->setData(ADR_COMMAND_NODE,command.node);
					connect(action,SIGNAL(triggered(bool)),SLOT(onExecuteActionTriggered(bool)));
					execMenu->addAction(action,AG_DEFAULT,false);
				}
				return execMenu->menuAction();
			}
		}
		else if (ADiscoInfo.features.contains(NS_COMMANDS))
		{
			Action *action = new Action(AParent);
			action->setText(tr("Request commands"));
			action->setIcon(RSR_STORAGE_MENUICONS,MNI_COMMANDS);
			action->setData(ADR_STREAM_JID,AStreamJid.full());
			action->setData(ADR_COMMAND_JID,ADiscoInfo.contactJid.full());
			connect(action,SIGNAL(triggered(bool)),SLOT(onRequestActionTriggered(bool)));
			return action;
		}
	}
	return NULL;
}

QList<QString> Commands::commandNodes() const
{
	return FServers.keys();
}

ICommandServer *Commands::commandServer(const QString &ANode) const
{
	return FServers.value(ANode);
}

void Commands::insertServer(const QString &ANode, ICommandServer *AServer)
{
	if (AServer && !FServers.contains(ANode))
	{
		FServers.insert(ANode,AServer);
		emit serverInserted(ANode, AServer);
	}
}

void Commands::removeServer(const QString &ANode)
{
	if (FServers.contains(ANode))
	{
		FServers.remove(ANode);
		emit serverRemoved(ANode);
	}
}

void Commands::insertClient(ICommandClient *AClient)
{
	if (AClient && !FClients.contains(AClient))
	{
		FClients.append(AClient);
		emit clientInserted(AClient);
	}
}

void Commands::removeClient(ICommandClient *AClient)
{
	if (FClients.contains(AClient))
	{
		FClients.removeAt(FClients.indexOf(AClient));
		emit clientRemoved(AClient);
	}
}

QString Commands::sendCommandRequest(const ICommandRequest &ARequest)
{
	if (FStanzaProcessor)
	{
		Stanza request(STANZA_KIND_IQ);
		request.setType(STANZA_TYPE_SET).setTo(ARequest.contactJid.full()).setUniqueId();
		QDomElement cmdElem = request.addElement(COMMAND_TAG_NAME,NS_COMMANDS);
		cmdElem.setAttribute("node",ARequest.node);
		if (!ARequest.sessionId.isEmpty())
			cmdElem.setAttribute("sessionid",ARequest.sessionId);
		if (!ARequest.action.isEmpty())
			cmdElem.setAttribute("action",ARequest.action);
		if (FDataForms && !ARequest.form.type.isEmpty())
			FDataForms->xmlForm(ARequest.form,cmdElem);
		if (FStanzaProcessor->sendStanzaRequest(this,ARequest.streamJid,request,COMMANDS_TIMEOUT))
		{
			LOG_STRM_INFO(ARequest.streamJid,QString("Command request sent to=%1, node=%2, sid=%3, id=%4").arg(ARequest.contactJid.full(),ARequest.node,ARequest.sessionId,request.id()));
			FRequests.append(request.id());
			return request.id();
		}
		else
		{
			LOG_STRM_WARNING(ARequest.streamJid,QString("Failed to send command request to=%1, node=%2, sid=%3").arg(ARequest.contactJid.full(),ARequest.node,ARequest.sessionId));
		}
	}
	return QString();
}

bool Commands::sendCommandResult(const ICommandResult &AResult)
{
	if (FStanzaProcessor)
	{
		Stanza result(STANZA_KIND_IQ);
		result.setType(STANZA_TYPE_RESULT).setTo(AResult.contactJid.full()).setId(AResult.stanzaId);

		QDomElement cmdElem = result.addElement(COMMAND_TAG_NAME,NS_COMMANDS);
		cmdElem.setAttribute("node",AResult.node);
		cmdElem.setAttribute("sessionid",AResult.sessionId);
		cmdElem.setAttribute("status",AResult.status);

		if (!AResult.actions.isEmpty())
		{
			QDomElement actElem = cmdElem.appendChild(result.createElement("actions")).toElement();
			actElem.setAttribute("execute",AResult.execute);
			foreach(const QString &action, AResult.actions)
				actElem.appendChild(result.createElement(action));
		}

		if (FDataForms && !AResult.form.type.isEmpty())
			FDataForms->xmlForm(AResult.form,cmdElem);

		foreach(const ICommandNote &note, AResult.notes)
		{
			QDomElement noteElem = cmdElem.appendChild(result.createElement("note")).toElement();
			noteElem.setAttribute("type",note.type);
			noteElem.appendChild(result.createTextNode(note.message));
		}

		if (FStanzaProcessor->sendStanzaOut(AResult.streamJid,result))
		{
			LOG_STRM_INFO(AResult.streamJid,QString("Command result sent to=%1, node=%2, sid=%3, id=%4").arg(AResult.contactJid.full(),AResult.node,AResult.sessionId,AResult.stanzaId));
			return true;
		}
		else
		{
			LOG_STRM_WARNING(AResult.streamJid,QString("Failed to send command result to=%1, node=%2, sid=%3, id=%4").arg(AResult.contactJid.full(),AResult.node,AResult.sessionId,AResult.stanzaId));
		}
	}
	return false;
}

QList<ICommand> Commands::contactCommands(const Jid &AStreamJid, const Jid &AContactJid) const
{
	return FCommands.value(AStreamJid).value(AContactJid);
}

bool Commands::executeCommand(const Jid &AStreamJid, const Jid &ACommandJid, const QString &ANode)
{
	IXmppStream *stream = FXmppStreamManager!=NULL ? FXmppStreamManager->findXmppStream(AStreamJid) : NULL;
	if (FDataForms && stream && stream->isOpen())
	{
		LOG_STRM_INFO(AStreamJid,QString("Executing command, server=%1, node=%2").arg(ACommandJid.full(),ANode));
		CommandDialog *dialog = new CommandDialog(this,FDataForms,AStreamJid,ACommandJid,ANode,NULL);
		connect(stream->instance(),SIGNAL(closed()),dialog,SLOT(reject()));
		dialog->executeCommand();
		dialog->show();
		return true;
	}
	return false;
}

ICommandResult Commands::prepareResult(const ICommandRequest &ARequest) const
{
	ICommandResult result;
	result.streamJid = ARequest.streamJid;
	result.contactJid = ARequest.contactJid;
	result.node = ARequest.node;
	result.stanzaId = ARequest.stanzaId;
	result.sessionId = ARequest.sessionId;
	return result;
}

void Commands::registerDiscoFeatures()
{
	IDiscoFeature dfeature;
	dfeature.active = true;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_COMMANDS);
	dfeature.var = NS_COMMANDS;
	dfeature.name = tr("Ad-Hoc Commands");
	dfeature.description = tr("Supports the running or performing of the special services commands");
	FDiscovery->insertDiscoFeature(dfeature);
}

void Commands::onXmppStreamOpened(IXmppStream *AXmppStream)
{
	if (FStanzaProcessor)
	{
		IStanzaHandle shandle;
		shandle.handler = this;
		shandle.order = SHO_DEFAULT;
		shandle.direction = IStanzaHandle::DirectionIn;
		shandle.streamJid = AXmppStream->streamJid();
		shandle.conditions.append(SHC_COMMANDS);
		FSHICommands.insert(AXmppStream->streamJid(), FStanzaProcessor->insertStanzaHandle(shandle));
	}
}

void Commands::onXmppStreamClosed(IXmppStream *AXmppStream)
{
	if (FStanzaProcessor)
		FStanzaProcessor->removeStanzaHandle(FSHICommands.take(AXmppStream->streamJid()));
	FCommands.remove(AXmppStream->streamJid());
	FOnlineAgents.remove(AXmppStream->streamJid());
}

void Commands::onDiscoInfoReceived(const IDiscoInfo &AInfo)
{
	if (AInfo.node.isEmpty() && FDiscovery->findIdentity(AInfo.identity,DIC_CLIENT,QString())<0)
		if (AInfo.features.contains(NS_COMMANDS) && !FCommands.value(AInfo.streamJid).contains(AInfo.contactJid))
			FDiscovery->requestDiscoItems(AInfo.streamJid,AInfo.contactJid,NS_COMMANDS);
}

void Commands::onDiscoInfoRemoved(const IDiscoInfo &AInfo)
{
	if (AInfo.node.isEmpty())
		FCommands[AInfo.streamJid].remove(AInfo.contactJid);
}

void Commands::onDiscoItemsReceived(const IDiscoItems &AItems)
{
	if (AItems.node == NS_COMMANDS)
	{
		QList<ICommand> &commands = FCommands[AItems.streamJid][AItems.contactJid];
		commands.clear();
		foreach(const IDiscoItem &ditem, AItems.items)
		{
			if (!ditem.node.isEmpty() && ditem.itemJid.isValid())
			{
				ICommand command;
				command.node = ditem.node;
				command.name = !ditem.name.isEmpty() ? ditem.name : ditem.node;
				command.itemJid = ditem.itemJid;
				commands.append(command);
			}
		}
		emit commandsUpdated(AItems.streamJid,AItems.contactJid,commands);
	}
}

void Commands::onPresenceItemReceived(IPresence *APresence, const IPresenceItem &AItem, const IPresenceItem &ABefore)
{
	Q_UNUSED(ABefore);
	if (FDiscovery && APresence->isOpen() && !AItem.itemJid.hasNode())
	{
		if (FDiscovery->discoInfo(APresence->streamJid(),AItem.itemJid).features.contains(NS_COMMANDS))
		{
			QList<Jid> &online = FOnlineAgents[APresence->streamJid()];
			if (AItem.show==IPresence::Offline || AItem.show==IPresence::Error)
			{
				if (online.contains(AItem.itemJid))
				{
					online.removeAll(AItem.itemJid);
					FDiscovery->requestDiscoItems(APresence->streamJid(),AItem.itemJid,NS_COMMANDS);
				}
			}
			else
			{
				if (!online.contains(AItem.itemJid))
				{
					online.append(AItem.itemJid);
					FDiscovery->requestDiscoItems(APresence->streamJid(),AItem.itemJid,NS_COMMANDS);
				}
			}
		}
	}
}

void Commands::onExecuteActionTriggered(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		Jid streamJid = action->data(ADR_STREAM_JID).toString();
		Jid commandJid = action->data(ADR_COMMAND_JID).toString();
		QString node = action->data(ADR_COMMAND_NODE).toString();
		executeCommand(streamJid,commandJid,node);
	}
}

void Commands::onRequestActionTriggered(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (FDiscovery && action)
	{
		Jid streamJid = action->data(ADR_STREAM_JID).toString();
		Jid commandJid = action->data(ADR_COMMAND_JID).toString();
		FDiscovery->requestDiscoItems(streamJid,commandJid,NS_COMMANDS);
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_commands, Commands)
#endif
