#include "rostersmodel.h"
#include <QDebug>
#include <QTimer>
#include <definitions/rosterindexkinds.h>
#include <definitions/rosterindexroles.h>
#include <definitions/rosterindexkindorders.h>
#include <definitions/rosterdataholderorders.h>
#include <utils/options.h>
#include <utils/logger.h>

static const QList<int> ContactsCacheRosterKinds = QList<int>() << RIK_CONTACT << RIK_AGENT << RIK_MY_RESOURCE;

RostersModel::RostersModel()
{
	FRosterManager = NULL;
	FPresenceManager = NULL;
	FAccountManager = NULL;
// *** <<< eyeCU <<< ***
	FShowSelf = false;
	FShowOfflineAgents = false;
// *** >>> eyeCU >>> ***

	FLayout = LayoutSeparately;

	FRootIndex = new RootIndex(this);
	FContactsRoot = newRosterIndex(RIK_CONTACTS_ROOT);

	setDelayedDataChangedSignals(true);
	setRecursiveParentDataChangedSignals(true);

	connect(this,SIGNAL(itemInserted(QStandardItem *)),SLOT(onAdvancedItemInserted(QStandardItem *)));
	connect(this,SIGNAL(itemRemoving(QStandardItem *)),SLOT(onAdvancedItemRemoving(QStandardItem *)));
	connect(this,SIGNAL(itemDataChanged(QStandardItem *,int)),SLOT(onAdvancedItemDataChanged(QStandardItem *,int)));
}

RostersModel::~RostersModel()
{
	delete FContactsRoot->instance();
	delete FRootIndex;
}

void RostersModel::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Roster Model");
	APluginInfo->description = tr("Creates a hierarchical model for display roster");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Potapov S.A. aka Lion";
	APluginInfo->homePage = "http://www.vacuum-im.org";
}

bool RostersModel::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);
	IPlugin *plugin = APluginManager->pluginInterface("IRosterManager").value(0,NULL);
	if (plugin)
	{
		FRosterManager = qobject_cast<IRosterManager *>(plugin->instance());
		if (FRosterManager)
		{
			connect(FRosterManager->instance(),SIGNAL(rosterItemReceived(IRoster *, const IRosterItem &, const IRosterItem &)),
				SLOT(onRosterItemReceived(IRoster *, const IRosterItem &, const IRosterItem &)));
			connect(FRosterManager->instance(),SIGNAL(rosterStreamJidChanged(IRoster *, const Jid &)),
				SLOT(onRosterStreamJidChanged(IRoster *, const Jid &)));
		}
	}

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0,NULL);
	if (plugin)
	{
		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());
		if (FPresenceManager)
		{
			connect(FPresenceManager->instance(),SIGNAL(presenceChanged(IPresence *, int, const QString &, int)),
				SLOT(onPresenceChanged(IPresence *, int , const QString &, int)));
			connect(FPresenceManager->instance(),SIGNAL(presenceItemReceived(IPresence *, const IPresenceItem &, const IPresenceItem &)),
				SLOT(onPresenceItemReceived(IPresence *, const IPresenceItem &, const IPresenceItem &)));
		}
	}

	plugin = APluginManager->pluginInterface("IAccountManager").value(0,NULL);
	if (plugin)
	{
		FAccountManager = qobject_cast<IAccountManager *>(plugin->instance());
		if (FAccountManager)
		{
			connect(FAccountManager->instance(),SIGNAL(accountActiveChanged(IAccount *, bool)),SLOT(onAccountActiveChanged(IAccount *, bool)));
		}
	}

	return true;
}

bool RostersModel::initObjects()
{
	FContactsRoot->setData(tr("All Contacts"),RDR_NAME);

	registerSingleGroup(RIK_GROUP_ACCOUNTS,tr("Accounts"));
	registerSingleGroup(RIK_GROUP_BLANK,tr("Without Groups"));
	registerSingleGroup(RIK_GROUP_AGENTS,tr("Agents"));
	registerSingleGroup(RIK_GROUP_MY_RESOURCES,tr("My Resources"));
	registerSingleGroup(RIK_GROUP_NOT_IN_ROSTER,tr("Not in Roster"));

	insertRosterDataHolder(RDHO_ROSTERSMODEL,this);

	return true;
}

QList<int> RostersModel::rosterDataRoles(int AOrder) const
{
	if (AOrder == RDHO_ROSTERSMODEL)
		return QList<int>() << RDR_STREAMS;
	return QList<int>();
}

QVariant RostersModel::rosterData(int AOrder, const IRosterIndex *AIndex, int ARole) const
{
	if (AOrder==RDHO_ROSTERSMODEL && ARole==RDR_STREAMS)
	{
		if (AIndex->kind() == RIK_CONTACTS_ROOT)
		{
			QStringList rootStreams;
			foreach(const Jid &streamJid, FStreamIndexes.keys())
				rootStreams.append(streamJid.pFull());
			return rootStreams;
		}
		else if (isGroupKind(AIndex->kind()))
		{
			QStringList groupStreams;
			if (FLayout == LayoutMerged)
			{
				QString group = AIndex->data(RDR_GROUP).toString();
				foreach(const Jid &streamJid, FStreamIndexes.keys())
				{
					if (AIndex->kind() == RIK_GROUP)
					{
						IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(streamJid) : NULL;
						if (roster && roster->hasGroup(group))
							groupStreams.append(streamJid.pFull());
					}
					else
					{
						groupStreams.append(streamJid.pFull());
					}
				}
			}
			else //if (FLayout == LayoutSeparately)
			{
				for(IRosterIndex *pindex = AIndex->parentIndex(); pindex!=NULL; pindex=pindex->parentIndex())
				{
					if (pindex->kind() == RIK_STREAM_ROOT)
					{
						groupStreams.append(pindex->data(RDR_STREAM_JID).toString());
						break;
					}
				}
			}
			return groupStreams;
		}
	}
	return QVariant();
}

