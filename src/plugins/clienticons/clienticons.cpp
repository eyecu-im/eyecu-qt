#include <QMouseEvent>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QFile>

#include <definitions/rosterlabelholderorders.h>
#include <definitions/stanzahandlerorders.h>
#include <definitions/multiuserdataholderorders.h>
#include <definitions/multiuserdataroles.h>
#include <definitions/multiuseritemlabels.h>
#include <definitions/multiusertooltiporders.h>
#include "clienticons.h"

#define PROPERTY_CLIENT "client"
#define SHC_PRESENCE  "/presence/c[@xmlns=" NS_CAPS "]"

#define ADR_STREAM_JID          Action::DR_StreamJid
#define ADR_CONTACT_JID         Action::DR_Parametr1
#define ADR_CLIPBOARD_ICON      Action::DR_Parametr2

ClientIcons::ClientIcons():
	FMainWindowPlugin(NULL),
	FPresenceManager(NULL),
	FStanzaProcessor(NULL),
	FXmppStreamManager(NULL),
	FOptionsManager(NULL),
	FRosterManager(NULL),
	FRostersModel(NULL),
	FRostersViewPlugin(NULL),
	FMessageWidgets(NULL),
	FServiceDiscovery(NULL),
	FMultiUserChatManager(NULL),
	FSimpleContactsView(false),
	FRosterLabelId(0),
	FRosterIndexKinds(QList<int>() << RIK_CONTACT << RIK_METACONTACT << RIK_METACONTACT_ITEM << RIK_RECENT_ITEM << RIK_MY_RESOURCE << RIK_STREAM_ROOT)
{
#ifdef DEBUG_RESOURCES_DIR
	FileStorage::setResourcesDirs(FileStorage::resourcesDirs() << DEBUG_RESOURCES_DIR);
#endif
}

ClientIcons::~ClientIcons()
{}

void ClientIcons::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Client Icons");
	APluginInfo->description = tr("Displays a client icon in the roster");
	APluginInfo->version = "0.3";
	APluginInfo->author = "Alexey Ivanov aka krab";
	APluginInfo->homePage = "http://code.google.com/p/vacuum-plugins";
	APluginInfo->dependences.append(XMPPSTREAMS_UUID);
	APluginInfo->dependences.append(PRESENCE_UUID);
}

