#ifndef OTRPLUGIN_H
#define OTRPLUGIN_H

#include <QMultiMap>

#include <interfaces/ipluginmanager.h>
#include <interfaces/imessagearchiver.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/istanzaprocessor.h>
#include <interfaces/inotifications.h>
#include <interfaces/iotr.h>

class IMessageArchiver;
class IAccountManager;
class IPresenceManager;
class IMessageProcessor;
class IPresence;
class IXmppStream;

class IMessageToolBarWidget;
class IMessageWidgets;
class IMessageNormalWindow;
class IMessageChatWindow;

class Action;

/**
 * This struct contains all data shown in the table of 'Known Fingerprints'.
 */
struct OtrFingerprint
{
	/**
	 * Pointer to fingerprint in libotr struct. Binary format.
	 */
	unsigned char* FFingerprint;

	/**
	 * own account
	 */
	Jid FStreamJid;

	/**
	 * owner of the fingerprint
	 */
	Jid FContactJid;

	/**
	 * The fingerprint in a human-readable format
	 */
	QString FFingerprintHuman;

	/**
	 * the level of trust
	 */
	QString FTrust;

	OtrFingerprint();
	OtrFingerprint(const OtrFingerprint &AOther);
	OtrFingerprint(unsigned char* AFingerprint, QString AStreamJid,
				   QString AContactJid, QString ATrust);
};

class OtrClosure;
class OtrPrivate;

class Otr:
    public QObject,
	public IPlugin,
	public IOtr,
	public IOptionsDialogHolder,
	public IArchiveHandler,
	public IStanzaHandler
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IOtr IOptionsDialogHolder IArchiveHandler IStanzaHandler)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IOtr")
#endif
public:
	Otr();
	~Otr();

	//
	// Former OtrMessaging class methods
	//
	/**
	 * Return true if the active fingerprint has been verified.
	 */
	bool isVerified(const Jid& AStreamJid, const Jid& AContactJid);

	/**
	 * Get hash of fingerprints of own private keys.
	 * Account -> KeyFingerprint
	 */
	QHash<Jid, QString> getPrivateKeys();

	/**
	 * Return the active fingerprint for a context.
	 */
	OtrFingerprint getActiveFingerprint(const Jid& AStreamJid, const Jid& AContactJid);
	/**
	 * Start the SMP with an optional question.
	 */
	void startSMP(const Jid& AStreamJid, const Jid& AContactJid,
				  const QString& AQuestion, const QString& ASecret);

	/**
	 * Continue the SMP.
	 */
	void continueSMP(const Jid& AStreamJid, const Jid& AContactJid,
					 const QString& ASecret);

	/**
	 * Abort the SMP.
	 */
	void abortSMP(const Jid &AStreamJid, const Jid &AContactJid);

	/**
	 * Return the messageState of a context,
	 * i.e. plaintext, encrypted, finished.
	 */
	IOtr::MessageState getMessageState(const Jid &AStreamJid, const Jid &AContactJid);

	/**
	 * Set fingerprint verified/not verified.
	 */
	void verifyFingerprint(const OtrFingerprint& AFingerprint, bool AVerified);

	/**
	 * Return true if Socialist Millionaires' Protocol succeeded.
	 */
	bool smpSucceeded(const Jid& AStreamJid, const Jid& AContactJid);

	/**
	 * Returns a list of known fingerprints.
	 */
	QList<OtrFingerprint> getFingerprints();

	/**
	 * Delete a known fingerprint.
	 */
	void deleteFingerprint(const OtrFingerprint& AFingerprint);

	/**
	 * Delete a private key.
	 */
	void deleteKey(const Jid& AStreamJid);

	/**
	 * Return the messageState as human-readable string.
	 */
	QString getMessageStateString(const Jid& AStreamJid, const Jid& AContactJid);
	/**
	 * Generate own keys.
	 * This function blocks until keys are available.
	 */
	void generateKey(const Jid &AStreamJid);

	QString dataDir();	// To be used by OtrPrivate class

	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return OTR_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin() { return true; }
    //IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId,
																		QWidget *AParent);
	//IArchiveHandler
	virtual bool archiveMessageEdit(int AOrder, const Jid &AStreamJid, Message &AMessage,
									bool ADirectionIn);
	//IStanzaHandler
	virtual bool stanzaReadWrite(int AHandlerId, const Jid &AStreamJid, Stanza &AStanza,
								 bool &AAccept);

    // OtrCallback
	virtual void sendMessage(const Jid& AStreamJid, const Jid& AContactJid,
							 const QString& AMessage, const QString &AHtml=QString());
	virtual bool isLoggedIn(const Jid& AStreamJid, const Jid& AContactJid) const;

	virtual bool displayOtrMessage(const Jid &AStreamJid, const Jid &AContactJid,
								   const QString& AMessage);
	virtual void stateChange(const Jid &AStreamJid, const Jid &AContactJid,
							 StateChange AChange);

	virtual void receivedSMP(const Jid &AStreamJid, const Jid &AContactJid,
							 const QString& AQuestion);
	virtual void updateSMP(const Jid &AStreamJid, const Jid &AContactJid,
						   int AProgress);

	virtual QString humanAccount(const Jid& AStreamJid);
	virtual QString humanAccountPublic(const Jid& AStreamJid);
	virtual QString humanContact(const Jid& AStreamJid, const Jid &AContactJid);
	virtual void authenticateContact(const Jid& AStreamJid, const Jid& AContactJid);

