#include "statusicons.h"

#include <QTimer>
#include <QComboBox>
#include <definitions/resources.h>
#include <definitions/statusicons.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/actiongroups.h>
#include <definitions/menuicons.h>
#include <definitions/rosterindexkinds.h>
#include <definitions/rosterindexroles.h>
#include <definitions/rosterdataholderorders.h>
#include <utils/iconsetdelegate.h>
#include <utils/options.h>
#include <utils/logger.h>

#define ADR_RULE                          Action::DR_Parametr1
#define ADR_SUBSTORAGE                    Action::DR_Parametr2

#define STATUSICONS_STORAGE_PATTERN      "pattern"

StatusIcons::StatusIcons()
{
	FPresenceManager = NULL;
	FRosterManager = NULL;
	FRostersModel = NULL;
	FRostersViewPlugin = NULL;
	FMultiChatManager = NULL;
	FOptionsManager = NULL;

	FDefaultStorage = NULL;
	FCustomIconMenu = NULL;
	FDefaultIconAction = NULL;
	FStatusIconsUpdateStarted = false;
}

StatusIcons::~StatusIcons()
{
	delete FCustomIconMenu;
}

void StatusIcons::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Status Icons Manager");
	APluginInfo->description = tr("Allows to set the status icons for contacts on the basis of standard rules or user-defined");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Potapov S.A. aka Lion";
	APluginInfo->homePage = "http://www.vacuum-im.org";
}

bool StatusIcons::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);
	IPlugin *plugin = APluginManager->pluginInterface("IPresenceManager").value(0,NULL);
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

	plugin = APluginManager->pluginInterface("IRosterManager").value(0,NULL);
	if (plugin)
	{
		FRosterManager = qobject_cast<IRosterManager *>(plugin->instance());
		if (FRosterManager)
		{
			connect(FRosterManager->instance(),SIGNAL(rosterItemReceived(IRoster *, const IRosterItem &, const IRosterItem &)),
				SLOT(onRosterItemReceived(IRoster *, const IRosterItem &, const IRosterItem &)));
		}
	}

	plugin = APluginManager->pluginInterface("IRostersModel").value(0,NULL);
	if (plugin)
	{
		FRostersModel = qobject_cast<IRostersModel *>(plugin->instance());
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

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
	{
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
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

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

	return true;
}

bool StatusIcons::initObjects()
{
	FCustomIconMenu = new Menu;
	FCustomIconMenu->setTitle(tr("Status icon"));

	FDefaultIconAction = new Action(FCustomIconMenu);
	FDefaultIconAction->setText(tr("Default"));
	FDefaultIconAction->setCheckable(true);
	connect(FDefaultIconAction,SIGNAL(triggered(bool)),SLOT(onSetCustomIconsetByAction(bool)));
	FCustomIconMenu->addAction(FDefaultIconAction,AG_DEFAULT-1,true);

	FDefaultStorage = IconStorage::staticStorage(RSR_STORAGE_STATUSICONS);
	connect(FDefaultStorage,SIGNAL(storageChanged()),SLOT(onDefaultIconsetChanged()));

	if (FRostersModel)
	{
		FRostersModel->insertRosterDataHolder(RDHO_STATUSICONS,this);
	}

	loadStorages();
	return true;
}

bool StatusIcons::initSettings()
{
	Options::setDefaultValue(OPV_STATUSICONS_DEFAULT,FILE_STORAGE_SHARED_DIR);
	Options::setDefaultValue(OPV_STATUSICONS_RULE_ICONSET,FILE_STORAGE_SHARED_DIR);

	if (FOptionsManager)
	{
		FOptionsManager->insertOptionsDialogHolder(this);
	}

	return true;
}

QMultiMap<int, IOptionsDialogWidget *> StatusIcons::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (FOptionsManager!=NULL && ANodeId==OPN_APPEARANCE)
	{
		QComboBox *cmbStatusIcons = new QComboBox(AParent);
		cmbStatusIcons->setItemDelegate(new IconsetDelegate(cmbStatusIcons));

		int index = 0;
		for (QMap<QString, IconStorage *>::const_iterator it=FStorages.constBegin(); it!=FStorages.constEnd(); ++it)
		{
			QString name = it.value()->storageProperty(FILE_STORAGE_NAME,it.key());
			cmbStatusIcons->addItem(it.value()->getIcon(SIK_ONLINE),name,it.key());

			cmbStatusIcons->setItemData(index,it.value()->storage(),IconsetDelegate::IDR_STORAGE);
			cmbStatusIcons->setItemData(index,it.value()->subStorage(),IconsetDelegate::IDR_SUBSTORAGE);
			cmbStatusIcons->setItemData(index,true,IconsetDelegate::IDR_HIDE_STORAGE_NAME);

			index++;
		}
		cmbStatusIcons->model()->sort(0);

		widgets.insertMulti(OHO_APPEARANCE_ROSTER, FOptionsManager->newOptionsDialogHeader(tr("Contacts list"),AParent));
		widgets.insertMulti(OWO_APPEARANCE_STATUSICONS, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_STATUSICONS_DEFAULT),tr("Status icons:"),cmbStatusIcons,AParent));
	}
	return widgets;
}

