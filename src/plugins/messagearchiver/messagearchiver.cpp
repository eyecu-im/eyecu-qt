#include "messagearchiver.h"

#include <QDir>
#include <QFile>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/shortcuts.h>
#include <definitions/namespaces.h>
#include <definitions/actiongroups.h>
#include <definitions/toolbargroups.h>
#include <definitions/internalerrors.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/messagedataroles.h>
#include <definitions/rosterindexkinds.h>
#include <definitions/rosterindexroles.h>
#include <definitions/stanzahandlerorders.h>
#include <definitions/sessionnegotiatororders.h>
#include <definitions/shortcutgrouporders.h>
#include <definitions/statisticsparams.h>
#include <utils/widgetmanager.h>
#include <utils/xmpperror.h>
#include <utils/shortcuts.h>
#include <utils/options.h>
#include <utils/logger.h>

#define ARCHIVE_DIR_NAME           "archive"
#define PENDING_FILE_NAME          "pending.xml"
#define SESSIONS_FILE_NAME         "sessions.xml"
#define ARCHIVE_REQUEST_TIMEOUT    30000

#define SHC_MESSAGE_BODY           "/message"
#define SHC_PREFS                  "/iq[@type='set']/pref[@xmlns=" NS_ARCHIVE "]"
#define SHC_PREFS_OLD              "/iq[@type='set']/pref[@xmlns=" NS_ARCHIVE_OLD "]"

#define ADR_STREAM_JID             Action::DR_StreamJid
#define ADR_CONTACT_JID            Action::DR_Parametr1
#define ADR_ITEM_SAVE              Action::DR_Parametr2
#define ADR_ITEM_OTR               Action::DR_Parametr3
#define ADR_METHOD_LOCAL           Action::DR_Parametr1
#define ADR_METHOD_AUTO            Action::DR_Parametr2
#define ADR_METHOD_MANUAL          Action::DR_Parametr3
#define ADR_FILTER_START           Action::DR_Parametr2
#define ADR_FILTER_END             Action::DR_Parametr3
#define ADR_GROUP_KIND             Action::DR_Parametr4

#define SFP_LOGGING                "logging"
#define SFV_MAY_LOGGING            "may"
#define SFV_MUSTNOT_LOGGING        "mustnot"

#define PST_ARCHIVE_PREFS          "pref"
#define PSN_ARCHIVE_PREFS          NS_ARCHIVE

#define NS_ARCHIVE_OLD             "http://www.xmpp.org/extensions/xep-0136.html#ns"
#define NS_ARCHIVE_OLD_AUTO        "http://www.xmpp.org/extensions/xep-0136.html#ns-auto"
#define NS_ARCHIVE_OLD_MANAGE      "http://www.xmpp.org/extensions/xep-0136.html#ns-manage"
#define NS_ARCHIVE_OLD_MANUAL      "http://www.xmpp.org/extensions/xep-0136.html#ns-manual"
#define NS_ARCHIVE_OLD_PREF        "http://www.xmpp.org/extensions/xep-0136.html#ns-pref"

MessageArchiver::MessageArchiver()
{
	FPluginManager = NULL;
	FXmppStreamManager = NULL;
	FStanzaProcessor = NULL;
	FOptionsManager = NULL;
	FPrivateStorage = NULL;
	FAccountManager = NULL;
	FRostersViewPlugin = NULL;
	FDiscovery = NULL;
	FDataForms = NULL;
	FMessageWidgets = NULL;
	FSessionNegotiation = NULL;
	FRosterManager = NULL;
	FMultiChatManager = NULL;
}

MessageArchiver::~MessageArchiver()
{

}

void MessageArchiver::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("History");
	APluginInfo->description = tr("Allows to save the history of communications");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Potapov S.A. aka Lion";
	APluginInfo->homePage = "http://www.vacuum-im.org";
	APluginInfo->dependences.append(XMPPSTREAMS_UUID);
	APluginInfo->dependences.append(STANZAPROCESSOR_UUID);
}

bool MessageArchiver::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);
	FPluginManager = APluginManager;

	IPlugin *plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0,NULL);
	if (plugin)
	{
		FXmppStreamManager = qobject_cast<IXmppStreamManager *>(plugin->instance());
		if (FXmppStreamManager)
		{
			connect(FXmppStreamManager->instance(),SIGNAL(streamOpened(IXmppStream *)),SLOT(onXmppStreamOpened(IXmppStream *)));
			connect(FXmppStreamManager->instance(),SIGNAL(streamClosed(IXmppStream *)),SLOT(onXmppStreamClosed(IXmppStream *)));
			connect(FXmppStreamManager->instance(),SIGNAL(streamAboutToClose(IXmppStream *)),SLOT(onXmppStreamAboutToClose(IXmppStream *)));
		}
	}

	plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0,NULL);
	if (plugin)
	{
		FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
	{
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IPrivateStorage").value(0,NULL);
	if (plugin)
	{
		FPrivateStorage = qobject_cast<IPrivateStorage *>(plugin->instance());
		if (FPrivateStorage)
		{
			connect(FPrivateStorage->instance(),SIGNAL(dataSaved(const QString &, const Jid &, const QDomElement &)),
				SLOT(onPrivateDataLoadedSaved(const QString &, const Jid &, const QDomElement &)));
			connect(FPrivateStorage->instance(),SIGNAL(dataLoaded(const QString &, const Jid &, const QDomElement &)),
				SLOT(onPrivateDataLoadedSaved(const QString &, const Jid &, const QDomElement &)));
			connect(FPrivateStorage->instance(),SIGNAL(dataChanged(const Jid &, const QString &, const QString &)),
				SLOT(onPrivateDataChanged(const Jid &, const QString &, const QString &)));
		}
	}

	plugin = APluginManager->pluginInterface("IAccountManager").value(0,NULL);
	if (plugin)
	{
		FAccountManager = qobject_cast<IAccountManager *>(plugin->instance());
		if (FAccountManager)
		{
			connect(FAccountManager->instance(),SIGNAL(accountInserted(IAccount *)),SLOT(onAccountInserted(IAccount *)));
			connect(FAccountManager->instance(),SIGNAL(accountRemoved(IAccount *)),SLOT(onAccountRemoved(IAccount *)));
		}
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

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
	{
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());
		if (FDiscovery)
		{
			connect(FDiscovery->instance(),SIGNAL(discoInfoReceived(const IDiscoInfo &)),SLOT(onDiscoveryInfoReceived(const IDiscoInfo &)));
		}
	}

	plugin = APluginManager->pluginInterface("IDataForms").value(0,NULL);
	if (plugin)
	{
		FDataForms = qobject_cast<IDataForms *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
		if (FMessageWidgets)
		{
			connect(FMessageWidgets->instance(),SIGNAL(toolBarWidgetCreated(IMessageToolBarWidget *)),SLOT(onToolBarWidgetCreated(IMessageToolBarWidget *)));
		}
	}

	plugin = APluginManager->pluginInterface("ISessionNegotiation").value(0,NULL);
	if (plugin)
	{
		FSessionNegotiation = qobject_cast<ISessionNegotiation *>(plugin->instance());
		if (FSessionNegotiation)
		{
			connect(FSessionNegotiation->instance(),SIGNAL(sessionActivated(const IStanzaSession &)),
				SLOT(onStanzaSessionActivated(const IStanzaSession &)));
			connect(FSessionNegotiation->instance(),SIGNAL(sessionTerminated(const IStanzaSession &)),
				SLOT(onStanzaSessionTerminated(const IStanzaSession &)));
		}
	}

	plugin = APluginManager->pluginInterface("IRosterManager").value(0,NULL);
	if (plugin)
	{
		FRosterManager = qobject_cast<IRosterManager *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IMultiUserChatManager").value(0,NULL);
	if (plugin)
	{
		FMultiChatManager = qobject_cast<IMultiUserChatManager *>(plugin->instance());
		if (FMultiChatManager)
		{
			connect(FMultiChatManager->instance(),SIGNAL(multiUserContextMenu(IMultiUserChatWindow *, IMultiUser *, Menu *)),
				SLOT(onMultiUserContextMenu(IMultiUserChatWindow *, IMultiUser *, Menu *)));
		}
	}

	connect(this,SIGNAL(requestFailed(const QString &, const XmppError &)),
		SLOT(onSelfRequestFailed(const QString &, const XmppError &)));
	connect(this,SIGNAL(headersLoaded(const QString &, const QList<IArchiveHeader> &)),
		SLOT(onSelfHeadersLoaded(const QString &, const QList<IArchiveHeader> &)));
	connect(this,SIGNAL(collectionLoaded(const QString &, const IArchiveCollection &)),
		SLOT(onSelfCollectionLoaded(const QString &, const IArchiveCollection &)));

	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));
	connect(Shortcuts::instance(),SIGNAL(shortcutActivated(const QString &, QWidget *)),SLOT(onShortcutActivated(const QString &, QWidget *)));

	return FXmppStreamManager!=NULL && FStanzaProcessor!=NULL;
}

bool MessageArchiver::initObjects()
{
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_SHOWHISTORY, tr("Show history"), tr("Ctrl+H","Show history"));
	Shortcuts::declareShortcut(SCT_ROSTERVIEW_SHOWHISTORY,tr("Show history"),tr("Ctrl+H","Show history"),Shortcuts::WidgetShortcut);

	XmppError::registerError(NS_INTERNAL_ERROR,IERR_HISTORY_HEADERS_LOAD_ERROR,tr("Failed to load conversation headers"));
	XmppError::registerError(NS_INTERNAL_ERROR,IERR_HISTORY_CONVERSATION_SAVE_ERROR,tr("Failed to save conversation"));
	XmppError::registerError(NS_INTERNAL_ERROR,IERR_HISTORY_CONVERSATION_LOAD_ERROR,tr("Failed to load conversation"));
	XmppError::registerError(NS_INTERNAL_ERROR,IERR_HISTORY_CONVERSATION_REMOVE_ERROR,tr("Failed to remove conversation"));
	XmppError::registerError(NS_INTERNAL_ERROR,IERR_HISTORY_MODIFICATIONS_LOAD_ERROR,tr("Failed to load archive modifications"));

	if (FDiscovery)
	{
		registerDiscoFeatures();
	}
	if (FSessionNegotiation)
	{
		FSessionNegotiation->insertNegotiator(this,SNO_DEFAULT);
	}
	if (FRostersViewPlugin)
	{
		Shortcuts::insertWidgetShortcut(SCT_ROSTERVIEW_SHOWHISTORY,FRostersViewPlugin->rostersView()->instance());
	}
	if (FOptionsManager)
	{
		IOptionsDialogNode historyNode = { ONO_HISTORY, OPN_HISTORY, MNI_HISTORY, tr("History") };
		FOptionsManager->insertOptionsDialogNode(historyNode);
		FOptionsManager->insertOptionsDialogHolder(this);
	}
	return true;
}

bool MessageArchiver::initSettings()
{
	Options::setDefaultValue(OPV_HISTORY_ENGINE_ENABLED,true);
	Options::setDefaultValue(OPV_HISTORY_ENGINE_REPLICATEAPPEND,true);
	Options::setDefaultValue(OPV_HISTORY_ENGINE_REPLICATEREMOVE,true);
	Options::setDefaultValue(OPV_HISTORY_ARCHIVEVIEW_FONTPOINTSIZE,10);

	Options::setDefaultValue(OPV_ACCOUNT_HISTORYREPLICATE,false);
	Options::setDefaultValue(OPV_ACCOUNT_HISTORYDUPLICATE,false);

	return true;
}

bool MessageArchiver::stanzaReadWrite(int AHandlerId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	if (FSHIMessageBlocks.value(AStreamJid) == AHandlerId)
	{
		Jid contactJid = AStanza.to();
		IArchiveItemPrefs itemPrefs = archiveItemPrefs(AStreamJid,contactJid,AStanza.firstElement("thread").text());
		if (itemPrefs.otr==ARCHIVE_OTR_REQUIRE && !isOTRStanzaSession(AStreamJid,contactJid))
		{
			LOG_STRM_INFO(AStreamJid,QString("Starting OTR session initialization with=%1").arg(contactJid.full()));
			int initResult = FSessionNegotiation!=NULL ? FSessionNegotiation->initSession(AStreamJid,contactJid) : ISessionNegotiator::Cancel;
			if (initResult == ISessionNegotiator::Skip)
				notifyInChatWindow(AStreamJid,contactJid, tr("Off-The-Record session not ready, please wait..."));
			else if (initResult != ISessionNegotiator::Cancel)
				notifyInChatWindow(AStreamJid,contactJid, tr("Negotiating Off-The-Record session..."));
			return true;
		}
	}
	else if (FSHIMessageIn.value(AStreamJid) == AHandlerId)
	{
		Message message(AStanza);
		processMessage(AStreamJid,message,true);
	}
	else if (FSHIMessageOut.value(AStreamJid) == AHandlerId)
	{
		Message message(AStanza);
		processMessage(AStreamJid,message,false);
	}
	else if (FSHIPrefs.value(AStreamJid)==AHandlerId && AStanza.isFromServer())
	{
		QDomElement prefElem = AStanza.firstElement(PST_ARCHIVE_PREFS,FNamespaces.value(AStreamJid));
		applyArchivePrefs(AStreamJid,prefElem);

		AAccept = true;
		Stanza result = FStanzaProcessor->makeReplyResult(AStanza);
		FStanzaProcessor->sendStanzaOut(AStreamJid,result);
	}
	return false;
}

