#include "recentcontacts.h"

#include <QDir>
#include <QDrag>
#include <QFile>
#include <QStyle>
#include <QPalette>
#if QT_VERSION >= 0x050000
#include <QMimeData>
#endif
#include <QMouseEvent>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/actiongroups.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/shortcuts.h>
#include <definitions/rosterlabels.h>
#include <definitions/rosterindexkinds.h>
#include <definitions/rosterindexroles.h>
#include <definitions/rostertooltiporders.h>
#include <definitions/rosterindexkindorders.h>
#include <definitions/rosterdataholderorders.h>
#include <definitions/rosterclickhookerorders.h>
#include <definitions/rosterlabelholderorders.h>
#include <definitions/rosterdragdropmimetypes.h>
#include <definitions/recentitemtypes.h>
#include <definitions/recentitemproperties.h>
#include <utils/iconstorage.h>
#include <utils/shortcuts.h>
#include <utils/datetime.h>
#include <utils/logger.h>
#include <utils/qt4qt5compat.h>

#define DIR_RECENT                   "recent"

#define PST_RECENTCONTACTS           "recent"
#define PSN_RECENTCONTACTS           "vacuum:recent-contacts"

#define MAX_STORAGE_CONTACTS         20
#define MIN_VISIBLE_CONTACTS         5

#define MAX_INACTIVE_TIMEOUT         31
#define MIN_INACTIVE_TIMEOUT         1

#define STORAGE_SAVE_TIMEOUT         100

#define ADR_STREAM_JID               Action::DR_StreamJid
#define ADR_CONTACT_JID              Action::DR_UserDefined + 1
#define ADR_INDEX_TYPE               Action::DR_UserDefined + 2
#define ADR_RECENT_TYPE              Action::DR_UserDefined + 3
#define ADR_RECENT_REFERENCE         Action::DR_UserDefined + 4

static const IRecentItem NullRecentItem = IRecentItem();

bool recentItemLessThen(const IRecentItem &AItem1, const IRecentItem &AItem2)
{
	bool favorite1 = AItem1.properties.value(REIP_FAVORITE).toBool();
	bool favorite2 = AItem2.properties.value(REIP_FAVORITE).toBool();
	return favorite1==favorite2 ? AItem1.activeTime>AItem2.activeTime : favorite1>favorite2;
}

RecentContacts::RecentContacts()
{
	FPrivateStorage = NULL;
	FRostersModel = NULL;
	FRostersView = NULL;
	FRostersViewPlugin = NULL;
	FMessageProcessor = NULL;
	FAccountManager = NULL;
	FStatusIcons = NULL;
	FOptionsManager = NULL;

	FRootIndex = NULL;
	FShowFavoriteLabelId = 0;
	
	FMaxVisibleItems = 20;
	FInactiveDaysTimeout = 7;
	FHideLaterContacts = true;
	FAllwaysShowOffline = true;
	FSimpleContactsView = true;
	FSortByLastActivity = true;
	FShowOnlyFavorite = false;

	FSaveTimer.setSingleShot(true);
	FSaveTimer.setInterval(STORAGE_SAVE_TIMEOUT);
	connect(&FSaveTimer,SIGNAL(timeout()),SLOT(onSaveItemsToStorageTimerTimeout()));
}

RecentContacts::~RecentContacts()
{
	if (FRootIndex)
		delete FRootIndex->instance();
}

void RecentContacts::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Recent Contacts");
	APluginInfo->description = tr("Displays a recently used contacts");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Potapov S.A. aka Lion";
	APluginInfo->homePage = "http://www.vacuum-im.org";
	APluginInfo->dependences.append(PRIVATESTORAGE_UUID);
}

bool RecentContacts::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);
	FPluginManager = APluginManager;

	IPlugin *plugin = APluginManager->pluginInterface("IPrivateStorage").value(0,NULL);
	if (plugin)
	{
		FPrivateStorage = qobject_cast<IPrivateStorage *>(plugin->instance());
		if (FPrivateStorage)
		{
			connect(FPrivateStorage->instance(),SIGNAL(storageOpened(const Jid &)),SLOT(onPrivateStorageOpened(const Jid &)));
			connect(FPrivateStorage->instance(),SIGNAL(dataLoaded(const QString &, const Jid &, const QDomElement &)),
				SLOT(onPrivateStorageDataLoaded(const QString &, const Jid &, const QDomElement &)));
			connect(FPrivateStorage->instance(),SIGNAL(dataChanged(const Jid &, const QString &, const QString &)),
				SLOT(onPrivateStorageDataChanged(const Jid &, const QString &, const QString &)));
			connect(FPrivateStorage->instance(),SIGNAL(storageNotifyAboutToClose(const Jid &)),SLOT(onPrivateStorageNotifyAboutToClose(const Jid &)));
			connect(FPrivateStorage->instance(),SIGNAL(storageClosed(const Jid &)),SLOT(onPrivateStorageClosed(const Jid &)));
		}
	}

	plugin = APluginManager->pluginInterface("IRostersModel").value(0,NULL);
	if (plugin)
	{
		FRostersModel = qobject_cast<IRostersModel *>(plugin->instance());
		if (FRostersModel)
		{
			connect(FRostersModel->instance(),SIGNAL(streamAdded(const Jid &)),SLOT(onRostersModelStreamAdded(const Jid &)));
			connect(FRostersModel->instance(),SIGNAL(streamRemoved(const Jid &)),SLOT(onRostersModelStreamRemoved(const Jid &)));
			connect(FRostersModel->instance(),SIGNAL(streamJidChanged(const Jid &, const Jid &)),SLOT(onRostersModelStreamJidChanged(const Jid &, const Jid &)));
			connect(FRostersModel->instance(),SIGNAL(indexInserted(IRosterIndex *)),SLOT(onRostersModelIndexInserted(IRosterIndex *)));
			connect(FRostersModel->instance(),SIGNAL(indexDataChanged(IRosterIndex *, int)),SLOT(onRostersModelIndexDataChanged(IRosterIndex*, int)));
			connect(FRostersModel->instance(),SIGNAL(indexRemoving(IRosterIndex *)),SLOT(onRostersModelIndexRemoving(IRosterIndex *)));
		}
	}

	plugin = APluginManager->pluginInterface("IRostersViewPlugin").value(0,NULL);
	if (plugin)
	{
		FRostersViewPlugin = qobject_cast<IRostersViewPlugin *>(plugin->instance());
		if (FRostersViewPlugin)
		{
			FRostersView = FRostersViewPlugin->rostersView();
			connect(FRostersView->instance(),SIGNAL(indexMultiSelection(const QList<IRosterIndex *> &, bool &)), 
				SLOT(onRostersViewIndexMultiSelection(const QList<IRosterIndex *> &, bool &)));
			connect(FRostersView->instance(), SIGNAL(indexContextMenu(const QList<IRosterIndex *> &, quint32 , Menu *)),
				SLOT(onRostersViewIndexContextMenu(const QList<IRosterIndex *> &, quint32 , Menu *)));
			connect(FRostersView->instance(), SIGNAL(indexToolTips(IRosterIndex*,quint32,QMap<int,QString>&)),
				SLOT(onRostersViewIndexToolTips(IRosterIndex*,quint32,QMap<int,QString>&)));
			connect(FRostersView->instance(), SIGNAL(notifyInserted(int)),SLOT(onRostersViewNotifyInserted(int)));
			connect(FRostersView->instance(), SIGNAL(notifyRemoved(int)),SLOT(onRostersViewNotifyRemoved(int)));
			connect(FRostersView->instance(), SIGNAL(notifyActivated(int)),SLOT(onRostersViewNotifyActivated(int)));
		}
	}

	plugin = APluginManager->pluginInterface("IMessageProcessor").value(0,NULL);
	if (plugin)
	{
		FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IAccountManager").value(0,NULL);
	if (plugin)
	{
		FAccountManager = qobject_cast<IAccountManager *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IStatusIcons").value(0,NULL);
	if (plugin)
	{
		FStatusIcons = qobject_cast<IStatusIcons *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
	{
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
	}

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

	connect(Shortcuts::instance(),SIGNAL(shortcutActivated(const QString &, QWidget *)),SLOT(onShortcutActivated(const QString &, QWidget *)));

	return FPrivateStorage!=NULL;
}

bool RecentContacts::initObjects()
{
	Shortcuts::declareShortcut(SCT_ROSTERVIEW_INSERTFAVORITE,tr("Add contact to favorites"),QKeySequence::UnknownKey,Shortcuts::WidgetShortcut);
	Shortcuts::declareShortcut(SCT_ROSTERVIEW_REMOVEFAVORITE,tr("Remove contact from favorites"),QKeySequence::UnknownKey,Shortcuts::WidgetShortcut);

	if (FRostersView)
	{
		AdvancedDelegateItem showFavorite(RLID_RECENT_FAVORITE);
		showFavorite.d->kind = AdvancedDelegateItem::CustomData;
		showFavorite.d->data = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_RECENT_FAVORITE);
		FShowFavoriteLabelId = FRostersView->registerLabel(showFavorite);

		FRostersView->insertDragDropHandler(this);
		FRostersView->insertLabelHolder(RLHO_RECENT_FILTER,this);
		FRostersView->insertClickHooker(RCHO_RECENTCONTACTS,this);
		FRostersViewPlugin->registerExpandableRosterIndexKind(RIK_RECENT_ROOT,RDR_KIND);

		Shortcuts::insertWidgetShortcut(SCT_ROSTERVIEW_INSERTFAVORITE,FRostersView->instance());
		Shortcuts::insertWidgetShortcut(SCT_ROSTERVIEW_REMOVEFAVORITE,FRostersView->instance());
	}

	if (FRostersModel)
	{
		FRootIndex = FRostersModel->newRosterIndex(RIK_RECENT_ROOT);
		FRootIndex->setData(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_RECENT),Qt::DecorationRole);
		FRootIndex->setData(RIKO_RECENT_ROOT,RDR_KIND_ORDER);
		FRootIndex->setData(tr("Recent Contacts"),RDR_NAME);

		FRostersModel->insertRosterDataHolder(RDHO_RECENTCONTACTS,this);
	}

	if (FOptionsManager)
	{
		FOptionsManager->insertOptionsDialogHolder(this);
	}
	
	registerItemHandler(REIT_CONTACT,this);
	return true;
}

