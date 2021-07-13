#include "rosteritemexchange.h"
#include <QMimeData>
#include <QDropEvent>
#include <QDataStream>
#include <QMessageBox>
#include <QTextDocument>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <definitions/namespaces.h>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/soundfiles.h>
#include <definitions/optionnodes.h>
#include <definitions/optionvalues.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/notificationtypes.h>
#include <definitions/notificationdataroles.h>
#include <definitions/notificationtypeorders.h>
#include <definitions/rosterindexkinds.h>
#include <definitions/rosterindexroles.h>
#include <definitions/rosternotifyorders.h>
#include <definitions/stanzahandlerorders.h>
#include <definitions/rosterdragdropmimetypes.h>
#include <utils/widgetmanager.h>
#include <utils/iconstorage.h>
#include <utils/message.h>
#include <utils/options.h>
#include <utils/logger.h>

#define ADR_STREAM_JID         Action::DR_StreamJid
#define ADR_CONTACT_JID        Action::DR_Parametr1
#define ADR_ITEMS_JIDS         Action::DR_Parametr2
#define ADR_ITEMS_NAMES        Action::DR_Parametr3
#define ADR_ITEMS_GROUPS       Action::DR_Parametr4

#define SHC_ROSTERX_IQ         "/iq/x[@xmlns='" NS_ROSTERX "']"
#define SHC_ROSTERX_MESSAGE    "/message/x[@xmlns='" NS_ROSTERX "']"

static const QList<int> DragRosterKinds = QList<int>() << RIK_CONTACT << RIK_AGENT << RIK_GROUP << RIK_METACONTACT << RIK_METACONTACT_ITEM;

RosterItemExchange::RosterItemExchange()
{
	FGateways = NULL;
	FRosterManager = NULL;
	FRosterChanger = NULL;
	FPresenceManager = NULL;
	FDiscovery = NULL;
	FStanzaProcessor = NULL;
	FOptionsManager = NULL;
	FNotifications = NULL;
	FMessageWidgets = NULL;
	FRostersViewPlugin = NULL;

	FSHIExchangeRequest = -1;
}

RosterItemExchange::~RosterItemExchange()
{

}

void RosterItemExchange::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Roster Item Exchange");
	APluginInfo->description = tr("Allows to exchange contact list items");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Potapov S.A. aka Lion";
	APluginInfo->homePage = "http://www.vacuum-im.org";
	APluginInfo->dependences.append(ROSTER_UUID);
	APluginInfo->dependences.append(STANZAPROCESSOR_UUID);
}