void MessageArchiver::stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza)
{
	XmppStanzaError err = !AStanza.isResult() ? XmppStanzaError(AStanza) : XmppStanzaError::null;

	if (FPrefsLoadRequests.contains(AStanza.id()))
	{
		if (AStanza.isResult())
		{
			LOG_STRM_INFO(AStreamJid,QString("Server archive prefs loaded, id=%1").arg(AStanza.id()));
			QDomElement prefElem = AStanza.firstElement(PST_ARCHIVE_PREFS,FNamespaces.value(AStreamJid));
			applyArchivePrefs(AStreamJid,prefElem);
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,QString("Failed to load server archive prefs, id=%1: %2").arg(AStanza.id(),err.condition()));
			applyArchivePrefs(AStreamJid,QDomElement());
		}
		FPrefsLoadRequests.remove(AStanza.id());
	}
	else if (FPrefsSaveRequests.contains(AStanza.id()))
	{
		FPrefsSaveRequests.remove(AStanza.id());
		if (AStanza.isResult())
		{
			LOG_STRM_INFO(AStreamJid,QString("Server archive prefs saved, id=%1").arg(AStanza.id()));
			startSuspendedStanzaSession(AStreamJid,AStanza.id());
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,QString("Failed to save server archive prefs, id=%1: %2").arg(AStanza.id(),err.condition()));
			cancelSuspendedStanzaSession(AStreamJid,AStanza.id(),err);
		}
	}
	else if (FPrefsAutoRequests.contains(AStanza.id()))
	{
		if (isReady(AStreamJid) && AStanza.isResult())
		{
			bool autoSave = FPrefsAutoRequests.value(AStanza.id());
			FArchivePrefs[AStreamJid].autoSave = autoSave;

			LOG_STRM_INFO(AStreamJid,QString("Auto save state updated, id=%1, auto=%2").arg(AStanza.id()).arg(autoSave));

			if (!isArchivePrefsEnabled(AStreamJid))
				applyArchivePrefs(AStreamJid,QDomElement());
			else if (!isSupported(AStreamJid,NS_ARCHIVE_PREF))
				loadStoragePrefs(AStreamJid);

			emit archivePrefsChanged(AStreamJid);
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,QString("Failed to update auto save state, id=%1: %2").arg(AStanza.id(),err.condition()));
		}
		FPrefsAutoRequests.remove(AStanza.id());
	}
	else if (FPrefsRemoveItemRequests.contains(AStanza.id()))
	{
		if (isReady(AStreamJid) && AStanza.isResult())
		{
			Jid itemJid = FPrefsRemoveItemRequests.value(AStanza.id());
			FArchivePrefs[AStreamJid].itemPrefs.remove(itemJid);
			LOG_STRM_INFO(AStreamJid,QString("Item prefs removed, id=%1, jid=%2").arg(AStanza.id(),itemJid.full()));
			applyArchivePrefs(AStreamJid,QDomElement());
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,QString("Failed to remove item prefs, id=%1: %2").arg(AStanza.id(),err.condition()));
		}
		FPrefsRemoveItemRequests.remove(AStanza.id());
	}
	else if (FPrefsRemoveSessionRequests.contains(AStanza.id()))
	{
		if (isReady(AStreamJid) && AStanza.isResult())
		{
			QString threadId = FPrefsRemoveSessionRequests.value(AStanza.id());
			FArchivePrefs[AStreamJid].sessionPrefs.remove(threadId);
			LOG_STRM_INFO(AStreamJid,QString("Session prefs removed, id=%1, thread=%2").arg(AStanza.id(),threadId));
			applyArchivePrefs(AStreamJid,QDomElement());
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,QString("Failed to remove session prefs, id=%1: %2").arg(AStanza.id(),err.condition()));
		}
		FPrefsRemoveSessionRequests.remove(AStanza.id());
	}

	if (FRestoreRequests.contains(AStanza.id()))
	{
		QString sessionId = FRestoreRequests.take(AStanza.id());
		if (AStanza.isResult())
		{
			LOG_STRM_DEBUG(AStreamJid,QString("Stanza session context restored, id=%1").arg(AStanza.id()));
			removeStanzaSessionContext(AStreamJid,sessionId);
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,QString("Failed to restore stanza session context, id=%1: %2").arg(AStanza.id(),err.condition()));
		}
	}

	if (AStanza.isResult())
		emit requestCompleted(AStanza.id());
	else
		emit requestFailed(AStanza.id(),err);
}

QMultiMap<int, IOptionsDialogWidget *> MessageArchiver::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *>  widgets;
	QStringList nodeTree = ANodeId.split(".",QString::SkipEmptyParts);
	if (nodeTree.count()==3 && nodeTree.at(0)==OPN_ACCOUNTS && nodeTree.at(2)=="History")
	{
		IAccount *account = FAccountManager!=NULL ? FAccountManager->findAccountById(nodeTree.at(1)) : NULL;
		if (account!=NULL && isReady(account->streamJid()))
		{
			OptionsNode options = account->optionsNode();

			widgets.insertMulti(OHO_ACCOUNTS_HISTORY_SERVERSETTINGS, FOptionsManager->newOptionsDialogHeader(tr("Archive preferences"),AParent));
			widgets.insertMulti(OWO_ACCOUNTS_HISTORY_SERVERSETTINGS, new ArchiveAccountOptionsWidget(this,account->streamJid(),AParent));

			int replCount = 0;
			int manualCount = 0;
			foreach(IArchiveEngine *engine, archiveEngines())
			{
				if (engine->isCapable(account->streamJid(),IArchiveEngine::ArchiveReplication))
					replCount++;
				else if (engine->isCapable(account->streamJid(),IArchiveEngine::ManualArchiving))
					manualCount++;
			}

			if (replCount>0 && replCount+manualCount>1)
			{
				widgets.insertMulti(OHO_ACCOUNTS_HISTORY_REPLICATION,FOptionsManager->newOptionsDialogHeader(tr("Archive synchronization"),AParent));
				widgets.insertMulti(OWO_ACCOUNTS_HISTORY_REPLICATION,FOptionsManager->newOptionsDialogWidget(options.node("history-replicate"),tr("Synchronize history between archives"),AParent));
			}

			if (isSupported(account->streamJid(), NS_ARCHIVE_AUTO))
			{
				widgets.insertMulti(OHO_ACCOUNTS_HISTORY_REPLICATION,FOptionsManager->newOptionsDialogHeader(tr("Archive synchronization"),AParent));
				widgets.insertMulti(OWO_ACCOUNTS_HISTORY_DUPLICATION,FOptionsManager->newOptionsDialogWidget(options.node("history-duplicate"),tr("Duplicate messages in local archive (not recommended)"),AParent));
			}
		}
	}
	else if (ANodeId == OPN_HISTORY)
	{
		int index = 0;
		widgets.insertMulti(OHO_HISORY_ENGINES, FOptionsManager->newOptionsDialogHeader(tr("Used history archives"),AParent));
		foreach(IArchiveEngine *engine, archiveEngines())
		{
			OptionsNode node = Options::node(OPV_HISTORY_ENGINE_ITEM,engine->engineId().toString()).node("enabled");
			widgets.insertMulti(OWO_HISORY_ENGINE,FOptionsManager->newOptionsDialogWidget(node,engine->engineName(),AParent));

			IOptionsDialogWidget *engineSettings = engine->engineSettingsWidget(AParent);
			if (engineSettings)
			{
				widgets.insertMulti(OHO_HISTORY_ENGINNAME + index,FOptionsManager->newOptionsDialogHeader(engine->engineName(),AParent));
				widgets.insertMulti(OWO_HISTORY_ENGINESETTINGS + index,engineSettings);
				index += 10;
			}
		}
	}
	return widgets;
}

int MessageArchiver::sessionInit(const IStanzaSession &ASession, IDataForm &ARequest)
{
	IArchiveItemPrefs itemPrefs = archiveItemPrefs(ASession.streamJid,ASession.contactJid);
	int result = itemPrefs.otr!=ARCHIVE_OTR_REQUIRE ? ISessionNegotiator::Skip : ISessionNegotiator::Cancel;

	if (FDataForms && isArchivePrefsEnabled(ASession.streamJid))
	{
		IDataField logging;
		logging.var = SFP_LOGGING;
		logging.type = DATAFIELD_TYPE_LISTSINGLE;
		logging.required = false;

		if (itemPrefs.otr != ARCHIVE_OTR_FORBID)
		{
			IDataOption option;
			option.value = SFV_MUSTNOT_LOGGING;
			logging.options.append(option);
		}
		if (itemPrefs.otr != ARCHIVE_OTR_REQUIRE)
		{
			IDataOption option;
			option.value = SFV_MAY_LOGGING;
			logging.options.append(option);
			logging.value = SFV_MAY_LOGGING;
		}
		else
		{
			logging.value = SFV_MUSTNOT_LOGGING;
			logging.required = true;
		}

		if (ASession.status == IStanzaSession::Init)
		{
			ARequest.fields.append(logging);
			result = ISessionNegotiator::Auto;
		}
		else if (ASession.status == IStanzaSession::Renegotiate)
		{
			int index = FDataForms->fieldIndex(SFP_LOGGING,ASession.form.fields);
			if (index<0 || ASession.form.fields.at(index).value!=logging.value)
			{
				ARequest.fields.append(logging);
				result = ISessionNegotiator::Auto;
			}
			else
				result = ISessionNegotiator::Skip;
		}
	}
	return result;
}

int MessageArchiver::sessionAccept(const IStanzaSession &ASession, const IDataForm &ARequest, IDataForm &ASubmit)
{
	IArchiveItemPrefs itemPrefs = archiveItemPrefs(ASession.streamJid,ASession.contactJid);
	int result = ISessionNegotiator::Skip;

	int rindex = FDataForms!=NULL ? FDataForms->fieldIndex(SFP_LOGGING,ARequest.fields) : -1;
	if (rindex>=0)
	{
		result = ISessionNegotiator::Auto;
		if (ARequest.type == DATAFORM_TYPE_FORM)
		{
			IDataField logging;
			logging.var = SFP_LOGGING;
			logging.type = DATAFIELD_TYPE_LISTSINGLE;
			logging.value = ARequest.fields.at(rindex).value;
			logging.required = false;

			QStringList options;
			for (int i=0; i<ARequest.fields.at(rindex).options.count(); i++)
				options.append(ARequest.fields.at(rindex).options.at(i).value);

			if (itemPrefs.otr == ARCHIVE_OTR_APPROVE)
			{
				if (logging.value == SFV_MUSTNOT_LOGGING)
				{
					result = ISessionNegotiator::Manual;
					ASubmit.pages[0].fieldrefs.append(SFP_LOGGING);
					ASubmit.pages[0].childOrder.append(DATALAYOUT_CHILD_FIELDREF);
				}
			}
			else if (itemPrefs.otr == ARCHIVE_OTR_FORBID)
			{
				if (options.contains(SFV_MAY_LOGGING))
					logging.value = SFV_MAY_LOGGING;
				else
					result = ISessionNegotiator::Cancel;
			}
			else if (itemPrefs.otr == ARCHIVE_OTR_OPPOSE)
			{
				if (options.contains(SFV_MAY_LOGGING))
					logging.value = SFV_MAY_LOGGING;
			}
			else if (itemPrefs.otr == ARCHIVE_OTR_PREFER)
			{
				if (options.contains(SFV_MUSTNOT_LOGGING))
					logging.value = SFV_MUSTNOT_LOGGING;
			}
			else if (itemPrefs.otr == ARCHIVE_OTR_REQUIRE)
			{
				if (options.contains(SFV_MUSTNOT_LOGGING))
					logging.value = SFV_MUSTNOT_LOGGING;
				else
					result = ISessionNegotiator::Cancel;
				logging.required = true;
			}
			ASubmit.fields.append(logging);
		}
		else if (ARequest.type == DATAFORM_TYPE_SUBMIT)
		{
			QString sessionValue = ARequest.fields.at(rindex).value.toString();
			if (itemPrefs.otr==ARCHIVE_OTR_FORBID && sessionValue==SFV_MUSTNOT_LOGGING)
				result = ISessionNegotiator::Cancel;
			else if (itemPrefs.otr==ARCHIVE_OTR_REQUIRE && sessionValue==SFV_MAY_LOGGING)
				result = ISessionNegotiator::Cancel;
			else if (itemPrefs.otr==ARCHIVE_OTR_APPROVE && sessionValue==SFV_MUSTNOT_LOGGING)
			{
				result = ISessionNegotiator::Manual;
				ASubmit.pages[0].fieldrefs.append(SFP_LOGGING);
				ASubmit.pages[0].childOrder.append(DATALAYOUT_CHILD_FIELDREF);
			}
		}
	}
	else if (ASession.status!=IStanzaSession::Active && itemPrefs.otr==ARCHIVE_OTR_REQUIRE)
	{
		result = ISessionNegotiator::Cancel;
	}
	return result;
}

int MessageArchiver::sessionApply(const IStanzaSession &ASession)
{
	int result = ISessionNegotiator::Skip;
	IArchiveItemPrefs itemPrefs = archiveItemPrefs(ASession.streamJid,ASession.contactJid);
	if (FDataForms && isReady(ASession.streamJid))
	{
		int index = FDataForms->fieldIndex(SFP_LOGGING,ASession.form.fields);
		QString sessionValue = index>=0 ? ASession.form.fields.at(index).value.toString() : QString();
		if (itemPrefs.otr==ARCHIVE_OTR_REQUIRE && sessionValue==SFV_MAY_LOGGING)
		{
			result = ISessionNegotiator::Cancel;
		}
		else if (itemPrefs.otr==ARCHIVE_OTR_FORBID && sessionValue==SFV_MUSTNOT_LOGGING)
		{
			result = ISessionNegotiator::Cancel;
		}
		else if (sessionValue==SFV_MUSTNOT_LOGGING && itemPrefs.save!=ARCHIVE_SAVE_FALSE)
		{
			StanzaSession &session = FSessions[ASession.streamJid][ASession.contactJid];
			if (FPrefsSaveRequests.contains(session.requestId))
			{
				result = ISessionNegotiator::Wait;
			}
			else if (!session.error.isNull())
			{
				result = ISessionNegotiator::Cancel;
			}
			else
			{
				IArchiveStreamPrefs prefs = archivePrefs(ASession.streamJid);
				if (session.sessionId.isEmpty())
				{
					session.sessionId = ASession.sessionId;
					session.saveMode = itemPrefs.save;
					session.defaultPrefs = !prefs.itemPrefs.contains(ASession.contactJid);
				}
				itemPrefs.save = ARCHIVE_SAVE_FALSE;
				prefs.itemPrefs[ASession.contactJid] = itemPrefs;
				session.requestId = setArchivePrefs(ASession.streamJid,prefs);
				result = !session.requestId.isEmpty() ? ISessionNegotiator::Wait : ISessionNegotiator::Cancel;
			}
		}
		else
		{
			result = ISessionNegotiator::Auto;
		}
	}
	else if (itemPrefs.otr == ARCHIVE_OTR_REQUIRE)
	{
		result = ISessionNegotiator::Cancel;
	}
	return result;
}

void MessageArchiver::sessionLocalize(const IStanzaSession &ASession, IDataForm &AForm)
{
	Q_UNUSED(ASession);
	if (FDataForms)
	{
		int index = FDataForms->fieldIndex(SFP_LOGGING,AForm.fields);
		if (index>=0)
		{
			AForm.fields[index].label = tr("Message logging");
			QList<IDataOption> &options = AForm.fields[index].options;
			for (int i=0;i<options.count();i++)
			{
				if (options[i].value == SFV_MAY_LOGGING)
					options[i].label = tr("Allow message logging");
				else if (options[i].value == SFV_MUSTNOT_LOGGING)
					options[i].label = tr("Disallow all message logging");
			}
		}
	}
}

bool MessageArchiver::isReady(const Jid &AStreamJid) const
{
	return FArchivePrefs.contains(AStreamJid);
}

QString MessageArchiver::archiveDirPath(const Jid &AStreamJid) const
{
	if (FArchiveDirPath.isEmpty())
	{
		QDir dir(FPluginManager->homePath());
		dir.mkdir(ARCHIVE_DIR_NAME);
		FArchiveDirPath = dir.cd(ARCHIVE_DIR_NAME) ? dir.absolutePath() : QString();
	}
	if (AStreamJid.isValid() && !FArchiveDirPath.isEmpty())
	{
		QString streamDir = Jid::encode(AStreamJid.pBare());

		QDir dir(FArchiveDirPath);
		dir.mkdir(streamDir);
		return dir.cd(streamDir) ? dir.absolutePath() : QString();
	}
	return FArchiveDirPath;
}

bool MessageArchiver::isSupported(const Jid &AStreamJid, const QString &AFeatureNS) const
{
	return isReady(AStreamJid) && FFeatures.value(AStreamJid).contains(AFeatureNS);
}

QWidget *MessageArchiver::showArchiveWindow(const QMultiMap<Jid,Jid> &AAddresses)
{
	ArchiveViewWindow *window = new ArchiveViewWindow(this,AAddresses);
	WidgetManager::showActivateRaiseWindow(window);
	return window;
}

