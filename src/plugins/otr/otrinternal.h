/*
 * otrinternal.h - Manages the OTR connection
 *
 * Off-the-Record Messaging plugin for Psi+
 * Copyright (C) 2007-2011  Timo Engel (timo-e@freenet.de)
 *                    2011  Florian Fieber
 *               2013-2014  Boris Pek (tehnick-8@mail.ru)
 *
 * This program was originally written as part of a diplom thesis
 * advised by Prof. Dr. Ruediger Weis (PST Labor)
 * at the Technical University of Applied Sciences Berlin.
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

#ifndef OTRINTERNAL_H_
#define OTRINTERNAL_H_

#include "otrmessaging.h"

#include <QList>
#include <QHash>

extern "C"
{
#include <proto.h>
#include <message.h>
#include <privkey.h>
#ifndef OTRL_PRIVKEY_FPRINT_HUMAN_LEN
#define OTRL_PRIVKEY_FPRINT_HUMAN_LEN 45
#endif
#include <instag.h>
#include "otrlextensions.h"
}

class QString;

// ---------------------------------------------------------------------------

/**
 * Handles all libotr calls and callbacks.
 */
class OtrInternal
{
public:
	OtrInternal(IOtr* AOtr);

    ~OtrInternal();

	void init();

	QString encryptMessage(const QString& AAccount, const QString& AContact,
						   const QString& AMessage);

	IOtr::MessageType decryptMessage(const QString& AAccount,
                                     const QString& AContact,
                                     const QString& AMessage,
                                     QString& ADecrypted);

	QList<OtrFingerprint> getFingerprints();

	void verifyFingerprint(const OtrFingerprint& AFingerprint, bool AVerified);

	void deleteFingerprint(const OtrFingerprint& AFingerprint);

    QHash<QString, QString> getPrivateKeys();

	void deleteKey(const QString& AAccount);


	void startSession(const QString& AAccount, const QString& AContact);

	void endSession(const QString& AAccount, const QString& AContact);

	void expireSession(const QString& AAccount, const QString& AContact);


	void startSMP(const QString& AAccount, const QString& AContact,
				  const QString& AQuestion, const QString& ASecret);

	void continueSMP(const QString& AAccount, const QString& AContact,
					 const QString& ASecret);

	void abortSMP(const QString& AAccount, const QString& AContact);
	void abortSMP(ConnContext* AContext);


	IOtr::MessageState getMessageState(const QString& AAccount,
										  const QString& AContact);

	QString getMessageStateString(const QString& AAccount,
								  const QString& AContact);

	QString getSessionId(const QString& AAccount, const QString& AContact);

	OtrFingerprint getActiveFingerprint(const QString& AAccount,
										const QString& AContact);

	bool isVerified(const QString& AAccount, const QString& AContact);
	bool isVerified(ConnContext* AContext);

	bool smpSucceeded(const QString& AAccount, const QString& AContact);

	void generateKey(const QString& AAccount);

	static QString humanFingerprint(const unsigned char* AFingerprint);

    /*** otr callback functions ***/
	static OtrlPolicy policy(IOtr::Policy APolicy);
    void createPrivkey(const char* AAccountname, const char* AProtocol);
    int isLoggedIn(const char* AAccountname, const char* AProtocol,
                   const char* ARecipient) const;
    void injectMessage(const char* AAccountname, const char* AProtocol,
						const char* ARecipient, const char* AMessage);
    void updateContextList();
    void newFingerprint(OtrlUserState AUserState, const char* AAccountName,
						 const char* AProtocol, const char* AUsername,
						 unsigned char AFingerprint[20]);
    void writeFingerprints();
    void goneSecure(ConnContext* AContext);
    void goneInsecure(ConnContext* AContext);
    void stillSecure(ConnContext* AContext, int AIsReply);
    void handleMsgEvent(OtrlMessageEvent AMsgEvent, ConnContext* AContext,
                          const char* AMessage, gcry_error_t AError);
    void handleSmpEvent(OtrlSMPEvent ASmpEvent, ConnContext* AContext,
                        unsigned short AProgressPercent, char* AQuestion);
    void createInstag(const char* AAccountName, const char* AProtocol);
    const char* accountName(const char* AAccount, const char* AProtocol);
    void accountNameFree(const char* AAccountName);

    /*** static otr callback wrapper-functions ***/
    static OtrlPolicy cbPolicy(void* AOpdata, ConnContext* AContext);
    static void cbCreatePrivkey(void* APpdata, const char* AAccountName,
                                const char* AProtocol);
    static int cbIsLoggedIn(void* AOpdata, const char* AAccountName,
                            const char* AProtocol, const char* ARecipient);
	static void cbInjectMessage(void* AOpdata, const char* AAccountname,
								const char* AProtocol, const char* ARecipient,
								const char* AMessage);
    static void cbUpdateContextList(void* AOpdata);
	static void cbNewFingerprint(void* AOpdata, OtrlUserState AUserState,
								 const char* AAccountName, const char* AProtocol,
								 const char* AUserName, unsigned char AFingerprint[20]);
	static void cbWriteFingerprints(void* AOpdata);
	static void cbGoneSecure(void* AOpdata, ConnContext* AContext);
	static void cbGoneInsecure(void* AOpdata, ConnContext* AContext);
	static void cbStillSecure(void* AOpdata, ConnContext* AContext, int AIsReply);
	static void cbHandleMsgEvent(void* AOpdata, OtrlMessageEvent AMsgEvent,
								 ConnContext* AContext, const char* AMessage,
								 gcry_error_t AError);
	static void cbHandleSmpEvent(void* AOpdata, OtrlSMPEvent ASmpEvent,
								 ConnContext* AContext, unsigned short AProgressPercent,
								 char* AQuestion);
	static void cbCreateInstag(void* AOpdata, const char* AAccountName, const char* AProtocol);
	static const char* cbAccountName(void* AOpdata, const char* AAccount, const char* AProtocol);
	static void cbAccountNameFree(void* AOpdata, const char* AAccountName);

private:
    /**
     * The userstate contains keys and known fingerprints.
     */
	OtrlUserState FUserState;

    /**
     * Pointers to callback functions.
     */
	OtrlMessageAppOps FUiOps;

    /**
     * Pointer to a class for callbacks from OTR to application.
     */
	IOtr* FOtr;

    /**
     * Name of the file storing dsa-keys.
     */
	QString FKeysFile;

    /**
     * Name of the file storing instance tags.
     */
	QString FInstagsFile;

    /**
     * Name of the file storing known fingerprints.
     */
	QString FFingerprintFile;

    /**
     * Variable used during generating of private key.
     */
	bool FIsGenerating;
};

// ---------------------------------------------------------------------------

#endif