bool RosterItemExchange::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);
	IPlugin *plugin = APluginManager->pluginInterface("IRosterManager").value(0,NULL);
	if (plugin)
	{
		FRosterManager = qobject_cast<IRosterManager *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0,NULL);
	if (plugin)
	{
		FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IRosterChanger").value(0,NULL);
	if (plugin)
	{
		FRosterChanger = qobject_cast<IRosterChanger *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0,NULL);
	if (plugin)
	{
		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
	{
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
	{
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("INotifications").value(0,NULL);
	if (plugin)
	{
		FNotifications = qobject_cast<INotifications *>(plugin->instance());
		if (FNotifications)
		{
			connect(FNotifications->instance(),SIGNAL(notificationActivated(int)), SLOT(onNotificationActivated(int)));
			connect(FNotifications->instance(),SIGNAL(notificationRemoved(int)), SLOT(onNotificationRemoved(int)));
		}
	}

	plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IRostersViewPlugin").value(0,NULL);
	if (plugin)
	{
		FRostersViewPlugin = qobject_cast<IRostersViewPlugin *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IGateways").value(0,NULL);
	if (plugin)
	{
		FGateways = qobject_cast<IGateways *>(plugin->instance());
	}

	return FRosterManager!=NULL && FStanzaProcessor!=NULL;
}

bool RosterItemExchange::initObjects()
{
	if (FDiscovery)
	{
		IDiscoFeature feature;
		feature.var = NS_ROSTERX;
		feature.active = true;
		feature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_ROSTEREXCHANGE_REQUEST);
		feature.name = tr("Roster Item Exchange");
		feature.description = tr("Supports the exchanging of contact list items");
		FDiscovery->insertDiscoFeature(feature);
	}

	if (FStanzaProcessor)
	{
		IStanzaHandle handle;
		handle.handler = this;
		handle.order = SHO_IMI_ROSTEREXCHANGE;
		handle.direction = IStanzaHandle::DirectionIn;
		handle.conditions.append(SHC_ROSTERX_IQ);
		handle.conditions.append(SHC_ROSTERX_MESSAGE);
		FSHIExchangeRequest = FStanzaProcessor->insertStanzaHandle(handle);
	}

	if (FOptionsManager)
	{
		FOptionsManager->insertOptionsDialogHolder(this);
	}

	if (FNotifications)
	{
		INotificationType notifyType;
		notifyType.order = NTO_ROSTEREXCHANGE_REQUEST;
		notifyType.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_ROSTEREXCHANGE_REQUEST);
		notifyType.title = tr("When receiving roster modification request");
		notifyType.kindMask = INotification::RosterNotify|INotification::TrayNotify|INotification::TrayAction|INotification::PopupWindow|INotification::SoundPlay|INotification::AlertWidget|INotification::ShowMinimized|INotification::AutoActivate;
		notifyType.kindDefs = notifyType.kindMask & ~(INotification::AutoActivate);
		FNotifications->registerNotificationType(NNT_ROSTEREXCHANGE_REQUEST,notifyType);
	}

	if (FMessageWidgets)
	{
		FMessageWidgets->insertViewDropHandler(this);
	}

	if (FRostersViewPlugin)
	{
		FRostersViewPlugin->rostersView()->insertDragDropHandler(this);
	}

	return true;
}

bool RosterItemExchange::initSettings()
{
	Options::setDefaultValue(OPV_ROSTER_EXCHANGE_AUTOAPPROVEENABLED,true);
	return true;
}

bool RosterItemExchange::stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	if (FSHIExchangeRequest==AHandleId && !AStanza.isError())
	{
		QDomElement xElem = AStanza.firstElement("x",NS_ROSTERX);
		if (!xElem.isNull() && !xElem.firstChildElement("item").isNull())
		{
			AAccept = true;

			LOG_STRM_INFO(AStreamJid,QString("Roster exchange request received, from=%1, kind=%2, id=%3").arg(AStanza.from(),AStanza.kind(),AStanza.id()));

			IRosterExchangeRequest request;
			request.streamJid = AStreamJid;
			request.contactJid = AStanza.from();
			request.id = AStanza.kind()==STANZA_KIND_IQ ? AStanza.id() : QString();
			request.message = AStanza.kind()==STANZA_KIND_MESSAGE ? Message(AStanza).body() : QString();

			QList<Jid> existItems;
			QDomElement itemElem = xElem.firstChildElement("item");

			bool isItemsValid = true;
			while (isItemsValid && !itemElem.isNull())
			{
				IRosterExchangeItem item;
				item.itemJid = Jid(itemElem.attribute("jid")).bare();
				item.name = itemElem.attribute("name");
				item.action = itemElem.attribute("action",ROSTEREXCHANGE_ACTION_ADD);

				QDomElement groupElem = itemElem.firstChildElement("group");
				while(!groupElem.isNull())
				{
					item.groups += groupElem.text();
					groupElem = groupElem.nextSiblingElement("group");
				}

				if (item.itemJid.isValid() && !existItems.contains(item.itemJid) &&
					(item.action==ROSTEREXCHANGE_ACTION_ADD || item.action==ROSTEREXCHANGE_ACTION_DELETE || item.action==ROSTEREXCHANGE_ACTION_MODIFY))
				{
					request.items.append(item);
					existItems.append(item.itemJid);
				}
				else
				{
					isItemsValid = false;
					LOG_STRM_WARNING(AStreamJid,QString("Failed to append roster exchange item, jid=%1, action=%2: Invalid item").arg(item.itemJid.bare(),item.action));
				}

				itemElem = itemElem.nextSiblingElement("item");
			}

			if (isItemsValid && !request.items.isEmpty())
				processRequest(request);
			else
				replyRequestError(request,XmppStanzaError::EC_BAD_REQUEST);

			return true;
		}
	}
	return false;
}

void RosterItemExchange::stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza)
{
	Q_UNUSED(AStreamJid);
	if (FSentRequests.contains(AStanza.id()))
	{
		IRosterExchangeRequest request = FSentRequests.take(AStanza.id());
		if (AStanza.isResult())
		{
			LOG_STRM_INFO(AStreamJid,QString("Roster exchange request accepted by=%1, id=%2").arg(AStanza.from(),AStanza.id()));
			emit exchangeRequestApproved(request);
		}
		else
		{
			XmppStanzaError err(AStanza);
			LOG_STRM_WARNING(AStreamJid,QString("Roster exchange request rejected by=%1, id=%2: %3").arg(AStanza.from(),AStanza.id(),err.condition()));
			emit exchangeRequestFailed(request,err);
		}
	}
}

