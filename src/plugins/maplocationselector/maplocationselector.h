#ifndef MAPLOCATIONSELECTOR_H
#define MAPLOCATIONSELECTOR_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imaplocationselector.h>
#include <interfaces/imap.h>

class MapLocationSelectorProxy: public QGraphicsItem
{
    MapLocationSelectorProxy(QGraphicsItem *parent = 0):
        QGraphicsItem(parent)
    {setFlags(ItemHasNoContents);}
	void paint(QPainter *APainter, const QStyleOptionGraphicsItem *AOption, QWidget *AWidget=0) {Q_UNUSED(APainter) Q_UNUSED(AOption) Q_UNUSED(AWidget)} // Nothing to paint
    QRectF boundingRect() const {return QRectF(0, 0, 0, 0);}
};

class MapLocationSelector:
        public QObject,
        public IPlugin,
        public IMapLocationSelector,
        public IMapMouseGrabber,
		public MapSceneObjectHandler,
		public MapObjectDataHolder
{
    Q_OBJECT
	Q_INTERFACES (IPlugin IMapLocationSelector IMapMouseGrabber MapSceneObjectHandler MapObjectDataHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapLocationSelector")
#endif
public:
    explicit MapLocationSelector(QObject *parent = 0);

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return MAPLOCATIONSELECTOR_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    //IMapSceneObjectHandler;
    virtual bool mouseHit() const {return false;}
    virtual float zValue() const {return 10.0;}
	virtual void mouseHoverEnter(SceneObject *ASceneObject) {Q_UNUSED(ASceneObject)}
	virtual void mouseHoverLeave(SceneObject *ASceneObject) {Q_UNUSED(ASceneObject)}
	virtual void objectUpdated(SceneObject *ASceneObject, int ARole=MDR_NONE);
	virtual bool mouseClicked(SceneObject *ASceneObject, Qt::MouseButton AButton) {Q_UNUSED(ASceneObject) Q_UNUSED(AButton) return false;}
	virtual bool mouseDoubleClicked(SceneObject *ASceneObject, Qt::MouseButton AButton) {Q_UNUSED(ASceneObject) Q_UNUSED(AButton) return false;}
	virtual bool contextMenu(SceneObject *ASceneObject, QMenu *AMenu) {Q_UNUSED(ASceneObject) Q_UNUSED(AMenu) return NULL;}
	virtual QString toolTipText(const MapObject *AMapObject, const QHelpEvent *AEvent) const {Q_UNUSED(AMapObject) Q_UNUSED(AEvent) return QString();}
	virtual bool isSticky(const SceneObject *ASceneObject) const {Q_UNUSED(ASceneObject) return false;}

    //IMapObjectDataHolder
	virtual QGraphicsItem *mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement);    // Get map data

    //IMapMouseGrabber
    virtual bool mapMouseMoved(QPointF ANewPosition, QPointF AOldPosition, MercatorCoordinates ACoordinates, Qt::MouseButtons AMouseButtons);
    virtual bool mapMouseClicked(MercatorCoordinates ACoordinates, Qt::MouseButton button);
    virtual bool mapMouseDoubleClicked(MercatorCoordinates ACoordinates, Qt::MouseButton AButton);
    virtual bool mapMouseWheelMoved(const QPointF &AScenePosition, int ADelta);
	virtual bool mapContextMenu(MercatorCoordinates ACoordinates, Menu *AMenu);


    //IMapLocationSelector
    virtual bool selectLocation(ILocationSelector *ALocationSelector);
    virtual bool finishSelectLocation(ILocationSelector *ALocationSelector, bool ACancel);
    virtual const MercatorCoordinates &selectedLocation() const;
    
public slots:
    //IMapObjectDataHolder
    virtual void onMapObjectInserted(int AType, const QString &AId);    // SLOT: Map object inserted
    virtual void onMapObjectRemoved(int AType, const QString &AId);     // SLOT: Map object removed
    void onMapObjectShowed(int AType, const QString &AId){Q_UNUSED(AType) Q_UNUSED(AId)}  // SLOT: Map object showed

signals:
    //IMapObjectDataHolder
    void mapDataChanged(int AType, const QString &AId, int ARole);   // SIGNAL: Map data changed

    //IMapLocationSelector
    void locationSelected();                                // SIGNAL: Location selected
    void locationSelectionCancelled();                      // SIGNAL: Location selection cancelled

private:
	MercatorCoordinates	FSelectedLocation;
	IMap				*FMap;
	GeoMap				*FGeoMap;
	ILocationSelector	*FLocationSelector;
};

#endif // MAPLOCATIONSELECTOR_H
