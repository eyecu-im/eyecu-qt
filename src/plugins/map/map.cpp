#include <QApplication>
#include <QGraphicsView>
#include <QPushButton>
#include <MapObject>
#include <MapTile>
#include <QDebug>

#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/shortcuts.h>
#include <definitions/shortcutgrouporders.h>
#include <definitions/version.h>

#include <definitions/toolbargroups.h>
#include <definitions/actiongroups.h>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/mapicons.h>
#include <definitions/rostertooltiporders.h>
#include <definitions/optionvalues.h>

#include <utils/systemmanager.h>

#include "map.h"
#include "mapform.h"
#include "mapoptions.h"

//---------------- *** <<< Map <<< *** -----------------
Map::Map():
	FMapSource(NULL),
	FOptionsManager(NULL),
	FConnectionManager(NULL),
	FNetworkAccessManager(new QNetworkAccessManager(this)),
	FMainWindowPlugin(NULL),
	FPositioning(NULL),
	FIconStorage(NULL),
	FMapForm(NULL),
	FMenuMap(NULL),
	FMenuToolbar(NULL),
	FFollowedObject(NULL),
	FSourceGroup(NULL),
	FMapsWidget(NULL),
	FMouseGrabber(NULL),
	FMyLocation(NULL),
	FFollowMyLocation(false)
{}

Map::~Map()
{
	// Clear object handlers
	for (QHash<int, MapObjectHandlerRecord *>::const_iterator it=FObjectHandlers.constBegin(); it!=FObjectHandlers.constEnd(); it++)
		(*it)->deleteLater();
	FObjectHandlers.clear();
	delete FMenuMap;
	delete FMenuToolbar;
}

//-----------------------------
void Map::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Map");
	APluginInfo->description = tr("Retrieves and displays world map from different sources");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
}

//-----------------------------
bool Map::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	MapHttpQuery::setCachePath(APluginManager->homePath());
	MapHttpQuery::setUserAgent(QString("%1/%2.%3 %4 (%5)").arg(CLIENT_NAME)
														  .arg(APluginManager->version())
														  .arg(APluginManager->revision())
														  .arg(CLIENT_VERSION_SUFFIX)
														  .arg(SystemManager::osVersion()));

	IPlugin *plugin = APluginManager->pluginInterface("IMainWindowPlugin").value(0,NULL);
	if (plugin)
		FMainWindowPlugin = qobject_cast<IMainWindowPlugin *>(plugin->instance());
	else
		return false;

	plugin = APluginManager->pluginInterface("IConnectionManager").value(0,NULL);
	if (plugin)
		FConnectionManager = qobject_cast<IConnectionManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IPositioning").value(0,NULL);
	if (plugin)
	{
		FPositioning = qobject_cast<IPositioning *>(plugin->instance());
		connect(FPositioning->instance(), SIGNAL(newPositionAvailable(GeolocElement)), SLOT(onNewPositionAvailable(GeolocElement)));
	}

	QList<IPlugin *> sources = APluginManager->pluginInterface("IMapSource");
	for (QList<IPlugin *>::const_iterator it = sources.constBegin(); it != sources.constEnd(); it++)
		FMapSources.append(qobject_cast<IMapSource *>((*it)->instance()));
	if (FMapSources.isEmpty())
		return false; // Map plugin is useless without sources

	connect(APluginManager->instance(), SIGNAL(shutdownStarted()), SLOT(onShutdownStarted()));
	connect(this,SIGNAL(mapObjectAdded(MapObject*)),SLOT(onMapObjectAdded(MapObject*)));

	AInitOrder=100; // This one should be initialized AFTER MapScene and BEFORE MapMessage
	return true;
}

