#include "rostersearch.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <definitions/actiongroups.h>
#include <definitions/toolbargroups.h>
#include <definitions/mainwindowwidgets.h>
#include <definitions/rosterindexkinds.h>
#include <definitions/rosterindexroles.h>
#include <definitions/rosterproxyorders.h>
#include <definitions/rosterclickhookerorders.h>
#include <definitions/rosterkeyhookerorders.h>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/optionvalues.h>
#include <utils/options.h>
#include <utils/action.h>
#include <utils/logger.h>

RosterSearch::RosterSearch()
{
	FRostersViewPlugin = NULL;
	FMainWindow = NULL;

	FAutoEnabled = false;
	FSearchStarted = false;
	FLastShowOffline = false;

	setDynamicSortFilter(false);
	setFilterCaseSensitivity(Qt::CaseInsensitive);

	FEnableAction = new Action(this);
	FEnableAction->setIcon(RSR_STORAGE_MENUICONS,MNI_ROSTERSEARCH_MENU);
	FEnableAction->setToolTip(tr("Show search toolbar"));
	FEnableAction->setCheckable(true);
	FEnableAction->setChecked(false);
	connect(FEnableAction,SIGNAL(triggered(bool)),SLOT(onEnableActionTriggered(bool)));

	QToolBar *searchToolBar = new QToolBar(tr("Search toolbar"));
	searchToolBar->setAllowedAreas(Qt::TopToolBarArea);
	searchToolBar->setMovable(false);
	FSearchToolBarChanger = new ToolBarChanger(searchToolBar);
	FSearchToolBarChanger->setAutoHideEmptyToolbar(false);
	FSearchToolBarChanger->setSeparatorsVisible(false);
	FSearchToolBarChanger->toolBar()->setVisible(false);

	FSearchEdit = new SearchLineEdit(searchToolBar);
	FSearchEdit->installEventFilter(this);
	FSearchEdit->setSearchMenuVisible(true);
	FSearchEdit->setSelectTextOnFocusEnabled(false);
	FSearchEdit->searchMenu()->setIcon(RSR_STORAGE_MENUICONS,MNI_ROSTERSEARCH_MENU);
#if QT_VERSION >= 0x040700	// *** <<< eyeCU >>> ***
	FSearchEdit->setPlaceholderText(tr("Search for Contacts"));
#endif						// *** <<< eyeCU >>> ***
	connect(FSearchEdit,SIGNAL(searchStart()),SLOT(onSearchEditStart()));
	FSearchToolBarChanger->insertWidget(FSearchEdit);
}

RosterSearch::~RosterSearch()
{

}

void RosterSearch::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Roster Search");
	APluginInfo->description = tr("Allows to search for contacts in the roster");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Potapov S.A. aka Lion";
	APluginInfo->homePage = "http://www.vacuum-im.org";
	APluginInfo->dependences.append(ROSTERSVIEW_UUID);
	APluginInfo->dependences.append(MAINWINDOW_UUID);
}

bool RosterSearch::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);
	IPlugin *plugin = APluginManager->pluginInterface("IRostersViewPlugin").value(0,NULL);
	if (plugin)
	{
		FRostersViewPlugin = qobject_cast<IRostersViewPlugin *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IMainWindowPlugin").value(0,NULL);
	if (plugin)
	{
		IMainWindowPlugin *mainWindowPlugin = qobject_cast<IMainWindowPlugin *>(plugin->instance());
		if (mainWindowPlugin)
			FMainWindow = mainWindowPlugin->mainWindow();
	}

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));

	return FRostersViewPlugin!=NULL && FMainWindow!=NULL;
}

