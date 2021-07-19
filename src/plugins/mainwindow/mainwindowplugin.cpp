#include "mainwindowplugin.h"

#include <QApplication>
#include <definitions/actiongroups.h>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/optionnodes.h>
#include <definitions/optionvalues.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/shortcuts.h>
#include <definitions/shortcutgrouporders.h>
#include <utils/widgetmanager.h>
#include <utils/shortcuts.h>
#include <utils/options.h>
#include <utils/action.h>

MainWindowPlugin::MainWindowPlugin()
{
	FPluginManager = NULL;
// *** <<< eyeCU >>> ***
#ifndef Q_OS_SYMBIAN
	FTrayManager = NULL;
// *** <<< eyeCU >>> ***
#endif
	FOptionsManager = NULL;

	FStartShowLoopCount = 0;

	FActivationChanged = QTime::currentTime();
	FMainWindow = new MainWindow(NULL, Qt::Window | Qt::WindowCloseButtonHint);
	FMainWindow->installEventFilter(this);
	WidgetManager::setWindowSticky(FMainWindow,true);
}

MainWindowPlugin::~MainWindowPlugin()
{
	delete FMainWindow;
}

void MainWindowPlugin::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Main Window");
	APluginInfo->description = tr("Allows other modules to place their widgets in the main window");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Potapov S.A. aka Lion";
	APluginInfo->homePage = "http://www.vacuum-im.org";
}

bool MainWindowPlugin::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);

	FPluginManager = APluginManager;
	connect(FPluginManager->instance(),SIGNAL(shutdownStarted()),SLOT(onApplicationShutdownStarted()));
// *** <<< eyeCU >>> ***
#ifndef Q_OS_SYMBIAN
	IPlugin *plugin = APluginManager->pluginInterface("ITrayManager").value(0,NULL);
	if (plugin)
	{
		FTrayManager = qobject_cast<ITrayManager *>(plugin->instance());
		if (FTrayManager)
		{
			connect(FTrayManager->instance(),SIGNAL(notifyActivated(int, QSystemTrayIcon::ActivationReason)),
				SLOT(onTrayNotifyActivated(int,QSystemTrayIcon::ActivationReason)));
		}
	}
// *** <<< eyeCU >>> ***
#endif
	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
	{
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
	}

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));

	connect(Shortcuts::instance(),SIGNAL(shortcutActivated(const QString, QWidget *)),SLOT(onShortcutActivated(const QString, QWidget *)));
	
	return true;
}

bool MainWindowPlugin::initObjects()
{
	Shortcuts::declareShortcut(SCT_GLOBAL_SHOW,tr("Show"),QKeySequence::UnknownKey,Shortcuts::GlobalShortcut); 	// *** <<< eyeCU >>> ***
	Shortcuts::declareShortcut(SCT_ROSTERVIEW_CLOSEWINDOW,QString(),tr("Esc","Close main window"));
	Shortcuts::insertWidgetShortcut(SCT_ROSTERVIEW_CLOSEWINDOW,FMainWindow);

	Action *quitAction = new Action(this);
	quitAction->setText(tr("Quit"));
	quitAction->setIcon(RSR_STORAGE_MENUICONS,MNI_MAINWINDOW_QUIT);
	connect(quitAction,SIGNAL(triggered()),FPluginManager->instance(),SLOT(quit()));
	FMainWindow->mainMenu()->addAction(quitAction,AG_MMENU_MAINWINDOW_QUIT,true);

	if (FTrayManager)
	{
		Action *showRosterAction = new Action(this);		
		showRosterAction->setText(tr("Show")); // *** <<< eyeCU >>> ***
		showRosterAction->setIcon(RSR_STORAGE_MENUICONS,MNI_MAINWINDOW_SHOW_ROSTER);
		connect(showRosterAction,SIGNAL(triggered(bool)),SLOT(onShowMainWindowByAction(bool)));
		FTrayManager->contextMenu()->addAction(showRosterAction,AG_TMTM_MAINWINDOW_SHOW,true);
	}
	if (FOptionsManager)
	{
		FOptionsManager->insertOptionsDialogHolder(this);
	}

	return true;
}

bool MainWindowPlugin::initSettings()
{
	Options::setDefaultValue(OPV_MAINWINDOW_SHOWONSTART,true);
	Options::setDefaultValue(OPV_ROSTER_MINIMIZEONCLOSE,false);
	return true;
}

QMultiMap<int, IOptionsDialogWidget *> MainWindowPlugin::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_ROSTERVIEW)
	{
		widgets.insertMulti(OWO_ROSTER_MINIMIZE_ON_CLOSE,FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ROSTER_MINIMIZEONCLOSE),tr("Minimize roster window instead of closing it"),AParent));
	}
	return widgets;
}

bool MainWindowPlugin::startPlugin()
{
	Shortcuts::setGlobalShortcut(SCT_GLOBAL_SHOW, true); 	// *** <<< eyeCU >>> ***
	return true;
}

IMainWindow *MainWindowPlugin::mainWindow() const
{
	return FMainWindow;
}

bool MainWindowPlugin::eventFilter(QObject *AWatched, QEvent *AEvent)
{
	if (AWatched==FMainWindow && AEvent->type()==QEvent::ActivationChange)
		FActivationChanged = QTime::currentTime();
	return QObject::eventFilter(AWatched,AEvent);
}

void MainWindowPlugin::onOptionsOpened()
{
	FMainWindow->loadWindowGeometryAndState();
	QTimer::singleShot(0,this,SLOT(onShowMainWindowOnStart()));
}

void MainWindowPlugin::onOptionsClosed()
{
	FMainWindow->saveWindowGeometryAndState();
	FMainWindow->closeWindow();
}

void MainWindowPlugin::onApplicationShutdownStarted()
{
	Options::node(OPV_MAINWINDOW_SHOWONSTART).setValue(FMainWindow->isVisible());
}

void MainWindowPlugin::onShowMainWindowOnStart()
{
	if (Options::node(OPV_MAINWINDOW_SHOWONSTART).value().toBool())
		FMainWindow->showWindow();
}

void MainWindowPlugin::onShowMainWindowByAction(bool)
{
	FMainWindow->showWindow();
}

void MainWindowPlugin::onShortcutActivated(const QString &AId, QWidget *AWidget)
{
	if (AWidget==NULL && AId==SCT_GLOBAL_SHOW) // *** <<< eyeCU >>> ***    
	{
		FMainWindow->showWindow();
	}
	else if (AWidget==FMainWindow && AId==SCT_ROSTERVIEW_CLOSEWINDOW)
	{
		FMainWindow->closeWindow();
	}
}

void MainWindowPlugin::onTrayNotifyActivated(int ANotifyId, QSystemTrayIcon::ActivationReason AReason)
{
	if (ANotifyId<=0 && AReason==QSystemTrayIcon::Trigger)
	{
		if (FMainWindow->isActive() || qAbs(FActivationChanged.msecsTo(QTime::currentTime())) < qApp->doubleClickInterval())
			FMainWindow->closeWindow();
		else
			FMainWindow->showWindow();
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mainwindow, MainWindowPlugin)
#endif
