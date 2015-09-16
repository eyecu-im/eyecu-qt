#ifndef POI_H
#define POI_H

#include <interfaces/ipoi.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/irostersview.h>
#include <interfaces/irostersmodel.h>
#include <interfaces/imainwindow.h>
#include <interfaces/iprivatestorage.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessagestylemanager.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/imap.h>
#include <interfaces/imaplocationselector.h>
#include <interfaces/imapmessage.h>
#include <interfaces/iaccountmanager.h>
#include <definitions/mapobjectdatarole.h>

#include "poilist.h"

typedef QHash<Jid, Menu *> MenuHash, *PMenuHash;
typedef QHash<Jid, GeolocElement> SendPoiHash, *PSendPoiHash;

class MessagePoiList;

class Poi: public QObject,
           public IPlugin,
           public IPoi,
           public IMessageEditor,
           public IMessageWriter,
		   public MapSceneObjectHandler,
		   public MapObjectDataHolder,
           public IMessageViewUrlHandler,
           public IBubbleUrlEventHandler,
		   public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IPoi IMessageEditor IMessageWriter MapSceneObjectHandler MapObjectDataHolder IMessageViewUrlHandler IBubbleUrlEventHandler IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IPoi")
#endif
public:
    Poi();
    ~Poi();

    //IPlugin
    QObject *instance() { return this; }
    QUuid pluginUuid() const { return POI_UUID; }
    void pluginInfo(IPluginInfo *APluginInfo);
    bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    bool initObjects();
    bool initSettings();
    bool startPlugin(){return true;}

    //IOptionsHolder
	QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

    //IMessageEditor
    bool    messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection);

    //IMessageWriter
    void    writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang);
    void    writeTextToMessage(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang);

	//MapObjectDataHolder
	QGraphicsItem *mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement);

	//MapSceneObjectHandler
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

    //IViewUrlHandler
    bool    messageViewUrlOpen(int AOrder, IMessageViewWidget *AWidget, const QUrl &AUrl);

    //IBubbleUrlHandler
    bool    bubbleUrlOpen(int AOrder, const QUrl &AUrl, const Jid &AStreamJid, const Jid &AContactJid);

    //IPoi
    QIcon   getIcon(const QString &AType) const;
    QString getIconFileName(const QString &AType) const;
    QIcon   getTypeIcon(const QString &AType) const;
    QString getTypeIconFileName(const QString &AType) const;
    QString getFullType(const QString &AType, const QString &AClass=QString()) const;
    GeolocElement getPoi(const QString &AId) const;
    bool    putPoi(const QString &AId, const GeolocElement &APoiData, const QString &ABareJid);
    bool    putPoi(const QString &AId, const GeolocElement &APoiData, bool AShow=false);
    void    insertMessagePoi(const Jid &AStreamJid, const Jid &AContactJid, int AMessageType, const GeolocElement &APoi);
    void    showSinglePoi(QString AId);
    void    hideOnePoi(QString AId);    
    bool    removePoi(const QString &AId);
    Action  *addMenuAction(QString text, QString icon, QString keyIcon);
    void    removeMenuAction(Action *action);
	bool    contextMenu(const QString &AId, Menu *AMenu);
    bool    insertPoiShortcut(const QString &AShortcutId);    
    void    setTreeWidgetShortcuts(QTreeWidget *ATreeWidget, bool ATemporary=false);
    QString getCoordString(const GeolocElement &APoiData) const;
    void    poiShow(const QString &APoiId) const;

    // Other
    QStringList getAllTypes() const;
    const QMap<QString, QString> &getTypeMap() const;
    const QHash<QString, QString> &getTranslatedTypes() const;
    Jid     findStreamJid(const QString &bareJid) const;

public slots:
    //IMapObjectDataHolder
    void onMapObjectInserted(int AType, const QString &AId);   // SLOT: Map object inserted
    void onMapObjectRemoved(int AType, const QString &AId);    // SLOT: Map object removed
    void onMapObjectShowed(int AType, const QString &AId){Q_UNUSED(AType) Q_UNUSED(AId)}  // SLOT: Map object showed

