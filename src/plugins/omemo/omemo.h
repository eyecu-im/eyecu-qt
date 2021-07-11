#ifndef OMEMO_H
#define OMEMO_H

#include <QFlags>

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
#include <interfaces/istanzacontentencryption.h>
#include <interfaces/imainwindow.h>
#include "signalprotocol.h"

class Omemo: public QObject,
			 public IPlugin,
			 public IOmemo,
			 public IOptionsDialogHolder,
			 public IPEPHandler,
			 public IStanzaHandler,
			 public IMessageEditor,
			 public IMessageWriter,
			 public IStanzaRequestOwner,
			 public SignalProtocol::ISessionStateListener
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IOmemo IOptionsDialogHolder IPEPHandler IStanzaHandler IMessageEditor IMessageWriter IStanzaRequestOwner)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IOmemo")
#endif
public:
	enum Support
	{
		SupportNone = 0,
		SupportOld  = 1,
		SupportNew  = 2
	};

	Q_DECLARE_FLAGS(SupportFlags, Support)

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
	QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent) override;

	// IPEPHandler
	virtual bool processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza) override;

	//IStanzaHandler
	virtual bool stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept) override;

	// IMessageEditor interface
	virtual bool messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection) override;

	// IMessageWriter interface
	virtual bool writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang) override;
	virtual bool writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang) override;
	virtual bool writeTextToMessage(int AOrder, QTextDocument *ADocument, Message &AMessage, const QString &ALang) override;

	// IStanzaRequestOwner interface
	void stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza) override;

	// ISessionStateListener interface
	bool onNewKeyReceived(const QString &ABareJid, quint32 ADeviceId, const QByteArray &AKeyData, bool AExists, SignalProtocol *ASignalProtocol) override;
	virtual void onSessionStateChanged(const QString &ABareJid, quint32 ADeviceId, SignalProtocol *ASignalProtocol) override;
	virtual void onSessionDeleted(const QString &ABareJid, quint32 ADeviceId, SignalProtocol *ASignalProtocol) override;
	virtual void onIdentityTrustChanged(const QString &ABareJid, quint32 ADeviceId, const QByteArray &AEd25519Key, bool ATrusted, SignalProtocol *ASignalProtocol) override;

protected:
	struct SignalDeviceBundle
	{
//		quint32 FDeviceId;
		quint32 FSignedPreKeyId;
		QByteArray FSignedPreKeyPublic;
		QByteArray FSignedPreKeySignature;
		QByteArray FIdentityKey;
		QMap<quint32, QByteArray> FPreKeys;
	};

	bool isSupported(const QString &ABareJid) const;
	SupportFlags isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
	int isSupported(const IMessageAddress *AAddresses) const;
	bool setActiveSession(const Jid &AStreamJid, const QString &ABareJid, bool AActive=true);
	bool isActiveSession(const Jid &AStreamJid, const QString &ABareJid) const;
	bool isActiveSession(const IMessageAddress *AAddresses) const;
	void registerDiscoFeatures();
	void updateChatWindowActions(IMessageChatWindow *AWindow);
	void updateOmemoAction(Action *AAction);
	void updateOmemoAction(const Jid &AStreamJid, const Jid &AContactJid);
	bool publishOwnDeviceIds(const Jid &AStreamJid);
	bool publishOwnKeys(const Jid &AStreamJid);
	bool removeOtherKeys(const Jid &AStreamJid);
	void removeOtherDevices(const Jid &AStreamJid);
	void sendOptOutStanza(const Jid &AStreamJid, const Jid &AContactJid);

	QString requestBundles4Devices(const Jid &AStreamJid, const QString &ABareJid, const QList<quint32> &ADevceIds);

	void bundlesProcessed(const Jid &AStreamJid, const QString &ABareJid);
	bool encryptMessage(Stanza &AMessageStanza);

	QString sessionStateIconName(const Jid &AStreamJid, const QString &ABareJid);

	void setImage(IMessageChatWindow *AWindow, const Jid &AStreamJid,
				  const QString &ABareJid, quint32 ADeviceId,
				  const QString &AImage, const QString &ATitle);

	void notifyInChatWindow(const Jid &AStreamJid, const Jid &AContactJid,
							const QString &AMessage, const QString &AIconKey=QString()) const;

	bool processBundles(const QDomElement &AItem, const QString &ABareJid, const Jid &AStreamJid);
	bool processBundlesOld(const QDomElement &AItem, const QString &ABareJid, const Jid &AStreamJid);

protected slots:
	void onOptionsOpened();
	void onOptionsChanged(const OptionsNode &ANode);
	void purgeDatabases();

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

	void onOptOut(const Jid &AStreamJid, const Jid &AContactJid, const QString &AReasonText);

signals:
	void optOut(const Jid &AStreamJid, const Jid &AContactJid, const QString &AReasonText);

private:
	IAccountManager*	FAccountManager;
	IOptionsManager*	FOptionsManager;
	IPEPManager*		FPepManager;
	IStanzaProcessor*	FStanzaProcessor;
	IStanzaContentEncrytion* FStanzaContentEncrytion;
	IMessageProcessor*	FMessageProcessor;
	IXmppStreamManager*	FXmppStreamManager;
	IPresenceManager*	FPresenceManager;
	IServiceDiscovery*	FDiscovery;
	IMessageWidgets*	FMessageWidgets;
	IMainWindowPlugin*	FMainWindowPlugin;
	IPluginManager*		FPluginManager;

	IconStorage*		FIconStorage;
	int					FOmemoHandlerIn;
	int					FOmemoHandlerOut;
	int					FSHIMessageIn;
	int					FSHIMessageCheck;
	int					FSHIMessageOut;

	QHash<Jid, SignalProtocol*> FSignalProtocols;

	QHash<IXmppStream *, QTimer*> FPepDelay;
	QHash<QString, QList<quint32> > FDeviceIds;
	QHash<QString, QList<quint32> > FFailedDeviceIds;
	QHash<Jid, QStringList> FActiveSessions;
	QHash<Jid, QStringList> FRunningSessions;
	QHash<QString, quint32> FBundleRequests; // Stanza ID, device ID
	QMultiHash<QString, quint32> FPendingRequests;	// Bare JID, Device ID
	QHash<QString, QHash<quint32, SignalDeviceBundle> > FBundles;
	QMultiHash<QString, Stanza> FPendingMessages;
	QMultiHash<QString, QString> FAcceptableElements;

	bool				FCleanup;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Omemo::SupportFlags)

#endif // OMEMO_H
