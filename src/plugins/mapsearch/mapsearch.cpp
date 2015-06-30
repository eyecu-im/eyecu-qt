#include "mapsearch.h"
#include "mapsearchdialog.h"
#include "mapsearchoptions.h"
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <definitions/toolbargroups.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/actiongroups.h>
#include <definitions/shortcutgrouporders.h>
#include <definitions/shortcuts.h>
#include <utils/shortcuts.h>

MapSearch::MapSearch(QObject *parent):
    QObject(parent),
    FMap(NULL),
    FPoi(NULL),
    FMainWindowPlugin(NULL),
    FConnectionManager(NULL),
    FOptionsManager(NULL),
	FMapSearchDialog(NULL)
{}

void MapSearch::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Map Search");
    APluginInfo->description = tr("Allows to search objects on the map, using different search providers");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(POI_UUID);
}

bool MapSearch::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    IPlugin *plugin = APluginManager->pluginInterface("IPoi").value(0,NULL);
    if (plugin)
        FPoi = qobject_cast<IPoi *>(plugin->instance());
    else
        return false;

    plugin = APluginManager->pluginInterface("IMainWindowPlugin").value(0,NULL);
    if (plugin)
        FMainWindowPlugin = qobject_cast<IMainWindowPlugin *>(plugin->instance());
    else
        return false;

    QList<IPlugin *> plugins = APluginManager->pluginInterface("IMapSearchProvider");
    if (plugins.isEmpty())
        return false;       // No providers found

    plugin = APluginManager->pluginInterface("IConnectionManager").value(0,NULL);
    if (plugin)
        FConnectionManager = qobject_cast<IConnectionManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
    FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
    if (FMessageWidgets)
    {
        connect(FMessageWidgets->instance(), SIGNAL(chatWindowCreated(IMessageChatWindow *)),SLOT(onChatWindowCreated(IMessageChatWindow *)));
        connect(FMessageWidgets->instance(),SIGNAL(normalWindowCreated(IMessageNormalWindow *)),SLOT(onNormalWindowCreated(IMessageNormalWindow *)));
    }

    for (QList<IPlugin *>::const_iterator it=plugins.constBegin(); it!=plugins.constEnd(); it++)
        FSearchProviders.insert((*it)->pluginUuid(), qobject_cast<IMapSearchProvider *>((*it)->instance()));

    plugin = APluginManager->pluginInterface("IMap").value(0,NULL);
    if (plugin)
        FMap = qobject_cast<IMap *>(plugin->instance());

    AInitOrder = 250;   // Init after all the providers
    return true;
}

bool MapSearch::initObjects()
{
    // Shortcuts
    Shortcuts::declareGroup(SCTG_MAPSEARCH, tr("Map Search"), SGO_MAPSEARCH);
    Shortcuts::declareShortcut(SCT_MAPSEARCH_CLEARLIST, tr("Clear list"), tr("Ctrl+C", "Clear"), Shortcuts::WindowShortcut);
    Shortcuts::declareShortcut(SCT_MAPSEARCH_SHOW, tr("Toggle showing search results on the map"), tr("Alt+S", "Toggle showing search results on the map"), Shortcuts::WindowShortcut);
    Shortcuts::declareShortcut(SCT_MAPSEARCH_LIMITRANGE, tr("Toggle \"Limit search range\" option"), tr("Alt+L", "Toggle \"Limit search range\" option"), Shortcuts::WindowShortcut);
    Shortcuts::declareShortcut(SCT_MAPSEARCH_SELECTPROVIDER, tr("Select provider"), tr("Alt+P", "Select provider"), Shortcuts::WindowShortcut);
    Shortcuts::declareShortcut(SCT_MAPSEARCH_SEARCHDIALOG, tr("Search dialog"), tr("Alt+F7", "Search"), Shortcuts::ApplicationShortcut);
    Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_POI_INSERTSEARCHRESULT, tr("Insert search result"), tr("Alt+S", "Add search result"), Shortcuts::WindowShortcut);    

    Action *action;
    if (FMap)
        action=FMap->addMenuAction(tr("Search"), RSR_STORAGE_MENUICONS, MNI_MAPSEARCH, 1);
    else
    {
        action = new Action();
        action->setText(tr("Search"));
        action->setIcon(RSR_STORAGE_MENUICONS, MNI_MAPSEARCH);
        IMainWindow *mainWindow = FMainWindowPlugin->mainWindow();
        mainWindow ->topToolBarChanger()->insertAction(action, TBG_MWTTB_MAPS); // Add action as a button
    }
    action->setShortcutId(SCT_MAPSEARCH_SEARCHDIALOG);
    connect(action, SIGNAL(triggered()), SLOT(onMapSearchTriggered()));
    return true;
}