bool RecentContacts::initSettings()
{
	Options::setDefaultValue(OPV_ROSTER_RECENT_ALWAYSSHOWOFFLINE,true);
	Options::setDefaultValue(OPV_ROSTER_RECENT_HIDEINACTIVEITEMS,true);
	Options::setDefaultValue(OPV_ROSTER_RECENT_SIMPLEITEMSVIEW,true);
	Options::setDefaultValue(OPV_ROSTER_RECENT_SORTBYACTIVETIME,true);
	Options::setDefaultValue(OPV_ROSTER_RECENT_SHOWONLYFAVORITE,false);
	Options::setDefaultValue(OPV_ROSTER_RECENT_MAXVISIBLEITEMS,20);
	Options::setDefaultValue(OPV_ROSTER_RECENT_INACTIVEDAYSTIMEOUT,7);
	return true;
}

bool RecentContacts::startPlugin()
{
	return true;
}

QList<int> RecentContacts::rosterDataRoles(int AOrder) const
{
	if (AOrder == RDHO_RECENTCONTACTS)
	{
		static const QList<int> roles = QList<int>() << RDR_ALL_ROLES
			<< RDR_FULL_JID << RDR_PREP_FULL_JID << RDR_PREP_BARE_JID;
		return roles;
	}
	return QList<int>();
}

QVariant RecentContacts::rosterData(int AOrder, const IRosterIndex *AIndex, int ARole) const
{
	if (AOrder == RDHO_RECENTCONTACTS)
	{
		static bool labelsBlock = false;

		switch (AIndex->kind())
		{
		case RIK_RECENT_ROOT:
			{
				QPalette palette = FRostersView!=NULL ? FRostersView->instance()->palette() : QPalette();
				switch (ARole)
				{
				case Qt::ForegroundRole:
					return palette.color(QPalette::Active, QPalette::BrightText);
				case Qt::BackgroundColorRole:
					return palette.color(QPalette::Active, QPalette::Dark);
				case RDR_FORCE_VISIBLE:
					return 1;
				}
				break;
			}
		case RIK_RECENT_ITEM:
			{
				IRosterIndex *proxy = NULL;
				switch (ARole)
				{
				case RDR_STREAM_JID:
				case RDR_RECENT_TYPE:
				case RDR_RECENT_REFERENCE:
				case RDR_RECENT_DATETIME:
					break;
				case RDR_FORCE_VISIBLE:
					return FAllwaysShowOffline ? 1 : 0;
				case RDR_LABEL_ITEMS:
					proxy = !labelsBlock ? FIndexToProxy.value(AIndex) : NULL;
					if (proxy != NULL)
					{
						labelsBlock = true;
						AdvancedDelegateItems proxyLabels = proxy->data(RDR_LABEL_ITEMS).value<AdvancedDelegateItems>();
						AdvancedDelegateItems recentLabels = AIndex->data(RDR_LABEL_ITEMS).value<AdvancedDelegateItems>();
						labelsBlock = false;

						for (AdvancedDelegateItems::const_iterator it=proxyLabels.constBegin(); it!=proxyLabels.constEnd(); ++it)
						{
							if (!recentLabels.contains(it.key()))
								recentLabels.insert(it.key(),it.value());
						}

						return QVariant::fromValue<AdvancedDelegateItems>(recentLabels);
					}
					break;
				default:
					proxy = FIndexToProxy.value(AIndex);
				}
				return proxy!=NULL ? proxy->data(ARole) : QVariant();
			}
		}
	}
	return QVariant();
}

bool RecentContacts::setRosterData(int AOrder, const QVariant &AValue, IRosterIndex *AIndex, int ARole)
{
	Q_UNUSED(AOrder); Q_UNUSED(AIndex); Q_UNUSED(ARole); Q_UNUSED(AValue);
	return false;
}

QMultiMap<int, IOptionsDialogWidget *> RecentContacts::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (FOptionsManager && ANodeId==OPN_ROSTERVIEW)
	{
		widgets.insertMulti(OHO_ROSTER_RECENT,FOptionsManager->newOptionsDialogHeader(tr("Recent contacts"),AParent));
		widgets.insertMulti(OWO_ROSTER_RECENT_HIDEINACTIVEITEMS,FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ROSTER_RECENT_HIDEINACTIVEITEMS),tr("Hide inactive contacts"),AParent));
		widgets.insertMulti(OWO_ROSTER_RECENT_SORTBYACTIVETIME,FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ROSTER_RECENT_SORTBYACTIVETIME),tr("Sort contacts by last activity"),AParent));
		widgets.insertMulti(OWO_ROSTER_RECENT_ALWAYSSHOWOFFLINE,FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ROSTER_RECENT_ALWAYSSHOWOFFLINE),tr("Always show offline contacts"),AParent));
		widgets.insertMulti(OWO_ROSTER_RECENT_SHOWONLYFAVORITE,FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ROSTER_RECENT_SHOWONLYFAVORITE),tr("Show only favorite contacts"),AParent));
		widgets.insertMulti(OWO_ROSTER_RECENT_SIMPLEITEMSVIEW,FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ROSTER_RECENT_SIMPLEITEMSVIEW),tr("Simplify recent contacts view"),AParent));
	}
	return widgets;
}

Qt::DropActions RecentContacts::rosterDragStart(const QMouseEvent *AEvent, IRosterIndex *AIndex, QDrag *ADrag)
{
	Qt::DropActions actions = Qt::IgnoreAction;
	if (AIndex->kind() == RIK_RECENT_ITEM)
	{
		IRosterIndex *proxy = FIndexToProxy.value(AIndex);
		if (proxy)
		{
			foreach(IRostersDragDropHandler *handler, FRostersView->dragDropHandlers())
				actions |= handler!=this ? handler->rosterDragStart(AEvent,proxy,ADrag) : Qt::IgnoreAction;

			if (actions != Qt::IgnoreAction)
			{
				QByteArray proxyData;
				QDataStream proxyStream(&proxyData,QIODevice::WriteOnly);
				operator<<(proxyStream,proxy->indexData());
				ADrag->mimeData()->setData(DDT_ROSTERSVIEW_INDEX_DATA,proxyData);

				QByteArray indexData;
				QDataStream indexStream(&indexData,QIODevice::WriteOnly);
				operator<<(indexStream,AIndex->indexData());
				ADrag->mimeData()->setData(DDT_RECENT_INDEX_DATA,indexData);
			}
		}
	}
	return actions;
}