QString MessageArchiver::prefsNamespace(const Jid &AStreamJid) const
{
	return FNamespaces.value(AStreamJid);
}

bool MessageArchiver::isArchivePrefsEnabled(const Jid &AStreamJid) const
{
	return isReady(AStreamJid) && (isSupported(AStreamJid,NS_ARCHIVE_PREF) || !isArchiveAutoSave(AStreamJid));
}

bool MessageArchiver::isArchiveReplicationEnabled(const Jid &AStreamJid) const
{
	IAccount *account = FAccountManager!=NULL ? FAccountManager->findAccountByStream(AStreamJid) : NULL;
	return account!=NULL ? account->optionsNode().value("history-replicate").toBool() : false;
}

bool MessageArchiver::isArchivingAllowed(const Jid &AStreamJid, const Jid &AItemJid, const QString &AThreadId) const
{
	if (isReady(AStreamJid) && AItemJid.isValid())
	{
		IArchiveItemPrefs itemPrefs = archiveItemPrefs(AStreamJid, AItemJid, AThreadId);
		return itemPrefs.save != ARCHIVE_SAVE_FALSE;
	}
	return false;
}

IArchiveStreamPrefs MessageArchiver::archivePrefs(const Jid &AStreamJid) const
{
	return FArchivePrefs.value(AStreamJid);
}

IArchiveItemPrefs MessageArchiver::archiveItemPrefs(const Jid &AStreamJid, const Jid &AItemJid, const QString &AThreadId) const
{
	IArchiveStreamPrefs streamPrefs = archivePrefs(AStreamJid);

	IArchiveItemPrefs sessionPrefs, domainPrefs, barePrefs, fullPrefs;
	if (!AThreadId.isEmpty() && streamPrefs.sessionPrefs.contains(AThreadId))
	{
		IArchiveSessionPrefs sprefs = streamPrefs.sessionPrefs.value(AThreadId);
		sessionPrefs = archiveItemPrefs(AStreamJid,AItemJid);
		sessionPrefs.otr = sprefs.otr;
		sessionPrefs.save = sprefs.save;
	}
	else for (QMap<Jid, IArchiveItemPrefs>::const_iterator it = streamPrefs.itemPrefs.constBegin(); it != streamPrefs.itemPrefs.constEnd(); ++it)
	{
		QString node = it.key().pNode();
		QString domain = it.key().pDomain();
		QString resource = it.key().pResource();
		if (it->exactmatch)
		{
			if (it.key() == AItemJid)
			{
				fullPrefs = it.value();
				break;
			}
		}
		else if (domain == AItemJid.pDomain())
		{
			if (node == AItemJid.pNode())
			{
				if (resource == AItemJid.pResource())
				{
					fullPrefs = it.value();
					break;
				}
				else if (resource.isEmpty())
				{
					barePrefs = it.value();
				}
			}
			else if (resource.isEmpty() && node.isEmpty())
			{
				domainPrefs = it.value();
			}
		}
	}

	IArchiveItemPrefs itemPrefs;
	if (!sessionPrefs.save.isEmpty())
		itemPrefs = sessionPrefs;
	else if (!fullPrefs.save.isEmpty())
		itemPrefs = fullPrefs;
	else if (!barePrefs.save.isEmpty())
		itemPrefs = barePrefs;
	else if (!domainPrefs.save.isEmpty())
		itemPrefs = domainPrefs;
	else
		itemPrefs = streamPrefs.defaultPrefs;

	return itemPrefs;
}

bool MessageArchiver::isArchiveAutoSave(const Jid &AStreamJid) const
{
	return isSupported(AStreamJid,NS_ARCHIVE_AUTO) && archivePrefs(AStreamJid).autoSave;
}

QString MessageArchiver::setArchiveAutoSave(const Jid &AStreamJid, bool AAuto, bool AGlobal)
{
	if (isSupported(AStreamJid,NS_ARCHIVE_AUTO))
	{
		Stanza autoSave(STANZA_KIND_IQ);
		autoSave.setType(STANZA_TYPE_SET).setUniqueId();
		QDomElement autoElem = autoSave.addElement("auto",FNamespaces.value(AStreamJid));
		autoElem.setAttribute("save",QVariant(AAuto).toString());
		autoElem.setAttribute("scope",AGlobal ? ARCHIVE_SCOPE_GLOBAL : ARCHIVE_SCOPE_STREAM);
		if (FStanzaProcessor->sendStanzaRequest(this,AStreamJid,autoSave,ARCHIVE_REQUEST_TIMEOUT))
		{
			LOG_STRM_INFO(AStreamJid,QString("Update auto save request sent, auto=%1, global=%2, id=%3").arg(AAuto).arg(AGlobal).arg(autoSave.id()));
			FPrefsAutoRequests.insert(autoSave.id(),AAuto);
			return autoSave.id();
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,"Failed to send update auto save request");
		}
	}
	return QString();
}

QString MessageArchiver::setArchivePrefs(const Jid &AStreamJid, const IArchiveStreamPrefs &APrefs)
{
	if (isArchivePrefsEnabled(AStreamJid))
	{
		bool storage = FInStoragePrefs.contains(AStreamJid);

		IArchiveStreamPrefs oldPrefs = archivePrefs(AStreamJid);
		IArchiveStreamPrefs newPrefs = oldPrefs;

		if (!APrefs.defaultPrefs.save.isEmpty() && !APrefs.defaultPrefs.otr.isEmpty())
		{
			newPrefs.defaultPrefs.otr = APrefs.defaultPrefs.otr;
			if (newPrefs.defaultPrefs.otr == ARCHIVE_OTR_REQUIRE)
				newPrefs.defaultPrefs.save = ARCHIVE_SAVE_FALSE;
			else
				newPrefs.defaultPrefs.save = APrefs.defaultPrefs.save;
			newPrefs.defaultPrefs.expire = APrefs.defaultPrefs.expire;
		}

		if (!APrefs.methodLocal.isEmpty())
			newPrefs.methodLocal = APrefs.methodLocal;
		if (!APrefs.methodAuto.isEmpty())
			newPrefs.methodAuto = APrefs.methodAuto;
		if (!APrefs.methodManual.isEmpty())
			newPrefs.methodManual = APrefs.methodManual;

		bool itemsChanged = false;
		foreach(const Jid &itemJid, APrefs.itemPrefs.keys())
		{
			IArchiveItemPrefs newItemPrefs = APrefs.itemPrefs.value(itemJid);
			if (!newItemPrefs.save.isEmpty() && !newItemPrefs.otr.isEmpty())
			{
				newPrefs.itemPrefs.insert(itemJid,newItemPrefs);
				if (newPrefs.itemPrefs.value(itemJid).otr == ARCHIVE_OTR_REQUIRE)
					newPrefs.itemPrefs[itemJid].save = ARCHIVE_SAVE_FALSE;
			}
			else
			{
				itemsChanged = true;
				newPrefs.itemPrefs.remove(itemJid);
			}
		}

		bool sessionsChanged = false;
		foreach(const QString &threadId, APrefs.sessionPrefs.keys())
		{
			IArchiveSessionPrefs newSessionPrefs = APrefs.sessionPrefs.value(threadId);
			if (!newSessionPrefs.save.isEmpty() && !newSessionPrefs.otr.isEmpty())
			{
				newPrefs.sessionPrefs[threadId] = newSessionPrefs;
			}
			else
			{
				sessionsChanged = true;
				newPrefs.sessionPrefs.remove(threadId);
			}
		}

		Stanza save(STANZA_KIND_IQ);
		save.setType(STANZA_TYPE_SET).setUniqueId();

		QDomElement prefElem = save.addElement(PST_ARCHIVE_PREFS,!storage ? FNamespaces.value(AStreamJid) : NS_ARCHIVE);

		bool defChanged = oldPrefs.defaultPrefs!=newPrefs.defaultPrefs;
		if (storage || defChanged)
		{
			QDomElement defElem = prefElem.appendChild(save.createElement("default")).toElement();
			if (newPrefs.defaultPrefs.expire > 0)
				defElem.setAttribute("expire",newPrefs.defaultPrefs.expire);
			defElem.setAttribute("save",newPrefs.defaultPrefs.save);
			defElem.setAttribute("otr",newPrefs.defaultPrefs.otr);
		}

		bool methodChanged = oldPrefs.methodAuto!=newPrefs.methodAuto  || oldPrefs.methodLocal!=newPrefs.methodLocal || oldPrefs.methodManual!=newPrefs.methodManual;
		if (!storage && methodChanged)
		{
			QDomElement methodAuto = prefElem.appendChild(save.createElement("method")).toElement();
			methodAuto.setAttribute("type","auto");
			methodAuto.setAttribute("use",newPrefs.methodAuto);

			QDomElement methodLocal = prefElem.appendChild(save.createElement("method")).toElement();
			methodLocal.setAttribute("type","local");
			methodLocal.setAttribute("use",newPrefs.methodLocal);

			QDomElement methodManual = prefElem.appendChild(save.createElement("method")).toElement();
			methodManual.setAttribute("type","manual");
			methodManual.setAttribute("use",newPrefs.methodManual);
		}

		foreach(const Jid &itemJid, newPrefs.itemPrefs.keys())
		{
			IArchiveItemPrefs newItemPrefs = newPrefs.itemPrefs.value(itemJid);
			IArchiveItemPrefs oldItemPrefs = oldPrefs.itemPrefs.value(itemJid);
			bool itemChanged = oldItemPrefs!=newItemPrefs;
			if (storage || itemChanged)
			{
				QDomElement itemElem = prefElem.appendChild(save.createElement("item")).toElement();
				if (newItemPrefs.expire > 0)
					itemElem.setAttribute("expire",newItemPrefs.expire);
				if (newItemPrefs.exactmatch)
					itemElem.setAttribute("exactmatch",QVariant(newItemPrefs.exactmatch).toString());
				itemElem.setAttribute("jid",itemJid.full());
				itemElem.setAttribute("otr",newItemPrefs.otr);
				itemElem.setAttribute("save",newItemPrefs.save);
			}
			itemsChanged |= itemChanged;
		}

		foreach(const QString &threadId, newPrefs.sessionPrefs.keys())
		{
			IArchiveSessionPrefs newSessionPrefs = newPrefs.sessionPrefs.value(threadId);
			IArchiveSessionPrefs oldSessionPrefs = oldPrefs.sessionPrefs.value(threadId);
			bool sessionChanged = oldSessionPrefs!=newSessionPrefs;
			if (storage || sessionChanged)
			{
				QDomElement sessionElem = prefElem.appendChild(save.createElement("session")).toElement();
				sessionElem.setAttribute("save",newSessionPrefs.save);
				sessionElem.setAttribute("otr",newSessionPrefs.otr);
			}
			sessionsChanged |= sessionChanged;
		}

		if (defChanged || methodChanged || itemsChanged || sessionsChanged)
		{
			QString requestId;
			if (storage)
				requestId = FPrivateStorage!=NULL ? FPrivateStorage->saveData(AStreamJid,prefElem) : QString();
			else if (FStanzaProcessor && FStanzaProcessor->sendStanzaRequest(this,AStreamJid,save,ARCHIVE_REQUEST_TIMEOUT))
				requestId = save.id();
			if (!requestId.isEmpty())
			{
				LOG_STRM_INFO(AStreamJid,QString("Update archive prefs request sent, id=%1").arg(requestId));
				FPrefsSaveRequests.insert(requestId,AStreamJid);
				return requestId;
			}
			else
			{
				LOG_STRM_WARNING(AStreamJid,QString("Failed to send update archive prefs request"));
			}
		}
	}
	return QString();
}

QString MessageArchiver::removeArchiveItemPrefs(const Jid &AStreamJid, const Jid &AItemJid)
{
	if (isArchivePrefsEnabled(AStreamJid) && archivePrefs(AStreamJid).itemPrefs.contains(AItemJid))
	{
		if (isSupported(AStreamJid,NS_ARCHIVE_PREF))
		{
			Stanza remove(STANZA_KIND_IQ);
			remove.setType(STANZA_TYPE_SET).setUniqueId();
			QDomElement itemElem = remove.addElement("itemremove",FNamespaces.value(AStreamJid)).appendChild(remove.createElement("item")).toElement();
			itemElem.setAttribute("jid",AItemJid.full());
			if (FStanzaProcessor->sendStanzaRequest(this,AStreamJid,remove,ARCHIVE_REQUEST_TIMEOUT))
			{
				LOG_STRM_INFO(AStreamJid,QString("Remove item prefs request sent, jid=%1, id=%2").arg(AItemJid.full(),remove.id()));
				FPrefsRemoveItemRequests.insert(remove.id(),AItemJid);
				return remove.id();
			}
			else
			{
				LOG_STRM_WARNING(AStreamJid,"Failed to send remove item prefs request");
			}
		}
		else
		{
			IArchiveStreamPrefs prefs;
			prefs.itemPrefs[AItemJid].otr = QString();
			prefs.itemPrefs[AItemJid].save = QString();
			return setArchivePrefs(AStreamJid,prefs);
		}
	}
	return QString();
}

QString MessageArchiver::removeArchiveSessionPrefs(const Jid &AStreamJid, const QString &AThreadId)
{
	if (isArchivePrefsEnabled(AStreamJid) && archivePrefs(AStreamJid).sessionPrefs.contains(AThreadId))
	{
		if (isSupported(AStreamJid,NS_ARCHIVE_PREF))
		{
			Stanza remove(STANZA_KIND_IQ);
			remove.setType(STANZA_TYPE_SET).setUniqueId();
			QDomElement sessionElem = remove.addElement("sessionremove",FNamespaces.value(AStreamJid)).appendChild(remove.createElement("session")).toElement();
			sessionElem.setAttribute("thread",AThreadId);
			if (FStanzaProcessor->sendStanzaRequest(this,AStreamJid,remove,ARCHIVE_REQUEST_TIMEOUT))
			{
				LOG_STRM_INFO(AStreamJid,QString("Remove session prefs request sent, thread=%1, id=%2").arg(AThreadId,remove.id()));
				FPrefsRemoveSessionRequests.insert(remove.id(),AThreadId);
				return remove.id();
			}
			else
			{
				LOG_STRM_WARNING(AStreamJid,"Failed to send remove session prefs request");
			}
		}
		else
		{
			IArchiveStreamPrefs prefs;
			prefs.sessionPrefs[AThreadId].save = QString();
			prefs.sessionPrefs[AThreadId].otr = QString();
			return setArchivePrefs(AStreamJid,prefs);
		}
	}
	return QString();
}

bool MessageArchiver::saveMessage(const Jid &AStreamJid, const Jid &AItemJid, const Message &AMessage)
{	
	if (!isArchiveAutoSave(AStreamJid) || isArchiveDuplicationEnabled(AStreamJid))
	{
		if (isArchivingAllowed(AStreamJid,AItemJid,AMessage.threadId()))
		{
			IArchiveEngine *engine = findEngineByCapability(AStreamJid,IArchiveEngine::DirectArchiving);
			if (engine)
			{
				Message message = AMessage;
				bool directionIn = AItemJid==message.from() || AStreamJid==message.to();
				if (prepareMessage(AStreamJid,message,directionIn))
					return engine->saveMessage(AStreamJid,message,directionIn);
			}
		}
	}
	return false;
}