QMultiMap<int, IOptionsDialogWidget *> RosterItemExchange::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_ROSTERVIEW)
	{
		widgets.insertMulti(OHO_ROSTER_MANAGEMENT,FOptionsManager->newOptionsDialogHeader(tr("Contacts list management"),AParent));
		widgets.insertMulti(OWO_ROSTER_EXCHANGEAUTO,FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ROSTER_EXCHANGE_AUTOAPPROVEENABLED),tr("Allow gateways and group services manage your contacts list"),AParent));
	}
	return widgets;
}

bool RosterItemExchange::messageViewDragEnter(IMessageViewWidget *AWidget, const QDragEnterEvent *AEvent)
{
	return !dropDataContacts(AWidget->messageWindow()->streamJid(),AWidget->messageWindow()->contactJid(),AEvent->mimeData()).isEmpty();
}

bool RosterItemExchange::messageViewDragMove(IMessageViewWidget *AWidget, const QDragMoveEvent *AEvent)
{
	Q_UNUSED(AWidget); Q_UNUSED(AEvent);
	return true;
}

void RosterItemExchange::messageViewDragLeave(IMessageViewWidget *AWidget, const QDragLeaveEvent *AEvent)
{
	Q_UNUSED(AWidget); Q_UNUSED(AEvent);
}

bool RosterItemExchange::messageViewDropAction(IMessageViewWidget *AWidget, const QDropEvent *AEvent, Menu *AMenu)
{
	return AEvent->dropAction()!=Qt::IgnoreAction ? insertDropActions(AWidget->messageWindow()->streamJid(),AWidget->messageWindow()->contactJid(),AEvent->mimeData(),AMenu) : false;
}

Qt::DropActions RosterItemExchange::rosterDragStart(const QMouseEvent *AEvent, IRosterIndex *AIndex, QDrag *ADrag)
{
	Q_UNUSED(AEvent); Q_UNUSED(ADrag);
	if (DragRosterKinds.contains(AIndex->kind()))
		return Qt::CopyAction|Qt::MoveAction;
	return Qt::IgnoreAction;
}

bool RosterItemExchange::rosterDragEnter(const QDragEnterEvent *AEvent)
{
	if (AEvent->source()==FRostersViewPlugin->rostersView()->instance() && AEvent->mimeData()->hasFormat(DDT_ROSTERSVIEW_INDEX_DATA))
	{
		QMap<int, QVariant> indexData;
		QDataStream stream(AEvent->mimeData()->data(DDT_ROSTERSVIEW_INDEX_DATA));
		operator>>(stream,indexData);

		if (DragRosterKinds.contains(indexData.value(RDR_KIND).toInt()))
		{
			Jid indexJid = indexData.value(RDR_PREP_BARE_JID).toString();
			if (indexJid.hasNode())
			{
				QList<Jid> services = FGateways!=NULL ? FGateways->streamServices(indexData.value(RDR_STREAM_JID).toString()) : QList<Jid>();
				return !services.contains(indexJid.domain());
			}
			return true;
		}
	}
	return false;
}

bool RosterItemExchange::rosterDragMove(const QDragMoveEvent *AEvent, IRosterIndex *AHover)
{
	return !dropDataContacts(AHover->data(RDR_STREAM_JID).toString(),AHover->data(RDR_FULL_JID).toString(),AEvent->mimeData()).isEmpty();
}

void RosterItemExchange::rosterDragLeave(const QDragLeaveEvent *AEvent)
{
	Q_UNUSED(AEvent);
}

