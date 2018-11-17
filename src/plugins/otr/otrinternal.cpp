/*
 * otrinternal.cpp - Manages the OTR connection
 *
 * Off-the-Record Messaging plugin for Psi+
 * Copyright (C) 2007-2011  Timo Engel (timo-e@freenet.de)
 *               2011-2012  Florian Fieber
 *                    2013  Georg Rudoy
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

#include <QMessageBox>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QDir>
#include <QFile>
#include <definitions/version.h>
#if QT_VERSION < 0x050000
#include <QtConcurrentRun>
#endif
#include "otrinternal.h"

//-----------------------------------------------------------------------------
static const char*   OTR_PROTOCOL_STRING = "prpl-jabber";
static const QString OTR_FINGERPRINTS_FILE = "otr.fingerprints";
static const QString OTR_KEYS_FILE = "otr.keys";
static const QString OTR_INSTAGS_FILE = "otr.instags";

// ============================================================================

OtrInternal::OtrInternal(IOtr* AOtr, IOtr::Policy& APolicy)
	: FUserState(),
	  FUiOps(),
	  FOtr(AOtr),
	  FOtrPolicy(APolicy),
	  FIsGenerating(false)
{
	QDir profileDir(AOtr->dataDir());

	FKeysFile        = profileDir.filePath(OTR_KEYS_FILE);
	FInstagsFile     = profileDir.filePath(OTR_INSTAGS_FILE);
	FFingerprintFile = profileDir.filePath(OTR_FINGERPRINTS_FILE);

    OTRL_INIT;
	FUserState                 = otrl_userstate_create();
	FUiOps.policy              = (*OtrInternal::cb_policy);
	FUiOps.create_privkey      = (*OtrInternal::cb_create_privkey);
	FUiOps.is_logged_in        = (*OtrInternal::cb_is_logged_in);
	FUiOps.inject_message      = (*OtrInternal::cb_inject_message);
	FUiOps.update_context_list = (*OtrInternal::cb_update_context_list);
	FUiOps.new_fingerprint     = (*OtrInternal::cb_new_fingerprint);
	FUiOps.write_fingerprints  = (*OtrInternal::cb_write_fingerprints);
	FUiOps.gone_secure         = (*OtrInternal::cb_gone_secure);
	FUiOps.gone_insecure       = (*OtrInternal::cb_gone_insecure);
	FUiOps.still_secure        = (*OtrInternal::cb_still_secure);

#if (OTRL_VERSION_MAJOR > 3 || (OTRL_VERSION_MAJOR == 3 && OTRL_VERSION_MINOR >= 2))
	FUiOps.max_message_size    = NULL;
	FUiOps.account_name        = (*OtrInternal::cb_account_name);
	FUiOps.account_name_free   = (*OtrInternal::cb_account_name_free);
#endif

#if (OTRL_VERSION_MAJOR >= 4)
	FUiOps.handle_msg_event    = (*OtrInternal::cb_handle_msg_event);
	FUiOps.handle_smp_event    = (*OtrInternal::cb_handle_smp_event);
	FUiOps.create_instag       = (*OtrInternal::cb_create_instag);
#else
    m_uiOps.log_message         = (*OtrInternal::cb_log_message);

    m_uiOps.notify              = (*OtrInternal::cb_notify);
    m_uiOps.display_otr_message = (*OtrInternal::cb_display_otr_message);

    m_uiOps.protocol_name       = (*OtrInternal::cb_protocol_name);
    m_uiOps.protocol_name_free  = (*OtrInternal::cb_protocol_name_free);
#endif

	otrl_privkey_read(FUserState, QFile::encodeName(FKeysFile).constData());
	otrl_privkey_read_fingerprints(FUserState,
								   QFile::encodeName(FFingerprintFile).constData(),
                                   NULL, NULL);
#if (OTRL_VERSION_MAJOR >= 4)
	otrl_instag_read(FUserState, QFile::encodeName(FInstagsFile).constData());
#endif
}

//-----------------------------------------------------------------------------

OtrInternal::~OtrInternal()
{
	otrl_userstate_free(FUserState);
}

//-----------------------------------------------------------------------------

QString OtrInternal::encryptMessage(const QString& AAccount, const QString& AContact,
									const QString& AMessage)
{
    char* encMessage = NULL;
    gcry_error_t err;

	err = otrl_message_sending(FUserState, &FUiOps, this,
							   AAccount.toUtf8().constData(), OTR_PROTOCOL_STRING,
							   AContact.toUtf8().constData(),
#if (OTRL_VERSION_MAJOR >= 4)
                               OTRL_INSTAG_BEST,
#endif
							   AMessage.toUtf8().constData(),
                               NULL, &encMessage,
#if (OTRL_VERSION_MAJOR >= 4)
                               OTRL_FRAGMENT_SEND_SKIP,
                               NULL,
#endif
                               NULL, NULL);
    if (err)
    {
        QString err_message = QObject::tr("Encrypting message to %1 "
                                          "failed.\nThe message was not sent.")
										  .arg(AContact);
		if (!FOtr->displayOtrMessage(AAccount, AContact, err_message))
        {
			FOtr->notifyUser(AAccount, AContact, err_message, IOtr::NotifyError);
        }
        return QString();
    }

    if (encMessage)
    {
        QString retMessage(QString::fromUtf8(encMessage));
        otrl_message_free(encMessage);

        return retMessage;
    }
	return AMessage;
}


//-----------------------------------------------------------------------------

IOtr::MessageType OtrInternal::decryptMessage(const QString& AAccount,
												 const QString& AContact,
												 const QString& AMessage,
												 QString& ADecrypted)
{
	QByteArray accArray  = AAccount.toUtf8();
	QByteArray userArray = AContact.toUtf8();
    const char* accountName = accArray.constData();
    const char* userName    = userArray.constData();

    int ignoreMessage = 0;
    char* newMessage  = NULL;
    OtrlTLV* tlvs     = NULL;
    OtrlTLV* tlv      = NULL;

	ignoreMessage = otrl_message_receiving(FUserState, &FUiOps, this,
                                           accountName,
                                           OTR_PROTOCOL_STRING,
                                           userName,
										   AMessage.toUtf8().constData(),
                                           &newMessage,
                                           &tlvs, NULL,
#if (OTRL_VERSION_MAJOR >= 4)
                                           NULL,
#endif
                                           NULL);
    tlv = otrl_tlv_find(tlvs, OTRL_TLV_DISCONNECTED);
    if (tlv) {
		FOtr->stateChange(accountName, userName, IOtr::StateChangeRemoteClose);
    }

#if (OTRL_VERSION_MAJOR >= 4)
    // Magic hack to force it work similar to libotr < 4.0.0.
    // If user received unencrypted message he (she) should be notified.
    // See OTRL_MSGEVENT_RCVDMSG_UNENCRYPTED as well.
	if (ignoreMessage && !newMessage && !AMessage.startsWith("?OTR")) {
        ignoreMessage = 0;
    }
#else
    // Check for SMP data (required only with libotr < 4.0.0)
    ConnContext* context = otrl_context_find(m_userstate, userName, accountName,
                                OTR_PROTOCOL_STRING,
                                false, NULL, NULL, NULL);
    if (context) {
        NextExpectedSMP nextMsg = context->smstate->nextExpected;

        if (context->smstate->sm_prog_state == OTRL_SMP_PROG_CHEATED) {
            abortSMP(context);
            // Reset state
            context->smstate->nextExpected  = OTRL_SMP_EXPECT1;
            context->smstate->sm_prog_state = OTRL_SMP_PROG_OK;
            // Report result to user
            m_callback->updateSMP(accountName, userName, -2);
        }
        else
        {
            tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP1Q);
            if (tlv) {
                if (nextMsg != OTRL_SMP_EXPECT1)
                {
                    abortSMP(context);
                }
                else
                {
                    char* question = (char *)tlv->data;
                    char* eoq = static_cast<char*>(memchr(question, '\0', tlv->len));
                    if (eoq) {
                        m_callback->receivedSMP(accountName, userName,
                                                QString::fromUtf8(question));
                    }
                }
            }
            tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP1);
            if (tlv) {
                if (nextMsg != OTRL_SMP_EXPECT1)
                {
                    abortSMP(context);
                }
                else
                {
                    m_callback->receivedSMP(accountName, userName, QString());
                }
            }
            tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP2);
            if (tlv) {
                if (nextMsg != OTRL_SMP_EXPECT2)
                {
                    abortSMP(context);
                }
                else
                {
                    // If we received TLV2, we will send TLV3 and expect TLV4
                    context->smstate->nextExpected = OTRL_SMP_EXPECT4;
                    // Report result to user
                    m_callback->updateSMP(accountName, userName, 66);
                }
            }
            tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP3);
            if (tlv) {
                if (nextMsg != OTRL_SMP_EXPECT3)
                {
                    abortSMP(context);
                }
                else
                {
                    // SMP finished, reset
                    context->smstate->nextExpected = OTRL_SMP_EXPECT1;
                    // Report result to user
                    m_callback->updateSMP(accountName, userName, 100);
                }
            }
            tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP4);
            if (tlv) {
                if (nextMsg != OTRL_SMP_EXPECT4)
                {
                    abortSMP(context);
                }
                else
                {
                    // SMP finished, reset
                    context->smstate->nextExpected = OTRL_SMP_EXPECT1;
                    // Report result to user
                    m_callback->updateSMP(accountName, userName, 100);
                }
            }
            tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP_ABORT);
            if (tlv) {
                // SMP aborted, reset
                context->smstate->nextExpected = OTRL_SMP_EXPECT1;
                // Report result to user
                m_callback->updateSMP(accountName, userName, -1);
            }
        }
    }
#endif
    otrl_tlv_free(tlvs);

    if (ignoreMessage == 1)
    {
        // Internal protocol message

		return IOtr::MsgTypeIgnore;
    }
    else if ((ignoreMessage == 0) && newMessage)
    {
        // Message has been decrypted, replace it
		ADecrypted = QString::fromUtf8(newMessage);
        otrl_message_free(newMessage);
		return IOtr::MsgTypeOtr;
    }

	return IOtr::MsgTypeNone;
}

//-----------------------------------------------------------------------------

QList<OtrFingerprint> OtrInternal::getFingerprints()
{
	QList<OtrFingerprint> fpList;
    ConnContext* context;
    ::Fingerprint* fingerprint;

	for (context = FUserState->context_root; context != NULL;
         context = context->next)
    {
        fingerprint = context->fingerprint_root.next;
        while(fingerprint)
        {
			OtrFingerprint fp(fingerprint->fingerprint,
							  QString::fromUtf8(context->accountname),
							  QString::fromUtf8(context->username),
							  QString::fromUtf8(fingerprint->trust));

            fpList.append(fp);
            fingerprint = fingerprint->next;
        }
    }
    return fpList;
}

//-----------------------------------------------------------------------------

void OtrInternal::verifyFingerprint(const OtrFingerprint &AFingerprint,
									bool AVerified)
{
	ConnContext* context = otrl_context_find(FUserState,
											 AFingerprint.username.toUtf8().constData(),
											 AFingerprint.account.toUtf8().constData(),
                                             OTR_PROTOCOL_STRING,
#if (OTRL_VERSION_MAJOR >= 4)
                                             OTRL_INSTAG_BEST,
#endif
                                             false, NULL, NULL, NULL);
    if (context)
    {
        ::Fingerprint* fp = otrl_context_find_fingerprint(context,
														  AFingerprint.fingerprint,
                                                          0, NULL);
        if (fp)
        {
			otrl_context_set_trust(fp, AVerified? "verified" : "");
            write_fingerprints();

            if (context->active_fingerprint == fp)
            {
				FOtr->stateChange(QString::fromUtf8(context->accountname),
								  QString::fromUtf8(context->username),
								  IOtr::StateChangeTrust);
            }
        }
    }
}

//-----------------------------------------------------------------------------

void OtrInternal::deleteFingerprint(const OtrFingerprint &AFingerprint)
{
	ConnContext* context = otrl_context_find(FUserState,
											 AFingerprint.username.toUtf8().constData(),
											 AFingerprint.account.toUtf8().constData(),
                                             OTR_PROTOCOL_STRING,
#if (OTRL_VERSION_MAJOR >= 4)
                                             OTRL_INSTAG_BEST,
#endif
                                             false, NULL, NULL, NULL);
    if (context)
    {
        ::Fingerprint* fp = otrl_context_find_fingerprint(context,
														  AFingerprint.fingerprint,
                                                          0, NULL);
        if (fp)
        {
            if (context->active_fingerprint == fp)
            {
                otrl_context_force_finished(context);
            }
            otrl_context_forget_fingerprint(fp, true);
            write_fingerprints();
        }
    }
}

//-----------------------------------------------------------------------------

QHash<QString, QString> OtrInternal::getPrivateKeys()
{
    QHash<QString, QString> privKeyList;
    OtrlPrivKey* privKey;

	for (privKey = FUserState->privkey_root; privKey != NULL;
         privKey = privKey->next)
    {
        char fingerprintBuf[OTRL_PRIVKEY_FPRINT_HUMAN_LEN];
		char* success = otrl_privkey_fingerprint(FUserState,
                                                 fingerprintBuf,
                                                 privKey->accountname,
                                                 OTR_PROTOCOL_STRING);
        if (success)
        {
            privKeyList.insert(QString::fromUtf8(privKey->accountname),
                               QString(fingerprintBuf));
        }
    }

    return privKeyList;
}

//-----------------------------------------------------------------------------

void OtrInternal::deleteKey(const QString& AAccount)
{
	OtrlPrivKey* privKey = otrl_privkey_find(FUserState,
											 AAccount.toUtf8().constData(),
                                             OTR_PROTOCOL_STRING);

    otrl_privkey_forget(privKey);

	otrl_privkey_write(FUserState, QFile::encodeName(FKeysFile).constData());
}

//-----------------------------------------------------------------------------

void OtrInternal::startSession(const QString& AAccount, const QString& AContact)
{
	FOtr->stateChange(AAccount, AContact, IOtr::StateChangeGoingSecure);

	if (!otrl_privkey_find(FUserState, AAccount.toUtf8().constData(),
                           OTR_PROTOCOL_STRING))
    {
		create_privkey(AAccount.toUtf8().constData(), OTR_PROTOCOL_STRING);
    }

    //TODO: make allowed otr versions configureable
	char* msg = otrl_proto_default_query_msg(FOtr->humanAccountPublic(AAccount).toUtf8().constData(),
                                             OTRL_POLICY_DEFAULT);

	FOtr->sendMessage(AAccount, AContact, QString::fromUtf8(msg));

    free(msg);
}

//-----------------------------------------------------------------------------

void OtrInternal::endSession(const QString& AAccount, const QString& AContact)
{
	ConnContext* context = otrl_context_find(FUserState,
											 AContact.toUtf8().constData(),
											 AAccount.toUtf8().constData(),
                                             OTR_PROTOCOL_STRING,
#if (OTRL_VERSION_MAJOR >= 4)
                                             OTRL_INSTAG_BEST,
#endif
                                             false, NULL, NULL, NULL);
    if (context && (context->msgstate != OTRL_MSGSTATE_PLAINTEXT))
    {
		FOtr->stateChange(AAccount, AContact, IOtr::StateChangeClose);
    }
	otrl_message_disconnect(FUserState, &FUiOps, this,
							AAccount.toUtf8().constData(), OTR_PROTOCOL_STRING,
							AContact.toUtf8().constData()
#if (OTRL_VERSION_MAJOR >= 4)
                            ,OTRL_INSTAG_BEST
#endif
                            );
}

//-----------------------------------------------------------------------------

void OtrInternal::expireSession(const QString& AAccount, const QString& AContact)
{
	ConnContext* context = otrl_context_find(FUserState,
											 AContact.toUtf8().constData(),
											 AAccount.toUtf8().constData(),
                                             OTR_PROTOCOL_STRING,
#if (OTRL_VERSION_MAJOR >= 4)
                                             OTRL_INSTAG_BEST,
#endif
                                             false, NULL, NULL, NULL);
    if (context && (context->msgstate == OTRL_MSGSTATE_ENCRYPTED))
    {
        otrl_context_force_finished(context);
		FOtr->stateChange(AAccount, AContact, IOtr::StateChangeGoneInsecure);
    }
}

//-----------------------------------------------------------------------------

void OtrInternal::startSMP(const QString& AAccount, const QString& AContact,
						   const QString& AQuestion, const QString& ASecret)
{
	ConnContext* context = otrl_context_find(FUserState,
											 AContact.toUtf8().constData(),
											 AAccount.toUtf8().constData(),
                                             OTR_PROTOCOL_STRING,
#if (OTRL_VERSION_MAJOR >= 4)
                                             OTRL_INSTAG_BEST,
#endif
                                             false, NULL, NULL, NULL);
    if (context)
    {
		QByteArray  secretArray   = ASecret.toUtf8();
        const char* secretPointer = secretArray.constData();
        size_t      secretLength  = qstrlen(secretPointer);

		if (AQuestion.isEmpty())
        {
			otrl_message_initiate_smp(FUserState, &FUiOps, this, context,
                                      reinterpret_cast<const unsigned char*>(const_cast<char*>(secretPointer)),
                                      secretLength);
        }
        else
        {
			otrl_message_initiate_smp_q(FUserState, &FUiOps, this, context,
										AQuestion.toUtf8().constData(),
                                        reinterpret_cast<const unsigned char*>(const_cast<char*>(secretPointer)),
                                        secretLength);
        }
    }
}

void OtrInternal::continueSMP(const QString& AAccount, const QString& AContact,
							  const QString& ASecret)
{
	ConnContext* context = otrl_context_find(FUserState,
											 AContact.toUtf8().constData(),
											 AAccount.toUtf8().constData(),
                                             OTR_PROTOCOL_STRING,
#if (OTRL_VERSION_MAJOR >= 4)
                                             OTRL_INSTAG_BEST,
#endif
                                             false, NULL, NULL, NULL);
    if (context)
    {
		QByteArray  secretArray   = ASecret.toUtf8();
        const char* secretPointer = secretArray.constData();
        size_t      secretLength  = qstrlen(secretPointer);

		otrl_message_respond_smp(FUserState, &FUiOps, this, context,
                                 reinterpret_cast<const unsigned char*>(secretPointer),
                                 secretLength);
    }
}

void OtrInternal::abortSMP(const QString& account, const QString& AContact)
{
	ConnContext* context = otrl_context_find(FUserState,
											 AContact.toUtf8().constData(),
                                             account.toUtf8().constData(),
                                             OTR_PROTOCOL_STRING,
#if (OTRL_VERSION_MAJOR >= 4)
                                             OTRL_INSTAG_BEST,
#endif
                                             false, NULL, NULL, NULL);
    if (context)
    {
        abortSMP(context);
    }
}

void OtrInternal::abortSMP(ConnContext* AContext)
{
	otrl_message_abort_smp(FUserState, &FUiOps, this, AContext);
}

//-----------------------------------------------------------------------------

IOtr::MessageState OtrInternal::getMessageState(const QString& AAccount,
												   const QString& AContact)
{
	ConnContext* context = otrl_context_find(FUserState,
											 AContact.toUtf8().constData(),
											 AAccount.toUtf8().constData(),
                                             OTR_PROTOCOL_STRING,
#if (OTRL_VERSION_MAJOR >= 4)
                                             OTRL_INSTAG_BEST,
#endif
                                             false, NULL, NULL, NULL);
    if (context)
    {
        if (context->msgstate == OTRL_MSGSTATE_PLAINTEXT)
        {
			return IOtr::MsgStatePlaintext;
        }
        else if (context->msgstate == OTRL_MSGSTATE_ENCRYPTED)
        {
			return IOtr::MsgStateEncrypted;
        }
        else if (context->msgstate == OTRL_MSGSTATE_FINISHED)
        {
			return IOtr::MsgStateFinished;
        }
    }

	return IOtr::MsgStateUnknown;
}

//-----------------------------------------------------------------------------

QString OtrInternal::getMessageStateString(const QString& AAccount,
										   const QString& AContact)
{
	IOtr::MessageState state = getMessageState(AAccount, AContact);

	if (state == IOtr::MsgStatePlaintext)
    {
        return QObject::tr("plaintext");
    }
	else if (state == IOtr::MsgStateEncrypted)
    {
        return QObject::tr("encrypted");
    }
	else if (state == IOtr::MsgStateFinished)
    {
        return QObject::tr("finished");
    }

    return QObject::tr("unknown");
}

//-----------------------------------------------------------------------------

QString OtrInternal::getSessionId(const QString& AAccount,
								  const QString& AContact)
{
    ConnContext* context;
	context = otrl_context_find(FUserState, AContact.toUtf8().constData(),
								AAccount.toUtf8().constData(), OTR_PROTOCOL_STRING,
#if (OTRL_VERSION_MAJOR >= 4)
                                OTRL_INSTAG_BEST,
#endif
                                false, NULL, NULL, NULL);
    if (context && (context->sessionid_len > 0))
    {
        QString firstHalf;
        QString secondHalf;

        for (unsigned int i = 0; i < context->sessionid_len / 2; i++)
        {
            if (context->sessionid[i] <= 0xf)
            {
                firstHalf.append("0");
            }
            firstHalf.append(QString::number(context->sessionid[i], 16));
        }
        for (size_t i = context->sessionid_len / 2;
             i < context->sessionid_len; i++)
        {
            if (context->sessionid[i] <= 0xf)
            {
                secondHalf.append("0");
            }
            secondHalf.append(QString::number(context->sessionid[i], 16));
        }

        if (context->sessionid_half == OTRL_SESSIONID_FIRST_HALF_BOLD)
        {
            return QString("<b>" + firstHalf + "</b> " + secondHalf);
        }
        else
        {
            return QString(firstHalf + " <b>" + secondHalf + "</b>");
        }
    }

    return QString();
}

//-----------------------------------------------------------------------------

OtrFingerprint OtrInternal::getActiveFingerprint(const QString& AAccount,
												 const QString& AContact)
{
    ConnContext* context;
	context = otrl_context_find(FUserState, AContact.toUtf8().constData(),
								AAccount.toUtf8().constData(), OTR_PROTOCOL_STRING,
#if (OTRL_VERSION_MAJOR >= 4)
                                OTRL_INSTAG_BEST,
#endif
                                false, NULL, NULL, NULL);

    if (context && context->active_fingerprint)
    {
		return OtrFingerprint(context->active_fingerprint->fingerprint,
							  QString::fromUtf8(context->accountname),
							  QString::fromUtf8(context->username),
							  QString::fromUtf8(context->active_fingerprint->trust));
    }

	return OtrFingerprint();
}

//-----------------------------------------------------------------------------

bool OtrInternal::isVerified(const QString& AAccount,
							 const QString& AContact)
{
    ConnContext* context;
	context = otrl_context_find(FUserState, AContact.toUtf8().constData(),
								AAccount.toUtf8().constData(),
                                OTR_PROTOCOL_STRING,
#if (OTRL_VERSION_MAJOR >= 4)
                                OTRL_INSTAG_BEST,
#endif
                                false, NULL, NULL, NULL);

    return isVerified(context);
}

//-----------------------------------------------------------------------------

bool OtrInternal::isVerified(ConnContext* AContext)
{

	if (AContext && AContext->active_fingerprint)
    {
		return (AContext->active_fingerprint->trust &&
				AContext->active_fingerprint->trust[0]);
    }

    return false;
}

//-----------------------------------------------------------------------------

bool OtrInternal::smpSucceeded(const QString& AAccount,
							   const QString& AContact)
{
    ConnContext* context;
	context = otrl_context_find(FUserState, AContact.toUtf8().constData(),
								AAccount.toUtf8().constData(), OTR_PROTOCOL_STRING,
#if (OTRL_VERSION_MAJOR >= 4)
                                OTRL_INSTAG_BEST,
#endif
                                false, NULL, NULL, NULL);

    if (context)
    {
        return context->smstate->sm_prog_state == OTRL_SMP_PROG_SUCCEEDED;
    }

    return false;
}

//-----------------------------------------------------------------------------

void OtrInternal::generateKey(const QString& AAccount)
{
	create_privkey(AAccount.toUtf8().constData(), OTR_PROTOCOL_STRING);
}

//-----------------------------------------------------------------------------

QString OtrInternal::humanFingerprint(const unsigned char* AFingerprint)
{
    char fpHash[OTRL_PRIVKEY_FPRINT_HUMAN_LEN];
	otrl_privkey_hash_to_human(fpHash, AFingerprint);
    return QString(fpHash);
}

//-----------------------------------------------------------------------------
/***  implemented callback functions for libotr ***/