protected:
    void    addMenuMap();
    void    deleteMenuMap();
    void    updateOnePoi(QString AId);
    void    updateStreamPoi(const QString &ABareJid);
    void    showAllPoi();
    void    hideAllPoi();    
    void    updateAllPoi();
    QString parsePOI(const Message &AMessage);
    void    addPoiToMessage(Message &AMessage, GeolocElement &element);//---
    bool    loadPoiTypes();
    bool    isEnabled(const QString &ABareJid) const {return FStreamPoi.contains(ABareJid);}
    bool    isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
    bool    isTypeEnabled(QString AType) const;
    void    updateChatWindowActions(IMessageChatWindow *AChatWindow);    

    bool    loadPoiList(const Jid &AStreamJid);
    bool    savePoiList(const Jid &AStreamJid);    
	void    showPoiList(const QSet<QString> &AStreamBareJids);

    bool    poiEdit(QString id, QWidget *AParent);
    bool    poiSave(QString AId, const IAccount *AAccount=NULL, QWidget *AParent=NULL);
    bool    poiDelete(QString AId);
    bool    poiOpenUri(QString AId);
    bool    processPoiAction(int AAction, QString AId, QWidget *AParentWidget);

    void registerDiscoFeatures();
    MessagePoiList *findPoiList(const Jid &AStreamJid, const Jid &AContactJid, int AMessageType) const;
    MessagePoiList *getPoiList(const IMessageNormalWindow *AWindow) const;
    MessagePoiList *getPoiList(const IMessageChatWindow *AWindow) const;
    QString locationString(const QDomElement &ALocation) const;

protected slots:
    //IPrivateStorage
    void onPrivateStorageOpened(const Jid &AStreamJid);
    void onPrivateDataSaved(const QString &AId, const Jid &AStreamJid, const QDomElement &AElement);
    void onPrivateDataLoaded(const QString &AId, const Jid &AStreamJid, const QDomElement &AElement);
    void onPrivateDataChanged(const Jid &AStreamJid, const QString &ATagName, const QString &ANamespace);
    void onPrivateStorageClosed(const Jid &AStreamJid);

    void onInsertPoi(bool);
    void onInsertLocation(bool);
    void onRosterIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu);
    void onOptionsOpened();
    void onOptionsClosed();
    void onOptionsChanged(const OptionsNode &ANode);
    void addNewPoi();
    void onPoiList(bool);
	void onPoiShow();
    void onPoiActionTriggered();
    void showStreamPoi(const QString &ABareJid);
    void hideStreamPoi(const QString &ABareJid);
    void onShortcutActivated(const QString &AId, QWidget *AWidget);

    void onChatWindowCreated(IMessageChatWindow *AWindow);
    void onNormalWindowCreated(IMessageNormalWindow *AWindow);
    void onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);

signals:
    //IMapObjectDataHolder
    void mapDataChanged(int AType, const QString &AId, int ARole);

    //IPoi
    void poisLoaded(const QString &ABareStreamJid, const PoiHash &APoiHash);
    void poisSaved(const QString &ABareStreamJid, const PoiHash &APoiHash);
    void poiModified(const QString &AId, int AType);
    void poisRemoved(const QString &AStreamBareJid);

private:
	IPrivateStorage		*FPrivateStorage;
	IMessageProcessor	*FMessageProcessor;
	IOptionsManager		*FOptionsManager;
	IAccountManager		*FAccountManager;
	IServiceDiscovery	*FDiscovery;
	IMainWindowPlugin	*FMainWindowPlugin;
	IRostersViewPlugin	*FRostersViewPlugin;
	IconStorage			*FIconStorage;
	IMessageWidgets		*FMessageWidgets;
	IMessageStyleManager *FMessageStyleManager;
	IMap				*FMap;
	GeoMap				*FGeoMap;
    IMapLocationSelector *FMapLocationSelector;
	IMapMessage			*FMapMessage;
	Action				*FActionNewPOI;
//	QHash<QSet<QString>, PoiList *> FPoiList;
	QPointer<PoiList>	FPoiList;
    QHash<QString, PoiList *> FPoiInsertLists;

	IconStorage			*FTypeStorage;
	Menu				*FMenuToolbar;

	QStringList			FStreamPoi;
	QList <IAccount *>	FPoiAccounts;
    QHash<QString, qulonglong> FGlCount;
    QHash<QString, PoiHash> FGeolocHash;
	PoiHash				FTempPois;
	QString				FTempPoiStreamJid;

	PoiHash				FExtGeolocHash;
	QList<Action *>		FActions;

	QMap<QString, Jid>	FLoadRequests;
	QMap<QString, Jid>	FSaveRequests;
	QMap<QString, int>	FOpenStreams;

    QMap<QString, QString> FSubTypes;
    QHash<QString, QString> FTranslatedTypes;

	QStringList			FPoiShortcuts;
	QStringList			FPoiFilter;
	QFont				FPoiFont;
};

#endif // POI_H
