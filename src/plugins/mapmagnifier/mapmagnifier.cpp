#include <QGraphicsProxyWidget>
#include <QGraphicsDropShadowEffect>
#include <QToolTip>
#include <interfaces/ioptionsmanager.h>
#include <definitions/mapobjecttyperole.h>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/shortcutgrouporders.h>
#include <definitions/shortcuts.h>
#include <utils/action.h>
#include <MapScale>
#include <MapObject>

#include "mapmagnifier.h"
#include "magnifieroptions.h"

#define MDR_MAGNIFIER 100

MapMagnifierView::MapMagnifierView(QGraphicsScene *AScene, QWidget *AParent): QGraphicsView(AScene, AParent)
{}

bool MapMagnifierView::event(QEvent *AEvent)
{
    if (AEvent->type() == QEvent::ToolTip)
    {
        QString toolTipText;
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(AEvent);
		SceneObject *activeObject = qobject_cast<MapScene *>(scene())->activeObject();
        if (activeObject)
            toolTipText = activeObject->mapObject()->toolTipText(helpEvent);
        if (toolTipText.isEmpty())
        {
            QToolTip::hideText();
            AEvent->ignore();
        }
        else
            QToolTip::showText(helpEvent->globalPos(), toolTipText);
        return true;
    }
    return QGraphicsView::event(AEvent);
}

void MapMagnifierView::paintEvent(QPaintEvent *APaintEvent)
{
    QGraphicsView::paintEvent(APaintEvent);
}

MapMagnifier::MapMagnifier(QObject *parent) :
    QObject(parent),
    FMap(NULL),
    FOptionsManager(NULL)
{}

void MapMagnifier::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Map Magnifier");
    APluginInfo->description = tr("Allows to see part of the map magnified");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(MAP_UUID);
}

bool MapMagnifier::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder)

    IPlugin *plugin = APluginManager->pluginInterface("IMap").value(0,NULL);
    if (plugin)
        FMap = qobject_cast<IMap *>(plugin->instance());
    else
        return false;

    plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    connect(Options::instance(), SIGNAL(optionsChanged(const OptionsNode &)), SLOT(onOptionsChanged(const OptionsNode &)));
    return true;
}

bool MapMagnifier::initObjects()
{
    Shortcuts::declareGroup(SCTG_MAP_MAGNIFIER, tr("Magnifier"), SGO_MAP);
    Shortcuts::declareShortcut(SCT_MAP_MAGNIFIER_TOGGLE, tr("On/off"), tr("F8", "Magnifier"), Shortcuts::ApplicationShortcut);
    Shortcuts::declareShortcut(SCT_MAP_MAGNIFIER_ZOOMIN, tr("Zoom in"), tr("Alt++", "Zoom In"), Shortcuts::WindowShortcut);
    Shortcuts::declareShortcut(SCT_MAP_MAGNIFIER_ZOOMOUT, tr("Zoom out"), tr("Alt+-", "Zoom Out"), Shortcuts::WindowShortcut);

	FMap->geoMap()->setObjectHandler(MOT_MAGNIFIER, this);
	FMap->geoMap()->registerDataType(MDR_MAGNIFIER, MOT_MAGNIFIER, 100, MOP_NONE, MOP_CENTER);
	FMap->geoMap()->addDataHolder(MOT_MAGNIFIER, this);

	Action *action = FMap->addMenuAction(tr("Magnifier"), QString(RSR_STORAGE_MENUICONS),QString(MNI_MAPMAGNFIER), 1);
    action->setCheckable(true);
    action->setShortcutId(SCT_MAP_MAGNIFIER_TOGGLE);
    connect(action, SIGNAL(triggered(bool)), SLOT(onMaginifier()));

    connect(Shortcuts::instance(), SIGNAL(shortcutActivated(QString,QWidget*)), SLOT(onShortcutActivated(QString,QWidget*)));
    return true;
}

