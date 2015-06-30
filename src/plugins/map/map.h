#ifndef MAP_H
#define MAP_H

#include <QNetworkAccessManager>
#include <GeoMap>
#include <MapScene>
#include <QComboBox>

#include <interfaces/imap.h>
#include <interfaces/imainwindow.h>
#include <interfaces/itraymanager.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipositioning.h>
#include <interfaces/iconnectionmanager.h>

#include <utils/options.h>
#include <utils/menu.h>

#include "mapsources.h"

class MapForm;
class Map;
class MapObject;
class MapObjectDataSource;

class Map:  public GeoMap,
			public IPlugin,
			public IMap,
			public IOptionsDialogHolder,
			public MapSceneListener
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IMap IOptionsDialogHolder MapSceneListener)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMap")
#endif
public:
	Map();
	~Map();

	//IPlugin
	QObject *instance() { return this; }
	QUuid pluginUuid() const { return MAP_UUID; }
	void pluginInfo(IPluginInfo *APluginInfo);
	bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	bool initObjects();
	bool initSettings();
	bool startPlugin(){return true;}

	// IMap
	GeoMap	*geoMap() {return this;}
	const QList<IMapSource *> &mapSources() const {return FMapSources;}
	IMapSource *getMapSource(int AIndex = -1);

	// Show (and optionally follow) scene object
	bool showObject(int AType, const QString &AId, bool AFollow=false, bool AShowMap=true);
	// Stop following map object
	void stopFollowing();

	// Get window icon
	QIcon   getIcon(const QString name) const;
	// Get window icon filename
	QString getIconFileName(const QString name) const;

	// Add action to map and toolbar menus
	Action  *addMenuAction(const QString &AText, const QString &AIcon, const QString &AKeyIcon, int AMenuTypes);
	void    removeMenuAction(Action *AAction);
	bool    isVisible() const;
	bool    isActive() const;
	void    insertWindowShortcut(const QString &AId) const;
	void    removeWindowShortcut(const QString &AId) const;

	const MercatorCoordinates &menuPopupPosition() const {return FMenuPopupPosition; }
	const MercatorCoordinates &mapCenter() const;

	QRect sceneRect() const;

	bool setMouseGrabber(IMapMouseGrabber *AMouseGrabber, bool AShowCoordinates=false);
	bool releaseMouseGrabber(IMapMouseGrabber *AMouseGrabber);
	bool isGrabbingMouseEvents() const;

	void	setCursor(const QCursor &ACursor);
	QCursor cursor() const;
	void	unsetCursor();
	void	insertOptionsDialogNode(const IOptionsDialogNode &ANode);

	// IMapSceneListener
	void mapSceneMousePressEvent(QGraphicsSceneMouseEvent   * event);
	void mapSceneMouseReleaseEvent(QGraphicsSceneMouseEvent * event);
	void mapSceneMouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mapSceneMouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
	void mapSceneKeyPressEvent(QKeyEvent * keyEvent);
	void mapSceneWheelEvent(QGraphicsSceneWheelEvent * wheelEvent);
	void mapSceneContextMenuEvent(QGraphicsSceneContextMenuEvent * contextMenuEvent);

	// IOptionsHolder
	QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

	void zoomIn();
	void zoomOut();

public slots:
	void onSliderValueChanged(int position);
	void updateScene();
	void showMap(bool AShow=true, bool AActivate=true);

protected:
	void        claculateBounding();
	int         findMapSource(const QUuid &AUuid=QString());
	void        centerMousePointer() const;
	void        setSelectionCoordinates(const MercatorCoordinates &ACoordinates);
	void        fillMenu();
	void        fillSources();
	QComboBox   *newWheelZoomComboBox(QWidget *AParent);

protected slots:
	void onOptionsOpened();
	void onOptionsChanged(const OptionsNode &ANode);
	void onOptionsClosed();
	void onShutdownStarted();

	void onMapCenterChanged(double ALatitude, double ALongitude, bool AManual);
	void onShortcutActivated(const QString &AId, QWidget *AWidget);
	void onNewPositionAvailable(const GeolocElement &APosition);

	void showOptions();
	void showMyLocation();

	void onCentralWidgetVisibleChanged(bool AVisible);
	void onCurrentCentralPageChanged(IMainCentralPage *APage);

signals:
	//IMap
	void sceneRectChanged(const QRectF &ARect);

signals:
	void locationSelected();
	void zoomChanged(int);

private:
	enum MouseWheelZooming
	{
		RelativeToMousePointerPosition,
		RelativeToMapCenter,
		FollowMousePointer
	};

	QList<IMapSource *>	FMapSources;
	IMapSource			*FMapSource;
	IOptionsManager		*FOptionsManager;
	IConnectionManager	*FConnectionManager;
	QNetworkAccessManager *FNetworkAccessManager;
	IMainWindowPlugin	*FMainWindowPlugin;
	IMainWindow			*FMainWindow;
	IPositioning		*FPositioning;
	IconStorage			*FIconStorage;
	MapForm				*FMapForm;
	Menu				*FMenuObject;
	Menu				*FMenuMap;
	Menu				*FMenuToolbar;
	MapObject			*FFollowedObject;
	QActionGroup		*FSourceGroup;
	QStackedWidget		*FMapsWidget;
	IMapMouseGrabber	*FMouseGrabber;
	Action				*FMyLocation;
	MapSources			*FMapSourcesWidget;
	bool				FFollowMyLocation;

	QPointF				FMousePosition;
	// Button processing
	QPointF				FPressedPosition;
	Qt::MouseButton		FPressedButton;
	// Wheel processing
	QPointF				FMouseWheelPosition;

	MercatorCoordinates	FMenuPopupPosition;

	QHash<int, MapObjectHandlerRecord *> FObjectHandlers;
};

#endif // MAP_H
