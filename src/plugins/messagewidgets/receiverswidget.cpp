#include "receiverswidget.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDomDocument>
#include <definitions/actiongroups.h>
#include <definitions/rosterindexroles.h>
#include <definitions/rosterindexkinds.h>
#include <definitions/rosterindexkindorders.h>
#include <utils/advanceditemdelegate.h>
#include <utils/pluginhelper.h>
#include <utils/options.h>
#include <utils/qt4qt5compat.h>

#define ADR_ITEMS                  Action::DR_Parametr1

#define RIDR_ITEM_COLLAPSED        RDR_USER_ROLE+111

static const QList<int> GroupKinds = QList<int>() << RIK_STREAM_ROOT << RIK_GROUP;

ReceiversProxyModel::ReceiversProxyModel(QObject *AParent) : QSortFilterProxyModel(AParent)
{
	FOfflineVisible = true;
	FSortMode = IMessageReceiversWidget::SortByStatus;

	setSortLocaleAware(true);
	setDynamicSortFilter(true);
	setSortCaseSensitivity(Qt::CaseInsensitive);
	setFilterCaseSensitivity(Qt::CaseInsensitive);
}

int ReceiversProxyModel::sortMode() const
{
	return FSortMode;
}

void ReceiversProxyModel::setSortMode(int AMode)
{
	if (FSortMode != AMode)
	{
		FSortMode = AMode;
		invalidate();
	}
}

bool ReceiversProxyModel::isOfflineContactsVisible() const
{
	return FOfflineVisible;
}

void ReceiversProxyModel::setOfflineContactsVisible(bool AVisible)
{
	if (FOfflineVisible != AVisible)
	{
		FOfflineVisible = AVisible;
		invalidateFilter();
	}
}

bool ReceiversProxyModel::lessThan(const QModelIndex &ALeft, const QModelIndex &ARight) const
{
	int leftTypeOrder = ALeft.data(RDR_KIND_ORDER).toInt();
	int rightTypeOrder = ARight.data(RDR_KIND_ORDER).toInt();
	if (leftTypeOrder == rightTypeOrder)
	{
		int leftSortOrder = ALeft.data(RDR_SORT_ORDER).toInt();
		int rightSortOrder = ARight.data(RDR_SORT_ORDER).toInt();
		if (leftSortOrder == rightSortOrder)
		{
			if (FSortMode==IMessageReceiversWidget::SortByStatus && leftTypeOrder!=RIKO_STREAM_ROOT)
			{
				int leftShow = ALeft.data(RDR_SHOW).toInt();
				int rightShow = ARight.data(RDR_SHOW).toInt();
				if (leftShow != rightShow)
				{
					static const int showOrders[] = {6,2,1,3,4,5,7,8};
					static const int showOrdersCount = sizeof(showOrders)/sizeof(showOrders[0]);
					if (leftShow<showOrdersCount && rightShow<showOrdersCount)
						return showOrders[leftShow] < showOrders[rightShow];
				}
			}
			return QSortFilterProxyModel::lessThan(ALeft,ARight);
		}
		return leftSortOrder < rightSortOrder;
	}
	return leftTypeOrder < rightTypeOrder;
}

bool ReceiversProxyModel::filterAcceptsRow(int AModelRow, const QModelIndex &AModelParent) const
{
	QAbstractItemModel *source = sourceModel();

	QModelIndex index = source->index(AModelRow,0,AModelParent);
	if (GroupKinds.contains(index.data(RDR_KIND).toInt()))
	{
		for (int childRow=0; source->index(childRow,0,index).isValid(); childRow++)
			if (filterAcceptsRow(childRow,index))
				return true;
		return false;
	}
	else if (!FOfflineVisible && filterRegExp().isEmpty() && index.data(RDR_SHOW).isValid())
	{
		int show = index.data(RDR_SHOW).toInt();
		if (show==IPresence::Offline || show==IPresence::Error)
			return false;
	}
	
	return QSortFilterProxyModel::filterAcceptsRow(AModelRow,AModelParent);
}