bool RostersModel::setRosterData(int AOrder, const QVariant &AValue, IRosterIndex *AIndex, int ARole)
{
	Q_UNUSED(AOrder); Q_UNUSED(AValue); Q_UNUSED(AIndex); Q_UNUSED(ARole);
	return false;
}

QList<Jid> RostersModel::streams() const
{
	return FStreamIndexes.keys();
}

IRosterIndex *RostersModel::addStream(const Jid &AStreamJid)
{
	IRosterIndex *sindex = streamIndex(AStreamJid);
	if (sindex == NULL)
	{
		IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(AStreamJid) : NULL;
		IPresence *presence = FPresenceManager!=NULL ? FPresenceManager->findPresence(AStreamJid) : NULL;
		IAccount *account = FAccountManager!=NULL ? FAccountManager->findAccountByStream(AStreamJid) : NULL;

		if (roster || presence)
		{
			LOG_STRM_INFO(AStreamJid,QString("Adding stream to model"));

			sindex = newRosterIndex(RIK_STREAM_ROOT);
			sindex->setData(AStreamJid.pFull(),RDR_STREAM_JID);
			sindex->setData(AStreamJid.full(),RDR_FULL_JID);
			sindex->setData(AStreamJid.pFull(),RDR_PREP_FULL_JID);
			sindex->setData(AStreamJid.pBare(),RDR_PREP_BARE_JID);

			if (presence)
			{
				sindex->setData(presence->show(),RDR_SHOW);
				sindex->setData(presence->status(),RDR_STATUS);
			}
			if (account)
			{
				sindex->setData(account->name(),RDR_NAME);
				sindex->setData(account->optionsNode().value("order").toInt(),RDR_SORT_ORDER);
				connect(account->instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onAccountOptionsChanged(const OptionsNode &)));
			}

			FStreamIndexes.insert(AStreamJid,sindex);
			emit rosterDataChanged(FContactsRoot,RDR_STREAMS);

			if (FLayout == LayoutMerged)
			{
				insertRosterIndex(FContactsRoot,FRootIndex);
				insertRosterIndex(sindex,getGroupIndex(RIK_GROUP_ACCOUNTS,QString(),FContactsRoot));
			}
			else
			{
				insertRosterIndex(sindex,FRootIndex);
			}

			emit streamAdded(AStreamJid);

			if (roster)
			{
				IRosterItem empty;
				foreach(const IRosterItem &ritem, roster->items())
					onRosterItemReceived(roster,ritem,empty);
			}
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,QString("Failed to add stream to model: Roster and Presence not found"));
		}
	}
	return sindex;
}

void RostersModel::removeStream(const Jid &AStreamJid)
{
	IRosterIndex *sindex =streamIndex(AStreamJid);
	if (sindex)
	{
		LOG_STRM_INFO(AStreamJid,QString("Removing stream from model"));

		IAccount *account = FAccountManager!=NULL ? FAccountManager->findAccountByStream(AStreamJid) : NULL;
		if (account)
			disconnect(account->instance(),SIGNAL(optionsChanged(const OptionsNode &)),this,SLOT(onAccountOptionsChanged(const OptionsNode &)));

		if (FLayout == LayoutMerged)
		{
			foreach(IRosterIndex *itemIndex, FContactsCache.value(sindex).values())
				removeRosterIndex(itemIndex);
		}
		removeRosterIndex(sindex);

		FContactsCache.remove(sindex);
		FStreamIndexes.remove(AStreamJid);
		emit rosterDataChanged(FContactsRoot,RDR_STREAMS);

		if (FLayout==LayoutMerged && FStreamIndexes.isEmpty())
		{
			FContactsRoot->removeChildren();
			removeRosterIndex(FContactsRoot,false);
		}

		emit streamRemoved(AStreamJid);
	}
}

int RostersModel::streamsLayout() const
{
	return FLayout;
}

