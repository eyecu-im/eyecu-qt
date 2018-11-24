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

#include <QDebug>
#include <QMessageBox>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QDir>
#include <QFile>
#if QT_VERSION < 0x050000
#include <QtConcurrentRun>
#endif
#include <definitions/version.h>
#include <definitions/optionvalues.h>
#include <utils/options.h>
#include "otrinternal.h"

//-----------------------------------------------------------------------------
static const char*   OTR_PROTOCOL_STRING = "prpl-jabber";
static const QString OTR_FINGERPRINTS_FILE = "otr.fingerprints";
static const QString OTR_KEYS_FILE = "otr.keys";
static const QString OTR_INSTAGS_FILE = "otr.instags";

// ============================================================================

OtrInternal::OtrInternal(IOtr* AOtr)
	: FUserState(),
	  FUiOps(),
	  FOtr(AOtr),
	  FIsGenerating(false)
{
	OTRL_INIT;
	FUserState                 = otrl_userstate_create();
	FUiOps.policy              = (*OtrInternal::cbPolicy);
	FUiOps.create_privkey      = (*OtrInternal::cbCreatePrivkey);
	FUiOps.is_logged_in        = (*OtrInternal::cbIsLoggedIn);
	FUiOps.inject_message      = (*OtrInternal::cbInjectMessage);
	FUiOps.update_context_list = (*OtrInternal::cbUpdateContextList);
	FUiOps.new_fingerprint     = (*OtrInternal::cbNewFingerprint);
	FUiOps.write_fingerprints  = (*OtrInternal::cbWriteFingerprints);
	FUiOps.gone_secure         = (*OtrInternal::cbGoneSecure);
	FUiOps.gone_insecure       = (*OtrInternal::cbGoneInsecure);
	FUiOps.still_secure        = (*OtrInternal::cbStillSecure);

	FUiOps.max_message_size    = nullptr;
	FUiOps.account_name        = (*OtrInternal::cbAccountName);
	FUiOps.account_name_free   = (*OtrInternal::cbAccountNameFree);

	FUiOps.handle_msg_event    = (*OtrInternal::cbHandleMsgEvent);
	FUiOps.handle_smp_event    = (*OtrInternal::cbHandleSmpEvent);
	FUiOps.create_instag       = (*OtrInternal::cbCreateInstag);
}

//-----------------------------------------------------------------------------

OtrInternal::~OtrInternal()
{
	otrl_userstate_free(FUserState);
}

void OtrInternal::init()
{
	QDir profileDir(FOtr->dataDir());

	FKeysFile        = profileDir.filePath(OTR_KEYS_FILE);
	FInstagsFile     = profileDir.filePath(OTR_INSTAGS_FILE);
	FFingerprintFile = profileDir.filePath(OTR_FINGERPRINTS_FILE);

	otrl_privkey_read(FUserState, QFile::encodeName(FKeysFile).constData());
	otrl_privkey_read_fingerprints(FUserState,
								   QFile::encodeName(FFingerprintFile).constData(),
                                   nullptr, nullptr);
	otrl_instag_read(FUserState, QFile::encodeName(FInstagsFile).constData());
}

//-----------------------------------------------------------------------------