OtrlPolicy OtrInternal::policy(ConnContext*)
{
	if (FOtrPolicy == IOtr::PolocyOff)
    {
        return OTRL_POLICY_NEVER; // otr disabled
    }
	else if (FOtrPolicy == IOtr::PolicyEnabled)
    {
        return OTRL_POLICY_MANUAL; // otr enabled, session started manual
    }
	else if (FOtrPolicy == IOtr::PolicyAuto)
    {
        return OTRL_POLICY_OPPORTUNISTIC; // automatically initiate private messaging
    }
	else if (FOtrPolicy == IOtr::PolicyRequire)
    {
        return OTRL_POLICY_ALWAYS; // require private messaging
    }

    return OTRL_POLICY_NEVER;
}

// ---------------------------------------------------------------------------

void OtrInternal::create_privkey(const char* AAccountname,
								 const char* AProtocol)
{
	if (FIsGenerating)
    {
        return;
    }

	QMessageBox qMB(QMessageBox::Question, QObject::tr("Off-the-Record Messaging"),
                    QObject::tr("Private keys for account \"%1\" need to be generated. "
                                "This takes quite some time (from a few seconds to a "
								"couple of minutes), and while you can use %2 in the "
                                "meantime, all the messages will be sent unencrypted "
                                "until keys are generated. You will be notified when "
                                "this process finishes.\n"
                                "\n"
                                "Do you want to generate keys now?")
								.arg(FOtr->humanAccount(
											QString::fromUtf8(AAccountname)))
								.arg(CLIENT_NAME),
                       QMessageBox::Yes | QMessageBox::No);

    if (qMB.exec() != QMessageBox::Yes)
    {
        return;
    }

    void *newkeyp;
	if (otrl_privkey_generate_start(FUserState, AAccountname, AProtocol, &newkeyp) == gcry_error(GPG_ERR_EEXIST)) {
        qWarning("libotr reports it's still generating a previous key while it shouldn't be");
        return;
    }

	FIsGenerating = true;

    QEventLoop loop;
    QFutureWatcher<gcry_error_t> watcher;

    QObject::connect(&watcher, SIGNAL(finished()), &loop, SLOT(quit()));

    QFuture<gcry_error_t> future = QtConcurrent::run(otrl_privkey_generate_calculate, newkeyp);
    watcher.setFuture(future);

    loop.exec();

	FIsGenerating = false;

    if (future.result() == gcry_error(GPG_ERR_NO_ERROR)) {
		otrl_privkey_generate_finish(FUserState, newkeyp, QFile::encodeName(FKeysFile));
    }

    char fingerprint[OTRL_PRIVKEY_FPRINT_HUMAN_LEN];
	if (otrl_privkey_fingerprint(FUserState, fingerprint, AAccountname,
								 AProtocol))
    {
		QMessageBox infoMb(QMessageBox::Information, QObject::tr("Off-the-Record Messaging"),
                           QObject::tr("Keys have been generated. "
                                       "Fingerprint for account \"%1\":\n"
                                       "%2\n"
                                       "\n"
                                       "Thanks for your patience.")
								   .arg(FOtr->humanAccount(
											QString::fromUtf8(AAccountname)))
                                   .arg(QString(fingerprint)));
        infoMb.exec();
    }
    else
    {
		QMessageBox failMb(QMessageBox::Critical, QObject::tr("Off-the-Record Messaging"),
                           QObject::tr("Failed to generate keys for account \"%1\"."
                                       "\nThe OTR Plugin will not work.")
									   .arg(FOtr->humanAccount(
												QString::fromUtf8(AAccountname))),
                           QMessageBox::Ok);
        failMb.exec();
    }
}