bool MapMagnifier::initSettings()
{
    Options::setDefaultValue(OPV_MAP_MAGNIFIER_SIZE, 64);
    Options::setDefaultValue(OPV_MAP_MAGNIFIER_SCALE, true);
    Options::setDefaultValue(OPV_MAP_MAGNIFIER_RULERS, true);
    Options::setDefaultValue(OPV_MAP_MAGNIFIER_OBJECTS, true);
    Options::setDefaultValue(OPV_MAP_MAGNIFIER_HIGHPRECISION, true);
    Options::setDefaultValue(OPV_MAP_MAGNIFIER_ZOOM, 1);
    Options::setDefaultValue(OPV_MAP_MAGNIFIER_SHADOW_COLOR, QColor(Qt::black));
    Options::setDefaultValue(OPV_MAP_MAGNIFIER_SHADOW_OPACITY, 255);
    Options::setDefaultValue(OPV_MAP_MAGNIFIER_SHADOW_SHIFT, QPointF(1,1));
    Options::setDefaultValue(OPV_MAP_MAGNIFIER_SHADOW_BLUR, 4);
    Options::setDefaultValue(OPV_MAP_MAGNIFIER_ZOOMFACTOR, true);
    Options::setDefaultValue(OPV_MAP_MAGNIFIER_ZOOMFACTOR_COLOR, QColor(Qt::cyan));
    Options::setDefaultValue(OPV_MAP_MAGNIFIER_ZOOMFACTOR_OPACITY, 1);
    Options::setDefaultValue(OPV_MAP_MAGNIFIER_ZOOMFACTOR_FONT, QFont("System,16,-1,5,75,0,0,0,0,0"));

    if (FOptionsManager)
    {
		IOptionsDialogNode dnode = {ONO_MAPMAGNIFIER, OPN_MAPMAGNIFIER, MNI_MAPMAGNFIER, tr("Map magnifier")};
        FOptionsManager->insertOptionsDialogNode(dnode);
		FOptionsManager->insertOptionsDialogHolder(this);
    }
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> MapMagnifier::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
    if (ANodeId == OPN_MAPMAGNIFIER )
		widgets.insertMulti(OWO_MAPMAGNIFIER, new MagnifierOptions(AParent));
    return widgets;
}

void MapMagnifier::objectUpdated(SceneObject *ASceneObject, int ARole)
{
    Q_UNUSED(ASceneObject)
    Q_UNUSED(ARole)
}

QString MapMagnifier::toolTipText(const MapObject *AMapObject, const QHelpEvent *AEvent) const
{
	Q_UNUSED(AMapObject)
	Q_UNUSED(AEvent)
//TODO: Implement!!!    
/*
    ISceneObject *sceneObject = FMap->getSceneObject(MOT_MAGNIFIER, AMapObject->id());
    QGraphicsItem *item = sceneObject->getElementByRole(MDR_MAGNIFIER);
    QGraphicsProxyWidget *proxyWidget = qgraphicsitem_cast<QGraphicsProxyWidget *>(item);
    if (proxyWidget)
    {
        MapMagnifierView *view = qobject_cast<MapMagnifierView *>(proxyWidget->widget());
        IMapScene *mapScene = qobject_cast<IMapScene *>(view->scene());
        if (mapScene)
        {
            ISceneObject *activeObject = mapScene->activeObject();
            if (activeObject)
                return activeObject->mapObject()->toolTipText(AEvent);
        }
    }
*/
    return QString();
}