bool RosterItemExchange::rosterDropAction(const QDropEvent *AEvent, IRosterIndex *AHover, Menu *AMenu)
{
	if (AEvent->dropAction() != Qt::IgnoreAction)
		return insertDropActions(AHover->data(RDR_STREAM_JID).toString(),AHover->data(RDR_FULL_JID).toString(),AEvent->mimeData(),AMenu);
	return false;
}

bool RosterItemExchange::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
	return FDiscovery!=NULL && FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_ROSTERX);
}

QString RosterItemExchange::sendExchangeRequest(const IRosterExchangeRequest &ARequest, bool AIqQuery)
{
	if (FStanzaProcessor && ARequest.streamJid.isValid() && ARequest.contactJid.isValid() && !ARequest.items.isEmpty())
	{
		Stanza request(AIqQuery ? STANZA_KIND_IQ : STANZA_KIND_MESSAGE);
		request.setTo(ARequest.contactJid.full()).setUniqueId();

		if (AIqQuery)
			request.setType(STANZA_TYPE_SET);
		else if (!ARequest.message.isEmpty())
			request.addElement("body").appendChild(request.createTextNode(ARequest.message));

		bool isItemsValid = !ARequest.items.isEmpty();
		QDomElement xElem = request.addElement("x",NS_ROSTERX);
		for (QList<IRosterExchangeItem>::const_iterator it=ARequest.items.constBegin(); it!=ARequest.items.constEnd(); ++it)
		{
			if (it->itemJid.isValid() && (it->action==ROSTEREXCHANGE_ACTION_ADD || it->action==ROSTEREXCHANGE_ACTION_DELETE || it->action==ROSTEREXCHANGE_ACTION_MODIFY))
			{
				QDomElement itemElem = xElem.appendChild(request.createElement("item")).toElement();
				itemElem.setAttribute("action",it->action);
				itemElem.setAttribute("jid",it->itemJid.full());
				if (!it->name.isEmpty())
					itemElem.setAttribute("name",it->name);
				foreach(const QString &group, it->groups)
				{
					if (!group.isEmpty())
						itemElem.appendChild(request.createElement("group")).appendChild(request.createTextNode(group));
				}
			}
			else
			{
				isItemsValid = false;
				LOG_STRM_ERROR(ARequest.streamJid,QString("Failed to send roster exchange item, jid=%1, action=%2: Invalid item").arg(it->itemJid.bare(),it->action));
			}
		}

		if (isItemsValid)
		{
			IRosterExchangeRequest sentRequest = ARequest;
			sentRequest.id = request.id();

			if (AIqQuery && FStanzaProcessor->sendStanzaRequest(this,ARequest.streamJid,request,0))
			{
				LOG_STRM_INFO(ARequest.streamJid,QString("Roster exchange request sent (iq), to=%1, count=%2, id=%3").arg(ARequest.contactJid.full()).arg(ARequest.items.count()).arg(request.id()));
				FSentRequests.insert(sentRequest.id,sentRequest);
				emit exchangeRequestSent(sentRequest);
				return request.id();
			}
			else if (!AIqQuery && FStanzaProcessor->sendStanzaOut(ARequest.streamJid,request))
			{
				LOG_STRM_INFO(ARequest.streamJid,QString("Roster exchange request sent (message), to=%1, count=%2, id=%3").arg(ARequest.contactJid.full()).arg(ARequest.items.count()).arg(request.id()));
				emit exchangeRequestSent(sentRequest);
				return request.id();
			}
			else
			{
				LOG_STRM_WARNING(ARequest.streamJid,QString("Failed to send roster exchange request, to=%1").arg(ARequest.contactJid.full()));
			}
		}
	}
	return QString();
}

