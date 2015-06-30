#include <QtGui>
#include <MapObject>

#include <definitions/resources.h>
#include <definitions/rosterindexroles.h>
#include <definitions/rosterindexkinds.h>
#include <definitions/actiongroups.h>
#include <definitions/menuicons.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>

#include <utils/action.h>

#include "streetview.h"
#include "streetviewgraphicsitems.h"
#include "streetviewform.h"

#define SDR_ID      1
#define SDR_ACTION  2
#define ACTION1     1
#define ACTION2     2

#define ADR_CONTACT_JID      Action::DR_Parametr1
#define ADR_STREAM_JID      Action::DR_StreamJid

StreetView::StreetView(QObject *parent) :
    QObject(parent),
	FMap(NULL),
	FGeoMap(NULL),
    FPoi(NULL),
    FGeoloc(NULL),
//    FConnectionManager(NULL),
    FOptionsManager(NULL),
    FRostersViewPlugin(NULL),
    FStreetViewForm(NULL),
    FDropped(false)
{}

void StreetView::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Street View", "Plugin name");
    APluginInfo->description = tr("Allows to view street-level imagery");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(MAP_UUID);
}

bool StreetView::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    IPlugin *plugin = APluginManager->pluginInterface("IMap").value(0,NULL);
    if (plugin)
		FGeoMap = (FMap = qobject_cast<IMap *>(plugin->instance()))->geoMap();
    else
        return false;

    plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

//    plugin = APluginManager->pluginInterface("IConnectionManager").value(0,NULL);
//    if (plugin)
//        FConnectionManager = qobject_cast<IConnectionManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IPoi").value(0,NULL);
    if (plugin)
        FPoi = qobject_cast<IPoi *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IGeoloc").value(0,NULL);
	if (plugin && (FGeoloc = qobject_cast<IGeoloc *>(plugin->instance())))
		if ((plugin = APluginManager->pluginInterface("IRostersViewPlugin").value(0,NULL)) &&
			(FRostersViewPlugin = qobject_cast<IRostersViewPlugin *>(plugin->instance())))
			connect(FRostersViewPlugin->rostersView()->instance(),
					SIGNAL(indexContextMenu(QList<IRosterIndex *>, quint32, Menu *)),
					SLOT(onRosterIndexContextMenu(QList<IRosterIndex *>, quint32, Menu *)));

    QList<IPlugin *> plugins = APluginManager->pluginInterface("IStreetViewProvider");
    if (plugins.isEmpty())
        return false;       // No providers found

    for (QList<IPlugin *>::const_iterator it=plugins.constBegin(); it!=plugins.constEnd(); it++)
        FStreetViewProviders.insert((*it)->pluginUuid(), qobject_cast<IStreetViewProvider *>((*it)->instance()));

    AInitOrder = 200;
    return true;
}

bool StreetView::initObjects()
{
	if (FGeoMap)
    {
		FGeoMap->setObjectHandler(MOT_STREETVIEW, this);
		FGeoMap->registerDataType(MDR_STREETVIEW_MAN, MOT_STREETVIEW, 150, MOP_TOP, MOP_TOP);
		FGeoMap->registerDataType(MDR_STREETVIEW_POINT, MOT_STREETVIEW, 140, MOP_CENTER, MOP_CENTER);
		FGeoMap->registerDataType(MDR_STREETVIEW_MARKER, MOT_STREETVIEW, 130, MOP_BOTTOM, MOP_NONE);
		FGeoMap->addDataHolder(MOT_STREETVIEW, this);

		Action *action = FMap->addMenuAction(tr("Street view", "Menu option"), QString(RSR_STORAGE_MENUICONS),QString(MNI_STREETVIEW), 0);
        action->setCheckable(false);
        //action->setShortcutId(SCT_MAP_MAGNIFIER_TOGGLE);
        connect(action, SIGNAL(triggered(bool)), SLOT(onStreetViewTriggered()));

        if(!FStreetViewForm)
        {
			FStreetViewForm = new StreetViewForm(FStreetViewProviders, FMap, this /*,FConnectionManager*/);
            connect(this, SIGNAL(destroyed()), FStreetViewForm, SLOT(deleteLater()));
        }
    }

    if (FPoi)
    {
        Action *action  = FPoi->addMenuAction(tr("Street view", "Menu option"),QString(RSR_STORAGE_MENUICONS),QString(MNI_STREETVIEW));
        action->setCheckable(false);
        connect(action,SIGNAL(triggered(bool)), SLOT(onStreetViewPoiTriggered()));
    }
    return true;
}

bool StreetView::initSettings()
{
	Options::setDefaultValue(OPV_MAP_STREETVIEW, true);
    Options::setDefaultValue(OPV_MAP_STREETVIEW_PROVIDER, "{3322a1aa-3fa4-40b8-ab68-af4da153a7e7}");    
    Options::setDefaultValue(OPV_MAP_STREETVIEW_FOV, 90);
	Options::setDefaultValue(OPV_MAP_STREETVIEW_IMAGEDIRECTORY, QDir::homePath());
//    if (FOptionsManager)
//        FOptionsManager->insertOptionsHolder(this);
    return true;
}

//! --IMapObjectDataHolder--
QGraphicsItem *StreetView::mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement)
{    
    Q_UNUSED(ASceneObject)

    if (!ACurrentElement)
        switch (ARole)
        {
            case MDR_STREETVIEW_MAN:
            {
                QIcon icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_STREETVIEWMAN);
                if (!icon.isNull())
                    ACurrentElement = new QGraphicsPixmapItem(icon.pixmap(icon.availableSizes().first()));
                break;
            }

            case MDR_STREETVIEW_POINT:
            {
                ACurrentElement = new StreetViewPoint(this);
                break;
            }

            case MDR_STREETVIEW_MARKER:
            {
                ACurrentElement = new StreetViewMarker();
                break;
            }

            default:
                break;
        }

    return ACurrentElement;
}

