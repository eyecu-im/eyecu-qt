#include <QGraphicsPixmapItem>
#include <QtGui>
#include <QFileDialog>
#include <QApplication>
#include <MapObject>

#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/mapicons.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>

#include "placeview.h"
#include "placeviewform.h"

#define SDR_ID      1
#define SDR_ACTION  2
#define ACTION1     1
#define ACTION2     2
#define ACTION3     3
#define ACTION4     4
#define ACTION5     5

PlaceView::PlaceView(QObject *parent) :
    QObject(parent),
	FMap(NULL),
	FGeoMap(NULL),
    FPoi(NULL),
    FOptionsManager(NULL),
    FPlaceViewForm(NULL)
{
}

void PlaceView::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("View photos of places");
    APluginInfo->description = tr("Allows you to view the photos of places");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
}


bool PlaceView::initConnections(IPluginManager *APluginManager, int &AInitOrder)
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

    QList<IPlugin *> plugins = APluginManager->pluginInterface("IPlaceViewProvider");
    if (plugins.isEmpty())
        return false;       // No providers found

    for (QList<IPlugin *>::const_iterator it=plugins.constBegin(); it!=plugins.constEnd(); it++)
        FPlaceViewProviders.insert((*it)->pluginUuid(), qobject_cast<IPlaceViewProvider *>((*it)->instance()));

    AInitOrder = 200;
    return true;
}

bool PlaceView::initObjects()
{
	if (FGeoMap)
    {
		FGeoMap->setObjectHandler(MOT_PLACEVIEW, this);
		FGeoMap->registerDataType(MDR_PLACEVIEW_ICON, MOT_PLACEVIEW, 150, MOP_CENTER, MOP_CENTER);
		FGeoMap->addDataHolder(MOT_PLACEVIEW, this);

		FGeoMap->setObjectHandler(MOT_PLACEVIEWOBJECT, this);
		FGeoMap->registerDataType(MDR_PLACEVIEW_OBJECT, MOT_PLACEVIEWOBJECT, 150, MOP_TOP, MOP_TOP);
		FGeoMap->addDataHolder(MOT_PLACEVIEWOBJECT, this);

		Action *action = FMap->addMenuAction(tr("View photos of places"), QString(RSR_STORAGE_MENUICONS),QString(MNI_MAP_PLACEVIEW), 0);
        action->setCheckable(false);
        //action->setUserData()
        //action->setShortcutId(SCT_MAP_MAGNIFIER_TOGGLE);
        connect(action, SIGNAL(triggered(bool)), SLOT(onPlaceViewTriggered()));

        if(!FPlaceViewForm)
        {
			FPlaceViewForm = new PlaceViewForm(FPlaceViewProviders, FMap,this);
            //connect(FPlaceViewForm,SIGNAL(gotoView(double,double)),this,SLOT(onGotoView(double,double)));
            connect(FPlaceViewForm,SIGNAL(poisReady()),this,SLOT(onPoisReady()));
            connect(FPlaceViewForm,SIGNAL(poisDelete()),this,SLOT(onPoisDelete()));
            connect(this, SIGNAL(destroyed()), FPlaceViewForm, SLOT(deleteLater()));
        }
    }

    if (FPoi)
    {
        Action *action  = FPoi->addMenuAction(tr("View photos of places"),QString(RSR_STORAGE_MENUICONS),QString(MNI_MAP_PLACEVIEW));
        action->setCheckable(false);
        //action->setUserData()
        connect(action,SIGNAL(triggered(bool)), SLOT(onPlaceViewPoiTriggered()));
    }
    return true;
}