void RostersModel::setStreamsLayout(StreamsLayout ALayout)
{
	if (ALayout != FLayout)
	{
		LOG_INFO(QString("Changing streams layout to=%1").arg(ALayout));
		emit streamsLayoutAboutToBeChanged(ALayout);

		StreamsLayout before = FLayout;
		FLayout = ALayout;

		if (!FStreamIndexes.isEmpty())
		{
			if (ALayout == LayoutMerged)
			{
				insertRosterIndex(FContactsRoot,FRootIndex);
			}
			else //if (ALayout == LayoutSeparately)
			{
				foreach(IRosterIndex *sindex, FStreamIndexes.values())
					insertRosterIndex(sindex,FRootIndex);
			}

			QHash<IRosterIndex *, QMultiHash<Jid, IRosterIndex *> > contacts = FContactsCache;
			for (QHash<IRosterIndex *, QMultiHash<Jid, IRosterIndex *> >::const_iterator streamIt=contacts.constBegin(); streamIt!=contacts.constEnd(); ++streamIt)
			{
				IRosterIndex *sroot = ALayout==LayoutMerged ? FContactsRoot : streamIt.key();
				for (QMultiHash<Jid, IRosterIndex *>::const_iterator itemIt=streamIt->constBegin(); itemIt!=streamIt->constEnd(); ++itemIt)
				{
					IRosterIndex *itemIndex = itemIt.value();
					IRosterIndex *pindex = itemIndex->parentIndex();
					if (isGroupKind(pindex->kind()))
					{
// *** <<< eyeCU <<< ***
						IRosterIndex *groot = pindex->kind()==RIK_GROUP_MY_RESOURCES?streamIt.key():sroot;
						IRosterIndex *groupIndex = getGroupIndex(pindex->kind(),pindex->data(RDR_GROUP).toString(),groot);
// *** >>> eyeCU >>> ***
						groupIndex->setData(pindex->data(RDR_KIND_ORDER),RDR_KIND_ORDER);
						insertRosterIndex(itemIndex,groupIndex);
					}
// *** <<< eyeCU <<< ***
					else if (itemIndex->kind()==RIK_MY_RESOURCE)
					{
						insertRosterIndex(itemIndex, getGroupIndex(RIK_GROUP_MY_RESOURCES,pindex->data(RDR_GROUP).toString(),streamIt.key()));
					}
// *** >>> eyeCU >>> ***
					else if (pindex==FContactsRoot || pindex==streamIt.key())
					{
						insertRosterIndex(itemIndex,sroot);
					}					
				}
			}

			if (ALayout == LayoutMerged)
			{
				foreach(IRosterIndex *sindex, FStreamIndexes.values())
					insertRosterIndex(sindex,getGroupIndex(RIK_GROUP_ACCOUNTS,QString(),FContactsRoot));
			}
			else //if (ALayout == LayoutSeparately)
			{
				removeRosterIndex(FContactsRoot,false);
			}
		}

		emit streamsLayoutChanged(before);
	}
}

IRosterIndex *RostersModel::rootIndex() const
{
	return FRootIndex;
}

IRosterIndex *RostersModel::contactsRoot() const
{
	return FContactsRoot;
}

IRosterIndex *RostersModel::streamRoot(const Jid &AStreamJid) const
{
	if (FStreamIndexes.contains(AStreamJid))
		return FLayout==LayoutSeparately ? streamIndex(AStreamJid) : contactsRoot();
	return NULL;
}

IRosterIndex *RostersModel::streamIndex(const Jid &AStreamJid) const
{
	return FStreamIndexes.value(AStreamJid);
}

IRosterIndex *RostersModel::newRosterIndex(int AKind)
{
	static const struct { int kind; int order; }	DefKindOrders[] = {
		{RIK_CONTACTS_ROOT,       RIKO_CONTACTS_ROOT},
		{RIK_STREAM_ROOT,         RIKO_STREAM_ROOT},
		{RIK_GROUP,               RIKO_GROUP},
		{RIK_GROUP_ACCOUNTS,      RIKO_GROUP_ACCOUNTS},
		{RIK_GROUP_BLANK,         RIKO_GROUP_BLANK},
		{RIK_GROUP_NOT_IN_ROSTER, RIKO_GROUP_NOT_IN_ROSTER},
		{RIK_GROUP_MY_RESOURCES,  RIKO_GROUP_MY_RESOURCES},
		{RIK_GROUP_AGENTS,        RIKO_GROUP_AGENTS},
		{-1,                      -1}
	};

	IRosterIndex *rindex = new RosterIndex(AKind,this);

	int typeOrder = RIKO_DEFAULT;
	for (int i=0; DefKindOrders[i].kind>=0; i++)
	{
		if (AKind == DefKindOrders[i].kind)
		{
			typeOrder = DefKindOrders[i].order;
			break;
		}
	}
	rindex->setData(typeOrder,RDR_KIND_ORDER);

	emit indexCreated(rindex);

	return rindex;
}

void RostersModel::insertRosterIndex(IRosterIndex *AIndex, IRosterIndex *AParent)
{
	IRosterIndex *pindex = AIndex->parentIndex();
	if (pindex != AParent)
	{
		if (pindex)
			removeRosterIndex(AIndex,false);
		AParent->appendChild(AIndex);
	}
}

void RostersModel::removeRosterIndex(IRosterIndex *AIndex, bool ADestroy)
{
	IRosterIndex *groupIndex = AIndex->parentIndex();
	if (groupIndex)
	{
		if (ADestroy)
			AIndex->remove();
		else
			groupIndex->takeIndex(AIndex->row());
		removeEmptyGroup(groupIndex);
	}
}

IRosterIndex *RostersModel::findGroupIndex(int AKind, const QString &AGroup, IRosterIndex *AParent) const
{
// *** <<< eyeCU <<< ***
	if (AKind==RIK_GROUP_MY_RESOURCES && FLayout==LayoutMerged)
		return AParent;
// *** >>> eyeCU >>> ***
	QString groupPath = getGroupName(AKind,AGroup);
	QList<QString> groupTree = groupPath.split(ROSTER_GROUP_DELIMITER);

	IRosterIndex *groupIndex = AParent;
	do {
		QList<IRosterIndex *> indexes = FGroupsCache.value(groupIndex).values(groupTree.takeFirst());

		groupIndex = NULL;
		for(QList<IRosterIndex *>::const_iterator it = indexes.constBegin(); !groupIndex && it!=indexes.constEnd(); ++it)
			if ((*it)->kind() == AKind)
				groupIndex = *it;

	} while (groupIndex && !groupTree.isEmpty());

	return groupIndex;
}