bool MapSearch::initSettings()
{
    Options::setDefaultValue(OPV_MAP_SEARCH_LABEL_COLOR, QColor(Qt::green));
    Options::setDefaultValue(OPV_MAP_SEARCH_SHOW, true);
    Options::setDefaultValue(OPV_MAP_SEARCH_LIMITRANGE, false);
    Options::setDefaultValue(OPV_MAP_SEARCH_PAGESIZE, 10);
    Options::setDefaultValue(OPV_MAP_SEARCH_PAGESIZE_DEFAULT, true);
    Options::setDefaultValue(OPV_MAP_SEARCH_PROVIDER, "{8c42dfa0-ba37-218c-b6d0-824fcbaf22fe}");
    Options::setDefaultValue(OPV_MAP_SEARCH_PROXY, APPLICATION_PROXY_REF_UUID);

    if (FOptionsManager)
    {
		IOptionsDialogNode dnode = {ONO_MAPSEARCH, OPN_MAPSEARCH, MNI_MAPSEARCH, tr("Map Search")};
        FOptionsManager->insertOptionsDialogNode(dnode);
		FOptionsManager->insertOptionsDialogHolder(this);
    }
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> MapSearch::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
    if (ANodeId == OPN_MAPSEARCH)
    {
		widgets.insertMulti(OHO_MAPSEARCH_GENERAL, FOptionsManager->newOptionsDialogHeader("General", AParent));
		widgets.insertMulti(OWO_MAPSEARCH_GENERAL, new MapSearchOptions(AParent));
		widgets.insertMulti(OHO_MAPSEARCH_CONNECTION, FOptionsManager->newOptionsDialogHeader("Connection", AParent));
        if (FConnectionManager)
			widgets.insertMulti(OWO_MAPSEARCH_CONNECTION, FConnectionManager->proxySettingsWidget(Options::node(OPV_MAP_SEARCH_PROXY), AParent));
		widgets.insertMulti(OHO_MAPSEARCH_PROVIDERS, FOptionsManager->newOptionsDialogHeader("Providers", AParent));
    }
	return widgets;
}

QSharedPointer<MapSearchDialog> &MapSearch::mapSearchDialog()
{
	if (!FMapSearchDialog)
		FMapSearchDialog = QSharedPointer<MapSearchDialog>(new MapSearchDialog(FSearchProviders, FMap, FPoi, FOptionsManager, FConnectionManager));
	return FMapSearchDialog;
}

void MapSearch::onMapSearchTriggered()
{
	if (!mapSearchDialog()->isVisible())
		mapSearchDialog()->show(true);
	else if (!mapSearchDialog()->isActiveWindow())
		mapSearchDialog()->activateWindow();
}

void MapSearch::onChatWindowCreated(IMessageChatWindow *AWindow)
{
    if (AWindow->viewWidget() != NULL )
    {
        QList<QAction *> actions=AWindow->toolBarWidget()->toolBarChanger()->groupItems(TBG_MWTBW_POI_VIEW);
        if (!actions.isEmpty())
        {
            Action *action=AWindow->toolBarWidget()->toolBarChanger()->handleAction(actions[0]);
            if (action)
            {
                Menu *menu=action->menu();
                if (menu)
                {
                    Action *action = new Action(menu);
                    action->setText(tr("Insert search result"));
                    action->setIcon(RSR_STORAGE_MENUICONS, MNI_MAPSEARCH);
                    action->setData(IPoi::ADR_MESSAGE_TYPE, Message::Chat);
                    action->setData(IPoi::ADR_CONTACT_JID, AWindow->contactJid().full());
                    action->setData(IPoi::ADR_STREAM_JID, AWindow->streamJid().full());
                    action->setShortcutId(SCT_MESSAGEWINDOWS_POI_INSERTSEARCHRESULT);
                    connect(action, SIGNAL(triggered(bool)), SLOT(onInsertSearchResult(bool)));
                    menu->addAction(action, IPoi::AG_PERSISTENT, false);
                }
            }
        }
    }
}

void MapSearch::onNormalWindowCreated(IMessageNormalWindow *AWindow)
{
    if (AWindow->viewWidget() != NULL )
    {
        QList<QAction *> actions=AWindow->editWidget()->editToolBarChanger()->groupItems(TBG_MWTBW_POI_VIEW);
        if (!actions.isEmpty())
        {
            Action *action=AWindow->editWidget()->editToolBarChanger()->handleAction(actions[0]);
            if (action)
            {
                Menu *menu=action->menu();
                if (menu)
                {
                    Action *action = new Action(menu);
                    action->setText(tr("Insert search result"));
                    action->setIcon(RSR_STORAGE_MENUICONS, MNI_MAPSEARCH);
                    action->setData(IPoi::ADR_MESSAGE_TYPE, Message::Normal);
                    action->setData(IPoi::ADR_CONTACT_JID, AWindow->contactJid().full());
                    action->setData(IPoi::ADR_STREAM_JID, AWindow->streamJid().full());
                    action->setShortcutId(SCT_MESSAGEWINDOWS_POI_INSERTSEARCHRESULT);
                    connect(action, SIGNAL(triggered(bool)), SLOT(onInsertSearchResult(bool)));
                    menu->addAction(action, IPoi::AG_PERSISTENT, false);
                }
            }
        }
    }
}

void MapSearch::onInsertSearchResult(bool AChecked)
{
	Q_UNUSED(AChecked)

    Action *action = qobject_cast<Action *>(sender());
    if (action)
	{
		if (mapSearchDialog()->exec())
            FPoi->insertMessagePoi(action->data(IPoi::ADR_STREAM_JID).toString(),
                                   action->data(IPoi::ADR_CONTACT_JID).toString(),
                                   action->data(IPoi::ADR_MESSAGE_TYPE).toInt(),
								   FPoi->getPoi(mapSearchDialog()->selectedId()));
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsearch, MapSearch)
#endif