bool RecentContacts::rosterDragEnter(const QDragEnterEvent *AEvent)
{
	FEnteredProxyDragHandlers.clear();
	foreach(IRostersDragDropHandler *handler, FRostersView->dragDropHandlers())
		if (handler!=this && handler->rosterDragEnter(AEvent))
			FEnteredProxyDragHandlers.append(handler);
	return !FEnteredProxyDragHandlers.isEmpty();
}

bool RecentContacts::rosterDragMove(const QDragMoveEvent *AEvent, IRosterIndex *AHover)
{
	FMovedProxyDragHandlers.clear();
	if (AHover->data(RDR_KIND).toInt() == RIK_RECENT_ITEM)
	{
		IRosterIndex *proxy = FIndexToProxy.value(AHover);
		if (proxy)
		{
			foreach(IRostersDragDropHandler *handler, FEnteredProxyDragHandlers)
				if (handler!=this && handler->rosterDragMove(AEvent,proxy))
					FMovedProxyDragHandlers.append(handler);
		}
	}
	return !FMovedProxyDragHandlers.isEmpty();
}

void RecentContacts::rosterDragLeave(const QDragLeaveEvent *AEvent)
{
	Q_UNUSED(AEvent);
}

bool RecentContacts::rosterDropAction(const QDropEvent *AEvent, IRosterIndex *AIndex, Menu *AMenu)
{
	bool accepted = false;
	if (AIndex->kind() == RIK_RECENT_ITEM)
	{
		IRosterIndex *proxy = FIndexToProxy.value(AIndex);
		if (proxy)
		{
			foreach(IRostersDragDropHandler *handler, FMovedProxyDragHandlers)
				if (handler!=this && handler->rosterDropAction(AEvent,proxy,AMenu))
					accepted = true;
		}
	}
	return accepted;
}

QList<quint32> RecentContacts::rosterLabels(int AOrder, const IRosterIndex *AIndex) const
{
	QList<quint32> labels;
	if (AOrder==RLHO_RECENT_FILTER && AIndex->kind()==RIK_RECENT_ITEM)
	{
		if (FSimpleContactsView)
		{
            // *** <<< eyeCU <<< ***
            labels.append(RLID_AVATAR_IMAGE_LEFT);
            labels.append(RLID_AVATAR_IMAGE_RIGHT);
            // *** >>> eyeCU >>> ***
			labels.append(RLID_ROSTERSVIEW_STATUS);
		}
		labels.append(RLID_METACONTACTS_BRANCH);
	}
	return labels;
}

AdvancedDelegateItem RecentContacts::rosterLabel(int AOrder, quint32 ALabelId, const IRosterIndex *AIndex) const
{
	Q_UNUSED(AOrder); Q_UNUSED(ALabelId); Q_UNUSED(AIndex);
	static const AdvancedDelegateItem null = AdvancedDelegateItem();
	return null;
}

bool RecentContacts::rosterIndexSingleClicked(int AOrder, IRosterIndex *AIndex, const QMouseEvent *AEvent)
{
	if (AOrder==RCHO_RECENTCONTACTS && AIndex->kind()==RIK_RECENT_ITEM)
	{
		IRosterIndex *proxy = FIndexToProxy.value(AIndex);
		if (proxy)
			return FRostersView->singleClickOnIndex(proxy,AEvent);
		else if (AIndex->data(RDR_RECENT_TYPE)==REIT_CONTACT && Options::node(OPV_MESSAGES_COMBINEWITHROSTER).value().toBool())
			return rosterIndexDoubleClicked(AOrder,AIndex,AEvent);
	}
	return false;
}

bool RecentContacts::rosterIndexDoubleClicked(int AOrder, IRosterIndex *AIndex, const QMouseEvent *AEvent)
{
	if (AOrder==RCHO_RECENTCONTACTS && AIndex->kind()==RIK_RECENT_ITEM)
	{
		IRosterIndex *proxy = FIndexToProxy.value(AIndex);
		if (proxy)
		{
			return FRostersView->doubleClickOnIndex(proxy,AEvent);
		}
		else if (FRostersModel!=NULL && AIndex->data(RDR_RECENT_TYPE)==REIT_CONTACT)
		{
			IRosterIndex *index = FRostersModel->getContactIndexes(AIndex->data(RDR_STREAM_JID).toString(),AIndex->data(RDR_RECENT_REFERENCE).toString()).value(0);
			return index!=NULL ? FRostersView->doubleClickOnIndex(index,AEvent) : false;
		}
	}
	return false;
}

bool RecentContacts::recentItemValid(const IRecentItem &AItem) const
{
	return !AItem.reference.isEmpty() && AItem.streamJid.pBare()!=AItem.reference && Jid(AItem.reference).hasNode();
}

bool RecentContacts::recentItemCanShow(const IRecentItem &AItem) const
{
	Q_UNUSED(AItem);
	return true;
}

QIcon RecentContacts::recentItemIcon(const IRecentItem &AItem) const
{
	return FStatusIcons!=NULL ? FStatusIcons->iconByJid(AItem.streamJid,AItem.reference) : QIcon();
}

QString RecentContacts::recentItemName(const IRecentItem &AItem) const
{
	QString name = itemProperty(AItem,REIP_NAME).toString();
	return name.isEmpty() ? AItem.reference : name;
}

IRecentItem RecentContacts::recentItemForIndex(const IRosterIndex *AIndex) const
{
	IRecentItem item;
	if (AIndex->kind() == RIK_CONTACT)
	{
		item.type = REIT_CONTACT;
		item.streamJid = AIndex->data(RDR_STREAM_JID).toString();
		item.reference = AIndex->data(RDR_PREP_BARE_JID).toString();
	}
	return item;
}

QList<IRosterIndex *> RecentContacts::recentItemProxyIndexes(const IRecentItem &AItem) const
{
	QList<IRosterIndex *> proxies = FRostersModel!=NULL ? FRostersModel->findContactIndexes(AItem.streamJid,AItem.reference) : QList<IRosterIndex *>();
	std::sort(proxies.begin(),proxies.end());
	return proxies;
}

bool RecentContacts::isReady(const Jid &AStreamJid) const
{
	return FPrivateStorage==NULL || FReadyStreams.contains(AStreamJid);
}

bool RecentContacts::isValidItem(const IRecentItem &AItem) const
{
	if (AItem.isNull())
		return false;
	if (!FStreamItems.contains(AItem.streamJid))
		return false;
	if (FItemHandlers.contains(AItem.type) && !FItemHandlers.value(AItem.type)->recentItemValid(AItem))
		return false;
	return true;
}

QList<IRecentItem> RecentContacts::streamItems(const Jid &AStreamJid) const
{
	return FStreamItems.value(AStreamJid);
}

QVariant RecentContacts::itemProperty(const IRecentItem &AItem, const QString &AName) const
{
	return findRealItem(AItem).properties.value(AName);
}