QGraphicsItem *MapMagnifier::mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement)
{
	Q_UNUSED(ASceneObject)
	Q_UNUSED(ARole)

    int size=Options::node(OPV_MAP_MAGNIFIER_SIZE).value().toInt();
    if (!ACurrentElement)
    {
//        IMapScene *mapScene = FMapScenePlugin->createMapScene(NULL, 0);
		MapScene *mapScene = new MapScene(NULL, NULL);
		mapScene->setMapSource(FMap->getMapSource()->mapSource());
        mapScene->selectMode(Options::node(OPV_MAP_MODE).value().toInt());
        mapScene->setZoom(Options::node(OPV_MAP_ZOOM).value().toInt()+Options::node(OPV_MAP_MAGNIFIER_ZOOM).value().toInt());
        mapScene->setMapCenter(FMap->mapCenter());
        QGraphicsScene *scene=mapScene->instance();
        scene->setSceneRect(0, 0, size*2, size*2);

        // Map scene on-screen display initialization
        if (Options::node(OPV_MAP_MAGNIFIER_RULERS).value().toBool())
        {
            float ZVLine = 6.0;
            QPen pen(Qt::red);
            pen.setWidth(1);

            _LineX  = scene->addLine(0, size, size*2, size, pen);  _LineX->setZValue(ZVLine);
            _Line1X = scene->addLine(0, -2, 0, +2, pen);    _Line1X->setZValue(ZVLine);
            _Line2X = scene->addLine(0, -4, 0, +4, pen);    _Line2X->setZValue(ZVLine);
            _LineX1 = scene->addLine(0, -2, 0, +2, pen);    _LineX1->setZValue(ZVLine);
            _LineX2 = scene->addLine(0, -4, 0, +4, pen);    _LineX2->setZValue(ZVLine);

            _LineY  = scene->addLine(size, 0, size, size*2, pen);  _LineY->setZValue(ZVLine);
            _Line1Y = scene->addLine(-2, 0, +2, 0, pen);    _Line1Y->setZValue(ZVLine);
            _Line2Y = scene->addLine(-4, 0, +4, 0, pen);    _Line2Y->setZValue(ZVLine);
            _LineY1 = scene->addLine(-2, 0, +2, 0, pen);    _LineY1->setZValue(ZVLine);
            _LineY2 = scene->addLine(-4, 0, +4, 0, pen);    _LineY2->setZValue(ZVLine);

			onOptionsChanged(Options::node(OPV_MAP_OSD_CMARKER_COLOR));
			onOptionsChanged(Options::node(OPV_MAP_OSD_CMARKER_VISIBLE));
        }

        MapMagnifierView *view=new MapMagnifierView(scene);
        view->setInteractive(false);
        view->setFixedSize(size*2, size*2);

        scene->setParent(view);

        QRegion mask(0, 0, size*2, size*2, QRegion::Ellipse);
        view->setMask(mask);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setFrameShape(QFrame::NoFrame);
        view->setCursor(Qt::CrossCursor);

        QGraphicsProxyWidget *proxyWidget = new QGraphicsProxyWidget();
        proxyWidget->setAcceptedMouseButtons(Qt::NoButton);
        proxyWidget->setAcceptHoverEvents(false);
        proxyWidget->setAcceptTouchEvents(false);
        proxyWidget->setAcceptDrops(false);
        proxyWidget->setWidget(view);

        QGraphicsDropShadowEffect *shadow=new QGraphicsDropShadowEffect(proxyWidget);
        shadow->setOffset(Options::node(OPV_MAP_MAGNIFIER_SHADOW_SHIFT).value().toPointF());
        shadow->setBlurRadius(Options::node(OPV_MAP_MAGNIFIER_SHADOW_BLUR).value().toInt());
        QColor color=Options::node(OPV_MAP_MAGNIFIER_SHADOW_COLOR).value().value<QColor>();
        color.setAlpha(Options::node(OPV_MAP_MAGNIFIER_SHADOW_OPACITY).value().toInt());
        shadow->setColor(color);
        proxyWidget->setGraphicsEffect(shadow);

        int zoomFactor=Options::node(OPV_MAP_MAGNIFIER_ZOOM).value().toInt();
        QGraphicsSimpleTextItem *zoomFactorItem=new QGraphicsSimpleTextItem(proxyWidget);
        zoomFactorItem->setText(tr("x%1").arg(1<<zoomFactor));
        QRectF zoomFactorRect=zoomFactorItem->boundingRect();
        zoomFactorItem->setPos(size*2-zoomFactorRect.width(), 0);
        zoomFactorItem->setBrush(Options::node(OPV_MAP_MAGNIFIER_ZOOMFACTOR_COLOR).value().value<QColor>());
        zoomFactorItem->setOpacity(Options::node(OPV_MAP_MAGNIFIER_ZOOMFACTOR_OPACITY).value().toFloat());
        zoomFactorItem->setFont(Options::node(OPV_MAP_MAGNIFIER_ZOOMFACTOR_FONT).value().value<QFont>());
        zoomFactorItem->setVisible(Options::node(OPV_MAP_MAGNIFIER_ZOOMFACTOR).value().toBool());
        connect(scene, SIGNAL(mppChanged(double)), SLOT(onMppChanged(double)));

        if (Options::node(OPV_MAP_MAGNIFIER_SCALE).value().toBool())
        {
            QPalette palette;
            palette.setColor(QPalette::Foreground, Options::node(OPV_MAP_OSD_CONTR_FOREGROUND).value().value<QColor>());
            palette.setColor(QPalette::Midlight, Options::node(OPV_MAP_OSD_CONTR_MIDLIGHT).value().value<QColor>());
            palette.setColor(QPalette::Shadow, Options::node(OPV_MAP_OSD_CONTR_SHADOW).value().value<QColor>());
            palette.setColor(QPalette::Dark, Options::node(OPV_MAP_OSD_CONTR_DARK).value().value<QColor>());
            GraphicsMapScaleItem *scale=new GraphicsMapScaleItem(size, palette, proxyWidget);
            scale->setLocalizedStrings(tr("%1 m"), tr("%1 km"));
            scale->setPos(size-1, size*2-scale->boundingRect().height());            
            connect(scene, SIGNAL(mppChanged(double)), SLOT(onMppChanged(double)));
        }
//        proxyWidget->setToolTip("Magnifier!!!");
        return proxyWidget;
    }
    return ACurrentElement;
}