bool ClientIcons::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	AInitOrder = 30;

	IPlugin *plugin = APluginManager->pluginInterface("IMainWindowPlugin").value(0);
	if(plugin)
		FMainWindowPlugin = qobject_cast<IMainWindowPlugin *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0,NULL);
	if (plugin)
		FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
		connect(FMessageWidgets->instance(), SIGNAL(chatWindowCreated(IMessageChatWindow *)), SLOT(onChatWindowCreated(IMessageChatWindow *)));
	}

	plugin = APluginManager->pluginInterface("IClientInfo").value(0,NULL);
	if (plugin)
		FClientInfo = qobject_cast<IClientInfo *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0, NULL);
	if(plugin)
	{
		FXmppStreamManager = qobject_cast<IXmppStreamManager *>(plugin->instance());
		if(FXmppStreamManager)
		{
			connect(FXmppStreamManager->instance(),SIGNAL(streamOpened(IXmppStream *)),SLOT(onStreamOpened(IXmppStream *)));
			connect(FXmppStreamManager->instance(),SIGNAL(streamClosed(IXmppStream *)),SLOT(onStreamClosed(IXmppStream *)));
		}
	}

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0, NULL);
	if(plugin)
	{
		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());
		if(FPresenceManager)
		{
			connect(FPresenceManager->instance(),SIGNAL(contactStateChanged(const Jid &, const Jid &, bool)),
					SLOT(onContactStateChanged(const Jid &, const Jid &, bool)));
		}
	}

	plugin = APluginManager->pluginInterface("IRosterManager").value(0,NULL);
	if (plugin)
		FRosterManager = qobject_cast<IRosterManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IRostersModel").value(0, NULL);
	if(plugin)
	{
		FRostersModel = qobject_cast<IRostersModel *>(plugin->instance());
		if (FRostersModel)
			connect(FRostersModel->instance(),SIGNAL(indexInserted(IRosterIndex *)), SLOT(onRosterIndexInserted(IRosterIndex *)));
	}

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0, NULL);
	if(plugin)
	{
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
		if (FOptionsManager)
		{
			connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
			connect(Options::instance(),SIGNAL(optionsChanged(OptionsNode)),SLOT(onOptionsChanged(OptionsNode)));
		}
	}

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0, NULL);
	if(plugin)
		FServiceDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IMultiUserChatManager").value(0, NULL);
	if(plugin)
	{
		FMultiUserChatManager = qobject_cast<IMultiUserChatManager *>(plugin->instance());
		connect(FMultiUserChatManager->instance(), SIGNAL(multiChatWindowCreated(IMultiUserChatWindow*)), SLOT(onMultiChatWindowCreated(IMultiUserChatWindow*)));
	}

	plugin = APluginManager->pluginInterface("IRostersViewPlugin").value(0,NULL);
	if (plugin)
	{
		FRostersViewPlugin = qobject_cast<IRostersViewPlugin *>(plugin->instance());
		if (FRostersViewPlugin)
		{
			if (FServiceDiscovery)
				connect(FRostersViewPlugin->rostersView()->instance(),SIGNAL(indexContextMenu(QList<IRosterIndex *>, quint32, Menu *)),
					SLOT(onRosterIndexContextMenu(QList<IRosterIndex *>, quint32, Menu *)));
				connect(FRostersViewPlugin->rostersView()->instance(),
					SIGNAL(indexClipboardMenu(QList<IRosterIndex *>, quint32, Menu *)),
					SLOT(onRosterIndexClipboardMenu(QList<IRosterIndex *>, quint32, Menu *)));
				connect(FRostersViewPlugin->rostersView()->instance(),SIGNAL(indexToolTips(IRosterIndex *, quint32, QMap<int,QString> &)),
					SLOT(onRostersViewIndexToolTips(IRosterIndex *, quint32, QMap<int,QString> &)));
		}
	}

	return FMainWindowPlugin != NULL && FRosterManager != NULL && FPresenceManager != NULL && FXmppStreamManager != NULL;
}

bool ClientIcons::initObjects()
{
	FIconStorage = IconStorage::staticStorage(RSR_STORAGE_CLIENTICONS);

	foreach (QString key, FIconStorage->fileFirstKeys())
	{
		Client client = {FIconStorage->fileProperty(key, PROPERTY_CLIENT), FIconStorage->getIcon(key)};
		FClients.insert(key, client);
	}

	if(FRostersModel)
	{
		FRostersModel->insertRosterDataHolder(RDHO_CLIENTICONS,this);
	}

	if(FRostersViewPlugin)
	{
		AdvancedDelegateItem label(RLID_CLIENTICONS);
		label.d->kind = AdvancedDelegateItem::CustomData;
		label.d->data = RDR_CLIENTICON_IMAGE;
		FRosterLabelId = FRostersViewPlugin->rostersView()->registerLabel(label);
		FRostersViewPlugin->rostersView()->insertLabelHolder(RLHO_CLIENTICONS, this);
		FRostersViewPlugin->rostersView()->insertClickHooker(RCHO_CLIENTICONS, this);
	}

	if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);

	if (FClientInfo)
		Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_CHAT_SHOWSOFTWAREVERSION, tr("Show software version"), tr("F3", "Show software version (chat)"), Shortcuts::WindowShortcut);

	return true;
}

bool ClientIcons::initSettings()
{
	Options::setDefaultValue(OPV_ROSTER_CLIENTICON_SHOW, true);	
	Options::setDefaultValue(OPV_MUC_CLIENTICON_SHOW, true);
	Options::setDefaultValue(OPV_MESSAGES_CLIENTICON_DISPLAY, true);

	return true;
}

