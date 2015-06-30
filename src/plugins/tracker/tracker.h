#ifndef TRACKER_H
#define TRACKER_H

#include <QObject>

#include <interfaces/itracker.h>
#include <interfaces/igeoloc.h>
#include <interfaces/imap.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/irostermanager.h>

#include <definitions/menuicons.h>
#include <definitions/optionnodes.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/resources.h>

#include "trackoptions.h"

#define MDT_TRAC_ICON 850
#define MDT_TRAC_TEXT 800


class Tracker: public QObject,
               public IPlugin,
               public ITracker,
			   public MapSceneObjectHandler,
			   public MapObjectDataHolder,
			   public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES(IPlugin ITracker MapSceneObjectHandler MapObjectDataHolder IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.ITracker")
#endif
public:
    Tracker();
    ~Tracker();
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return TRACKER_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    //IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

    //IMapObjectDataHolder
	virtual QGraphicsItem * mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement);

    //ISceneObjectHandler
	virtual void mouseHoverEnter(SceneObject *ASceneObject) {Q_UNUSED(ASceneObject)}
	virtual void mouseHoverLeave(SceneObject *ASceneObject) {Q_UNUSED(ASceneObject)}
    virtual bool mouseHit() const {return true; }
	virtual bool mouseClicked(SceneObject *ASceneObject, Qt::MouseButton AButton) {Q_UNUSED(ASceneObject) Q_UNUSED(AButton) return false;}
	virtual bool mouseDoubleClicked(SceneObject *ASceneObject, Qt::MouseButton AButton) {Q_UNUSED(ASceneObject) Q_UNUSED(AButton) return false;}
	virtual bool contextMenu(SceneObject *ASceneObject, QMenu *AMenu) {Q_UNUSED(ASceneObject) Q_UNUSED(AMenu) return false;}
    virtual float zValue() const {return 5.0;}
	virtual void objectUpdated(SceneObject *ASceneObject, int ARole=MDR_NONE);
	virtual QString toolTipText(const MapObject *AMapObject, const QHelpEvent *AEvent) const {Q_UNUSED(AMapObject) Q_UNUSED(AEvent) return QString();}
	virtual bool isSticky(const SceneObject *ASceneObject) const {Q_UNUSED(ASceneObject) return false;}

    virtual QIcon getIcon(const QString locName) const;
    virtual QString getIconFileName(const QString locName) const;

public slots:
    void onLocationReceived(const Jid &AStreamJid, const Jid &AContactJid, const MercatorCoordinates &ACoordinates, bool AReliabilityChanged);
    void rosterOpened(IRoster *ARoster);
    void rosterClosed(IRoster *ARoster);

    // IMapObjectDataHolder
    void onMapObjectInserted(int AType, const QString &AId);         // SLOT: Map object inserted
    void onMapObjectRemoved(int AType, const QString &AId);         // SLOT: Map object removed
    void onMapObjectShowed(int AType, const QString &AId){Q_UNUSED(AType) Q_UNUSED(AId)}  // SLOT: Map object showed

protected slots:
    void onOptionsOpened();
    void onOptionsClosed();
    void onOptionsChanged(const OptionsNode &ANode);

signals:
    //IMapObjectDataHolder
    void mapDataChanged(int AType, const QString &AId, int ARole);

private:
	IOptionsManager	*FOptionsManager;
	IGeoloc			*FGeoloc;
	IconStorage		*FIconStorage;
	IRosterManager	*FRosterManager;
};

#endif // TRACKER_H