ReceiversWidget::ReceiversWidget(IMessageWidgets *AMessageWidgets, IMessageWindow *AWindow, QWidget *AParent) : QWidget(AParent)
{
	ui.setupUi(this);
	setWindowIconText(tr("Receivers"));
	
	qRegisterMetaType< QList<QStandardItem *> >("QList<QStandardItem *>");

	FWindow = AWindow;
	FMessageWidgets = AMessageWidgets;

	FPresenceManager = PluginHelper::pluginInstance<IPresenceManager>();
	if (FPresenceManager)
	{
		connect(FPresenceManager->instance(),SIGNAL(presenceOpened(IPresence *)),SLOT(onPresenceOpened(IPresence *)));
		connect(FPresenceManager->instance(),SIGNAL(presenceClosed(IPresence *)),SLOT(onPresenceClosed(IPresence *)));
		connect(FPresenceManager->instance(),SIGNAL(presenceItemReceived(IPresence *, const IPresenceItem &, const IPresenceItem &)),
			SLOT(onPresenceItemReceived(IPresence *, const IPresenceItem &, const IPresenceItem &)));
	}

	FRosterManager = PluginHelper::pluginInstance<IRosterManager>();
	if (FRosterManager)
	{
		connect(FRosterManager->instance(),SIGNAL(rosterItemReceived(IRoster *, const IRosterItem &, const IRosterItem &)),
			SLOT(onRosterItemReceived(IRoster *, const IRosterItem &, const IRosterItem &)));
	}

	FMessageProcessor = PluginHelper::pluginInstance<IMessageProcessor>();
	if (FMessageProcessor)
	{
		connect(FMessageProcessor->instance(),SIGNAL(activeStreamAppended(const Jid &)),SLOT(onActiveStreamAppended(const Jid &)));
		connect(FMessageProcessor->instance(),SIGNAL(activeStreamRemoved(const Jid &)),SLOT(onActiveStreamRemoved(const Jid &)));
	}

	FStatusIcons = PluginHelper::pluginInstance<IStatusIcons>();
	FRostersModel = PluginHelper::pluginInstance<IRostersModel>();
	FAccountManager = PluginHelper::pluginInstance<IAccountManager>();

	AdvancedItemDelegate *itemDelegate = new AdvancedItemDelegate(this);
	itemDelegate->setItemsRole(RDR_LABEL_ITEMS);
	ui.trvReceivers->setItemDelegate(itemDelegate);

	FModel = new AdvancedItemModel(this);
	FModel->setDelayedDataChangedSignals(true);
	FModel->setRecursiveParentDataChangedSignals(true);
	connect(FModel,SIGNAL(itemInserted(QStandardItem *)),SLOT(onModelItemInserted(QStandardItem *)));
	connect(FModel,SIGNAL(itemRemoving(QStandardItem *)),SLOT(onModelItemRemoving(QStandardItem *)));
	connect(FModel,SIGNAL(itemDataChanged(QStandardItem *,int)),SLOT(onModelItemDataChanged(QStandardItem *,int)));

	FProxyModel = new ReceiversProxyModel(this);
	FProxyModel->sort(0,Qt::AscendingOrder);

	FSelectionSignalTimer.setSingleShot(true);
	FSelectionSignalTimer.setInterval(0);
	connect(&FSelectionSignalTimer,SIGNAL(timeout()),SIGNAL(addressSelectionChanged()));

	foreach(const Jid &streamJid, FMessageProcessor!=NULL ? FMessageProcessor->activeStreams() : QList<Jid>())
		onActiveStreamAppended(streamJid);

	connect(ui.sleSearch,SIGNAL(searchStart()),SLOT(onStartSearchContacts()));
	connect(ui.trvReceivers,SIGNAL(collapsed(const QModelIndex &)),SLOT(onViewIndexCollapsed(const QModelIndex &)));
	connect(ui.trvReceivers,SIGNAL(expanded(const QModelIndex &)),SLOT(onViewIndexExpanded(const QModelIndex &)));
	connect(ui.trvReceivers,SIGNAL(customContextMenuRequested(const QPoint &)),SLOT(onViewContextMenuRequested(const QPoint &)));

	insertProxyModel(FProxyModel);
}

ReceiversWidget::~ReceiversWidget()
{

}

bool ReceiversWidget::isVisibleOnWindow() const
{
	return FWindow!=NULL ? isVisibleTo(FWindow->instance()) : false;
}

IMessageWindow *ReceiversWidget::messageWindow() const
{
	return FWindow;
}

QList<Jid> ReceiversWidget::availStreams() const
{
	return FStreamItems.keys();
}

QTreeView *ReceiversWidget::receiversView() const
{
	return ui.trvReceivers;
}

AdvancedItemModel *ReceiversWidget::receiversModel() const
{
	return FModel;
}

QList<QAbstractProxyModel *> ReceiversWidget::proxyModels() const
{
	return FProxyModels;
}

void ReceiversWidget::insertProxyModel(QAbstractProxyModel *AProxy)
{
	if (AProxy!=NULL && !FProxyModels.contains(AProxy))
	{
		emit proxyModelsAboutToBeChanged();

		if (ui.trvReceivers->model())
			disconnect(ui.trvReceivers->model(),SIGNAL(rowsInserted(const QModelIndex &, int , int )),this,SLOT(onViewModelRowsInserted(const QModelIndex &, int , int )));

		AProxy->setSourceModel(FModel);

		bool viewModelChanged = false;
		QAbstractProxyModel *firstProxy = FProxyModels.value(0);
		if (firstProxy != NULL)
		{
			firstProxy->setSourceModel(NULL); // fix bug in QSortFilterProxyModel
			firstProxy->setSourceModel(AProxy);
		}
		else
		{
			viewModelChanged = true;
			ui.trvReceivers->setModel(AProxy);
		}

		FProxyModels.prepend(AProxy);

		if (ui.trvReceivers->model())
			connect(ui.trvReceivers->model(),SIGNAL(rowsInserted(const QModelIndex &, int , int )),this,SLOT(onViewModelRowsInserted(const QModelIndex &, int , int )));

		restoreExpandState(FModel->invisibleRootItem());
		emit proxyModelsChanged(viewModelChanged);
	}
}

void ReceiversWidget::removeProxyModel(QAbstractProxyModel *AProxy)
{
	int index = FProxyModels.indexOf(AProxy);
	if (index >= 0)
	{
		emit proxyModelsAboutToBeChanged();

		if (ui.trvReceivers->model())
			disconnect(ui.trvReceivers->model(),SIGNAL(rowsInserted(const QModelIndex &, int , int )),this,SLOT(onViewModelRowsInserted(const QModelIndex &, int , int )));

		FProxyModels.removeAt(index);

		bool viewModelChanged = false;
		if (FProxyModels.isEmpty())
		{
			// No more proxy models
			ui.trvReceivers->setModel(FModel);
			viewModelChanged = true;
		}
		else if (index == FProxyModels.count())
		{
			// Last proxy model was removed
			ui.trvReceivers->setModel(FProxyModels.last());
			viewModelChanged = true;
		}
		else if (index == 0)
		{
			// First proxy model was removed
			FProxyModels[0]->setSourceModel(FModel);
		}
		else
		{
			// Middle proxy model was removed
			FProxyModels[index]->setSourceModel(FProxyModels[index-1]);
		}

		if (ui.trvReceivers->model())
			connect(ui.trvReceivers->model(),SIGNAL(rowsInserted(const QModelIndex &, int , int )),this,SLOT(onViewModelRowsInserted(const QModelIndex &, int , int )));

		restoreExpandState(FModel->invisibleRootItem());
		emit proxyModelsChanged(viewModelChanged);
	}
}

QModelIndex ReceiversWidget::mapModelToView(QStandardItem *AItem)
{
	QModelIndex index = FModel->indexFromItem(AItem);
	for (int i=0; i<FProxyModels.count(); i++)
		index = FProxyModels.at(i)->mapFromSource(index);
	return index;
}