QList<int> StatusIcons::rosterDataRoles(int AOrder) const
{
	if (AOrder == RDHO_STATUSICONS)
		return QList<int>() << Qt::DecorationRole;
	return QList<int>();
}

QVariant StatusIcons::rosterData(int AOrder, const IRosterIndex *AIndex, int ARole) const
{
	if (AOrder==RDHO_STATUSICONS && ARole==Qt::DecorationRole)
	{
		switch (AIndex->kind())
		{
		case RIK_CONTACTS_ROOT:
			return iconByStatus(AIndex->data(RDR_SHOW).toInt(),SUBSCRIPTION_BOTH,false);
		case RIK_STREAM_ROOT:
		case RIK_CONTACT:
		case RIK_AGENT:
		case RIK_MY_RESOURCE:
			return iconByJid(AIndex->data(RDR_STREAM_JID).toString(),AIndex->data(RDR_FULL_JID).toString());
		}
	}
	return QVariant();
}

bool StatusIcons::setRosterData(int AOrder, const QVariant &AValue, IRosterIndex *AIndex, int ARole)
{
	Q_UNUSED(AOrder); Q_UNUSED(AIndex); Q_UNUSED(ARole); Q_UNUSED(AValue);
	return false;
}

QList<QString> StatusIcons::rules(RuleType ARuleType) const
{
	switch (ARuleType)
	{
	case UserRule:
		return FUserRules.keys();
	case DefaultRule:
		return FDefaultRules.keys();
	}
	return QList<QString>();
}

QString StatusIcons::ruleIconset(const QString &APattern, RuleType ARuleType) const
{
	switch (ARuleType)
	{
	case UserRule:
		return FUserRules.value(APattern,FDefaultStorage!=NULL ? FDefaultStorage->subStorage() : FILE_STORAGE_SHARED_DIR);
	case DefaultRule:
		return FDefaultRules.value(APattern,FDefaultStorage!=NULL ? FDefaultStorage->subStorage() : FILE_STORAGE_SHARED_DIR);
	}
	return QString();
}

void StatusIcons::insertRule(const QString &APattern, const QString &ASubStorage, RuleType ARuleType)
{
	if (!APattern.isEmpty() && !ASubStorage.isEmpty() && QRegExp(APattern).isValid())
	{
		switch (ARuleType)
		{
		case UserRule:
			LOG_DEBUG(QString("User status icon rule inserted, pattern=%1, storage=%2").arg(APattern,ASubStorage));
			FUserRules.insert(APattern,ASubStorage);
			break;
		case DefaultRule:
			LOG_DEBUG(QString("Default status icon rule inserted, pattern=%1, storage=%2").arg(APattern,ASubStorage));
			FDefaultRules.insert(APattern,ASubStorage);
			break;
		}

		FJid2Storage.clear();
		emit ruleInserted(APattern,ASubStorage,ARuleType);

		startStatusIconsUpdate();
	}
	else
	{
		REPORT_ERROR("Failed to insert status icon rule: Invalid params");
	}
}

void StatusIcons::removeRule(const QString &APattern, RuleType ARuleType)
{
	if (rules(ARuleType).contains(APattern))
	{
		switch (ARuleType)
		{
		case UserRule:
			LOG_DEBUG(QString("User status icon rule removed, pattern=%1").arg(APattern));
			FUserRules.remove(APattern);
			break;
		case DefaultRule:
			LOG_DEBUG(QString("Default status icon rule removed, pattern=%1").arg(APattern));
			FDefaultRules.remove(APattern);
			break;
		}

		FJid2Storage.clear();
		emit ruleRemoved(APattern,ARuleType);

		startStatusIconsUpdate();
	}
}

QIcon StatusIcons::iconByJid(const Jid &AStreamJid, const Jid &AContactJid) const
{
	QString substorage = iconsetByJid(AContactJid);
	QString iconKey = iconKeyByJid(AStreamJid, AContactJid);
	IconStorage *storage = FStorages.value(substorage,FDefaultStorage);
	return storage!=NULL ? storage->getIcon(iconKey) : QIcon();
}