bool MessageArchiver::saveNote(const Jid &AStreamJid, const Jid &AItemJid, const QString &ANote, const QString &AThreadId)
{
	if (!isArchiveAutoSave(AStreamJid) || isArchiveDuplicationEnabled(AStreamJid))
	{
		if (isArchivingAllowed(AStreamJid,AItemJid,AThreadId))
		{
			IArchiveEngine *engine = findEngineByCapability(AStreamJid,IArchiveEngine::DirectArchiving);
			if (engine)
			{
				Message message;
				message.setTo(AStreamJid.full()).setFrom(AItemJid.full()).setBody(ANote).setThreadId(AThreadId);
				return engine->saveNote(AStreamJid,message,true);
			}
		}
	}
	return false;
}

QString MessageArchiver::loadMessages(const Jid &AStreamJid, const IArchiveRequest &ARequest)
{
	QString id = loadHeaders(AStreamJid,ARequest);
	if (!id.isEmpty())
	{
		MessagesRequest request;
		request.request = ARequest;
		request.streamJid = AStreamJid;
		QString localId = QUuid::createUuid().toString();
		FRequestId2LocalId.insert(id,localId);
		FMesssagesRequests.insert(localId,request);
		LOG_STRM_DEBUG(AStreamJid,QString("Load messages request sent, id=%1").arg(localId));
		Logger::startTiming(STMP_HISTORY_MESSAGES_LOAD,localId);
		return localId;
	}
	else
	{
		LOG_STRM_WARNING(AStreamJid,"Failed to send load messages request: Headers not requested");
	}
	return QString();
}

QString MessageArchiver::loadHeaders(const Jid &AStreamJid, const IArchiveRequest &ARequest)
{
	HeadersRequest request;
	QString localId = QUuid::createUuid().toString();
	foreach(IArchiveEngine *engine, engineOrderByCapability(AStreamJid,IArchiveEngine::ArchiveManagement))
	{
		if (ARequest.text.isEmpty() || engine->isCapable(AStreamJid,IArchiveEngine::FullTextSearch))
		{
			QString id = engine->loadHeaders(AStreamJid,ARequest);
			if (!id.isEmpty())
			{
				request.engines.append(engine);
				FRequestId2LocalId.insert(id,localId);
			}
			else
			{
				LOG_STRM_WARNING(AStreamJid,QString("Failed to send load headers request to engine=%1").arg(engine->engineName()));
			}
		}
	}

	if (!request.engines.isEmpty())
	{
		request.request = ARequest;
		FHeadersRequests.insert(localId,request);
		LOG_STRM_DEBUG(AStreamJid,QString("Load headers request sent to %1 engines, id=%2").arg(request.engines.count()).arg(localId));
		Logger::startTiming(STMP_HISTORY_HEADERS_LOAD,localId);
		return localId;
	}
	else
	{
		LOG_STRM_WARNING(AStreamJid,"Failed to send load headers request to any engine");
	}

	return QString();
}

QString MessageArchiver::loadCollection(const Jid &AStreamJid, const IArchiveHeader &AHeader)
{
	IArchiveEngine *engine = findArchiveEngine(AHeader.engineId);
	if (engine)
	{
		QString id = engine->loadCollection(AStreamJid,AHeader);
		if (!id.isEmpty())
		{
			CollectionRequest request;
			QString localId = QUuid::createUuid().toString();
			FRequestId2LocalId.insert(id,localId);
			FCollectionRequests.insert(localId,request);
			LOG_STRM_DEBUG(AStreamJid,QString("Load collection request sent to engine=%1, id=%2").arg(engine->engineName(),localId));
			return localId;
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,QString("Failed to send load collection request to engine=%1").arg(engine->engineName()));
		}
	}
	else
	{
		REPORT_ERROR("Failed to send load collection request: Engine not found");
	}
	return QString();
}

QString MessageArchiver::removeCollections(const Jid &AStreamJid, const IArchiveRequest &ARequest)
{
	RemoveRequest request;
	QString localId = QUuid::createUuid().toString();
	foreach(IArchiveEngine *engine, engineOrderByCapability(AStreamJid,IArchiveEngine::ArchiveManagement))
	{
		QString id = engine->removeCollections(AStreamJid,ARequest);
		if (!id.isEmpty())
		{
			FRequestId2LocalId.insert(id,localId);
			request.engines.append(engine);
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,QString("Failed to send remove collections request to engine=%1").arg(engine->engineName()));
		}
	}

	if (!request.engines.isEmpty())
	{
		request.request = ARequest;
		FRemoveRequests.insert(localId,request);
		LOG_STRM_DEBUG(AStreamJid,QString("Remove collections request sent to %1 engines, id=%2").arg(request.engines.count()).arg(localId));
		return localId;
	}
	else
	{
		LOG_STRM_WARNING(AStreamJid,"Failed to send remove collections request to any engine");
	}

	return QString();
}

void MessageArchiver::elementToCollection(const Jid &AStreamJid, const QDomElement &AChatElem, IArchiveCollection &ACollection) const
{
	ACollection.header.with = AChatElem.attribute("with");
	ACollection.header.start = DateTime(AChatElem.attribute("start")).toLocal();
	ACollection.header.subject = AChatElem.attribute("subject");
	ACollection.header.threadId = AChatElem.attribute("thread");
	ACollection.header.version = AChatElem.attribute("version").toUInt();

	QDomElement nodeElem = AChatElem.firstChildElement();

	bool isSecsFromStart;
	if (!AChatElem.hasAttribute("secsFromLast"))
	{
		int secsLast = 0;
		isSecsFromStart = true;
		while (!nodeElem.isNull() && isSecsFromStart)
		{
			if (nodeElem.hasAttribute("secs"))
			{
				int secs = nodeElem.attribute("secs").toInt();
				if (secs < secsLast)
					isSecsFromStart = false;
				secsLast = secs;
			}
			nodeElem = nodeElem.nextSiblingElement();
		}
	}
	else
	{
		isSecsFromStart = AChatElem.attribute("secsFromLast")!="true";
	}

	int secsLast = 0;
	QDateTime lastMessageDT;
	nodeElem = AChatElem.firstChildElement();
	while (!nodeElem.isNull())
	{
		if (nodeElem.tagName()=="to" || nodeElem.tagName()=="from")
		{
			Message message;

			Jid with = ACollection.header.with;
			QString nick = nodeElem.attribute("name");
			Jid contactJid(with.node(),with.domain(),nick.isEmpty() ? with.resource() : nick);

			if (nodeElem.tagName()=="to")
			{
				message.setTo(contactJid.full());
				message.setFrom(AStreamJid.full());
				message.setData(MDR_MESSAGE_DIRECTION,IMessageProcessor::DirectionOut);
			}
			else
			{
				message.setTo(AStreamJid.full());
				message.setFrom(contactJid.full());
				message.setData(MDR_MESSAGE_DIRECTION,IMessageProcessor::DirectionIn);
			}

			message.setType(nick.isEmpty() ? Message::Chat : Message::GroupChat);

			QString utc = nodeElem.attribute("utc");
			if (utc.isEmpty())
			{
				int secs = nodeElem.attribute("secs").toInt();
				QDateTime messageDT = ACollection.header.start.addSecs(isSecsFromStart ? secs : secsLast+secs);

				if (lastMessageDT.isValid() && lastMessageDT>=messageDT)
					messageDT = lastMessageDT.addMSecs(1);

				message.setDateTime(messageDT);
				lastMessageDT = messageDT;
			}
			else
			{
				QDateTime messageDT = DateTime(utc).toLocal();
				message.setDateTime(messageDT.isValid() ? messageDT : ACollection.header.start.addSecs(secsLast));
			}
			secsLast = ACollection.header.start.secsTo(message.dateTime());

			QDomElement childElem = nodeElem.firstChildElement();
			while (!childElem.isNull())
			{
				message.stanza().element().appendChild(childElem.cloneNode(true));
				childElem = childElem.nextSiblingElement();
			}
			message.setThreadId(ACollection.header.threadId);

			ACollection.body.messages.append(message);
		}
		else if (nodeElem.tagName() == "note")
		{
			QString utc = nodeElem.attribute("utc");
			ACollection.body.notes.insertMulti(DateTime(utc).toLocal(),nodeElem.text());
		}
		else if(nodeElem.tagName() == "next")
		{
			ACollection.next.with = nodeElem.attribute("with");
			ACollection.next.start = DateTime(nodeElem.attribute("start")).toLocal();
		}
		else if(nodeElem.tagName() == "previous")
		{
			ACollection.previous.with = nodeElem.attribute("with");
			ACollection.previous.start = DateTime(nodeElem.attribute("start")).toLocal();
		}
		else if (FDataForms && nodeElem.tagName()=="x" && nodeElem.namespaceURI()==NS_JABBER_DATA)
		{
			ACollection.attributes = FDataForms->dataForm(nodeElem);
		}
		nodeElem = nodeElem.nextSiblingElement();
	}
}

void MessageArchiver::collectionToElement(const IArchiveCollection &ACollection, QDomElement &AChatElem, const QString &ASaveMode) const
{
	QDomDocument ownerDoc = AChatElem.ownerDocument();
	AChatElem.setAttribute("with",ACollection.header.with.full());
	AChatElem.setAttribute("start",DateTime(ACollection.header.start).toX85UTC());
	AChatElem.setAttribute("version",ACollection.header.version);
	if (!ACollection.header.subject.isEmpty())
		AChatElem.setAttribute("subject",ACollection.header.subject);
	if (!ACollection.header.threadId.isEmpty())
		AChatElem.setAttribute("thread",ACollection.header.threadId);
	AChatElem.setAttribute("secsFromLast","false");

	bool groupChat = false;
	QList<Message>::const_iterator messageIt = ACollection.body.messages.constBegin();
	QMultiMap<QDateTime,QString>::const_iterator noteIt = ACollection.body.notes.constBegin();
	while(messageIt!=ACollection.body.messages.constEnd() || noteIt!=ACollection.body.notes.constEnd())
	{
		bool writeNote = false;
		bool writeMessage = false;
		if (noteIt == ACollection.body.notes.constEnd())
			writeMessage = true;
		else if (messageIt == ACollection.body.messages.constEnd())
			writeNote = true;
		else if (messageIt->dateTime() <= noteIt.key())
			writeMessage = true;
		else
			writeNote = true;

		if (writeMessage)
		{
			groupChat |= messageIt->type()==Message::GroupChat;
			if (!groupChat || messageIt->fromJid().hasResource())
			{
				bool directionIn = ACollection.header.with.pBare() == messageIt->fromJid().pBare();
				QDomElement messageElem = AChatElem.appendChild(ownerDoc.createElement(directionIn ? "from" : "to")).toElement();

				int secs = ACollection.header.start.secsTo(messageIt->dateTime());
				if (secs >= 0)
					messageElem.setAttribute("secs",secs);
				else
					messageElem.setAttribute("utc",DateTime(messageIt->dateTime()).toX85UTC());

				if (groupChat)
					messageElem.setAttribute("name",messageIt->fromJid().resource());

				if (ASaveMode==ARCHIVE_SAVE_MESSAGE || ASaveMode==ARCHIVE_SAVE_STREAM)
				{
					QDomElement childElem = messageIt->stanza().element().firstChildElement();
					while (!childElem.isNull())
					{
						if (childElem.tagName() != "thread")
							messageElem.appendChild(childElem.cloneNode(true));
						childElem = childElem.nextSiblingElement();
					}
				}
				else if (ASaveMode == ARCHIVE_SAVE_BODY)
				{
					messageElem.appendChild(ownerDoc.createElement("body")).appendChild(ownerDoc.createTextNode(messageIt->body()));
				}
			}
			++messageIt;
		}

		if (writeNote)
		{
			QDomElement noteElem = AChatElem.appendChild(ownerDoc.createElement("note")).toElement();
			noteElem.setAttribute("utc",DateTime(noteIt.key()).toX85UTC());
			noteElem.appendChild(ownerDoc.createTextNode(noteIt.value()));
			++noteIt;
		}
	}

	if (ACollection.previous.with.isValid() && ACollection.previous.start.isValid())
	{
		QDomElement prevElem = AChatElem.appendChild(ownerDoc.createElement("previous")).toElement();
		prevElem.setAttribute("with",ACollection.previous.with.full());
		prevElem.setAttribute("start",DateTime(ACollection.previous.start).toX85UTC());
	}

	if (ACollection.next.with.isValid() && ACollection.next.start.isValid())
	{
		QDomElement nextElem = AChatElem.appendChild(ownerDoc.createElement("next")).toElement();
		nextElem.setAttribute("with",ACollection.next.with.full());
		nextElem.setAttribute("start",DateTime(ACollection.next.start).toX85UTC());
	}

	if (FDataForms && FDataForms->isFormValid(ACollection.attributes))
	{
		FDataForms->xmlForm(ACollection.attributes,AChatElem);
	}
}

void MessageArchiver::insertArchiveHandler(int AOrder, IArchiveHandler *AHandler)
{
	if (AHandler)
		FArchiveHandlers.insertMulti(AOrder,AHandler);
}

void MessageArchiver::removeArchiveHandler(int AOrder, IArchiveHandler *AHandler)
{
	FArchiveHandlers.remove(AOrder,AHandler);
}

quint32 MessageArchiver::totalCapabilities(const Jid &AStreamJid) const
{
	quint32 caps = 0;
	foreach(IArchiveEngine *engine, FArchiveEngines)
	{
		if (isArchiveEngineEnabled(engine->engineId()))
			caps |= engine->capabilities(AStreamJid);
	}
	return caps;
}

QList<IArchiveEngine *> MessageArchiver::archiveEngines() const
{
	return FArchiveEngines.values();
}

IArchiveEngine *MessageArchiver::findArchiveEngine(const QUuid &AId) const
{
	return FArchiveEngines.value(AId);
}

bool MessageArchiver::isArchiveEngineEnabled(const QUuid &AId) const
{
	return Options::node(OPV_HISTORY_ENGINE_ITEM,AId.toString()).value("enabled").toBool();
}

void MessageArchiver::setArchiveEngineEnabled(const QUuid &AId, bool AEnabled)
{
	if (isArchiveEngineEnabled(AId) != AEnabled)
		Options::node(OPV_HISTORY_ENGINE_ITEM,AId.toString()).setValue(AEnabled,"enabled");
}

void MessageArchiver::registerArchiveEngine(IArchiveEngine *AEngine)
{
	if (AEngine!=NULL && !FArchiveEngines.contains(AEngine->engineId()))
	{
		LOG_DEBUG(QString("Archive engine registered, id=%1, name=%2").arg(AEngine->engineId().toString(),AEngine->engineName()));
		connect(AEngine->instance(),SIGNAL(capabilitiesChanged(const Jid &)),
			SLOT(onEngineCapabilitiesChanged(const Jid &)));
		connect(AEngine->instance(),SIGNAL(requestFailed(const QString &, const XmppError &)),
			SLOT(onEngineRequestFailed(const QString &, const XmppError &)));
		connect(AEngine->instance(),SIGNAL(collectionsRemoved(const QString &, const IArchiveRequest &)),
			SLOT(onEngineCollectionsRemoved(const QString &, const IArchiveRequest &)));
		connect(AEngine->instance(),SIGNAL(headersLoaded(const QString &, const QList<IArchiveHeader> &)),
			SLOT(onEngineHeadersLoaded(const QString &, const QList<IArchiveHeader> &)));
		connect(AEngine->instance(),SIGNAL(collectionLoaded(const QString &, const IArchiveCollection &)),
			SLOT(onEngineCollectionLoaded(const QString &, const IArchiveCollection &)));
		FArchiveEngines.insert(AEngine->engineId(),AEngine);
		emit archiveEngineRegistered(AEngine);
		emit totalCapabilitiesChanged(Jid::null);
	}
}