QString OtrInternal::encryptMessage(const QString& AAccount, const QString& AContact,
									const QString& AMessage)
{
    char* encMessage = nullptr;
    gcry_error_t err;

	err = otrl_message_sending(FUserState, &FUiOps, this,
							   AAccount.toUtf8().constData(), OTR_PROTOCOL_STRING,
							   AContact.toUtf8().constData(),
                               OTRL_INSTAG_BEST,
							   AMessage.toUtf8().constData(),
                               nullptr, &encMessage,
                               OTRL_FRAGMENT_SEND_SKIP,
                               nullptr, nullptr, nullptr);
    if (err)
    {
        QString err_message = QObject::tr("Encrypting message to %1 "
                                          "failed.\nThe message was not sent.")
										  .arg(AContact);
		if (!FOtr->displayOtrMessage(AAccount, AContact, err_message))
			FOtr->notifyUser(AAccount, AContact, err_message, IOtr::NotifyError);
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
    char* newMessage  = nullptr;
    OtrlTLV* tlvs     = nullptr;
    OtrlTLV* tlv      = nullptr;

	qDebug() << "calling otrl_message_receiving(FUserState, &FUiOps, " << this
			 << accountName << "," << OTR_PROTOCOL_STRING << "," << userName
			 << AMessage << ", &newMessage, &tlvs, nullptr, nullptr, nullptr)";
	ignoreMessage = otrl_message_receiving(FUserState, &FUiOps, this,
                                           accountName,
                                           OTR_PROTOCOL_STRING,
                                           userName,
										   AMessage.toUtf8().constData(),
                                           &newMessage, &tlvs, nullptr,
                                           nullptr, nullptr);
	qDebug() << "Ok!";
    tlv = otrl_tlv_find(tlvs, OTRL_TLV_DISCONNECTED);
    if (tlv) {
		FOtr->stateChange(accountName, userName, IOtr::StateChangeRemoteClose);
    }

    // Magic hack to force it work similar to libotr < 4.0.0.
    // If user received unencrypted message he (she) should be notified.
    // See OTRL_MSGEVENT_RCVDMSG_UNENCRYPTED as well.
	if (ignoreMessage && !newMessage && !AMessage.startsWith("?OTR")) {
        ignoreMessage = 0;
    }

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

    for (context = FUserState->context_root; context != nullptr;
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
                                             OTRL_INSTAG_BEST,
                                             false, nullptr, nullptr, nullptr);
    if (context)
    {
        ::Fingerprint* fp = otrl_context_find_fingerprint(context,
														  AFingerprint.fingerprint,
                                                          0, nullptr);
        if (fp)
        {
			otrl_context_set_trust(fp, AVerified? "verified" : "");
            writeFingerprints();

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
                                             OTRL_INSTAG_BEST,
                                             false, nullptr, nullptr, nullptr);
    if (context)
    {
        ::Fingerprint* fp = otrl_context_find_fingerprint(context,
														  AFingerprint.fingerprint,
                                                          0, nullptr);
        if (fp)
        {
            if (context->active_fingerprint == fp)
            {
                otrl_context_force_finished(context);
            }
            otrl_context_forget_fingerprint(fp, true);
            writeFingerprints();
        }
    }
}

//-----------------------------------------------------------------------------

QHash<QString, QString> OtrInternal::getPrivateKeys()
{
    QHash<QString, QString> privKeyList;
    OtrlPrivKey* privKey;

    for (privKey = FUserState->privkey_root; privKey != nullptr;
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
        createPrivkey(AAccount.toUtf8().constData(), OTR_PROTOCOL_STRING);
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
                                             OTRL_INSTAG_BEST,
                                             false, nullptr, nullptr, nullptr);
    if (context && (context->msgstate != OTRL_MSGSTATE_PLAINTEXT))
    {
		FOtr->stateChange(AAccount, AContact, IOtr::StateChangeClose);
    }
	otrl_message_disconnect(FUserState, &FUiOps, this,
							AAccount.toUtf8().constData(), OTR_PROTOCOL_STRING,
                            AContact.toUtf8().constData(),OTRL_INSTAG_BEST);
}

//-----------------------------------------------------------------------------

void OtrInternal::expireSession(const QString& AAccount, const QString& AContact)
{
	ConnContext* context = otrl_context_find(FUserState,
											 AContact.toUtf8().constData(),
											 AAccount.toUtf8().constData(),
                                             OTR_PROTOCOL_STRING,
                                             OTRL_INSTAG_BEST, false,
                                             nullptr, nullptr, nullptr);
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
                                             OTRL_INSTAG_BEST, false,
                                             nullptr, nullptr, nullptr);
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
                                             OTRL_INSTAG_BEST, false,
                                             nullptr, nullptr, nullptr);
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
                                             OTRL_INSTAG_BEST, false,
                                             nullptr, nullptr, nullptr);
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
                                             OTRL_INSTAG_BEST, false,
                                             nullptr, nullptr, nullptr);
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
                                OTRL_INSTAG_BEST, false,
                                nullptr, nullptr, nullptr);
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
                                OTRL_INSTAG_BEST, false,
                                nullptr, nullptr, nullptr);

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
                                OTRL_INSTAG_BEST, false,
                                nullptr, nullptr, nullptr);

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
                                OTRL_INSTAG_BEST, false,
                                nullptr, nullptr, nullptr);
    if (context)
    {
        return context->smstate->sm_prog_state == OTRL_SMP_PROG_SUCCEEDED;
    }

    return false;
}