void RecentContacts::setItemProperty(const IRecentItem &AItem, const QString &AName, const QVariant &AValue)
{
	if (isReady(AItem.streamJid) && isValidItem(AItem))
	{
		bool isItemChanged = false;

		IRecentItem item = findRealItem(AItem);
		if (item.isNull())
		{
			item = AItem;
			isItemChanged = true;
		}

		QVariant nullValue = QVariant(AValue.type());
		if (AValue != nullValue)
		{
			if (!item.properties.contains(AName))
			{
				isItemChanged = true;
				item.properties.insert(AName,AValue);
			}
			else if (item.properties.value(AName).toString() != AValue.toString())
			{
				isItemChanged = true;
				item.properties.insert(AName,AValue);
			}
		}
		else if (item.properties.contains(AName))
		{
			isItemChanged = true;
			item.properties.remove(AName);
		}
	
		if (isItemChanged)
		{
			LOG_STRM_DEBUG(AItem.streamJid,QString("Recent item property changed, type=%1, ref=%2, property=%3, value=%4").arg(AItem.type,AItem.reference,AName,AValue.toString()));
			
			item.updateTime = QDateTime::currentDateTime();
			mergeRecentItems(item.streamJid, QList<IRecentItem>() << item, false);

			startSaveItemsToStorage(item.streamJid);
		}
	}
	else if (!isReady(AItem.streamJid))
	{
		LOG_STRM_WARNING(AItem.streamJid,QString("Failed to change recent item property, type=%1, ref=%2, property=%3, value=%4: Stream not ready").arg(AItem.type,AItem.reference,AName,AValue.toString()));
	}
	else
	{
		LOG_STRM_ERROR(AItem.streamJid,QString("Failed to change recent item property, type=%1, ref=%2, property=%3, value=%4: Item not valid").arg(AItem.type,AItem.reference,AName,AValue.toString()));
	}
}

void RecentContacts::setItemActiveTime(const IRecentItem &AItem, const QDateTime &ATime)
{
	if (isReady(AItem.streamJid) && isValidItem(AItem))
	{
		LOG_STRM_DEBUG(AItem.streamJid,QString("Changing recent item active time, type=%1, ref=%2, time=%3").arg(AItem.type,AItem.reference,ATime.toString(Qt::ISODate)));
		IRecentItem item = findRealItem(AItem);
		if (item.isNull())
		{
			item = AItem;
			
			item.activeTime = ATime;
			mergeRecentItems(item.streamJid, QList<IRecentItem>() << item, false);
			
			startSaveItemsToStorage(item.streamJid);
		}
		else if (item.activeTime < ATime)
		{
			item.activeTime = ATime;
			mergeRecentItems(item.streamJid, QList<IRecentItem>() << item, false);
		}
	}
	else if (!isReady(AItem.streamJid))
	{
		LOG_STRM_WARNING(AItem.streamJid,QString("Failed to change recent item active time, type=%1, ref=%2, time=%3: Stream not ready").arg(AItem.type,AItem.reference,ATime.toString(Qt::ISODate)));
	}
	else
	{
		LOG_STRM_ERROR(AItem.streamJid,QString("Failed to change recent item active time, type=%1, ref=%2, time=%3: Item not valid").arg(AItem.type,AItem.reference,ATime.toString(Qt::ISODate)));
	}
}

void RecentContacts::removeItem(const IRecentItem &AItem)
{
	if (isReady(AItem.streamJid))
	{
		QList<IRecentItem> newItems = FStreamItems.value(AItem.streamJid);
		int index = newItems.indexOf(AItem);
		if (index >= 0)
		{
			LOG_STRM_DEBUG(AItem.streamJid,QString("Removing recent item, type=%1, ref=%2").arg(AItem.type,AItem.reference));
			newItems.removeAt(index);
			mergeRecentItems(AItem.streamJid,newItems,true);
			startSaveItemsToStorage(AItem.streamJid);
		}
	}
	else
	{
		LOG_STRM_WARNING(AItem.streamJid,QString("Failed to remove recent item, type=%1, ref=%2: Stream not ready").arg(AItem.type,AItem.reference));
	}
}

QList<IRecentItem> RecentContacts::visibleItems() const
{
	return FVisibleItems.keys();
}

IRecentItem RecentContacts::rosterIndexItem(const IRosterIndex *AIndex) const
{
	if (AIndex->kind() == RIK_RECENT_ITEM)
	{
		IRecentItem item;
		item.type = AIndex->data(RDR_RECENT_TYPE).toString();
		item.streamJid = AIndex->data(RDR_STREAM_JID).toString();
		item.reference = AIndex->data(RDR_RECENT_REFERENCE).toString();
		return item;
	}
	else foreach(IRecentItemHandler *handler, FItemHandlers)
	{
		IRecentItem item = handler->recentItemForIndex(AIndex);
		if (isValidItem(item))
			return item;
	}
	return NullRecentItem;
}

IRosterIndex *RecentContacts::itemRosterIndex(const IRecentItem &AItem) const
{
	return FVisibleItems.value(AItem);
}

IRosterIndex *RecentContacts::itemRosterProxyIndex(const IRecentItem &AItem) const
{
	return FIndexToProxy.value(FVisibleItems.value(AItem));
}

QList<QString> RecentContacts::itemHandlerTypes() const
{
	return FItemHandlers.keys();
}

IRecentItemHandler *RecentContacts::itemTypeHandler(const QString &AType) const
{
	return FItemHandlers.value(AType);
}

void RecentContacts::registerItemHandler(const QString &AType, IRecentItemHandler *AHandler)
{
	if (AHandler != NULL)
	{
		LOG_DEBUG(QString("Recent item handler registered, type=%1").arg(AType));
		if (!FItemHandlers.values().contains(AHandler))
			connect(AHandler->instance(),SIGNAL(recentItemUpdated(const IRecentItem &)),SLOT(onHandlerRecentItemUpdated(const IRecentItem &)));
		FItemHandlers.insert(AType,AHandler);
		emit itemHandlerRegistered(AType,AHandler);
	}
}

void RecentContacts::updateVisibleItems()
{
	if (FRostersModel)
	{
		int favoriteCount = 0;
		QList<IRecentItem> common;
		for (QMap<Jid, QList<IRecentItem> >::const_iterator stream_it=FStreamItems.constBegin(); stream_it!=FStreamItems.constEnd(); ++stream_it)
		{
			for (QList<IRecentItem>::const_iterator it = stream_it->constBegin(); it!=stream_it->constEnd(); ++it)
			{
				IRecentItemHandler *handler = FItemHandlers.value(it->type);
				if (handler!=NULL && handler->recentItemCanShow(*it))
				{
					if (it->properties.value(REIP_FAVORITE).toBool())
						favoriteCount++;
					common.append(*it);
				}
			}
		}
		std::sort(common.begin(),common.end(),recentItemLessThen);

		QDateTime firstTime;
		for (QList<IRecentItem>::iterator it=common.begin(); it!=common.end(); )
		{
			if (it->properties.value(REIP_FAVORITE).toBool())
			{
				++it;
			}
			else if (FShowOnlyFavorite)
			{
				it = common.erase(it);
			}
			else if (FHideLaterContacts)
			{
				if (firstTime.isNull())
				{
					firstTime = it->activeTime;
					++it;
				}
				else if (it->activeTime.daysTo(firstTime) > FInactiveDaysTimeout)
				{
					it = common.erase(it);
				}
				else
				{
					++it;
				}
			}
			else
			{
				++it;
			}
		}

		QSet<IRecentItem> curVisible = FVisibleItems.keys().toSet();
		QSet<IRecentItem> newVisible = common.mid(0,FMaxVisibleItems+favoriteCount).toSet();

		QSet<IRecentItem> addItems = newVisible - curVisible;
		QSet<IRecentItem> removeItems = curVisible - newVisible;

		foreach(const IRecentItem &item, removeItems)
			removeItemIndex(item);

		foreach(const IRecentItem &item, addItems)
			createItemIndex(item);

		if (!addItems.isEmpty() || !removeItems.isEmpty())
			emit visibleItemsChanged();
	}
}

void RecentContacts::createItemIndex(const IRecentItem &AItem)
{
	IRosterIndex *index = FVisibleItems.value(AItem);
	if (index == NULL)
	{
		IRecentItemHandler *handler = FItemHandlers.value(AItem.type);
		if (handler)
		{
			index = FRostersModel->newRosterIndex(RIK_RECENT_ITEM);
			FVisibleItems.insert(AItem,index);

			index->setData(AItem.type,RDR_RECENT_TYPE);
			index->setData(AItem.reference,RDR_RECENT_REFERENCE);
			index->setData(AItem.streamJid.pFull(),RDR_STREAM_JID);

			FRostersModel->insertRosterIndex(index,FRootIndex);
			emit recentItemIndexCreated(AItem,index);

			updateItemProxy(AItem);
			updateItemIndex(AItem);
		}
	}
}