//-----------------------------
bool Map::initObjects()
{
	// Shortcuts
	Shortcuts::declareGroup(SCTG_MAP, tr("Map"), SGO_MAP);
	Shortcuts::declareShortcut(SCT_MAP_NEWCENTER, tr("New center"), tr("Ctrl+N", "New center"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MAP_MYLOCATION, tr("My location"), tr("Ctrl+M", "My location"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MAP_SHOW, tr("Toggle map"), tr("Ctrl+F10", "Toggle map"), Shortcuts::ApplicationShortcut);
	Shortcuts::declareShortcut(SCT_MAP_REFRESH, tr("Refresh"), tr("F5", "Refresh map"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MAP_OPTIONS, tr("Show options dialog"), tr("Ctrl+O", "Show options dialog"), Shortcuts::WindowShortcut);

	Shortcuts::declareGroup(SCTG_MAP_MOVE, tr("Movement"), SGO_MAP_MOVE);
	Shortcuts::declareShortcut(SCT_MAP_MOVE_LEFT, tr("Left"), tr("Ctrl+Left", "Map move Left"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MAP_MOVE_RIGHT, tr("Right"), tr("Ctrl+Right", "Map move Right"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MAP_MOVE_UP, tr("Up"), tr("Ctrl+Up", "Map move Up"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MAP_MOVE_DOWN, tr("Down"), tr("Ctrl+Down", "Map move Down"), Shortcuts::WindowShortcut);

	Shortcuts::declareGroup(SCTG_MAP_ZOOM, tr("Zoom"), SGO_MAP_ZOOM);
	Shortcuts::declareShortcut(SCT_MAP_ZOOM_IN, tr("In"), tr("+", "Map zoom in"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MAP_ZOOM_OUT, tr("Out"), tr("-", "Map zoom out"), Shortcuts::WindowShortcut);

	MapTile::buildNotFound();
	FIconStorage = IconStorage::staticStorage(RSR_STORAGE_MAPICONS);
	MapTile::setLoading(FIconStorage->getIcon(MPI_HOURGLASS).pixmap(256, 256));
	MapHttpQuery::setNetworkAccessManager(FNetworkAccessManager);	

	FMenuMap = new Menu;
	FMenuMap->setTitle(tr("Map"));
	FMenuMap->setIcon(RSR_STORAGE_MENUICONS, MNI_MAP);
	FMenuMap->menuAction()->setEnabled(true);

	FMenuToolbar = new Menu;
	FMenuToolbar->setTitle(tr("Map"));
	FMenuToolbar->setIcon(RSR_STORAGE_MENUICONS, MNI_MAP);
	FMenuToolbar->menuAction()->setEnabled(true);
	FMenuToolbar->menuAction()->setCheckable(true);

	FMenuObject = new Menu;

	FMapForm = new MapForm(this, createScene(this, this));
	Shortcuts::insertWidgetShortcut(SCT_MAP_ZOOM_IN, FMapForm);
	Shortcuts::insertWidgetShortcut(SCT_MAP_ZOOM_OUT, FMapForm);
	connect(Shortcuts::instance(), SIGNAL(shortcutActivated(QString,QWidget*)), SLOT(onShortcutActivated(QString,QWidget*)));

	FMainWindow = FMainWindowPlugin->mainWindow();

	QToolButton *action = FMainWindow ->topToolBarChanger()    // Get toolbar changer
			   ->insertAction(FMenuToolbar->menuAction(), TBG_MWTTB_MAPS); // Add action as a button

	action->setPopupMode(QToolButton::MenuButtonPopup);
	FMenuToolbar->menuAction()->setShortcutId(SCT_MAP_SHOW);
	connect(FMenuToolbar->menuAction(), SIGNAL(toggled(bool)), SLOT(showMap(bool)));
	fillMenu();
	fillSources();

	if (FPositioning)
		showMyLocation();

	connect(FMainWindow->instance(), SIGNAL(centralWidgetVisibleChanged(bool)), SLOT(onCentralWidgetVisibleChanged(bool)));
	connect(FMainWindow->mainCentralWidget()->instance(), SIGNAL(currentCentralPageChanged(IMainCentralPage*)), SLOT(onCurrentCentralPageChanged(IMainCentralPage*)));
	connect(FMapForm->mapScene()->instance(), SIGNAL(sceneRectChanged(QRectF)), SIGNAL(sceneRectChanged(QRectF)));
	connect(FMapForm->mapScene()->instance(), SIGNAL(mapCenterChanged(double,double,bool)), SLOT(onMapCenterChanged(double,double,bool)));

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsChanged(OptionsNode)),SLOT(onOptionsChanged(OptionsNode)));
	connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
	return true;
}

bool Map::initSettings()
{
	QPalette palette=QApplication::palette();
	Options::setDefaultValue(OPV_MAP_ZOOM, 13);
	Options::setDefaultValue(OPV_MAP_SOURCE, QString());
	Options::setDefaultValue(OPV_MAP_MODE, TYPE_MAP);
	Options::setDefaultValue(OPV_MAP_COORDS, QPointF(0.0, 51.47772));
	Options::setDefaultValue(OPV_MAP_ZOOM_WHEEL, RelativeToMousePointerPosition);
	Options::setDefaultValue(OPV_MAP_ZOOM_SLIDERTRACK, false);
	Options::setDefaultValue(OPV_MAP_OSD_BOX_SHAPE, QFrame::Box);
	Options::setDefaultValue(OPV_MAP_OSD_BOX_SHADOW, QFrame::Raised);
	Options::setDefaultValue(OPV_MAP_OSD_BOX_FOREGROUND, palette.color(QPalette::Text));
	Options::setDefaultValue(OPV_MAP_OSD_BOX_LIGHT, palette.color(QPalette::Light));
	Options::setDefaultValue(OPV_MAP_OSD_BOX_MIDLIGHT, palette.color(QPalette::Midlight));
	Options::setDefaultValue(OPV_MAP_OSD_BOX_DARK, palette.color(QPalette::Dark));
	Options::setDefaultValue(OPV_MAP_OSD_BOX_BACKGROUND_COLOR, palette.color(QPalette::Window));
	Options::setDefaultValue(OPV_MAP_OSD_BOX_BACKGROUND_ALPHA, 224);
	Options::setDefaultValue(OPV_MAP_OSD_BOX_BACKGROUND_TRANSPARENT, false);

	Options::setDefaultValue(OPV_MAP_OSD_TEXT_COLOR, QColor(QPalette::BrightText));
	Options::setDefaultValue(OPV_MAP_OSD_SHADOW_COLOR, QColor(QPalette::Shadow));
	Options::setDefaultValue(OPV_MAP_OSD_SHADOW_BLUR, 1);
	Options::setDefaultValue(OPV_MAP_OSD_SHADOW_SHIFT, QPointF(0.5, 0.5));
	Options::setDefaultValue(OPV_MAP_OSD_CONTR_FOREGROUND, palette.color(QPalette::Text));
	Options::setDefaultValue(OPV_MAP_OSD_CONTR_BASE, palette.color(QPalette::Base));
	Options::setDefaultValue(OPV_MAP_OSD_CONTR_BUTTON, palette.color(QPalette::Button));
	Options::setDefaultValue(OPV_MAP_OSD_CONTR_LIGHT, palette.color(QPalette::Light));
	Options::setDefaultValue(OPV_MAP_OSD_CONTR_MIDLIGHT, palette.color(QPalette::Midlight));
	Options::setDefaultValue(OPV_MAP_OSD_CONTR_SHADOW, palette.color(QPalette::Shadow));
	Options::setDefaultValue(OPV_MAP_OSD_CONTR_DARK, palette.color(QPalette::Dark));
	Options::setDefaultValue(OPV_MAP_OSD_CONTR_BACKGROUND_COLOR, palette.color(QPalette::Window));
	Options::setDefaultValue(OPV_MAP_OSD_CONTR_BACKGROUND_ALPHA, 127);
	Options::setDefaultValue(OPV_MAP_OSD_CONTR_BACKGROUND_TRANSPARENT, true);
	Options::setDefaultValue(OPV_MAP_OSD_CONTR_CMARKER_COLOR, QColor(Qt::darkYellow));
	Options::setDefaultValue(OPV_MAP_OSD_CONTR_CMARKER_ALPHA, 127);
	Options::setDefaultValue(OPV_MAP_OSD_CONTR_CMARKER_VISIBLE, true);
	Options::setDefaultValue(OPV_MAP_OSD_FONT, QFont("DejaVu Sans Condensed,10,-1,5,50,0,0,0,0,0"));

	Options::setDefaultValue(OPV_MAP_ATTACH_TO_ROSTER, true);
	Options::setDefaultValue(OPV_MAP_SHOWING, true);

	Options::setDefaultValue(OPV_MAP_PROXY, APPLICATION_PROXY_REF_UUID);
	Options::setDefaultValue(OPV_MAP_LOADING, true);

	if (FOptionsManager)
	{
		IOptionsDialogNode dnode = {ONO_MAP, OPN_MAP, MNI_MAP, tr("Map")};
		FOptionsManager->insertOptionsDialogNode(dnode);
		FOptionsManager->insertOptionsDialogHolder(this);
	}
	return true;
}

//-------
void Map::onOptionsOpened()
{
	qDebug() << "Map::onOptionsOpened()";
	FMapForm->selectMapSource(Options::node(OPV_MAP_SOURCE).value().toString());

	onOptionsChanged(Options::node(OPV_MAP_PROXY));
	onOptionsChanged(Options::node(OPV_MAP_LOADING));

	onOptionsChanged(Options::node(OPV_MAP_OSD_CONTR_BACKGROUND_COLOR));
	onOptionsChanged(Options::node(OPV_MAP_OSD_CONTR_BACKGROUND_TRANSPARENT));
	onOptionsChanged(Options::node(OPV_MAP_OSD_CONTR_FOREGROUND));
	onOptionsChanged(Options::node(OPV_MAP_OSD_CONTR_CMARKER_COLOR));
	onOptionsChanged(Options::node(OPV_MAP_OSD_CONTR_CMARKER_VISIBLE));
	onOptionsChanged(Options::node(OPV_MAP_OSD_CONTR_BASE));
	onOptionsChanged(Options::node(OPV_MAP_OSD_CONTR_BUTTON));
	onOptionsChanged(Options::node(OPV_MAP_OSD_CONTR_LIGHT));
	onOptionsChanged(Options::node(OPV_MAP_OSD_CONTR_MIDLIGHT));
	onOptionsChanged(Options::node(OPV_MAP_OSD_CONTR_SHADOW));
	onOptionsChanged(Options::node(OPV_MAP_OSD_CONTR_DARK));

	onOptionsChanged(Options::node(OPV_MAP_OSD_BOX_BACKGROUND_COLOR));
	onOptionsChanged(Options::node(OPV_MAP_OSD_BOX_BACKGROUND_TRANSPARENT));
	onOptionsChanged(Options::node(OPV_MAP_OSD_BOX_SHAPE));
	onOptionsChanged(Options::node(OPV_MAP_OSD_BOX_SHADOW));

	onOptionsChanged(Options::node(OPV_MAP_OSD_FONT));
	onOptionsChanged(Options::node(OPV_MAP_OSD_TEXT_COLOR));
	onOptionsChanged(Options::node(OPV_MAP_OSD_SHADOW_COLOR));

	onOptionsChanged(Options::node(OPV_MAP_ATTACH_TO_ROSTER));
	onOptionsChanged(Options::node(OPV_MAP_SHOWING));

	onOptionsChanged(Options::node(OPV_MAP_SOURCE));
	onOptionsChanged(Options::node(OPV_MAP_MODE));
	onOptionsChanged(Options::node(OPV_MAP_ZOOM));
	onOptionsChanged(Options::node(OPV_MAP_COORDS));
	onOptionsChanged(Options::node(OPV_MAP_ZOOM_WHEEL));
	onOptionsChanged(Options::node(OPV_MAP_ZOOM_SLIDERTRACK));

	QPointF coords=Options::node(OPV_MAP_COORDS).value().toPointF();
	FMapForm->mapScene()->setMapCenter(coords.y(), coords.x());
}

void Map::onOptionsClosed()
{
	FMapForm->closeOptions();
}

void Map::onShutdownStarted()
{
	if (FMapForm)
		FMapForm->disableHideEvent();
}

void Map::fillMenu()
{
	Action *action;
	FMyLocation = addMenuAction(tr("My location"), RSR_STORAGE_MAPICONS, MPI_MYLOCATION, 1);
	FMyLocation->setShortcutId(SCT_MAP_MYLOCATION);
	FMyLocation->setDisabled(FFollowMyLocation);
	FMapForm->showMapCenter(!FFollowMyLocation);
	connect(FMyLocation, SIGNAL(triggered(bool)), SLOT(showMyLocation()));

	FMenuMap->insertSeparator(FMyLocation);
	FMenuToolbar->insertSeparator(FMyLocation);

	action = addMenuAction(tr("New center"), RSR_STORAGE_MAPICONS, MPI_NEWCENTER, 1);
	action->setShortcutId(SCT_MAP_NEWCENTER);
	connect(action, SIGNAL(triggered(bool)), FMapForm, SLOT(onSetNewCenter()));

	action = addMenuAction(tr("Options"), RSR_STORAGE_MENUICONS, MNI_OPTIONS_DIALOG, 1);
	action->setShortcutId(SCT_MAP_OPTIONS);
	connect(action, SIGNAL(triggered(bool)), SLOT(showOptions()));
}

void Map::fillSources()
{
	FMapForm->addMapSource(tr("No map"), FIconStorage->getIcon(MPI_NOMAP));

	IconStorage *storage = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
	for (QList<IMapSource*>::const_iterator it=FMapSources.constBegin(); it!=FMapSources.constEnd(); it++)
		FMapForm->addMapSource((*it)->mapSource()->getFriendlyName(),
							   storage->getIcon((*it)->getIconId()),
							   qobject_cast<IPlugin *>((*it)->instance())->pluginUuid());
}

void Map::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path()==OPV_MAP_COORDS) // Coordinates
		FMapForm->setMapCenter(ANode.value().toPointF());
	else if (ANode.path()==OPV_MAP_SOURCE)
		FMapForm->setMapSource(getMapSource(findMapSource(ANode.value().toString())));
	else if (ANode.path()==OPV_MAP_MODE)
		FMapForm->setMapMode(ANode.value().toInt());
	else if (ANode.path()==OPV_MAP_ZOOM)
	{
		MercatorCoordinates coordsOld;
		MapScene *scene = getScene();
		if (!FMouseWheelPosition.isNull())
			coordsOld=scene->calculateLocation(scene->translatePos(FMouseWheelPosition, true));
		scene->setZoom(ANode.value().toInt());
		if (!FMouseWheelPosition.isNull())
		{
			switch (Options::node(OPV_MAP_ZOOM_WHEEL).value().toInt())
			{
				case RelativeToMousePointerPosition:
				{
					MercatorCoordinates coordsNew=scene->calculateLocation(scene->translatePos(FMouseWheelPosition, true));
					MercatorCoordinates center=mapCenter();
					scene->setMapCenter(center.latitude()+coordsOld.latitude()-coordsNew.latitude(),
										center.longitude()+coordsOld.longitude()-coordsNew.longitude(),
										true);
					break;
				}
				case FollowMousePointer:
					scene->setMapCenter(coordsOld, true);
					break;
				case RelativeToMapCenter:
					break;
			}
			FMouseWheelPosition=QPointF();
		}

		if (FMouseGrabber)
		{
			MercatorCoordinates coords=scene->calculateLocation(FMousePosition);
			setSelectionCoordinates(coords);
			FMouseGrabber->mapMouseMoved(FMousePosition, FMousePosition, coords, Qt::NoButton); // Notify grabber about coordinates change
		}
		emit zoomChanged(ANode.value().toInt());
	}
	else if (ANode.path()==OPV_MAP_ZOOM_SLIDERTRACK)
	{
		qDebug() << "OPV_MAP_ZOOM_SLIDERTRACK";
		qDebug() << "value=" << ANode.value().toBool();
		FMapForm->setZoomSliderTracknig(ANode.value().toBool());
	}
	else if (ANode.path()==OPV_MAP_OSD_FONT)
	{
		FMapForm->setOsdFont(Options::node(OPV_MAP_OSD_FONT).value().value<QFont>());
	}
	else if (ANode.path()==OPV_MAP_OSD_TEXT_COLOR)
	{
		QColor text = ANode.value().value<QColor>();
		FMapForm->setOsdTextColor(text);
	}
	else if (ANode.path()==OPV_MAP_OSD_CONTR_FOREGROUND)
	{
		QColor color = ANode.value().value<QColor>();
		FMapForm->setOsdControlColor(QPalette::Text, color);
	}
	else if (ANode.path()==OPV_MAP_OSD_CONTR_BASE)
	{
		QColor color = ANode.value().value<QColor>();
		FMapForm->setOsdControlColor(QPalette::Base, color);
	}
	else if (ANode.path()==OPV_MAP_OSD_CONTR_BUTTON)
	{
		QColor color = ANode.value().value<QColor>();
		FMapForm->setOsdControlColor(QPalette::Button, color);
	}
	else if (ANode.path()==OPV_MAP_OSD_CONTR_LIGHT)
	{
		QColor color = ANode.value().value<QColor>();
		FMapForm->setOsdControlColor(QPalette::Light, color);
	}
	else if (ANode.path()==OPV_MAP_OSD_CONTR_MIDLIGHT)
	{
		QColor color = ANode.value().value<QColor>();
		FMapForm->setOsdControlColor(QPalette::Midlight, color);
	}
	else if (ANode.path()==OPV_MAP_OSD_CONTR_SHADOW)
	{
		QColor color = ANode.value().value<QColor>();
		FMapForm->setOsdControlColor(QPalette::Shadow, color);
	}
	else if (ANode.path()==OPV_MAP_OSD_CONTR_DARK)
	{
		QColor color = ANode.value().value<QColor>();
		FMapForm->setOsdControlColor(QPalette::Dark, color);
	}
	else if (ANode.path()==OPV_MAP_OSD_BOX_FOREGROUND)
		FMapForm->setOsdBoxColor(QPalette::Text, ANode.value().value<QColor>());
	else if (ANode.path()==OPV_MAP_OSD_BOX_LIGHT)
		FMapForm->setOsdBoxColor(QPalette::Light, ANode.value().value<QColor>());
	else if (ANode.path()==OPV_MAP_OSD_BOX_MIDLIGHT)
		FMapForm->setOsdBoxColor(QPalette::Midlight, ANode.value().value<QColor>());
	else if (ANode.path()==OPV_MAP_OSD_BOX_DARK)
		FMapForm->setOsdBoxColor(QPalette::Dark, ANode.value().value<QColor>());
	else if (ANode.path()==OPV_MAP_OSD_BOX_BACKGROUND_TRANSPARENT)
		FMapForm->setOsdBoxBgTransparent(ANode.value().toBool());
	else if (ANode.path()==OPV_MAP_OSD_BOX_BACKGROUND_COLOR)
	{
		int bgAlpha = Options::node(OPV_MAP_OSD_BOX_BACKGROUND_ALPHA).value().toInt();
		QColor bgColor = ANode.value().value<QColor>();
		bgColor.setAlpha(bgAlpha);
		FMapForm->setOsdBoxColor(QPalette::Window, bgColor);
	}
	else if (ANode.path()==OPV_MAP_OSD_BOX_BACKGROUND_ALPHA)
	{
		int bgAlpha = ANode.value().toInt();
		QColor bgColor = Options::node(OPV_MAP_OSD_BOX_BACKGROUND_COLOR).value().value<QColor>();
		bgColor.setAlpha(bgAlpha);
		FMapForm->setOsdBoxColor(QPalette::Window, bgColor);
	}
	else if (ANode.path()==OPV_MAP_OSD_BOX_SHAPE)
		FMapForm->setOsdBoxShape(ANode.value().toInt());
	else if (ANode.path()==OPV_MAP_OSD_BOX_SHADOW)
		FMapForm->setOsdBoxShadow(ANode.value().toInt());
	else if (ANode.path()==OPV_MAP_OSD_CONTR_BACKGROUND_TRANSPARENT)
		FMapForm->setOsdControlBgTransparent(ANode.value().toBool());
	else if (ANode.path()==OPV_MAP_OSD_CONTR_BACKGROUND_COLOR)
	{
		QColor color = ANode.value().value<QColor>();
		color.setAlpha(Options::node(OPV_MAP_OSD_CONTR_BACKGROUND_ALPHA).value().toInt());
		FMapForm->setOsdControlColor(QPalette::Background, color);
	}
	else if (ANode.path()==OPV_MAP_OSD_CONTR_BACKGROUND_ALPHA)
	{
		QColor color = Options::node(OPV_MAP_OSD_CONTR_BACKGROUND_COLOR).value().value<QColor>();
		color.setAlpha(ANode.value().toInt());
		FMapForm->setOsdControlColor(QPalette::Background, color);
	}
	else if (ANode.path()==OPV_MAP_OSD_SHADOW_COLOR)
	{
		int alpha = Options::node(OPV_MAP_OSD_SHADOW_ALPHA).value().toInt();
		QColor color = ANode.value().value<QColor>();
		color.setAlpha(alpha);
		qreal blur = Options::node(OPV_MAP_OSD_SHADOW_BLUR).value().toReal();
		QPointF shift = Options::node(OPV_MAP_OSD_SHADOW_SHIFT).value().toPointF();
		FMapForm->setOsdShadow(color, blur, shift);
	}
	else if (ANode.path()==OPV_MAP_OSD_SHADOW_ALPHA)
	{
		int alpha = ANode.value().toInt();
		QColor color = Options::node(OPV_MAP_OSD_SHADOW_COLOR).value().value<QColor>();
		color.setAlpha(alpha);
		qreal blur = Options::node(OPV_MAP_OSD_SHADOW_BLUR).value().toReal();
		QPointF shift = Options::node(OPV_MAP_OSD_SHADOW_SHIFT).value().toPointF();
		FMapForm->setOsdShadow(color, blur, shift);
	}
	else if (ANode.path()==OPV_MAP_OSD_SHADOW_BLUR)
	{
		int alpha = Options::node(OPV_MAP_OSD_SHADOW_ALPHA).value().toInt();
		QColor color = Options::node(OPV_MAP_OSD_SHADOW_COLOR).value().value<QColor>();
		color.setAlpha(alpha);
		qreal blur = ANode.value().toReal();
		QPointF shift = Options::node(OPV_MAP_OSD_SHADOW_SHIFT).value().toPointF();
		FMapForm->setOsdShadow(color, blur, shift);
	}
	else if (ANode.path()==OPV_MAP_OSD_SHADOW_SHIFT)
	{
		int alpha = Options::node(OPV_MAP_OSD_SHADOW_ALPHA).value().toInt();
		QColor color = Options::node(OPV_MAP_OSD_SHADOW_COLOR).value().value<QColor>();
		color.setAlpha(alpha);
		qreal blur = Options::node(OPV_MAP_OSD_SHADOW_BLUR).value().toReal();
		QPointF shift = ANode.value().toPointF();
		FMapForm->setOsdShadow(color, blur, shift);
	}
	else if (ANode.path()==OPV_MAP_OSD_CONTR_CMARKER_COLOR)
	{
		QColor color = ANode.value().value<QColor>();
		color.setAlpha(Options::node(OPV_MAP_OSD_CONTR_CMARKER_ALPHA).value().toInt());
		FMapForm->setOsdCenterMarkerColor(color);
	}
	else if (ANode.path()==OPV_MAP_OSD_CONTR_CMARKER_ALPHA)
	{
		QColor color = Options::node(OPV_MAP_OSD_CONTR_CMARKER_COLOR).value().value<QColor>();
		color.setAlpha(ANode.value().toInt());
		FMapForm->setOsdCenterMarkerColor(color);
	}
	else if (ANode.path()==OPV_MAP_OSD_CONTR_CMARKER_VISIBLE)
		FMapForm->setOsdCenterMarkerVisible(ANode.value().toBool());
	else if (ANode.path()==OPV_MAP_ATTACH_TO_ROSTER)
	{
		FMapForm->disableHideEvent();
		if (ANode.value().toBool())
		{
			FMapForm->saveWindowGeometry();
			FMapForm->setWindowFlags(FMapForm->windowFlags()&~Qt::Window);
			if (Options::node(OPV_MAP_SHOWING).value().toBool())
			{
				if (FMainWindow->isCentralWidgetVisible() && FMainWindow->mainCentralWidget()->currentCentralPage())
					Options::node(OPV_MAP_SHOWING).setValue(false);
				else
					showMap(true, false);
			}
		}
		else
		{
			FMainWindow->mainCentralWidget()->removeCentralPage(FMapForm);
			int flags=FMapForm->windowFlags();
			FMapForm->setWindowFlags(Qt::WindowFlags(flags|Qt::Window));
			FMapForm->updateMapTitle();
			FMapForm->loadWindowGeometry();
			if (Options::node(OPV_MAP_SHOWING).value().toBool())
				showMap(true, false);
		}
		FMapForm->enableHideEvent();
	}
	else if (ANode.path()==OPV_MAP_SHOWING)
	{
		FMenuToolbar->menuAction()->blockSignals(true);
		FMenuToolbar->menuAction()->setChecked(ANode.value().toBool());
		FMenuToolbar->menuAction()->blockSignals(false);
	}
	else if(ANode.path()==OPV_MAP_PROXY) // Proxy
		FNetworkAccessManager->setProxy(FConnectionManager->proxyById(ANode.value().toString()).proxy);
	else if (ANode.path()==OPV_MAP_LOADING)
		MapTile::setDisplayLoading(ANode.value().toBool());
}

//----- 0-all, 1-toolbar, 2-menu -----
Action *Map::addMenuAction(const QString &AText, const QString &AIcon, const QString &AKeyIcon, int AMenuTypes)
{
	Action * action = new Action(AMenuTypes==2?FMenuToolbar:FMenuMap);
	action->setText(AText);
	action->setIcon(AIcon, AKeyIcon);
	if(AMenuTypes==0)
		FMenuMap->addAction(action, AG_MAPS_MENU_COMMON, false);
	if(AMenuTypes==1){
		FMenuToolbar->addAction(action, AG_MAPS_MENU_COMMON, false);
		FMenuMap->addAction(action, AG_MAPS_MENU_COMMON, false);
	}
	if(AMenuTypes==2)
		FMenuToolbar->addAction(action, AG_MAPS_MENU_COMMON, false);
	return action;
}

void Map::removeMenuAction(Action *AAction)
{
	FMenuMap->removeAction(AAction);
	FMenuToolbar->removeAction(AAction);
}

bool Map::isVisible() const
{
	return FMapForm && FMapForm->isVisible();
}

bool Map::isActive() const
{
	return FMapForm && FMapForm->isActiveWindow();
}

void Map::insertWindowShortcut(const QString &AId) const
{
	Shortcuts::insertWidgetShortcut(AId, FMapForm);
}

void Map::removeWindowShortcut(const QString &AId) const
{
	Shortcuts::removeWidgetShortcut(AId, FMapForm);
}

bool Map::showObject(int AType, const QString &AId, bool AFollow, bool AShowMap)
{
	if (GeoMap::showObject(AType, AId, AFollow))
	{
		if (AFollow)
		{
			FMyLocation->setEnabled(true);
			FFollowMyLocation=false;
			FMapForm->showMapCenter(true);
		}
		if (AShowMap)
		{
			showMap();
			centerMousePointer();
		}
	}
	return false;
}

void Map::stopFollowing()
{
	FFollowMyLocation = false;
	FMyLocation->setEnabled(true);
	FMapForm->showMapCenter(true);
	GeoMap::stopFollowing();
}

QIcon Map::getIcon(const QString name) const
{
	return FIconStorage->getIcon(name);
}

QString Map::getIconFileName(const QString name) const
{
	return FIconStorage->fileFullName(name);
}

void Map::centerMousePointer() const
{
	FMapForm->centerMousePointer();
}

void Map::setSelectionCoordinates(const MercatorCoordinates &ACoordinates)
{
	FMapForm->setSelectionCoordinates(ACoordinates);
}

void Map::setCursor(const QCursor &ACursor)
{
	FMapForm->graphicsView().setCursor(ACursor);
}

QCursor Map::cursor() const
{
	return FMapForm->graphicsView().cursor();
}

void Map::unsetCursor()
{
	FMapForm->graphicsView().unsetCursor();
}

void Map::insertOptionsDialogNode(const IOptionsDialogNode &ANode)
{
	FOptionsDialogNodes.append(ANode);
	FOptionsManager->insertOptionsDialogNode(ANode);
}

void Map::mapSceneMousePressEvent(QGraphicsSceneMouseEvent *event)
{
	FPressedPosition = event->scenePos();
	FPressedButton   = event->button();
}

void Map::mapSceneMouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	if (FPressedButton == event->button())
	{
		if (FPressedPosition == event->scenePos())   // Click!
		{
			MapScene *scene=FMapForm->mapScene();
			if (FMouseGrabber)
			{
				if (FMouseGrabber->mapMouseClicked(scene->calculateLocation(scene->translatePos(event->scenePos(), true)), event->button()))
					return;
			}

			SceneObject *activeObject=FMapForm->mapScene()->activeObject();
			if(activeObject)
				activeObject->mouseClicked(event->button());
		}
		FPressedButton=Qt::NoButton;
		FPressedPosition=QPointF();
	}
}

//---- IMapSceneListener ----
void Map::mapSceneMouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	event->lastPos();
	if (FMouseGrabber)
	{
		QPointF newPosition=FMapForm->mapScene()->translatePos(event->scenePos(), true);
		QPointF oldPosition=FMapForm->mapScene()->translatePos(event->lastScenePos(), true);
		FMousePosition=newPosition;
		MercatorCoordinates coords=FMapForm->mapScene()->calculateLocation(newPosition);
		setSelectionCoordinates(coords);
		if (FMouseGrabber->mapMouseMoved(newPosition, oldPosition, coords, event->buttons()))
		{
			event->accept();
			return;
		}
	}

	if (event->buttons() & Qt::LeftButton)
	{
		QPointF shift=event->scenePos()-event->lastScenePos();
		if ((event->scenePos()-FPressedPosition).manhattanLength() >= 4)
		{
			FMapForm->mapScene()->shiftMap(-shift);
			FMapForm->mapScene()->setIgnoreMouseMovement(false);
			event->accept();
		}
	}
}

void Map::mapSceneMouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	if (FMouseGrabber)
		if (FMouseGrabber->mapMouseDoubleClicked(FMapForm->mapScene()->calculateLocation(FMapForm->mapScene()->translatePos(event->scenePos(), true)), event->button()))
			return;

	SceneObject *activeObject=FMapForm->mapScene()->activeObject();
	if(activeObject)
		if (activeObject->mouseDoubleClicked(event->button()))
			event->accept();

	if (!event->isAccepted())
		FMapForm->mapScene()->shiftMap(event->scenePos()-FMapForm->mapScene()->center());
}