// ---------------------------------------------------------------------------

int OtrInternal::is_logged_in(const char* AAccountname, const char* AProtocol,
							  const char* ARecipient)
{
	Q_UNUSED(AProtocol);

	if (FOtr->isLoggedIn(QString::fromUtf8(AAccountname),
							   QString::fromUtf8(ARecipient)))
    {
        return 1; // contact online
    }
    else
    {
        return 0; // contact offline
    }
}

// ---------------------------------------------------------------------------

void OtrInternal::inject_message(const char* AAccountname,
								 const char* AProtocol, const char* ARecipient,
								 const char* AMessage)
{
	Q_UNUSED(AProtocol);

	FOtr->sendMessage(QString::fromUtf8(AAccountname),
							QString::fromUtf8(ARecipient),
							QString::fromUtf8(AMessage));
}

// ---------------------------------------------------------------------------

#if (OTRL_VERSION_MAJOR >= 4)
void OtrInternal::handle_msg_event(OtrlMessageEvent msg_event, ConnContext* context,
                                   const char* message, gcry_error_t err)
{
    Q_UNUSED(err);
    Q_UNUSED(message);

    QString account = QString::fromUtf8(context->accountname);
    QString contact = QString::fromUtf8(context->username);

    QString errorString;
    switch (msg_event)
    {
        case OTRL_MSGEVENT_RCVDMSG_UNENCRYPTED:
            errorString = QObject::tr("<b>The following message received "
                                    "from %1 was <i>not</i> encrypted:</b>")
									.arg(FOtr->humanContact(account, contact));
            break;
        case OTRL_MSGEVENT_CONNECTION_ENDED:
            errorString = QObject::tr("Your message was not sent. Either end your "
                                       "private conversation, or restart it.");
            break;
        case OTRL_MSGEVENT_RCVDMSG_UNRECOGNIZED:
            errorString = QObject::tr("Unreadable encrypted message was received.");
            break;
        case OTRL_MSGEVENT_RCVDMSG_NOT_IN_PRIVATE:
            errorString = QObject::tr("Received an encrypted message but it cannot "
                                      "be read because no private connection is "
                                      "established yet.");
            break;
        case OTRL_MSGEVENT_RCVDMSG_UNREADABLE:
            errorString = QObject::tr("Received message is unreadable.");
            break;
        case OTRL_MSGEVENT_RCVDMSG_MALFORMED:
            errorString = QObject::tr("Received message contains malformed data.");
            break;
        default: ;
    }

    if (!errorString.isEmpty()) {
		FOtr->displayOtrMessage(QString::fromUtf8(context->accountname),
                                      QString::fromUtf8(context->username),
                                      errorString);
    }
}