QStandardItem *ReceiversWidget::mapViewToModel(const QModelIndex &AIndex)
{
	QModelIndex index = AIndex;
	for (int i=FProxyModels.count()-1; i>=0; i--)
		index = FProxyModels.at(i)->mapToSource(index);
	return FModel->itemFromIndex(index);
}

void ReceiversWidget::contextMenuForItems(QList<QStandardItem *> AItems, Menu *AMenu)
{
	bool allHasChildren = true;
	foreach(QStandardItem *item, AItems)
	{
		if (!item->hasChildren())
			allHasChildren = false;
	}

	if (allHasChildren)
	{
		QVariant items;
		items.setValue(AItems);

		Action *selectAll = new Action(AMenu);
		selectAll->setText(tr("Select All Contacts"));
		selectAll->setData(ADR_ITEMS,items);
		connect(selectAll,SIGNAL(triggered()),SLOT(onSelectAllContacts()));
		AMenu->addAction(selectAll,AG_MWRWCM_MWIDGETS_SELECT_ALL);

		Action *selectOnline = new Action(AMenu);
		selectOnline->setText(tr("Select Online Contact"));
		selectOnline->setData(ADR_ITEMS,items);
		connect(selectOnline,SIGNAL(triggered()),SLOT(onSelectOnlineContacts()));
		AMenu->addAction(selectOnline,AG_MWRWCM_MWIDGETS_SELECT_ONLINE);

		Action *selectNotBusy = new Action(AMenu);
		selectNotBusy->setText(tr("Select Available Contacts"));
		selectNotBusy->setData(ADR_ITEMS,items);
		connect(selectNotBusy,SIGNAL(triggered()),SLOT(onSelectNotBusyContacts()));
		AMenu->addAction(selectNotBusy,AG_MWRWCM_MWIDGETS_SELECT_NOTBUSY);

		Action *selectNone = new Action(AMenu);
		selectNone->setText(tr("Clear Selection"));
		selectNone->setData(ADR_ITEMS,items);
		connect(selectNone,SIGNAL(triggered()),SLOT(onSelectNoneContacts()));
		AMenu->addAction(selectNone,AG_MWRWCM_MWIDGETS_SELECT_CLEAR);

		Action *expandAll = new Action(AMenu);
		expandAll->setText(tr("Expand All Groups"));
		expandAll->setData(ADR_ITEMS,items);
		connect(expandAll,SIGNAL(triggered()),SLOT(onExpandAllChilds()));
		AMenu->addAction(expandAll,AG_MWRWCM_MWIDGETS_EXPAND_ALL);

		Action *collapseAll = new Action(AMenu);
		collapseAll->setText(tr("Collapse All Groups"));
		collapseAll->setData(ADR_ITEMS,items);
		connect(collapseAll,SIGNAL(triggered()),SLOT(onCollapseAllChilds()));
		AMenu->addAction(collapseAll,AG_MWRWCM_MWIDGETS_COLLAPSE_ALL);

		if (AItems.first() == FModel->invisibleRootItem())
		{
			Action *selectLast = new Action(AMenu);
			selectLast->setText(tr("Load Last Selection"));
			selectLast->setEnabled(QFile::exists(Options::fileValue("messagewidgets.receiverswidget.last-selection").toString()));
			connect(selectLast,SIGNAL(triggered()),SLOT(onSelectionLast()));
			AMenu->addAction(selectLast,AG_MWRWCM_MWIDGETS_SELECT_LAST);

			Action *selectLoad = new Action(AMenu);
			selectLoad->setText(tr("Load Selection"));
			connect(selectLoad,SIGNAL(triggered()),SLOT(onSelectionLoad()));
			AMenu->addAction(selectLoad,AG_MWRWCM_MWIDGETS_SELECT_LOAD);

			Action *selectSave = new Action(AMenu);
			selectSave->setText(tr("Save Selection"));
			connect(selectSave,SIGNAL(triggered()),SLOT(onSelectionSave()));
			AMenu->addAction(selectSave,AG_MWRWCM_MWIDGETS_SELECT_SAVE);

			Action *hideOffline = new Action(AMenu);
			hideOffline->setText(tr("Hide Offline Contacts"));
			hideOffline->setCheckable(true);
			hideOffline->setChecked(!isOfflineContactsVisible());
			connect(hideOffline,SIGNAL(triggered()),SLOT(onHideOfflineContacts()));
			AMenu->addAction(hideOffline,AG_MWRWCM_MWIDGETS_HIDE_OFFLINE);

			Action *sortByStatus = new Action(AMenu);
			sortByStatus->setText(tr("Sort Contacts by Status"));
			sortByStatus->setCheckable(true);
			sortByStatus->setChecked(sortMode() == IMessageReceiversWidget::SortByStatus);
			connect(sortByStatus,SIGNAL(triggered()),SLOT(onSortContactByStatus()));
			AMenu->addAction(sortByStatus,AG_MWRWCM_MWIDGETS_SORT_BY_STATUS);
		}
	}

	emit contextMenuForItemsRequested(AItems,AMenu);
}

int ReceiversWidget::sortMode() const
{
	return FProxyModel->sortMode();
}

void ReceiversWidget::setSortMode(int AMode)
{
	FProxyModel->setSortMode(AMode);
}

bool ReceiversWidget::isOfflineContactsVisible() const
{
	return FProxyModel->isOfflineContactsVisible();
}

void ReceiversWidget::setOfflineContactsVisible(bool AVisible)
{
	if (FProxyModel->isOfflineContactsVisible() != AVisible)
	{
		FProxyModel->setOfflineContactsVisible(AVisible);
		restoreExpandState(FModel->invisibleRootItem());
	}
}

