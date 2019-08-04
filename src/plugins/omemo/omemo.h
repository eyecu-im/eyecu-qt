#ifndef OMEMO_H
#define OMEMO_H

#include <interfaces/iomemo.h>
#include <interfaces/ixmppstreammanager.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipepmanager.h>
#include <interfaces/iaccountmanager.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/istanzaprocessor.h>

class SignalProtocol;

class Omemo: public QObject,
			 public IPlugin,
			 public IOmemo,
			 public IOptionsDialogHolder,
			 public IPEPHandler,
			 public IStanzaHandler,
			 public IStanzaRequestOwner
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IOmemo IOptionsDialogHolder IPEPHandler IStanzaHandler IStanzaRequestOwner)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IOmemo")
#endif
public:
	Omemo();
	~Omemo() override;

	SignalProtocol *signalProtocol(const Jid &AStreamJid);

	//IPlugin
	virtual QObject *instance() override { return this; }
	virtual QUuid pluginUuid() const override { return OMEMO_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo) override;
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) override;
	virtual bool initObjects() override;
	virtual bool initSettings() override;
	virtual bool startPlugin() override { return true; }

	// IOptionsDialogHolder
	QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

	// IPEPHandler
	virtual bool processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza) override;

	//IStanzaHandler
	virtual bool stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept) override;

	// IStanzaRequestOwner interface
	void stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza) override;

protected:
	bool isSupported(const QString &ABareJid) const;
	bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
	int isSupported(const IMessageAddress *AAddresses) const;
	bool setActiveSession(const Jid &AStreamJid, const QString &ABareJid, bool AActive=true);
	bool isActiveSession(const Jid &AStreamJid, const QString &ABareJid) const;
	bool isActiveSession(const IMessageAddress *AAddresses) const;
	void registerDiscoFeatures();
	void updateChatWindowActions(IMessageChatWindow *AWindow);
	bool publishOwnDeviceIds(const Jid &AStreamJid);
	bool publishOwnKeys(const Jid &AStreamJid);
	bool removeOtherKeys(const Jid &AStreamJid);

	QString requestDeviceBundle(const Jid &AStreamJid, const QString &ABareJid, quint32 ADevceId);

	void bundlesProcessed(const QString &ABareJid);
	void encryptMessage(Stanza &AMessageStanza);

	struct SignalDeviceBundle
	{
		quint32 FDeviceId;
		quint32 FSignedPreKeyId;
		QByteArray FSignedPreKeyPublic;
		QByteArray FSignedPreKeySignature;
		QByteArray FIdentityKey;
		QMap<quint32, QByteArray> FPreKeys;
	};

protected slots:
	void onOptionsOpened();

	void onPresenceOpened(IPresence *APresence);
	void onPresenceClosed(IPresence *APresence);
	void onPepTimeout();

	void onChatWindowCreated(IMessageChatWindow *AWindow);
	void onNormalWindowCreated(IMessageNormalWindow *AWindow);
	void onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);

	void onAccountInserted(IAccount *AAccount);
	void onAccountRemoved(IAccount *AAccount);
	void onAccountDestroyed(const QUuid &AAccountId);

	void onUpdateMessageState(const Jid &AStreamJid, const Jid &AContactJid);
	void onOmemoActionTriggered();

private:
	IAccountManager*	FAccountManager;
	IOptionsManager*	FOptionsManager;
	IPEPManager*		FPepManager;
	IStanzaProcessor*	FStanzaProcessor;
	IXmppStreamManager*	FXmppStreamManager;
	IPresenceManager*	FPresenceManager;
	IServiceDiscovery*	FDiscovery;
	IMessageWidgets*	FMessageWidgets;
	IPluginManager*		FPluginManager;

	IconStorage*		FIconStorage;
	int					FOmemoHandlerIn;
	int					FOmemoHandlerOut;

	int					FSHIMessageIn;
	int					FSHIMessageOut;

	QHash<Jid, SignalProtocol*> FSignalProtocols;

	QHash<IXmppStream *, QTimer*> FPepDelay;
	QHash<QString, QList<quint32> > FDeviceIds;
	QHash<Jid, QStringList> FActiveSessions;
	QHash<QString, quint32> FBundleRequests; // Stanza ID, device ID
	QMultiHash<QString, quint32> FPendingRequests;	// Bare JID, Device ID
	QMultiHash<QString, SignalDeviceBundle> FBundles;
	QMultiHash<QString, Stanza> FPendingMessages;

	bool				FCleanup;
};

#endif // OMEMO_H