void OtrInternal::handle_smp_event(OtrlSMPEvent smp_event, ConnContext* context,
                                   unsigned short progress_percent, char* question)
{
    if (smp_event == OTRL_SMPEVENT_CHEATED || smp_event == OTRL_SMPEVENT_ERROR) {
        abortSMP(context);
		FOtr->updateSMP(QString::fromUtf8(context->accountname),
                              QString::fromUtf8(context->username),
                              -2);
    }
    else if (smp_event == OTRL_SMPEVENT_ASK_FOR_SECRET ||
             smp_event == OTRL_SMPEVENT_ASK_FOR_ANSWER) {
		FOtr->receivedSMP(QString::fromUtf8(context->accountname),
                                QString::fromUtf8(context->username),
                                QString::fromUtf8(question));
    }
    else {
		FOtr->updateSMP(QString::fromUtf8(context->accountname),
                              QString::fromUtf8(context->username),
                              progress_percent);
    }
}

void OtrInternal::create_instag(const char* accountname, const char* protocol)
{
	otrl_instag_generate(FUserState, QFile::encodeName(FInstagsFile).constData(),
                         accountname, protocol);
}
#else
void OtrInternal::notify(OtrlNotifyLevel level, const char* accountname,
                         const char* protocol, const char* username,
                         const char* title, const char* primary, const char* secondary)
{
    Q_UNUSED(protocol);
    Q_UNUSED(title);

    QString account = QString::fromUtf8(accountname);
    QString contact = QString::fromUtf8(username);
    QString message = QString(primary) + "\n" + QString(secondary);

    if (!m_callback->displayOtrMessage(account, contact, message))
    {
		IOtr::OtrNotifyType type;

        if (level == OTRL_NOTIFY_ERROR )
        {
			type = IOtr::OTR_NOTIFY_ERROR;
        }
        else if (level == OTRL_NOTIFY_WARNING)
        {
			type = IOtr::OTR_NOTIFY_WARNING;
        }
        else
        {
			type = IOtr::OTR_NOTIFY_ERROR;
        }

        m_callback->notifyUser(account, contact, message, type);
    }
}

