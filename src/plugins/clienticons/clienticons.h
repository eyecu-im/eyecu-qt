#ifndef CLIENTICONS_H
#define CLIENTICONS_H

#include <QDebug>
#include <definitions/menuicons.h>
#include <definitions/namespaces.h>
#include <definitions/notificationdataroles.h>
#include <definitions/notificationtypeorders.h>
#include <definitions/notificationtypes.h>
#include <definitions/actiongroups.h>
#include <definitions/optionnodes.h>
#include <definitions/optionvalues.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/resources.h>
#include <definitions/shortcuts.h>
#include <definitions/rosterdataholderorders.h>
#include <definitions/rosterindexkinds.h>
#include <definitions/rosterindexroles.h>
#include <definitions/rostertooltiporders.h>
#include <definitions/rosterclickhookerorders.h>
#include <definitions/rosterlabels.h>

#include <interfaces/imainwindow.h>
#include <interfaces/inotifications.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/irostermanager.h>
#include <interfaces/irostersmodel.h>
#include <interfaces/irostersview.h>
#include <interfaces/istanzaprocessor.h>
#include <interfaces/ixmppstreammanager.h>
#include <interfaces/iclienticons.h>
#include <interfaces/iclientinfo.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/iservicediscovery.h>

#include <utils/action.h>
#include <utils/advanceditemdelegate.h>
#include <utils/filestorage.h>
#include <utils/menu.h>
#include <utils/options.h>

class ClientIcons :
	public QObject,
	public IPlugin,
    public IClientIcons,
	public IStanzaHandler,
	public IRosterDataHolder,
	public IRostersLabelHolder,
    public IRostersClickHooker,
	public IOptionsDialogHolder
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IClientIcons IStanzaHandler IRosterDataHolder IRostersLabelHolder IRostersClickHooker IOptionsDialogHolder IRosterDataHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "org.jrudevels.vacuum.IClientIcons")
#endif
public:
	ClientIcons();
	~ClientIcons();
	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return CLIENTICONS_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin() { return true; }
	//IOptionsDialogHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
	//IStanzaHandler
	virtual bool stanzaReadWrite(int AHandlerId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);
	//IRostersLabelHolder
	QList<quint32> rosterLabels(int AOrder, const IRosterIndex *AIndex) const;
	AdvancedDelegateItem rosterLabel(int AOrder, quint32 ALabelId, const IRosterIndex *AIndex) const;
	//IRosterDataHolder
	virtual QList<int> rosterDataRoles(int AOrder) const;
	virtual QVariant rosterData(int AOrder, const IRosterIndex *AIndex, int ARole) const;
	virtual bool setRosterData(int AOrder, const QVariant &AValue, IRosterIndex *AIndex, int ARole);
    //IRostersClickHooker
    virtual bool rosterIndexSingleClicked(int AOrder, IRosterIndex *AIndex, const QMouseEvent *AEvent);
	virtual bool rosterIndexDoubleClicked(int AOrder, IRosterIndex *AIndex, const QMouseEvent *AEvent){Q_UNUSED(AOrder) Q_UNUSED(AIndex) Q_UNUSED(AEvent) return false;}
	//IClientIcons
	virtual quint32 rosterLabelId() const {return FRosterLabelId;}
	virtual QIcon iconByKey(const QString &key) const;
	virtual QString clientByKey(const QString &key) const;
	virtual QString contactClient(const Jid &contactJid) const;
	virtual QIcon contactIcon(const Jid &contactJid) const;

protected slots:
    void onChatWindowCreated(IMessageChatWindow *AWindow);
    void onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);
	//IRostersView
	void onRosterIndexInserted(IRosterIndex *AIndex);
	void onRostersViewIndexToolTips(IRosterIndex *AIndex, quint32 ALabelId, QMap<int, QString> &AToolTips);
	void onRosterIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu);
	//IXmppStreamManager
	void onStreamOpened(IXmppStream *AXmppStream);
	void onStreamClosed(IXmppStream *AXmppStream);
	//IPresencePlugin
	void onContactStateChanged(const Jid &streamJid, const Jid &contactJid, bool AStateOnline);

	void onOptionsOpened();
	void onOptionsChanged(const OptionsNode &ANode);
    void onSoftwareVersionActionTriggered();
protected:
	void updateChatWindows();
	void updateChatWindows(const Jid &AContactJid, const Jid &AStreamJid);
    void updateChatWindow(IMessageChatWindow *AMessageChatWindow);
	//IRosterDataHolder
	void updateDataHolder(const Jid &streamJid, const Jid &clientJid);

signals:
	//IRosterDataHolder
	void rosterDataChanged(IRosterIndex *AIndex = NULL, int ARole = 0);
	//IRostersLabelHolder
	void rosterLabelChanged(quint32 ALabelId, IRosterIndex *AIndex);

private:
	IMainWindowPlugin	*FMainWindowPlugin;
	IPresenceManager	*FPresenceManager;
	IStanzaProcessor	*FStanzaProcessor;
	IXmppStreamManager	*FXmppStreamManager;
	IOptionsManager		*FOptionsManager;
	IRosterManager		*FRosterManager;
	IRostersModel		*FRostersModel;
	IRostersViewPlugin	*FRostersViewPlugin;
	INotifications		*FNotifications;
	IMessageWidgets		*FMessageWidgets;
	IClientInfo			*FClientInfo;
	IServiceDiscovery	*FServiceDiscovery;

private:
	bool				FSimpleContactsView;
	quint32				FRosterLabelId;
	QMap<Jid, int>		FSHIPresence;
	QHash<QString, Client> FClients;
	QHash<Jid, QString>	FContacts;
	const QList<int>	FRosterIndexKinds;
};

#endif // CLIENTICONS_H