void Map::mapSceneContextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent)
{
	Menu *menu=NULL;
	FMenuObject->clear();

	if (!(FMouseGrabber &&(FMouseGrabber->mapContextMenu(FMapForm->mapScene()->calculateLocation(FMapForm->mapScene()->translatePos(contextMenuEvent->scenePos(), true)), FMenuObject))))
	{
		MapScene *mapScene=FMapForm->mapScene();
		FMenuPopupPosition=mapScene->calculateLocation(mapScene->translatePos(contextMenuEvent->scenePos(), true));
		SceneObject *activeObject=mapScene->activeObject();
		if(activeObject && activeObject->contextMenu(FMenuObject))
			menu=FMenuObject;
		if(!menu)
			menu=FMenuMap;
	}
	else
		menu=FMenuObject;

	if (menu && !menu->isEmpty())
	{
		menu->exec(QPoint((int)contextMenuEvent->screenPos().x(),(int)contextMenuEvent->screenPos().y()));
		contextMenuEvent->accept();
	}
}

void Map::mapSceneKeyPressEvent(QKeyEvent *keyEvent)
{
	Q_UNUSED(keyEvent)
}

void Map::mapSceneWheelEvent(QGraphicsSceneWheelEvent *wheelEvent)
{
	if (FMouseGrabber)
		if (FMouseGrabber->mapMouseWheelMoved(wheelEvent->scenePos(), wheelEvent->delta()))
			return;

	FMouseWheelPosition=wheelEvent->scenePos();
	if(wheelEvent->delta()>0)
		zoomIn();
	else
		zoomOut();
}