//-----------------------------------------------------------------------------

void OtrInternal::generateKey(const QString& AAccount)
{
    createPrivkey(AAccount.toUtf8().constData(), OTR_PROTOCOL_STRING);
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

OtrlPolicy OtrInternal::policy(IOtr::Policy APolicy)
{
	if (APolicy == IOtr::PolocyOff)
    {
        return OTRL_POLICY_NEVER; // otr disabled
    }
	else if (APolicy == IOtr::PolicyEnabled)
    {
        return OTRL_POLICY_MANUAL; // otr enabled, session started manual
    }
	else if (APolicy == IOtr::PolicyAuto)
    {
        return OTRL_POLICY_OPPORTUNISTIC; // automatically initiate private messaging
    }
	else if (APolicy == IOtr::PolicyRequire)
    {
        return OTRL_POLICY_ALWAYS; // require private messaging
    }

    return OTRL_POLICY_NEVER;
}

// ---------------------------------------------------------------------------

void OtrInternal::createPrivkey(const char* AAccountname,
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
	if (otrl_privkey_fingerprint(FUserState, fingerprint, AAccountname, AProtocol))
    {
		QMessageBox infoMb(QMessageBox::Information, QObject::tr("Off-the-Record Messaging"),
                           QObject::tr("Keys have been generated. "
									   "Fingerprint for account \"%1\":\n%2\n\n"
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

int OtrInternal::isLoggedIn(const char* AAccountname, const char* AProtocol,
                              const char* ARecipient) const
{
	Q_UNUSED(AProtocol);

    return FOtr->isLoggedIn(QString::fromUtf8(AAccountname),
                            QString::fromUtf8(ARecipient));
}

// ---------------------------------------------------------------------------

void OtrInternal::injectMessage(const char* AAccountname,
                                const char* AProtocol,
                                const char* ARecipient,
                                const char* AMessage)
{
	Q_UNUSED(AProtocol);

	FOtr->sendMessage(QString::fromUtf8(AAccountname),
							QString::fromUtf8(ARecipient),
							QString::fromUtf8(AMessage));
}

// ---------------------------------------------------------------------------

void OtrInternal::handleMsgEvent(OtrlMessageEvent AMsgEvent, ConnContext* AContext,
                                 const char* AMessage, gcry_error_t AError)
{
    Q_UNUSED(AError);
    Q_UNUSED(AMessage);

    QString account = QString::fromUtf8(AContext->accountname);
    QString contact = QString::fromUtf8(AContext->username);

    QString errorString;
    switch (AMsgEvent)
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
        FOtr->displayOtrMessage(QString::fromUtf8(AContext->accountname),
                                      QString::fromUtf8(AContext->username),
                                      errorString);
    }
}

void OtrInternal::handleSmpEvent(OtrlSMPEvent ASmpEvent, ConnContext* AContext,
                                   unsigned short AProgressPercent, char* AQuestion)
{
    if (ASmpEvent == OTRL_SMPEVENT_CHEATED || ASmpEvent == OTRL_SMPEVENT_ERROR) {
        abortSMP(AContext);
        FOtr->updateSMP(QString::fromUtf8(AContext->accountname),
                              QString::fromUtf8(AContext->username),
                              -2);
    }
    else if (ASmpEvent == OTRL_SMPEVENT_ASK_FOR_SECRET ||
             ASmpEvent == OTRL_SMPEVENT_ASK_FOR_ANSWER) {
        FOtr->receivedSMP(QString::fromUtf8(AContext->accountname),
                                QString::fromUtf8(AContext->username),
                                QString::fromUtf8(AQuestion));
    }
    else {
        FOtr->updateSMP(QString::fromUtf8(AContext->accountname),
                              QString::fromUtf8(AContext->username),
                              AProgressPercent);
    }
}

void OtrInternal::createInstag(const char* AAccountName, const char* AProtocol)
{
	otrl_instag_generate(FUserState, QFile::encodeName(FInstagsFile).constData(),
                         AAccountName, AProtocol);
}
// ---------------------------------------------------------------------------

void OtrInternal::updateContextList()
{
}

// ---------------------------------------------------------------------------

void OtrInternal::newFingerprint(OtrlUserState AUserState, const char* AAccountName,
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

void OtrInternal::writeFingerprints()
{
	otrl_privkey_write_fingerprints(FUserState,
									QFile::encodeName(FFingerprintFile).constData());
}

// ---------------------------------------------------------------------------

void OtrInternal::goneSecure(ConnContext* AContext)
{
    FOtr->stateChange(QString::fromUtf8(AContext->accountname),
                      QString::fromUtf8(AContext->username),
					  IOtr::StateChangeGoneSecure);
}

// ---------------------------------------------------------------------------

void OtrInternal::goneInsecure(ConnContext* AContext)
{
    FOtr->stateChange(QString::fromUtf8(AContext->accountname),
                      QString::fromUtf8(AContext->username),
					  IOtr::StateChangeGoneInsecure);
}

// ---------------------------------------------------------------------------

void OtrInternal::stillSecure(ConnContext* AContext, int AIsReply)
{
    Q_UNUSED(AIsReply);
    FOtr->stateChange(QString::fromUtf8(AContext->accountname),
                      QString::fromUtf8(AContext->username),
					  IOtr::StateChangeStillSecure);
}

// ---------------------------------------------------------------------------

const char* OtrInternal::accountName(const char* AAccount,
                                      const char* AProtocol)
{
    Q_UNUSED(AProtocol);
    return qstrdup(FOtr->humanAccountPublic(QString::fromUtf8(AAccount))
                                                 .toUtf8().constData());
}

// ---------------------------------------------------------------------------

void OtrInternal::accountNameFree(const char* AAccountName)
{
    delete [] AAccountName;
}

// ---------------------------------------------------------------------------
/*** static wrapper functions ***/

OtrlPolicy OtrInternal::cbPolicy(void* AOpdata, ConnContext* AContext) {
    Q_UNUSED(AOpdata);
    Q_UNUSED(AContext);
	qDebug() << "OtrInternal::cbPolicy()";
	return policy(IOtr::Policy(Options::node(OPV_OTR_POLICY).value().toInt()));
//	return static_cast<OtrInternal*>(opdata)->policy(context);
}

void OtrInternal::cbCreatePrivkey(void* APpdata, const char* AAccountName, const char* AProtocol) {
	qDebug() << "OtrInternal::cbCreatePrivkey()";
    static_cast<OtrInternal*>(APpdata)->createPrivkey(AAccountName, AProtocol);
}

int OtrInternal::cbIsLoggedIn(void* AOpdata, const char* AAccountName, const char* AProtocol, const char* ARecipient) {
	qDebug() << "OtrInternal::cbIsLoggedIn()";
    return static_cast<OtrInternal*>(AOpdata)->isLoggedIn(AAccountName, AProtocol, ARecipient);
}

void OtrInternal::cbInjectMessage(void* AOpdata, const char* AAccountname, const char* AProtocol, const char* ARecipient, const char* AMessage) {
	qDebug() << "OtrInternal::cbInjectMessage()";
    static_cast<OtrInternal*>(AOpdata)->injectMessage(AAccountname, AProtocol, ARecipient, AMessage);
}

void OtrInternal::cbHandleMsgEvent(void* AOpdata, OtrlMessageEvent AMsgEvent, ConnContext* AContext, const char* AMessage, gcry_error_t AError) {
	qDebug() << "OtrInternal::cbHandleMsgEvent()";
    static_cast<OtrInternal*>(AOpdata)->handleMsgEvent(AMsgEvent, AContext, AMessage, AError);
}

void OtrInternal::cbHandleSmpEvent(void* AOpdata, OtrlSMPEvent ASmpEvent, ConnContext* AContext, unsigned short AProgressPercent, char* AQuestion) {
	qDebug() << "OtrInternal::cbHandleSmpEvent()";
    static_cast<OtrInternal*>(AOpdata)->handleSmpEvent(ASmpEvent, AContext, AProgressPercent, AQuestion);
}

void OtrInternal::cbCreateInstag(void* AOpdata, const char* AAccountName, const char* AProtocol) {
	qDebug() << "OtrInternal::cbCreateInstag()";
    static_cast<OtrInternal*>(AOpdata)->createInstag(AAccountName, AProtocol);
}

void OtrInternal::cbUpdateContextList(void* AOpdata) {
	qDebug() << "OtrInternal::cbUpdateContextList()";
    static_cast<OtrInternal*>(AOpdata)->updateContextList();
	qDebug() << "OtrInternal::cbUpdateContextList(): finished!";
}

void OtrInternal::cbNewFingerprint(void* AOpdata, OtrlUserState AUserState, const char* AAccountName, const char* AProtocol, const char* AUserName, unsigned char AFingerprint[20]) {
	qDebug() << "OtrInternal::cbNewFingerprint()";
    static_cast<OtrInternal*>(AOpdata)->newFingerprint(AUserState, AAccountName, AProtocol, AUserName, AFingerprint);
}

void OtrInternal::cbWriteFingerprints(void* AOpdata) {
	qDebug() << "OtrInternal::cbWriteFingerprints()";
    static_cast<OtrInternal*>(AOpdata)->writeFingerprints();
}

void OtrInternal::cbGoneSecure(void* AOpdata, ConnContext* AContext) {
	qDebug() << "OtrInternal::cbGoneSecure()";
    static_cast<OtrInternal*>(AOpdata)->goneSecure(AContext);
}

void OtrInternal::cbGoneInsecure(void* AOpdata, ConnContext* AContext) {
	qDebug() << "OtrInternal::cbGoneInecure()";
    static_cast<OtrInternal*>(AOpdata)->goneInsecure(AContext);
}

void OtrInternal::cbStillSecure(void* AOpdata, ConnContext* AContext, int AIsReply) {
	qDebug() << "OtrInternal::cbStillSecure()";
    static_cast<OtrInternal*>(AOpdata)->stillSecure(AContext, AIsReply);
}

const char* OtrInternal::cbAccountName(void* AOpdata, const char* AAccount,
                                         const char* AProtocol) {
	qDebug() << "OtrInternal::cbAccountName()";
    return static_cast<OtrInternal*>(AOpdata)->accountName(AAccount, AProtocol);
}

void OtrInternal::cbAccountNameFree(void* AOpdata, const char* AAccountName) {
	qDebug() << "OtrInternal::cbAccountNameFree()";
    static_cast<OtrInternal*>(AOpdata)->accountNameFree(AAccountName);
}
// ---------------------------------------------------------------------------