QIcon StatusIcons::iconByStatus(int AShow, const QString &ASubscription, bool AAsk) const
{
	QString iconKey = iconKeyByStatus(AShow,ASubscription,AAsk);
	return FDefaultStorage!=NULL ? FDefaultStorage->getIcon(iconKey) : QIcon();
}

QIcon StatusIcons::iconByJidStatus(const Jid &AContactJid, int AShow, const QString &ASubscription, bool AAsk) const
{
	QString substorage = iconsetByJid(AContactJid);
	QString iconKey = iconKeyByStatus(AShow,ASubscription,AAsk);
	IconStorage *storage = FStorages.value(substorage,FDefaultStorage);
	return storage!=NULL ? storage->getIcon(iconKey) : QIcon();
}

QString StatusIcons::iconsetByJid(const Jid &AContactJid) const
{
	QString &substorage = FJid2Storage[AContactJid];
	if (substorage.isEmpty())
	{
		QRegExp regExp;
		regExp.setCaseSensitivity(Qt::CaseSensitive);

		QString contactStr = AContactJid.pFull();

		for (QMap<QString, QString>::const_iterator it=FUserRules.constBegin(); substorage.isEmpty() && it!=FUserRules.constEnd(); ++it)
		{
			regExp.setPattern(it.key());
			if (contactStr.contains(regExp))
				substorage = it.value();
		}

		for (QMap<QString, QString>::const_iterator it=FDefaultRules.constBegin(); substorage.isEmpty() && it!=FDefaultRules.constEnd(); ++it)
		{
			regExp.setPattern(it.key());
			if (contactStr.contains(regExp))
				substorage = it.value();
		}

		if (substorage.isEmpty())
			substorage = FDefaultStorage!=NULL ? FDefaultStorage->subStorage() : FILE_STORAGE_SHARED_DIR;
	}
	return substorage;
}

QString StatusIcons::iconKeyByJid(const Jid &AStreamJid, const Jid &AContactJid) const
{
	bool ask = false;
	int show = IPresence::Offline;
	QString subscription = SUBSCRIPTION_NONE;

	IPresence *presence = FPresenceManager!=NULL ? FPresenceManager->findPresence(AStreamJid) : NULL;
	if (AStreamJid == AContactJid)
	{
		subscription = SUBSCRIPTION_BOTH;
		show = presence!=NULL ? presence->show() : show;
	}
	else if (AStreamJid.pBare() == AContactJid.pBare())
	{
		subscription = SUBSCRIPTION_BOTH;
		show = presence!=NULL ? presence->findItem(AContactJid).show : show;
	}
	else
	{
		IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(AStreamJid) : NULL;
		IRosterItem ritem = roster!=NULL ? roster->findItem(AContactJid) : IRosterItem();
		ask = !ritem.subscriptionAsk.isEmpty();
		subscription = ritem.subscription;
		show = presence!=NULL ? presence->findItem(AContactJid).show : show;
	}
	return iconKeyByStatus(show,subscription,ask);
}

QString StatusIcons::iconKeyByStatus(int AShow, const QString &ASubscription, bool AAsk) const
{
	switch (AShow)
	{
	case IPresence::Offline:
		if (AAsk)
			return SIK_ASK;
		if (ASubscription==SUBSCRIPTION_NONE)
			return SIK_NOAUTH;
		return SIK_OFFLINE;
	case IPresence::Online:
		return SIK_ONLINE;
	case IPresence::Chat:
		return SIK_CHAT;
	case IPresence::Away:
		return SIK_AWAY;
	case IPresence::ExtendedAway:
		return SIK_XAWAY;
	case IPresence::DoNotDisturb:
		return SIK_DND;
	case IPresence::Invisible:
		return SIK_INVISIBLE;
	default:
		return SIK_ERROR;
	}
}

QString StatusIcons::iconFileName(const Jid &AStreamJid, const Jid &AContactJid) const
{
	return iconFileName(iconsetByJid(AContactJid),iconKeyByJid(AStreamJid,AContactJid));
}

QString StatusIcons::iconFileName(const QString &ASubStorage, const QString &AIconKey) const
{
	IconStorage *storage = FStorages.value(ASubStorage,FDefaultStorage);
	return storage!=NULL ? storage->fileFullName(AIconKey) : QString();
}

