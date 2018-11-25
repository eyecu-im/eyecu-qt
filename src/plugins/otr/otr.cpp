#include <QDebug>

#include <QFutureWatcher>
#include <QDir>
#include <QFile>
#if QT_VERSION < 0x050000
#include <QtConcurrentRun>
#include <QEventLoop>
#else
#include <QtConcurrent>
#endif

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

#include "otr.h"
#include "otrclosure.h"
#include "otroptions.h"

#include <interfaces/ipresencemanager.h>
#include <interfaces/ixmppstreammanager.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessagestylemanager.h>
#include <interfaces/iaccountmanager.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/imessagewidgets.h>

#include <definitions/archivehandlerorders.h>
#include <definitions/menuicons.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionvalues.h>
#include <definitions/resources.h>
#include <definitions/stanzahandlerorders.h>
#include <definitions/toolbargroups.h>
#include <definitions/version.h>

#include <utils/message.h>
#include <utils/options.h>
#include <utils/logger.h>

//-----------------------------------------------------------------------------
static const char*   OTR_PROTOCOL_STRING = "prpl-jabber";
static const QString OTR_FINGERPRINTS_FILE = "otr.fingerprints";
static const QString OTR_KEYS_FILE = "otr.keys";
static const QString OTR_INSTAGS_FILE = "otr.instags";

// ============================================================================

/**
 * Handles all libotr calls and callbacks.
 */
class OtrInternal
{
public:
	OtrInternal(IOtr* AOtr):
		FUserState(),
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

	~OtrInternal()
	{
		otrl_userstate_free(FUserState);
	}

	void init()
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