bool MapMagnifier::mapMouseClicked(MercatorCoordinates ACoordinates, Qt::MouseButton AButton)
{
	Q_UNUSED(ACoordinates)
	SceneObject *object=mapScene()->activeObject();
    return object?object->mouseClicked(AButton):false;
}

bool MapMagnifier::mapMouseDoubleClicked(MercatorCoordinates ACoordinates, Qt::MouseButton AButton)
{
	Q_UNUSED(ACoordinates)
	SceneObject *object=mapScene()->activeObject();
    return object?object->mouseDoubleClicked(AButton):false;
}

bool MapMagnifier::mapContextMenu(MercatorCoordinates ACoordinates, Menu *AMenu)
{
	Q_UNUSED(ACoordinates)
	SceneObject *object=mapScene()->activeObject();
    return object?object->contextMenu(AMenu):false;
}

bool MapMagnifier::mapMouseWheelMoved(const QPointF &AScenePosition, int ADelta)
{
	Q_UNUSED(AScenePosition)

    int z=Options::node(OPV_MAP_MAGNIFIER_ZOOM).value().toInt();
    if (ADelta>0)
        {if(z<4) z++;}
    else
    {if (z>1) z--;}
    Options::node(OPV_MAP_MAGNIFIER_ZOOM).setValue(z);
    return true;
}

bool MapMagnifier::mapMouseMoved(QPointF ANewPosition, QPointF AOldPosition, MercatorCoordinates ACoordinates, Qt::MouseButtons AMouseButtons)
{
    Q_UNUSED(AOldPosition)
    Q_UNUSED(ACoordinates)

    if (AMouseButtons==Qt::NoButton)
    {
		SceneObject *object=FMap->geoMap()->getSceneObject(MOT_MAGNIFIER, "magnifier");
		MapScene *scene=mapScene(object);
        if (scene)
        {
            int r=Options::node(OPV_MAP_MAGNIFIER_SIZE).value().toInt();
            int cx=ANewPosition.x()-object->position().x();
            int cy=ANewPosition.y()-object->position().y();
            if (Options::node(OPV_MAP_MAGNIFIER_HIGHPRECISION).value().toBool() && FMap->isActive() && (cx*cx+cy*cy<r*r))
            {
                QPointF shift(ANewPosition-object->position());
                if (!shift.isNull())
                {
                    MercatorCoordinates coordinates=scene->calculateLocation(shift);
                    scene->setMapCenter(coordinates);
					FMap->geoMap()->addObject(MOT_MAGNIFIER, "magnifier", coordinates);
                    shift=object->position()-ANewPosition;
                    if (!shift.isNull())
                        QCursor::setPos(QCursor::pos()+shift.toPoint());
                }
            }
            else
            {
                object->setPosition(ANewPosition);
                scene->setMapCenter(ACoordinates);
            }
            scene->mouseMove(QPointF(0,0));
        }
    }
    return false;
}

void MapMagnifier::adjustCentralRulers(int ALength)
{
	MapScene *scene=mapScene();
    if (scene)
    {
        qreal x=scene->instance()->width()/2;
        qreal y=scene->instance()->height()/2;
        _Line1X->setPos(x-ALength/2, y);
        _Line2X->setPos(x-ALength, y);
        _LineX1->setPos(x+ALength/2, y);
        _LineX2->setPos(x+ALength, y);
        _Line1Y->setPos(x, y-ALength/2);
        _Line2Y->setPos(x, y-ALength);
        _LineY1->setPos(x, y+ALength/2);
        _LineY2->setPos(x, y+ALength);
    }
}