QMultiMap<Jid, Jid> ReceiversWidget::selectedAddresses() const
{
	QMultiMap<Jid, Jid> addresses;
	for (QMap<Jid, QMultiHash<Jid, QStandardItem *> >::const_iterator streamIt=FContactItems.constBegin(); streamIt!=FContactItems.constEnd(); ++streamIt)
	{
		for (QMultiHash<Jid, QStandardItem *>::const_iterator contactIt=streamIt->constBegin(); contactIt!=streamIt->constEnd(); ++contactIt)
		{
			if (contactIt.value()->checkState() == Qt::Checked)
			{
				if (!addresses.contains(streamIt.key(),contactIt.key()))
					addresses.insertMulti(streamIt.key(),contactIt.key());
			}
		}
	}
	return addresses;
}

void ReceiversWidget::setGroupSelection(const Jid &AStreamJid, const QString &AGroup, bool ASelected)
{
	QString group = AGroup.isEmpty() ? (FRostersModel!=NULL ? FRostersModel->singleGroupName(RIK_GROUP_BLANK) : tr("Without Groups")) : AGroup;
	QStandardItem *groupItem = FGroupItems.value(AStreamJid).value(group);
	if (groupItem)
		groupItem->setCheckState(ASelected ? Qt::Checked : Qt::Unchecked);
}

void ReceiversWidget::setAddressSelection(const Jid &AStreamJid, const Jid &AContactJid, bool ASelected)
{
	QList<QStandardItem *> contactItems = findContactItems(AStreamJid,AContactJid);
	if (ASelected && contactItems.isEmpty() && FStreamItems.contains(AStreamJid) && AContactJid.isValid())
	{
		QString group = FRostersModel!=NULL ? FRostersModel->singleGroupName(RIK_GROUP_NOT_IN_ROSTER) : tr("Not in Roster");
		QStandardItem *contactItem = getContactItem(AStreamJid,AContactJid,AContactJid.uBare(),group,RIKO_GROUP_NOT_IN_ROSTER);
		updateContactItemsPresence(AStreamJid,AContactJid);
		contactItems.append(contactItem);
	}

	foreach(QStandardItem *contactItem, contactItems)
		contactItem->setCheckState(ASelected ? Qt::Checked : Qt::Unchecked);
}

void ReceiversWidget::clearSelection()
{
	for (QMap<Jid, QMultiHash<Jid, QStandardItem *> >::const_iterator streamIt=FContactItems.constBegin(); streamIt!=FContactItems.constEnd(); ++streamIt)
		for (QMultiHash<Jid, QStandardItem *>::const_iterator contactIt=streamIt->constBegin(); contactIt!=streamIt->constEnd(); ++contactIt)
			contactIt.value()->setCheckState(Qt::Unchecked);
}

void ReceiversWidget::initialize()
{
}

void ReceiversWidget::createStreamItems(const Jid &AStreamJid)
{
	if (getStreamItem(AStreamJid))
	{
		IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(AStreamJid) : NULL;
		foreach(const IRosterItem &ritem, roster!=NULL ? roster->items() : QList<IRosterItem>())
			onRosterItemReceived(roster,ritem,IRosterItem());
	}
}

void ReceiversWidget::destroyStreamItems(const Jid &AStreamJid)
{
	QStandardItem *streamItem = FStreamItems.value(AStreamJid);
	if (streamItem)
	{
		QMultiHash<Jid, QStandardItem *> contactItems = FContactItems.value(AStreamJid);
		for (QMultiHash<Jid, QStandardItem *>::const_iterator contactIt=contactItems.constBegin(); contactIt!=contactItems.constEnd(); ++contactIt)
		{
			QStandardItem *contactItem = contactIt.value();
			contactItem->setCheckState(Qt::Unchecked);
		}
		FModel->removeRow(streamItem->row());

		FStreamItems.remove(AStreamJid);
		FContactItems.remove(AStreamJid);
		FGroupItems.remove(AStreamJid);
	}
}

QStandardItem *ReceiversWidget::getStreamItem(const Jid &AStreamJid)
{
	QStandardItem *streamItem = FStreamItems.value(AStreamJid);
	if (streamItem == NULL)
	{
		IAccount *account = FAccountManager!=NULL ? FAccountManager->findAccountByStream(AStreamJid) : NULL;
		int streamOrder =  account!=NULL ? account->accountOrder() : 0;

		streamItem = new AdvancedItem();
		streamItem->setCheckable(true);
		streamItem->setData(RIK_STREAM_ROOT,RDR_KIND);
		streamItem->setData(RIKO_STREAM_ROOT,RDR_KIND_ORDER);
		streamItem->setData(streamOrder,RDR_SORT_ORDER);
		streamItem->setData(AStreamJid.pFull(),RDR_STREAM_JID);
		streamItem->setText(account!=NULL ? account->name() : AStreamJid.uBare());

		QFont streamFont = streamItem->font();
		streamFont.setBold(true);
		streamItem->setFont(streamFont);

		streamItem->setBackground(ui.trvReceivers->palette().color(QPalette::Active, QPalette::Dark));
		streamItem->setForeground(ui.trvReceivers->palette().color(QPalette::Active, QPalette::BrightText));

		FModel->invisibleRootItem()->appendRow(streamItem);
		ui.trvReceivers->expand(mapModelToView(streamItem));
	}
	return streamItem;
}

QStandardItem *ReceiversWidget::getGroupItem(const Jid &AStreamJid, const QString &AGroup, int AGroupOrder)
{
	QStandardItem *groupItem = FGroupItems.value(AStreamJid).value(AGroup);
	if (groupItem == NULL)
	{
		QStringList groupPath = AGroup.split(ROSTER_GROUP_DELIMITER);
		QString groupName = groupPath.takeLast();

		groupItem = new AdvancedItem(groupName);
		groupItem->setCheckable(true);
		groupItem->setData(RIK_GROUP,RDR_KIND);
		groupItem->setData(AGroupOrder,RDR_KIND_ORDER);
		groupItem->setData(AStreamJid.pFull(),RDR_STREAM_JID);
		groupItem->setData(AGroup,RDR_GROUP);
		groupItem->setText(groupName);

		QFont groupFont = groupItem->font();
		groupFont.setBold(true);
		groupItem->setFont(groupFont);

		groupItem->setForeground(ui.trvReceivers->palette().color(QPalette::Active, QPalette::Highlight));

		QStandardItem *parentItem = groupPath.isEmpty() ? getStreamItem(AStreamJid) : getGroupItem(AStreamJid,groupPath.join(ROSTER_GROUP_DELIMITER),AGroupOrder);
		parentItem->appendRow(groupItem);

		ui.trvReceivers->expand(mapModelToView(groupItem));
	}
	return groupItem;
}