QMultiMap<int, IOptionsDialogWidget *> ClientIcons::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_ROSTERVIEW)
	{
		if (Options::node(OPV_COMMON_ADVANCED).value().toBool())
			widgets.insertMulti(OWO_ROSTER_CLIENTICONS, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ROSTER_CLIENTICON_SHOW), tr("Show client icons"), NULL));
	}
	if (ANodeId == OPN_CONFERENCES)
	{
		if (Options::node(OPV_COMMON_ADVANCED).value().toBool())
			widgets.insertMulti(OWO_MUC_CLIENTICONS, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MUC_CLIENTICON_SHOW), tr("Show client icons"), NULL));
	}
	else if (ANodeId == OPN_MESSAGES)
		widgets.insertMulti(OWO_MESSAGES_INFOBAR_CLIENTICONS, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MESSAGES_CLIENTICON_DISPLAY), tr("Display client icon"), AParent));
	return widgets;
}

bool ClientIcons::stanzaReadWrite(int AHandlerId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{	
	Q_UNUSED(AAccept);
	if (FSHIPresence.value(AStreamJid) == AHandlerId)
	{
		Jid contactJid = AStanza.from();
		QString node = AStanza.firstElement("c",NS_CAPS).attribute("node");
		if (FContacts.value(contactJid) != node)
		{
			if (!node.isEmpty())
				FContacts.insert(contactJid, node);
			else
				FContacts.remove(contactJid);
			updateDataHolder(AStreamJid, contactJid);
		}
	}
	return false;
}

QList<quint32> ClientIcons::rosterLabels(int AOrder, const IRosterIndex *AIndex) const
{
	QList<quint32> labels;
	if (AOrder==RLHO_CLIENTICONS && AIndex->kind()==RIK_RECENT_ITEM)
		if (FSimpleContactsView)
			labels.append(RLID_CLIENTICONS);
	return labels;
}

AdvancedDelegateItem ClientIcons::rosterLabel(int AOrder, quint32 ALabelId, const IRosterIndex *AIndex) const
{
	Q_UNUSED(AOrder); Q_UNUSED(ALabelId); Q_UNUSED(AIndex);
	static const AdvancedDelegateItem null = AdvancedDelegateItem();
	return null;
}

QList<int> ClientIcons::rosterDataRoles(int AOrder) const
{
	if (AOrder == RDHO_CLIENTICONS)
		return QList<int>() << RDR_CLIENTICON_IMAGE;
	return QList<int>();
}

QVariant ClientIcons::rosterData(int AOrder, const IRosterIndex *AIndex, int ARole) const
{
	Q_UNUSED(AOrder)
	return ARole == RDR_CLIENTICON_IMAGE && (FRosterIndexKinds.contains(AIndex->kind()))?QIcon(contactIcon(AIndex->data(RDR_PREP_FULL_JID).toString())):QVariant();
}

bool ClientIcons::setRosterData(int AOrder, const QVariant &AValue, IRosterIndex *AIndex, int ARole)
{
	Q_UNUSED(AOrder);
	Q_UNUSED(AIndex);
	Q_UNUSED(ARole);
	Q_UNUSED(AValue);
	return false;
}

bool ClientIcons::rosterIndexSingleClicked(int AOrder, IRosterIndex *AIndex, const QMouseEvent *AEvent)
{
	Q_UNUSED(AOrder)

	QModelIndex index = FRostersViewPlugin->rostersView()->mapFromModel(FRostersViewPlugin->rostersView()->rostersModel()->modelIndexFromRosterIndex(AIndex));
	if (FRostersViewPlugin->rostersView()->labelAt(AEvent->pos(),index) == FRosterLabelId)
	{
		FClientInfo->showClientInfo(AIndex->data(RDR_STREAM_JID).toString(), AIndex->data(RDR_FULL_JID).toString(), IClientInfo::SoftwareVersion);
		return true;
	}
	return false;
}

QList<int> ClientIcons::advancedItemDataRoles(int AOrder) const
{
	if (AOrder == MUDHO_CLIENTICONS)
	{
		static const QList<int> roles = QList<int>() << MUDR_CLIENT_ICON;
		return roles;
	}
	return QList<int>();
}

QVariant ClientIcons::advancedItemData(int AOrder, const QStandardItem *AItem, int ARole) const
{
	if (AOrder == MUDHO_CLIENTICONS)
		if (ARole == MUDR_CLIENT_ICON)
			return contactIcon(Jid(AItem->data(MUDR_USER_JID).toString()));
	return QVariant();
}

void ClientIcons::onRostersViewIndexToolTips(IRosterIndex *AIndex, quint32 ALabelId, QMap<int, QString> &AToolTips)
{
	if ((ALabelId==AdvancedDelegateItem::DisplayId || ALabelId == FRosterLabelId) && FRosterIndexKinds.contains(AIndex->kind()))
	{
		QStringList resources=AIndex->data(RDR_RESOURCES).toStringList();
		if (resources.isEmpty())
			resources.append(AIndex->data(RDR_FULL_JID).toString());
		int order=RTTO_ROSTERSVIEW_CLIENTICONS;
		for (QStringList::const_iterator it=resources.constBegin(); it!=resources.constEnd(); it++, order+=100)
			if (!contactClient(*it).isEmpty())
				AToolTips.insert(order, QString("<b>%1</b> %2").arg(tr("Client:")).arg(contactClient(*it)));
	}
}

void ClientIcons::onRosterIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
	if (ALabelId == FRosterLabelId && AIndexes.size()==1)
	{
		Jid streamJid = AIndexes.first()->data(RDR_STREAM_JID).toString();
		Jid contactJid = AIndexes.first()->data(RDR_FULL_JID).toString();
		IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(streamJid) : NULL;
		QStringList resources = AIndexes.first()->data(RDR_RESOURCES).toStringList();
		if (resources.isEmpty())
			resources.append(contactJid.pFull());
		bool multiResorces = resources.count()>1;

		QMap<QString, Menu *> resMenu;
		for (QStringList::ConstIterator it=resources.constBegin(); it!=resources.constEnd(); it++)
		{
			Jid itemJid(*it);
			IDiscoInfo dinfo = FServiceDiscovery->discoInfo(streamJid, itemJid);
			IRosterItem ritem = roster!=NULL ? roster->findItem(itemJid) : IRosterItem();
			QString resName = (!ritem.name.isEmpty() ? ritem.name : itemJid.uBare()) + (!itemJid.resource().isEmpty() ? QString("/")+itemJid.resource() : QString::null);
//TODO: Check, if it's really necessary
			// Many clients support version info but don`t show it in disco info
			if (dinfo.streamJid.isValid() && !dinfo.features.contains(NS_JABBER_VERSION))
				dinfo.features.append(NS_JABBER_VERSION);

			Action *action = FServiceDiscovery->createFeatureAction(streamJid, NS_JABBER_VERSION, dinfo, AMenu);
			if (action)
			{
				if (multiResorces)
				{
					Menu *menu = resMenu.value(action->text());
					if (menu == NULL)
					{
						menu = new Menu(AMenu);
						menu->setIcon(action->icon());
						menu->setTitle(action->text());
						resMenu.insert(action->text(),menu);
						AMenu->addAction(menu->menuAction(),AG_RVCM_DISCOVERY_FEATURES,true);
					}
					action->setText(resName);
					action->setParent(action->parent()==AMenu ? menu : action->parent());
					action->setIcon(contactIcon(itemJid));
					menu->addAction(action,AG_RVCM_DISCOVERY_FEATURES,false);
				}
				else
					AMenu->addAction(action,AG_RVCM_DISCOVERY_FEATURES,true);
			}
		}
	}
}