void StatusIcons::loadStorages()
{
	clearStorages();
	QList<QString> storages = FileStorage::availSubStorages(RSR_STORAGE_STATUSICONS) << FILE_STORAGE_SHARED_DIR;
	foreach(const QString &substorage, storages)
	{
		LOG_DEBUG(QString("Status icon storage added, storage=%1").arg(substorage));

		IconStorage *storage = new IconStorage(RSR_STORAGE_STATUSICONS,substorage,this);
		FStorages.insert(substorage,storage);

		QString pattern = storage->storageProperty(STATUSICONS_STORAGE_PATTERN);
		if (!pattern.isEmpty())
		{
			insertRule(pattern,substorage,IStatusIcons::DefaultRule);
			FStatusRules += pattern;
		}

		Action *action = new Action(FCustomIconMenu);
		action->setCheckable(true);
		action->setIcon(storage->getIcon(SIK_ONLINE));
		action->setText(storage->storageProperty(FILE_STORAGE_NAME,substorage));
		action->setData(ADR_SUBSTORAGE,substorage);
		connect(action,SIGNAL(triggered(bool)),SLOT(onSetCustomIconsetByAction(bool)));
		
		FCustomIconActions.insert(substorage,action);
		FCustomIconMenu->addAction(action,AG_DEFAULT,true);
	}
}

void StatusIcons::clearStorages()
{
	foreach(const QString &rule, FStatusRules)
		removeRule(rule,IStatusIcons::DefaultRule);

	FStatusRules.clear();
	FCustomIconActions.clear();
	
	qDeleteAll(FStorages);
	qDeleteAll(FCustomIconMenu->actions(AG_DEFAULT));
}

void StatusIcons::startStatusIconsUpdate()
{
	if (!FStatusIconsUpdateStarted)
	{
		QTimer::singleShot(0,this,SLOT(onUpdateStatusIcons()));
		FStatusIconsUpdateStarted = true;
	}
}

void StatusIcons::updateCustomIconMenu(const QStringList &APatterns)
{
	QString substorage = FUserRules.value(APatterns.value(0));
	
	FDefaultIconAction->setData(ADR_RULE,APatterns);
	FDefaultIconAction->setIcon(iconByStatus(IPresence::Online,SUBSCRIPTION_BOTH,false));
	FDefaultIconAction->setChecked(APatterns.count()==1 && substorage.isEmpty());

	foreach(Action *action, FCustomIconActions)
	{
		action->setData(ADR_RULE, APatterns);
		action->setChecked(APatterns.count()==1 && action->data(ADR_SUBSTORAGE).toString()==substorage);
	}
}

bool StatusIcons::isSelectionAccepted(const QList<IRosterIndex *> &ASelected) const
{
	foreach(IRosterIndex *index, ASelected)
	{
		Jid streamJid = index->data(RDR_STREAM_JID).toString();
		Jid contactJid = index->data(RDR_PREP_BARE_JID).toString();
		if (!contactJid.isValid() || contactJid.pBare()==streamJid.pBare())
			return false;
	}
	return !ASelected.isEmpty();
}

void StatusIcons::onUpdateStatusIcons()
{
	emit statusIconsChanged();
	emit rosterDataChanged(NULL,Qt::DecorationRole);
	FStatusIconsUpdateStarted = false;
}

void StatusIcons::onPresenceChanged(IPresence *APresence, int AShow, const QString &AStatus, int APriority)
{
	Q_UNUSED(AShow); Q_UNUSED(AStatus); Q_UNUSED(APriority);
	IRosterIndex *sindex = FRostersModel!=NULL ? FRostersModel->streamIndex(APresence->streamJid()) : NULL;
	if (sindex)
		emit rosterDataChanged(sindex,Qt::DecorationRole);
}

void StatusIcons::onRosterItemReceived(IRoster *ARoster, const IRosterItem &AItem, const IRosterItem &ABefore)
{
	if (FRostersModel && (AItem.subscription!=ABefore.subscription || AItem.subscriptionAsk!=ABefore.subscriptionAsk))
	{
		foreach (IRosterIndex *index, FRostersModel->findContactIndexes(ARoster->streamJid(),AItem.itemJid))
			emit rosterDataChanged(index,Qt::DecorationRole);
	}
}

void StatusIcons::onPresenceItemReceived(IPresence *APresence, const IPresenceItem &AItem, const IPresenceItem &ABefore)
{
	if (FRostersModel && AItem.show!=ABefore.show)
	{
		foreach (IRosterIndex *index, FRostersModel->findContactIndexes(APresence->streamJid(),AItem.itemJid))
			emit rosterDataChanged(index,Qt::DecorationRole);
	}
}