void RecentContacts::updateItemIndex(const IRecentItem &AItem)
{
	static const QDateTime zero = QDateTime::fromTime_t(0);

	IRosterIndex *index = FVisibleItems.value(AItem);
	if (index)
	{
		IRecentItem item = findRealItem(AItem);
		bool favorite = item.properties.value(REIP_FAVORITE).toBool();

		IRosterIndex *proxy = FIndexToProxy.value(index);
		if (proxy == NULL)
		{
			IRecentItemHandler *handler = FItemHandlers.value(item.type);
			if (handler)
			{
				index->setData(handler->recentItemName(item),RDR_NAME);
				index->setData(handler->recentItemIcon(item),Qt::DecorationRole);
			}
		}
		index->setData(item.activeTime,RDR_RECENT_DATETIME);
		
		if (FSortByLastActivity)
			index->setData((int)(favorite ? 0x80000000 : item.activeTime.secsTo(zero)),RDR_SORT_ORDER);
		else
			index->setData(favorite ? QString("") : index->data(Qt::DisplayRole).toString(),RDR_SORT_ORDER);

		if (FRostersView)
		{
			if (favorite)
				FRostersView->insertLabel(FShowFavoriteLabelId,index);
			else
				FRostersView->removeLabel(FShowFavoriteLabelId,index);
		}
	}
}

void RecentContacts::removeItemIndex(const IRecentItem &AItem)
{
	IRosterIndex *index = FVisibleItems.take(AItem);
	if (index)
	{
		FIndexProxies.remove(index);
		FProxyToIndex.remove(FIndexToProxy.take(index));
		FRostersModel->removeRosterIndex(index);
	}
}

void RecentContacts::updateItemProxy(const IRecentItem &AItem)
{
	IRosterIndex *index = FVisibleItems.value(AItem);
	if (index)
	{
		IRecentItemHandler *handler = FItemHandlers.value(AItem.type);
		if (handler)
		{
			QList<IRosterIndex *> proxies = handler->recentItemProxyIndexes(AItem);
			FIndexProxies.insert(index,proxies);

			IRosterIndex *proxy = proxies.value(0);
			IRosterIndex *oldProxy = FIndexToProxy.value(index);
			if (oldProxy != proxy)
			{
				FProxyToIndex.remove(FIndexToProxy.take(index));
				if (proxy)
				{
					FIndexToProxy.insert(index,proxy);
					FProxyToIndex.insert(proxy,index);
				}
			}
		}
	}
}

IRecentItem &RecentContacts::findRealItem(const IRecentItem &AItem)
{
	static IRecentItem nullItem;
	if (FStreamItems.contains(AItem.streamJid))
	{
		QList<IRecentItem> &items = FStreamItems[AItem.streamJid];
		int index = items.indexOf(AItem);
		return index>=0 ? items[index] : nullItem;
	}
	return nullItem;
}

IRecentItem RecentContacts::findRealItem(const IRecentItem &AItem) const
{
	const QList<IRecentItem> items = FStreamItems.value(AItem.streamJid);
	int index = items.indexOf(AItem);
	return index>=0 ? items.value(index) : NullRecentItem;
}

void RecentContacts::mergeRecentItems(const Jid &AStreamJid, const QList<IRecentItem> &AItems, bool AReplace)
{
	bool hasChanges = false;
	QSet<IRecentItem> newItems;
	QSet<IRecentItem> addedItems;
	QSet<IRecentItem> changedItems;
	QSet<IRecentItem> removedItems;

	QList<IRecentItem> &curItems = FStreamItems[AStreamJid];
	for (QList<IRecentItem>::const_iterator it=AItems.constBegin(); it!=AItems.constEnd(); ++it)
	{
		IRecentItem newItem = *it;
		newItem.streamJid = AStreamJid;

		if (isValidItem(newItem))
		{
			if (!newItem.activeTime.isValid() || newItem.activeTime > QDateTime::currentDateTime())
				newItem.activeTime = QDateTime::currentDateTime();
			if (!newItem.updateTime.isValid() || newItem.updateTime > QDateTime::currentDateTime())
				newItem.updateTime = QDateTime::currentDateTime();
			newItems += newItem;

			int index = curItems.indexOf(newItem);
			if (index >= 0)
			{
				IRecentItem &curItem = curItems[index];
				if (curItem.updateTime < newItem.updateTime)
				{
					curItem.updateTime = newItem.updateTime;
					curItem.properties = newItem.properties;
					changedItems += curItem;
					hasChanges = true;
				}
				if (curItem.activeTime < newItem.activeTime)
				{
					curItem.activeTime = newItem.activeTime;
					changedItems += curItem;
					hasChanges = true;
				}
			}
			else
			{
				curItems.append(newItem);
				addedItems += newItem;
				hasChanges = true;
			}
		}
	}

	if (AReplace)
	{
		removedItems += curItems.toSet()-newItems;
		foreach(const IRecentItem &item, removedItems)
		{
			curItems.removeAll(item);
			hasChanges = true;
		}
	}

	if (hasChanges)
	{
		std::sort(curItems.begin(),curItems.end(),recentItemLessThen);

		int favoriteCount = 0;
		while(favoriteCount<curItems.count() && curItems.at(favoriteCount).properties.value(REIP_FAVORITE).toBool())
			favoriteCount++;

		int removeCount = curItems.count() - favoriteCount - MAX_STORAGE_CONTACTS;
		for(int index = curItems.count()-1; removeCount>0 && index>=0; index--)
		{
			if (!curItems.at(index).properties.value(REIP_FAVORITE).toBool())
			{
				removedItems += curItems.takeAt(index);
				removeCount--;
			}
		}

		updateVisibleItems();
	}

	bool isStreamReady = isReady(AStreamJid);
	foreach(const IRecentItem &item, addedItems)
	{
		if (isStreamReady)
			LOG_STRM_INFO(AStreamJid,QString("Recent item added, type=%1, ref=%2").arg(item.type,item.reference));
		else
			LOG_STRM_DEBUG(AStreamJid,QString("Recent item added, type=%1, ref=%2").arg(item.type,item.reference));
		emit recentItemAdded(item);
	}
 
	foreach(const IRecentItem &item, removedItems)
	{
		if (isStreamReady)
			LOG_STRM_INFO(AStreamJid,QString("Recent item removed, type=%1, ref=%2").arg(item.type,item.reference));
		else
			LOG_STRM_DEBUG(AStreamJid,QString("Recent item removed, type=%1, ref=%2").arg(item.type,item.reference));
		emit recentItemRemoved(item);
	}

	foreach(const IRecentItem &item, changedItems)
	{
		LOG_STRM_DEBUG(AStreamJid,QString("Recent item changed, type=%1, ref=%2").arg(item.type,item.reference));
		updateItemIndex(item);
		emit recentItemChanged(item);
	}
}

void RecentContacts::startSaveItemsToStorage(const Jid &AStreamJid)
{
	if (FPrivateStorage && isReady(AStreamJid))
	{
		FSaveTimer.start();
		FSaveStreams += AStreamJid;
	}
	else if (FPrivateStorage)
	{
		LOG_STRM_WARNING(AStreamJid,"Failed to start save recent items to storage: Stream not ready");
	}
}

bool RecentContacts::saveItemsToStorage(const Jid &AStreamJid) const
{
	if (FPrivateStorage && isReady(AStreamJid))
	{
		QDomDocument doc;
		QDomElement itemsElem = doc.appendChild(doc.createElementNS(PSN_RECENTCONTACTS,PST_RECENTCONTACTS)).toElement();
		saveItemsToXML(itemsElem,streamItems(AStreamJid),true);
		if (!FPrivateStorage->saveData(AStreamJid,itemsElem).isEmpty())
		{
			LOG_STRM_INFO(AStreamJid,"Save recent items request sent");
			return true;
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,"Failed to send save recent items request");
		}
	}
	else if (FPrivateStorage)
	{
		REPORT_ERROR("Failed to save recent items to storage: Stream not ready");
	}
	return false;
}

QString RecentContacts::recentFileName(const Jid &AStreamJid) const
{
	QDir dir(FPluginManager->homePath());
	if (!dir.exists(DIR_RECENT))
		dir.mkdir(DIR_RECENT);
	dir.cd(DIR_RECENT);
	return dir.absoluteFilePath(Jid::encode(AStreamJid.pBare())+".xml");
}