QList<QStandardItem *> ReceiversWidget::findContactItems(const Jid &AStreamJid, const Jid &AContactJid) const
{
	return FContactItems.value(AStreamJid).values(AContactJid.bare());
}

QStandardItem *ReceiversWidget::findContactItem(const Jid &AStreamJid, const Jid &AContactJid, const QString &AGroup) const
{
	foreach(QStandardItem *item, findContactItems(AStreamJid,AContactJid))
		if (item->data(RDR_GROUP).toString() == AGroup)
			return item;
	return NULL;
}

QStandardItem *ReceiversWidget::getContactItem(const Jid &AStreamJid, const Jid &AContactJid, const QString &AName, const QString &AGroup, int AGroupOrder)
{
	QStandardItem *contactItem = findContactItem(AStreamJid,AContactJid,AGroup);
	if (contactItem == NULL)
	{
		contactItem = new AdvancedItem();
		contactItem->setCheckable(true);
		contactItem->setData(RIK_CONTACT,RDR_KIND);
		contactItem->setData(RIKO_DEFAULT,RDR_KIND_ORDER);
		contactItem->setData(AStreamJid.pFull(),RDR_STREAM_JID);
		contactItem->setData(AContactJid.full(),RDR_FULL_JID);
		contactItem->setData(AContactJid.pFull(),RDR_PREP_FULL_JID);
		contactItem->setData(AContactJid.pBare(),RDR_PREP_BARE_JID);
		contactItem->setData(AGroup,RDR_GROUP);

		contactItem->setToolTip(HTML_ESCAPE(AContactJid.uBare()));

		QStandardItem *parentItem = getGroupItem(AStreamJid,AGroup,AGroupOrder);
		parentItem->appendRow(contactItem);
	}
	contactItem->setText(AName);
	return contactItem;
}

void ReceiversWidget::deleteItemLater(QStandardItem *AItem)
{
	if (AItem && !FDeleteDelayed.contains(AItem))
	{
		FDeleteDelayed.append(AItem);
		QTimer::singleShot(0,this,SIGNAL(onDeleteDelayedItems()));
	}
}

void ReceiversWidget::updateCheckState(QStandardItem *AItem)
{
	if (AItem && AItem->hasChildren() && AItem!=FModel->invisibleRootItem())
	{
		bool allChecked = true;
		bool allUnchecked = true;
		for (int row=0; row<AItem->rowCount(); row++)
		{
			QStandardItem *childItem = AItem->child(row);
			if (!FModel->isRemovedItem(childItem))
			{
				QModelIndex index = mapModelToView(childItem);
				if (index.isValid())
				{
					allChecked = allChecked && (childItem->checkState()==Qt::Checked);
					allUnchecked = allUnchecked && (childItem->checkState()==Qt::Unchecked);
				}
			}
		}

		if (allChecked && !allUnchecked)
			AItem->setCheckState(Qt::Checked);
		else if (!allChecked && allUnchecked)
			AItem->setCheckState(Qt::Unchecked);
		else if (!allChecked && !allUnchecked)
			AItem->setCheckState(Qt::PartiallyChecked);
	}
}

void ReceiversWidget::updateContactItemsPresence(const Jid &AStreamJid, const Jid &AContactJid)
{
	IPresence *presence = FPresenceManager!=NULL ? FPresenceManager->findPresence(AStreamJid) : NULL;
	QList<IPresenceItem> pitemList = presence!=NULL ? FPresenceManager->sortPresenceItems(presence->findItems(AContactJid)) : QList<IPresenceItem>();

	QStringList resources;
	foreach(const IPresenceItem &pitem, pitemList)
	{
		if (pitem.show!=IPresence::Offline && pitem.show!=IPresence::Error)
			resources.append(pitem.itemJid.pFull());
	}

	IPresenceItem pitem = pitemList.value(0);
	foreach(QStandardItem *contactItem, findContactItems(AStreamJid,AContactJid))
	{
		contactItem->setData(pitem.show,RDR_SHOW);
		contactItem->setData(pitem.status,RDR_STATUS);
		contactItem->setData(pitem.priority,RDR_PRIORITY);
		contactItem->setData(resources,RDR_RESOURCES);
		contactItem->setIcon(FStatusIcons!=NULL ? FStatusIcons->iconByJidStatus(pitem.itemJid,pitem.show,SUBSCRIPTION_BOTH,false) : QIcon());
	}
}

Jid ReceiversWidget::findAvailStream(const Jid &AStreamJid) const
{
	foreach(const Jid &streamJid, FStreamItems.keys())
		if (streamJid.pBare() == AStreamJid.pBare())
			return streamJid;
	return Jid::null;
}

void ReceiversWidget::selectionLoad(const QString &AFileName)
{
	if (!AFileName.isEmpty())
	{
		QFile file(AFileName);
		if (file.open(QFile::ReadOnly))
		{
			QString xmlError;
			QDomDocument doc;
			if (doc.setContent(&file,true,&xmlError))
			{
				if (doc.documentElement().namespaceURI() == "vacuum:messagewidgets:receiverswidget:selection")
				{
					clearSelection();
					QDomElement streamElem = doc.documentElement().firstChildElement("stream");
					while(!streamElem.isNull())
					{
						Jid streamJid = findAvailStream(streamElem.attribute("jid"));
						if (streamJid.isValid())
						{
							QDomElement itemElem = streamElem.firstChildElement("item");
							while(!itemElem.isNull())
							{
								setAddressSelection(streamJid,itemElem.text(),true);
								itemElem = itemElem.nextSiblingElement("item");
							}
						}
						streamElem = streamElem.nextSiblingElement("stream");
					}
				}
				else
				{
					QMessageBox::critical(this,tr("Failed to Load Contacts"),tr("Incorrect file format"),QMessageBox::Ok);
				}
			}
			else
			{
				QMessageBox::critical(this,tr("Failed to Load Contacts"),tr("Failed to read file: %1").arg(xmlError),QMessageBox::Ok);
			}
		}
		else
		{
			QMessageBox::critical(this,tr("Failed to Load Contacts"),tr("Failed to open file: %1").arg(file.errorString()),QMessageBox::Ok);
		}
	}
}

