#ifndef OTRPLUGIN_H
#define OTRPLUGIN_H

#include <QMultiMap>

#include <interfaces/ipluginmanager.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/istanzaprocessor.h>
#include <interfaces/ixmppstreammanager.h>
#include <interfaces/imessagearchiver.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessagestylemanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/iotr.h>

#include <definitions/menuicons.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>

#include <utils/message.h>

class IAccountManager;
class IMessageProcessor;
class Action;

class OtrClosure;
class OtrInternal;

/**
 * This struct contains all data shown in the table of 'Known Fingerprints'.
 */
struct OtrFingerprint
{
	/**
	 * Pointer to fingerprint in libotr struct. Binary format.
	 */
	unsigned char* fingerprint;

	/**
	 * own account
	 */
	QString account;

	/**
	 * owner of the fingerprint
	 */
	QString username;

	/**
	 * The fingerprint in a human-readable format
	 */
	QString fingerprintHuman;

	/**
	 * the level of trust
	 */
	QString trust;

	OtrFingerprint();
	OtrFingerprint(const OtrFingerprint &fp);
	OtrFingerprint(unsigned char* fingerprint,
				QString account, QString username,
				QString trust);
};

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
	bool isVerified(const QString& AAccount, const QString& AContact);

	/**
	 * Get hash of fingerprints of own private keys.
	 * Account -> KeyFingerprint
	 */
	QHash<QString, QString> getPrivateKeys();

	/**
	 * Return the active fingerprint for a context.
	 */
	OtrFingerprint getActiveFingerprint(const QString& AAccount,
										const QString& AContact);
	/**
	 * Start the SMP with an optional question.
	 */
	void startSMP(const QString& AAccount, const QString& AContact,
				  const QString& AQuestion, const QString& ASecret);

	/**
	 * Continue the SMP.
	 */
	void continueSMP(const QString& AAccount, const QString& AContact,
					 const QString& ASecret);

	/**
	 * Abort the SMP.
	 */
	void abortSMP(const QString& AAccount, const QString& AContact);

	/**
	 * Return the messageState of a context,
	 * i.e. plaintext, encrypted, finished.
	 */
	IOtr::MessageState getMessageState(const QString& AAccount,
									const QString& AContact);

	/**
	 * Set fingerprint verified/not verified.
	 */
	void verifyFingerprint(const OtrFingerprint& AFingerprint, bool AVerified);

	/**
	 * Return true if Socialist Millionaires' Protocol succeeded.
	 */
	bool smpSucceeded(const QString& AAccount, const QString& AContact);

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
	void deleteKey(const QString& AAccount);

	/**
	 * Return the messageState as human-readable string.
	 */
	QString getMessageStateString(const QString& AAccount,
								  const QString& AContact);
	/**
	 * Generate own keys.
	 * This function blocks until keys are available.
	 */
	void generateKey(const QString& AAccount);

	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return OTR_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin() { return true; }
    //IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
	//IArchiveHandler
	virtual bool archiveMessageEdit(int AOrder, const Jid &AStreamJid, Message &AMessage, bool ADirectionIn);
	//IStanzaHandler
	virtual bool stanzaReadWrite(int AHandlerId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);

    // OtrCallback
    virtual QString dataDir();
	virtual void sendMessage(const QString &AAccount, const QString &AContact,
							 const QString& AMessage);
    virtual bool isLoggedIn(const QString &AAccount, const QString &AContact) const;
	virtual void notifyUser(const QString &AAccount, const QString &AContact,
							const QString& AMessage, const NotifyType& AType);

	virtual bool displayOtrMessage(const QString &AAccount, const QString &AContact,
								   const QString& AMessage);
	virtual void stateChange(const QString &AAccount, const QString &AContact,
							 StateChange AChange);

	virtual void receivedSMP(const QString &AAccount, const QString &AContact,
							 const QString& AQuestion);
	virtual void updateSMP(const QString &AAccount, const QString &AContact,
						   int AProgress);

	virtual QString humanAccount(const QString& AAccountId);
	virtual QString humanAccountPublic(const QString& AAccountId);
	virtual QString humanContact(const QString& AAccountId, const QString &AContactJid);
	virtual void authenticateContact(const QString &AAccount, const QString &AContact);

protected:
	void notifyInChatWindow(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessage) const;

	//
	// Former OtrMessaging class methods
	//
	/**
	 * Process an outgoing message.
	 *
	 * @param account Account the message is send from
	 * @param contact Recipient of the message
	 * @param message The message itself
	 *
	 * @return The encrypted message
	 */
	QString encryptMessage(const QString& AAccount,
						   const QString& AContact,
						   const QString& AMessage);

	/**
	 * Decrypt an incoming message.
	 *
	 * @param account Account the message is send to
	 * @param contact Sender of the message
	 * @param message The message itself
	 * @param decrypted The decrypted message if the original message was
	 *                  encrypted
	 * @return Type of incoming message
	 */
	IOtr::MessageType decryptMessage(const QString& AAccount, const QString& AContact,
									 const QString& AMessage, QString& ADecrypted);

	/**
	 * Send an OTR query message from account to contact.
	 */
	void startSession(const QString& AAccount, const QString& AContact);

	/**
	 * Send otr-finished message to user.
	 */
	void endSession(const QString& AAccount, const QString& AContact);

	/**
	 * Force a session to expire.
	 */
	void expireSession(const QString& AAccount, const QString& AContact);

	/**
	 * Return the secure session id (ssid) for a context.
	 */
	QString getSessionId(const QString& AAccount, const QString& AContact);

protected slots:
	void onStreamOpened(IXmppStream *AXmppStream);
	void onStreamClosed(IXmppStream *AXmppStream);
	void onToolBarWidgetCreated(IMessageToolBarWidget *AWidget);
	void onMessageWindowCreated(IMessageNormalWindow *AWindow);
	void onMessageWindowDestroyed(IMessageNormalWindow *AWindow);
	void onChatWindowCreated(IMessageChatWindow *AWindow);
	void onChatWindowDestroyed(IMessageChatWindow *AWindow);
	void onPresenceOpened(IPresence *APresence);
	void onProfileOpened(const QString &AProfile);

	// OTR tool button
	void onSessionInitiate(bool b);
	void onSessionEnd(bool b);
	void onContactAuthenticate(bool b);
	void onSessionID(bool b);
	void onFingerprint(bool b);

	void onWindowAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);
	void onUpdateMessageState(const Jid &AStreamJid, const Jid &AContactJid);

signals:
	void otrStateChanged(const Jid &AStreamJid, const Jid &AContactJid) const;

private:
	OtrInternal* FOtrInternal;

	QHash<QString, QHash<QString, OtrClosure*> > FOnlineUsers;
	IOptionsManager* FOptionsManager;
	IStanzaProcessor *FStanzaProcessor;
	IMessageArchiver *FMessageArchiver;
	IAccountManager* FAccountManager;
	IPresenceManager *FPresenceManager;
	IMessageProcessor* FMessageProcessor;
    QString         FHomePath;
	QHash<IMessageToolBarWidget*, Action*> FActions;
	QHash<Action*, QToolButton*> FButtons;
	IMessageWidgets *FMessageWidgets;
	int				FSHIMessage;
	int				FSHIPresence;
	int				FSHOMessage;
	int				FSHOPresence;
};

#endif //OTRPLUGIN_H