bool RosterSearch::initObjects()
{
	if (FMainWindow)
	{
		FMainWindow->topToolBarChanger()->insertAction(FEnableAction,TBG_MWTTB_ROSTERSEARCH);
		FMainWindow->insertToolBarChanger(MWW_SEARCH_TOOLBAR,FSearchToolBarChanger);
	}

	if (FRostersViewPlugin)
	{
		FRostersViewPlugin->rostersView()->insertClickHooker(RCHO_ROSTERSEARCH,this);
		FRostersViewPlugin->rostersView()->insertKeyHooker(RKHO_ROSTERSEARCH,this);
	}

	insertSearchField(RDR_NAME,tr("Name"));
	insertSearchField(RDR_STATUS,tr("Status"));
	insertSearchField(RDR_FULL_JID,tr("Jabber ID"));
	insertSearchField(RDR_GROUP,tr("Group"));

	return true;
}

bool RosterSearch::initSettings()
{
	Options::setDefaultValue(OPV_ROSTER_SEARCH_ENABLED,true);
	Options::setDefaultValue(OPV_ROSTER_SEARCH_FIELDEBANLED,true);
	return true;
}

bool RosterSearch::rosterIndexSingleClicked(int AOrder, IRosterIndex *AIndex, const QMouseEvent *AEvent)
{
	if (Options::node(OPV_MESSAGES_COMBINEWITHROSTER).value().toBool())
		return rosterIndexDoubleClicked(AOrder, AIndex, AEvent);
	return false;
}

bool RosterSearch::rosterIndexDoubleClicked(int AOrder, IRosterIndex *AIndex, const QMouseEvent *AEvent)
{
	Q_UNUSED(AEvent);
	if (AOrder==RCHO_ROSTERSEARCH && AEvent->modifiers()==Qt::NoModifier)
	{
		if (!searchPattern().isEmpty() && AIndex->childCount()==0)
		{
			FSelectedIndexes.clear();
			FSelectedIndexes.append(AIndex);
			setSearchPattern(QString());
		}
	}
	return false;
}

bool RosterSearch::rosterKeyPressed(int AOrder, const QList<IRosterIndex *> &AIndexes, const QKeyEvent *AEvent)
{
	Q_UNUSED(AIndexes);
	if (AOrder == RKHO_ROSTERSEARCH)
	{
		if ((AEvent->modifiers() & ~(Qt::ShiftModifier|Qt::KeypadModifier))==0)
		{
			QChar key = !AEvent->text().isEmpty() ? AEvent->text().at(0) : QChar();
			if (key.isLetterOrNumber() || key.isPunct())
				return true;
		}
	}
	return false;
}

bool RosterSearch::rosterKeyReleased(int AOrder, const QList<IRosterIndex *> &AIndexes, const QKeyEvent *AEvent)
{
	Q_UNUSED(AIndexes);
	if (AOrder == RKHO_ROSTERSEARCH)
	{
		if ((AEvent->modifiers() & ~(Qt::ShiftModifier|Qt::KeypadModifier))==0)
		{
			QChar key = !AEvent->text().isEmpty() ? AEvent->text().at(0) : QChar();
			if (key.isLetterOrNumber() || key.isPunct())
			{
				if (!isSearchEnabled())
				{
					FSearchEdit->clear();
					setSearchEnabled(true);
					FAutoEnabled = true;
				}
				FSearchEdit->setFocus();
				FSearchEdit->setText(FSearchEdit->text()+AEvent->text().trimmed());
				return true;
			}
		}
	}
	return false;
}