void ReceiversWidget::selectionSave(const QString &AFileName)
{
	if (!AFileName.isEmpty())
	{
		QFile file(AFileName);
		if (file.open(QFile::WriteOnly))
		{
			QDomDocument doc;
			doc.appendChild(doc.createElementNS("vacuum:messagewidgets:receiverswidget:selection","addresses"));

			Jid streamJid;
			QDomElement streamElem;
			QMultiMap<Jid, Jid> addresses = selectedAddresses();
			for (QMultiMap<Jid,Jid>::const_iterator it=addresses.constBegin(); it!=addresses.constEnd(); ++it)
			{
				if (streamJid != it.key())
				{
					streamJid = it.key();
					streamElem = doc.documentElement().appendChild(doc.createElement("stream")).toElement();
					streamElem.setAttribute("jid",streamJid.bare());
				}

				QDomElement itemElem = streamElem.appendChild(doc.createElement("item")).toElement();
				itemElem.appendChild(doc.createTextNode(it->bare()));
			}

			file.write(doc.toByteArray());
			file.close();

			Options::setFileValue(AFileName,"messagewidgets.receiverswidget.last-selection");
		}
		else
		{
			QMessageBox::critical(this,tr("Failed to Save Contacts"),tr("Failed to create file: %1").arg(file.errorString()),QMessageBox::Ok);
		}
	}
}

void ReceiversWidget::selectAllContacts(QList<QStandardItem *> AParents)
{
	foreach(QStandardItem *parentItem, AParents)
	{
		for (int row=0; row<parentItem->rowCount(); row++)
		{
			QStandardItem *item = parentItem->child(row);
			if (mapModelToView(item).isValid())
			{
				if (item->data(RDR_KIND).toInt() == RIK_CONTACT)
					item->setCheckState(Qt::Checked);
				else if (item->hasChildren())
					selectAllContacts(QList<QStandardItem *>() << item);
			}
		}
	}
}

void ReceiversWidget::selectOnlineContacts(QList<QStandardItem *> AParents)
{
	foreach(QStandardItem *parentItem, AParents)
	{
		for (int row=0; row<parentItem->rowCount(); row++)
		{
			QStandardItem *item = parentItem->child(row);
			if (mapModelToView(item).isValid())
			{
				if (item->data(RDR_KIND).toInt() == RIK_CONTACT)
				{
					int show = item->data(RDR_SHOW).toInt();
					if (show!=IPresence::Offline && show!=IPresence::Error)
						item->setCheckState(Qt::Checked);
					else
						item->setCheckState(Qt::Unchecked);
				}
				else if (item->hasChildren())
				{
					selectOnlineContacts(QList<QStandardItem *>() << item);
				}
			}
		}
	}
}

void ReceiversWidget::selectNotBusyContacts(QList<QStandardItem *> AParents)
{
	foreach(QStandardItem *parentItem, AParents)
	{
		for (int row=0; row<parentItem->rowCount(); row++)
		{
			QStandardItem *item = parentItem->child(row);
			if (mapModelToView(item).isValid())
			{
				if (item->data(RDR_KIND).toInt() == RIK_CONTACT)
				{
					int show = item->data(RDR_SHOW).toInt();
					if (show!=IPresence::Offline && show!=IPresence::Error && show!=IPresence::DoNotDisturb)
						item->setCheckState(Qt::Checked);
					else
						item->setCheckState(Qt::Unchecked);
				}
				else if (item->hasChildren())
				{
					selectNotBusyContacts(QList<QStandardItem *>() << item);
				}
			}
		}
	}
}

void ReceiversWidget::selectNoneContacts(QList<QStandardItem *> AParents)
{
	foreach(QStandardItem *parentItem, AParents)
	{
		for (int row=0; row<parentItem->rowCount(); row++)
		{
			QStandardItem *item = parentItem->child(row);
			if (mapModelToView(item).isValid())
			{
				if (item->data(RDR_KIND).toInt() == RIK_CONTACT)
					item->setCheckState(Qt::Unchecked);
				else if (item->hasChildren())
					selectNoneContacts(QList<QStandardItem *>() << item);
			}
		}
	}
}

void ReceiversWidget::expandAllChilds(QList<QStandardItem *> AParents)
{
	foreach(QStandardItem *parentItem, AParents)
	{
		QModelIndex index = mapModelToView(parentItem);
		if (index.isValid())
			ui.trvReceivers->expand(index);

		for (int row=0; row<parentItem->rowCount(); row++)
		{
			QStandardItem *item = parentItem->child(row);
			if (item->hasChildren())
				expandAllChilds(QList<QStandardItem *>() << item);
		}
	}
}

void ReceiversWidget::collapseAllChilds(QList<QStandardItem *> AParents)
{
	foreach(QStandardItem *parentItem, AParents)
	{
		for (int row=0; row<parentItem->rowCount(); row++)
		{
			QStandardItem *item = parentItem->child(row);
			if (item->hasChildren())
				collapseAllChilds(QList<QStandardItem *>() << item);
			if (item->parent() != NULL)
				ui.trvReceivers->collapse(mapModelToView(item));
		}
	}
}

