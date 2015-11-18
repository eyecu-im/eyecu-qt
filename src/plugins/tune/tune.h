#ifndef TUNE_H
#define TUNE_H

#include <QDir>
#include <QThread>

#include <interfaces/itune.h>
#include <interfaces/imap.h>
#include <interfaces/ipepmanager.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/irostersview.h>
#include <interfaces/irostersmodel.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/ixmppstreammanager.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessagestylemanager.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/imapcontacts.h>
#include <interfaces/iaccountmanager.h>
#include <interfaces/iconnectionmanager.h>

class UrlRequest: public QThread
{
public:
    UrlRequest(const QUrl &AUrl);
    void run();

private:
    QUrl FUrl;
};

class Tune: public QObject,
            public IPlugin,
            public ITune,
			public IOptionsDialogHolder,
            public IPEPHandler,
			public IRostersLabelHolder,
			public MapObjectDataHolder,
            public IRostersClickHooker
{
    Q_OBJECT
	Q_INTERFACES(IPlugin ITune IOptionsDialogHolder IPEPHandler IRostersLabelHolder MapObjectDataHolder IRostersClickHooker)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.ITune")
#endif
    enum TuneInfoStatus
    {
        NotRequested,
        Requested,
        Recieved,
        Error
    };

public:
    Tune();
    ~Tune();

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return TUNE_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    //IPEPHandler
    virtual bool processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza);

	//IRostersLabelHolder
	QList<quint32> rosterLabels(int AOrder, const IRosterIndex *AIndex) const;
	AdvancedDelegateItem rosterLabel(int AOrder, quint32 ALabelId, const IRosterIndex *AIndex) const;

    //IMapDataHolder
	virtual QGraphicsItem *mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement);

    //IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

    //IRostersClickHooker
    virtual bool rosterIndexSingleClicked(int AOrder, IRosterIndex *AIndex, const QMouseEvent *AEvent);
	virtual bool rosterIndexDoubleClicked(int AOrder, IRosterIndex *AIndex, const QMouseEvent *AEvent){Q_UNUSED(AOrder) Q_UNUSED(AIndex) Q_UNUSED(AEvent) return false;}

    // Other
    virtual void updateRosterLabels(const Jid &AContactJid);
    virtual QIcon getIcon() const;
    virtual QString getIconFileName() const;

public slots:
    void onMapObjectInserted(int AType, const QString &AId);         // SLOT: Map object inserted
    void onMapObjectRemoved(int AType, const QString &AId);         // SLOT: Map object removed
    void onMapObjectShowed(int AType, const QString &AId){Q_UNUSED(AType) Q_UNUSED(AId)}  // SLOT: Map object showed

protected:
    bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;    
    QString getLabel(const Jid &AContactJid) const;
    QString getLabel(const TuneData &ATuneData) const;

    void registerDiscoFeatures();
	void updateChatWindows(bool AInfoBar);
	void updateChatWindows(const Jid &AContactJid, const Jid &AStreamJid, bool AAddMessage);
	void updateChatWindow(IMessageChatWindow *AMessageChatWindow);
	void updateChatWindowInfo(IMessageChatWindow *AMessageChatWindow);
    void sendTune(const TuneData &ATuneData, const Jid &AStreamJid) const;
    void displayNotification(const Jid &AStreamJid, const Jid &AContactJid);
    void removeNotifiedMessages(IMessageChatWindow *AWindow);
    IPresenceItem presenceItemForBareJid(const Jid &AStreamJid, const Jid &AContactJid) const;
    QString currentItemId(const Jid &AStreamJid) const;
    QString tuneInfo(const QString &AKey, const QString &AArtist, const QString &AAlbum = QString(), const QString &ATrack = QString(), bool FDetailed = false) const;
    TuneInfoStatus tuneInfoStatus(const QString &AArtist, const QString &AAlbum = QString(), const QString &ATrack = QString()) const;
    void requsetTuneInfo(const QString &AArtist, const QString &AAlbum, const QString &ATrack = QString());
    QDomElement findElement(const QString &AArtist, const QString &AAlbum = QString(), const QString &ATrack = QString()) const;
    QDomElement createElement(const QString &AArtist, const QString &AAlbum, const QString &ATrack);
	void publishCurrentTune(bool ARetract = false) const;
	void notifyCurrentTune() const;
    void saveTuneInfoCache() const;
    void openUrl(const QUrl &AUrl) const;

    static QString lengthString(const quint16 &ASeconds);

protected slots:
    void onStreamOpened(IXmppStream *AXmppStream);
    void onStreamClosed(IXmppStream *AXmppStream);
    void onRosterIndexInserted(IRosterIndex *AIndex);
    void onRosterIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32, Menu *AMenu);
	void onRosterIndexClipboardMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu);
    void onRosterIndexToolTips(IRosterIndex *AIndex, quint32 ALabelId, QMap<int,QString> &AToolTips);
	void onCopyToClipboard();
    void onOptionsOpened();
    void onOptionsChanged(const OptionsNode &ANode);
    void onChatWindowCreated(IMessageChatWindow *AWindow);
    void onWindowActivated();
    void onNotificationActivated(int ANotifyId);    
    void onResultReceived(const QByteArray &AResult, const QString &AArtist, const QString &AAlbum);
    void onTuneInfoReceived(const QString &AArtist, const QString &AAlbum, const QString &ATrack, const QHash<QString, QString> &ATuneInfo);
    void onTuneActionTriggered() const;
	void onPublishUserTuneTriggered(bool APublish) const;
	void onPresenceOpened(IPresence *APresence);

    void onPlaying(const TuneData &ATuneData);
    void onStopped();
    void onClearCache();

signals:
    //IMapDataHolder
    void mapDataChanged(int AType, const QString &AId, int ARole);
	//IRostersLabelHolder
	void rosterLabelChanged(quint32 ALabelId, IRosterIndex *AIndex);

private:
	IOptionsManager		*FOptionsManager;
	IMessageProcessor	*FMessageProcessor;
	IAccountManager		*FAccountManager;
	IPEPManager			*FPEPManager;
	IServiceDiscovery	*FDiscovery;
	IRostersViewPlugin	*FRostersViewPlugin;
	IPresenceManager	*FPresenceManager;
	IRostersModel		*FRostersModel;
	IXmppStreamManager	*FXmppStreamManager;
	IMessageWidgets		*FMessageWidgets;
	IMessageStyleManager *FMessageStyleManager;
	IMap				*FMap;
	IMapContacts		*FMapContacts;
	INotifications		*FNotifications;
	IConnectionManager	*FConnectionManager;
    QNetworkAccessManager *FNetworkAccessManager;

	IconStorage			*FIconStorage;
	QSet<Jid>			FStreamsOnline;
    QHash<QString, TuneData > FTuneHash;
	QHash<Jid, QString>	FIdHash;
	QTimer				FPollingTimer;
	bool				FPollingPlugins;
	bool				FListenersFound;
	bool				FSimpleContactsView;
	quint32				FRosterLabelId;
	const QList<int>	FRosterIndexKinds;
    QHash<Jid, QHash<Jid, int> > FNotifies;
	TuneData			FCurrentTuneData;
    QHash<QUuid, ITuneInfoRequester *> FRequesters;
	ITuneInfoRequester	*FCurrentRequester;
	QDir				FCachePath;
	QDomDocument		FTuneInfoCache;
};

#endif // TUNE_H