QList<IRosterItem> RosterItemExchange::dragDataContacts(const QMimeData *AData) const
{
	QList<IRosterItem> contactList;
	if (AData->hasFormat(DDT_ROSTERSVIEW_INDEX_DATA))
	{
		QMap<int, QVariant> indexData;
		QDataStream stream(AData->data(DDT_ROSTERSVIEW_INDEX_DATA));
		operator>>(stream,indexData);

		int indexKind = indexData.value(RDR_KIND).toInt();
		if (DragRosterKinds.contains(indexKind))
		{
			if (indexKind == RIK_GROUP)
			{
				QList<Jid> totalContacts;
				foreach(const Jid &streamJid, indexData.value(RDR_STREAMS).toStringList())
				{
					IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(streamJid) : NULL;
					QList<IRosterItem> ritems = roster!=NULL ? roster->groupItems(indexData.value(RDR_GROUP).toString()) : QList<IRosterItem>();
					for (QList<IRosterItem>::iterator it = ritems.begin(); it!=ritems.end(); ++it)
					{
						if (!totalContacts.contains(it->itemJid))
						{
							it->groups.clear();
							it->groups += indexData.value(RDR_NAME).toString();
							contactList.append(*it);
							totalContacts.append(it->itemJid);
						}
					}
				}
			}
			else
			{
				IRosterItem ritem;
				ritem.itemJid = indexData.value(RDR_PREP_BARE_JID).toString();
				ritem.name = indexData.value(RDR_NAME).toString();
				contactList.append(ritem);
			}

			if (!contactList.isEmpty())
			{
				QList<Jid> services = FGateways!=NULL ? FGateways->streamServices(indexData.value(RDR_STREAM_JID).toString()) : QList<Jid>();
				for (QList<IRosterItem>::iterator it=contactList.begin(); it!=contactList.end(); )
				{
					if (it->itemJid.hasNode() && services.contains(it->itemJid.domain()))
						it = contactList.erase(it);
					else
						++it;
				}
			}
		}
	}
	return contactList;
}

QList<IRosterItem> RosterItemExchange::dropDataContacts(const Jid &AStreamJid, const Jid &AContactJid, const QMimeData *AData) const
{
	QList<IRosterItem> contactList;
	if (isSupported(AStreamJid,AContactJid) && AData->hasFormat(DDT_ROSTERSVIEW_INDEX_DATA))
	{
		QMap<int, QVariant> indexData;
		QDataStream stream(AData->data(DDT_ROSTERSVIEW_INDEX_DATA));
		operator>>(stream,indexData);

		if (AStreamJid!=AContactJid || AStreamJid!=indexData.value(RDR_STREAM_JID).toString())
		{
			contactList = dragDataContacts(AData);
			for (QList<IRosterItem>::iterator it = contactList.begin(); it!=contactList.end(); )
			{
				if (AContactJid.pBare() == it->itemJid.pBare())
					it = contactList.erase(it);
				else
					++it;
			}
		}
	}
	return contactList;
}

bool RosterItemExchange::insertDropActions(const Jid &AStreamJid, const Jid &AContactJid, const QMimeData *AData, Menu *AMenu) const
{
	QList<IRosterItem> contactList = dropDataContacts(AStreamJid,AContactJid,AData);

	QStringList itemsJids;
	QStringList itemsNames;
	QStringList itemsGroups;
	for (QList<IRosterItem>::const_iterator it = contactList.constBegin(); it!=contactList.constEnd(); ++it)
	{
		itemsJids.append(it->itemJid.pBare());
		itemsNames.append(it->name);
		itemsGroups.append(it->groups.toList().value(0));
	}

	if (!itemsJids.isEmpty())
	{
		Action *action = new Action(AMenu);
		action->setText(tr("Send %n Contact(s)","",itemsJids.count()));
		action->setIcon(RSR_STORAGE_MENUICONS,MNI_ROSTEREXCHANGE_REQUEST);
		action->setData(ADR_STREAM_JID,AStreamJid.full());
		action->setData(ADR_CONTACT_JID,AContactJid.full());
		action->setData(ADR_ITEMS_JIDS, itemsJids);
		action->setData(ADR_ITEMS_NAMES, itemsNames);
		action->setData(ADR_ITEMS_GROUPS, itemsGroups);
		connect(action,SIGNAL(triggered()),SLOT(onSendExchangeRequestByAction()));
		AMenu->addAction(action, AG_DEFAULT, true);
		return true;
	}

	return false;
}