	QString encryptMessage(const QString& AAccount, const QString& AContact,
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

	IOtr::MessageType decryptMessage(const QString& AAccount,
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

	QList<OtrFingerprint> getFingerprints()
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

	void verifyFingerprint(const OtrFingerprint &AFingerprint,
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

	void deleteFingerprint(const OtrFingerprint &AFingerprint)
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

	QHash<QString, QString> getPrivateKeys()
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

	void deleteKey(const QString& AAccount)
	{
		OtrlPrivKey* privKey = otrl_privkey_find(FUserState,
												 AAccount.toUtf8().constData(),
												 OTR_PROTOCOL_STRING);

		otrl_privkey_forget(privKey);

		otrl_privkey_write(FUserState, QFile::encodeName(FKeysFile).constData());
	}

	//-----------------------------------------------------------------------------

	void startSession(const QString& AAccount, const QString& AContact)
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

	void endSession(const QString& AAccount, const QString& AContact)
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

	void expireSession(const QString& AAccount, const QString& AContact)
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

	void startSMP(const QString& AAccount, const QString& AContact,
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

	void continueSMP(const QString& AAccount, const QString& AContact,
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

	void abortSMP(const QString& account, const QString& AContact)
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

	void abortSMP(ConnContext* AContext)
	{
		otrl_message_abort_smp(FUserState, &FUiOps, this, AContext);
	}

	//-----------------------------------------------------------------------------

	IOtr::MessageState getMessageState(const QString& AAccount,
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

	QString getMessageStateString(const QString& AAccount,
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

	QString getSessionId(const QString& AAccount,
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

	OtrFingerprint getActiveFingerprint(const QString& AAccount,
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

	bool isVerified(const QString& AAccount,
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

	bool isVerified(ConnContext* AContext)
	{

		if (AContext && AContext->active_fingerprint)
		{
			return (AContext->active_fingerprint->trust &&
					AContext->active_fingerprint->trust[0]);
		}

		return false;
	}

	//-----------------------------------------------------------------------------

	bool smpSucceeded(const QString& AAccount,
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

	void generateKey(const QString& AAccount)
	{
		createPrivkey(AAccount.toUtf8().constData(), OTR_PROTOCOL_STRING);
	}

	//-----------------------------------------------------------------------------

	static QString humanFingerprint(const unsigned char* AFingerprint)
	{
		char fpHash[OTRL_PRIVKEY_FPRINT_HUMAN_LEN];
		otrl_privkey_hash_to_human(fpHash, AFingerprint);
		return QString(fpHash);
	}

	//-----------------------------------------------------------------------------

protected:
	/***  implemented callback functions for libotr ***/
	static OtrlPolicy policy(IOtr::Policy APolicy)
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

	void createPrivkey(const char* AAccountname,
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
	int isLoggedIn(const char* AAccountname, const char* AProtocol,
								  const char* ARecipient) const
	{
		Q_UNUSED(AProtocol);

		return FOtr->isLoggedIn(QString::fromUtf8(AAccountname),
								QString::fromUtf8(ARecipient));
	}

	// ---------------------------------------------------------------------------

	void injectMessage(const char* AAccountname,
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

	void handleMsgEvent(OtrlMessageEvent AMsgEvent, ConnContext* AContext,
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

	void handleSmpEvent(OtrlSMPEvent ASmpEvent, ConnContext* AContext,
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

	void createInstag(const char* AAccountName, const char* AProtocol)
	{
		otrl_instag_generate(FUserState, QFile::encodeName(FInstagsFile).constData(),
							 AAccountName, AProtocol);
	}
	// ---------------------------------------------------------------------------

	void updateContextList()
	{
	}

	// ---------------------------------------------------------------------------

	void newFingerprint(OtrlUserState AUserState, const char* AAccountName,
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

	void writeFingerprints()
	{
		otrl_privkey_write_fingerprints(FUserState,
										QFile::encodeName(FFingerprintFile).constData());
	}

	// ---------------------------------------------------------------------------

	void goneSecure(ConnContext* AContext)
	{
		FOtr->stateChange(QString::fromUtf8(AContext->accountname),
						  QString::fromUtf8(AContext->username),
						  IOtr::StateChangeGoneSecure);
	}

	// ---------------------------------------------------------------------------

	void goneInsecure(ConnContext* AContext)
	{
		FOtr->stateChange(QString::fromUtf8(AContext->accountname),
						  QString::fromUtf8(AContext->username),
						  IOtr::StateChangeGoneInsecure);
	}

	// ---------------------------------------------------------------------------

	void stillSecure(ConnContext* AContext, int AIsReply)
	{
		Q_UNUSED(AIsReply);
		FOtr->stateChange(QString::fromUtf8(AContext->accountname),
						  QString::fromUtf8(AContext->username),
						  IOtr::StateChangeStillSecure);
	}

	// ---------------------------------------------------------------------------

	const char* accountName(const char* AAccount, const char* AProtocol)
	{
		Q_UNUSED(AProtocol);
		return qstrdup(FOtr->humanAccountPublic(QString::fromUtf8(AAccount))
													 .toUtf8().constData());
	}

	// ---------------------------------------------------------------------------

	void accountNameFree(const char* AAccountName)
	{
		delete [] AAccountName;
	}

	// ---------------------------------------------------------------------------
	/*** static wrapper functions ***/
	static OtrlPolicy cbPolicy(void* AOpdata, ConnContext* AContext) {
		Q_UNUSED(AOpdata);
		Q_UNUSED(AContext);
		qDebug() << "OtrInternal::cbPolicy()";
		return policy(IOtr::Policy(Options::node(OPV_OTR_POLICY).value().toInt()));
	}

	static void cbCreatePrivkey(void* APpdata, const char* AAccountName, const char* AProtocol) {
		qDebug() << "OtrInternal::cbCreatePrivkey()";
		static_cast<OtrInternal*>(APpdata)->createPrivkey(AAccountName, AProtocol);
	}

	static int cbIsLoggedIn(void* AOpdata, const char* AAccountName, const char* AProtocol, const char* ARecipient) {
		qDebug() << "OtrInternal::cbIsLoggedIn()";
		return static_cast<OtrInternal*>(AOpdata)->isLoggedIn(AAccountName, AProtocol, ARecipient);
	}

	static void cbInjectMessage(void* AOpdata, const char* AAccountname, const char* AProtocol, const char* ARecipient, const char* AMessage) {
		qDebug() << "OtrInternal::cbInjectMessage()";
		static_cast<OtrInternal*>(AOpdata)->injectMessage(AAccountname, AProtocol, ARecipient, AMessage);
	}

	static void cbHandleMsgEvent(void* AOpdata, OtrlMessageEvent AMsgEvent, ConnContext* AContext, const char* AMessage, gcry_error_t AError) {
		qDebug() << "OtrInternal::cbHandleMsgEvent()";
		static_cast<OtrInternal*>(AOpdata)->handleMsgEvent(AMsgEvent, AContext, AMessage, AError);
	}

	static void cbHandleSmpEvent(void* AOpdata, OtrlSMPEvent ASmpEvent, ConnContext* AContext, unsigned short AProgressPercent, char* AQuestion) {
		qDebug() << "OtrInternal::cbHandleSmpEvent()";
		static_cast<OtrInternal*>(AOpdata)->handleSmpEvent(ASmpEvent, AContext, AProgressPercent, AQuestion);
	}

	static void cbCreateInstag(void* AOpdata, const char* AAccountName, const char* AProtocol) {
		qDebug() << "OtrInternal::cbCreateInstag()";
		static_cast<OtrInternal*>(AOpdata)->createInstag(AAccountName, AProtocol);
	}

	static void cbUpdateContextList(void* AOpdata) {
		qDebug() << "OtrInternal::cbUpdateContextList()";
		static_cast<OtrInternal*>(AOpdata)->updateContextList();
		qDebug() << "OtrInternal::cbUpdateContextList(): finished!";
	}

	static void cbNewFingerprint(void* AOpdata, OtrlUserState AUserState,
								 const char* AAccountName, const char* AProtocol,
								 const char* AUserName, unsigned char AFingerprint[20]) {
		qDebug() << "OtrInternal::cbNewFingerprint()";
		static_cast<OtrInternal*>(AOpdata)->newFingerprint(AUserState, AAccountName, AProtocol, AUserName, AFingerprint);
	}

	static void cbWriteFingerprints(void* AOpdata) {
		qDebug() << "OtrInternal::cbWriteFingerprints()";
		static_cast<OtrInternal*>(AOpdata)->writeFingerprints();
	}

	static void cbGoneSecure(void* AOpdata, ConnContext* AContext) {
		qDebug() << "OtrInternal::cbGoneSecure()";
		static_cast<OtrInternal*>(AOpdata)->goneSecure(AContext);
	}

	static void cbGoneInsecure(void* AOpdata, ConnContext* AContext) {
		qDebug() << "OtrInternal::cbGoneInecure()";
		static_cast<OtrInternal*>(AOpdata)->goneInsecure(AContext);
	}

	static void cbStillSecure(void* AOpdata, ConnContext* AContext, int AIsReply) {
		qDebug() << "OtrInternal::cbStillSecure()";
		static_cast<OtrInternal*>(AOpdata)->stillSecure(AContext, AIsReply);
	}

	static const char* cbAccountName(void* AOpdata, const char* AAccount,
											 const char* AProtocol) {
		qDebug() << "OtrInternal::cbAccountName()";
		return static_cast<OtrInternal*>(AOpdata)->accountName(AAccount, AProtocol);
	}

	static void cbAccountNameFree(void* AOpdata, const char* AAccountName) {
		qDebug() << "OtrInternal::cbAccountNameFree()";
		static_cast<OtrInternal*>(AOpdata)->accountNameFree(AAccountName);
	}
	// ---------------------------------------------------------------------------

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

#define SHC_PRESENCE        "/presence"
#define SHC_MESSAGE         "/message"

#define SKIP_OTR_FLAG       "skip_otr_processing"

#define ADR_ACCOUNT Action::DR_Parametr1
#define ADR_CONTACT_JID Action::DR_Parametr2
#define ADR_STREAM_JID Action::DR_StreamJid

OtrFingerprint::OtrFingerprint():
	fingerprint(nullptr)
{}

OtrFingerprint::OtrFingerprint(const OtrFingerprint &fp):
	fingerprint(fp.fingerprint),
	account(fp.account),
	username(fp.username),
	fingerprintHuman(fp.fingerprintHuman),
	trust(fp.trust)
{}

OtrFingerprint::OtrFingerprint(unsigned char* fingerprint,
						 QString account, QString username,
						 QString trust):
	fingerprint(fingerprint),
	account(account),
	username(username),
	trust(trust)
{
	fingerprintHuman = OtrInternal::humanFingerprint(fingerprint);
}

Otr::Otr() :
	FOtrInternal(new OtrInternal(this)),
    FOptionsManager(nullptr),
    FAccountManager(nullptr),
    FPresenceManager(nullptr),
    FMessageProcessor(nullptr)
{
}

Otr::~Otr()
{
	delete FOtrInternal;
}

void Otr::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Off-the-Record Messaging Plugin");
    APluginInfo->description = tr("Off-the-Record (OTR) Messaging allows you to have private conversations over instant messaging");
    APluginInfo->version = "1.0.3";
    APluginInfo->author = "John Smith";
    APluginInfo->homePage = "https://github.com/xnamed";    
    APluginInfo->dependences.append(MESSAGEPROCESSOR_UUID);
    APluginInfo->dependences.append(ACCOUNTMANAGER_UUID);
    APluginInfo->dependences.append(STANZAPROCESSOR_UUID);
    APluginInfo->dependences.append(XMPPSTREAMS_UUID);
}

bool Otr::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(AInitOrder);

    IPlugin *plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0,nullptr);
    if (plugin)
        FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IPresenceManager").value(0);
    if (plugin)
    {
        FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());
        if (FPresenceManager)
        {
            connect(FPresenceManager->instance(),SIGNAL(presenceOpened(IPresence *)),SLOT(onPresenceOpened(IPresence *)));
        }
    }

    plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0,nullptr);
    if (plugin)
    {
        IXmppStreamManager *FXmppStreams = qobject_cast<IXmppStreamManager *>(plugin->instance());
        if (FXmppStreams)
        {
			connect(FXmppStreams->instance(), SIGNAL(streamOpened(IXmppStream *)), SLOT(onStreamOpened(IXmppStream *)));
			connect(FXmppStreams->instance(), SIGNAL(streamClosed(IXmppStream *)), SLOT(onStreamClosed(IXmppStream *)));
        }
    }

    plugin = APluginManager->pluginInterface("IOptionsManager").value(0,nullptr);
    if (plugin)
    {
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
        if (FOptionsManager)
        {
            connect(FOptionsManager->instance(),SIGNAL(profileOpened(const QString &)),SLOT(onProfileOpened(const QString &)));
        }
    }

    plugin = APluginManager->pluginInterface("IMessageArchiver").value(0);
    if (plugin)
        FMessageArchiver = qobject_cast<IMessageArchiver *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IAccountManager").value(0,nullptr);
    FAccountManager = qobject_cast<IAccountManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IMessageProcessor").value(0,nullptr);
    if (plugin)
        FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,nullptr);
    if (plugin)
    {
        FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
        if (FMessageWidgets)
        {
            connect(FMessageWidgets->instance(), SIGNAL(toolBarWidgetCreated(IMessageToolBarWidget *)), SLOT(onToolBarWidgetCreated(IMessageToolBarWidget *)));
			connect(FMessageWidgets->instance(), SIGNAL(normalWindowCreated(IMessageNormalWindow *)), SLOT(onMessageWindowCreated(IMessageNormalWindow *)));
			connect(FMessageWidgets->instance(), SIGNAL(normalWindowDestroyed(IMessageNormalWindow *)), SLOT(onMessageWindowDestroyed(IMessageNormalWindow *)));
            connect(FMessageWidgets->instance(), SIGNAL(chatWindowCreated(IMessageChatWindow *)), SLOT(onChatWindowCreated(IMessageChatWindow *)));
            connect(FMessageWidgets->instance(), SIGNAL(chatWindowDestroyed(IMessageChatWindow *)), SLOT(onChatWindowDestroyed(IMessageChatWindow *)));
        }
    }

    return (FStanzaProcessor != NULL);
}

bool Otr::initObjects()
{
    return true;
}

bool Otr::initSettings()
{
	Options::setDefaultValue(OPV_OTR_POLICY, PolicyEnabled);
	Options::setDefaultValue(OPV_OTR_ENDWHENOFFLINE, false);
    if (FOptionsManager)
    {
        IOptionsDialogNode otrNode = { ONO_OTR, OPN_OTR, MNI_OTR_ENCRYPTED, tr("OTR Messaging") };
        FOptionsManager->insertOptionsDialogNode(otrNode);
        FOptionsManager->insertOptionsDialogHolder(this);		
    }
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> Otr::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
    Q_UNUSED(AParent);
    QMultiMap<int, IOptionsDialogWidget *> widgets;
    if (ANodeId == OPN_OTR)
		widgets.insertMulti(ONO_OTR, new OtrOptions(this, AParent));
    return widgets;
}

bool Otr::archiveMessageEdit(int AOrder, const Jid &AStreamJid, Message &AMessage, bool ADirectionIn)
{
    Q_UNUSED(AOrder);
    Q_UNUSED(AStreamJid);
    Q_UNUSED(ADirectionIn);

    return AMessage.stanza().attribute(SKIP_OTR_FLAG) != "true";
}

void Otr::onStreamOpened( IXmppStream *AXmppStream )
{
    Q_UNUSED(AXmppStream);
}

void Otr::onStreamClosed( IXmppStream *AXmppStream )
{
	QString account = FAccountManager->findAccountByStream(AXmppStream->streamJid())->accountId().toString();

	if (FOnlineUsers.contains(account))
    {
		foreach(QString contact, FOnlineUsers.value(account).keys())
        {
			FOtrInternal->endSession(account, contact);
			FOnlineUsers[account][contact]->setIsLoggedIn(false);
        }
    }
}

void Otr::onToolBarWidgetCreated(IMessageToolBarWidget *)
{
}

void Otr::onMessageWindowCreated(IMessageNormalWindow *)
{
}

void Otr::onMessageWindowDestroyed(IMessageNormalWindow *)
{
}

void Otr::onChatWindowCreated(IMessageChatWindow *AWindow)
{
    QString account = FAccountManager->findAccountByStream(AWindow->streamJid())->accountId().toString();
    QString contact = AWindow->contactJid().uFull();
	QString stream = AWindow->streamJid().uFull();
	Action *otrAction = new Action(AWindow->toolBarWidget()->instance());
	otrAction->setData(ADR_ACCOUNT, account);
	otrAction->setData(ADR_CONTACT_JID, contact);

	Menu *menu = new Menu();
	QActionGroup *actionGroup = new QActionGroup(menu);
	otrAction->setMenu(menu);

	// 0: Session initiate
	Action *action = new Action(menu);
	action->setData(ADR_ACCOUNT, account);
	action->setData(ADR_CONTACT_JID, contact);
	connect(action, SIGNAL(triggered()), SLOT(onSessionInitiate()));
	action->setActionGroup(actionGroup);
	menu->addAction(action);

	// 1: End private conversation
	action = new Action(menu);
	action->setData(ADR_ACCOUNT, account);
	action->setData(ADR_CONTACT_JID, contact);
	action->setData(ADR_STREAM_JID, stream);
	action->setText(tr("&End private conversation"));
	connect(action, SIGNAL(triggered()), SLOT(onSessionEnd()));
	action->setActionGroup(actionGroup);
	menu->addAction(action);

	// 2: Separator
	menu->insertSeparator(NULL);

	// 3: Authenticate contact
	action = new Action(menu);
	action->setText(tr("&Authenticate contact"));
	action->setData(ADR_ACCOUNT, account);
	action->setData(ADR_CONTACT_JID, contact);
	connect(action, SIGNAL(triggered()), SLOT(onContactAuthenticate()));
	action->setActionGroup(actionGroup);
	menu->addAction(action);

	// 4: Show secure session ID
	action = new Action(menu);
	action->setText(tr("Show secure session &ID"));
	action->setData(ADR_ACCOUNT, account);
	action->setData(ADR_CONTACT_JID, contact);
	connect(action, SIGNAL(triggered()), SLOT(onSessionID()));
	action->setActionGroup(actionGroup);
	menu->addAction(action);

	// 5: Show own fingerprint
	action = new Action(menu);
	action->setText(tr("Show own &fingerprint"));
	action->setData(ADR_ACCOUNT, account);
	action->setData(ADR_CONTACT_JID, contact);
	connect(action, SIGNAL(triggered()), SLOT(onFingerprint()));
	action->setActionGroup(actionGroup);
	menu->addAction(action);

	menu->setToolTip(tr("OTR Messaging"));

	QToolButton *otrButton = AWindow->toolBarWidget()->toolBarChanger()->insertAction(otrAction, TBG_MWTBW_OTR);
	otrButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	otrButton->setPopupMode(QToolButton::InstantPopup);

	connect(AWindow->address()->instance(), SIGNAL(addressChanged(const Jid &, const Jid &)),
											SLOT(onWindowAddressChanged(const Jid &, const Jid &)));
	connect(this,SIGNAL(otrStateChanged(const Jid &, const Jid &)),SLOT(onUpdateMessageState(const Jid &, const Jid &)));

	onUpdateMessageState(AWindow->streamJid(), AWindow->contactJid());
}

void Otr::onChatWindowDestroyed(IMessageChatWindow *AWindow)
{
    Q_UNUSED(AWindow)
}

void Otr::onProfileOpened(const QString &AProfile)
{
	FHomePath = FOptionsManager->profilePath(AProfile);
	FOtrInternal->init();
}

// OTR tool button slots
void Otr::onSessionInitiate()
{
	Action *action = qobject_cast<Action *>(sender());
	QString account = action->data(ADR_ACCOUNT).toString();
	QString contact = action->data(ADR_CONTACT_JID).toString();
	FOtrInternal->startSession(account, contact);
}

void Otr::onSessionEnd()
{
	Action *action = qobject_cast<Action *>(sender());
	QString account = action->data(ADR_ACCOUNT).toString();
	QString contact = action->data(ADR_CONTACT_JID).toString();
	QString streamJid = action->data(ADR_STREAM_JID).toString();
	FOtrInternal->endSession(account, contact);
	onUpdateMessageState(streamJid, contact);
}

void Otr::onContactAuthenticate()
{
	Action *action = qobject_cast<Action *>(sender());
	QString account = action->data(ADR_ACCOUNT).toString();
	QString contact = action->data(ADR_CONTACT_JID).toString();
	authenticateContact(account, contact);
}

void Otr::onSessionID()
{
	Action *action = qobject_cast<Action *>(sender());
	QString account = action->data(ADR_ACCOUNT).toString();
	QString contact = action->data(ADR_CONTACT_JID).toString();
	QString sId = FOtrInternal->getSessionId(account, contact);
	QString msg;

	if (sId.isEmpty())
	{
		msg = tr("No active encrypted session");
	}
	else
	{
		msg = tr("Session ID between account \"%1\" and %2: %3")
				.arg(humanAccount(account))
				.arg(contact)
				.arg(sId);
	}

	displayOtrMessage(account, contact, msg);
}

void Otr::onFingerprint()
{
	Action *action = qobject_cast<Action *>(sender());
	QString account = action->data(ADR_ACCOUNT).toString();
	QString contact = action->data(ADR_CONTACT_JID).toString();
	QString fingerprint = getPrivateKeys()
							.value(account, tr("No private key for account \"%1\"")
								.arg(humanAccount(account)));

	QString msg(tr("Fingerprint for account \"%1\": %2")
				   .arg(humanAccount(account))
				   .arg(fingerprint));

	displayOtrMessage(account, contact, msg);
}

void Otr::onWindowAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore)
{
	Q_UNUSED(AStreamBefore)
	Q_UNUSED(AContactBefore)
	IMessageAddress *address = qobject_cast<IMessageAddress *>(sender());
	onUpdateMessageState(address->streamJid(), address->contactJid());
}

void Otr::onUpdateMessageState(const Jid &AStreamJid, const Jid &AContactJid)
{
	IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
	if (window)
	{
		if (window->streamJid()==AStreamJid && window->contactJid()==AContactJid.full())
		{
			QList<QAction*> otrActions = window->toolBarWidget()->toolBarChanger()->groupItems(TBG_MWTBW_OTR);
			QAction *otrActionHandle = otrActions.first();
			Action *otrAction = window->toolBarWidget()->toolBarChanger()->handleAction(otrActionHandle);
			if (otrAction)
			{
				QString contact = otrAction->data(ADR_CONTACT_JID).toString();
				QString account = otrAction->data(ADR_ACCOUNT).toString();

				QString iconKey;
				IOtr::MessageState state = getMessageState(account, contact);
				QString stateString(getMessageStateString(account, contact));

				if (state == IOtr::MsgStateEncrypted)
				{
					if (isVerified(account, contact))
					{
						iconKey = MNI_OTR_ENCRYPTED;
						otrAction->setIcon(RSR_STORAGE_MENUICONS, MNI_OTR_ENCRYPTED);
					}
					else
					{
						iconKey = MNI_OTR_UNVERFIFIED;
						stateString += ", " + tr("unverified");
					}
				}
				else
				{
					iconKey = MNI_OTR_NO;
				}

				otrAction->setText(tr("OTR Messaging [%1]").arg(stateString));
				otrAction->setIcon(RSR_STORAGE_MENUICONS, iconKey);

				QList<Action *> actions = otrAction->menu()->actions();
				if (state == IOtr::MsgStateEncrypted)
				{
					// Session initiate
					actions[0]->setText(tr("Refre&sh private conversation"));
					// End private conversation
					actions[1]->setEnabled(true);
					// Authenticate contact
					actions[3]->setEnabled(true);
					// Show session ID
					actions[4]->setEnabled(true);
				}
				else
				{
					// Session initiate
					actions[0]->setText(tr("&Start private conversation"));
					if (state == IOtr::MsgStatePlaintext)
					{
						// End private conversation
						actions[1]->setEnabled(false);
						// Authenticate contact
						actions[3]->setEnabled(false);
						// Show session ID
						actions[4]->setEnabled(false);
					}
					else // finished, unknown
					{
						// End private conversation
						actions[1]->setEnabled(true);
						// Authenticate contact
						actions[3]->setEnabled(false);
						// Show session ID
						actions[4]->setEnabled(false);
					}
				}

				if (Options::node(OPV_OTR_POLICY).value().toInt() < IOtr::PolicyEnabled)
				{
					// Session initiate
					actions[0]->setEnabled(false);
					// End private conversation
					actions[1]->setEnabled(false);
				}
			}
		}
	}
}

void Otr::onPresenceOpened(IPresence *APresence)
{
    Q_UNUSED(APresence)

    if (FStanzaProcessor)
    {
        IStanzaHandle shandle;
        shandle.handler = this;
        shandle.order = SHO_OTR;
        shandle.direction = IStanzaHandle::DirectionIn;
        shandle.conditions.append(SHC_PRESENCE);
        FSHIPresence = FStanzaProcessor->insertStanzaHandle(shandle);
        //
        IStanzaHandle handle_in;
        handle_in.handler = this;
        handle_in.order = SHO_OTR; // SHO
        handle_in.direction = IStanzaHandle::DirectionIn;
        handle_in.conditions.append(SHC_MESSAGE);

        IStanzaHandle handle_out;
        handle_out.handler = this;
        handle_out.order = SHO_OTR; // SHO
        handle_out.direction = IStanzaHandle::DirectionOut;
        handle_out.conditions.append(SHC_MESSAGE);

        FSHIMessage = FStanzaProcessor->insertStanzaHandle(handle_in);
        FSHOMessage = FStanzaProcessor->insertStanzaHandle(handle_out);
    }
}

//-----------------------------------------------------------------------------

void Otr::authenticateContact(const QString &AAccount, const QString &AContact)
{
	if (!FOnlineUsers.value(AAccount).contains(AContact))
		FOnlineUsers[AAccount][AContact] = new OtrClosure(AAccount, AContact, this);

	FOnlineUsers[AAccount][AContact]->authenticateContact();
}

//-----------------------------------------------------------------------------

QString Otr::dataDir()
{
	return FHomePath;
}

//-----------------------------------------------------------------------------

void Otr::sendMessage(const QString &account, const QString &contact, const QString& AMessage)
{
    Message message;
	message.setType(Message::Chat).setBody(AMessage);

    if (!message.body().isEmpty())
    {
        message.setTo(contact);//.setId(id);
        message.stanza().setAttribute(SKIP_OTR_FLAG, "true");
        FMessageProcessor->sendMessage(FAccountManager->findAccountById(account)->streamJid(), message, IMessageProcessor::DirectionOut);
    }

}

//-----------------------------------------------------------------------------

bool Otr::isLoggedIn(const QString &AAccount, const QString &AContact) const
{
	if (FOnlineUsers.contains(AAccount) &&
		FOnlineUsers.value(AAccount).contains(AContact))
		return FOnlineUsers.value(AAccount).value(AContact)->isLoggedIn();
    return false;
}

//-----------------------------------------------------------------------------

void Otr::notifyUser(const QString &AAccount, const QString &AContact,
					 const QString& AMessage, const NotifyType& AType)
{
	Q_UNUSED(AMessage);
	Q_UNUSED(AType);

	LOG_STRM_INFO(FAccountManager->findAccountById(AAccount)->streamJid(),QString("OTR notifyUser, contact=%1").arg(AContact));
}

//-----------------------------------------------------------------------------

bool Otr::displayOtrMessage(const QString &AAccount, const QString &AContact,
							const QString& AMessage)
{
	Jid contactJid(AContact);
	notifyInChatWindow(FAccountManager->findAccountById(AAccount)->streamJid(),contactJid, AMessage);
	LOG_STRM_INFO(FAccountManager->findAccountById(AAccount)->streamJid(),QString("OTR displayOtrMessage, contact=%1").arg(AContact));
    return true;
}

//-----------------------------------------------------------------------------

void Otr::stateChange(const QString &AAccount, const QString &AContact,
					  StateChange AChange)
{
	LOG_STRM_INFO(FAccountManager->findAccountById(AAccount)->streamJid(),QString("OTR stateChange, contact=%1").arg(AContact));

	if (!FOnlineUsers.value(AAccount).contains(AContact))
    {
		FOnlineUsers[AAccount][AContact] = new OtrClosure(AAccount, AContact, this);
    }

	bool verified  = isVerified(AAccount, AContact);
	bool encrypted = FOnlineUsers[AAccount][AContact]->encrypted();
    QString msg;

	switch (AChange)
    {
		case StateChangeGoingSecure:
            msg = encrypted?
                      tr("Attempting to refresh the private conversation")
                    : tr("Attempting to start a private conversation");
            break;

		case StateChangeGoneSecure:
            msg  = verified? tr("Private conversation started")
                           : tr("Unverified conversation started");
            break;

		case StateChangeGoneInsecure:
            msg  = tr("Private conversation lost");
            break;

		case StateChangeClose:
            msg  = tr("Private conversation closed");
            break;

		case StateChangeRemoteClose:
            msg  = tr("%1 has ended the private conversation with you; "
                      "you should do the same.")
					  .arg(humanContact(AAccount, AContact));
            break;

		case StateChangeStillSecure:
            msg  = verified? tr("Private conversation refreshed")
                           : tr("Unverified conversation refreshed");
            break;

		case StateChangeTrust:
            msg  = verified? tr("Contact authenticated")
                           : tr("Contact not authenticated");
            break;
    }

	Jid contactJid(AContact);
	notifyInChatWindow(FAccountManager->findAccountById(AAccount)->streamJid(),contactJid, msg);
	emit otrStateChanged(FAccountManager->findAccountById(AAccount)->streamJid(),contactJid);
}

//-----------------------------------------------------------------------------

void Otr::receivedSMP(const QString &AAccount, const QString &AContact,
					  const QString& AQuestion)
{
	LOG_STRM_INFO(FAccountManager->findAccountById(AAccount)->streamJid(),QString("OTR receivedSMP, contact=%1").arg(AContact));

	if (FOnlineUsers.contains(AAccount) &&
		FOnlineUsers.value(AAccount).contains(AContact))
    {
		FOnlineUsers[AAccount][AContact]->receivedSMP(AQuestion);
    }
}

//-----------------------------------------------------------------------------

void Otr::updateSMP(const QString &AAccount, const QString &AContact,
							 int AProgress)
{
	LOG_STRM_INFO(FAccountManager->findAccountById(AAccount)->streamJid(),QString("OTR updateSMP, contact=%1").arg(AContact));

	if (FOnlineUsers.contains(AAccount) &&
		FOnlineUsers.value(AAccount).contains(AContact))
		FOnlineUsers[AAccount][AContact]->updateSMP(AProgress);
}

//-----------------------------------------------------------------------------

QString Otr::humanAccount(const QString& AAccountId)
{
	return FAccountManager->findAccountById(AAccountId)->streamJid().bare();
}

//-----------------------------------------------------------------------------

QString Otr::humanAccountPublic(const QString& AAccountId)
{
	return FAccountManager->findAccountById(AAccountId)->streamJid().bare();
}

//-----------------------------------------------------------------------------

QString Otr::humanContact(const QString& AAccountId,
								   const QString &AContactJid)
{
	Q_UNUSED(AAccountId)
    return AContactJid;
}

//-----------------------------------------------------------------------------

void Otr::notifyInChatWindow(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessage) const
{
    IMessageChatWindow *window = FMessageWidgets ? FMessageWidgets->findChatWindow(AStreamJid,AContactJid,true)
                                                 : nullptr;
    if (window)
    {
        IMessageStyleContentOptions options;
        options.kind = IMessageStyleContentOptions::KindStatus;
        options.type |= IMessageStyleContentOptions::TypeEvent;
        options.direction = IMessageStyleContentOptions::DirectionIn;
        options.time = QDateTime::currentDateTime();
        window->viewWidget()->appendText(AMessage,options);
	}
}

bool Otr::stanzaReadWrite(int AHandlerId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
    Q_UNUSED(AAccept)

    if (AHandlerId == FSHIPresence)
    {
        QDomElement xml = AStanza.document().firstChildElement("presence");
        if (!xml.isNull())
        {
            QString contact = AStanza.from();
			QString account = FAccountManager->findAccountByStream(AStreamJid)->accountId().toString();

            if (AStanza.type() == PRESENCE_TYPE_AVAILABLE)
            {
				if (!FOnlineUsers.value(account).contains(contact))
					FOnlineUsers[account][contact] = new OtrClosure(account, contact, this);
				FOnlineUsers[account][contact]->setIsLoggedIn(true);
            }
            else if (AStanza.type() == PRESENCE_TYPE_UNAVAILABLE)
            {
				if (FOnlineUsers.contains(account) &&
					FOnlineUsers.value(account).contains(contact))
                {
					if (Options::node(OPV_OTR_ENDWHENOFFLINE).value().toBool())
						FOtrInternal->expireSession(account, contact);
					FOnlineUsers[account][contact]->setIsLoggedIn(false);
                    Jid contactJid(AStanza.from());
                    emit otrStateChanged(AStreamJid,contactJid);
                }
            }
        }
    }
    else if (AHandlerId == FSHIMessage || AHandlerId == FSHOMessage)
    {
        Message message(AStanza);
        if (message.type() != Message::Chat)
            return false;

        if (message.body().isEmpty())
            return false;

        if (message.stanza().attribute(SKIP_OTR_FLAG) != "true")
        {
            if (AHandlerId == FSHOMessage)
            {
                QString contact = message.to();
                QString account = FAccountManager->findAccountByStream(AStreamJid)->accountId().toString();

				qDebug() << "Encrypting message...";
				QString encrypted = FOtrInternal->encryptMessage(account, contact, message.body());
				qDebug() << "Done! Encrypted message:" << encrypted;
                message.setBody(encrypted);

                //if there has been an error, drop the message
                if (encrypted.isEmpty())
                    return true;

                AStanza = message.stanza();


                /*if (!m_onlineUsers.value(account).contains(contact))
                {
                    m_onlineUsers[account][contact] = new PsiOtrClosure(account, contact,
                                                                        m_otrConnection);
                }*/
                //if (m_onlineUsers[account][contact]->encrypted()) {
				if (getMessageState(account, contact) == IOtr::MsgStateEncrypted)
                {
                    if (AStanza.to().contains("/")) // if not a bare jid
                        AStanza.document().appendChild(AStanza.document().createElementNS("urn:xmpp:hints" ,"no-copy")).toElement();
                    AStanza.document().appendChild(AStanza.document().createElementNS("urn:xmpp:hints", "no-permanent-store")).toElement();
                    AStanza.document().appendChild(AStanza.document().createElementNS("urn:xmpp:carbons:2", "private")).toElement();
                }

                return false;
            }
            else
            {
                bool ignore = false;
				qDebug() << "AStanza=" << AStanza.toString();

                QString contact = message.from();
                QString account = FAccountManager->findAccountByStream(AStreamJid)->accountId().toString();
                QString plainBody = message.body();

                QString decrypted;
				qDebug() << "Decrypting message...";
				qDebug() << "Encrypted message:" << plainBody;
				IOtr::MessageType messageType = FOtrInternal->decryptMessage(account, contact,
																			 plainBody, decrypted);
				qDebug() << "Decrypted message:" << decrypted;
                switch (messageType)
                {
                    case IOtr::MsgTypeNone:
                        break;
                    case IOtr::MsgTypeIgnore:
                        ignore = true;
                        break;
                    case IOtr::MsgTypeOtr:
                        QString bodyText;

                        bodyText = decrypted;

                        message.setBody(bodyText);
                        AStanza = message.stanza();
                        break;
                }
                return ignore;
            }
        }
        else
        {
            message.stanza().element().removeAttribute("skip_otr_processing");
        }
    }

    return false;
}

QList<OtrFingerprint> Otr::getFingerprints()
{
	return FOtrInternal->getFingerprints();
}

void Otr::deleteFingerprint(const OtrFingerprint &AFingerprint)
{
	return FOtrInternal->deleteFingerprint(AFingerprint);
}

void Otr::verifyFingerprint(const OtrFingerprint& AFingerprint, bool AVerified)
{
	FOtrInternal->verifyFingerprint(AFingerprint, AVerified);
}

QHash<QString, QString> Otr::getPrivateKeys()
{
	return FOtrInternal->getPrivateKeys();
}

void Otr::deleteKey(const QString& AAccount)
{
	FOtrInternal->deleteKey(AAccount);
}

void Otr::startSMP(const QString& AAccount, const QString& AContact,
				   const QString& AQuestion, const QString& ASecret)
{
	FOtrInternal->startSMP(AAccount, AContact, AQuestion, ASecret);
}

void Otr::continueSMP(const QString& AAccount, const QString& AContact,
					  const QString& ASecret)
{
	FOtrInternal->continueSMP(AAccount, AContact, ASecret);
}

void Otr::abortSMP(const QString& AAccount, const QString& AContact)
{
	FOtrInternal->abortSMP(AAccount, AContact);
}

IOtr::MessageState Otr::getMessageState(const QString& AAccount, const QString& AContact)
{
	return FOtrInternal->getMessageState(AAccount, AContact);
}

QString Otr::getMessageStateString(const QString& AAccount, const QString& AContact)
{
	return FOtrInternal->getMessageStateString(AAccount, AContact);
}

OtrFingerprint Otr::getActiveFingerprint(const QString& AAccount,
										 const QString& AContact)
{
	return FOtrInternal->getActiveFingerprint(AAccount, AContact);
}

bool Otr::isVerified(const QString& AAccount, const QString& AContact)
{
	return FOtrInternal->isVerified(AAccount, AContact);
}

bool Otr::smpSucceeded(const QString& AAccount, const QString& AContact)
{
	return FOtrInternal->smpSucceeded(AAccount, AContact);
}

void Otr::generateKey(const QString& AAccount)
{
	FOtrInternal->generateKey(AAccount);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_otr, Otr)
#endif