QList<IRecentItem> RecentContacts::loadItemsFromFile(const QString &AFileName) const
{
	QList<IRecentItem> items;

	QFile file(AFileName);
	if (file.open(QIODevice::ReadOnly))
	{
		QString xmlError;
		QDomDocument doc;
		if (doc.setContent(&file,true,&xmlError))
		{
			QDomElement itemsElem = doc.firstChildElement(PST_RECENTCONTACTS);
			items = loadItemsFromXML(itemsElem,false);
		}
		else
		{
			REPORT_ERROR(QString("Failed to load recent items from file content: %1").arg(xmlError));
			file.remove();
		}
	}
	else if (file.exists())
	{
		REPORT_ERROR(QString("Failed to load recent items from file: %1").arg(file.errorString()));
	}

	return items;
}

void RecentContacts::saveItemsToFile(const QString &AFileName, const QList<IRecentItem> &AItems) const
{
	QFile file(AFileName);
	if (file.open(QIODevice::WriteOnly|QIODevice::Truncate))
	{
		QDomDocument doc;
		QDomElement itemsElem = doc.appendChild(doc.createElementNS(PSN_RECENTCONTACTS,PST_RECENTCONTACTS)).toElement();
		saveItemsToXML(itemsElem,AItems,false);
		file.write(doc.toByteArray());
		file.close();
	}
	else
	{
		REPORT_ERROR(QString("Failed to save recent items to file: %1").arg(file.errorString()));
	}
}

QList<IRecentItem> RecentContacts::loadItemsFromXML(const QDomElement &AElement, bool APlainPassword) const
{
	QList<IRecentItem> items;
	QDomElement itemElem = AElement.firstChildElement("item");
	while (!itemElem.isNull())
	{
		IRecentItem item;
		item.type = itemElem.attribute("type");
		item.reference = itemElem.attribute("reference");
		item.activeTime = DateTime(itemElem.attribute("activeTime")).toLocal();
		item.updateTime = DateTime(itemElem.attribute("updateTime")).toLocal();

		QDomElement propElem = itemElem.firstChildElement("property");
		while(!propElem.isNull())
		{
			QString propName = propElem.attribute("name");
			QString propValue = propElem.text();
			bool decryptValue = !APlainPassword && propName=="password";

			item.properties.insert(propName, decryptValue ? Options::decrypt(propValue.toLatin1()).toString() : propValue);
			propElem = propElem.nextSiblingElement("property");
		}
		items.append(item);

		itemElem = itemElem.nextSiblingElement("item");
	}
	return items;
}

void RecentContacts::saveItemsToXML(QDomElement &AElement, const QList<IRecentItem> &AItems, bool APlainPassword) const
{
	for (QList<IRecentItem>::const_iterator itemIt=AItems.constBegin(); itemIt!=AItems.constEnd(); ++itemIt)
	{
		QDomElement itemElem = AElement.ownerDocument().createElement("item");
		itemElem.setAttribute("type",itemIt->type);
		itemElem.setAttribute("reference",itemIt->reference);
		itemElem.setAttribute("activeTime",DateTime(itemIt->activeTime).toX85DateTime());
		itemElem.setAttribute("updateTime",DateTime(itemIt->updateTime).toX85DateTime());

		for (QMap<QString, QVariant>::const_iterator propIt=itemIt->properties.constBegin(); propIt!=itemIt->properties.constEnd(); ++propIt)
		{
			QString propName = propIt.key();
			QString propValue = propIt->toString();
			bool encryptValue = !APlainPassword && propName=="password";

			QDomElement propElem = AElement.ownerDocument().createElement("property");
			propElem.setAttribute("name",propName);
			propElem.appendChild(AElement.ownerDocument().createTextNode(encryptValue ? QString::fromLatin1(Options::encrypt(propValue)) : propValue));
			itemElem.appendChild(propElem);
		}
		
		AElement.appendChild(itemElem);
	}
}

bool RecentContacts::isSelectionAccepted(const QList<IRosterIndex *> &ASelected) const
{
	foreach(IRosterIndex *index, ASelected)
		if (rosterIndexItem(index).type.isEmpty())
			return false;
	return !ASelected.isEmpty();
}

bool RecentContacts::isRecentSelectionAccepted(const QList<IRosterIndex *> &ASelected) const
{
	foreach(IRosterIndex *index, ASelected)
		if (index->kind() != RIK_RECENT_ITEM)
			return false;
	return !ASelected.isEmpty();
}

bool RecentContacts::hasProxiedIndexes(const QList<IRosterIndex *> &AIndexes) const
{
	foreach(IRosterIndex *index, AIndexes)
		if (FIndexToProxy.contains(index))
			return true;
	return false;
}

QList<IRosterIndex *> RecentContacts::indexesProxies(const QList<IRosterIndex *> &AIndexes, bool ASelfProxy) const
{
	QList<IRosterIndex *> proxies;
	foreach(IRosterIndex *index, AIndexes)
	{
		if (FIndexToProxy.contains(index))
			proxies.append(FIndexToProxy.value(index));
		else if (ASelfProxy)
			proxies.append(index);
	}
	proxies.removeAll(NULL);
	return proxies;
}

void RecentContacts::removeRecentItems(const QStringList &ATypes, const QStringList &AStreamJids, const QStringList &AReferences)
{
	for (int index=0; index<ATypes.count(); index++)
	{
		IRecentItem item;
		item.type = ATypes.value(index);
		item.streamJid = AStreamJids.value(index);
		item.reference = AReferences.value(index);
		removeItem(item);
	}
}

void RecentContacts::setItemsFavorite(bool AFavorite, const QStringList &ATypes, const QStringList &AStreamJids, const QStringList &AReferences)
{
	for (int index=0; index<ATypes.count(); index++)
	{
		IRecentItem item;
		item.type = ATypes.value(index);
		item.streamJid = AStreamJids.value(index);
		item.reference = AReferences.value(index);
		setItemProperty(item,REIP_FAVORITE,AFavorite);
	}
}

void RecentContacts::onRostersModelStreamAdded(const Jid &AStreamJid)
{
	if (FRootIndex && FStreamItems.isEmpty())
		FRostersModel->insertRosterIndex(FRootIndex,FRostersModel->rootIndex());

	FStreamItems[AStreamJid].clear();
	mergeRecentItems(AStreamJid,loadItemsFromFile(recentFileName(AStreamJid)),true);
}

void RecentContacts::onRostersModelStreamRemoved(const Jid &AStreamJid)
{
	saveItemsToFile(recentFileName(AStreamJid),FStreamItems.take(AStreamJid));

	FSaveStreams -= AStreamJid;
	updateVisibleItems();
	
	if (FRootIndex && FStreamItems.isEmpty())
		FRootIndex->remove(false);
}

void RecentContacts::onRostersModelStreamJidChanged(const Jid &ABefore, const Jid &AAfter)
{
	if (FSaveStreams.contains(ABefore))
	{
		FSaveStreams -= ABefore;
		FSaveStreams += AAfter;
	}

	QList<IRecentItem> items = FStreamItems.take(ABefore);
	for (QList<IRecentItem>::iterator it=items.begin(); it!=items.end(); ++it)
	{
		IRosterIndex *index = FVisibleItems.take(*it);
		it->streamJid = AAfter;
		if (index)
		{
			index->setData(AAfter.pFull(),RDR_STREAM_JID);
			FVisibleItems.insert(*it,index);
		}
	}
	FStreamItems.insert(AAfter,items);
}

void RecentContacts::onRostersModelIndexInserted(IRosterIndex *AIndex)
{
	if (AIndex->kind() == RIK_CONTACT)
	{
		IRecentItem item = recentItemForIndex(AIndex);
		if (FVisibleItems.contains(item))
			emit recentItemUpdated(item);
	}
}