IRosterIndex *RostersModel::getGroupIndex(int AKind, const QString &AGroup, IRosterIndex *AParent)
{
	IRosterIndex *groupIndex = findGroupIndex(AKind,AGroup,AParent);
	if (!groupIndex)
	{
		QString groupPath = getGroupName(AKind,AGroup);
		QList<QString> groupTree = groupPath.split(ROSTER_GROUP_DELIMITER);

		int i = 0;
		groupIndex = AParent;
		IRosterIndex *childGroupIndex = groupIndex;
		QString group = AParent->data(RDR_GROUP).toString();
		while (childGroupIndex && i<groupTree.count())
		{
			if (group.isEmpty())
				group = groupTree.at(i);
			else
				group += ROSTER_GROUP_DELIMITER + groupTree.at(i);

			childGroupIndex = findGroupIndex(AKind, groupTree.at(i), groupIndex);
			if (childGroupIndex)
			{
				groupIndex = childGroupIndex;
				i++;
			}
		}

		while (i < groupTree.count())
		{
			childGroupIndex = newRosterIndex(AKind);
			if (!FSingleGroups.contains(AKind))
				childGroupIndex->setData(group,RDR_GROUP);
			childGroupIndex->setData(groupTree.at(i),RDR_NAME);
			insertRosterIndex(childGroupIndex,groupIndex);

			groupIndex = childGroupIndex;
			group += ROSTER_GROUP_DELIMITER + groupTree.value(++i);
		}
	}
	return groupIndex;
}

QList<IRosterIndex *> RostersModel::findContactIndexes(const Jid &AStreamJid, const Jid &AContactJid, IRosterIndex *AParent) const
{
	QList<IRosterIndex *> indexes = FContactsCache.value(streamIndex(AStreamJid)).values(AContactJid.bare());

	bool checkParent = AParent!=NULL;
	bool checkResource = AContactJid.hasResource() && AStreamJid.pBare()==AContactJid.pBare();
	if (checkParent || checkResource)
	{
		for(QList<IRosterIndex *>::iterator it=indexes.begin(); it!=indexes.end(); )
		{
			IRosterIndex *index = *it;
			if (checkParent && index->parentIndex()!=AParent)
				it = indexes.erase(it);
			else if (checkResource && AContactJid!=index->data(RDR_PREP_FULL_JID).toString())
				it = indexes.erase(it);
			else
				++it;
		}
	}
	return indexes;
}

QList<IRosterIndex *> RostersModel::getContactIndexes(const Jid &AStreamJid, const Jid &AContactJid, IRosterIndex *AParent)
{
	QList<IRosterIndex *> indexes = findContactIndexes(AStreamJid,AContactJid,AParent);
	if (indexes.isEmpty())
	{
		IRosterIndex *sroot = streamRoot(AStreamJid);
		if (sroot)
		{
			int type = RIK_CONTACT;
			if (!AContactJid.hasNode())
				type = RIK_AGENT;
			else if (AContactJid.pBare() == AStreamJid.pBare())
				type = RIK_MY_RESOURCE;

			IRosterIndex *groupIndex;
			if (AParent)
				groupIndex = AParent;
			else if (type == RIK_MY_RESOURCE)
			{
// *** <<< eyeCU <<< ***
				sroot = streamRoot(AStreamJid);
// *** >>> eyeCU >>> ***
				groupIndex = getGroupIndex(RIK_GROUP_MY_RESOURCES,QString(),sroot);
			}
			else
				groupIndex = getGroupIndex(RIK_GROUP_NOT_IN_ROSTER,QString(),sroot);

			IRosterIndex *itemIndex = newRosterIndex(type);
			itemIndex->setData(AStreamJid.pFull(),RDR_STREAM_JID);
			itemIndex->setData(AContactJid.full(),RDR_FULL_JID);
			itemIndex->setData(AContactJid.pFull(),RDR_PREP_FULL_JID);
			itemIndex->setData(AContactJid.pBare(),RDR_PREP_BARE_JID);
			itemIndex->setData(groupIndex->data(RDR_GROUP),RDR_GROUP);
			itemIndex->setData(IPresence::Offline,RDR_SHOW);
			insertRosterIndex(itemIndex,groupIndex);

			indexes.append(itemIndex);
		}
	}
	return indexes;
}

QModelIndex RostersModel::modelIndexFromRosterIndex(IRosterIndex *AIndex) const
{
	return AIndex!=NULL && AIndex!=FRootIndex ? indexFromItem(AIndex->instance()) : QModelIndex();
}

IRosterIndex *RostersModel::rosterIndexFromModelIndex(const QModelIndex &AIndex) const
{
	if (AIndex.isValid())
		return static_cast<RosterIndex *>(itemFromIndex(AIndex));
	return FRootIndex;
}

bool RostersModel::isGroupKind(int AKind) const
{
	return AKind==RIK_GROUP || FSingleGroups.contains(AKind);
}

QList<int> RostersModel::singleGroupKinds() const
{
	return FSingleGroups.keys();
}

QString RostersModel::singleGroupName(int AKind) const
{
	return FSingleGroups.value(AKind);
}

