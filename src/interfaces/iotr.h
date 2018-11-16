#ifndef IOTR_H
#define IOTR_H

#include <QObject>
#include <utils/jid.h>

#define OTR_UUID "{8592e3c3-ef5e-42a9-91c9-faf1ed9a91c5}"

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

/**
 * Interface for callbacks from libotr to application
 */
class IOtr
{
public:
	enum OtrNotifyType
	{
		OTR_NOTIFY_INFO,
		OTR_NOTIFY_WARNING,
		OTR_NOTIFY_ERROR
	};

	enum OtrPolicy
	{
		OTR_POLICY_OFF,
		OTR_POLICY_ENABLED,
		OTR_POLICY_AUTO,
		OTR_POLICY_REQUIRE
	};

	enum OtrMessageType
	{
		OTR_MESSAGETYPE_NONE,
		OTR_MESSAGETYPE_IGNORE,
		OTR_MESSAGETYPE_OTR
	};

	enum OtrMessageState
	{
		OTR_MESSAGESTATE_UNKNOWN,
		OTR_MESSAGESTATE_PLAINTEXT,
		OTR_MESSAGESTATE_ENCRYPTED,
		OTR_MESSAGESTATE_FINISHED
	};

	enum OtrStateChange
	{
		OTR_STATECHANGE_GOINGSECURE,
		OTR_STATECHANGE_GONESECURE,
		OTR_STATECHANGE_GONEINSECURE,
		OTR_STATECHANGE_STILLSECURE,
		OTR_STATECHANGE_CLOSE,
		OTR_STATECHANGE_REMOTECLOSE,
		OTR_STATECHANGE_TRUST
	};

	virtual QObject *instance() =0;

	virtual QString dataDir() = 0;

	virtual OtrPolicy policy() const = 0;
	/**
	 * Sends a message from the Account account to the user contact.
	 * The method is called from the OtrConnection to send messages
	 * during key-exchange.
	 */
	virtual void sendMessage(const QString& account, const QString& contact,
							 const QString& message) = 0;

	virtual bool isLoggedIn(const QString& account, const QString& contact) = 0;

	virtual void notifyUser(const QString& account, const QString& contact,
							const QString& message, const OtrNotifyType& type) = 0;

	virtual bool displayOtrMessage(const QString& account, const QString& contact,
								   const QString& message) = 0;

	virtual void stateChange(const QString& account, const QString& contact,
							 OtrStateChange change) = 0;

	virtual void receivedSMP(const QString& account, const QString& contact,
							 const QString& question) = 0;

	virtual void updateSMP(const QString& account, const QString& contact,
						   int progress) = 0;

	virtual QString humanAccount(const QString& accountId) = 0;
	virtual QString humanAccountPublic(const QString& accountId) = 0;
	virtual QString humanContact(const QString& accountId,
								 const QString& contact) = 0;
	virtual void authenticateContact(const QString &account, const QString &contact) =0;
protected:
	virtual void otrStateChanged(const Jid &AStreamJid, const Jid &AContactJid) const =0;
};

Q_DECLARE_INTERFACE(IOtr,"RWS.Plugin.IOtr/1.0")

#endif // IOTR_H
