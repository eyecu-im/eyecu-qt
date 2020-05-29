#ifndef IOTR_H
#define IOTR_H

#include <QObject>
#include <utils/jid.h>

#define OTR_UUID "{8592e3c3-ef5e-42a9-91c9-faf1ed9a91c5}"

/**
 * Interface for callbacks from libotr to application
 */
class IOtr
{
public:
	enum NotifyType
	{
		NotifyInfo,
		NotifyWarning,
		NotifyError
	};

	enum Policy
	{
		PolicyOff,
		PolicyEnabled,
		PolicyAuto,
		PolicyRequire
	};

	enum MessageType
	{
		MsgTypeNone,
		MsgTypeIgnore,
		MsgTypeOtr
	};

	enum MessageState
	{
		MsgStateUnknown,
		MsgStatePlaintext,
		MsgStateEncrypted,
		MsgStateFinished
	};

	enum StateChange
	{
		StateChangeGoingSecure,
		StateChangeGoneSecure,
		StateChangeGoneInsecure,
		StateChangeStillSecure,
		StateChangeClose,
		StateChangeRemoteClose,
		StateChangeTrust
	};

	virtual QObject *instance() =0;

	/**
	 * Sends a message from the Account account to the user contact.
	 * The method is called from the OtrConnection to send messages
	 * during key-exchange.
	 */
	virtual void sendMessage(const Jid& AStreamJid, const Jid& AContactJid,
							 const QString& AMessage, const QString &AHtml=QString()) = 0;

	virtual bool isLoggedIn(const Jid& AStreamJid, const Jid& AContactJid) const = 0;

	virtual bool displayOtrMessage(const Jid &AStreamJid, const Jid &AContactJid,
								   const QString& AMessage) = 0;

	virtual void stateChange(const Jid& AStreamJid, const Jid& AContactJid,
							 StateChange AChange) = 0;

	virtual void receivedSMP(const Jid& AStreamJid, const Jid& AContactJid,
							 const QString& AQuestion) = 0;

	virtual void updateSMP(const Jid& AStreamJid, const Jid& AContactJid,
						   int AProgress) = 0;

	virtual QString humanAccount(const Jid& AStreamJid) = 0;
	virtual QString humanAccountPublic(const Jid& AStreamJid) = 0;
	virtual QString humanContact(const Jid& AStreamJid, const Jid& AContactJid) = 0;
	virtual void authenticateContact(const Jid& AStreamJid, const Jid& AContactJid) =0;
protected:
	virtual void otrStateChanged(const Jid &AStreamJid, const Jid &AContactJid)=0;
};

Q_DECLARE_INTERFACE(IOtr,"RWS.Plugin.IOtr/1.0")

#endif // IOTR_H
