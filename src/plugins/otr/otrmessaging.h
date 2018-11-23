/*
 * otrmessaging.h - Interface to libotr
 *
 * Off-the-Record Messaging plugin for Psi+
 * Copyright (C) 2007-2011  Timo Engel (timo-e@freenet.de)
 *                    2011  Florian Fieber
 *                    2014  Boris Pek (tehnick-8@mail.ru)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef OTRMESSAGING_H_
#define OTRMESSAGING_H_

#include <QList>
#include <QHash>
#include <QString>

#include <interfaces/iotr.h>
#include <utils/jid.h>

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

// ---------------------------------------------------------------------------

/**
 * This class is the interface to the Off the Record Messaging library.
 * See the libotr documentation for more information.
 */
class OtrMessaging
{
public:

    /**
     * Constructor
     *
     * @param plugin Pointer to the plugin, used for sending messages.
     * @param policy The default OTR policy
     */
	OtrMessaging(IOtr* AOtr);

    /**
     * Deconstructor
     */
    ~OtrMessaging();

	void init();

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
     * Returns a list of known fingerprints.
     */
	QList<OtrFingerprint> getFingerprints();

    /**
     * Set fingerprint verified/not verified.
     */
	void verifyFingerprint(const OtrFingerprint& AFingerprint, bool AVerified);

    /**
     * Delete a known fingerprint.
     */
	void deleteFingerprint(const OtrFingerprint& AFingerprint);

    /**
     * Get hash of fingerprints of own private keys.
     * Account -> KeyFingerprint
     */
    QHash<QString, QString> getPrivateKeys();

    /**
     * Delete a private key.
     */
	void deleteKey(const QString& AAccount);

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
     * Return the messageState as human-readable string.
     */
	QString getMessageStateString(const QString& AAccount,
								  const QString& AContact);

    /**
     * Return the secure session id (ssid) for a context.
     */
	QString getSessionId(const QString& AAccount, const QString& AContact);

    /**
     * Return the active fingerprint for a context.
     */
	OtrFingerprint getActiveFingerprint(const QString& AAccount,
										const QString& AContact);

    /**
     * Return true if the active fingerprint has been verified.
     */
	bool isVerified(const QString& AAccount, const QString& AContact);

    /**
     * Return true if Socialist Millionaires' Protocol succeeded.
     */
	bool smpSucceeded(const QString& AAccount, const QString& AContact);

    /**
     * Set the default OTR policy.
     */
//	void setPolicy(IOtr::Policy APolicy);

    /**
     * Return the default OTR policy.
     */
//	IOtr::Policy policy() const;

    /**
     * Generate own keys.
     * This function blocks until keys are available.
     */
	void generateKey(const QString& AAccount);

    /**
     * Display OTR message.
     */
	bool displayOtrMessage(const QString& AAccount, const QString& AContact,
						   const QString& AMessage);

    /**
     * Report a change of state.
     */
	void stateChange(const QString& AAccount, const QString& AContact,
					 IOtr::StateChange AChange);

    /**
     * Return a human-readable representation
     * of an account identified by accountId.
     */
	QString humanAccount(const QString& AAccountId);

    /**
     * Return the name of a contact.
     */
	QString humanContact(const QString& AAccountId, const QString& AContact);

private:
//	IOtr::Policy FOtrPolicy;
	OtrInternal* FOtrInternal;
	IOtr* FOtr;
};

#endif