void RosterSearch::startSearch()
{
	QString pattern = isSearchEnabled() ? searchPattern() : QString();

	if (FRostersViewPlugin)
	{
		if (isSearchEnabled() && !pattern.isEmpty())
		{
			if (!FSearchStarted)
			{
				if (FRostersViewPlugin->rostersView()->rostersModel())
				{
					FRostersViewPlugin->setExpandStateActive(false);
					FSelectedIndexes = FRostersViewPlugin->rostersView()->selectedRosterIndexes();
					connect(FRostersViewPlugin->rostersView()->rostersModel()->instance(),SIGNAL(indexDestroyed(IRosterIndex *)),SLOT(onRosterIndexDestroyed(IRosterIndex *)));
				}
				
				FLastShowOffline = Options::node(OPV_ROSTER_SHOWOFFLINE).value().toBool();
				Options::node(OPV_ROSTER_SHOWOFFLINE).setValue(true);
			}
			FSearchStarted = true;
		}
	}

	if (filterRegExp().pattern() != pattern)
	{
		LOG_DEBUG(QString("Changing roster search pattern to='%1'").arg(pattern));
		QRegExp regExp(pattern,Qt::CaseInsensitive,QRegExp::Wildcard);
		setFilterRegExp(regExp);
	}
	invalidate();

	if (FRostersViewPlugin)
	{
		if (FSearchStarted)
		{
			FRostersViewPlugin->rostersView()->instance()->expandAll();
			FRostersViewPlugin->rostersView()->setSelectedRosterIndexes(FSelectedIndexes);
		}

		if (!isSearchEnabled() || pattern.isEmpty())
		{
			if (FSearchStarted)
			{
				if (FRostersViewPlugin->rostersView()->rostersModel())
				{
					FSelectedIndexes.clear();
					disconnect(FRostersViewPlugin->rostersView()->rostersModel()->instance(),SIGNAL(indexDestroyed(IRosterIndex *)),this,SLOT(onRosterIndexDestroyed(IRosterIndex *)));
				}

				Options::node(OPV_ROSTER_SHOWOFFLINE).setValue(FLastShowOffline);

				FRostersViewPlugin->startRestoreExpandState();
				FRostersViewPlugin->setExpandStateActive(true);
			}
			FSearchStarted = false;

			if (FAutoEnabled)
			{
				setSearchEnabled(false);
				FRostersViewPlugin->rostersView()->instance()->setFocus();
			}
		}
	}

	emit searchResultUpdated();
}

QString RosterSearch::searchPattern() const
{
	return FSearchEdit->text();
}

void RosterSearch::setSearchPattern(const QString &APattern)
{
	FSearchEdit->setText(APattern);
	emit searchPatternChanged(APattern);
}

bool RosterSearch::isSearchEnabled() const
{
	return FEnableAction->isChecked();
}

void RosterSearch::setSearchEnabled(bool AEnabled)
{
	FAutoEnabled = false;
	FEnableAction->setChecked(AEnabled);
	if (FRostersViewPlugin)
	{
		if (AEnabled)
			FRostersViewPlugin->rostersView()->insertProxyModel(this,RPO_ROSTERSEARCH_FILTER);
		else
			FRostersViewPlugin->rostersView()->removeProxyModel(this);
	}
	FSearchToolBarChanger->toolBar()->setVisible(AEnabled);
	emit searchStateChanged(AEnabled);
}

void RosterSearch::insertSearchField(int ADataRole, const QString &AName)
{
	LOG_DEBUG(QString("Roster search field inserted, role=%1, name=%2").arg(ADataRole).arg(AName));
	Action *action = FFieldActions.value(ADataRole,NULL);
	if (action == NULL)
	{
		action = new Action(searchFieldsMenu());
		action->setData(Action::DR_SortString,QString("%1").arg(ADataRole,5,10,QChar('0')));
		connect(action,SIGNAL(triggered(bool)),SLOT(onFieldActionTriggered(bool)));
		FFieldActions.insert(ADataRole,action);
		searchFieldsMenu()->addAction(action,AG_DEFAULT,true);
	}
	action->setText(AName);
	action->setCheckable(true);
	action->setChecked(true);
	emit searchFieldInserted(ADataRole,AName);
}

Menu *RosterSearch::searchFieldsMenu() const
{
	return FSearchEdit->searchMenu();
}

QList<int> RosterSearch::searchFields() const
{
	return FFieldActions.keys();
}

bool RosterSearch::isSearchFieldEnabled(int ADataRole) const
{
	return FFieldActions.contains(ADataRole) && FFieldActions.value(ADataRole)->isChecked();
}