void RecentContacts::onRostersModelIndexDataChanged(IRosterIndex *AIndex, int ARole)
{
	if (FProxyToIndex.contains(AIndex))
	{
		if (AIndex->kind() == RIK_CONTACT)
		{
			static const QList<int> updateItemRoles = QList<int>() << RDR_SHOW << RDR_PRIORITY;

			if (updateItemRoles.contains(ARole))
				emit recentItemUpdated(recentItemForIndex(AIndex));
		}
		emit rosterDataChanged(FProxyToIndex.value(AIndex),ARole);
	}
}

void RecentContacts::onRostersModelIndexRemoving(IRosterIndex *AIndex)
{
	IRosterIndex *index = FProxyToIndex.take(AIndex);
	if (index)
	{
		FIndexToProxy.remove(index);
		FIndexProxies[index].removeAll(AIndex);
	}
	onRostersModelIndexInserted(AIndex);
}

void RecentContacts::onPrivateStorageOpened(const Jid &AStreamJid)
{
	QString id = FPrivateStorage->loadData(AStreamJid,PST_RECENTCONTACTS,PSN_RECENTCONTACTS);
	if (!id.isEmpty())
	{
		FLoadRequestId[AStreamJid] = id;
		LOG_STRM_INFO(AStreamJid,"Recent items load request sent");
	}
	else
	{
		LOG_STRM_WARNING(AStreamJid,"Failed to send load roster items request");
	}
}

void RecentContacts::onPrivateStorageDataLoaded(const QString &AId, const Jid &AStreamJid, const QDomElement &AElement)
{
	if (AElement.tagName()==PST_RECENTCONTACTS && AElement.namespaceURI()==PSN_RECENTCONTACTS)
	{
		if (FLoadRequestId.value(AStreamJid) == AId)
		{
			FLoadRequestId.remove(AStreamJid);
			LOG_STRM_INFO(AStreamJid,"Recent items loaded");
			mergeRecentItems(AStreamJid,loadItemsFromXML(AElement,true),true);

			FReadyStreams.append(AStreamJid);
			emit recentContactsOpened(AStreamJid);
		}
		else
		{
			LOG_STRM_INFO(AStreamJid,"Recent items updated");
			mergeRecentItems(AStreamJid,loadItemsFromXML(AElement,true),true);
		}
	}
}

void RecentContacts::onPrivateStorageDataChanged(const Jid &AStreamJid, const QString &ATagName, const QString &ANamespace)
{
	if (ATagName==PST_RECENTCONTACTS && ANamespace==PSN_RECENTCONTACTS)
		FPrivateStorage->loadData(AStreamJid,PST_RECENTCONTACTS,PSN_RECENTCONTACTS);
}

void RecentContacts::onPrivateStorageNotifyAboutToClose(const Jid &AStreamJid)
{
	if (isReady(AStreamJid))
	{
		saveItemsToStorage(AStreamJid);
		FSaveStreams -= AStreamJid;
	}
}

void RecentContacts::onPrivateStorageClosed(const Jid &AStreamJid)
{
	FReadyStreams.removeAll(AStreamJid);
	emit recentContactsClosed(AStreamJid);
}

void RecentContacts::onRostersViewIndexContextMenuAboutToShow()
{
	Menu *menu = qobject_cast<Menu *>(sender());
	Menu *proxyMenu = FProxyContextMenu.value(menu);
	if (proxyMenu != NULL)
	{
		// Emit aboutToShow in proxyMenu
		proxyMenu->setMaximumSize(0,0);
		proxyMenu->popup(QPoint(0,0));

		QStringList proxyCaptions;
		QList<Action *> proxyActions;
		foreach(Action *action, proxyMenu->actions())
		{
			proxyActions.append(action);
			proxyCaptions.append(action->text());
			int proxyGroup = proxyMenu->actionGroup(action);

			proxyMenu->removeAction(action);
			menu->addAction(action,proxyGroup);
		}

		foreach(Action *action, menu->actions())
		{
			if (proxyCaptions.contains(action->text()) && !proxyActions.contains(action))
				menu->removeAction(action);
		}

		proxyMenu->hide();
	}
}

void RecentContacts::onRostersViewIndexMultiSelection(const QList<IRosterIndex *> &ASelected, bool &AAccepted)
{
	AAccepted = AAccepted || isSelectionAccepted(ASelected);
}

void RecentContacts::onRostersViewIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
	static bool blocked = false;
	if (!blocked && ALabelId==AdvancedDelegateItem::DisplayId && isSelectionAccepted(AIndexes))
	{
		bool allReady = true;
		bool allFavorite = true;
		bool anyFavorite = false;
		QMap<int, QStringList> rolesMap;
		foreach(IRosterIndex *index, AIndexes)
		{
			IRecentItem item = rosterIndexItem(index);

			if (itemProperty(item,REIP_FAVORITE).toBool())
				anyFavorite = true;
			else
				allFavorite = false;

			rolesMap[RDR_RECENT_TYPE].append(item.type);
			rolesMap[RDR_STREAM_JID].append(item.streamJid.full());
			rolesMap[RDR_RECENT_REFERENCE].append(item.reference);
			allReady = allReady && isReady(item.streamJid);
		}

		if (allReady)
		{
			QHash<int,QVariant> data;
			data.insert(ADR_RECENT_TYPE,rolesMap.value(RDR_RECENT_TYPE));
			data.insert(ADR_STREAM_JID,rolesMap.value(RDR_STREAM_JID));
			data.insert(ADR_RECENT_REFERENCE,rolesMap.value(RDR_RECENT_REFERENCE));

			if (!allFavorite)
			{
				Action *insertFavorite = new Action(AMenu);
				insertFavorite->setText(tr("Add to Favorites"));
				insertFavorite->setIcon(RSR_STORAGE_MENUICONS,MNI_RECENT_INSERT_FAVORITE);
				insertFavorite->setData(data);
				insertFavorite->setShortcutId(SCT_ROSTERVIEW_INSERTFAVORITE);
				connect(insertFavorite,SIGNAL(triggered(bool)),SLOT(onInsertToFavoritesByAction()));
				AMenu->addAction(insertFavorite,AG_RVCM_RECENT_FAVORITES);

			}
			if (anyFavorite)
			{
				Action *removeFavorite = new Action(AMenu);
				removeFavorite->setText(tr("Remove from Favorites"));
				removeFavorite->setIcon(RSR_STORAGE_MENUICONS,MNI_RECENT_REMOVE_FAVORITE);
				removeFavorite->setData(data);
				removeFavorite->setShortcutId(SCT_ROSTERVIEW_REMOVEFAVORITE);
				connect(removeFavorite,SIGNAL(triggered(bool)),SLOT(onRemoveFromFavoritesByAction()));
				AMenu->addAction(removeFavorite,AG_RVCM_RECENT_FAVORITES);
			}
			if (isRecentSelectionAccepted(AIndexes))
			{
				Action *removeRecent = new Action(AMenu);
				removeRecent->setText(tr("Remove from Recent Contacts"));
				removeRecent->setIcon(RSR_STORAGE_MENUICONS,MNI_RECENT_REMOVE_RECENT);
				removeRecent->setData(data);
				connect(removeRecent,SIGNAL(triggered(bool)),SLOT(onRemoveFromRecentByAction()));
				AMenu->addAction(removeRecent,AG_RVCM_RECENT_FAVORITES);
			}
		}

		if (hasProxiedIndexes(AIndexes))
		{
			QList<IRosterIndex *> proxies = indexesProxies(AIndexes);
			if (!proxies.isEmpty())
			{
				blocked = true;

				Menu *proxyMenu = new Menu(AMenu);
				FProxyContextMenu.insert(AMenu,proxyMenu);
				FRostersView->contextMenuForIndex(proxies,NULL,proxyMenu);
				connect(AMenu,SIGNAL(aboutToShow()),SLOT(onRostersViewIndexContextMenuAboutToShow()),Qt::UniqueConnection);

				blocked = false;
			}
		}
	}
}

