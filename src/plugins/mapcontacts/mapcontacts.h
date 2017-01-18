#ifndef MAPCONTACTS_H
#define MAPCONTACTS_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/imapcontacts.h>
#include <interfaces/imap.h>
#include <interfaces/irostermanager.h>
#include <interfaces/irostersview.h>
#include <interfaces/irostersmodel.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/istatusicons.h>
#include <interfaces/igeoloc.h>
#include <interfaces/iavatars.h>
#include <interfaces/imessageprocessor.h>

class MapContacts:  public QObject,
					public IPlugin,
					public IMapContacts,
					public IOptionsDialogHolder,
//					public IRostersClickHooker,
					public MapSceneObjectHandler,
					public MapObjectDataHolder
{
	Q_OBJECT
	Q_INTERFACES (IPlugin IMapContacts IOptionsDialogHolder MapSceneObjectHandler MapObjectDataHolder) // IRostersClickHooker
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapContacts")
#endif
public:
	MapContacts(QObject *parent = 0);

	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return MAPCONTACTS_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin(){return true;}

	//IMapObjectDataHolder
	virtual QGraphicsItem *mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement);

	//IMapSceneObjectHandler
	virtual void mouseHoverEnter(SceneObject *ASceneObject);
	virtual void mouseHoverLeave(SceneObject *ASceneObject);
	virtual bool mouseClicked(SceneObject *ASceneObject, Qt::MouseButton AButton);
	virtual bool mouseDoubleClicked(SceneObject *ASceneObject, Qt::MouseButton AButton);
	virtual bool contextMenu(SceneObject *ASceneObject, QMenu *AMenu);
	virtual bool mouseHit() const {return true; }
	virtual float zValue() const {return 4.0;}
	virtual void objectUpdated(SceneObject *ASceneObject, int ARole = MDR_NONE);
	virtual QString toolTipText(const MapObject *AMapObject, const QHelpEvent *AEvent) const;
	virtual bool isSticky(const SceneObject *ASceneObject) const {Q_UNUSED(ASceneObject) return false;}

	//IRostersClickHooker
//	virtual bool rosterIndexSingleClicked(int AOrder, IRosterIndex *AIndex, const QMouseEvent *AEvent);
//	virtual bool rosterIndexDoubleClicked(int AOrder, IRosterIndex *AIndex, const QMouseEvent *AEvent){Q_UNUSED(AOrder) Q_UNUSED(AIndex) Q_UNUSED(AEvent) return false;}

	//IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

	//IMapContacts
	virtual QStringList getFullJidList(const QString &bareJid) const;
	virtual bool hasGeoloc(QString &jid) const;

public slots:
	void showContact(QString AJid, bool AShowMap=true) const;
	void onGeolocActionTriggered() const;

public slots:
	//IMapObjectDataHolder
	void onMapObjectInserted(int AType, const QString &AId);    // SLOT: Map object inserted
	void onMapObjectRemoved(int AType, const QString &AId);     // SLOT: Map object inserted
	void onMapObjectShowed(int AType, const QString &AId);      // SLOT: Map object showed

protected:
	int  getShow(const QString &AJid) const;
	inline bool isOnline(const QString &AJid) const {return isOnline(getShow(AJid));}
	inline bool isOnline(int AShow) const {return (AShow>IPresence::Offline && AShow<IPresence::Invisible);}

	Qt::GlobalColor getColor(const QString &AJid, int AShow) const;
	void updateStatus(MapObject *AContact) const;
	IRosterIndex *getRosterIndex(const QString &AId) const;

	void addMapContact(const Jid &AContactJid, const MercatorCoordinates &ACoordinates, bool AUpdateColor = false);
	void removeMapContact(const Jid &AContactJid);

protected slots:
	void onRosterIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu);
	void onShowContact();
	void onShortcutActivated(const QString &AId, QWidget *AWidget);
	void onRosterOpened(IRoster *ARoster);
	void onRosterClosed(IRoster *ARoster);
	void onOptionsOpened();
	void onOptionsClosed();
	void onOptionsChanged(const OptionsNode &ANode);
	void onIndexDataChanged(IRosterIndex *AIndex, int ARole);
	void onLocationReceived(const Jid &AStreamJid, const Jid &AContactJid, const MercatorCoordinates &ACoordinates, bool AReliabilityChanged);
	void onLocationRemoved(const Jid &AStreamJid, Jid AContactJid);
	void onIndexInserted(IRosterIndex *AIndex);
	void onIndexRemoving(IRosterIndex *AIndex);

signals:
	//IMapObjectDataHolder
	void mapDataChanged(int AType, const QString &AId, int ARole);

	void contactShowedOnTheMap(const QString &AId);

private:
	IMap                *FMap;
	IGeoloc             *FGeoloc;
	IRosterManager		*FRosterManager;
	IRostersView        *FRostersView;
	IRostersModel       *FRostersModel;
	IPresenceManager	*FPresenceManager;
	IStatusIcons        *FStatusIcons;
	IAvatars            *FAvatars;
	IOptionsManager     *FOptionsManager;
	IMessageProcessor   *FMessageProcessor;

//FIXME: Avatar plugin should be used instead
	bool                FShowEmptyAvatars;
	QImage              FEmptyAvatar;
	quint8              FAvatarSize;
	QMultiHash<QString, QString> FResourceHash;
	QMultiHash<IRosterIndex *, QString> FIndexResourceHash;
	QList<IRoster *>    FRosterList;
	QStringList         FContacts;
};

#endif // MAPCONTACTS_H
