#ifndef MAPMAGNIFIER_H
#define MAPMAGNIFIER_H

#include <QAction>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QContextMenuEvent>
#include <MapScene>

#include <interfaces/imap.h>
#include <interfaces/imapmagnifier.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipositioning.h>

class MapMagnifierView: public QGraphicsView
{
    Q_OBJECT
public:
	MapMagnifierView(QGraphicsScene *AScene, const QSize &ASize, QWidget *AParent = 0);

protected:
    // QWidget interface
    bool event(QEvent *AEvent);        
    virtual void paintEvent(QPaintEvent *APaintEvent);
	void contextMenuEvent(QContextMenuEvent *AEvent);
};

class MapMagnifier: public QObject,
                    public IPlugin,
                    public IMapMagnifier,
					public MapSceneObjectHandler,
					public MapObjectDataHolder,
					public IOptionsDialogHolder,
					public IMapMouseGrabber
{
    Q_OBJECT
	Q_INTERFACES (IPlugin IMapMagnifier MapSceneObjectHandler MapObjectDataHolder IOptionsDialogHolder IMapMouseGrabber)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapMagnifier")
#endif
public:
    MapMagnifier(QObject *parent = 0);

    //IPlugin
    QObject *instance() { return this; }
    QUuid pluginUuid() const { return MAPMAGNIFIER_UUID; }
    void pluginInfo(IPluginInfo *APluginInfo);
    bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    bool initObjects();
    bool initSettings();
    bool startPlugin(){return true;}

    //IOptionsHolder
	QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

    //IMapSceneObjectHandler
    virtual bool mouseHit() const {return true;}
    virtual float zValue() const {return 11;}
	virtual void mouseHoverEnter(SceneObject *ASceneObject) {Q_UNUSED(ASceneObject)}
	virtual void mouseHoverLeave(SceneObject *ASceneObject) {Q_UNUSED(ASceneObject)}
	virtual void objectUpdated(SceneObject *ASceneObject, int ARole=MDR_NONE);
	virtual inline bool mouseClicked(SceneObject *ASceneObject, Qt::MouseButton AButton) {Q_UNUSED(ASceneObject) Q_UNUSED(AButton) return false;}
	virtual inline bool mouseDoubleClicked(SceneObject *ASceneObject, Qt::MouseButton AButton) {Q_UNUSED(ASceneObject) Q_UNUSED(AButton) return false;}
	virtual inline bool contextMenu(SceneObject *ASceneObject, QMenu *AMenu) {Q_UNUSED(ASceneObject) Q_UNUSED(AMenu) return false;}
	virtual QString toolTipText(const MapObject *AMapObject, const QHelpEvent *AEvent) const;
	virtual bool isSticky(const SceneObject *ASceneObject) const {Q_UNUSED(ASceneObject) return false;}

    //IMapObjectDataHolder
	QGraphicsItem *mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement);    // Get map data

    //IMapMouseGrabber
    bool mapMouseMoved(QPointF ANewPosition, QPointF AOldPosition, MercatorCoordinates ACoordinates, Qt::MouseButtons AMouseButtons);
    bool mapMouseClicked(MercatorCoordinates ACoordinates, Qt::MouseButton AButton);
    bool mapMouseDoubleClicked(MercatorCoordinates ACoordinates, Qt::MouseButton AButton);
    bool mapMouseWheelMoved(const QPointF &AScenePosition, int ADelta);
	bool mapContextMenu(MercatorCoordinates ACoordinates, Menu *AMenu);

protected:
    void adjustCentralRulers(int ALength);
    void setScaleColor(const QColor &AColor);
    void setCenterMarkerColor(const QColor &AColor);
    void setCenterMarkerVisible(bool AVisible);
	MapScene *mapScene(const SceneObject *ASceneObject) const;
	MapScene *mapScene(const MapObject *AMapObject) const;
	MapScene *mapScene() const;

signals:
    //IMapObjectDataHolder
    void mapDataChanged(int AType, const QString &AId, int ARole);               // SIGNAL: Map data changed

public slots:
    //IMapObjectDataHolder
    virtual void onMapObjectInserted(int AType, const QString &AId);    // SLOT: Map object inserted
    virtual void onMapObjectRemoved(int AType, const QString &AId);     // SLOT: Map object inserted
    virtual void onMapObjectShowed(int AType, const QString &AId){Q_UNUSED(AType) Q_UNUSED(AId)}  // SLOT: Map object showed

protected slots:
    void onMaginifier();
	void onMapObjectAdded(MapObject *AMapObject);
    void onMppChanged(double mpp);
    void onOptionsChanged(const OptionsNode &ANode);
    void onShortcutActivated(const QString &AShortcutId, QWidget *AWidget);

private:
    IMap            *FMap;
    IOptionsManager *FOptionsManager;    

    // Central rulers
    QGraphicsLineItem *_LineX;
    QGraphicsLineItem *_Line1X;
    QGraphicsLineItem *_Line2X;
    QGraphicsLineItem *_LineX1;
    QGraphicsLineItem *_LineX2;

    QGraphicsLineItem *_LineY;
    QGraphicsLineItem *_Line1Y;
    QGraphicsLineItem *_LineY1;
    QGraphicsLineItem *_Line2Y;
    QGraphicsLineItem *_LineY2;
};

#endif // MAPMAGNIFIER_H
