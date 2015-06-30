#include <QDebug>
#include <QApplication>
#include <QtGui>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>
#include <MapObject>

#include "weather.h"

#define SDR_ID      1
#define SDR_ACTION  2
#define ACTION1     1
#define ACTION2     2
#define ACTION3     3
#define ACTION4     4

Weather::Weather(QObject *parent) :
         QObject(parent)
		,FMap(NULL)
		,FGeoMap(NULL)
        ,FPoi(NULL)
        ,FOptionsManager(NULL)
        //FWeatherForm(NULL)
{
}

void Weather::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Weather viewer");
    APluginInfo->description = tr("Provides data about the weather");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
}

bool Weather::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    IPlugin *plugin = APluginManager->pluginInterface("IMap").value(0,NULL);
    if (plugin)
		FGeoMap = (FMap = qobject_cast<IMap *>(plugin->instance()))->geoMap();
    else
        return false;

    plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IPoi").value(0,NULL);
    if (plugin)
        FPoi = qobject_cast<IPoi *>(plugin->instance());

    QList<IPlugin *> plugins = APluginManager->pluginInterface("IWeatherProvider");
    if (plugins.isEmpty())
        return false;       // No providers found

    for (QList<IPlugin *>::const_iterator it=plugins.constBegin(); it!=plugins.constEnd(); it++)
        FWeatherProviders.insert((*it)->pluginUuid(), qobject_cast<IWeatherProvider *>((*it)->instance()));

    AInitOrder = 200;
    return true;
}

bool Weather::initObjects()
{
	if (FGeoMap)
    {
		FGeoMap->setObjectHandler(MOT_WEATHER, this);
		FGeoMap->registerDataType(MDR_WEATHER_ICON, MOT_WEATHER, 150, MOP_CENTER, MOP_CENTER);
		FGeoMap->addDataHolder(MOT_WEATHER, this);

		FGeoMap->setObjectHandler(MOT_WEATHEROBJECT, this);
		FGeoMap->registerDataType(MDR_WEATHER_OBJECT, MOT_WEATHEROBJECT, 150, MOP_TOP, MOP_TOP);
		FGeoMap->addDataHolder(MOT_WEATHEROBJECT, this);

		Action *action = FMap->addMenuAction(tr("View Weather"), QString(RSR_STORAGE_MENUICONS),QString(MNI_MAP_WEATHER), 0);
        action->setCheckable(false);
        //action->setUserData()
        //action->setShortcutId(SCT_MAP_MAGNIFIER_TOGGLE);
        connect(action, SIGNAL(triggered(bool)), SLOT(onPlaceViewTriggered()));
/*
        if(!FPlaceViewForm)
        {
            FPlaceViewForm = new PlaceViewForm(FPlaceViewProviders, FMap,this);
            //connect(FPlaceViewForm,SIGNAL(gotoView(double,double)),this,SLOT(onGotoView(double,double)));
            connect(FPlaceViewForm,SIGNAL(poisReady()),this,SLOT(onPoisReady()));
            connect(FPlaceViewForm,SIGNAL(poisDelete()),this,SLOT(onPoisDelete()));
            connect(this, SIGNAL(destroyed()), FPlaceViewForm, SLOT(deleteLater()));
        }
*/
    }

    if (FPoi)
    {
        Action *action  = FPoi->addMenuAction(tr("View Weather"),QString(RSR_STORAGE_MENUICONS),QString(MNI_MAP_WEATHER));
        action->setCheckable(false);
        //action->setUserData()
        connect(action,SIGNAL(triggered(bool)), SLOT(onPlaceViewPoiTriggered()));
    }

    return true;
}

bool Weather::initSettings()
{
//OPV_MAP_WEATHER
//    Options::setDefaultValue(OPV_MAP_WEATHER_PROVIDER, "{515bb127-0d6a-49a3-9ca5-2b3fc486cfae}");
    Options::setDefaultValue(OPV_MAP_WEATHER,true);


//    Options::setDefaultValue(OPV_MAP_WEATHER_PROVIDER, "{}");
//    if (FOptionsManager)
//        FOptionsManager->insertOptionsHolder(this);
    return true;
}