bool PlaceView::initSettings()
{
    Options::setDefaultValue(OPV_MAP_PLACEVIEW_PROVIDER, "{515bb127-0d6a-49a3-9ca5-2b3fc486cfae}");
    Options::setDefaultValue(OPV_MAP_PLACEVIEW,true);
    Options::setDefaultValue(OPV_MAP_PLACEVIEW_RADIUS,0);
//Options::setDefaultValue(OPV_MAP_PLACEVIEW_TYPE,"accounting");
    Options::setDefaultValue(OPV_MAP_PLACEVIEW_TYPE,0);
    Options::setDefaultValue(OPV_MAP_PLACEVIEW_RANKBY,0);
    Options::setDefaultValue(OPV_MAP_PLACEVIEW_WAY,0);

//    if (FOptionsManager)
//        FOptionsManager->insertOptionsHolder(this);
    return true;
}
/*
QMultiMap<int, IOptionsWidget *> PlaceView::optionsWidgets(const QString &ANodeId, QWidget *AParent)
{
    QMultiMap<int, IOptionsWidget *> widgets;
//    if (FOptionsManager && ANodeId == OPN_ROSTER)
//        widgets.insertMulti(OWO_MAPPLACEVIEW, FOptionsManager->optionsNodeWidget(Options::node(OPV_MAP_STREETVIEW),tr("Map Street Viewer"),AParent));
//        widgets.insertMulti(OWO_MAPPLACEVIEW, new MapSearchOptions(AParent));
    return widgets;
}
*/
bool PlaceView::contextMenu(const QString &AId, QMenu *AMenu)
{
//AId= "viewcenter" or "6a3795486254f29013e8c31d93d421e14e5ba4bf"
	Menu *menu = qobject_cast<Menu *>(AMenu);
	if (menu)
	{
		qDebug() << "Here!";
		Action *action;
		if(AId==MNI_MAP_PLACEVIEW)
		{
			QStyle *style = QApplication::style();
			action=new Action(menu);
			action->setText(tr("View"));
			action->setIcon(RSR_STORAGE_MENUICONS, MNI_MAP_PLACEVIEW);
			action->setData(SDR_ID, AId);
			action->setData(SDR_ACTION, ACTION1);
			connect(action, SIGNAL(triggered()), SLOT(onViewActionTriggered()));
			menu->addAction(action);

			action=new Action(menu);
			action->setText(tr("Close"));
			action->setIcon(style->standardIcon(QStyle::SP_TitleBarCloseButton));
			action->setData(SDR_ID, AId);
			action->setData(SDR_ACTION, ACTION2);
			connect(action, SIGNAL(triggered()), SLOT(onViewActionTriggered()));
			menu->addAction(action);
		}
		else if(FPoiMassive.contains(AId)) //"6a3795486254f29013e8c31d93d421e14e5ba4bf"
		{
			action=new Action(menu);
			action->setText(tr("Save point of interest"));
			action->setIcon(RSR_STORAGE_MENUICONS, MNI_POI);
			action->setData(SDR_ID, AId);
			action->setData(SDR_ACTION, ACTION3);
			connect(action, SIGNAL(triggered()), SLOT(onViewActionTriggered()));
			menu->addAction(action);

			GeolocElement poi;
			poi=FPoiMassive.value(AId);
			if(poi.property("photo_is").toBool())
			{
				action=new Action(menu);
				action->setText(tr("Save picture"));
				action->setIcon(RSR_STORAGE_MENUICONS, MNI_EDIT_ADD);
				action->setData(SDR_ID, AId);
				action->setData(SDR_ACTION, ACTION4);
				connect(action, SIGNAL(triggered()), SLOT(onViewActionTriggered()));
				menu->addAction(action);

				action=new Action(menu);
				action->setText(tr("Copy picture"));
				action->setIcon(RSR_STORAGE_MENUICONS, MNI_EDIT_COPY);
				action->setData(SDR_ID, AId);
				action->setData(SDR_ACTION, ACTION5);
				connect(action, SIGNAL(triggered()), SLOT(onViewActionTriggered()));
				menu->addAction(action);

			}
		}
	}
    return true;
}

