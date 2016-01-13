#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <interfaces/iactivity.h>
#include <interfaces/imap.h>
#include <interfaces/ipepmanager.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/imainwindow.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/irostersview.h>
#include <interfaces/irostersmodel.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/ixmppstreammanager.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessagestylemanager.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/imapcontacts.h>

class ActivityData
{
public:
	explicit ActivityData(QString ANameBasic, QString ANameDetailed=QString(), QString AText=QString());
	ActivityData();
	QString nameBasic;
	QString nameDetailed;
	QString text;

	QString iconFileName() const;
	bool operator == (const ActivityData &AOther) {return nameBasic==AOther.nameBasic &&
														  nameDetailed==AOther.nameDetailed &&
														  text==AOther.text;}
	bool operator != (const ActivityData &AOther) {return !operator == (AOther);}

	bool isEmpty() const {return nameBasic.isEmpty();}
	bool isNull() const {return nameBasic.isNull();}
	void clear();
};

class Activity: public QObject,
				public IPlugin,
				public IActivity,
				public IRostersLabelHolder,
				public IRosterDataHolder,
				public IOptionsDialogHolder,
				public IPEPHandler,
				public MapObjectDataHolder
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IActivity IRostersLabelHolder IRosterDataHolder IOptionsDialogHolder IPEPHandler MapObjectDataHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.Activity")
#endif
public:
	Activity();
	~Activity();

	//IPlugin
	QObject *instance() { return this; }
	QUuid pluginUuid() const { return ACTIVITY_UUID; }
	void pluginInfo(IPluginInfo *APluginInfo);
	bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	bool initObjects();
	bool initSettings();
	bool startPlugin(){return true;}

	//IPEPHandler
	bool processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza);

	//IRostersLabelHolder
	QList<quint32> rosterLabels(int AOrder, const IRosterIndex *AIndex) const;
	AdvancedDelegateItem rosterLabel(int AOrder, quint32 ALabelId, const IRosterIndex *AIndex) const;

	//IRosterDataHolder
	QList<int> rosterDataRoles(int AOrder) const;
	QVariant rosterData(int AOrder, const IRosterIndex *AIndex, int ARole) const;
	bool	setRosterData(int AOrder, const QVariant &AValue, IRosterIndex *AIndex, int ARole);

	//IMapDataHolder
	QGraphicsItem *mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement);

	//IOptionsHolder
	QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
	void	updateDataHolder(const Jid &AContactJid);

	//IActivity
	QIcon	getIcon(const QString &AName) const;
	QIcon	getIcon(const Jid &AContactJid) const;
	QString	getIconFileName(const QString &AActivityName) const;
	QString getIconFileName(const Jid &AContactJid) const;
	QString	getActivityText(const QString &AActivityName) const {return FTranslatedNames.value(AActivityName);}
	QString	getIconName(const Jid &AContactJid) const;
	QString	getText(const Jid &AContactJid) const;
	QString	getLabel(const Jid &AContactJid) const;
	QIcon	getIcon(const ActivityData &AActivity) const;

public slots:
	void	onMapObjectInserted(int AType, const QString &AId);    // SLOT: Map object inserted
	void	onMapObjectRemoved(int AType, const QString &AId);     // SLOT: Map object removed
	void	onMapObjectShowed(int AType, const QString &AId){Q_UNUSED(AType) Q_UNUSED(AId)}  // SLOT: Map object showed

protected:
	QString getIconFileName(const ActivityData &AActivity) const;
	void	setActivityForAccount(Jid AStreamJid);
	void	registerDiscoFeatures();
	void	updateChatWindows(bool AInfoBar);
	void	updateChatWindows(const Jid &AContactJid, const Jid &AStreamJid);
	void	updateChatWindow(IMessageChatWindow *AMessageChatWindow);
	void	updateChatWindowInfo(IMessageChatWindow *AMessageChatWindow);
	void	sendActivity(const ActivityData &AActivityData, const Jid &AStreamJid);
	void	loadActivityList();
	bool	isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
	QString	getLabel(const ActivityData &AActivityData) const;
	void	displayNotification(const Jid &AStreamJid, const Jid &AContactJid);
	void	removeNotifiedMessages(IMessageChatWindow *AWindow);
	IPresenceItem presenceItemForBareJid(const Jid &AStreamJid, const Jid &AContactJid) const;
	QString	currentItemId(const Jid &AStreamJid) const;
	void    loadTextList(const QString &AActivityName);
	void    saveComments(const ActivityData &AActivityData);

protected slots:
	void onStreamOpened(IXmppStream *AXmppStream);
	void onStreamClosed(IXmppStream *AXmppStream);
	void onRosterIndexInserted(IRosterIndex *AIndex);
	void onRosterIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu);
	void onRosterIndexClipboardMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu);
	void onRosterIndexToolTips(IRosterIndex *AIndex, quint32 ALabelId, QMap<int,QString> &AToolTips);
	void onCopyToClipboard();
	void onSetActivityByAction(bool);
	void onShortcutActivated(const QString &AString, QWidget *AWidget);

	void onOptionsOpened();
	void onOptionsChanged(const OptionsNode &ANode);

	void onChatWindowCreated(IMessageChatWindow *AWindow);
	void menuActivity();

	void onWindowActivated();
	void onNotificationActivated(int ANotifyId);

signals:
	//IRosterDataHolder
	void rosterDataChanged(IRosterIndex *AIndex = NULL, int ARole = 0);
	//IMapObjectDataHolder
	void mapDataChanged(int AType, const QString &AId, int ARole);
	//IRostersLabelHolder
	void rosterLabelChanged(quint32 ALabelId, IRosterIndex *AIndex);

private:
	IOptionsManager		*FOptionsManager;
	IMessageProcessor	*FMessageProcessor;
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
	IconStorage			*FActivityIconStorage;
	IconStorage			*FMenuIconStorage;
	bool				FSimpleContactsView;
	quint32				FRosterLabelId;
	QSet<Jid>			FStreamsOnline;
	QHash<QString, ActivityData> FActivityHash;
	QHash<QString, QStringList> FActivityTextLists;
	const QList<int>	FRosterIndexKinds;
	QHash<QString, QString> FIdHash;
	ActivityData        FCurrentActivity;
	QHash<QString, QString>      FTranslatedNames;
	QHash<QString, QStringList>  FActivityList;
	QHash<Jid, QHash<Jid, int> > FNotifies;
};

#endif // ACTIVITY_H
