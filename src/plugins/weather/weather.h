#ifndef WEATHER_H
#define WEATHER_H

#include <interfaces/iweather.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/iconnectionmanager.h>
#include <interfaces/imap.h>
#include <interfaces/ipoi.h>

class Weather : public QObject,
                public IPlugin,
				public MapSceneObjectHandler,
				public MapObjectDataHolder
{
    Q_OBJECT
	Q_INTERFACES (IPlugin MapSceneObjectHandler MapObjectDataHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IWeather")
#endif
public:
    explicit Weather(QObject *parent = 0);

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return WEATHER_UUID;}
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    //IOptionsHolder
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

protected:
    //! This
	bool contextMenu(const QString &AId, QMenu *AMenu);
    //IPlaceViewProvider
    void showCenterView(int AType, QString AId, int ARole, double ALat, double ALng);
    void statrViewer(double ALat, double ALng);

public slots:
    //IMapObjectDataHolder
    void onMapObjectInserted(int AType, const QString &AId);   // SLOT: Map object inserted
    void onMapObjectRemoved(int AType, const QString &AId);    // SLOT: Map object removed
    void onMapObjectShowed(int AType, const QString &AId);  // SLOT: Map object showed

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
    QHash <QUuid, IWeatherProvider *> FWeatherProviders;

};

#endif // WEATHER_H