/*
QMultiMap<int, IOptionsWidget *> Weather::optionsWidgets(const QString &ANodeId, QWidget *AParent)
{
	Q_UNUSED(ANodeId)
	Q_UNUSED(AParent)

    QMultiMap<int, IOptionsWidget *> widgets;
//    if (FOptionsManager && ANodeId == OPN_ROSTER)
//        widgets.insertMulti(OWO_WEATHER, FOptionsManager->optionsNodeWidget(Options::node(OPV_MAP_STREETVIEW),tr("Map Street Viewer"),AParent));
//        widgets.insertMulti(OWO_WEATHER, new MapSearchOptions(AParent));
    return widgets;
}
*/
bool Weather::contextMenu(const QString &AId, QMenu *AMenu)
{
    Action *action;
    if(AId==MNI_MAP_WEATHER)//MNI_MAP_WEATHER
    {
        QStyle *style = QApplication::style();
        action=new Action(AMenu);
        action->setText(tr("View"));
        action->setIcon(RSR_STORAGE_MENUICONS, MNI_MAP_WEATHER);//MNI_MAP_WEATHER
        action->setData(SDR_ID, AId);
        action->setData(SDR_ACTION, ACTION1);
        connect(action, SIGNAL(triggered()), SLOT(onViewActionTriggered()));
        AMenu->addAction(action);

        action=new Action(AMenu);
        action->setText(tr("Close"));
        action->setIcon(style->standardIcon(QStyle::SP_TitleBarCloseButton));
        action->setData(SDR_ID, AId);
        action->setData(SDR_ACTION, ACTION2);
        connect(action, SIGNAL(triggered()), SLOT(onViewActionTriggered()));
        AMenu->addAction(action);
    }
    else{

    }

    return true;
}

//IMapObjectDataHolder
QGraphicsItem *Weather::mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement)
{
    Q_UNUSED(ASceneObject)
    Q_UNUSED(ARole)

    return ACurrentElement;
}

//IMapObjectDataHolder
void Weather::onMapObjectInserted(int AType, const QString &AId)
{
    Q_UNUSED(AType)
    Q_UNUSED(AId)
}

void Weather::onMapObjectRemoved(int AType, const QString &AId)
{
    Q_UNUSED(AType)
    Q_UNUSED(AId)
}

void Weather::onMapObjectShowed(int AType, const QString &AId)
{
    Q_UNUSED(AType)
    Q_UNUSED(AId)
}
//!----------
void Weather::onPlaceViewTriggered()
{
	if (FMap)
    {
		MercatorCoordinates coords = FMap->menuPopupPosition();
        double lat=coords.latitude();
        double lng=coords.longitude();
        statrViewer(lat,lng);
    }
}

void Weather::onPlaceViewPoiTriggered()
{
	if (FGeoMap)
    {
        Action *action=qobject_cast<Action *>(sender());
        GeolocElement poi=FPoi->getPoi(action->data(IPoi::ADR_ID).toString());
        double lat=poi.lat();
        double lng=poi.lon();
        statrViewer(lat,lng);
    }
}

void Weather::onViewActionTriggered()
{

}

//IMapSceneObjectHandler
void Weather::mouseHoverEnter(SceneObject *ASceneObject)
{
    Q_UNUSED(ASceneObject)
}

void Weather::mouseHoverLeave(SceneObject *ASceneObject)
{
    Q_UNUSED(ASceneObject)
}

bool Weather::mouseDoubleClicked(SceneObject *ASceneObject, Qt::MouseButton AButton)
{
    Q_UNUSED(ASceneObject)
    Q_UNUSED(AButton)

    return false;
}

bool Weather::contextMenu(SceneObject *ASceneObject, QMenu *AMenu)
{
    Q_UNUSED(ASceneObject)
    Q_UNUSED(AMenu)

    return false;
}

void Weather::objectUpdated(SceneObject *ASceneObject, int ARole)
{
    Q_UNUSED(ASceneObject)
    Q_UNUSED(ARole)
}

QString Weather::toolTipText(const MapObject *AMapObject, const QHelpEvent *AEvent) const
{
	Q_UNUSED(AMapObject)
	Q_UNUSED(AEvent)

    QString htmlPoi;
    return htmlPoi;
}

void Weather::showCenterView(int AType,QString AIdIcon,int ARole,double ALat,double ALng)
{
	FGeoMap->addObject(AType, AIdIcon, MercatorCoordinates(ALat, ALng));
    emit mapDataChanged(AType,AIdIcon,ARole);
}

void Weather::statrViewer(double ALat, double ALng)
{
    showCenterView(MOT_PLACEVIEW, MNI_MAP_PLACEVIEW, MDR_PLACEVIEW_ICON,ALat,ALng);
/*    if (!FPlaceViewForm->isVisible())
        FPlaceViewForm->show();
    else if (!FPlaceViewForm->isActiveWindow())
        FPlaceViewForm->activateWindow();
    FPlaceViewForm->formDialogActivate(ALat,ALng);
*/
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_weather, Weather)
#endif