void Map::zoomIn()
{
	int zoom=Options::node(OPV_MAP_ZOOM).value().toInt();
	if (zoom<FMapForm->mapScene()->zoomMax())
	{
		zoom++;
		Options::node(OPV_MAP_ZOOM).setValue(zoom);
	}
}

void Map::zoomOut()
{
	int zoom=Options::node(OPV_MAP_ZOOM).value().toInt();
	if (zoom>FMapForm->mapScene()->zoomMin())
	{
		zoom--;
		Options::node(OPV_MAP_ZOOM).setValue(zoom);
	}
}

QMultiMap<int, IOptionsDialogWidget *> Map::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{	
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_MAP )
	{
		widgets.insertMulti(OHO_MAP_CONNECTION, FOptionsManager->newOptionsDialogHeader(tr("Connection"), AParent));
		if (FConnectionManager)
			widgets.insertMulti(OWO_MAP_CONNECTION, FConnectionManager->proxySettingsWidget(Options::node(OPV_MAP_PROXY), AParent));
		widgets.insertMulti(OHO_MAP_GENERAL, FOptionsManager->newOptionsDialogHeader(tr("General"), AParent));
		widgets.insertMulti(OWO_MAP_LOADING, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MAP_LOADING), tr("Display \"Loading\" tiles"), AParent));
		widgets.insertMulti(OWO_MAP_ATTACH_TO_ROSTER, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MAP_ATTACH_TO_ROSTER), tr("Combine map window with contact list"), AParent));
		widgets.insertMulti(OHO_MAP_ZOOM, FOptionsManager->newOptionsDialogHeader(tr("Zoom"), AParent));
		widgets.insertMulti(OWO_MAP_ZOOM_WHEEL, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MAP_ZOOM_WHEEL), tr("Mouse wheel zooming"), newWheelZoomComboBox(AParent), AParent));
		widgets.insertMulti(OWO_MAP_ZOOM_SLIDERTRACK, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MAP_ZOOM_SLIDERTRACK), tr("Update zoom during slider movement"), AParent));
		widgets.insertMulti(OHO_MAP_OSD, FOptionsManager->newOptionsDialogHeader(tr("On-screen display"), AParent));
		widgets.insertMulti(OWO_MAP_OSD, new MapOptions(AParent));		
		widgets.insertMulti(OHO_MAP_SOURCES, FOptionsManager->newOptionsDialogHeader(tr("Setup map sources"), AParent));
		if (!FOptionsDialogNodes.isEmpty())
		{
			MapSources *mapSources = new MapSources(FOptionsManager);
			for (QList<IOptionsDialogNode>::ConstIterator it=FOptionsDialogNodes.constBegin(); it!=FOptionsDialogNodes.constEnd(); it++)
				mapSources->addMapSource(*it);
			widgets.insertMulti(OWO_MAP_SOURCES, mapSources);
		}
	}
	return widgets;
}