void RostersModel::registerSingleGroup(int AKind, const QString &AName)
{
	if (!FSingleGroups.contains(AKind) && !AName.trimmed().isEmpty())
	{
		LOG_DEBUG(QString("Single group registered, kind=%1, name=%2").arg(AKind).arg(AName));
		FSingleGroups.insert(AKind,AName);
	}
}

QMultiMap<int, IRosterDataHolder *> RostersModel::rosterDataHolders() const
{
	return FRosterDataHolders;
}

void RostersModel::insertRosterDataHolder(int AOrder, IRosterDataHolder *AHolder)
{
	if (AHolder && !FRosterDataHolders.contains(AOrder,AHolder))
	{
		FRosterDataHolders.insertMulti(AOrder,AHolder);
		DataHolder *proxyHolder = FAdvancedDataHolders.value(AHolder);
		if (proxyHolder == NULL)
		{
			proxyHolder = new DataHolder(AHolder,this);
			FAdvancedDataHolders.insert(AHolder,proxyHolder);
		}

		LOG_DEBUG(QString("Roster data holder inserted, order=%1, class=%2").arg(AOrder).arg(AHolder->instance()->metaObject()->className()));
		AdvancedItemModel::insertItemDataHolder(AOrder,proxyHolder);
	}
}

void RostersModel::removeRosterDataHolder(int AOrder, IRosterDataHolder *AHolder)
{
	if (FRosterDataHolders.contains(AOrder,AHolder))
	{
		FRosterDataHolders.remove(AOrder,AHolder);
		DataHolder *proxyHolder = FRosterDataHolders.values().contains(AHolder) ? FAdvancedDataHolders.value(AHolder) : FAdvancedDataHolders.take(AHolder);

		LOG_DEBUG(QString("Roster data holder removed, order=%1, class=%2").arg(AOrder).arg(AHolder->instance()->metaObject()->className()));
		AdvancedItemModel::removeItemDataHolder(AOrder,proxyHolder);
	}
}

// *** <<< eyeCU <<< ***
void RostersModel::setShowSelf(bool AShow)
{
	FShowSelf=AShow;
	for (QMap<Jid,IRosterIndex *>::const_iterator i=FStreamIndexes.constBegin(); i!=FStreamIndexes.constEnd(); i++)
	{
		IRosterIndex *sindex = *i;
		int s=sindex->data(RDR_SHOW).toInt();
		if (s != IPresence::Offline && s != IPresence::Error)
		{
			IRosterIndex *groupIndex = findGroupIndex(RIK_GROUP_MY_RESOURCES, QString::null, streamIndex(sindex->data(RDR_STREAM_JID).toString()));
			IRosterIndex *selfIndex = NULL;
			if (groupIndex)
			{
				QMultiMap<int, QVariant> findData;
				findData.insertMulti(RDR_KIND, RIK_MY_RESOURCE);
				findData.insertMulti(RDR_FULL_JID, sindex->data(RDR_FULL_JID));
				QList<IRosterIndex *> resources = groupIndex->findChilds(findData);
				if (!resources.isEmpty())
					selfIndex=resources.first();
			}
			if (AShow)
			{
				if (!selfIndex)
				{
					if (!groupIndex)
						groupIndex = getGroupIndex(RIK_GROUP_MY_RESOURCES, QString::null, streamIndex(sindex->data(RDR_STREAM_JID).toString()));
					selfIndex = newRosterIndex(RIK_MY_RESOURCE);
					selfIndex->setData(sindex->data(RDR_STREAM_JID), RDR_STREAM_JID);
					selfIndex->setData(sindex->data(RDR_FULL_JID), RDR_FULL_JID);
					selfIndex->setData(sindex->data(RDR_PREP_FULL_JID), RDR_PREP_FULL_JID);
					selfIndex->setData(sindex->data(RDR_PREP_BARE_JID), RDR_PREP_BARE_JID);
					selfIndex->setData(sindex->data(RDR_SHOW), RDR_SHOW);
					selfIndex->setData(sindex->data(RDR_STATUS), RDR_STATUS);
					insertRosterIndex(selfIndex, groupIndex);
				}
			}
			else
				if (selfIndex)
					removeRosterIndex(selfIndex);
		}
	}
}
// *** >>> eyeCU >>> ***

void RostersModel::updateStreamsLayout()
{
	if (FLayout == LayoutMerged)
	{
		if (!FStreamIndexes.isEmpty())
			insertRosterIndex(FContactsRoot,FRootIndex);
	}
	else
	{
		removeRosterIndex(FContactsRoot,false);
	}
}

void RostersModel::emitIndexDestroyed(IRosterIndex *AIndex)
{
	emit indexDestroyed(AIndex);
}

void RostersModel::removeEmptyGroup(IRosterIndex *AGroupIndex)
{
	if (AGroupIndex && AGroupIndex->childCount()==0 && isGroupKind(AGroupIndex->kind()))
	{
		IRosterIndex *parentGroup = AGroupIndex->parentIndex();
		AGroupIndex->remove();
		removeEmptyGroup(parentGroup);
	}
}

QString RostersModel::getGroupName(int AKind, const QString &AGroup) const
{
	if (FSingleGroups.contains(AKind))
		return singleGroupName(AKind);
	else if (AGroup.isEmpty())
		return singleGroupName(RIK_GROUP_BLANK);
	return AGroup;
}