int OtrInternal::display_otr_message(const char* accountname,
                                     const char* protocol,
                                     const char* username,
                                     const char* msg)
{
    Q_UNUSED(protocol);

    QString message = QString::fromUtf8(msg);

    if (QRegExp("^<b>The following message received "
                "from .+ was <i>not</i> encrypted: "
                "\\[</b>.+<b>\\]</b>$").exactMatch(message))
    {
        return -1;
    }
    else
    {
		return FOtr->displayOtrMessage(QString::fromUtf8(accountname),
									   QString::fromUtf8(username),
									   message)? 0 : -1;
    }
}
#endif

// ---------------------------------------------------------------------------

void OtrInternal::update_context_list()
{
}

// ---------------------------------------------------------------------------

#if !(OTRL_VERSION_MAJOR >= 4)
const char* OtrInternal::protocol_name(const char* protocol)
{
    Q_UNUSED(protocol);
    return OTR_PROTOCOL_STRING;
}

void OtrInternal::protocol_name_free(const char* protocol_name)
{
    Q_UNUSED(protocol_name);
}
#endif

// ---------------------------------------------------------------------------

void OtrInternal::new_fingerprint(OtrlUserState AUserState, const char* AAccountName,
								  const char* AProtocol, const char* AUsername,
								  unsigned char AFingerprint[20])
{
	Q_UNUSED(AUserState);
	Q_UNUSED(AProtocol);

	QString account = QString::fromUtf8(AAccountName);
	QString contact = QString::fromUtf8(AUsername);
    QString message = QObject::tr("You have received a new "
								  "fingerprint from %1:\n%2")
								.arg(FOtr->humanContact(account, contact))
								.arg(humanFingerprint(AFingerprint));

	if (!FOtr->displayOtrMessage(account, contact, message))
    {
		FOtr->notifyUser(account, contact, message, IOtr::NotifyInfo);
    }
}