void ClientIcons::onRosterIndexClipboardMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
	if (ALabelId == AdvancedDelegateItem::DisplayId || ALabelId == FRosterLabelId)
		for (QList<IRosterIndex *>::const_iterator it=AIndexes.constBegin(); it!=AIndexes.constEnd(); it++)
		{
			Jid jid((*it)->data(RDR_FULL_JID).toString());
			if (FContacts.contains(jid))
			{
				QString key = FContacts[jid];
				if (FClients.contains(key))
				{
					Client client = FClients[key];
					Action *action = new Action(AMenu);
					action->setText(client.name);
					action->setIcon(client.icon);
					action->setData(ADR_CLIPBOARD_ICON, FIconStorage->fileFullName(key));
					connect(action, SIGNAL(triggered()), SLOT(onCopyToClipboard()));
					AMenu->addAction(action, AG_RVCBM_PEP, true);
				}
			}
		}
}

void ClientIcons::onCopyToClipboard()
{
	Action *action = qobject_cast<Action *>(sender());
	QString fileName = action->data(ADR_CLIPBOARD_ICON).toString();
	QString text = action->text();
	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *mime = new QMimeData();

	mime->setText(text);

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
				mime->setHtml(QString("<body><img src=\"%1\" alt=\"%2\" title=\"%2\" /> %2</body>").arg(url.toString()).arg(text));
			}
		}
	}

	clipboard->setMimeData(mime);
}

