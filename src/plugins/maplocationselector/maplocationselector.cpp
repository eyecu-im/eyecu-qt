#include "maplocationselector.h"
#include <QGraphicsLineItem>

#define MDR_LOCATION_SELECTOR_TOP           100
#define MDR_LOCATION_SELECTOR_BOTTOM        101
#define MDR_LOCATION_SELECTOR_LEFT          102
#define MDR_LOCATION_SELECTOR_RIGHT         103
MapLocationSelector::MapLocationSelector(QObject *parent) :
	QObject(parent), FMap(NULL), FGeoMap(NULL), FLocationSelector(NULL)
{
}

//IPlugin
void MapLocationSelector::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Map location selector");
    APluginInfo->description = tr("Allows other plugins to select a location on the Map");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(MAP_UUID);
}

bool MapLocationSelector::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder)

    IPlugin *plugin = APluginManager->pluginInterface("IMap").value(0,NULL);
    if (plugin)
		FGeoMap = (FMap = qobject_cast<IMap *>(plugin->instance()))->geoMap();
    else
        return false;
    return true;
}

bool MapLocationSelector::initObjects()
{
	FGeoMap->setObjectHandler(MOT_LOCATION_SELECTOR, this);
	FGeoMap->registerDataType(MDR_LOCATION_SELECTOR_TOP, MOT_LOCATION_SELECTOR, 100, MOP_NONE, MOP_TOP);
	FGeoMap->registerDataType(MDR_LOCATION_SELECTOR_BOTTOM, MOT_LOCATION_SELECTOR, 100, MOP_NONE, MOP_BOTTOM);
	FGeoMap->registerDataType(MDR_LOCATION_SELECTOR_LEFT, MOT_LOCATION_SELECTOR, 100, MOP_NONE, MOP_LEFT);
	FGeoMap->registerDataType(MDR_LOCATION_SELECTOR_RIGHT, MOT_LOCATION_SELECTOR, 100, MOP_NONE, MOP_RIGHT);
	FGeoMap->addDataHolder(MOT_LOCATION_SELECTOR, this);
    return true;
}

bool MapLocationSelector::initSettings()
{
    return true;
}

//IMapLocationSelector
bool MapLocationSelector::selectLocation(ILocationSelector *ALocationSelector)
{
    if (ALocationSelector)
    {
        if (FLocationSelector==ALocationSelector)
            return true;
        else
            if (!FLocationSelector)
				if (FMap->setMouseGrabber(this, true))
                {
					FMap->setCursor(Qt::CrossCursor);
					FGeoMap->addObject(MOT_LOCATION_SELECTOR, "LocationSelector", 0, 0);
					FMap->showMap();
                    connect(this, SIGNAL(locationSelected()), ALocationSelector->instance(), SLOT(onLocationSelected()));
                    connect(this, SIGNAL(locationSelectionCancelled()), ALocationSelector->instance(), SLOT(onLocationSelectionCancelled()));
                    FLocationSelector=ALocationSelector;
                    return true;
                }
    }
    return false;
}

bool MapLocationSelector::finishSelectLocation(ILocationSelector *ALocationSelector, bool ACancel)
{
    if (ALocationSelector)
        if (FLocationSelector==ALocationSelector)
			if (FMap->releaseMouseGrabber(this))
            {
				FMap->unsetCursor();
				FGeoMap->removeObject(MOT_LOCATION_SELECTOR, "LocationSelector");
                if (ACancel)
                    emit locationSelectionCancelled();
                else
                    emit locationSelected();
                disconnect(this, SIGNAL(locationSelected()), FLocationSelector->instance(), SLOT(onLocationSelected()));
                disconnect(this, SIGNAL(locationSelectionCancelled()), FLocationSelector->instance(), SLOT(onLocationSelectionCancelled()));
                FLocationSelector=NULL;
                return true;
            }
    return false;
}

const MercatorCoordinates &MapLocationSelector::selectedLocation() const
{
    return FSelectedLocation;
}