// ---------------------------------------------------------------------------

void OtrInternal::write_fingerprints()
{
	otrl_privkey_write_fingerprints(FUserState,
									QFile::encodeName(FFingerprintFile).constData());
}

// ---------------------------------------------------------------------------

void OtrInternal::gone_secure(ConnContext* context)
{
	FOtr->stateChange(QString::fromUtf8(context->accountname),
					  QString::fromUtf8(context->username),
					  IOtr::StateChangeGoneSecure);
}

// ---------------------------------------------------------------------------

void OtrInternal::gone_insecure(ConnContext* context)
{
	FOtr->stateChange(QString::fromUtf8(context->accountname),
					  QString::fromUtf8(context->username),
					  IOtr::StateChangeGoneInsecure);
}

// ---------------------------------------------------------------------------

void OtrInternal::still_secure(ConnContext* context, int is_reply)
{
    Q_UNUSED(is_reply);
	FOtr->stateChange(QString::fromUtf8(context->accountname),
					  QString::fromUtf8(context->username),
					  IOtr::StateChangeStillSecure);
}

// ---------------------------------------------------------------------------

#if !(OTRL_VERSION_MAJOR >= 4)
void OtrInternal::log_message(const char* message)
{
    Q_UNUSED(message);
}
#endif

// ---------------------------------------------------------------------------