bool RostersModel::isChildIndex(IRosterIndex *AIndex, IRosterIndex *AParent) const
{
	IRosterIndex *pindex = AIndex->parentIndex();
	while (pindex!=NULL && pindex!=AParent)
		pindex = pindex->parentIndex();
	return pindex==AParent;
}

void RostersModel::onAdvancedItemInserted(QStandardItem *AItem)
{
	if (AItem->type() == IRosterIndex::RosterItemTypeValue)
	{
		IRosterIndex *rindex = static_cast<RosterIndex *>(AItem);
		Jid streamJid = rindex->data(RDR_STREAM_JID).toString();
		if (isGroupKind(rindex->kind()))
		{
			IRosterIndex *pindex = rindex->parentIndex();
			if (pindex)
				FGroupsCache[pindex].insertMulti(rindex->data(RDR_NAME).toString(),rindex);
		}
		else if (!streamJid.isEmpty() && ContactsCacheRosterKinds.contains(rindex->kind()))
		{
			QString bareJid = rindex->data(RDR_PREP_BARE_JID).toString();
			IRosterIndex *sindex = !bareJid.isEmpty() ? streamIndex(streamJid) : NULL;
			if (sindex && sindex!=rindex && isChildIndex(rindex,streamRoot(streamJid)))
				FContactsCache[sindex].insertMulti(bareJid,rindex);
		}
		emit indexInserted(rindex);
	}
}

void RostersModel::onAdvancedItemRemoving(QStandardItem *AItem)
{
	if (AItem->type() == IRosterIndex::RosterItemTypeValue)
	{
		IRosterIndex *rindex = static_cast<RosterIndex *>(AItem);
		Jid streamJid = rindex->data(RDR_STREAM_JID).toString();
		if (isGroupKind(rindex->kind()))
		{
			IRosterIndex *pindex = rindex->parentIndex();
			if (pindex)
				FGroupsCache[pindex].remove(rindex->data(RDR_NAME).toString(),rindex);
		}
		else if (!streamJid.isEmpty() && ContactsCacheRosterKinds.contains(rindex->kind()))
		{
			QString bareJid = rindex->data(RDR_PREP_BARE_JID).toString();
			IRosterIndex *sindex = !bareJid.isEmpty() ? streamIndex(streamJid) : NULL;
			if (sindex)
				FContactsCache[sindex].remove(bareJid,rindex);
		}
		emit indexRemoving(rindex);
	}
}

void RostersModel::onAdvancedItemDataChanged(QStandardItem *AItem, int ARole)
{
	if (AItem->type() == IRosterIndex::RosterItemTypeValue)
		emit indexDataChanged(static_cast<RosterIndex *>(AItem),ARole);
}

void RostersModel::onAccountOptionsChanged(const OptionsNode &ANode)
{
	IAccount *account = qobject_cast<IAccount *>(sender());
	if (account!=NULL && account->optionsNode().childPath(ANode)=="name")
	{
		IRosterIndex *sindex = streamIndex(account->streamJid());
		if (sindex)
			sindex->setData(account->name(),RDR_NAME);
	}
	else if (account && account->optionsNode().childPath(ANode)=="order")
	{
		IRosterIndex *sindex = streamIndex(account->streamJid());
		if (sindex)
			sindex->setData(ANode.value().toInt(),RDR_SORT_ORDER);
	}
}

void RostersModel::onAccountActiveChanged(IAccount *AAccount, bool AActive)
{
	if (AActive)
		addStream(AAccount->streamJid());
	else
		removeStream(AAccount->streamJid());
}