QString MessageArchiver::archiveFilePath(const Jid &AStreamJid, const QString &AFileName) const
{
	if (AStreamJid.isValid() && !AFileName.isEmpty())
	{
		QString dirPath = archiveDirPath(AStreamJid);
		if (!dirPath.isEmpty())
			return dirPath+"/"+AFileName;
	}
	return QString();
}

QString MessageArchiver::loadServerPrefs(const Jid &AStreamJid)
{
	if (FStanzaProcessor)
	{
		Stanza load(STANZA_KIND_IQ);
		load.setType(STANZA_TYPE_GET).setUniqueId();
		load.addElement(PST_ARCHIVE_PREFS,FNamespaces.value(AStreamJid));
		if (FStanzaProcessor->sendStanzaRequest(this,AStreamJid,load,ARCHIVE_REQUEST_TIMEOUT))
		{
			LOG_STRM_INFO(AStreamJid,QString("Load server archive prefs request sent, id=%1").arg(load.id()));
			FPrefsLoadRequests.insert(load.id(),AStreamJid);
			return load.id();
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,"Failed to send load server archive prefs request");
			applyArchivePrefs(AStreamJid,QDomElement());
		}
	}
	else
	{
			LOG_STRM_WARNING(AStreamJid,"Failed to send load server archive prefs request: StanzaProcessor is NULL");
			applyArchivePrefs(AStreamJid,QDomElement());
	}
	return QString();
}

QString MessageArchiver::loadStoragePrefs(const Jid &AStreamJid)
{
	QString requestId = FPrivateStorage!=NULL ? FPrivateStorage->loadData(AStreamJid,PST_ARCHIVE_PREFS,PSN_ARCHIVE_PREFS) : QString();
	if (!requestId.isEmpty())
	{
		LOG_STRM_INFO(AStreamJid,QString("Load storage archive prefs request sent, id=%1").arg(requestId));
		FPrefsLoadRequests.insert(requestId,AStreamJid);
	}
	else
	{
		LOG_STRM_WARNING(AStreamJid,"Failed to send load storage archive prefs request");
		applyArchivePrefs(AStreamJid,QDomElement());
	}
	return requestId;
}

void MessageArchiver::applyArchivePrefs(const Jid &AStreamJid, const QDomElement &AElem)
{
	if (isReady(AStreamJid) || AElem.hasChildNodes() || FInStoragePrefs.contains(AStreamJid))
	{
		//Hack for Jabberd 1.4.3
		if (!FInStoragePrefs.contains(AStreamJid) && AElem.hasAttribute("j_private_flag"))
			FInStoragePrefs.append(AStreamJid);

		bool initPrefs = !isReady(AStreamJid);
		bool prefsInStgorage = FInStoragePrefs.contains(AStreamJid);
		LOG_STRM_INFO(AStreamJid,QString("Applying new archive prefs, init=%1, in_storage=%2").arg(initPrefs).arg(prefsInStgorage));

		IArchiveStreamPrefs &prefs = FArchivePrefs[AStreamJid];

		QDomElement autoElem = isSupported(AStreamJid,NS_ARCHIVE_PREF) ? AElem.firstChildElement("auto") : QDomElement();
		if (!autoElem.isNull())
		{
			prefs.autoSave = QVariant(autoElem.attribute("save","false")).toBool();
			prefs.autoScope = autoElem.attribute("scope",ARCHIVE_SCOPE_STREAM);
		}
		else if (initPrefs)
		{
			prefs.autoSave = isSupported(AStreamJid,NS_ARCHIVE_AUTO);
			prefs.autoScope = ARCHIVE_SCOPE_GLOBAL;
		}

		bool prefsDisabled = !isArchivePrefsEnabled(AStreamJid);

		QDomElement defElem = AElem.firstChildElement("default");
		if (!defElem.isNull())
		{
			prefs.defaultPrefs.save = defElem.attribute("save",prefs.defaultPrefs.save);
			prefs.defaultPrefs.otr = defElem.attribute("otr",prefs.defaultPrefs.otr);
			prefs.defaultPrefs.expire = defElem.attribute("expire","0").toUInt();
		}
		else if (initPrefs || prefsDisabled)
		{
			prefs.defaultPrefs.save = ARCHIVE_SAVE_MESSAGE;
			prefs.defaultPrefs.otr = ARCHIVE_OTR_CONCEDE;
			prefs.defaultPrefs.expire = 0;
		}

		QDomElement methodElem = AElem.firstChildElement("method");
		if (methodElem.isNull() && (initPrefs || prefsDisabled))
		{
			prefs.methodAuto = ARCHIVE_METHOD_PREFER;
			prefs.methodLocal = ARCHIVE_METHOD_PREFER;
			prefs.methodManual = ARCHIVE_METHOD_PREFER;
		}
		else while (!methodElem.isNull())
		{
			if (methodElem.attribute("type") == "auto")
				prefs.methodAuto = methodElem.attribute("use",prefs.methodAuto);
			else if (methodElem.attribute("type") == "local")
				prefs.methodLocal = methodElem.attribute("use",prefs.methodLocal);
			else if (methodElem.attribute("type") == "manual")
				prefs.methodManual = methodElem.attribute("use",prefs.methodManual);
			methodElem = methodElem.nextSiblingElement("method");
		}

		QSet<Jid> oldItemJids = prefs.itemPrefs.keys().toSet();
		QDomElement itemElem = AElem.firstChildElement("item");
		while (!itemElem.isNull())
		{
			Jid itemJid = itemElem.attribute("jid");
			oldItemJids -= itemJid;

			IArchiveItemPrefs itemPrefs;
			itemPrefs.save = itemElem.attribute("save",prefs.defaultPrefs.save);
			itemPrefs.otr = itemElem.attribute("otr",prefs.defaultPrefs.otr);
			itemPrefs.expire = itemElem.attribute("expire","0").toUInt();
			itemPrefs.exactmatch = QVariant(itemElem.attribute("exactmatch","false")).toBool();

			if (!itemPrefs.save.isEmpty() && !itemPrefs.otr.isEmpty())
				prefs.itemPrefs.insert(itemJid,itemPrefs);
			else
				prefs.itemPrefs.remove(itemJid);

			itemElem = itemElem.nextSiblingElement("item");
		}

		QSet<QString> oldSessionIds = prefs.sessionPrefs.keys().toSet();
		QDomElement sessionElem = AElem.firstChildElement("session");
		while (!sessionElem.isNull())
		{
			QString threadId = sessionElem.attribute("thread");
			oldSessionIds -= threadId;

			IArchiveSessionPrefs sessionPrefs;
			sessionPrefs.save = sessionElem.attribute("save");
			sessionPrefs.otr = sessionElem.attribute("otr",prefs.defaultPrefs.otr);
			sessionPrefs.timeout = sessionElem.attribute("timeout","0").toInt();

			if (!sessionPrefs.save.isEmpty())
				prefs.sessionPrefs.insert(threadId,sessionPrefs);
			else
				prefs.sessionPrefs.remove(threadId);

			sessionElem = sessionElem.nextSiblingElement("session");
		}

		if (prefsInStgorage)
		{
			foreach(const Jid &itemJid, oldItemJids)
				prefs.itemPrefs.remove(itemJid);
			foreach(const QString &threadId, oldSessionIds)
				prefs.sessionPrefs.remove(threadId);
		}

		if (initPrefs)
		{
			restoreStanzaSessionContext(AStreamJid);

			if (prefsDisabled)
				setArchiveAutoSave(AStreamJid,prefs.autoSave);

			emit archivePrefsOpened(AStreamJid);
			processPendingMessages(AStreamJid);
		}
		else
		{
			renegotiateStanzaSessions(AStreamJid);
		}

		emit archivePrefsChanged(AStreamJid);
	}
	else
	{
		FInStoragePrefs.append(AStreamJid);
		loadStoragePrefs(AStreamJid);
	}
}

void MessageArchiver::loadPendingMessages(const Jid &AStreamJid)
{
	QFile file(archiveFilePath(AStreamJid,PENDING_FILE_NAME));
	if (file.open(QFile::ReadOnly))
	{
		QString xmlError;
		QDomDocument doc;
		if (doc.setContent(&file,true,&xmlError))
		{
			if (AStreamJid.pBare() == doc.documentElement().attribute("jid"))
			{
				QList< QPair<Message,bool> > &messages = FPendingMessages[AStreamJid];
				QDomElement messageElem = doc.documentElement().firstChildElement("message");
				while (!messageElem.isNull())
				{
					bool directionIn = QVariant(messageElem.attribute("x-archive-direction-in")).toBool();
					messageElem.removeAttribute("x-archive-direction-in");

					Stanza stanza(messageElem);
					Message message(stanza);

					if (directionIn)
						message.setTo(AStreamJid.full());
					else
						message.setFrom(AStreamJid.full());
					messages.append(qMakePair<Message,bool>(message,directionIn));

					messageElem = messageElem.nextSiblingElement("message");
				}
				LOG_STRM_INFO(AStreamJid,QString("Pending messages loaded, count=%1").arg(messages.count()));
			}
			else
			{
				REPORT_ERROR("Failed to load pending messages from file content: Invalid stream JID");
				file.remove();
			}
		}
		else
		{
			REPORT_ERROR(QString("Failed to load pending messages from file content: %1").arg(xmlError));
			file.remove();
		}
	}
	else if (file.exists())
	{
		REPORT_ERROR(QString("Failed to load pending messages from file: %1").arg(file.errorString()));
	}
}

void MessageArchiver::savePendingMessages(const Jid &AStreamJid)
{
	QList< QPair<Message,bool> > messages = FPendingMessages.take(AStreamJid);
	if (!messages.isEmpty())
	{
		QDomDocument doc;
		doc.appendChild(doc.createElement("pending-messages"));
		doc.documentElement().setAttribute("version","1.0");
		doc.documentElement().setAttribute("jid",AStreamJid.pBare());

		for (int i=0; i<messages.count(); i++)
		{
			QPair<Message,bool> &message = messages[i];
			message.first.setDelayed(message.first.dateTime(),message.first.from());
			if (prepareMessage(AStreamJid,message.first,message.second))
			{
				QDomElement messageElem = doc.documentElement().appendChild(doc.importNode(message.first.stanza().element(),true)).toElement();
				messageElem.setAttribute("x-archive-direction-in",QVariant(message.second).toString());
			}
		}

		QFile file(archiveFilePath(AStreamJid,PENDING_FILE_NAME));
		if (file.open(QFile::WriteOnly|QFile::Truncate))
		{
			LOG_STRM_INFO(AStreamJid,QString("Pending messages saved, count=%1").arg(messages.count()));
			file.write(doc.toByteArray());
			file.close();
		}
		else
		{
			REPORT_ERROR(QString("Failed to save pending messages to file: %1").arg(file.errorString()));
		}
	}
}

void MessageArchiver::processPendingMessages(const Jid &AStreamJid)
{
	QList< QPair<Message,bool> > messages = FPendingMessages.take(AStreamJid);
	if (!messages.isEmpty())
	{
		LOG_STRM_INFO(AStreamJid,QString("Processing pending messages, count=%1").arg(messages.count()));
		for (int i = 0; i<messages.count(); i++)
		{
			QPair<Message, bool> message = messages.at(i);
			processMessage(AStreamJid, message.first, message.second);
		}
	}
	QFile::remove(archiveFilePath(AStreamJid,PENDING_FILE_NAME));
}

bool MessageArchiver::prepareMessage(const Jid &AStreamJid, Message &AMessage, bool ADirectionIn)
{
//	qDebug() << "MessageArchiver::prepareMessage(" << AStreamJid.full()
//			 << "," << AMessage.stanza().toString()
//			 << "," << ADirectionIn << ")";
//	if (AMessage.body().isEmpty())
//		return false;
	if (AMessage.type()==Message::Error)
		return false;
	if (AMessage.type()==Message::GroupChat && !ADirectionIn)
		return false;
	if (AMessage.type()==Message::GroupChat && AMessage.isDelayed())
		return false;

	if (ADirectionIn && AMessage.from().isEmpty())
		AMessage.setFrom(AStreamJid.domain());
	else if (!ADirectionIn && AMessage.to().isEmpty())
		AMessage.setTo(AStreamJid.domain());

//	qDebug() << "HERE!";

	for (QMultiMap<int,IArchiveHandler *>::const_iterator it = FArchiveHandlers.constBegin(); it!=FArchiveHandlers.constEnd(); ++it)
	{
		IArchiveHandler *handler = it.value();
		if (handler->archiveMessageEdit(it.key(),AStreamJid,AMessage,ADirectionIn))
			return false;
	}

//	qDebug() << "HERE!!!";

	if (AMessage.type()==Message::Chat || AMessage.type()==Message::GroupChat)
	{
		if (!AMessage.threadId().isEmpty())
			AMessage.setThreadId(QString());
	}

	return true;
}

bool MessageArchiver::processMessage(const Jid &AStreamJid, const Message &AMessage, bool ADirectionIn)
{
//	qDebug() << "MessageArchiver::processMessage(" << AStreamJid.full()
//			 << "," << AMessage.stanza().toString()
//			 << "," << ADirectionIn << ")";

	Jid itemJid = ADirectionIn ? (!AMessage.from().isEmpty() ? AMessage.from() : AStreamJid.domain()) : AMessage.to();
	if (!isReady(AStreamJid))
	{
		FPendingMessages[AStreamJid].append(qMakePair<Message,bool>(AMessage,ADirectionIn));
		return true;
	}
	return saveMessage(AStreamJid,itemJid,AMessage);
}

IArchiveEngine *MessageArchiver::findEngineByCapability(const Jid &AStreamJid, quint32 ACapability) const
{
	QMultiMap<int, IArchiveEngine *> order = engineOrderByCapability(AStreamJid,ACapability);
	return !order.isEmpty() ? order.constBegin().value() : NULL;
}

QMultiMap<int, IArchiveEngine *> MessageArchiver::engineOrderByCapability(const Jid &AStreamJid, quint32 ACapability) const
{
	QMultiMap<int, IArchiveEngine *> order;
	for (QMap<QUuid,IArchiveEngine *>::const_iterator it=FArchiveEngines.constBegin(); it!=FArchiveEngines.constEnd(); ++it)
	{
		if (isArchiveEngineEnabled(it.key()))
		{
			int engineOrder = (*it)->capabilityOrder(ACapability,AStreamJid);
			if (engineOrder > 0)
				order.insertMulti(engineOrder,it.value());
		}
	}
	return order;
}