QComboBox *Map::newWheelZoomComboBox(QWidget *AParent)
{
	QComboBox *comboBox = new QComboBox(AParent);
	comboBox->addItem(tr("Relative to map center"), RelativeToMapCenter);
	comboBox->addItem(tr("Relative to mouse position"), RelativeToMousePointerPosition);
	comboBox->addItem(tr("Follow mouse pointer"), FollowMousePointer);
	return comboBox;
}

/******************* Protected slots *************************/
void Map::onSliderValueChanged(int APosition)
{
	qDebug() << "Map::onSliderValueChanged(" << APosition << ")";
	Options::node(OPV_MAP_ZOOM).setValue(APosition);
}

void Map::onMapCenterChanged(double ALatitude, double ALongitude, bool AManual)
{
	Options::node(OPV_MAP_COORDS).setValue(QPointF(ALongitude, ALatitude));
	if (FMouseGrabber)
	{
		MercatorCoordinates coords=FMapForm->mapScene()->calculateLocation(FMousePosition);
		setSelectionCoordinates(coords);

		if (!AManual)
			FMouseGrabber->mapMouseMoved(FMousePosition, FMousePosition, coords, Qt::NoButton); // Notify grabber about coordinates change
	}
	if (AManual)
		stopFollowing();   // Stop following
}