void PlaceView::onViewActionTriggered()
{
    Action *action=qobject_cast<Action *>(sender());
    int     data= action->data(SDR_ACTION).toInt();
    QString AId = action->data(SDR_ID).toString();
    GeolocElement poi;

    switch (data)
    {
        case ACTION1:
            if (!FPlaceViewForm->isVisible())
                FPlaceViewForm->show();
            else if (!FPlaceViewForm->isActiveWindow())
                FPlaceViewForm->activateWindow();
        break;
        case ACTION2:
            if(FPlaceViewForm->isVisible())
                FPlaceViewForm->close();
            closeForm();
        break;
        case ACTION3:   //! save this POI
            poi=FPoiMassive.value(AId);
            //! convert and save this poi----??? ---
        break;
        case ACTION4:   //! Save picture
            if(FPoiMassive.contains(AId))
            {
                poi=FPoiMassive.value(AId);
                QString fileName;
				if(poi.property("photo_is").toBool())
                {
					fileName=poi.property("full_name").toString();
                    onSaveFile(fileName);
                }
            }
		break;
		case ACTION5:   //! Copy picture
			if(FPoiMassive.contains(AId))
			{
				poi=FPoiMassive.value(AId);
				QString fileName;
				if(poi.property("photo_is").toBool())
				{
					fileName=poi.property("full_name").toString();
					onCopyClicked(fileName);
				}
			}
        break;
    }
}

void PlaceView::onSaveFile(QString fileName)
{
    QFileDialog dialog;
    QImage image;
    image.load(fileName);
    QString nameNew =dialog.getSaveFileName(NULL,tr("Save file"), "PlaceViewImage.jpg", "JPEG fles (*.jpg *.jpeg)");
    if (!nameNew.isEmpty())
        image.save(nameNew);
}

void PlaceView::onCopyClicked(QString fileName)
{
	QImage im;
	im.load(fileName);
	QImage im2=im.scaledToWidth(100,Qt::SmoothTransformation);
//qDebug()<<"im2.size="<<im2.size();
//	im2.save("d:/im2-file.jpeg");
//	im.convertToFormat(QImage::Format_ARGB32);
	QApplication::clipboard()->setImage(im2);
}

void PlaceView::onPlaceViewTriggered()
{
	if (FGeoMap)
    {
		MercatorCoordinates coords = FMap->menuPopupPosition();
        double lat=coords.latitude();
        double lng=coords.longitude();
        statrViewer(lat,lng);
    }
}

void PlaceView::onPlaceViewPoiTriggered()
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

void PlaceView::statrViewer(double ALat,double ALng)
{
    showCenterView(MOT_PLACEVIEW, MNI_MAP_PLACEVIEW, MDR_PLACEVIEW_ICON,ALat,ALng);
    if (!FPlaceViewForm->isVisible())
        FPlaceViewForm->show();
    else if (!FPlaceViewForm->isActiveWindow())
        FPlaceViewForm->activateWindow();
    FPlaceViewForm->formDialogActivate(ALat,ALng);
}