void MessageArchiver::processMessagesRequest(const QString &ALocalId, MessagesRequest &ARequest)
{
	if (!ARequest.lastError.isNull())
	{
		Logger::finishTiming(STMP_HISTORY_MESSAGES_LOAD,ALocalId);
		LOG_WARNING(QString("Failed to load messages, id=%1: %2").arg(ALocalId,ARequest.lastError.condition()));

		emit requestFailed(ALocalId,ARequest.lastError);
		FMesssagesRequests.remove(ALocalId);
	}
	else if (ARequest.headers.isEmpty() || (quint32)ARequest.body.messages.count()>ARequest.request.maxItems)
	{
		if (ARequest.request.order == Qt::AscendingOrder)
			std::sort(ARequest.body.messages.begin(),ARequest.body.messages.end(),qLess<Message>());
		else
			std::sort(ARequest.body.messages.begin(),ARequest.body.messages.end(),qGreater<Message>());

		REPORT_TIMING(STMP_HISTORY_MESSAGES_LOAD,Logger::finishTiming(STMP_HISTORY_MESSAGES_LOAD,ALocalId));
		LOG_DEBUG(QString("Messages successfully loaded, id=%1").arg(ALocalId));

		emit messagesLoaded(ALocalId,ARequest.body);
		FMesssagesRequests.remove(ALocalId);
	}
	else
	{
		QString id = loadCollection(ARequest.streamJid,ARequest.headers.takeFirst());
		if (!id.isEmpty())
		{
			FRequestId2LocalId.insert(id,ALocalId);
		}
		else
		{
			ARequest.lastError = XmppError(IERR_HISTORY_CONVERSATION_LOAD_ERROR);
			processMessagesRequest(ALocalId,ARequest);
		}
	}
}

void MessageArchiver::processHeadersRequest(const QString &ALocalId, HeadersRequest &ARequest)
{
	if (ARequest.engines.count() == ARequest.headers.count())
	{
		if (!ARequest.engines.isEmpty() || ARequest.lastError.isNull())
		{
			QList<IArchiveHeader> headers;
			foreach(IArchiveEngine *engine, ARequest.engines)
			{
				foreach(const IArchiveHeader &header, ARequest.headers.value(engine))
				{
					if (!headers.contains(header))
						headers.append(header);
				}
			}

			if (ARequest.request.order == Qt::AscendingOrder)
				std::sort(headers.begin(),headers.end(),qLess<IArchiveHeader>());
			else
				std::sort(headers.begin(),headers.end(),qGreater<IArchiveHeader>());

			if ((quint32)headers.count() > ARequest.request.maxItems)
				headers = headers.mid(0,ARequest.request.maxItems);

			REPORT_TIMING(STMP_HISTORY_HEADERS_LOAD,Logger::finishTiming(STMP_HISTORY_HEADERS_LOAD,ALocalId));
			LOG_DEBUG(QString("Headers successfully loaded, id=%1").arg(ALocalId));

			emit headersLoaded(ALocalId,headers);
		}
		else
		{
			Logger::finishTiming(STMP_HISTORY_HEADERS_LOAD,ALocalId);
			LOG_WARNING(QString("Failed to load headers, id=%1: %2").arg(ALocalId,ARequest.lastError.condition()));

			emit requestFailed(ALocalId,ARequest.lastError);
		}
		FHeadersRequests.remove(ALocalId);
	}
}

void MessageArchiver::processCollectionRequest(const QString &ALocalId, CollectionRequest &ARequest)
{
	if (ARequest.lastError.isNull())
	{
		LOG_DEBUG(QString("Collection successfully loaded, id=%1").arg(ALocalId));
		emit collectionLoaded(ALocalId,ARequest.collection);
	}
	else
	{
		LOG_WARNING(QString("Failed to load collection, id=%1").arg(ALocalId));
		emit requestFailed(ALocalId,ARequest.lastError);
	}
	FCollectionRequests.remove(ALocalId);
}

void MessageArchiver::processRemoveRequest(const QString &ALocalId, RemoveRequest &ARequest)
{
	if (ARequest.engines.isEmpty())
	{
		if (ARequest.lastError.isNull())
		{
			LOG_DEBUG(QString("Collections successfully removed, id=%1").arg(ALocalId));
			emit collectionsRemoved(ALocalId,ARequest.request);
		}
		else
		{
			LOG_WARNING(QString("Failed to remove collections, id=%1: %2").arg(ALocalId,ARequest.lastError.condition()));
			emit requestFailed(ALocalId,ARequest.lastError);
		}
		FRemoveRequests.remove(ALocalId);
	}
}

bool MessageArchiver::hasStanzaSession(const Jid &AStreamJid, const Jid &AContactJid) const
{
	return FSessionNegotiation!=NULL ? FSessionNegotiation->findSession(AStreamJid,AContactJid).status==IStanzaSession::Active : false;
}

bool MessageArchiver::isOTRStanzaSession(const IStanzaSession &ASession) const
{
	if (FDataForms)
	{
		int index = FDataForms->fieldIndex(SFP_LOGGING,ASession.form.fields);
		if (index>=0)
			return ASession.form.fields.at(index).value.toString() == SFV_MUSTNOT_LOGGING;
	}
	return false;
}

bool MessageArchiver::isOTRStanzaSession(const Jid &AStreamJid, const Jid &AContactJid) const
{
	if (FSessionNegotiation && FDataForms)
	{
		IStanzaSession session = FSessionNegotiation->findSession(AStreamJid,AContactJid);
		if (session.status == IStanzaSession::Active)
			return isOTRStanzaSession(session);
	}
	return false;
}

QDomDocument MessageArchiver::loadStanzaSessionsContexts(const Jid &AStreamJid) const
{
	QDomDocument sessions;

	QFile file(archiveFilePath(AStreamJid,SESSIONS_FILE_NAME));
	if (file.open(QFile::ReadOnly))
	{
		QString xmlError;
		if (!sessions.setContent(&file,true,&xmlError))
		{
			REPORT_ERROR(QString("Failed to load stanza sessions contexts from file content: %1").arg(xmlError));
			sessions.clear();
			file.remove();
		}
	}
	else if (file.exists())
	{
		REPORT_ERROR(QString("Failed to load stanza sessions contexts from file: %1").arg(file.errorString()));
	}

	if (sessions.isNull())
		sessions.appendChild(sessions.createElement("stanzaSessions"));

	return sessions;
}

void MessageArchiver::saveStanzaSessionContext(const Jid &AStreamJid, const Jid &AContactJid) const
{
	QDomDocument sessions = loadStanzaSessionsContexts(AStreamJid);

	QFile file(archiveFilePath(AStreamJid,SESSIONS_FILE_NAME));
	if (file.open(QFile::WriteOnly|QFile::Truncate))
	{
		const StanzaSession &session = FSessions.value(AStreamJid).value(AContactJid);
		QDomElement elem = sessions.documentElement().appendChild(sessions.createElement("session")).toElement();
		elem.setAttribute("id",session.sessionId);
		elem.appendChild(sessions.createElement("jid")).appendChild(sessions.createTextNode(AContactJid.pFull()));
		if (!session.defaultPrefs)
			elem.appendChild(sessions.createElement("saveMode")).appendChild(sessions.createTextNode(session.saveMode));

		file.write(sessions.toByteArray());
		file.close();

		LOG_STRM_DEBUG(AStreamJid,QString("Stanza session context saved, jid=%1, sid=%2").arg(AContactJid.full(),session.sessionId));
	}
	else
	{
		REPORT_ERROR(QString("Failed to save stanza session context to file: %1").arg(file.errorString()));
	}
}

void MessageArchiver::restoreStanzaSessionContext(const Jid &AStreamJid, const QString &ASessionId)
{
	LOG_STRM_DEBUG(AStreamJid,QString("Restoring stanza session context, sid=%1").arg(ASessionId));

	QDomDocument sessions = loadStanzaSessionsContexts(AStreamJid);
	QDomElement elem = sessions.documentElement().firstChildElement("session");
	while (!elem.isNull())
	{
		if (ASessionId.isEmpty() || elem.attribute("id")==ASessionId)
		{
			QString requestId;
			Jid contactJid = elem.firstChildElement("jid").text();
			QString saveMode= elem.firstChildElement("saveMode").text();

			if (saveMode.isEmpty() && archivePrefs(AStreamJid).itemPrefs.contains(contactJid))
			{
				requestId = removeArchiveItemPrefs(AStreamJid,contactJid);
			}
			else if (!saveMode.isEmpty() && archiveItemPrefs(AStreamJid,contactJid).save!=saveMode)
			{
				IArchiveStreamPrefs prefs = archivePrefs(AStreamJid);
				prefs.itemPrefs[contactJid].save = saveMode;
				requestId = setArchivePrefs(AStreamJid,prefs);
			}
			else
			{
				removeStanzaSessionContext(AStreamJid,elem.attribute("id"));
			}

			if (!requestId.isEmpty())
				FRestoreRequests.insert(requestId,elem.attribute("id"));
		}
		elem = elem.nextSiblingElement("session");
	}
}

void MessageArchiver::removeStanzaSessionContext(const Jid &AStreamJid, const QString &ASessionId) const
{
	LOG_STRM_DEBUG(AStreamJid,QString("Removing stanza session context, sid=%1").arg(ASessionId));

	QDomDocument sessions = loadStanzaSessionsContexts(AStreamJid);
	QDomElement elem = sessions.documentElement().firstChildElement("session");
	while (!elem.isNull())
	{
		if (elem.attribute("id") == ASessionId)
		{
			elem.parentNode().removeChild(elem);
			break;
		}
		elem = elem.nextSiblingElement("session");
	}

	QFile file(archiveFilePath(AStreamJid,SESSIONS_FILE_NAME));
	if (sessions.documentElement().hasChildNodes())
	{
		if (file.open(QFile::WriteOnly|QFile::Truncate))
		{
			file.write(sessions.toByteArray());
			file.close();
		}
		else
		{
			REPORT_ERROR(QString("Failed to remove stanza session context: %1").arg(file.errorString()));
		}
	}
	else if (!file.remove() && file.exists())
	{
		REPORT_ERROR(QString("Failed to remove stanza session context from file: %1").arg(file.errorString()));
	}
}

void MessageArchiver::startSuspendedStanzaSession(const Jid &AStreamJid, const QString &ARequestId)
{
	if (FSessionNegotiation)
	{
		foreach(const Jid &contactJid, FSessions.value(AStreamJid).keys())
		{
			const StanzaSession &session = FSessions.value(AStreamJid).value(contactJid);
			if (session.requestId == ARequestId)
			{
				LOG_STRM_INFO(AStreamJid,QString("Starting suspending stanza session, sid=%1").arg(session.sessionId));
				saveStanzaSessionContext(AStreamJid,contactJid);
				FSessionNegotiation->resumeSession(AStreamJid,contactJid);
				break;
			}
		}
	}
}

void MessageArchiver::cancelSuspendedStanzaSession(const Jid &AStreamJid, const QString &ARequestId, const XmppStanzaError &AError)
{
	if (FSessionNegotiation)
	{
		foreach(const Jid &contactJid, FSessions.value(AStreamJid).keys())
		{
			StanzaSession &session = FSessions[AStreamJid][contactJid];
			if (session.requestId == ARequestId)
			{
				LOG_STRM_INFO(AStreamJid,QString("Canceling suspending stanza session, sid=%1").arg(session.sessionId));
				session.error = AError;
				FSessionNegotiation->resumeSession(AStreamJid,contactJid);
				break;
			}
		}
	}
}

void MessageArchiver::renegotiateStanzaSessions(const Jid &AStreamJid) const
{
	if (FSessionNegotiation)
	{
		QList<IStanzaSession> sessions = FSessionNegotiation->findSessions(AStreamJid,IStanzaSession::Active);
		foreach(const IStanzaSession &session, sessions)
		{
			bool isOTRSession = isOTRStanzaSession(session);
			IArchiveItemPrefs itemPrefs = archiveItemPrefs(AStreamJid,session.contactJid);
			if ((isOTRSession && itemPrefs.save!=ARCHIVE_SAVE_FALSE) || (!isOTRSession && itemPrefs.otr==ARCHIVE_OTR_REQUIRE))
			{
				LOG_STRM_INFO(AStreamJid,QString("Renegotiating stanza session, sid=%1").arg(session.sessionId));
				removeStanzaSessionContext(AStreamJid,session.sessionId);
				FSessionNegotiation->initSession(AStreamJid,session.contactJid);
			}
		}
	}
}

void MessageArchiver::registerDiscoFeatures()
{
	IDiscoFeature dfeature;

	dfeature.active = false;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_HISTORY);
	dfeature.var = NS_ARCHIVE;
	dfeature.name = tr("Messages Archiving");
	dfeature.description = tr("Supports the archiving of the messages");
	FDiscovery->insertDiscoFeature(dfeature);
	dfeature.var = NS_ARCHIVE_OLD;
	FDiscovery->insertDiscoFeature(dfeature);

	dfeature.var = NS_ARCHIVE_AUTO;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_HISTORY);
	dfeature.name = tr("Automatic Messages Archiving");
	dfeature.description = tr("Supports the automatic archiving of the messages");
	FDiscovery->insertDiscoFeature(dfeature);
	dfeature.var = NS_ARCHIVE_OLD_AUTO;
	FDiscovery->insertDiscoFeature(dfeature);

	dfeature.var = NS_ARCHIVE_MANAGE;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_HISTORY);
	dfeature.name = tr("Managing Archived Messages");
	dfeature.description = tr("Supports the managing of the archived messages");
	FDiscovery->insertDiscoFeature(dfeature);
	dfeature.var = NS_ARCHIVE_OLD_MANAGE;
	FDiscovery->insertDiscoFeature(dfeature);

	dfeature.var = NS_ARCHIVE_MANUAL;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_HISTORY);
	dfeature.name = tr("Manual Messages Archiving");
	dfeature.description = tr("Supports the manual archiving of the messages");
	FDiscovery->insertDiscoFeature(dfeature);
	dfeature.var = NS_ARCHIVE_OLD_MANUAL;
	FDiscovery->insertDiscoFeature(dfeature);

	dfeature.var = NS_ARCHIVE_PREF;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_HISTORY);
	dfeature.name = tr("Messages Archive Preferences");
	dfeature.description = tr("Supports the storing of the archive preferences");
	FDiscovery->insertDiscoFeature(dfeature);
	dfeature.var = NS_ARCHIVE_OLD_PREF;
	FDiscovery->insertDiscoFeature(dfeature);
}

void MessageArchiver::openHistoryOptionsNode(const QUuid &AAccountId)
{
	if (FOptionsManager)
	{
		QString historyNodeId = QString(OPN_ACCOUNTS_HISTORY).replace("[id]",AAccountId.toString());
		IOptionsDialogNode historyNode = { ONO_ACCOUNTS_HISTORY, historyNodeId, MNI_HISTORY, tr("History") };
		FOptionsManager->insertOptionsDialogNode(historyNode);
	}
}

void MessageArchiver::closeHistoryOptionsNode(const QUuid &AAccountId)
{
	if (FOptionsManager)
	{
		QString historyNodeId = QString(OPN_ACCOUNTS_HISTORY).replace("[id]",AAccountId.toString());
		FOptionsManager->removeOptionsDialogNode(historyNodeId);
	}
}

bool MessageArchiver::isArchiveDuplicationEnabled(const Jid &AStreamJid) const
{
	IAccount *account = FAccountManager!=NULL ? FAccountManager->findAccountByStream(AStreamJid) : NULL;
	return account!=NULL ? account->optionsNode().value("history-duplicate").toBool() : false;
}