protected:
	void notifyInChatWindow(const Jid &AStreamJid, const Jid &AContactJid,
							const QString &AMessage) const;
	INotification eventNotify(const QString &ATypeId, const QString &AMessagePopup,
							  const QString &AMessageTooltip, const QString &AIcon,
							  const Jid &AStreamJid, const Jid &AContactJid);
	void removeNotifications(IMessageChatWindow *AWindow);

protected slots:
	void onStreamOpened(IXmppStream *AXmppStream);
	void onStreamClosed(IXmppStream *AXmppStream);
	void onChatWindowCreated(IMessageChatWindow *AWindow);
	void onChatWindowDestroyed(IMessageChatWindow *AWindow);
	void onChatWindowActivated();
//	void onPresenceOpened(IPresence *APresence);
	void onProfileOpened(const QString &AProfile);

	// Notifications
	void onNotificationActivated(int ANotifyId);

	// OTR tool button
	void onSessionInitiate();
	void onSessionEnd();
	void onContactAuthenticate();
	void onSessionID();
	void onFingerprint();

	void onWindowAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);
	void onUpdateMessageState(const Jid &AStreamJid, const Jid &AContactJid);

signals:
	void otrStateChanged(const Jid &AStreamJid, const Jid &AContactJid) const;
	void fingerprintsUpdated() const;
	void privKeyGenerated(const Jid &AStreamJid, const QString &Fingerprint);
	void privKeyGenerationFailed(const Jid &AStreamJid);

private:
	OtrPrivate * const	FOtrPrivate;
	QHash<Jid, QHash<Jid, OtrClosure*> > FOnlineUsers;
	IOptionsManager		*FOptionsManager;
	IStanzaProcessor	*FStanzaProcessor;
	IMessageArchiver	*FMessageArchiver;
	IAccountManager		*FAccountManager;
//	IPresenceManager	*FPresenceManager;
	IMessageProcessor	*FMessageProcessor;
	QString				FHomePath;
	IMessageWidgets		*FMessageWidgets;
	INotifications      *FNotifications;
	QHash<Jid, QHash<Jid, int> > FNotifies;
	IconStorage			*FMenuIcons;
	int					FSHIMessage;
	int					FSHIPresence;
	int					FSHOMessage;
	int					FSHOPresence;
};

#endif //OTRPLUGIN_H