//! --IMapObjectDataHolder--
QGraphicsItem *PlaceView::mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement)
{
    QPixmap pixmap;
    switch (ARole)
    {
        case MDR_PLACEVIEW_ICON:
			pixmap = IconStorage::staticStorage(RSR_STORAGE_MAPICONS)->getIcon(MPI_VIEWCENTER).pixmap(32);
            if (!pixmap.isNull())
            {
                if (ACurrentElement)
                {
                    QGraphicsPixmapItem *item = qgraphicsitem_cast<QGraphicsPixmapItem *>(ACurrentElement);
                    if (item)
                        item->setPixmap(pixmap);
                    return item;
                }
				else
                    return new QGraphicsPixmapItem(pixmap);
            }
        break;
        case MDR_PLACEVIEW_OBJECT:
            pixmap = getImageFromFile(ASceneObject->mapObject()->id());

            GeolocElement poi=FPoiMassive.value(ASceneObject->mapObject()->id());
			if(poi.property("photo_is").toBool())
            {
				if (!pixmap.isNull())
				{
                    QGraphicsPixmapItem * pixmapItem=NULL;
                    QSize size=pixmap.size();
                    int width=size.width();
                    int height=size.height();
                    int tmp=(width-10)/2;
                    int sm=1;

                    QPolygonF polygon;
                    polygon.append(QPointF(0,0));
                    polygon.append(QPointF(sm+width, 0));
                    polygon.append(QPointF(sm+width, sm+height));
                    polygon.append(QPointF(sm+width-tmp,sm+height));
                    polygon.append(QPointF((sm+width)/2,sm+height+5));
                    polygon.append(QPointF(tmp,sm+height));
                    polygon.append(QPointF(0,sm+height));

                    QGraphicsPolygonItem *polygonItem;
                    if (ACurrentElement)
                    {
                        polygonItem=qgraphicsitem_cast<QGraphicsPolygonItem *>(ACurrentElement);
                        polygonItem->setPolygon(polygon);
                    }
                    else
                    {
                        polygonItem = new QGraphicsPolygonItem(polygon);
                    }
                    pixmapItem = new QGraphicsPixmapItem(pixmap, polygonItem);
                    polygonItem->setBrush(QBrush(Qt::yellow, Qt::SolidPattern));// :green
                    polygonItem->setPen(QPen(Qt::red));
                    if (pixmapItem)
                        pixmapItem->setPos(1,1);
                    return polygonItem;
                }
            }
            else{
                if (!pixmap.isNull())
                {
                    if (ACurrentElement)
                    {
                        QGraphicsPixmapItem *item = qgraphicsitem_cast<QGraphicsPixmapItem *>(ACurrentElement);
                        if (item)
                            item->setPixmap(pixmap);
                        return item;
                    }
					else
                        return new QGraphicsPixmapItem(pixmap);
                }
            }
        break;
    }
    return ACurrentElement;
}