bool MessageArchiver::isSelectionAccepted(const QList<IRosterIndex *> &ASelected) const
{
	int singleKind = -1;
	foreach(IRosterIndex *index, ASelected)
	{
		Jid contactJid = index->data(RDR_FULL_JID).toString();
		if (!contactJid.isValid())
			return false;
		else if (singleKind!=-1 && singleKind!=index->kind())
			return false;
		singleKind = index->kind();
	}
	return !ASelected.isEmpty();
}

Menu *MessageArchiver::createContextMenu(const QStringList &AStreams, const QStringList &AContacts, QWidget *AParent) const
{
	bool isMultiSelection = AStreams.count()>1;
	bool isStreamMenu = AContacts.first().isEmpty();

	bool isAnyMangement = false;
	bool isAllAutoSave = true;
	bool isAllSupported = true;
	bool isAllPrefsEnabled = true;
	bool isAllDefaultPrefs = !isStreamMenu;
	QString allPrefsSave = QString();
	QString allPrefsOtr = QString();
	for (int i=0; i<AStreams.count(); i++)
	{
		isAllAutoSave = isAllAutoSave && isArchiveAutoSave(AStreams.at(i));
		isAllSupported = isAllSupported && isSupported(AStreams.at(i),NS_ARCHIVE_AUTO);
		isAllPrefsEnabled = isAllPrefsEnabled && isArchivePrefsEnabled(AStreams.at(i));

		if (isAllPrefsEnabled)
		{
			IArchiveItemPrefs itemPrefs = isStreamMenu ? archivePrefs(AStreams.at(i)).defaultPrefs : archiveItemPrefs(AStreams.at(i),AContacts.at(i));

			if (allPrefsSave.isNull())
				allPrefsSave = itemPrefs.save;
			else if (allPrefsSave != itemPrefs.save)
				allPrefsSave = "-=different=-";

			if (allPrefsOtr.isNull())
				allPrefsOtr = itemPrefs.otr;
			else if (allPrefsOtr != itemPrefs.otr)
				allPrefsOtr = "-=different=-";

			isAllDefaultPrefs = isAllDefaultPrefs && !archivePrefs(AStreams.at(i)).itemPrefs.contains(AContacts.at(i));
		}

		isAnyMangement = isAnyMangement || !engineOrderByCapability(AStreams.at(i),IArchiveEngine::ArchiveManagement).isEmpty();
	}

	Menu *menu = new Menu(AParent);
	menu->setTitle(tr("History"));
	menu->setIcon(RSR_STORAGE_MENUICONS,MNI_HISTORY);

	if (isAnyMangement)
	{
		Action *viewAction = new Action(menu);
		viewAction->setText(tr("View History"));
		viewAction->setIcon(RSR_STORAGE_MENUICONS,MNI_HISTORY);
		viewAction->setData(ADR_STREAM_JID,AStreams);
		viewAction->setData(ADR_CONTACT_JID,AContacts);
		viewAction->setShortcutId(SCT_ROSTERVIEW_SHOWHISTORY);
		connect(viewAction,SIGNAL(triggered(bool)),SLOT(onShowArchiveWindowByAction(bool)));
		menu->addAction(viewAction,AG_DEFAULT,false);
	}

	if (isStreamMenu && isAllSupported)
	{
		Action *autoAction = new Action(menu);
		autoAction->setCheckable(true);
		autoAction->setText(tr("Automatic Archiving"));
		autoAction->setData(ADR_STREAM_JID,AStreams);
		autoAction->setChecked(isAllAutoSave);
		connect(autoAction,SIGNAL(triggered(bool)),SLOT(onSetAutoArchivingByAction(bool)));
		menu->addAction(autoAction,AG_DEFAULT+100,false);
	}

	if (isAllPrefsEnabled)
	{
		Action *fullSaveAction = new Action(menu);
		fullSaveAction->setCheckable(true);
		fullSaveAction->setText(tr("Save Messages with Formatting"));
		fullSaveAction->setData(ADR_STREAM_JID,AStreams);
		fullSaveAction->setData(ADR_CONTACT_JID,AContacts);
		fullSaveAction->setData(ADR_ITEM_SAVE,ARCHIVE_SAVE_MESSAGE);
		fullSaveAction->setChecked(allPrefsSave==ARCHIVE_SAVE_MESSAGE || allPrefsSave==ARCHIVE_SAVE_STREAM);
		connect(fullSaveAction,SIGNAL(triggered(bool)),SLOT(onSetItemPrefsByAction(bool)));
		menu->addAction(fullSaveAction,AG_DEFAULT+200);

		Action *bodySaveAction = new Action(menu);
		bodySaveAction->setCheckable(true);
		bodySaveAction->setText(tr("Save Only Messages Text"));
		bodySaveAction->setData(ADR_STREAM_JID,AStreams);
		bodySaveAction->setData(ADR_CONTACT_JID,AContacts);
		bodySaveAction->setData(ADR_ITEM_SAVE,ARCHIVE_SAVE_BODY);
		bodySaveAction->setChecked(allPrefsSave == ARCHIVE_SAVE_BODY);
		connect(bodySaveAction,SIGNAL(triggered(bool)),SLOT(onSetItemPrefsByAction(bool)));
		menu->addAction(bodySaveAction,AG_DEFAULT+200);

		Action *disableSaveAction = new Action(menu);
		disableSaveAction->setCheckable(true);
		disableSaveAction->setText(tr("Do not Save Messages"));
		disableSaveAction->setData(ADR_STREAM_JID,AStreams);
		disableSaveAction->setData(ADR_CONTACT_JID,AContacts);
		disableSaveAction->setData(ADR_ITEM_SAVE,ARCHIVE_SAVE_FALSE);
		disableSaveAction->setChecked(allPrefsSave == ARCHIVE_SAVE_FALSE);
		connect(disableSaveAction,SIGNAL(triggered(bool)),SLOT(onSetItemPrefsByAction(bool)));
		menu->addAction(disableSaveAction,AG_DEFAULT+200);

		QActionGroup *saveGroup = new QActionGroup(menu);
		saveGroup->addAction(fullSaveAction);
		saveGroup->addAction(bodySaveAction);
		saveGroup->addAction(disableSaveAction);

		Action *allowOTRAction = new Action(menu);
		allowOTRAction->setCheckable(true);
		allowOTRAction->setText(tr("Allow Off-The-Record Sessions"));
		allowOTRAction->setData(ADR_STREAM_JID,AStreams);
		allowOTRAction->setData(ADR_CONTACT_JID,AContacts);
		allowOTRAction->setData(ADR_ITEM_OTR,ARCHIVE_OTR_CONCEDE);
		allowOTRAction->setChecked(allPrefsOtr!=ARCHIVE_OTR_APPROVE && allPrefsOtr!=ARCHIVE_OTR_FORBID);
		connect(allowOTRAction,SIGNAL(triggered(bool)),SLOT(onSetItemPrefsByAction(bool)));
		menu->addAction(allowOTRAction,AG_DEFAULT+300);

		Action *forbidOTRAction = new Action(menu);
		forbidOTRAction->setCheckable(true);
		forbidOTRAction->setText(tr("Forbid Off-The-Record Sessions"));
		forbidOTRAction->setData(ADR_STREAM_JID,AStreams);
		forbidOTRAction->setData(ADR_CONTACT_JID,AContacts);
		forbidOTRAction->setData(ADR_ITEM_OTR,ARCHIVE_OTR_FORBID);
		forbidOTRAction->setChecked(allPrefsOtr == ARCHIVE_OTR_FORBID);
		connect(forbidOTRAction,SIGNAL(triggered(bool)),SLOT(onSetItemPrefsByAction(bool)));
		menu->addAction(forbidOTRAction,AG_DEFAULT+300);

		Action *approveOTRAction = new Action(menu);
		approveOTRAction->setCheckable(true);
		approveOTRAction->setText(tr("Manually Approve Off-The-Record Sessions"));
		approveOTRAction->setData(ADR_STREAM_JID,AStreams);
		approveOTRAction->setData(ADR_CONTACT_JID,AContacts);
		approveOTRAction->setData(ADR_ITEM_OTR,ARCHIVE_OTR_APPROVE);
		approveOTRAction->setChecked(allPrefsOtr == ARCHIVE_OTR_APPROVE);
		connect(approveOTRAction,SIGNAL(triggered(bool)),SLOT(onSetItemPrefsByAction(bool)));
		menu->addAction(approveOTRAction,AG_DEFAULT+300);

		QActionGroup *otrGroup = new QActionGroup(menu);
		otrGroup->addAction(allowOTRAction);
		otrGroup->addAction(forbidOTRAction);
		otrGroup->addAction(approveOTRAction);

		if (!isStreamMenu)
		{
			Action *defAction = new Action(menu);
			defAction->setCheckable(true);
			defAction->setText(tr("Use Default Options"));
			defAction->setData(ADR_STREAM_JID,AStreams);
			defAction->setData(ADR_CONTACT_JID,AContacts);
			defAction->setChecked(isAllDefaultPrefs);
			connect(defAction,SIGNAL(triggered(bool)),SLOT(onRemoveItemPrefsByAction(bool)));
			menu->addAction(defAction,AG_DEFAULT+500,false);
		}
	}

	if (!isMultiSelection && isStreamMenu && isReady(AStreams.first()))
	{
		Action *optionsAction = new Action(menu);
		optionsAction->setText(tr("Options..."));
		optionsAction->setData(ADR_STREAM_JID,AStreams.first());
		connect(optionsAction,SIGNAL(triggered(bool)),SLOT(onShowHistoryOptionsDialogByAction(bool)));
		menu->addAction(optionsAction,AG_DEFAULT+500,false);
	}

	return menu;
}

void MessageArchiver::notifyInChatWindow(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessage) const
{
	IMessageChatWindow *window = FMessageWidgets!=NULL ? FMessageWidgets->findChatWindow(AStreamJid,AContactJid,true) : NULL;
	if (window)
	{
		IMessageStyleContentOptions options;
		options.kind = IMessageStyleContentOptions::KindStatus;
		options.type |= IMessageStyleContentOptions::TypeEvent;
		options.direction = IMessageStyleContentOptions::DirectionIn;
		options.time = QDateTime::currentDateTime();
		window->viewWidget()->appendText(AMessage,options);
	}
}

void MessageArchiver::onEngineCapabilitiesChanged(const Jid &AStreamJid)
{
	emit totalCapabilitiesChanged(AStreamJid);
}

void MessageArchiver::onEngineRequestFailed(const QString &AId, const XmppError &AError)
{
	if (FRequestId2LocalId.contains(AId))
	{
		QString localId = FRequestId2LocalId.take(AId);
		IArchiveEngine *engine = qobject_cast<IArchiveEngine *>(sender());
		if (FHeadersRequests.contains(localId))
		{
			HeadersRequest &request = FHeadersRequests[localId];
			request.lastError = AError;
			request.engines.removeAll(engine);
			processHeadersRequest(localId,request);
		}
		else if (FCollectionRequests.contains(localId))
		{
			CollectionRequest &request = FCollectionRequests[localId];
			request.lastError = AError;
			processCollectionRequest(localId,request);
		}
		else if (FRemoveRequests.contains(localId))
		{
			RemoveRequest &request = FRemoveRequests[localId];
			request.lastError = AError;
			request.engines.removeAll(engine);
			processRemoveRequest(localId,request);
		}
	}
}

void MessageArchiver::onEngineHeadersLoaded(const QString &AId, const QList<IArchiveHeader> &AHeaders)
{
	if (FRequestId2LocalId.contains(AId))
	{
		QString localId = FRequestId2LocalId.take(AId);
		if (FHeadersRequests.contains(localId))
		{
			IArchiveEngine *engine = qobject_cast<IArchiveEngine *>(sender());
			HeadersRequest &request = FHeadersRequests[localId];
			request.headers.insert(engine,AHeaders);
			processHeadersRequest(localId,request);
		}
	}
}

void MessageArchiver::onEngineCollectionLoaded(const QString &AId, const IArchiveCollection &ACollection)
{
	if (FRequestId2LocalId.contains(AId))
	{
		QString localId = FRequestId2LocalId.take(AId);
		if (FCollectionRequests.contains(localId))
		{
			CollectionRequest &request = FCollectionRequests[localId];
			request.collection = ACollection;
			processCollectionRequest(localId,request);
		}
	}
}

void MessageArchiver::onEngineCollectionsRemoved(const QString &AId, const IArchiveRequest &ARequest)
{
	Q_UNUSED(ARequest);
	if (FRequestId2LocalId.contains(AId))
	{
		QString localId = FRequestId2LocalId.take(AId);
		if (FRemoveRequests.contains(localId))
		{
			IArchiveEngine *engine = qobject_cast<IArchiveEngine *>(sender());
			RemoveRequest &request = FRemoveRequests[localId];
			request.engines.removeAll(engine);
			processRemoveRequest(localId,request);
		}
	}
}

void MessageArchiver::onSelfRequestFailed(const QString &AId, const XmppError &AError)
{
	if (FRequestId2LocalId.contains(AId))
	{
		QString localId = FRequestId2LocalId.take(AId);
		if (FMesssagesRequests.contains(localId))
		{
			MessagesRequest &request = FMesssagesRequests[localId];
			request.lastError = AError;
			processMessagesRequest(localId,request);
		}
	}
}

void MessageArchiver::onSelfHeadersLoaded(const QString &AId, const QList<IArchiveHeader> &AHeaders)
{
	if (FRequestId2LocalId.contains(AId))
	{
		QString localId = FRequestId2LocalId.take(AId);
		if (FMesssagesRequests.contains(localId))
		{
			MessagesRequest &request = FMesssagesRequests[localId];
			request.headers = AHeaders;
			processMessagesRequest(localId,request);
		}
	}
}

void MessageArchiver::onSelfCollectionLoaded(const QString &AId, const IArchiveCollection &ACollection)
{
	if (FRequestId2LocalId.contains(AId))
	{
		QString localId = FRequestId2LocalId.take(AId);
		if (FMesssagesRequests.contains(localId))
		{
			MessagesRequest &request = FMesssagesRequests[localId];
			request.body.messages += ACollection.body.messages;
			request.body.notes += ACollection.body.notes;
			processMessagesRequest(localId,request);
		}
	}
}

void MessageArchiver::onAccountInserted(IAccount *AAccount)
{
	openHistoryOptionsNode(AAccount->accountId());
}

void MessageArchiver::onAccountRemoved(IAccount *AAccount)
{
	closeHistoryOptionsNode(AAccount->accountId());
}