void RosterItemExchange::processRequest(const IRosterExchangeRequest &ARequest)
{
	IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(ARequest.streamJid) : NULL;
	if (roster && roster->hasItem(ARequest.contactJid))
	{
		bool isGateway = false;
		bool isDirectory = false;
		if (!ARequest.contactJid.hasNode())
		{
			if (!ARequest.contactJid.isEmpty() && ARequest.contactJid!=ARequest.streamJid.bare() && ARequest.contactJid!=ARequest.streamJid.domain())
			{
				isGateway = true;
				if (FDiscovery && FDiscovery->hasDiscoInfo(ARequest.contactJid,ARequest.contactJid))
				{
					IDiscoInfo dinfo = FDiscovery->discoInfo(ARequest.streamJid,ARequest.contactJid);
					isDirectory = FDiscovery->findIdentity(dinfo.identity,"directory","group")>=0;
				}
			}
			else
			{
				isDirectory = true;
			}
		}

		bool isForbidden = false;
		QList<IRosterExchangeItem> approveList;
		bool autoApprove = (isGateway || isDirectory) && Options::node(OPV_ROSTER_EXCHANGE_AUTOAPPROVEENABLED).value().toBool();
		for(QList<IRosterExchangeItem>::const_iterator it=ARequest.items.constBegin(); !isForbidden && it!=ARequest.items.constEnd(); ++it)
		{
			if (autoApprove && !isDirectory && isGateway && it->itemJid.pDomain()!=ARequest.contactJid.pDomain())
				autoApprove = false;

			IRosterItem ritem = roster->findItem(it->itemJid);
			if (!isGateway && !isDirectory && it->action!=ROSTEREXCHANGE_ACTION_ADD)
			{
				isForbidden = true;
			}
			else if (it->itemJid!=ARequest.streamJid.bare() && it->action==ROSTEREXCHANGE_ACTION_ADD)
			{
				if (ritem.isNull())
					approveList.append(*it);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
				else if (!it->groups.isEmpty() && !ritem.groups.contains(it->groups))
#else
				else if (!it->groups.isEmpty())
#endif
					approveList.append(*it);
			}
			else if (!ritem.isNull() && it->action==ROSTEREXCHANGE_ACTION_DELETE)
			{
				approveList.append(*it);
			}
			else if (!ritem.isNull() && it->action==ROSTEREXCHANGE_ACTION_MODIFY)
			{
				if (ritem.name!=it->name || ritem.groups!=it->groups)
					approveList.append(*it);
			}
		}

		if (isForbidden)
		{
			replyRequestError(ARequest,XmppStanzaError::EC_FORBIDDEN);
		}
		else if (!approveList.isEmpty())
		{
			IRosterExchangeRequest request = ARequest;
			request.items = approveList;
			emit exchangeRequestReceived(request);

			if (!autoApprove)
			{
				ExchangeApproveDialog *dialog = new ExchangeApproveDialog(roster,request);
				dialog->installEventFilter(this);
				connect(dialog,SIGNAL(accepted()),SLOT(onExchangeApproveDialogAccepted()));
				connect(dialog,SIGNAL(rejected()),SLOT(onExchangeApproveDialogRejected()));
				connect(dialog,SIGNAL(dialogDestroyed()),SLOT(onExchangeApproveDialogDestroyed()));
				notifyExchangeRequest(dialog);
			}
			else
			{
				applyRequest(request,true,true);
				replyRequestResult(request);
			}
		}
		else
		{
			replyRequestResult(ARequest);
		}
	}
	else
	{
		replyRequestError(ARequest,XmppStanzaError::EC_NOT_AUTHORIZED);
	}
}