void StatusIcons::onRostersViewIndexMultiSelection(const QList<IRosterIndex *> &ASelected, bool &AAccepted)
{
	AAccepted = AAccepted || isSelectionAccepted(ASelected);
}

void StatusIcons::onRostersViewIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
	if (ALabelId==AdvancedDelegateItem::DisplayId && isSelectionAccepted(AIndexes))
	{
		QMap<int, QStringList> rolesMap = FRostersViewPlugin->rostersView()->indexesRolesMap(AIndexes,QList<int>()<<RDR_PREP_BARE_JID,RDR_PREP_BARE_JID);
		
		QStringList patterns;
		foreach(const QString &contactJid, rolesMap.value(RDR_PREP_BARE_JID))
			patterns.append(QRegExp::escape(contactJid));
		updateCustomIconMenu(patterns);

		if (AIndexes.count() > 1)
			FCustomIconMenu->setIcon(iconByStatus(IPresence::Online,SUBSCRIPTION_BOTH,false));
		else if (AIndexes.count() == 1)
			FCustomIconMenu->setIcon(iconByJidStatus(AIndexes.first()->data(RDR_FULL_JID).toString(),IPresence::Online,SUBSCRIPTION_BOTH,false));

		AMenu->addAction(FCustomIconMenu->menuAction(),AG_RVCM_STATUSICONS_CUSTOM,true);
	}
}

void StatusIcons::onMultiUserContextMenu(IMultiUserChatWindow *AWindow, IMultiUser *AUser, Menu *AMenu)
{
	Q_UNUSED(AWindow);
	QString pattern = QString(".*@%1/%2").arg(QRegExp::escape(AUser->userJid().pDomain())).arg(QRegExp::escape(AUser->nick()));
	updateCustomIconMenu(QStringList() << pattern);
	FCustomIconMenu->setIcon(iconByJidStatus(AUser->userJid(),IPresence::Online,SUBSCRIPTION_BOTH,false));
	AMenu->addAction(FCustomIconMenu->menuAction(),AG_MUCM_STATUSICONS_CUSTOM,true);
}

void StatusIcons::onOptionsOpened()
{
	foreach(const QString &nspace, Options::node(OPV_STATUSICONS_RULES_ROOT).childNSpaces("rule"))
	{
		OptionsNode ruleNode = Options::node(OPV_STATUSICONS_RULES_ROOT).node("rule",nspace);
		insertRule(ruleNode.value("pattern").toString(),ruleNode.value("iconset").toString(),IStatusIcons::UserRule);
	}
	onOptionsChanged(Options::node(OPV_STATUSICONS_DEFAULT));
}

void StatusIcons::onOptionsClosed()
{
	Options::node(OPV_STATUSICONS_RULES_ROOT).removeChilds();

	int nspace = 0;
	for (QMap<QString,QString>::const_iterator it=FUserRules.constBegin(); it!=FUserRules.constEnd(); ++it)
	{
		OptionsNode ruleNode = Options::node(OPV_STATUSICONS_RULES_ROOT).node("rule",QString::number(nspace));
		ruleNode.setValue(it.key(),"pattern");
		ruleNode.setValue(it.value(),"iconset");
		nspace++;
	}
}

void StatusIcons::onOptionsChanged(const OptionsNode &ANode)
{
	if (FDefaultStorage && ANode.path()==OPV_STATUSICONS_DEFAULT)
	{
		if (IconStorage::availSubStorages(RSR_STORAGE_STATUSICONS).contains(ANode.value().toString()))
			FDefaultStorage->setSubStorage(ANode.value().toString());
		else
			FDefaultStorage->setSubStorage(FILE_STORAGE_SHARED_DIR);
	}
}

void StatusIcons::onDefaultIconsetChanged()
{
	IconStorage *storage = qobject_cast<IconStorage*>(sender());
	if (storage)
	{
		LOG_INFO(QString("Default status icon storage changed to=%1").arg(storage->subStorage()));

		FJid2Storage.clear();
		emit defaultIconsetChanged(storage->subStorage());
		emit defaultIconsChanged();

		startStatusIconsUpdate();
	}
}

void StatusIcons::onSetCustomIconsetByAction(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		QString substorage = action->data(ADR_SUBSTORAGE).toString();
		foreach(const QString &rule, action->data(ADR_RULE).toStringList())
		{
			if (substorage.isEmpty())
				removeRule(rule,IStatusIcons::UserRule);
			else
				insertRule(rule,substorage,IStatusIcons::UserRule);
		}
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_statusicons, StatusIcons)
#endif