void Map::showMap(bool AShow, bool AActivate)
{
	qDebug() << "Map::showMap(" << AShow << "," << AActivate << ")";
	if (AShow)
	{
		if (Options::node(OPV_MAP_ATTACH_TO_ROSTER).value().toBool())
		{
			qDebug() << "Appending central page...";
			FMainWindow->mainCentralWidget()->appendCentralPage(FMapForm);
			if (AActivate)
			{
				qDebug() << "Showing...";
				FMapForm->showWindow();
			}
		}
		else
		{
			qDebug() << "Showing...";
			FMapForm->showWindow();
		}
	}
	else
	{
		qDebug() << "Hiding..";
		FMapForm->hideWindow();
	}
	qDebug() << "Done!";
}

void Map::onShortcutActivated(const QString &AId, QWidget *AWidget)
{
	Q_UNUSED(AWidget)
	if (AId==SCT_MAP_ZOOM_IN)
		zoomIn();
	else if (AId==SCT_MAP_ZOOM_OUT)
		zoomOut();
}

void Map::onNewPositionAvailable(const GeolocElement &APosition)
{
	if (APosition.isValid())
	{
		FMapForm->setOwnLocation(APosition.propertyAsString(GeolocElement::Lat), APosition.propertyAsString(GeolocElement::Lon), APosition.reliability());
		if (FFollowMyLocation)
		{
			FMapForm->mapScene()->setMapCenter(APosition.lat(), APosition.lon());
			FMapForm->showMapCenter(false);
		}
	}
	else
	{
		FMapForm->removeOwnLocation();
		FMapForm->showMapCenter(true);
	}
}