void RostersModel::onRosterItemReceived(IRoster *ARoster, const IRosterItem &AItem, const IRosterItem &ABefore)
{
	Q_UNUSED(ABefore);
	IRosterIndex *sroot = streamRoot(ARoster->streamJid());
	if (sroot)
	{
		QList<IRosterIndex *> curItemList = findContactIndexes(ARoster->streamJid(),AItem.itemJid);

		int groupKind;
		QSet<QString> itemGroups;
		int itemKind = AItem.itemJid.hasNode() ? RIK_CONTACT : RIK_AGENT;
		if (itemKind == RIK_AGENT)
		{
			groupKind = RIK_GROUP_AGENTS;
			itemGroups += QString();
		}
		else if (AItem.groups.isEmpty())
		{
			groupKind = RIK_GROUP_BLANK;
			itemGroups += QString();
		}
		else
		{
			groupKind = RIK_GROUP;
			itemGroups = AItem.groups;
		}

		QList<IRosterIndex *> itemList;
		if (AItem.subscription != SUBSCRIPTION_REMOVE)
		{
			QSet<QString> curGroups;
			foreach(IRosterIndex *itemIndex, curItemList)
				curGroups.insert(itemIndex->data(RDR_GROUP).toString());

			QSet<QString> newGroups = itemGroups - curGroups;
			QSet<QString> oldGroups = curGroups - itemGroups;

			IPresence *presence = FPresenceManager!=NULL ? FPresenceManager->findPresence(ARoster->streamJid()) : NULL;
			QList<IPresenceItem> pitemList = presence!=NULL ? FPresenceManager->sortPresenceItems(presence->findItems(AItem.itemJid)) : QList<IPresenceItem>();

			QStringList resources;
			foreach(const IPresenceItem &pitem, pitemList)
				if (pitem.show != IPresence::Offline)
					resources.append(pitem.itemJid.pFull());

			foreach(const QString &group, itemGroups)
			{
// *** <<< eyeCU <<< ***
				IRosterIndex *groupIndex = getGroupIndex(groupKind,group,groupKind==RIK_GROUP_MY_RESOURCES?streamIndex(ARoster->streamJid()):sroot);
// *** >>> eyeCU >>> ***
				QList<IRosterIndex *> groupItemList;
				if (newGroups.contains(group) && !oldGroups.isEmpty())
				{
					IRosterIndex *oldGroupIndex;
					QString oldGroup = oldGroups.values().value(0);
					if (oldGroup.isEmpty())
						oldGroupIndex = findGroupIndex(RIK_GROUP_BLANK,QString(),sroot);
					else
						oldGroupIndex = findGroupIndex(RIK_GROUP,oldGroup,sroot);

					if (oldGroupIndex)
					{
						groupItemList = findContactIndexes(ARoster->streamJid(),AItem.itemJid,oldGroupIndex);
						foreach(IRosterIndex *itemIndex, groupItemList)
						{
							itemIndex->setData(group,RDR_GROUP);
							insertRosterIndex(itemIndex,groupIndex);
						}
					}
					oldGroups -= group;
				}
				else
				{
					groupItemList = findContactIndexes(ARoster->streamJid(),AItem.itemJid,groupIndex);
				}

				if (groupItemList.isEmpty())
				{
					IPresenceItem pitem = pitemList.value(0);

					IRosterIndex *itemIndex = newRosterIndex(itemKind);
					if (!pitem.isNull())
					{
						itemIndex->setData(pitem.itemJid.full(),RDR_FULL_JID);
						itemIndex->setData(pitem.itemJid.pFull(),RDR_PREP_FULL_JID);
						itemIndex->setData(pitem.priority,RDR_PRIORITY);
					}
					else
					{
						itemIndex->setData(AItem.itemJid.bare(),RDR_FULL_JID);
						itemIndex->setData(AItem.itemJid.pBare(),RDR_PREP_FULL_JID);
					}
					itemIndex->setData(ARoster->streamJid().pFull(),RDR_STREAM_JID);

					itemIndex->setData(AItem.itemJid.pBare(),RDR_PREP_BARE_JID);
					itemIndex->setData(AItem.name,RDR_NAME);
					itemIndex->setData(AItem.subscription,RDR_SUBSCRIBTION);
					itemIndex->setData(AItem.subscriptionAsk,RDR_SUBSCRIPTION_ASK);
					itemIndex->setData(group,RDR_GROUP);

					itemIndex->setData(pitem.show,RDR_SHOW);
					itemIndex->setData(pitem.status,RDR_STATUS);
					itemIndex->setData(resources,RDR_RESOURCES);

					insertRosterIndex(itemIndex,groupIndex);
					itemList.append(itemIndex);
				}
				else foreach(IRosterIndex *itemIndex, groupItemList)
				{
					itemIndex->setData(AItem.name,RDR_NAME);
					itemIndex->setData(AItem.subscription,RDR_SUBSCRIBTION);
					itemIndex->setData(AItem.subscriptionAsk,RDR_SUBSCRIPTION_ASK);
					itemList.append(itemIndex);
				}
			}
		}

		foreach(IRosterIndex *itemIndex, curItemList)
		{
			if (!itemList.contains(itemIndex))
				removeRosterIndex(itemIndex);
		}
// *** <<< eyeCU <<< ***
		// Set RDR_FORCE_VISIBLE attribute for agents, if "Show offline agents" feature is enabled
		if (FShowOfflineAgents && (itemKind == RIK_AGENT))
		{
			curItemList = findContactIndexes(ARoster->streamJid(), AItem.itemJid);
			for (QList<IRosterIndex *>::const_iterator it=curItemList.constBegin(); it!=curItemList.constEnd(); it++)
				(*it)->setData((*it)->data(RDR_FORCE_VISIBLE).toInt()+1, RDR_FORCE_VISIBLE);
		}
// *** >>> eyeCU >>> ***
	}
}

void RostersModel::onRosterStreamJidChanged(IRoster *ARoster, const Jid &ABefore)
{
	IRosterIndex *sindex = streamIndex(ABefore);
	if (sindex)
	{
		Jid after = ARoster->streamJid();

		QMultiMap<int,QVariant> findData;
		findData.insert(RDR_STREAM_JID,ABefore.pFull());
		QList<IRosterIndex *> itemList = FRootIndex->findChilds(findData,true);
		foreach(IRosterIndex *itemIndex, itemList)
			itemIndex->setData(after.pFull(),RDR_STREAM_JID);

		sindex->setData(after.full(),RDR_FULL_JID);
		sindex->setData(after.pFull(),RDR_PREP_FULL_JID);

		FStreamIndexes.remove(ABefore);
		FStreamIndexes.insert(after,sindex);
		emit rosterDataChanged(FContactsRoot,RDR_STREAMS);

		emit streamJidChanged(ABefore,after);
	}
}