void MapMagnifier::setScaleColor(const QColor &AColor)
{
	Q_UNUSED(AColor)
}

void MapMagnifier::setCenterMarkerColor(const QColor &AColor)
{
	if (FMap->geoMap()->isObjectExists(MOT_MAGNIFIER, "magnifier"))
    {
        _Line1X->setPen(AColor);
        _Line1Y->setPen(AColor);
        _Line2X->setPen(AColor);
        _Line2Y->setPen(AColor);
        _LineX->setPen(AColor);
        _LineX1->setPen(AColor);
        _LineX2->setPen(AColor);
        _LineY->setPen(AColor);
        _LineY1->setPen(AColor);
        _LineY2->setPen(AColor);
    }
}

void MapMagnifier::setCenterMarkerVisible(bool AVisible)
{
	if (FMap->geoMap()->isObjectExists(MOT_MAGNIFIER, "magnifier"))
    {
        _Line1X->setVisible(AVisible);
        _Line1Y->setVisible(AVisible);
        _Line2X->setVisible(AVisible);
        _Line2Y->setVisible(AVisible);
        _LineX->setVisible(AVisible);
        _LineX1->setVisible(AVisible);
        _LineX2->setVisible(AVisible);
        _LineY->setVisible(AVisible);
        _LineY1->setVisible(AVisible);
        _LineY2->setVisible(AVisible);
    }
}

MapScene *MapMagnifier::mapScene(const SceneObject *ASceneObject) const
{
    if (ASceneObject)
    {
        QGraphicsProxyWidget *proxyWidget=qgraphicsitem_cast<QGraphicsProxyWidget *>(ASceneObject->getElementByRole(MDR_MAGNIFIER));
        if (proxyWidget)
        {
            MapMagnifierView *view=qobject_cast<MapMagnifierView *>(proxyWidget->widget());
            if (view)
				return qobject_cast<MapScene *>(view->scene());
        }
    }
    return NULL;
}

MapScene *MapMagnifier::mapScene(const MapObject *AMapObject) const
{
	return mapScene(FMap->geoMap()->getScene()->sceneObject(AMapObject));
}

MapScene *MapMagnifier::mapScene() const
{
	return mapScene(FMap->geoMap()->getSceneObject(MOT_MAGNIFIER, "magnifier"));
}

//SLOTS
void MapMagnifier::onMapObjectInserted(int AType, const QString &AId)
{
    if (AType==MOT_MAGNIFIER && AId=="magnifier")
        emit mapDataChanged(AType, AId, MDR_MAGNIFIER);
}

void MapMagnifier::onMapObjectRemoved(int AType, const QString &AId)
{
	Q_UNUSED(AType)
	Q_UNUSED(AId)
//    if (AType==MOT_MAGNIFIER && AId=="magnifier")
//        ISceneObject *sceneObject = FMap->getSceneObject(MOT_MAGNIFIER, "magnifier");
}

void MapMagnifier::onMaginifier()
{
    Action *action=qobject_cast<Action *>(sender());
    if (action->isChecked())
    {
		if (FMap->setMouseGrabber(this))
        {
			QList<MapObject *>objects=FMap->geoMap()->getMapObjects();
			MapScene *scene=mapScene(FMap->geoMap()->getScene()->sceneObject(FMap->geoMap()->addObject(MOT_MAGNIFIER, "magnifier", FMap->mapCenter())));
            if (scene)
            {
				for (QList<MapObject *>::const_iterator it=objects.begin(); it!=objects.end(); it++)
                    if ((*it)->hasLocation() && (*it)->objectType()!=MOT_MAGNIFIER)
                    {
						SceneObject *object=scene->addObject(*it);
                        object->updatePosition();
                        object->updateZValue();
                    }
				connect(FMap->instance(), SIGNAL(mapObjectAdded(MapObject *)), SLOT(onMapObjectAdded(MapObject*)));

                FMap->insertWindowShortcut(SCT_MAP_MAGNIFIER_ZOOMIN);
                FMap->insertWindowShortcut(SCT_MAP_MAGNIFIER_ZOOMOUT);
            }
        }
    }
    else
    {
		if (FMap->releaseMouseGrabber(this))
        {
			disconnect(FMap->instance(), SIGNAL(mapObjectAdded(MapObject *)), this, SLOT(onMapObjectAdded(MapObject*)));

            FMap->removeWindowShortcut(SCT_MAP_MAGNIFIER_ZOOMIN);
            FMap->removeWindowShortcut(SCT_MAP_MAGNIFIER_ZOOMOUT);
			FMap->geoMap()->removeObject(MOT_MAGNIFIER, "magnifier");
        }
    }
}