void ClientIcons::onMucItemToolTips(QStandardItem *AItem, QMap<int, QString> &AToolTips)
{
	Jid userJid(AItem->data(MUDR_USER_JID).toString());
	QString client = contactClient(userJid);
	if (!client.isEmpty())
		AToolTips.insert(MUTTO_MULTIUSERCHAT_CLIENT, QString("<b>%1</b> %2").arg(tr("Client:")).arg(client));
}

void ClientIcons::updateDataHolder(const Jid &streamJid, const Jid &clientJid)
{
	if(FRostersModel)
	{
		QMultiMap<int, QVariant> findData;
		if (!streamJid.isEmpty())
			findData.insert(RDR_STREAM_JID,streamJid.pFull());
		if (!clientJid.isEmpty())
			findData.insert(RDR_PREP_BARE_JID,clientJid.pBare());
		for(QList<int>::const_iterator it=FRosterIndexKinds.constBegin(); it!=FRosterIndexKinds.constEnd(); it++)
			findData.insert(RDR_KIND, *it);

		QList<IRosterIndex *> indexes = FRostersModel->rootIndex()->findChilds(findData, true);
		for (QList<IRosterIndex *>::ConstIterator it = indexes.constBegin(); it!=indexes.constEnd(); it++)
			emit rosterDataChanged(*it, RDR_CLIENTICON_IMAGE);
	}
}

void ClientIcons::onStreamOpened(IXmppStream *AXmppStream)
{
	if (FStanzaProcessor)
	{
		IStanzaHandle shandle;
		shandle.handler = this;
		shandle.streamJid = AXmppStream->streamJid();
		shandle.order = SHO_PI_CLIENTICONS;
		shandle.direction = IStanzaHandle::DirectionIn;
		shandle.conditions.append(SHC_PRESENCE);
		FSHIPresence.insert(shandle.streamJid,FStanzaProcessor->insertStanzaHandle(shandle));
	}
}

void ClientIcons::onStreamClosed(IXmppStream *AXmppStream)
{
	if (FStanzaProcessor)
	{
		FStanzaProcessor->removeStanzaHandle(FSHIPresence.take(AXmppStream->streamJid()));
	}
}

void ClientIcons::onContactStateChanged(const Jid &streamJid, const Jid &contactJid, bool AStateOnline)
{
	if (!AStateOnline && FContacts.contains(contactJid))
	{
		FContacts.remove(contactJid);
		updateDataHolder(streamJid, contactJid);
	}
}

void ClientIcons::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_ROSTER_RECENT_SIMPLEITEMSVIEW));
	onOptionsChanged(Options::node(OPV_ROSTER_CLIENTICON_SHOW));
	onOptionsChanged(Options::node(OPV_MESSAGES_CLIENTICON_DISPLAY));
}