void ReceiversWidget::restoreExpandState(QStandardItem * AParent)
{
	QModelIndex index = mapModelToView(AParent);
	if (index.isValid())
	{
		if (index.data(RIDR_ITEM_COLLAPSED).toBool())
			ui.trvReceivers->collapse(index);
		else
			ui.trvReceivers->expand(index);
	}

	for (int row=0; row<AParent->rowCount(); row++)
	{
		QStandardItem *item = AParent->child(row);
		if (item->hasChildren())
			restoreExpandState(item);
	}
}

void ReceiversWidget::onModelItemInserted(QStandardItem *AItem)
{
	int itemKind = AItem->data(RDR_KIND).toInt();
	Jid streamJid = AItem->data(RDR_STREAM_JID).toString();
	if (itemKind==RIK_STREAM_ROOT || FStreamItems.contains(streamJid))
	{
		if (itemKind == RIK_STREAM_ROOT)
			FStreamItems.insert(streamJid,AItem);
		else if (itemKind == RIK_GROUP)
			FGroupItems[streamJid].insert(AItem->data(RDR_GROUP).toString(),AItem);
		else if (itemKind == RIK_CONTACT)
			FContactItems[streamJid].insertMulti(AItem->data(RDR_PREP_BARE_JID).toString(),AItem);
	}
	updateCheckState(AItem->parent());
}

void ReceiversWidget::onModelItemRemoving(QStandardItem *AItem)
{
	int itemKind = AItem->data(RDR_KIND).toInt();
	Jid streamJid = AItem->data(RDR_STREAM_JID).toString();
	if (FStreamItems.contains(streamJid))
	{
		AItem->setCheckState(Qt::Unchecked);
		if (itemKind == RIK_STREAM_ROOT)
			FStreamItems.remove(streamJid);
		else if (itemKind == RIK_GROUP)
			FGroupItems[streamJid].remove(AItem->data(RDR_GROUP).toString());
		else if (itemKind == RIK_CONTACT)
			FContactItems[streamJid].remove(AItem->data(RDR_PREP_BARE_JID).toString(),AItem);
	}
	updateCheckState(AItem->parent());

	if (AItem->parent() && AItem->parent()->rowCount()<2 && AItem->parent()->data(RDR_KIND).toInt()!=RIK_STREAM_ROOT)
		deleteItemLater(AItem->parent());
	FDeleteDelayed.removeAll(AItem);
}

void ReceiversWidget::onModelItemDataChanged(QStandardItem *AItem, int ARole)
{
	if (ARole == Qt::CheckStateRole)
	{
		if (AItem->data(RDR_KIND).toInt() == RIK_CONTACT)
		{
			static bool block = false;
			if (!block)
			{
				block = true;
				Jid streamJid = AItem->data(RDR_STREAM_JID).toString();
				Jid contactJid = AItem->data(RDR_PREP_BARE_JID).toString();
				QList<QStandardItem *> contactItems = findContactItems(streamJid,contactJid);
				if (!FModel->isRemovedItem(AItem))
				{
					foreach(QStandardItem *contactItem, contactItems)
						contactItem->setCheckState(AItem->checkState());
					FSelectionSignalTimer.start();
				}
				else if (contactItems.count() < 2)
				{
					FSelectionSignalTimer.start();
				}
				block = false;
			}
		}

		bool updateSelfState = false;
		Qt::CheckState state = AItem->checkState();
		if (state != Qt::PartiallyChecked)
		{
			for (int row=0; row<AItem->rowCount(); row++)
			{
				QStandardItem *childItem = AItem->child(row);
				if (mapModelToView(childItem).isValid())
					childItem->setCheckState(state);
				else
					updateSelfState = true;
			}
		}

		if (updateSelfState)
			updateCheckState(AItem);
		updateCheckState(AItem->parent());
	}
}

void ReceiversWidget::onViewIndexExpanded(const QModelIndex &AIndex)
{
	QStandardItem *item = mapViewToModel(AIndex);
	if (item!=NULL && FProxyModel->filterRegExp().isEmpty())
		item->setData(false,RIDR_ITEM_COLLAPSED);
}

void ReceiversWidget::onViewIndexCollapsed(const QModelIndex &AIndex)
{
	QStandardItem *item = mapViewToModel(AIndex);
	if (item!=NULL && FProxyModel->filterRegExp().isEmpty())
		item->setData(true,RIDR_ITEM_COLLAPSED);
}

void ReceiversWidget::onViewContextMenuRequested(const QPoint &APos)
{
	if (ui.trvReceivers->selectionModel()->hasSelection())
	{
		Menu *menu = new Menu(this);
		menu->setAttribute(Qt::WA_DeleteOnClose,true);

		QList<QStandardItem *> items;
		foreach(const QModelIndex &index, ui.trvReceivers->selectionModel()->selectedIndexes())
			items.append(mapViewToModel(index));
		contextMenuForItems(items,menu);

		if (!menu->isEmpty())
			menu->popup(ui.trvReceivers->mapToGlobal(APos));
		else
			delete menu;
	}
}

void ReceiversWidget::onViewModelRowsInserted(const QModelIndex &AParent, int AStart, int AEnd)
{
	Q_UNUSED(AStart); Q_UNUSED(AEnd);
	restoreExpandState(AParent.isValid() ? mapViewToModel(AParent) : FModel->invisibleRootItem());
}

void ReceiversWidget::onActiveStreamAppended(const Jid &AStreamJid)
{
	IPresence *presence = FPresenceManager!=NULL ? FPresenceManager->findPresence(AStreamJid) : NULL;
	if (presence && presence->isOpen())
		onPresenceOpened(presence);
}

void ReceiversWidget::onActiveStreamRemoved(const Jid &AStreamJid)
{
	IPresence *presence = FPresenceManager!=NULL ? FPresenceManager->findPresence(AStreamJid) : NULL;
	if (presence)
		onPresenceClosed(presence);
}

void ReceiversWidget::onPresenceOpened(IPresence *APresence)
{
	if (!FStreamItems.contains(APresence->streamJid()))
	{
		if (FMessageProcessor==NULL || FMessageProcessor->activeStreams().contains(APresence->streamJid()))
		{
			createStreamItems(APresence->streamJid());
			emit availStreamsChanged();
		}
	}
}