void MapMagnifier::onMapObjectAdded(MapObject *AMapObject)
{
    if (AMapObject->hasLocation())
        mapScene()->addObject(AMapObject)->instance()->setVisible(true);
}

void MapMagnifier::onMppChanged(double mpp)
{
	MapObject *mapObject = FMap->geoMap()->getObject(MOT_MAGNIFIER, "magnifier");
	QGraphicsItem *view = FMap->geoMap()->getScene()->sceneObject(mapObject)->getElementByRole(MDR_MAGNIFIER);
    QList<QGraphicsItem *> children=view->childItems();
    for (QList<QGraphicsItem *>::const_iterator it=children.begin(); it!=children.end(); it++)
        if ((*it)->type()==GraphicsMapScaleItem::Type)
        {
            GraphicsMapScaleItem *scale=qgraphicsitem_cast<GraphicsMapScaleItem *>(*it);
            if (scale)
            {
                scale->setMpp(mpp);
                if (Options::node(OPV_MAP_MAGNIFIER_RULERS).value().toBool())
                    adjustCentralRulers(scale->length());
            }
        }
        else if ((*it)->type()==QGraphicsSimpleTextItem::Type)
        {
            QGraphicsSimpleTextItem *zoomFactorItem=qgraphicsitem_cast<QGraphicsSimpleTextItem *>(*it);
            zoomFactorItem->setText(tr("x%1").arg(1<<Options::node(OPV_MAP_MAGNIFIER_ZOOM).value().toInt()));
        }
}