void RecentContacts::onRostersViewIndexToolTips(IRosterIndex *AIndex, quint32 ALabelId, QMap<int, QString> &AToolTips)
{
	if (ALabelId==AdvancedDelegateItem::DisplayId && AIndex->kind()==RIK_RECENT_ITEM)
	{
		IRosterIndex *proxy = FIndexToProxy.value(AIndex);
		if (proxy)
			FRostersView->toolTipsForIndex(proxy,NULL,AToolTips);

		if (FRostersModel && FRostersModel->streamsLayout()==IRostersModel::LayoutSeparately)
		{
			Jid streamJid = AIndex->data(RDR_STREAM_JID).toString();
			IAccount *account = FAccountManager!=NULL ? FAccountManager->findAccountByStream(streamJid) : NULL;
			AToolTips.insert(RTTO_ROSTERSVIEW_INFO_ACCOUNT,tr("<b>Account:</b> %1").arg(HTML_ESCAPE(account!=NULL ? account->name() : streamJid.uBare())));
		}
	}
}

void RecentContacts::onRostersViewNotifyInserted(int ANotifyId)
{
	QList<IRosterIndex *> indexes;
	foreach(IRosterIndex *proxy, FRostersView->notifyIndexes(ANotifyId))
	{
		if (!FIndexProxies.contains(proxy))
		{
			foreach(IRosterIndex *index, FIndexProxies.keys())
				if (FIndexProxies.value(index).contains(proxy))
					indexes.append(index);
		}
	}
	
	if (!indexes.isEmpty())
	{
		int notifyId = FRostersView->insertNotify(FRostersView->notifyById(ANotifyId),indexes);
		FProxyToIndexNotify.insert(ANotifyId,notifyId);
	}
}

void RecentContacts::onRostersViewNotifyRemoved(int ANotifyId)
{
	int notifyId = FProxyToIndexNotify.take(ANotifyId);
	if (notifyId > 0)
		FRostersView->removeNotify(notifyId);
}

void RecentContacts::onRostersViewNotifyActivated(int ANotifyId)
{
	int notifyId = FProxyToIndexNotify.key(ANotifyId);
	if (notifyId > 0)
		FRostersView->activateNotify(notifyId);
}

void RecentContacts::onHandlerRecentItemUpdated(const IRecentItem &AItem)
{
	IRecentItemHandler *handler = FItemHandlers.value(AItem.type);
	if (handler)
	{
		bool show = handler->recentItemCanShow(AItem);
		bool visible = FVisibleItems.contains(AItem);
		if (show != visible)
		{
			updateVisibleItems();
		}
		else if (visible)
		{
			updateItemProxy(AItem);
			updateItemIndex(AItem);
		}
	}
	else
	{
		LOG_ERROR(QString("Failed to process recent item update, type=%1: Handler not found").arg(AItem.type));
	}
}

void RecentContacts::onRemoveFromRecentByAction()
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
		removeRecentItems(action->data(ADR_RECENT_TYPE).toStringList(),action->data(ADR_STREAM_JID).toStringList(),action->data(ADR_RECENT_REFERENCE).toStringList());
}

void RecentContacts::onInsertToFavoritesByAction()
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
		setItemsFavorite(true,action->data(ADR_RECENT_TYPE).toStringList(),action->data(ADR_STREAM_JID).toStringList(),action->data(ADR_RECENT_REFERENCE).toStringList());
}

void RecentContacts::onRemoveFromFavoritesByAction()
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
		setItemsFavorite(false,action->data(ADR_RECENT_TYPE).toStringList(),action->data(ADR_STREAM_JID).toStringList(),action->data(ADR_RECENT_REFERENCE).toStringList());
}

void RecentContacts::onSaveItemsToStorageTimerTimeout()
{
	foreach(const Jid &streamJid, FSaveStreams)
		saveItemsToStorage(streamJid);
	FSaveStreams.clear();
}

void RecentContacts::onShortcutActivated(const QString &AId, QWidget *AWidget)
{
	if (FRostersModel && FRostersView && AWidget==FRostersView->instance())
	{
		QList<IRosterIndex *> indexes = FRostersView->selectedRosterIndexes();
		if (AId==SCT_ROSTERVIEW_INSERTFAVORITE || AId==SCT_ROSTERVIEW_REMOVEFAVORITE)
		{
			if (isSelectionAccepted(indexes))
			{
				QMap<int, QStringList> rolesMap;
				foreach(IRosterIndex *index, indexes)
				{
					IRecentItem item = rosterIndexItem(index);
					rolesMap[RDR_RECENT_TYPE].append(item.type);
					rolesMap[RDR_STREAM_JID].append(item.streamJid.full());
					rolesMap[RDR_RECENT_REFERENCE].append(item.reference);
				}
				setItemsFavorite(AId==SCT_ROSTERVIEW_INSERTFAVORITE,rolesMap.value(RDR_RECENT_TYPE),rolesMap.value(RDR_STREAM_JID),rolesMap.value(RDR_RECENT_REFERENCE));
			}
		}
		else if (hasProxiedIndexes(indexes))
		{
			QList<IRosterIndex *> proxies = indexesProxies(indexes);
			if (!proxies.isEmpty() && FRostersView->isSelectionAcceptable(proxies))
			{
				FRostersView->setSelectedRosterIndexes(proxies);
				Shortcuts::activateShortcut(AId,AWidget);
				FRostersView->setSelectedRosterIndexes(indexes);
			}
		}
	}
}

void RecentContacts::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_ROSTER_RECENT_ALWAYSSHOWOFFLINE));
	onOptionsChanged(Options::node(OPV_ROSTER_RECENT_HIDEINACTIVEITEMS));
	onOptionsChanged(Options::node(OPV_ROSTER_RECENT_SIMPLEITEMSVIEW));
	onOptionsChanged(Options::node(OPV_ROSTER_RECENT_SORTBYACTIVETIME));
	onOptionsChanged(Options::node(OPV_ROSTER_RECENT_SHOWONLYFAVORITE));
	onOptionsChanged(Options::node(OPV_ROSTER_RECENT_MAXVISIBLEITEMS));
	onOptionsChanged(Options::node(OPV_ROSTER_RECENT_INACTIVEDAYSTIMEOUT));
}

void RecentContacts::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_ROSTER_RECENT_ALWAYSSHOWOFFLINE)
	{
		FAllwaysShowOffline = ANode.value().toBool();
		foreach(IRosterIndex *index, FVisibleItems.values())
			emit rosterDataChanged(index,RDR_FORCE_VISIBLE);
	}
	else if (ANode.path() == OPV_ROSTER_RECENT_HIDEINACTIVEITEMS)
	{
		FHideLaterContacts = ANode.value().toBool();
		updateVisibleItems();
	}
	else if (ANode.path() == OPV_ROSTER_RECENT_SIMPLEITEMSVIEW)
	{
		FSimpleContactsView = ANode.value().toBool();
		// *** <<< eyeCU <<< ***
		emit rosterLabelChanged(RLID_AVATAR_IMAGE_LEFT);
		emit rosterLabelChanged(RLID_AVATAR_IMAGE_RIGHT);
		// *** >>> eyeCU >>> ***
		emit rosterLabelChanged(RLID_ROSTERSVIEW_STATUS);
	}
	else if (ANode.path() == OPV_ROSTER_RECENT_SORTBYACTIVETIME)
	{
		FSortByLastActivity = ANode.value().toBool();
		foreach(const IRecentItem &item, FVisibleItems.keys())
			updateItemIndex(item);
	}
	else if (ANode.path() == OPV_ROSTER_RECENT_SHOWONLYFAVORITE)
	{
		FShowOnlyFavorite = ANode.value().toBool();
		updateVisibleItems();
	}
	else if (ANode.path() == OPV_ROSTER_RECENT_MAXVISIBLEITEMS)
	{
		FMaxVisibleItems = qMin(qMax(ANode.value().toInt(),MIN_VISIBLE_CONTACTS),MAX_STORAGE_CONTACTS);
		updateVisibleItems();
	}
	else if (ANode.path() == OPV_ROSTER_RECENT_INACTIVEDAYSTIMEOUT)
	{
		FInactiveDaysTimeout = qMin(qMax(ANode.value().toInt(),MIN_INACTIVE_TIMEOUT),MAX_INACTIVE_TIMEOUT);
		updateVisibleItems();
	}
}

uint qHash(const IRecentItem &AKey)
{
	return qHash(AKey.type+"~"+AKey.streamJid.pFull()+"~"+AKey.reference);
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_recentcontacts, RecentContacts)
#endif