void ReceiversWidget::onPresenceClosed(IPresence *APresence)
{
	if (FStreamItems.contains(APresence->streamJid()))
	{
		destroyStreamItems(APresence->streamJid());
		emit availStreamsChanged();
	}
}

void ReceiversWidget::onPresenceItemReceived(IPresence *APresence, const IPresenceItem &AItem, const IPresenceItem &ABefore)
{
	if (FStreamItems.contains(APresence->streamJid()))
		if (AItem.show!=ABefore.show || AItem.priority!=ABefore.priority)
			updateContactItemsPresence(APresence->streamJid(),AItem.itemJid);
}

void ReceiversWidget::onRosterItemReceived(IRoster *ARoster, const IRosterItem &AItem, const IRosterItem &ABefore)
{
	if (FStreamItems.contains(ARoster->streamJid()))
	{
		QList<QStandardItem *> contactItems = findContactItems(ARoster->streamJid(),AItem.itemJid);
		if (AItem.subscription == SUBSCRIPTION_REMOVE)
		{
			foreach(QStandardItem *contactItem, contactItems)
				contactItem->parent()->removeRow(contactItem->row());
		}
		else
		{
			bool updatePresence = false;
			QString name = AItem.name.isEmpty() ? AItem.itemJid.uBare() : AItem.name;

			if (contactItems.isEmpty() || AItem.groups!=ABefore.groups)
			{
				QSet<QString> oldGroups;
				foreach(QStandardItem *contactItem, contactItems)
					oldGroups += contactItem->data(RDR_GROUP).toString();

				int groupOrder;
				QSet<QString> newGroups;
				if (!AItem.itemJid.hasNode())
				{
					groupOrder = RIKO_GROUP_AGENTS;
					newGroups << (FRostersModel!=NULL ? FRostersModel->singleGroupName(RIK_GROUP_AGENTS) : tr("Agents"));
				}
				else if (AItem.groups.isEmpty())
				{
					groupOrder = RIKO_GROUP_BLANK;
					newGroups << (FRostersModel!=NULL ? FRostersModel->singleGroupName(RIK_GROUP_BLANK) : tr("Without Groups"));
				}
				else
				{
					groupOrder = RIKO_GROUP;
					newGroups = AItem.groups;
				}

				foreach(const QString &group, newGroups-oldGroups)
				{
					QStandardItem *contactItem = getContactItem(ARoster->streamJid(),AItem.itemJid,name,group,groupOrder);
					if (!contactItems.isEmpty())
						contactItem->setCheckState(contactItems.first()->checkState());
					contactItems.append(contactItem);
					updatePresence = true;
				}

				foreach(const QString &group, oldGroups-newGroups)
				{
					QStandardItem *contactItem = findContactItem(ARoster->streamJid(),AItem.itemJid,group);
					if (contactItem)
					{
						contactItems.removeAll(contactItem);
						contactItem->parent()->removeRow(contactItem->row());
					}
				}
			}

			foreach(QStandardItem *contactItem, contactItems)
			{
				contactItem->setText(name);
				contactItem->setData(name,RDR_NAME);
				contactItem->setData(AItem.subscription,RDR_SUBSCRIBTION);
				contactItem->setData(AItem.subscriptionAsk,RDR_SUBSCRIPTION_ASK);
			}

			if (updatePresence)
				updateContactItemsPresence(ARoster->streamJid(),AItem.itemJid);
		}
	}
}

void ReceiversWidget::onSelectionLast()
{
	selectionLoad(Options::fileValue("messagewidgets.receiverswidget.last-selection").toString());
}

void ReceiversWidget::onSelectionLoad()
{
	selectionLoad(QFileDialog::getOpenFileName(this,tr("Load Contacts from File"),QString(),"*.cts"));
}

void ReceiversWidget::onSelectionSave()
{
	selectionSave(QFileDialog::getSaveFileName(this,tr("Save Contacts to File"),QString(),"*.cts"));
}

void ReceiversWidget::onSelectAllContacts()
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
		selectAllContacts(action->data(ADR_ITEMS).value< QList<QStandardItem *> >());
}

void ReceiversWidget::onSelectOnlineContacts()
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
		selectOnlineContacts(action->data(ADR_ITEMS).value< QList<QStandardItem *> >());
}

void ReceiversWidget::onSelectNotBusyContacts()
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
		selectNotBusyContacts(action->data(ADR_ITEMS).value< QList<QStandardItem *> >());
}

void ReceiversWidget::onSelectNoneContacts()
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
		selectNoneContacts(action->data(ADR_ITEMS).value< QList<QStandardItem *> >());
}

void ReceiversWidget::onExpandAllChilds()
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
		expandAllChilds(action->data(ADR_ITEMS).value< QList<QStandardItem *> >());
}

void ReceiversWidget::onCollapseAllChilds()
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
		collapseAllChilds(action->data(ADR_ITEMS).value< QList<QStandardItem *> >());
}

void ReceiversWidget::onHideOfflineContacts()
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
		setOfflineContactsVisible(!action->isChecked());
}

void ReceiversWidget::onSortContactByStatus()
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
		setSortMode(action->isChecked() ? IMessageReceiversWidget::SortByStatus : IMessageReceiversWidget::SortAlphabetically);
}

void ReceiversWidget::onDeleteDelayedItems()
{
	QList<QStandardItem *> deleteNow = FDeleteDelayed;
	foreach(QStandardItem *item, deleteNow)
	{
		if (FDeleteDelayed.contains(item))
			item->parent()->removeRow(item->row());
	}
}

void ReceiversWidget::onStartSearchContacts()
{
	FProxyModel->setFilterWildcard(ui.sleSearch->text());
	if (!FProxyModel->filterRegExp().isEmpty())
		ui.trvReceivers->expandAll();
	else
		restoreExpandState(FModel->invisibleRootItem());
}