void Map::showOptions()
{
	FOptionsManager->showOptionsDialog(OPN_MAP);
}

void Map::showMyLocation()
{
	FFollowMyLocation = true;
	if (FPositioning)
	{
		GeolocElement position = FPositioning->currentPosition();
		if (position.isValid())
		{
			FMapForm->mapScene()->setMapCenter(position.lat(), position.lon());
			FMapForm->showMapCenter(false);
		}
	}
	FMyLocation->setDisabled(true);
	if (FFollowedObject)
		FFollowedObject = NULL;
}

void Map::onCentralWidgetVisibleChanged(bool AVisible)
{
	if (AVisible && Options::node(OPV_MAP_ATTACH_TO_ROSTER).value().toBool() && Options::node(OPV_MAP_SHOWING).value().toBool())
		FMainWindow->mainCentralWidget()->appendCentralPage(FMapForm);
}

void Map::onCurrentCentralPageChanged(IMainCentralPage *APage)
{
	if (Options::node(OPV_MAP_ATTACH_TO_ROSTER).value().toBool() && APage != FMapForm)
		Options::node(OPV_MAP_SHOWING).setValue(false);
}

/***************************************************************/
QRect Map::sceneRect() const
{
	return FMapForm->sceneRect();
}

bool Map::setMouseGrabber(IMapMouseGrabber *AMouseGrabber, bool AShowCoordinates)
{
	if (FMouseGrabber && AMouseGrabber!=FMouseGrabber)
		return false;
	FMouseGrabber=AMouseGrabber;
	FMapForm->showSelectionBox(AShowCoordinates);
	return true;
}