QPixmap PlaceView::getImageFromFile(QString const &AId)
{
    QPixmap pixmap;
    GeolocElement poi=FPoiMassive.value(AId);
	if(poi.property("photo_is").toBool())
    {
		pixmap.load(poi.property("full_name").toString());
        return pixmap.scaled(QSize(36, 36), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    else
    {
		pixmap.load(poi.property("icon_name").toString());
        return pixmap.scaled(QSize(18, 18), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
}

//! SLOTS - IMapObjectDataHolder
void PlaceView::onMapObjectInserted(int AType, const QString &AId)
{
    if(AType==MOT_PLACEVIEW){
        emit mapDataChanged(AType, AId, MDR_PLACEVIEW_ICON);
    }
    if(AType==MOT_PLACEVIEWOBJECT){
        emit mapDataChanged(AType, AId, MDR_PLACEVIEW_OBJECT);
    }
}

void PlaceView::onMapObjectRemoved(int AType, const QString &AId)
{
    Q_UNUSED(AType)
    Q_UNUSED(AId)
}

//! --IMapObjectHandler---
void PlaceView::mouseHoverEnter(SceneObject *ASceneObject)
{
    objectUpdated(ASceneObject);
}

void PlaceView::mouseHoverLeave(SceneObject *ASceneObject)
{
    objectUpdated(ASceneObject);
}

bool PlaceView::contextMenu(SceneObject *ASceneObject, QMenu *AMenu)
{
    return contextMenu(ASceneObject->mapObject()->id(), AMenu);
}

bool PlaceView::mouseDoubleClicked(SceneObject *ASceneObject, Qt::MouseButton AButton)
{
    Q_UNUSED(ASceneObject)
    Q_UNUSED(AButton)
//    if (!FPlaceViewForm->isActiveWindow())
//        FPlaceViewForm->activateWindow();
    return true;
}

//! IMapObjectHandler
void PlaceView::objectUpdated(SceneObject *ASceneObject, int ARole)
{    switch (ARole)
    {
        case MDR_PLACEVIEW_ICON:
            emit mapDataChanged(MOT_PLACEVIEW, ASceneObject->mapObject()->id(), ARole);
        break;
        case MDR_PLACEVIEW_OBJECT:
            emit mapDataChanged(MOT_PLACEVIEWOBJECT, ASceneObject->mapObject()->id(), ARole);
        break;
    }
}

//! IMapObjectHandler
QString PlaceView::toolTipText(const MapObject *AMapObject, const QHelpEvent *AEvent) const
{
    Q_UNUSED(AEvent)
    QString htmlPoi;
    QString AId=AMapObject->id();
    if(AId==MNI_MAP_PLACEVIEW)
    {
        htmlPoi.append("<font color='red'>")
                .append(tr("View Center"))
                .append("</font>");
    }
    else if(FPoiMassive.contains(AId))
    {
        QString fileName,html_attribut;
        GeolocElement APoi=FPoiMassive.value(AId);
		if(!APoi.property("photo_is").isNull()){
			html_attribut=APoi.property("photo_html_attribute").toString();
			fileName=APoi.property("full_name").toString();
        }
		else
			fileName=APoi.property("icon_name").toString();
        htmlPoi=QString("<tr><td colspan='2'><img src='%1'></td></tr>")
				.arg(fileName).append(APoi.description());

        if(!html_attribut.isEmpty())
            htmlPoi.append("<tr><td colspan='2'><b>%1:</b></td></tr>").arg(html_attribut);
    }
    return htmlPoi;
}

void PlaceView::showCenterView(int AType,QString AIdIcon,int ARole,double ALat,double ALng)
{
	FGeoMap->addObject(AType, AIdIcon, MercatorCoordinates(ALat, ALng));
    emit mapDataChanged(AType,AIdIcon,ARole);
}

void PlaceView::onPoisReady()
{
    if(!FPoiMassive.isEmpty())
    {
        onPoisDelete();     //! delete from map
        FPoiMassive.clear();

    }
    FPoiMassive= FPlaceViewForm->getPois();
    showAllPois();
}

void PlaceView::showAllPois()
{
    if(!FPoiMassive.isEmpty())
    {
        QHashIterator<QString, GeolocElement> it(FPoiMassive);
        while(it.hasNext())
        {
            GeolocElement poi=it.next().value();
            putOnePoi(poi);
        }
    }
}

void PlaceView::putOnePoi(GeolocElement poi)
{
	QString AId=poi.property("id").toString();
	if (FGeoMap->isObjectExists(MOT_PLACEVIEWOBJECT, AId))
    {
		FGeoMap->updateObject(MOT_PLACEVIEWOBJECT, AId);
        return;
    }
    else
    {
		FGeoMap->addObject(MOT_PLACEVIEWOBJECT, AId, MercatorCoordinates(poi.lat(), poi.lon()));
        emit mapDataChanged(MOT_PLACEVIEWOBJECT, AId, MDR_PLACEVIEW_OBJECT);
    }
}

void PlaceView::closeForm()
{
	FGeoMap->removeObject(MOT_PLACEVIEW, MNI_MAP_PLACEVIEW);
    onPoisDelete();
}

void PlaceView::onPoisDelete()
{
    if(!FPoiMassive.isEmpty())
    {
        QHashIterator<QString, GeolocElement> it(FPoiMassive);
        while(it.hasNext())
        {
            GeolocElement poi=it.next().value();
			QString AId=poi.property("id").toString();
            if(FPoiMassive.contains(AId)){
                FPoiMassive.remove(AId);
				FGeoMap->removeObject(MOT_PLACEVIEWOBJECT, AId);
            }
            //! delete icons
			QString fileName=poi.property("icon_name").toString();
            if(!fileName.isEmpty()){
                QFile file(fileName);
                if(file.exists())
                    file.remove();
            }
            //! delete photos
			if(!poi.property("photo_is").isNull())
            {
				QFile file(poi.property("full_name").toString());
                if(file.exists())
                    file.remove();
            }
        }
    }
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_placeview, PlaceView)
#endif