void StreetView::onMapObjectInserted(int AType, const QString &AId)
{
    if (AType==MOT_STREETVIEW)
        emit mapDataChanged(AType, AId, MDR_ALL); //MDR_STREETVIEW_MAN);
}

void StreetView::onMapObjectRemoved(int AType, const QString &AId)
{
    Q_UNUSED(AType)
    Q_UNUSED(AId)
}

//! --IMapObjectHandler---
void StreetView::mouseHoverEnter(SceneObject *ASceneObject)
{
	ASceneObject->setFull(true);
}

void StreetView::mouseHoverLeave(SceneObject *ASceneObject)
{
    ASceneObject->setFull(false);
}

bool StreetView::contextMenu(SceneObject *ASceneObject, QMenu *AMenu)
{
    Q_UNUSED(ASceneObject)
    Q_UNUSED(AMenu)

    if (FDropped)
    {
        FDropped = false;
        return true;
    }
    return false;
}

bool StreetView::mouseDoubleClicked(SceneObject *ASceneObject, Qt::MouseButton AButton)
{
    Q_UNUSED(ASceneObject)
    Q_UNUSED(AButton)
    if (!FStreetViewForm->isActiveWindow())
        FStreetViewForm->activateWindow();
    return true;
}

void StreetView::objectUpdated(SceneObject *ASceneObject, int ARole)
{
    emit mapDataChanged(MOT_STREETVIEW, ASceneObject->mapObject()->id(), ARole);
}

QString StreetView::toolTipText(const MapObject *AMapObject, const QHelpEvent *AEvent) const
{
    Q_UNUSED(AMapObject)
    Q_UNUSED(AEvent)
    return QString();
}

//!-----------------------------
void StreetView::onStreetViewTriggered()
{
	if (FGeoMap)
    {
		MercatorCoordinates coords = FMap->menuPopupPosition();
        startStreetView(coords.latitude(), coords.longitude());
    }
}

void StreetView::onStreetViewContactTriggered()
{
    Jid jid(qobject_cast<Action *>(sender())->data(ADR_CONTACT_JID).toString());
	GeolocElement geoloc = FGeoloc->getGeoloc(jid);
	startStreetView(geoloc.lat(), geoloc.lon());
}

void StreetView::onStreetViewPoiTriggered()
{
	if (FGeoMap)
    {
        Action *action=qobject_cast<Action *>(sender());
        GeolocElement poi=FPoi->getPoi(action->data(IPoi::ADR_ID).toString());
		startStreetView(poi.lat(), poi.lon());
    }
}

void StreetView::onRosterIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
    if (ALabelId == AdvancedDelegateItem::DisplayId || ALabelId == FGeoloc->rosterLabelId())
    {
        QStringList list;

        for(QList<IRosterIndex *>::const_iterator it=AIndexes.constBegin(); it!=AIndexes.constEnd(); it++)
            if((*it)->kind() == RIK_MY_RESOURCE || (*it)->kind() == RIK_CONTACT || (*it)->kind() == RIK_AGENT || (*it)->kind() == RIK_STREAM_ROOT)
            {
                QStringList resources=(*it)->data(RDR_RESOURCES).toStringList();
                if (resources.isEmpty())
                    resources.append((*it)->data(RDR_FULL_JID).toString());
                for (QStringList::const_iterator rit=resources.constBegin(); rit!=resources.constEnd(); rit++)
                    if(!FGeoloc->getGeoloc(*rit).isEmpty())
                        list.append(*rit);
            }

        Menu *menu;
        if (list.size()>1)
        {
            menu = new Menu(AMenu);
            menu->menuAction()->setText(tr("Street view"));
            menu->setIcon(RSR_STORAGE_MENUICONS, MNI_STREETVIEW);
            AMenu->addAction(menu->menuAction(), AG_RVCM_GEOLOC, true);
        }
        else
            menu=AMenu;

        for (QStringList::const_iterator rit=list.constBegin(); rit!=list.constEnd(); rit++)
        {
            Action *action = new Action(menu);
            if (menu==AMenu)
                action->setText(tr("Street view"));
            else
                action->setText(*rit);

            action->setData(ADR_CONTACT_JID, *rit);
            action->setIcon(RSR_STORAGE_MENUICONS, MNI_STREETVIEW);

            menu->addAction(action, AG_RVCM_GEOLOC, true);
            connect(action,SIGNAL(triggered()),SLOT(onStreetViewContactTriggered()));
        }
    }
}

void StreetView::startStreetView(double ALat, double ALng)
{
    showStreetViewMarker(ALat, ALng);

    if (!FStreetViewForm->isVisible())
        FStreetViewForm->show();
    else if (!FStreetViewForm->isActiveWindow())
        FStreetViewForm->activateWindow();
    FStreetViewForm->setLocation(ALat, ALng);
    FStreetViewForm->startView();
}

void StreetView::showStreetViewMarker(double ALat, double ALng)
{
	FGeoMap->addObject(MOT_STREETVIEW, "StreetView", MercatorCoordinates(ALat, ALng));
}

void StreetView::hideStreetViewMarker()
{
	FGeoMap->removeObject(MOT_STREETVIEW, "StreetView");
}

void StreetView::dropped()
{
	SceneObject *object = FGeoMap->getSceneObject(MOT_STREETVIEW, "StreetView");
    if (object)
    {
        startStreetView(object->mapObject()->location().latitude(), object->mapObject()->location().longitude());
        FDropped = true;
    }
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_streetview, StreetView)
#endif
