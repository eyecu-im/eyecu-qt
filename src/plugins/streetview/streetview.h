#ifndef STREETVIEW_H
#define STREETVIEW_H

#include <interfaces/istreetview.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/iconnectionmanager.h>
#include <interfaces/irostersview.h>
#include <interfaces/igeoloc.h>
#include <interfaces/imap.h>
#include <interfaces/ipoi.h>

class StreetViewForm;

class StreetView :  public QObject,
					public MapSceneObjectHandler,
					public MapObjectDataHolder,
                    public IPlugin
{
    Q_OBJECT
	Q_INTERFACES (IPlugin MapSceneObjectHandler MapObjectDataHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IStreetView")
#endif
public:
    explicit StreetView(QObject *parent = 0);
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return STREETVIEW_UUID;}
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    ////IOptionsHolder
    //virtual QMultiMap<int, IOptionsWidget *> optionsWidgets(const QString &ANodeId, QWidget *AParent);

    //IMapObjectDataHolder
	QGraphicsItem *mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement);

    //IMapSceneObjectHandler
	void    mouseHoverEnter(SceneObject *ASceneObject);
	void    mouseHoverLeave(SceneObject *ASceneObject);
    bool    mouseHit() const {return true; }
	bool    mouseClicked(SceneObject *ASceneObject, Qt::MouseButton AButton) {Q_UNUSED(ASceneObject) Q_UNUSED(AButton) return false;}
	bool    mouseDoubleClicked(SceneObject *ASceneObject, Qt::MouseButton AButton);
	bool    contextMenu(SceneObject *ASceneObject, QMenu *AMenu);
    float   zValue() const {return 4.0;}
	void    objectUpdated(SceneObject *ASceneObject, int ARole=MDR_NONE);
	QString toolTipText(const MapObject *AMapObject, const QHelpEvent *AEvent) const;
	virtual bool isSticky(const SceneObject *ASceneObject) const {Q_UNUSED(ASceneObject) return false;}

//    bool contextMenu(const QString &AId, Menu *AMenu);
    void showStreetViewMarker(double ALat, double ALng);
    void hideStreetViewMarker();

    void dropped();
    void startStreetView(double ALat, double ALng);

public slots:
    //IMapObjectDataHolder
    void onMapObjectInserted(int AType, const QString &AId);   // SLOT: Map object inserted
    void onMapObjectRemoved(int AType, const QString &AId);    // SLOT: Map object removed
    void onMapObjectShowed(int AType, const QString &AId){Q_UNUSED(AType) Q_UNUSED(AId)}  // SLOT: Map object showed        

protected slots:
    void onStreetViewTriggered();
    void onStreetViewContactTriggered();
    void onStreetViewPoiTriggered();
    void onRosterIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu);

signals:
    //IMapObjectDataHolder
    void mapDataChanged(int AType, const QString &AId, int ARole);

private:
	IMap				*FMap;
	GeoMap				*FGeoMap;
    IPoi                *FPoi;
    IGeoloc             *FGeoloc;
    IOptionsManager     *FOptionsManager;
    IRostersViewPlugin  *FRostersViewPlugin;
    QHash <QUuid, IStreetViewProvider *> FStreetViewProviders;
    StreetViewForm      *FStreetViewForm;
    QHash<QString, PoiHash> FGeolocHash;
    PoiHash             FTempPois;
    bool                FDropped;
};

#endif // STREETVIEW_H