bool MapLocationSelector::mapMouseMoved(QPointF ANewPosition, QPointF AOldPosition, MercatorCoordinates ACoordinates, Qt::MouseButtons AMouseButtons)
{
    Q_UNUSED(AOldPosition)
    Q_UNUSED(AMouseButtons)
	Q_UNUSED(ACoordinates)

	MapObject *object=FGeoMap->addObject(MOT_LOCATION_SELECTOR, "LocationSelector", ANewPosition);
    if (object)
    {
		MapScene *scene=FGeoMap->getScene();
		QRect rect=FMap->sceneRect();
        qgraphicsitem_cast<QGraphicsLineItem *>(scene->sceneObject(object)->getElementByRole(MDR_LOCATION_SELECTOR_TOP))->setLine(0, -1, 0, rect.top()-ANewPosition.y());
        qgraphicsitem_cast<QGraphicsLineItem *>(scene->sceneObject(object)->getElementByRole(MDR_LOCATION_SELECTOR_BOTTOM))->setLine(0, 1, 0, rect.bottom()-ANewPosition.y());
        qgraphicsitem_cast<QGraphicsLineItem *>(scene->sceneObject(object)->getElementByRole(MDR_LOCATION_SELECTOR_LEFT))->setLine(-1, 0, rect.left()-ANewPosition.x(), 0);
        qgraphicsitem_cast<QGraphicsLineItem *>(scene->sceneObject(object)->getElementByRole(MDR_LOCATION_SELECTOR_RIGHT))->setLine(1, 0, rect.right()-ANewPosition.x(), 0);
    }
    return false;
}

bool MapLocationSelector::mapMouseClicked(MercatorCoordinates ACoordinates, Qt::MouseButton button)
{
    if (button==Qt::LeftButton)
    {
        FSelectedLocation=ACoordinates;
        finishSelectLocation(FLocationSelector, false);
        return true;
    }
    return false;
}

bool MapLocationSelector::mapMouseDoubleClicked(MercatorCoordinates ACoordinates, Qt::MouseButton AButton)
{
    Q_UNUSED(ACoordinates)
    Q_UNUSED(AButton)
    return false;
}

bool MapLocationSelector::mapMouseWheelMoved(const QPointF &AScenePosition, int ADelta)
{
    Q_UNUSED(AScenePosition)
    Q_UNUSED(ADelta)
    return false;
}

bool MapLocationSelector::mapContextMenu(MercatorCoordinates ACoordinates, Menu *AMenu)
{
    Q_UNUSED(ACoordinates)
    Q_UNUSED(AMenu)
    finishSelectLocation(FLocationSelector, true);
    return true;    // Display no menu!!!
}

//IMapObjectDataHolder
void MapLocationSelector::objectUpdated(SceneObject *ASceneObject, int ARole)
{
    Q_UNUSED(ASceneObject)
    Q_UNUSED(ARole)
}

QGraphicsItem *MapLocationSelector::mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement)
{
	Q_UNUSED(ASceneObject)
	Q_UNUSED(ARole)

    if (!ACurrentElement)
    {
        QPen pen;
        pen.setColor(Qt::green);
        pen.setWidth(1);
        QGraphicsLineItem *item=new QGraphicsLineItem(0, 0, 0, 0);
        item->setPen(pen);
        return item;
    }
    return ACurrentElement;
}

//SLOTS
//IMapObjectHandler
void MapLocationSelector::onMapObjectInserted(int AType, const QString &AId)
{
    if (AType==MOT_LOCATION_SELECTOR)
    {
        emit mapDataChanged(AType, AId, MDR_LOCATION_SELECTOR_TOP);
        emit mapDataChanged(AType, AId, MDR_LOCATION_SELECTOR_BOTTOM);
        emit mapDataChanged(AType, AId, MDR_LOCATION_SELECTOR_LEFT);
        emit mapDataChanged(AType, AId, MDR_LOCATION_SELECTOR_RIGHT);
    }
}

void MapLocationSelector::onMapObjectRemoved(int AType, const QString &AId)
{
    Q_UNUSED(AType)
    Q_UNUSED(AId)
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_maplocationselector, MapLocationSelector)
#endif