void RosterItemExchange::notifyExchangeRequest(ExchangeApproveDialog *ADialog)
{
	if (FNotifications)
	{
		IRosterExchangeRequest request = ADialog->receivedRequest();

		INotification notify;
		notify.kinds =  FNotifications->enabledTypeNotificationKinds(NNT_ROSTEREXCHANGE_REQUEST);
		if (notify.kinds > 0)
		{
			notify.typeId = NNT_ROSTEREXCHANGE_REQUEST;
			notify.data.insert(NDR_ICON,IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_ROSTEREXCHANGE_REQUEST));
			notify.data.insert(NDR_TOOLTIP,tr("Roster modification request from %1").arg(FNotifications->contactName(request.streamJid,request.contactJid)));
			notify.data.insert(NDR_STREAM_JID,request.streamJid.full());
			notify.data.insert(NDR_CONTACT_JID,request.contactJid.full());
			notify.data.insert(NDR_ROSTER_ORDER,RNO_ROSTEREXCHANGE_REQUEST);
			notify.data.insert(NDR_ROSTER_FLAGS,IRostersNotify::Blink|IRostersNotify::AllwaysVisible|IRostersNotify::HookClicks);
			notify.data.insert(NDR_ROSTER_CREATE_INDEX,false);
			notify.data.insert(NDR_POPUP_CAPTION, tr("Roster modification"));
			notify.data.insert(NDR_POPUP_TITLE,FNotifications->contactName(request.streamJid,request.contactJid));
			notify.data.insert(NDR_POPUP_IMAGE,FNotifications->contactAvatar(request.contactJid));
			notify.data.insert(NDR_POPUP_TEXT,tr("%1 offers you to make some changes in your contact list.").arg(FNotifications->contactName(request.streamJid,request.contactJid)));
			notify.data.insert(NDR_SOUND_FILE,SDF_ROSTEREXCHANGE_REQUEST);
			notify.data.insert(NDR_ALERT_WIDGET,(qint64)ADialog);
			notify.data.insert(NDR_SHOWMINIMIZED_WIDGET,(qint64)ADialog);
			FNotifyApproveDialog.insert(FNotifications->appendNotification(notify),ADialog);
		}
		else
		{
			ADialog->reject();
		}
	}
	else
	{
		WidgetManager::showActivateRaiseWindow(ADialog);
	}
}

bool RosterItemExchange::applyRequest(const IRosterExchangeRequest &ARequest, bool ASubscribe, bool ASilent)
{
	IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(ARequest.streamJid) : NULL;
	if (roster && roster->isOpen())
	{
		LOG_STRM_INFO(ARequest.streamJid,QString("Applying roster exchange request from=%1, id=%2").arg(ARequest.contactJid.full(),ARequest.id));

		bool applied = false;
		for(QList<IRosterExchangeItem>::const_iterator it=ARequest.items.constBegin(); it!=ARequest.items.constEnd(); ++it)
		{
			IRosterItem ritem = roster->findItem(it->itemJid);
			if (it->action == ROSTEREXCHANGE_ACTION_ADD)
			{
				if (ritem.isNull())
				{
					applied = true;
					roster->setItem(it->itemJid,it->name,it->groups);
					if (ASubscribe)
					{
						if (FRosterChanger)
							FRosterChanger->subscribeContact(ARequest.streamJid,it->itemJid,QString(),ASilent);
						else
							roster->sendSubscription(it->itemJid,IRoster::Subscribe);
					}
				}
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
				else if (!it->groups.isEmpty() && !ritem.groups.contains(it->groups))
#else
				else if (!it->groups.isEmpty())
#endif
				{
					applied = true;
					roster->setItem(ritem.itemJid,ritem.name,ritem.groups+it->groups);
				}
			}
			else if (!ritem.isNull() && it->action==ROSTEREXCHANGE_ACTION_DELETE)
			{
				applied = true;
				if (!it->groups.isEmpty())
					roster->setItem(ritem.itemJid,ritem.name,ritem.groups-it->groups);
				else
					roster->removeItem(ritem.itemJid);
			}
			else if (!ritem.isNull() && it->action==ROSTEREXCHANGE_ACTION_MODIFY)
			{
				if (ritem.name!=it->name || ritem.groups!=it->groups)
				{
					applied =true;
					roster->setItem(ritem.itemJid,it->name,it->groups);
				}
			}
		}
		emit exchangeRequestApplied(ARequest);
		return applied;
	}
	return false;
}

void RosterItemExchange::replyRequestResult(const IRosterExchangeRequest &ARequest)
{
	LOG_STRM_INFO(ARequest.streamJid,QString("Roster exchange request processed, from=%1, id=%2").arg(ARequest.contactJid.full(),ARequest.id));
	if (FStanzaProcessor && !ARequest.id.isEmpty())
	{
		Stanza result(STANZA_KIND_IQ);
		result.setType(STANZA_TYPE_RESULT).setTo(ARequest.contactJid.full()).setId(ARequest.id);
		FStanzaProcessor->sendStanzaOut(ARequest.streamJid,result);
	}
	emit exchangeRequestApproved(ARequest);
}