bool Map::releaseMouseGrabber(IMapMouseGrabber *AMouseGrabber)
{
	if (AMouseGrabber && AMouseGrabber==FMouseGrabber)
	{
		FMouseGrabber=NULL;
		FMapForm->showSelectionBox(false);
		return true;
	}
	return false;
}

bool Map::isGrabbingMouseEvents() const
{
	return FMouseGrabber!=NULL;
}

void Map::updateScene()
{
	FMapForm->mapScene()->instance()->update();
}

const MercatorCoordinates & Map::mapCenter() const
{
	return FMapForm->mapScene()->mapCenter();
}

IMapSource *Map::getMapSource(int AIndex)
{
	if (AIndex==-1)
		AIndex = findMapSource();
	if (AIndex>0 && AIndex<=FMapSources.size())
		return FMapSources.at(AIndex-1);
	return NULL;
}

int Map::findMapSource(const QUuid &AUuid)
{
	QUuid uuid(AUuid);
	if (uuid.isNull())
		uuid = Options::node(OPV_MAP_SOURCE).value().toString();

	int i=1;
	for (QList<IMapSource *>::ConstIterator it=FMapSources.constBegin(); it!=FMapSources.constEnd(); it++, i++)
		if (qobject_cast<IPlugin *>((*it)->instance())->pluginUuid() == uuid)
			return i;
	return 0;
}

//---------------- *** >>> Map >>> *** -----------------
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_map, Map)
#endif