void RostersModel::onPresenceChanged(IPresence *APresence, int AShow, const QString &AStatus, int APriority)
{
	IRosterIndex *sindex = streamIndex(APresence->streamJid());
	if (sindex)
	{
		sindex->setData(AShow,RDR_SHOW);
		sindex->setData(AStatus,RDR_STATUS);
// *** <<< eyeCU <<< ***
		IRosterIndex *groupIndex=NULL;
		IRosterIndex *selfIndex=NULL;
		if (FShowSelf)
		{
			groupIndex = findGroupIndex(RIK_GROUP_MY_RESOURCES, QString::null, sindex);
			if (groupIndex)
			{
				QMultiMap<int, QVariant> findData;
				findData.insertMulti(RDR_KIND, RIK_MY_RESOURCE);
				findData.insertMulti(RDR_FULL_JID, APresence->streamJid().full());
				findData.insertMulti(RDR_STREAM_JID, APresence->streamJid().full());
				QList<IRosterIndex *> children = groupIndex->findChilds(findData);
				if (!children.isEmpty())
					selfIndex=children.first();
			}
		}
// *** >>> eyeCU >>> ***

		if (AShow!=IPresence::Offline && AShow!=IPresence::Error)
		{	// *** <<< eyeCU >>> ***
			sindex->setData(APriority,RDR_PRIORITY);
// *** <<< eyeCU <<< ***
			if (FShowSelf)
			{
				if (!selfIndex)
				{
					if (!groupIndex)
						groupIndex = getGroupIndex(RIK_GROUP_MY_RESOURCES, QString::null, streamIndex(APresence->streamJid()));
					selfIndex = newRosterIndex(RIK_MY_RESOURCE);
					selfIndex->setData(APresence->streamJid().pFull(), RDR_STREAM_JID);
					selfIndex->setData(APresence->streamJid().full(), RDR_FULL_JID);
					selfIndex->setData(APresence->streamJid().pFull(), RDR_PREP_FULL_JID);
					selfIndex->setData(APresence->streamJid().pBare(), RDR_PREP_BARE_JID);
					insertRosterIndex(selfIndex, groupIndex);
				}
				selfIndex->setData(AShow, RDR_SHOW);
				selfIndex->setData(AStatus, RDR_STATUS);
			}
		}
// *** >>> eyeCU >>> ***
		else
		{	// *** <<< eyeCU >>> ***
			sindex->setData(QVariant(),RDR_PRIORITY);
// *** <<< eyeCU <<< ***
			if (FShowSelf && selfIndex)
				removeRosterIndex(selfIndex);
		}
// *** >>> eyeCU >>> ***
	}
}

void RostersModel::onPresenceItemReceived(IPresence *APresence, const IPresenceItem &AItem, const IPresenceItem &ABefore)
{
	Q_UNUSED(ABefore);
	IRosterIndex *sroot = streamRoot(APresence->streamJid());
	if (sroot)
	{
		int itemKind = RIK_CONTACT;
		if (!AItem.itemJid.hasNode())
			itemKind = RIK_AGENT;
		else if (AItem.itemJid.pBare() == APresence->streamJid().pBare())
			itemKind = RIK_MY_RESOURCE;

		QList<IRosterIndex *> itemList = findContactIndexes(APresence->streamJid(),AItem.itemJid);
		QList<IPresenceItem> pitemList = FPresenceManager->sortPresenceItems(APresence->findItems(AItem.itemJid));

		if (itemKind == RIK_MY_RESOURCE)
		{
			IRosterIndex *itemIndex = NULL;
			for (int i=0; itemIndex==NULL && i<itemList.count(); i++)
			{
				IRosterIndex *index = itemList.at(i);
				if (index->kind()==RIK_MY_RESOURCE && index->data(RDR_PREP_FULL_JID).toString()==AItem.itemJid.pFull())
					itemIndex = index;
			}

			if (AItem.show == IPresence::Offline)
			{
				if (itemIndex)
					removeRosterIndex(itemIndex);
				itemList.clear();
			}
			else
			{
				if (itemIndex == NULL)
				{
// *** <<< eyeCU <<< ***
					IRosterIndex *groupIndex = getGroupIndex(RIK_GROUP_MY_RESOURCES,QString(),streamIndex(APresence->streamJid()));
// *** >>> eyeCU >>> ***
					itemIndex = newRosterIndex(itemKind);
					itemIndex->setData(APresence->streamJid().pFull(),RDR_STREAM_JID);
					itemIndex->setData(AItem.itemJid.pBare(),RDR_PREP_BARE_JID);
					insertRosterIndex(itemIndex,groupIndex);
				}
				pitemList.clear();
				itemList = QList<IRosterIndex *>() << itemIndex;
			}
		}

		if (pitemList.isEmpty())
			pitemList.append(AItem);
		IPresenceItem pitem = pitemList.at(0);

		QStringList resources;
		foreach(const IPresenceItem &pitem, pitemList)
			if (pitem.show != IPresence::Offline)
				resources.append(pitem.itemJid.pFull());

		foreach(IRosterIndex *itemIndex, itemList)
		{
			if (pitem.show == IPresence::Offline)
			{
				itemIndex->setData(pitem.itemJid.bare(),RDR_FULL_JID);
				itemIndex->setData(pitem.itemJid.pBare(),RDR_PREP_FULL_JID);
				itemIndex->setData(QVariant(),RDR_PRIORITY);
			}
			else
			{
				itemIndex->setData(pitem.itemJid.full(),RDR_FULL_JID);
				itemIndex->setData(pitem.itemJid.pFull(),RDR_PREP_FULL_JID);
				itemIndex->setData(pitem.priority,RDR_PRIORITY);
			}
			itemIndex->setData(pitem.show,RDR_SHOW);
			itemIndex->setData(pitem.status,RDR_STATUS);
			itemIndex->setData(resources,RDR_RESOURCES);
		}
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_rostersmodel, RostersModel)
#endif