const char* OtrInternal::account_name(const char* account,
                                      const char* protocol)
{
    Q_UNUSED(protocol);
	return qstrdup(FOtr->humanAccountPublic(QString::fromUtf8(account))
                                                 .toUtf8().constData());
}

// ---------------------------------------------------------------------------

void OtrInternal::account_name_free(const char* account_name)
{
    delete [] account_name;
}

// ---------------------------------------------------------------------------
/*** static wrapper functions ***/

OtrlPolicy OtrInternal::cb_policy(void* opdata, ConnContext* context) {
    return static_cast<OtrInternal*>(opdata)->policy(context);
}

void OtrInternal::cb_create_privkey(void* opdata, const char* accountname, const char* protocol) {
    static_cast<OtrInternal*>(opdata)->create_privkey(accountname, protocol);
}

int OtrInternal::cb_is_logged_in(void* opdata, const char* accountname, const char* protocol, const char* recipient) {
    return static_cast<OtrInternal*>(opdata)->is_logged_in(accountname, protocol, recipient);
}

void OtrInternal::cb_inject_message(void* opdata, const char* accountname, const char* protocol, const char* recipient, const char* message) {
    static_cast<OtrInternal*>(opdata)->inject_message(accountname, protocol, recipient, message);
}

#if (OTRL_VERSION_MAJOR >= 4)
void OtrInternal::cb_handle_msg_event(void* opdata, OtrlMessageEvent msg_event, ConnContext* context, const char* message, gcry_error_t err) {
    static_cast<OtrInternal*>(opdata)->handle_msg_event(msg_event, context, message, err);
}

