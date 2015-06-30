#ifndef PLACEVIEW_H
#define PLACEVIEW_H

#include <interfaces/iplaceview.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/iconnectionmanager.h>
#include <interfaces/imap.h>
#include <interfaces/ipoi.h>

class PlaceViewForm;

class PlaceView :   public QObject,
                    //public IOptionsHolder,
					public MapSceneObjectHandler,
					public MapObjectDataHolder,
                    public IPlugin
{
    Q_OBJECT
	Q_INTERFACES (IPlugin MapSceneObjectHandler MapObjectDataHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IPlaceView")
#endif
public:
    explicit PlaceView(QObject *parent = 0);
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return PLACEVIEW_UUID;}
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    ////IOptionsHolder
    //virtual QMultiMap<int, IOptionsWidget *> optionsWidgets(const QString &ANodeId, QWidget *AParent);

    //IMapObjectDataHolder
	QGraphicsItem *mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement);

    //IMapObjectHandler
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

    void closeForm();

protected:
	bool contextMenu(const QString &AId, QMenu *AMenu);
    void showAllPois();
    void putOnePoi(GeolocElement poi);
    void onSaveFile(QString fileName);
	void onCopyClicked(QString fileName);

    //IPlaceViewProvider
    void showCenterView(int AType, QString AId, int ARole, double ALat, double ALng);
    void statrViewer(double ALat, double ALng);
    QPixmap getImageFromFile(const QString &AId);

public slots:
    //IMapObjectDataHolder
    void onMapObjectInserted(int AType, const QString &AId);   // SLOT: Map object inserted
    void onMapObjectRemoved(int AType, const QString &AId);    // SLOT: Map object removed
    void onMapObjectShowed(int AType, const QString &AId){Q_UNUSED(AType) Q_UNUSED(AId)}  // SLOT: Map object showed
    void onPoisReady(void);
    void onPoisDelete();

signals:
    //IMapObjectDataHolder
    void mapDataChanged(int AType, const QString &AId, int ARole);

protected slots:
    void onPlaceViewTriggered();
    void onPlaceViewPoiTriggered();
    void onViewActionTriggered();

private:
	IMap			*FMap;
	GeoMap			*FGeoMap;
	IPoi			*FPoi;
	IOptionsManager	*FOptionsManager;
    QHash <QUuid, IPlaceViewProvider *> FPlaceViewProviders;
	PlaceViewForm	*FPlaceViewForm;
    QHash<QString, GeolocElement >  FPoiMassive;

};

#endif // PLACEVIEW_H