void MessageArchiver::onXmppStreamOpened(IXmppStream *AXmppStream)
{
	if (FStanzaProcessor)
	{
		IStanzaHandle shandle;
		shandle.handler = this;
		shandle.streamJid = AXmppStream->streamJid();

		shandle.order = SHO_DEFAULT;
		shandle.direction = IStanzaHandle::DirectionIn;
		shandle.conditions.append(SHC_PREFS);
		shandle.conditions.append(SHC_PREFS_OLD);
		FSHIPrefs.insert(shandle.streamJid,FStanzaProcessor->insertStanzaHandle(shandle));

		shandle.conditions.clear();
		shandle.conditions.append(SHC_MESSAGE_BODY);
		FSHIMessageIn.insert(shandle.streamJid,FStanzaProcessor->insertStanzaHandle(shandle));

		shandle.direction = IStanzaHandle::DirectionOut;
		FSHIMessageOut.insert(shandle.streamJid,FStanzaProcessor->insertStanzaHandle(shandle));

		shandle.order = SHO_MO_ARCHIVER;
		FSHIMessageBlocks.insert(shandle.streamJid,FStanzaProcessor->insertStanzaHandle(shandle));
	}

	loadPendingMessages(AXmppStream->streamJid());

	if (FDiscovery == NULL)
		applyArchivePrefs(AXmppStream->streamJid(),QDomElement());

	ArchiveReplicator *replicator = new ArchiveReplicator(this,AXmppStream->streamJid(),this);
	FReplicators.insert(AXmppStream->streamJid(),replicator);
}

void MessageArchiver::onXmppStreamClosed(IXmppStream *AXmppStream)
{
	if (FStanzaProcessor)
	{
		FStanzaProcessor->removeStanzaHandle(FSHIPrefs.take(AXmppStream->streamJid()));
		FStanzaProcessor->removeStanzaHandle(FSHIMessageIn.take(AXmppStream->streamJid()));
		FStanzaProcessor->removeStanzaHandle(FSHIMessageOut.take(AXmppStream->streamJid()));
	}

	savePendingMessages(AXmppStream->streamJid());

	FFeatures.remove(AXmppStream->streamJid());
	FNamespaces.remove(AXmppStream->streamJid());
	FArchivePrefs.remove(AXmppStream->streamJid());
	FInStoragePrefs.removeAll(AXmppStream->streamJid());
	FSessions.remove(AXmppStream->streamJid());

	emit archivePrefsChanged(AXmppStream->streamJid());
	emit archivePrefsClosed(AXmppStream->streamJid());
}

void MessageArchiver::onXmppStreamAboutToClose(IXmppStream *AXmppStream)
{
	ArchiveReplicator *replicator = FReplicators.take(AXmppStream->streamJid());
	if (replicator)
		replicator->quitAndDestroy();
}

void MessageArchiver::onPrivateDataLoadedSaved(const QString &AId, const Jid &AStreamJid, const QDomElement &AElement)
{
	if (FPrefsLoadRequests.contains(AId))
	{
		LOG_STRM_INFO(AStreamJid,QString("Storage archive prefs loaded, id=%1").arg(AId));
		FPrefsLoadRequests.remove(AId);
		applyArchivePrefs(AStreamJid,AElement);
		emit requestCompleted(AId);
	}
	else if (FPrefsSaveRequests.contains(AId))
	{
		LOG_STRM_INFO(AStreamJid,QString("Storage archive prefs saved, id=%1").arg(AId));
		applyArchivePrefs(AStreamJid,AElement);
		FPrefsSaveRequests.remove(AId);

		if (FRestoreRequests.contains(AId))
		{
			LOG_STRM_DEBUG(AStreamJid,QString("Stanza session context restored, id=%1").arg(AId));
			removeStanzaSessionContext(AStreamJid,FRestoreRequests.take(AId));
		}
		else
		{
			startSuspendedStanzaSession(AStreamJid,AId);
		}

		emit requestCompleted(AId);
	}
}

void MessageArchiver::onPrivateDataChanged(const Jid &AStreamJid, const QString &ATagName, const QString &ANamespace)
{
	if (FInStoragePrefs.contains(AStreamJid) && ATagName==PST_ARCHIVE_PREFS && ANamespace==PSN_ARCHIVE_PREFS)
	{
		loadStoragePrefs(AStreamJid);
	}
}

void MessageArchiver::onShortcutActivated(const QString &AId, QWidget *AWidget)
{
	if (FRostersViewPlugin && AWidget==FRostersViewPlugin->rostersView()->instance())
	{
		QList<IRosterIndex *> indexes = FRostersViewPlugin->rostersView()->selectedRosterIndexes();
		if (AId==SCT_ROSTERVIEW_SHOWHISTORY && isSelectionAccepted(indexes))
		{
			QMultiMap<Jid,Jid> addresses;
			foreach(IRosterIndex *index, indexes)
			{
				if (index->kind() == RIK_STREAM_ROOT)
				{
					addresses.insertMulti(index->data(RDR_STREAM_JID).toString(),Jid::null);
				}
				else if (index->kind() == RIK_METACONTACT)
				{
					for (int row=0; row<index->childCount(); row++)
					{
						IRosterIndex *metaItemIndex = index->childIndex(row);
						addresses.insertMulti(metaItemIndex->data(RDR_STREAM_JID).toString(),metaItemIndex->data(RDR_PREP_BARE_JID).toString());
					}
				}
				else
				{
					addresses.insertMulti(index->data(RDR_STREAM_JID).toString(),index->data(RDR_PREP_BARE_JID).toString());
				}
			}
			showArchiveWindow(addresses);
		}
	}
}

void MessageArchiver::onRostersViewIndexMultiSelection(const QList<IRosterIndex *> &ASelected, bool &AAccepted)
{
	AAccepted = AAccepted || isSelectionAccepted(ASelected);
}

void MessageArchiver::onRostersViewIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
	if (ALabelId==AdvancedDelegateItem::DisplayId && isSelectionAccepted(AIndexes))
	{
		int indexKind = AIndexes.first()->kind();
		QMap<int, QStringList> rolesMap = FRostersViewPlugin->rostersView()->indexesRolesMap(AIndexes,
			QList<int>()<<RDR_STREAM_JID<<RDR_PREP_BARE_JID<<RDR_ANY_ROLE,RDR_PREP_BARE_JID,RDR_STREAM_JID);

		Menu *menu;
		if (indexKind == RIK_STREAM_ROOT)
			menu = createContextMenu(rolesMap.value(RDR_STREAM_JID),rolesMap.value(RDR_ANY_ROLE),AMenu);
		else
			menu = createContextMenu(rolesMap.value(RDR_STREAM_JID),rolesMap.value(RDR_PREP_BARE_JID),AMenu);

		if (!menu->isEmpty())
			AMenu->addAction(menu->menuAction(),AG_RVCM_ARCHIVER_OPTIONS,true);
		else
			delete menu;
	}
}

void MessageArchiver::onMultiUserContextMenu(IMultiUserChatWindow *AWindow, IMultiUser *AUser, Menu *AMenu)
{
	Menu *menu = createContextMenu(QStringList()<<AWindow->streamJid().pFull(),QStringList()<<AUser->userJid().pFull(),AMenu);
	if (!menu->isEmpty())
		AMenu->addAction(menu->menuAction(),AG_MUCM_ARCHIVER_OPTIONS,true);
	else
		delete menu;
}

void MessageArchiver::onSetItemPrefsByAction(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		QMap<Jid, IArchiveStreamPrefs> streamPrefs;
		QStringList streams = action->data(ADR_STREAM_JID).toStringList();
		QStringList contacts = action->data(ADR_CONTACT_JID).toStringList();
		for (int i=0; i<streams.count(); i++)
		{
			Jid streamJid = streams.at(i);
			Jid contactJid = contacts.at(i);

			if (!streamPrefs.contains(streamJid))
				streamPrefs[streamJid] = archivePrefs(streamJid);
			IArchiveStreamPrefs &prefs = streamPrefs[streamJid];

			QString itemSave = action->data(ADR_ITEM_SAVE).toString();
			if (!itemSave.isEmpty())
			{
				if (streamJid.pBare() != contactJid.pBare())
				{
					prefs.itemPrefs[contactJid] = archiveItemPrefs(streamJid,contactJid);
					prefs.itemPrefs[contactJid].save = itemSave;
				}
				else
				{
					prefs.defaultPrefs.save = itemSave;
				}
			}

			QString itemOTR = action->data(ADR_ITEM_OTR).toString();
			if (!itemOTR.isEmpty())
			{
				if (streamJid.pBare() != contactJid.pBare())
				{
					prefs.itemPrefs[contactJid] = archiveItemPrefs(streamJid,contactJid);
					prefs.itemPrefs[contactJid].otr = itemOTR;
				}
				else
				{
					prefs.defaultPrefs.otr = itemOTR;
				}
			}
		}

		for (QMap<Jid, IArchiveStreamPrefs>::const_iterator it=streamPrefs.constBegin(); it!=streamPrefs.constEnd(); ++it)
			setArchivePrefs(it.key(),it.value());
	}
}

void MessageArchiver::onSetAutoArchivingByAction(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		foreach(const Jid &streamJid, action->data(ADR_STREAM_JID).toStringList())
			setArchiveAutoSave(streamJid,action->isChecked());
	}
}

void MessageArchiver::onRemoveItemPrefsByAction(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		QMap<Jid, IArchiveStreamPrefs> streamPrefs;
		QStringList streams = action->data(ADR_STREAM_JID).toStringList();
		QStringList contacts = action->data(ADR_CONTACT_JID).toStringList();
		for (int i=0; i<streams.count(); i++)
		{
			if (!isSupported(streams.at(i),NS_ARCHIVE_PREF))
			{
				if (!streamPrefs.contains(streams.at(i)))
					streamPrefs[streams.at(i)] = archivePrefs(streams.at(i));
				IArchiveStreamPrefs &prefs = streamPrefs[streams.at(i)];

				prefs.itemPrefs[contacts.at(i)].save = QString();
				prefs.itemPrefs[contacts.at(i)].otr = QString();
			}
			else
			{
				removeArchiveItemPrefs(streams.at(i),contacts.at(i));
			}
		}

		for (QMap<Jid, IArchiveStreamPrefs>::const_iterator it=streamPrefs.constBegin(); it!=streamPrefs.constEnd(); ++it)
			setArchivePrefs(it.key(),it.value());
	}
}

void MessageArchiver::onShowArchiveWindowByAction(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		QMultiMap<Jid,Jid> addresses;
		QStringList streams = action->data(ADR_STREAM_JID).toStringList();
		QStringList contacts = action->data(ADR_CONTACT_JID).toStringList();
		for(int i=0; i<streams.count() && i<contacts.count(); i++)
			addresses.insertMulti(streams.at(i),contacts.at(i));
		showArchiveWindow(addresses);
	}
}

void MessageArchiver::onShowArchiveWindowByToolBarAction(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		IMessageToolBarWidget *toolBarWidget = qobject_cast<IMessageToolBarWidget *>(action->parent());
		if (toolBarWidget)
			showArchiveWindow(toolBarWidget->messageWindow()->address()->availAddresses(true));
	}
}

void MessageArchiver::onShowHistoryOptionsDialogByAction(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (FOptionsManager && FAccountManager && action)
	{
		Jid streamJid = action->data(ADR_STREAM_JID).toString();
		IAccount *account = FAccountManager->findAccountByStream(streamJid);
		if (account)
		{
			QString rootId = OPN_ACCOUNTS"."+account->accountId().toString();
			QString nodeId = QString(OPN_ACCOUNTS_HISTORY).replace("[id]",account->accountId().toString());
			FOptionsManager->showOptionsDialog(nodeId, rootId);
		}
	}
}

void MessageArchiver::onDiscoveryInfoReceived(const IDiscoInfo &AInfo)
{
	if (!FNamespaces.contains(AInfo.streamJid) && !FInStoragePrefs.contains(AInfo.streamJid) && AInfo.node.isEmpty() && AInfo.streamJid.pDomain()==AInfo.contactJid.pFull())
	{
		QList<QString> &features = FFeatures[AInfo.streamJid];
		foreach(const QString &feature, AInfo.features)
		{
			if (feature==NS_ARCHIVE || feature==NS_ARCHIVE_OLD)
				features.append(NS_ARCHIVE);
			else if (feature==NS_ARCHIVE_AUTO || feature==NS_ARCHIVE_OLD_AUTO)
				features.append(NS_ARCHIVE_AUTO);
			else if (feature==NS_ARCHIVE_MANAGE || feature==NS_ARCHIVE_OLD_MANAGE)
				features.append(NS_ARCHIVE_MANAGE);
			else if (feature==NS_ARCHIVE_MANUAL || feature==NS_ARCHIVE_OLD_MANUAL)
				features.append(NS_ARCHIVE_MANUAL);
			else if (feature==NS_ARCHIVE_PREF || feature==NS_ARCHIVE_OLD_PREF)
				features.append(NS_ARCHIVE_PREF);
		}

		if (!features.isEmpty() && !AInfo.features.contains(features.first()))
			FNamespaces.insert(AInfo.streamJid, NS_ARCHIVE_OLD);
		else
			FNamespaces.insert(AInfo.streamJid, NS_ARCHIVE);

		if (features.contains(NS_ARCHIVE_PREF))
		{
			loadServerPrefs(AInfo.streamJid);
		}
		else if (features.contains(NS_ARCHIVE_AUTO))
		{
			FInStoragePrefs.append(AInfo.streamJid);
			applyArchivePrefs(AInfo.streamJid,QDomElement());
		}
		else
		{
			applyArchivePrefs(AInfo.streamJid,QDomElement());
		}
	}
}

void MessageArchiver::onStanzaSessionActivated(const IStanzaSession &ASession)
{
	bool isOTR = isOTRStanzaSession(ASession);
	if (!isOTR && FSessions.value(ASession.streamJid).contains(ASession.contactJid))
		restoreStanzaSessionContext(ASession.streamJid,ASession.sessionId);
	notifyInChatWindow(ASession.streamJid,ASession.contactJid,tr("Session negotiated: message logging %1").arg(isOTR ? tr("disallowed") : tr("allowed")));
}

void MessageArchiver::onStanzaSessionTerminated(const IStanzaSession &ASession)
{
	if (FSessions.value(ASession.streamJid).contains(ASession.contactJid))
	{
		restoreStanzaSessionContext(ASession.streamJid,ASession.sessionId);
		FSessions[ASession.streamJid].remove(ASession.contactJid);
	}
	if (ASession.error.isNull())
		notifyInChatWindow(ASession.streamJid,ASession.contactJid,tr("Session terminated"));
	else
		notifyInChatWindow(ASession.streamJid,ASession.contactJid,tr("Session failed: %1").arg(ASession.error.errorMessage()));
}

void MessageArchiver::onToolBarWidgetCreated(IMessageToolBarWidget *AWidget)
{
	Action *action = new Action(AWidget->toolBarChanger()->toolBar());
	action->setText(tr("View History"));
	action->setIcon(RSR_STORAGE_MENUICONS,MNI_HISTORY);
	action->setShortcutId(SCT_MESSAGEWINDOWS_SHOWHISTORY);
	connect(action,SIGNAL(triggered(bool)),SLOT(onShowArchiveWindowByToolBarAction(bool)));
	QToolButton *historyButton = AWidget->toolBarChanger()->insertAction(action,TBG_MWTBW_ARCHIVE_VIEW);

	ChatWindowMenu *historyMenu = new ChatWindowMenu(this,AWidget,AWidget->toolBarChanger()->toolBar());
	historyButton->setMenu(historyMenu);
	historyButton->setPopupMode(QToolButton::MenuButtonPopup);
}

void MessageArchiver::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.cleanPath() == OPV_HISTORY_ENGINE_ENABLED)
	{
		QUuid id = ANode.parent().nspace();
		emit archiveEngineEnableChanged(id,ANode.value().toBool());
		emit totalCapabilitiesChanged(Jid::null);
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_messagearchiver, MessageArchiver)
#endif