void MapMagnifier::onOptionsChanged(const OptionsNode &ANode)
{
	MapScene *scene=mapScene(FMap->geoMap()->getSceneObject(MOT_MAGNIFIER, "magnifier"));
    if (ANode.path()==OPV_MAP_SOURCE)
    {
        if (scene)
			scene->setMapSource(FMap->getMapSource()->mapSource());
    }
    else if (ANode.path()==OPV_MAP_MODE)
    {
        if (scene)
            scene->selectMode(ANode.value().toInt());
    }
    else if (ANode.path()==OPV_MAP_ZOOM)
    {
        if (scene)
            scene->setZoom(ANode.value().toInt()+Options::node(OPV_MAP_MAGNIFIER_ZOOM).value().toInt());
    }    
	else if (ANode.path()==OPV_MAP_OSD_CMARKER_COLOR)
    {
        QColor color = ANode.value().value<QColor>();
		color.setAlpha(Options::node(OPV_MAP_OSD_CMARKER_ALPHA).value().toInt());
        setCenterMarkerColor(color);
    }
	else if (ANode.path()==OPV_MAP_OSD_CMARKER_ALPHA)
    {
		QColor color = Options::node(OPV_MAP_OSD_CMARKER_COLOR).value().value<QColor>();
        color.setAlpha(ANode.value().toInt());
        setCenterMarkerColor(color);
    }
	else if (ANode.path()==OPV_MAP_OSD_CMARKER_VISIBLE)
        setCenterMarkerVisible(ANode.value().toBool());
    else if (ANode.path()==OPV_MAP_OSD_CONTR_FOREGROUND ||
             ANode.path()==OPV_MAP_OSD_CONTR_SHADOW ||
             ANode.path()==OPV_MAP_OSD_CONTR_MIDLIGHT ||
             ANode.path()==OPV_MAP_OSD_CONTR_DARK ||
             ANode.path()==OPV_MAP_MAGNIFIER_ZOOM ||
             ANode.path()==OPV_MAP_MAGNIFIER_ZOOMFACTOR ||
             ANode.path()==OPV_MAP_MAGNIFIER_ZOOMFACTOR_COLOR ||
             ANode.path()==OPV_MAP_MAGNIFIER_ZOOMFACTOR_OPACITY ||
             ANode.path()==OPV_MAP_MAGNIFIER_ZOOMFACTOR_FONT)
    {
        if (ANode.path()==OPV_MAP_MAGNIFIER_ZOOM)
            if (scene)
                scene->setZoom(Options::node(OPV_MAP_ZOOM).value().toInt()+ANode.value().toInt());

		MapObject *object=FMap->geoMap()->getObject(MOT_MAGNIFIER, "magnifier");
        if (object) // Map Magnifier is on
        {
            bool zoomFactor=ANode.path()==OPV_MAP_MAGNIFIER_ZOOM ||
                            ANode.path()==OPV_MAP_MAGNIFIER_ZOOMFACTOR ||
                            ANode.path()==OPV_MAP_MAGNIFIER_ZOOMFACTOR_COLOR ||
                            ANode.path()==OPV_MAP_MAGNIFIER_ZOOMFACTOR_OPACITY ||
                            ANode.path()==OPV_MAP_MAGNIFIER_ZOOMFACTOR_FONT;
			QGraphicsItem *magnifier=FMap->geoMap()->getScene()->sceneObject(object)->getElementByRole(MDR_MAGNIFIER);
            QList<QGraphicsItem *> childItems=magnifier->childItems();
            for (QList<QGraphicsItem *>::const_iterator it=childItems.constBegin(); it!=childItems.constEnd(); it++)
                if (zoomFactor && (*it)->type()==QGraphicsSimpleTextItem::Type)
                {
                    QGraphicsSimpleTextItem *zoomFactorItem=qgraphicsitem_cast<QGraphicsSimpleTextItem *>(*it);
                    if (ANode.path()==OPV_MAP_MAGNIFIER_ZOOMFACTOR)                        
                        zoomFactorItem->setVisible(ANode.value().toBool());
                    else if (ANode.path()==OPV_MAP_MAGNIFIER_ZOOMFACTOR_COLOR)
                        zoomFactorItem->setBrush(ANode.value().value<QColor>());
                    else if (ANode.path()==OPV_MAP_MAGNIFIER_ZOOMFACTOR_OPACITY)
                        zoomFactorItem->setOpacity(ANode.value().toFloat());
                    else if (ANode.path()==OPV_MAP_MAGNIFIER_ZOOMFACTOR_FONT || ANode.path()==OPV_MAP_MAGNIFIER_ZOOM)
                    {
                        if (ANode.path()==OPV_MAP_MAGNIFIER_ZOOMFACTOR_FONT)
                            zoomFactorItem->setFont(ANode.value().value<QFont>());
                        QRectF zoomFactorRect=zoomFactorItem->boundingRect();
                        QRectF magnifierRect=magnifier->boundingRect();
                        zoomFactorItem->setPos(magnifierRect.width()-zoomFactorRect.width(), 0);
                    }
                }
                else if (!zoomFactor && (*it)->type()==GraphicsMapScaleItem::Type)
                {
                    GraphicsMapScaleItem *scale=qgraphicsitem_cast<GraphicsMapScaleItem *>(*it);
                    QPalette::ColorRole role=ANode.path()==OPV_MAP_OSD_CONTR_FOREGROUND?QPalette::Text:
                                             ANode.path()==OPV_MAP_OSD_CONTR_SHADOW?QPalette::Shadow:
                                             ANode.path()==OPV_MAP_OSD_CONTR_MIDLIGHT?QPalette::Midlight:QPalette::Dark;
                    scale->palette().setColor(role, ANode.value().value<QColor>());
                    break;
                }
        }
    }
}

void MapMagnifier::onShortcutActivated(const QString &AShortcutId, QWidget *AWidget)
{
	Q_UNUSED(AWidget)

    if (AShortcutId==SCT_MAP_MAGNIFIER_ZOOMIN ||
        AShortcutId==SCT_MAP_MAGNIFIER_ZOOMOUT)
    {
        int z=Options::node(OPV_MAP_MAGNIFIER_ZOOM).value().toInt();
        if (AShortcutId==SCT_MAP_MAGNIFIER_ZOOMIN)
            {if(z<4) z++;}
        else
            {if (z>1) z--;}
        Options::node(OPV_MAP_MAGNIFIER_ZOOM).setValue(z);
    }
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapmagnifier, MapMagnifier)
#endif