void RosterItemExchange::replyRequestError(const IRosterExchangeRequest &ARequest, const XmppStanzaError &AError)
{
	LOG_STRM_WARNING(ARequest.streamJid,QString("Failed to process roster exchange request from=%1, id=%2: %3").arg(ARequest.contactJid.full(),ARequest.id,AError.errorMessage()));
	if (FStanzaProcessor && !ARequest.id.isEmpty())
	{
		Stanza error(STANZA_KIND_IQ);
		error.setFrom(ARequest.contactJid.full()).setId(ARequest.id);
		error = FStanzaProcessor->makeReplyError(error,AError);
		FStanzaProcessor->sendStanzaOut(ARequest.streamJid,error);
	}
	emit exchangeRequestFailed(ARequest,AError);
}

void RosterItemExchange::notifyInChatWindow(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessage) const
{
	IMessageChatWindow *window = FMessageWidgets!=NULL ? FMessageWidgets->findChatWindow(AStreamJid,AContactJid) : NULL;
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

bool RosterItemExchange::eventFilter( QObject *AObject, QEvent *AEvent )
{
	if (AEvent->type() == QEvent::WindowActivate)
	{
		if (FNotifications)
		{
			int notifyId = FNotifyApproveDialog.key(qobject_cast<ExchangeApproveDialog *>(AObject));
			FNotifications->activateNotification(notifyId);
		}
	}
	return QObject::eventFilter(AObject,AEvent);
}

void RosterItemExchange::onSendExchangeRequestByAction()
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		IRosterExchangeRequest request;
		request.streamJid = action->data(ADR_STREAM_JID).toString();
		request.contactJid = action->data(ADR_CONTACT_JID).toString();

		QStringList itemJids = action->data(ADR_ITEMS_JIDS).toStringList();
		QStringList itemNames = action->data(ADR_ITEMS_NAMES).toStringList();
		QStringList itemGroups = action->data(ADR_ITEMS_GROUPS).toStringList();
		for (int i=0; i<itemJids.count(); i++)
		{
			IRosterExchangeItem item;
			item.action = ROSTEREXCHANGE_ACTION_ADD;
			item.itemJid = itemJids.value(i);
			item.name = itemNames.value(i);
			if (!itemGroups.value(i).isEmpty())
				item.groups += itemGroups.value(i);
			request.items.append(item);
		}

		if (!sendExchangeRequest(request,false).isEmpty())
			notifyInChatWindow(request.streamJid,request.contactJid,tr("%n contact(s) sent","",request.items.count()));
		else
			notifyInChatWindow(request.streamJid,request.contactJid,tr("Failed to send %n contact(s)","",request.items.count()));
	}
}

void RosterItemExchange::onNotificationActivated(int ANotifyId)
{
	if (FNotifyApproveDialog.contains(ANotifyId))
	{
		ExchangeApproveDialog *dialog = FNotifyApproveDialog.take(ANotifyId);
		WidgetManager::showActivateRaiseWindow(dialog);
		FNotifications->removeNotification(ANotifyId);
	}
}

void RosterItemExchange::onNotificationRemoved(int ANotifyId)
{
	if (FNotifyApproveDialog.contains(ANotifyId))
	{
		ExchangeApproveDialog *dialog = FNotifyApproveDialog.take(ANotifyId);
		dialog->reject();
	}
}

void RosterItemExchange::onExchangeApproveDialogAccepted()
{
	ExchangeApproveDialog *dialog = qobject_cast<ExchangeApproveDialog *>(sender());
	if (dialog)
	{
		IRosterExchangeRequest request = dialog->approvedRequest();
		applyRequest(request,dialog->subscribeNewContacts(),false);
		replyRequestResult(request);
	}
}

void RosterItemExchange::onExchangeApproveDialogRejected()
{
	ExchangeApproveDialog *dialog = qobject_cast<ExchangeApproveDialog *>(sender());
	if (dialog)
		replyRequestError(dialog->receivedRequest(),XmppStanzaError::EC_NOT_ALLOWED);
}

void RosterItemExchange::onExchangeApproveDialogDestroyed()
{
	ExchangeApproveDialog *dialog = qobject_cast<ExchangeApproveDialog *>(sender());
	if (FNotifications && dialog)
	{
		int notifyId = FNotifyApproveDialog.key(dialog);
		FNotifications->removeNotification(notifyId);
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_rosteritemexchange, RosterItemExchange)
#endif