void ClientIcons::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_MESSAGES_CLIENTICON_DISPLAY)
		updateChatWindows();
	else if (ANode.path() == OPV_ROSTER_RECENT_SIMPLEITEMSVIEW)
	{
		FSimpleContactsView = ANode.value().toBool();
		QMultiMap<int, QVariant> findData;
		findData.insert(RDR_KIND, RIK_RECENT_ITEM);
		QList<IRosterIndex *> indexes = FRostersModel->rootIndex()->findChilds(findData, true);
		for (QList<IRosterIndex *>::ConstIterator it = indexes.constBegin(); it!=indexes.constEnd(); it++)
			emit rosterLabelChanged(RLID_CLIENTICONS, *it);
	}
	else if (ANode.path() == (Options::node(OPV_COMMON_ADVANCED).value().toBool()?OPV_ROSTER_CLIENTICON_SHOW:OPV_ROSTER_VIEWMODE))
	{
		if (FRostersViewPlugin && FRostersModel)
		{
			if (ANode.path() == OPV_ROSTER_CLIENTICON_SHOW?ANode.value().toBool():ANode.value().toInt() == IRostersView::ViewFull)
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
	else if (ANode.path() == OPV_MUC_CLIENTICON_SHOW)
	{
		QList<IMultiUserChatWindow *> windows = FMultiUserChatManager->multiChatWindows();
		for (QList<IMultiUserChatWindow *>::ConstIterator it = windows.constBegin(); it != windows.constEnd(); ++it)
			displayMucLabels((*it)->multiUserView(), ANode.value().toBool());
	}
}

void ClientIcons::onSoftwareVersionActionTriggered()
{
	Action *action=qobject_cast<Action *>(sender());
	FClientInfo->showClientInfo(action->data(ADR_STREAM_JID).toString(), action->data(ADR_CONTACT_JID).toString(), IClientInfo::SoftwareVersion);
}

void ClientIcons::displayMucLabels(IMultiUserView *AView, bool ADisplay)
{
	if (ADisplay)
	{
		AdvancedDelegateItem label;
		label.d->id = MUIL_MULTIUSERCHAT_CLIENTICON;
		label.d->kind = AdvancedDelegateItem::CustomData;
		label.d->data = MUDR_CLIENT_ICON;
		AView->insertGeneralLabel(label);
	}
	else
		AView->removeGeneralLabel(MUIL_MULTIUSERCHAT_CLIENTICON);
}

void ClientIcons::onViewModeChanged(int AMode)
{
	displayMucLabels(qobject_cast<IMultiUserView *>(sender()), AMode == IMultiUserView::ViewFull);
}

void ClientIcons::updateChatWindows()
{
	if (FMessageWidgets)
	{
		QList<IMessageChatWindow *>chatWindows=FMessageWidgets->chatWindows();
		for(QList<IMessageChatWindow *>::const_iterator it=chatWindows.constBegin(); it!=chatWindows.constEnd(); it++)
			updateChatWindow(*it);
	}
}

void ClientIcons::updateChatWindows(const Jid &AContactJid, const Jid &AStreamJid)
{
	if (FMessageWidgets)
	{
		QList<IMessageChatWindow *>chatWindows=FMessageWidgets->chatWindows();
		for(QList<IMessageChatWindow *>::const_iterator it=chatWindows.constBegin(); it!=chatWindows.constEnd(); it++)
			if((AContactJid.bare() == (*it)->contactJid().bare())  &&
			   (AStreamJid.bare()  == (*it)->streamJid().bare()))
				updateChatWindow(*it);
	}
}

void ClientIcons::updateChatWindow(IMessageChatWindow *AMessageChatWindow)
{
	if (Options::node(OPV_MESSAGES_CLIENTICON_DISPLAY).value().toBool() && FClients.contains(FContacts[AMessageChatWindow->contactJid()]))
	{
		Action *action=NULL;
		QList<QAction *> actions=AMessageChatWindow->infoWidget()->infoToolBarChanger()->groupItems(AG_CLIENTICONS);
		if (actions.isEmpty())
		{
			action=new Action(AMessageChatWindow->infoWidget()->infoToolBarChanger()->toolBar());
			AMessageChatWindow->infoWidget()->infoToolBarChanger()->insertAction(action, AG_CLIENTICONS);
		}
		else
			action=AMessageChatWindow->infoWidget()->infoToolBarChanger()->handleAction(actions[0]);
		action->setIcon(contactIcon(AMessageChatWindow->contactJid()));
		action->setToolTip(QString("<b>%1</b> %2").arg(tr("Client:")).arg(contactClient(AMessageChatWindow->contactJid())));
		action->setData(ADR_CONTACT_JID, AMessageChatWindow->contactJid().full());
		action->setData(ADR_STREAM_JID, AMessageChatWindow->streamJid().full());
		if (FClientInfo)
		{
			connect(action, SIGNAL(triggered()), SLOT(onSoftwareVersionActionTriggered()));
			action->setShortcutId(SCT_MESSAGEWINDOWS_CHAT_SHOWSOFTWAREVERSION);
		}
	}
	else
	{
		QList<QAction *> actions=AMessageChatWindow->infoWidget()->infoToolBarChanger()->groupItems(AG_CLIENTICONS);
		if (!actions.isEmpty())
		{
			AMessageChatWindow->infoWidget()->infoToolBarChanger()->removeItem(actions[0]);
			actions[0]->deleteLater();
		}
	}
}

QIcon ClientIcons::iconByKey(const QString &key) const
{
	return FClients.value(key).icon;
}

QString ClientIcons::clientByKey(const QString &key) const
{
	return FClients.value(key).name;
}

QString ClientIcons::contactClient(const Jid &contactJid) const
{
	return FClients.value(FContacts.value(contactJid)).name;
}

QIcon ClientIcons::contactIcon(const Jid &contactJid) const
{
	return FClients.value(FContacts.value(contactJid)).icon;
}

void ClientIcons::onChatWindowCreated(IMessageChatWindow *AWindow)
{
	updateChatWindow(AWindow);
	connect(AWindow->address()->instance(), SIGNAL(addressChanged(Jid, Jid)), SLOT(onAddressChanged(Jid,Jid)));
}

void ClientIcons::onMultiChatWindowCreated(IMultiUserChatWindow *AWindow)
{
	AWindow->multiUserView()->model()->insertItemDataHolder(MUDHO_CLIENTICONS, this);
	displayMucLabels(AWindow->multiUserView(),
					 Options::node(OPV_COMMON_ADVANCED).value().toBool()
						?Options::node(OPV_MUC_CLIENTICON_SHOW).value().toBool()
						:AWindow->multiUserView()->viewMode()== IMultiUserView::ViewFull);
	connect(AWindow->multiUserView()->instance(), SIGNAL(viewModeChanged(int)),
												  SLOT(onViewModeChanged(int)));
	connect(AWindow->multiUserView()->instance(), SIGNAL(itemToolTips(QStandardItem*,QMap<int,QString>&)),
												  SLOT(onMucItemToolTips(QStandardItem*,QMap<int,QString>&)));
}

void ClientIcons::onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore)
{
	Q_UNUSED(AStreamBefore)
	Q_UNUSED(AContactBefore)

	IMessageAddress *address=qobject_cast<IMessageAddress *>(sender());
	IMessageChatWindow *window=FMessageWidgets->findChatWindow(address->streamJid(), address->contactJid());
	if (window)
		updateChatWindow(window);
}

void ClientIcons::onRosterIndexInserted(IRosterIndex *AIndex)
{
	if (FRostersViewPlugin && FRosterIndexKinds.contains(AIndex->kind()) &&
		(Options::node(OPV_COMMON_ADVANCED).value().toBool()?Options::node(OPV_ROSTER_CLIENTICON_SHOW).value().toBool():Options::node(OPV_ROSTER_VIEWMODE).value().toInt()==IRostersView::ViewFull))
		FRostersViewPlugin->rostersView()->insertLabel(FRosterLabelId, AIndex);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_clienticons, ClientIcons)
#endif