void RosterSearch::setSearchFieldEnabled(int ADataRole, bool AEnabled)
{
	if (FFieldActions.contains(ADataRole))
	{
		LOG_DEBUG(QString("Roster search field enabled changed, role=%1, enabled=%2").arg(ADataRole).arg(AEnabled));
		FFieldActions.value(ADataRole)->setChecked(AEnabled);
		emit searchFieldChanged(ADataRole);
	}
}

void RosterSearch::removeSearchField(int ADataRole)
{
	if (FFieldActions.contains(ADataRole))
	{
		LOG_DEBUG(QString("Roster search field removed, role=%1").arg(ADataRole));
		Action *action = FFieldActions.take(ADataRole);
		searchFieldsMenu()->removeAction(action);
		action->deleteLater();
		emit searchFieldRemoved(ADataRole);
	}
}

bool RosterSearch::eventFilter(QObject *AObject, QEvent *AEvent)
{
	if (AObject==FSearchEdit && AEvent->type()==QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(AEvent);
		if (FRostersViewPlugin && keyEvent->key()==Qt::Key_Down)
		{
			QModelIndex index = FRostersViewPlugin->rostersView()->instance()->model()->index(0,0);
			if (index.isValid())
			{
				FRostersViewPlugin->rostersView()->instance()->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
				FRostersViewPlugin->rostersView()->instance()->setCurrentIndex(index);
			}
			FRostersViewPlugin->rostersView()->instance()->setFocus();
			return true;
		}
	}
	return QSortFilterProxyModel::eventFilter(AObject,AEvent);
}

bool RosterSearch::filterAcceptsRow(int ARow, const QModelIndex &AParent) const
{
	if (!searchPattern().isEmpty() && AParent.isValid() && sourceModel()!=NULL)
	{
		QModelIndex index = sourceModel()->index(ARow,0,AParent);
		if (!sourceModel()->hasChildren(index))
		{
			bool accept = true;
			const QRegExp regExp = filterRegExp();
			foreach(int dataField, FFieldActions.keys())
			{
				if (isSearchFieldEnabled(dataField))
				{
					accept = false;
					QVariant fieldData = index.data(dataField);
					QString string = fieldData.type()==QVariant::StringList ? fieldData.toStringList().join("\t") : fieldData.toString();
					if (string.contains(regExp))
						return true;
				}
			}
			return accept;
		}
		else
		{
			for (int childRow = 0; sourceModel()->index(childRow,0,index).isValid(); childRow++)
				if (filterAcceptsRow(childRow,index))
					return true;
			return false;
		}
	}
	return true;
}

void RosterSearch::onFieldActionTriggered(bool)
{
	startSearch();
}

void RosterSearch::onEnableActionTriggered(bool AChecked)
{
	setSearchEnabled(AChecked);
	startSearch();

	if (AChecked)
	{
		FSearchEdit->setFocus();
		FSearchEdit->selectAll();
	}
}

void RosterSearch::onRosterIndexDestroyed(IRosterIndex *AIndex)
{
	FSelectedIndexes.removeAll(AIndex);
}

void RosterSearch::onSearchEditStart()
{
	emit searchPatternChanged(FSearchEdit->text());
	startSearch();
}

void RosterSearch::onOptionsOpened()
{
	foreach(int dataRole, FFieldActions.keys())
		setSearchFieldEnabled(dataRole,Options::node(OPV_ROSTER_SEARCH_FIELDEBANLED,QString::number(dataRole)).value().toBool());
	setSearchEnabled(Options::node(OPV_ROSTER_SEARCH_ENABLED).value().toBool());
}

void RosterSearch::onOptionsClosed()
{
	foreach(int dataRole, FFieldActions.keys())
		Options::node(OPV_ROSTER_SEARCH_FIELDEBANLED,QString::number(dataRole)).setValue(isSearchFieldEnabled(dataRole));
	Options::node(OPV_ROSTER_SEARCH_ENABLED).setValue(isSearchEnabled());
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_rostersearch, RosterSearch)
#endif