void OtrInternal::cb_handle_smp_event(void* opdata, OtrlSMPEvent smp_event, ConnContext* context, unsigned short progress_percent, char* question) {
    static_cast<OtrInternal*>(opdata)->handle_smp_event(smp_event, context, progress_percent, question);
}

void OtrInternal::cb_create_instag(void* opdata, const char* accountname, const char* protocol) {
    static_cast<OtrInternal*>(opdata)->create_instag(accountname, protocol);
}
#else
void OtrInternal::cb_notify(void* opdata, OtrlNotifyLevel level, const char* accountname, const char* protocol, const char* username, const char* title, const char* primary, const char* secondary) {
    static_cast<OtrInternal*>(opdata)->notify(level, accountname, protocol, username, title, primary, secondary);
}

int OtrInternal::cb_display_otr_message(void* opdata, const char* accountname, const char* protocol, const char* username, const char* msg) {
    return static_cast<OtrInternal*>(opdata)->display_otr_message(accountname, protocol, username, msg);
}
#endif

void OtrInternal::cb_update_context_list(void* opdata) {
    static_cast<OtrInternal*>(opdata)->update_context_list();
}

#if !(OTRL_VERSION_MAJOR >= 4)
const char* OtrInternal::cb_protocol_name(void* opdata, const char* protocol) {
    return static_cast<OtrInternal*>(opdata)->protocol_name(protocol);
}

void OtrInternal::cb_protocol_name_free(void* opdata, const char* protocol_name) {
    static_cast<OtrInternal*>(opdata)->protocol_name(protocol_name);
}
#endif

void OtrInternal::cb_new_fingerprint(void* opdata, OtrlUserState us, const char* accountname, const char* protocol, const char* username, unsigned char fingerprint[20]) {
    static_cast<OtrInternal*>(opdata)->new_fingerprint(us, accountname, protocol, username, fingerprint);
}

void OtrInternal::cb_write_fingerprints(void* opdata) {
    static_cast<OtrInternal*>(opdata)->write_fingerprints();
}

void OtrInternal::cb_gone_secure(void* opdata, ConnContext* context) {
    static_cast<OtrInternal*>(opdata)->gone_secure(context);
}

void OtrInternal::cb_gone_insecure(void* opdata, ConnContext* context) {
    static_cast<OtrInternal*>(opdata)->gone_insecure(context);
}

void OtrInternal::cb_still_secure(void* opdata, ConnContext* context, int is_reply) {
    static_cast<OtrInternal*>(opdata)->still_secure(context, is_reply);
}

#if !(OTRL_VERSION_MAJOR >= 4)
void OtrInternal::cb_log_message(void* opdata, const char* message) {
    static_cast<OtrInternal*>(opdata)->log_message(message);
}
#endif

const char* OtrInternal::cb_account_name(void* opdata, const char* account,
                                         const char* protocol) {
    return static_cast<OtrInternal*>(opdata)->account_name(account, protocol);
}

void OtrInternal::cb_account_name_free(void* opdata, const char* account_name) {
    static_cast<OtrInternal*>(opdata)->account_name_free(account_name);
}
// ---------------------------------------------------------------------------
