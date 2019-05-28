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
#include <interfaces/inotifications.h>
#include <interfaces/irostersview.h>

#include <definitions/archivehandlerorders.h>
#include <definitions/menuicons.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionvalues.h>
#include <definitions/resources.h>
#include <definitions/stanzahandlerorders.h>
#include <definitions/toolbargroups.h>
#include <definitions/tabpagenotifypriorities.h>
#include "definitions/notificationdataroles.h"
#include "definitions/notificationtypeorders.h"
#include "definitions/notificationtypes.h"
#include "definitions/rosternotifyorders.h"
#include "definitions/soundfiles.h"
#include <definitions/namespaces.h>
#include <definitions/version.h>

#include <utils/message.h>
#include <utils/options.h>
#include <utils/logger.h>

//-----------------------------------------------------------------------------
static const char*   OTR_PROTOCOL_STRING = "prpl-jabber";
static const QString OTR_FINGERPRINTS_FILE = "otr.fingerprints";
static const QString OTR_KEYS_FILE = "otr.keys";
static const QString OTR_INSTAGS_FILE = "otr.instags";

#define OTR_WEB "https://otr.cypherpunks.ca/"

// ============================================================================

///
/// Private implementation of OTR functionality.
///
class OtrPrivate
{
public:
	///
	/// \brief OtrPrivate class constructor
	/// \param AOtr OTR plugin object
	///
	OtrPrivate(Otr* AOtr):
		FUserState(),
		FUiOps(),
		FOtr(AOtr),
		FIsGenerating(false)
	{
		OTRL_INIT;
		FUserState                 = otrl_userstate_create();
		FUiOps.policy              = (*OtrPrivate::cbPolicy);
		FUiOps.create_privkey      = (*OtrPrivate::cbCreatePrivkey);
		FUiOps.is_logged_in        = (*OtrPrivate::cbIsLoggedIn);
		FUiOps.inject_message      = (*OtrPrivate::cbInjectMessage);
		FUiOps.update_context_list = (*OtrPrivate::cbUpdateContextList);
		FUiOps.new_fingerprint     = (*OtrPrivate::cbNewFingerprint);
		FUiOps.write_fingerprints  = (*OtrPrivate::cbWriteFingerprints);
		FUiOps.gone_secure         = (*OtrPrivate::cbGoneSecure);
		FUiOps.gone_insecure       = (*OtrPrivate::cbGoneInsecure);
		FUiOps.still_secure        = (*OtrPrivate::cbStillSecure);

		FUiOps.max_message_size    = nullptr;
		FUiOps.account_name        = (*OtrPrivate::cbAccountName);
		FUiOps.account_name_free   = (*OtrPrivate::cbAccountNameFree);

		FUiOps.handle_msg_event    = (*OtrPrivate::cbHandleMsgEvent);
		FUiOps.handle_smp_event    = (*OtrPrivate::cbHandleSmpEvent);
		FUiOps.create_instag       = (*OtrPrivate::cbCreateInstag);
	}

	///
	/// \brief OtrPrivate class destructor
	///
	~OtrPrivate()
	{
		otrl_userstate_free(FUserState);
	}

	//-----------------------------------------------------------------------------

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

	QString encryptMessage(const Jid& AStreamJid, const Jid& AContactJid, const QString& AMessage)
	{
		char* encMessage = nullptr;
		gcry_error_t err;

		err = otrl_message_sending(FUserState, &FUiOps, this,
								   AStreamJid.full().toUtf8().constData(),
								   OTR_PROTOCOL_STRING,
								   AContactJid.full().toUtf8().constData(),
								   OTRL_INSTAG_BEST, AMessage.toUtf8().constData(),
								   nullptr, &encMessage,
								   OTRL_FRAGMENT_SEND_SKIP,
								   nullptr, nullptr, nullptr);
		if (err)
		{
			QString err_message = QObject::tr("Encrypting message to %1 "
											  "failed.\nThe message was not sent.")
											  .arg(AContactJid.full());
			FOtr->displayOtrMessage(AStreamJid, AContactJid, err_message);
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

	IOtr::MessageType decryptMessage(const Jid& AStreamJid,
									 const Jid& AContactJid,
									 const QString& AMessage,
									 QString& ADecrypted)
	{
		int ignoreMessage = 0;
		char* newMessage  = nullptr;
		OtrlTLV* tlvs     = nullptr;
		OtrlTLV* tlv      = nullptr;

		ignoreMessage = otrl_message_receiving(FUserState, &FUiOps, this,
											   AStreamJid.full().toUtf8().constData(),
											   OTR_PROTOCOL_STRING,
											   AContactJid.full().toUtf8().constData(),
											   AMessage.toUtf8().constData(),
											   &newMessage, &tlvs, nullptr,
											   nullptr, nullptr);

		tlv = otrl_tlv_find(tlvs, OTRL_TLV_DISCONNECTED);
		if (tlv)
		{
			FOtr->stateChange(AStreamJid, AContactJid, IOtr::StateChangeRemoteClose);
			endSession(AStreamJid, AContactJid);
		}

		// Magic hack to force it work similar to libotr < 4.0.0.
		// If user received unencrypted message he (she) should be notified.
		// See OTRL_MSGEVENT_RCVDMSG_UNENCRYPTED as well.
		if (ignoreMessage && !newMessage && !AMessage.startsWith("?OTR"))
			ignoreMessage = 0;

		otrl_tlv_free(tlvs);

		if (ignoreMessage == 1) // Internal protocol message
			return IOtr::MsgTypeIgnore;
		else if ((ignoreMessage == 0) && newMessage)
		{ // Message has been decrypted, replace it
			ADecrypted = QString::fromUtf8(newMessage);
			otrl_message_free(newMessage);
			return IOtr::MsgTypeOtr;
		}

		return IOtr::MsgTypeNone;
	}

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

	void verifyFingerprint(const OtrFingerprint &AFingerprint,
										bool AVerified)
	{
		ConnContext* context = otrl_context_find(FUserState,
												 AFingerprint.FContactJid.full().toUtf8().constData(),
												 AFingerprint.FStreamJid.full().toUtf8().constData(),
												 OTR_PROTOCOL_STRING,
												 OTRL_INSTAG_BEST,
												 false, nullptr, nullptr, nullptr);
		if (context)
		{
			::Fingerprint* fp = otrl_context_find_fingerprint(context,
															  AFingerprint.FFingerprint,
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

	void deleteFingerprint(const OtrFingerprint &AFingerprint)
	{
		ConnContext* context = otrl_context_find(FUserState,
												 AFingerprint.FContactJid.full().toUtf8().constData(),
												 AFingerprint.FStreamJid.full().toUtf8().constData(),
												 OTR_PROTOCOL_STRING,
												 OTRL_INSTAG_BEST,
												 false, nullptr, nullptr, nullptr);
		if (context)
		{
			::Fingerprint* fp = otrl_context_find_fingerprint(context,
															  AFingerprint.FFingerprint,
															  0, nullptr);
			if (fp)
			{
				if (context->active_fingerprint == fp)
					otrl_context_force_finished(context);
				otrl_context_forget_fingerprint(fp, true);
				writeFingerprints();
			}
		}
	}

	//-----------------------------------------------------------------------------

	QHash<Jid, QString> getPrivateKeys()
	{
		QHash<Jid, QString> privKeyList;
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
				privKeyList.insert(QString::fromUtf8(privKey->accountname),
								   QString(fingerprintBuf));
		}

		return privKeyList;
	}

	//-----------------------------------------------------------------------------

	void deleteKey(const Jid& AStreamJid)
	{
		OtrlPrivKey* privKey = otrl_privkey_find(FUserState,
												 AStreamJid.full().toUtf8().constData(),
												 OTR_PROTOCOL_STRING);
		otrl_privkey_forget(privKey);
		otrl_privkey_write(FUserState, QFile::encodeName(FKeysFile).constData());
	}

	//-----------------------------------------------------------------------------

	void startSession(const Jid& AStreamJid, const Jid& AContactJid)
	{
		FOtr->stateChange(AStreamJid, AContactJid, IOtr::StateChangeGoingSecure);

		if (!otrl_privkey_find(FUserState, AStreamJid.full().toUtf8().constData(),
							   OTR_PROTOCOL_STRING))
			createPrivkey(AStreamJid.full().toUtf8().constData(), OTR_PROTOCOL_STRING);

		//TODO: make allowed otr versions configureable
		char* msg = otrl_proto_default_query_msg(FOtr->humanAccountPublic(AStreamJid).toUtf8().constData(),
												 OTRL_POLICY_DEFAULT);
		QString otrMsg(QString::fromUtf8(msg));

		QString otr("Off-the-Record private conversation");

		QString msgTmpl("%1 has requested an %2. "
						"However, you do not have a plugin to support that.\n"
						"See %3 for more information.");

		QString tmpl("%1\n%2");

		otrMsg = tmpl.arg(otrMsg.split('\n').first())
					 .arg(msgTmpl.arg(FOtr->humanAccountPublic(AStreamJid))
								 .arg(otr)
								 .arg(OTR_WEB));

		QString html = msgTmpl.arg(QString("<b>%1</b>").arg(FOtr->humanAccountPublic(AStreamJid)))
							  .arg(QString("<a href=\"%1\">%2</a>").arg(OTR_WEB).arg(otr))
							  .arg(QString("<a href=\"%1\">%1</a>").arg(OTR_WEB));

		FOtr->sendMessage(AStreamJid, AContactJid, otrMsg, html);

		free(msg);
	}

	//-----------------------------------------------------------------------------

	void endSession(const Jid& AStreamJid, const Jid& AContactJid)
	{
		ConnContext* context = otrl_context_find(FUserState,
												 AContactJid.full().toUtf8().constData(),
												 AStreamJid.full().toUtf8().constData(),
												 OTR_PROTOCOL_STRING,
												 OTRL_INSTAG_BEST,
												 false, nullptr, nullptr, nullptr);

		if (context && (context->msgstate == OTRL_MSGSTATE_ENCRYPTED))
			FOtr->stateChange(AStreamJid, AContactJid, IOtr::StateChangeClose);

		otrl_message_disconnect(FUserState, &FUiOps, this,
								AStreamJid.full().toUtf8().constData(),
								OTR_PROTOCOL_STRING,
								AContactJid.full().toUtf8().constData(),
								OTRL_INSTAG_BEST);
	}

	void expireSession(const Jid& AStreamJid, const Jid& AContactJid)
	{
		ConnContext* context = otrl_context_find(FUserState,
												 AContactJid.full().toUtf8().constData(),
												 AStreamJid.full().toUtf8().constData(),
												 OTR_PROTOCOL_STRING,
												 OTRL_INSTAG_BEST, false,
												 nullptr, nullptr, nullptr);
		if (context && (context->msgstate == OTRL_MSGSTATE_ENCRYPTED))
		{
			otrl_context_force_finished(context);
			FOtr->stateChange(AStreamJid, AContactJid, IOtr::StateChangeGoneInsecure);
		}
	}

	//-----------------------------------------------------------------------------

	void startSMP(const Jid& AStreamJid, const Jid& AContactJid,
				  const QString& AQuestion, const QString& ASecret)
	{
		ConnContext* context = otrl_context_find(FUserState,
												 AContactJid.full().toUtf8().constData(),
												 AStreamJid.full().toUtf8().constData(),
												 OTR_PROTOCOL_STRING,
												 OTRL_INSTAG_BEST, false,
												 nullptr, nullptr, nullptr);
		if (context)
		{
			QByteArray  secretArray   = ASecret.toUtf8();
			const char* secretPointer = secretArray.constData();
			size_t      secretLength  = qstrlen(secretPointer);

			if (AQuestion.isEmpty())
				otrl_message_initiate_smp(FUserState, &FUiOps, this, context,
										  reinterpret_cast<const unsigned char*>
											(const_cast<char*>(secretPointer)),
										  secretLength);
			else
				otrl_message_initiate_smp_q(FUserState, &FUiOps, this, context,
											AQuestion.toUtf8().constData(),
											reinterpret_cast<const unsigned char*>
												(const_cast<char*>(secretPointer)),
											secretLength);
		}
	}

	void continueSMP(const Jid& AStreamJid, const Jid& AContactJid, const QString& ASecret)
	{
		ConnContext* context = otrl_context_find(FUserState,
												 AContactJid.full().toUtf8().constData(),
												 AStreamJid.full().toUtf8().constData(),
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

	void abortSMP(const Jid& AStreamJid, const Jid& AContactJid)
	{
		ConnContext* context = otrl_context_find(FUserState,
												 AContactJid.full().toUtf8().constData(),
												 AStreamJid.full().toUtf8().constData(),
												 OTR_PROTOCOL_STRING,
												 OTRL_INSTAG_BEST, false,
												 nullptr, nullptr, nullptr);
		if (context)
			abortSMP(context);
	}

	void abortSMP(ConnContext* AContext)
	{
		otrl_message_abort_smp(FUserState, &FUiOps, this, AContext);
	}

	//-----------------------------------------------------------------------------

	IOtr::MessageState getMessageState(const Jid& AStreamJid, const Jid& AContactJid)
	{
		ConnContext* context = otrl_context_find(FUserState,
												 AContactJid.full().toUtf8().constData(),
												 AStreamJid.full().toUtf8().constData(),
												 OTR_PROTOCOL_STRING,
												 OTRL_INSTAG_BEST, false,
												 nullptr, nullptr, nullptr);
		if (context)
		{
			if (context->msgstate == OTRL_MSGSTATE_PLAINTEXT)
				return IOtr::MsgStatePlaintext;
			else if (context->msgstate == OTRL_MSGSTATE_ENCRYPTED)
				return IOtr::MsgStateEncrypted;
			else if (context->msgstate == OTRL_MSGSTATE_FINISHED)
				return IOtr::MsgStateFinished;
		}

		return IOtr::MsgStateUnknown;
	}

	QString getMessageStateString(const Jid& AStreamJid, const Jid& AContactJid)
	{
		IOtr::MessageState state = getMessageState(AStreamJid, AContactJid);

		if (state == IOtr::MsgStatePlaintext)
			return QObject::tr("plaintext");
		else if (state == IOtr::MsgStateEncrypted)
			return QObject::tr("encrypted");
		else if (state == IOtr::MsgStateFinished)
			return QObject::tr("finished");

		return QObject::tr("unknown");
	}

	QString getSessionId(const Jid& AStreamJid, const Jid& AContactJid)
	{
		ConnContext* context;
		context = otrl_context_find(FUserState, AContactJid.full().toUtf8().constData(),
									AStreamJid.full().toUtf8().constData(), OTR_PROTOCOL_STRING,
									OTRL_INSTAG_BEST, false,
									nullptr, nullptr, nullptr);
		if (context && (context->sessionid_len > 0))
		{
			QString firstHalf;
			QString secondHalf;

			for (unsigned int i = 0; i < context->sessionid_len / 2; i++)
			{
				if (context->sessionid[i] <= 0xf)
					firstHalf.append("0");
				firstHalf.append(QString::number(context->sessionid[i], 16));
			}
			for (size_t i = context->sessionid_len / 2;
				 i < context->sessionid_len; i++)
			{
				if (context->sessionid[i] <= 0xf)
					secondHalf.append("0");
				secondHalf.append(QString::number(context->sessionid[i], 16));
			}

			if (context->sessionid_half == OTRL_SESSIONID_FIRST_HALF_BOLD)
				return QString("<b>" + firstHalf + "</b> " + secondHalf);
			else
				return QString(firstHalf + " <b>" + secondHalf + "</b>");
		}

		return QString();
	}

	OtrFingerprint getActiveFingerprint(const Jid& AStreamJid, const Jid& AContactJid)
	{
		ConnContext* context = otrl_context_find(
					FUserState, AContactJid.full().toUtf8().constData(),
					AStreamJid.full().toUtf8().constData(),
					OTR_PROTOCOL_STRING, OTRL_INSTAG_BEST,
					false, nullptr, nullptr, nullptr);

		if (context && context->active_fingerprint)
			return OtrFingerprint(context->active_fingerprint->fingerprint,
								  QString::fromUtf8(context->accountname),
								  QString::fromUtf8(context->username),
								  QString::fromUtf8(context->active_fingerprint->trust));

		return OtrFingerprint();
	}

	bool isVerified(const Jid& AStreamJid, const Jid& AContactJid)
	{
		return isVerified(otrl_context_find(FUserState,
											AContactJid.full().toUtf8().constData(),
											AStreamJid.full().toUtf8().constData(),
											OTR_PROTOCOL_STRING, OTRL_INSTAG_BEST,
											false, nullptr, nullptr, nullptr));
	}

	bool isVerified(ConnContext* AContext)
	{

		if (AContext && AContext->active_fingerprint)
			return (AContext->active_fingerprint->trust &&
					AContext->active_fingerprint->trust[0]);

		return false;
	}

	bool smpSucceeded(const Jid& AStreamJid, const Jid& AContactJid)
	{
		ConnContext *context = otrl_context_find(FUserState,
												 AContactJid.full().toUtf8().constData(),
												 AStreamJid.full().toUtf8().constData(),
												 OTR_PROTOCOL_STRING, OTRL_INSTAG_BEST,
												 false, nullptr, nullptr, nullptr);
		if (context)
			return context->smstate->sm_prog_state == OTRL_SMP_PROG_SUCCEEDED;

		return false;
	}

	void generateKey(const Jid& AStreamJid)
	{
		createPrivkey(AStreamJid.full().toUtf8().constData(), OTR_PROTOCOL_STRING);
	}

	static QString humanFingerprint(const unsigned char* AFingerprint)
	{
		char fpHash[OTRL_PRIVKEY_FPRINT_HUMAN_LEN];
		otrl_privkey_hash_to_human(fpHash, AFingerprint);
		return QString(fpHash);
	}

protected:
	//  implemented callback functions for libotr
	static OtrlPolicy policy(IOtr::Policy APolicy)
	{
		switch (APolicy)
		{
//			case IOtr::PolicyOff:
//				return OTRL_POLICY_NEVER; // otr disabled

			case IOtr::PolicyEnabled:
				return OTRL_POLICY_MANUAL;	// otr enabled, session started manual

			case IOtr::PolicyAuto:
				return OTRL_POLICY_OPPORTUNISTIC; // automatically initiate private messaging

			case IOtr::PolicyRequire:
				return OTRL_POLICY_ALWAYS;	// require private messaging

			default:
				return OTRL_POLICY_NEVER;	// otr disabled
		}
	}

	// ---------------------------------------------------------------------------

	void createPrivkey(const char* AAccountName, const char* AProtocol)
	{
		if (FIsGenerating)
			return;

		Jid streamJid(QString::fromUtf8(AAccountName));
		QString accountName(FOtr->humanAccount(streamJid));

		QMessageBox qMB(QMessageBox::Question, QObject::tr("Off-the-Record Messaging"),
						QObject::tr("Private keys for account \"%1\" need to be generated. "
									"This takes quite some time (from a few seconds to a "
									"couple of minutes), and while you can use %2 in the "
									"meantime, all the messages will be sent unencrypted "
									"until keys are generated. You will be notified when "
									"this process finishes.\n"
									"\n"
									"Do you want to generate keys now?")
									.arg(accountName)
									.arg(CLIENT_NAME),
						   QMessageBox::Yes | QMessageBox::No);

		if (qMB.exec() != QMessageBox::Yes)
			return;

		void *newkeyp;
		if (otrl_privkey_generate_start(FUserState, AAccountName, AProtocol, &newkeyp) == gcry_error(GPG_ERR_EEXIST)) {
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

		if (future.result() == gcry_error(GPG_ERR_NO_ERROR))
			otrl_privkey_generate_finish(FUserState, newkeyp, QFile::encodeName(FKeysFile));

		char fingerprint[OTRL_PRIVKEY_FPRINT_HUMAN_LEN];
		if (otrl_privkey_fingerprint(FUserState, fingerprint, AAccountName, AProtocol))
		{
			QString fp(fingerprint);
			QMessageBox infoMb(QMessageBox::Information, QObject::tr("Off-the-Record Messaging"),
							   QObject::tr("Keys have been generated. "
										   "Fingerprint for account \"%1\":\n%2\n\n"
										   "Thanks for your patience.")
									   .arg(accountName)
									   .arg(fp));
			infoMb.exec();
			emit FOtr->privKeyGenerated(streamJid, fp);
		}
		else
		{
			QMessageBox failMb(QMessageBox::Critical, QObject::tr("Off-the-Record Messaging"),
							   QObject::tr("Failed to generate keys for account \"%1\"."
										   "\nThe OTR Plugin will not work.")
										   .arg(accountName),
							   QMessageBox::Ok);
			failMb.exec();
			emit FOtr->privKeyGenerationFailed(streamJid);
		}
	}

	int isLoggedIn(const char* AAccountname, const char* AProtocol,
				   const char* ARecipient) const
	{
		Q_UNUSED(AProtocol);

		return FOtr->isLoggedIn(QString::fromUtf8(AAccountname),
								QString::fromUtf8(ARecipient));
	}

	void injectMessage(const char* AAccountName, const char* AProtocol,
					   const char* ARecipient, const char* AMessage)
	{
		Q_UNUSED(AProtocol);

		FOtr->sendMessage(QString::fromUtf8(AAccountName),
						  QString::fromUtf8(ARecipient),
						  QString::fromUtf8(AMessage));
	}

	void handleMsgEvent(OtrlMessageEvent AMsgEvent, ConnContext* AContext,
						const char* AMessage, gcry_error_t AError)
	{
		Q_UNUSED(AError);
		Q_UNUSED(AMessage);

		Jid streamJid(QString::fromUtf8(AContext->accountname));
		Jid contactJid(QString::fromUtf8(AContext->username));

		QString errorString;
		switch (AMsgEvent)
		{
			case OTRL_MSGEVENT_RCVDMSG_UNENCRYPTED:
				errorString = QObject::tr("<b>The following message received "
										"from %1 was <i>not</i> encrypted:</b>")
										.arg(FOtr->humanContact(streamJid, contactJid));
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

		if (!errorString.isEmpty())
			FOtr->displayOtrMessage(streamJid, contactJid, errorString);
	}

	void handleSmpEvent(OtrlSMPEvent ASmpEvent, ConnContext* AContext,
						unsigned short AProgressPercent, char* AQuestion)
	{
		Jid streamJid(QString::fromUtf8(AContext->accountname));
		Jid contactJid(QString::fromUtf8(AContext->username));

		if (ASmpEvent == OTRL_SMPEVENT_CHEATED || ASmpEvent == OTRL_SMPEVENT_ERROR) {
			abortSMP(AContext);
			FOtr->updateSMP(streamJid, contactJid, -2);
		}
		else if (ASmpEvent == OTRL_SMPEVENT_ASK_FOR_SECRET ||
				 ASmpEvent == OTRL_SMPEVENT_ASK_FOR_ANSWER)
			FOtr->receivedSMP(streamJid, contactJid, QString::fromUtf8(AQuestion));
		else
			FOtr->updateSMP(streamJid, contactJid, AProgressPercent);
	}

	void createInstag(const char* AAccountName, const char* AProtocol)
	{
		otrl_instag_generate(FUserState, QFile::encodeName(FInstagsFile).constData(),
							 AAccountName, AProtocol);
	}

	void updateContextList()
	{
	}

	void newFingerprint(OtrlUserState AUserState, const char* AAccountName,
									  const char* AProtocol, const char* AUsername,
									  unsigned char AFingerprint[20])
	{
		Q_UNUSED(AUserState);
		Q_UNUSED(AProtocol);

		Jid streamJid(QString::fromUtf8(AAccountName));
		Jid contactJid(QString::fromUtf8(AUsername));
		QString message = QObject::tr("You have received a new "
									  "fingerprint from %1:\n%2")
									.arg(FOtr->humanContact(streamJid, contactJid))
									.arg(humanFingerprint(AFingerprint));

		FOtr->displayOtrMessage(streamJid, contactJid, message);
	}

	void writeFingerprints()
	{
		otrl_privkey_write_fingerprints(FUserState,
										QFile::encodeName(FFingerprintFile).constData());
		emit FOtr->fingerprintsUpdated();
	}

	void goneSecure(ConnContext* AContext)
	{
		FOtr->stateChange(QString::fromUtf8(AContext->accountname),
						  QString::fromUtf8(AContext->username),
						  IOtr::StateChangeGoneSecure);
	}

	void goneInsecure(ConnContext* AContext)
	{
		FOtr->stateChange(QString::fromUtf8(AContext->accountname),
						  QString::fromUtf8(AContext->username),
						  IOtr::StateChangeGoneInsecure);
	}

	void stillSecure(ConnContext* AContext, int AIsReply)
	{
		Q_UNUSED(AIsReply);
		FOtr->stateChange(QString::fromUtf8(AContext->accountname),
						  QString::fromUtf8(AContext->username),
						  IOtr::StateChangeStillSecure);
	}

	const char* accountName(const char* AAccount, const char* AProtocol)
	{
		Q_UNUSED(AProtocol);
		return qstrdup(FOtr->humanAccountPublic(QString::fromUtf8(AAccount))
													 .toUtf8().constData());
	}

	void accountNameFree(const char* AAccountName)
	{
		delete [] AAccountName;
	}

	// ---------------------------------------------------------------------------
	/*** static wrapper functions ***/
	static OtrlPolicy cbPolicy(void* AOpdata, ConnContext* AContext) {
		Q_UNUSED(AOpdata);
		Q_UNUSED(AContext);
		return policy(IOtr::Policy(Options::node(OPV_OTR_POLICY).value().toInt()));
	}

	static void cbCreatePrivkey(void* APpdata, const char* AAccountName, const char* AProtocol) {
		static_cast<OtrPrivate*>(APpdata)->createPrivkey(AAccountName, AProtocol);
	}

	static int cbIsLoggedIn(void* AOpdata, const char* AAccountName, const char* AProtocol, const char* ARecipient) {
		return static_cast<OtrPrivate*>(AOpdata)->isLoggedIn(AAccountName, AProtocol, ARecipient);
	}

	static void cbInjectMessage(void* AOpdata, const char* AAccountname, const char* AProtocol, const char* ARecipient, const char* AMessage) {
		static_cast<OtrPrivate*>(AOpdata)->injectMessage(AAccountname, AProtocol, ARecipient, AMessage);
	}

	static void cbHandleMsgEvent(void* AOpdata, OtrlMessageEvent AMsgEvent, ConnContext* AContext, const char* AMessage, gcry_error_t AError) {
		static_cast<OtrPrivate*>(AOpdata)->handleMsgEvent(AMsgEvent, AContext, AMessage, AError);
	}

	static void cbHandleSmpEvent(void* AOpdata, OtrlSMPEvent ASmpEvent, ConnContext* AContext, unsigned short AProgressPercent, char* AQuestion) {
		static_cast<OtrPrivate*>(AOpdata)->handleSmpEvent(ASmpEvent, AContext, AProgressPercent, AQuestion);
	}

	static void cbCreateInstag(void* AOpdata, const char* AAccountName, const char* AProtocol) {
		static_cast<OtrPrivate*>(AOpdata)->createInstag(AAccountName, AProtocol);
	}

	static void cbUpdateContextList(void* AOpdata) {
		static_cast<OtrPrivate*>(AOpdata)->updateContextList();
	}

	static void cbNewFingerprint(void* AOpdata, OtrlUserState AUserState,
								 const char* AAccountName, const char* AProtocol,
								 const char* AUserName, unsigned char AFingerprint[20]) {
		static_cast<OtrPrivate*>(AOpdata)->newFingerprint(AUserState, AAccountName, AProtocol, AUserName, AFingerprint);
	}

	static void cbWriteFingerprints(void* AOpdata) {
		static_cast<OtrPrivate*>(AOpdata)->writeFingerprints();
	}

	static void cbGoneSecure(void* AOpdata, ConnContext* AContext) {
		static_cast<OtrPrivate*>(AOpdata)->goneSecure(AContext);
	}

	static void cbGoneInsecure(void* AOpdata, ConnContext* AContext) {
		static_cast<OtrPrivate*>(AOpdata)->goneInsecure(AContext);
	}

	static void cbStillSecure(void* AOpdata, ConnContext* AContext, int AIsReply) {
		static_cast<OtrPrivate*>(AOpdata)->stillSecure(AContext, AIsReply);
	}

	static const char* cbAccountName(void* AOpdata, const char* AAccount,
											 const char* AProtocol) {
		return static_cast<OtrPrivate*>(AOpdata)->accountName(AAccount, AProtocol);
	}

	static void cbAccountNameFree(void* AOpdata, const char* AAccountName) {
		static_cast<OtrPrivate*>(AOpdata)->accountNameFree(AAccountName);
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
	Otr* FOtr;

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

#define ADR_CONTACT_JID Action::DR_Parametr2
#define ADR_STREAM_JID Action::DR_StreamJid

OtrFingerprint::OtrFingerprint():
	FFingerprint(nullptr)
{}

OtrFingerprint::OtrFingerprint(const OtrFingerprint &AOther):
	FFingerprint(AOther.FFingerprint),
	FStreamJid(AOther.FStreamJid),
	FContactJid(AOther.FContactJid),
	FFingerprintHuman(AOther.FFingerprintHuman),
	FTrust(AOther.FTrust)
{}

OtrFingerprint::OtrFingerprint(unsigned char* AFingerprint,
						 QString AStreamJid, QString AContactJid,
						 QString ATrust):
	FFingerprint(AFingerprint),
	FStreamJid(AStreamJid),
	FContactJid(AContactJid),
	FTrust(ATrust)
{
	FFingerprintHuman = OtrPrivate::humanFingerprint(AFingerprint);
}

Otr::Otr() :
	FOtrPrivate(new OtrPrivate(this)),
	FOptionsManager(nullptr),
	FAccountManager(nullptr),
//	FPresenceManager(nullptr),
	FMessageProcessor(nullptr),
	FNotifications(nullptr)
{
}

Otr::~Otr()
{
	delete FOtrPrivate;
}

void Otr::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Off-the-Record Messaging");
	APluginInfo->description = tr("Allows you to have private conversations over instant messaging");
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

//	plugin = APluginManager->pluginInterface("IPresenceManager").value(0);
//	if (plugin)
//		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());

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
	if (plugin)
		FAccountManager = qobject_cast<IAccountManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IMessageProcessor").value(0,nullptr);
	if (plugin)
		FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());

	plugin = APluginManager->pluginInterface("INotifications").value(0,nullptr);
	if (plugin)
	{
		FNotifications = qobject_cast<INotifications *>(plugin->instance());
		if (FNotifications)
			connect(FNotifications->instance(), SIGNAL(notificationActivated(int)),
												SLOT(onNotificationActivated(int)));
	}

	plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,nullptr);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
		if (FMessageWidgets)
		{
			connect(FMessageWidgets->instance(), SIGNAL(chatWindowCreated(IMessageChatWindow *)), SLOT(onChatWindowCreated(IMessageChatWindow *)));
			connect(FMessageWidgets->instance(), SIGNAL(chatWindowDestroyed(IMessageChatWindow *)), SLOT(onChatWindowDestroyed(IMessageChatWindow *)));
		}
	}

	return (FStanzaProcessor != nullptr);
}

bool Otr::initObjects()
{
	FMenuIcons = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);

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

	if (FMessageArchiver)
		FMessageArchiver->insertArchiveHandler(AHO_DEFAULT, this);

	if (FNotifications)
	{
		INotificationType notifyType;
		notifyType.order = NTO_OTR_ESTABLISHED;
		notifyType.icon = FMenuIcons->getIcon(MNI_OTR_ENCRYPTED);
		notifyType.title = tr("When OTR private conversation established");
		notifyType.kindMask = INotification::PopupWindow|INotification::SoundPlay|
							  INotification::TrayNotify|INotification::TrayAction|
							  INotification::ShowMinimized;
		notifyType.kindDefs = notifyType.kindMask;
		FNotifications->registerNotificationType(NNT_OTR_ESTABLISHED, notifyType);

		notifyType.order = NTO_OTR_TERMINATED;
		notifyType.icon = FMenuIcons->getIcon(MNI_OTR_NO);
		notifyType.title = tr("When OTR private conversation terminated");
		notifyType.kindDefs = notifyType.kindMask;
		FNotifications->registerNotificationType(NNT_OTR_TERMINATED, notifyType);

		notifyType.order = NTO_OTR_VERIFY;
		notifyType.icon = FMenuIcons->getIcon(MNI_OTR_UNVERFIFIED);
		notifyType.title = tr("When OTR fingerprint verification initiated");
		notifyType.kindDefs = notifyType.kindMask;
		FNotifications->registerNotificationType(NNT_OTR_VERIFY, notifyType);
	}

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

	return AMessage.body().startsWith("?OTR");
}

void Otr::onStreamOpened( IXmppStream *AXmppStream )
{
	Q_UNUSED(AXmppStream);
}

void Otr::onStreamClosed( IXmppStream *AXmppStream )
{
	Jid streamJid = AXmppStream->streamJid();
	if (FOnlineUsers.contains(streamJid))
	{
		QHash<Jid, OtrClosure*> &contacts = FOnlineUsers[streamJid];
		for (QHash<Jid, OtrClosure*>::ConstIterator it = contacts.constBegin();
			 it != contacts.constEnd(); ++it)
		{
			FOtrPrivate->endSession(streamJid.full(), it.key().full());
			(*it)->setIsLoggedIn(false);
		}
	}
}

void Otr::onChatWindowCreated(IMessageChatWindow *AWindow)
{
	QString contact = AWindow->contactJid().uFull();
	QString stream = AWindow->streamJid().uFull();
	Action *otrAction = new Action(AWindow->toolBarWidget()->instance());
	otrAction->setData(ADR_STREAM_JID, stream);
	otrAction->setData(ADR_CONTACT_JID, contact);

	Menu *menu = new Menu();
	QActionGroup *actionGroup = new QActionGroup(menu);
	otrAction->setMenu(menu);

	// 0: Session initiate
	Action *action = new Action(menu);
	action->setData(ADR_STREAM_JID, stream);
	action->setData(ADR_CONTACT_JID, contact);
	connect(action, SIGNAL(triggered()), SLOT(onSessionInitiate()));
	action->setActionGroup(actionGroup);
	menu->addAction(action);

	// 1: End private conversation
	action = new Action(menu);
	action->setData(ADR_CONTACT_JID, contact);
	action->setData(ADR_STREAM_JID, stream);
	action->setText(tr("&End private conversation"));
	connect(action, SIGNAL(triggered()), SLOT(onSessionEnd()));
	action->setActionGroup(actionGroup);
	menu->addAction(action);

	// 2: Separator
	menu->insertSeparator(nullptr);

	// 3: Authenticate contact
	action = new Action(menu);
	action->setText(tr("&Authenticate contact"));
	action->setData(ADR_STREAM_JID, stream);
	action->setData(ADR_CONTACT_JID, contact);
	connect(action, SIGNAL(triggered()), SLOT(onContactAuthenticate()));
	action->setActionGroup(actionGroup);
	menu->addAction(action);

	// 4: Show secure session ID
	action = new Action(menu);
	action->setText(tr("Show secure session &ID"));
	action->setData(ADR_STREAM_JID, stream);
	action->setData(ADR_CONTACT_JID, contact);
	connect(action, SIGNAL(triggered()), SLOT(onSessionID()));
	action->setActionGroup(actionGroup);
	menu->addAction(action);

	// 5: Show own fingerprint
	action = new Action(menu);
	action->setText(tr("Show own &fingerprint"));
	action->setData(ADR_STREAM_JID, stream);
	action->setData(ADR_CONTACT_JID, contact);
	connect(action, SIGNAL(triggered()), SLOT(onFingerprint()));
	action->setActionGroup(actionGroup);
	menu->addAction(action);

	menu->setToolTip(tr("OTR Messaging"));

	QToolButton *otrButton = AWindow->toolBarWidget()->toolBarChanger()->insertAction(otrAction, TBG_MWTBW_OTR);
	otrButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	otrButton->setPopupMode(QToolButton::InstantPopup);

	connect(AWindow->instance(), SIGNAL(tabPageActivated()), SLOT(onChatWindowActivated()));
	connect(AWindow->address()->instance(), SIGNAL(addressChanged(const Jid &, const Jid &)),
											SLOT(onWindowAddressChanged(const Jid &, const Jid &)));
	connect(this, SIGNAL(otrStateChanged(const Jid &, const Jid &)),
				  SLOT(onUpdateMessageState(const Jid &, const Jid &)));

	onUpdateMessageState(AWindow->streamJid(), AWindow->contactJid());
}

void Otr::onChatWindowDestroyed(IMessageChatWindow *AWindow)
{
	Q_UNUSED(AWindow)
}

void Otr::onChatWindowActivated()
{
	IMessageChatWindow *window = qobject_cast<IMessageChatWindow *>(sender());
	if (window)
	{
		removeNotifications(window);
//TODO: Check all window addresses
		if (FOnlineUsers.contains(window->streamJid()) &&
			FOnlineUsers[window->streamJid()].contains(window->contactJid()) &&
			FOnlineUsers[window->streamJid()][window->contactJid()]->isRunning())
			FOnlineUsers[window->streamJid()][window->contactJid()]->showSmpDialog();
	}
}

void Otr::onProfileOpened(const QString &AProfile)
{
	FHomePath = FOptionsManager->profilePath(AProfile);
	FOtrPrivate->init();
}

void Otr::onNotificationActivated(int ANotifyId)
{
	for (QHash<Jid, QHash<Jid, int> >::const_iterator its=FNotifies.constBegin(); its!=FNotifies.constEnd(); its++)
		for (QHash<Jid, int>::ConstIterator itc=(*its).constBegin(); itc!=(*its).constEnd(); itc++)
			if (itc.value()==ANotifyId) // Notification found! Activate window!
			{				
				Jid streamJid=its.key();
				Jid contactJid=itc.key();

				IMessageChatWindow *window=FMessageWidgets->findChatWindow(streamJid, contactJid);
				if (!window)
				{
					FMessageProcessor->getMessageWindow(streamJid, contactJid, Message::Chat, IMessageProcessor::ActionAssign);
					window = FMessageWidgets->findChatWindow(streamJid, contactJid);
				}

				if (window)
				{
					window->showTabPage();
					return;
				}
			}
}

// OTR tool button slots
void Otr::onSessionInitiate()
{
	Action *action = qobject_cast<Action *>(sender());
	FOtrPrivate->startSession(action->data(ADR_STREAM_JID).toString(),
							  action->data(ADR_CONTACT_JID).toString());
}

void Otr::onSessionEnd()
{
	Action *action = qobject_cast<Action *>(sender());
	Jid contactJid(action->data(ADR_CONTACT_JID).toString());
	Jid streamJid(action->data(ADR_STREAM_JID).toString());
	FOtrPrivate->endSession(streamJid, contactJid);
	onUpdateMessageState(streamJid, contactJid);
}

void Otr::onContactAuthenticate()
{
	Action *action = qobject_cast<Action *>(sender());
	Jid streamJid(action->data(ADR_STREAM_JID).toString());
	Jid contactJid(action->data(ADR_CONTACT_JID).toString());
	authenticateContact(streamJid, contactJid);
}

void Otr::onSessionID()
{
	Action *action = qobject_cast<Action *>(sender());
	Jid streamJid(action->data(ADR_STREAM_JID).toString());
	Jid contactJid(action->data(ADR_CONTACT_JID).toString());
	QString sId = FOtrPrivate->getSessionId(streamJid, contactJid);

	QString msg = sId.isEmpty() ? tr("No active encrypted session")
								: tr("Session ID between account \"%1\" and %2: %3")
									.arg(humanAccount(streamJid))
									.arg(contactJid.full())
									.arg(sId);

	displayOtrMessage(streamJid, contactJid, msg);
}

void Otr::onFingerprint()
{
	Action *action = qobject_cast<Action *>(sender());
	Jid streamJid(action->data(ADR_STREAM_JID).toString());
	Jid contactJid(action->data(ADR_CONTACT_JID).toString());
	QString fingerprint = getPrivateKeys()
							.value(streamJid, tr("No private key for account \"%1\"")
								.arg(humanAccount(streamJid)));

	QString msg(tr("Fingerprint for account \"%1\": %2")
				   .arg(humanAccount(streamJid))
				   .arg(fingerprint));

	displayOtrMessage(streamJid, contactJid, msg);
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
				Jid contactJid(otrAction->data(ADR_CONTACT_JID).toString());
				Jid streamJid(otrAction->data(ADR_STREAM_JID).toString());

				QString iconKey;
				IOtr::MessageState state = getMessageState(streamJid, contactJid);
				QString stateString(getMessageStateString(streamJid, contactJid));

				if (state == IOtr::MsgStateEncrypted)
				{
					if (isVerified(streamJid, contactJid))
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

//-----------------------------------------------------------------------------

void Otr::authenticateContact(const Jid &AStreamJid, const Jid &AContactJid)
{
	if (!FOnlineUsers.value(AStreamJid).contains(AContactJid))
		FOnlineUsers[AStreamJid][AContactJid] = new OtrClosure(AStreamJid, AContactJid, this);

	FOnlineUsers[AStreamJid][AContactJid]->authenticateContact();
}

//-----------------------------------------------------------------------------

QString Otr::dataDir()
{
	return FHomePath;
}

//-----------------------------------------------------------------------------

void Otr::sendMessage(const Jid &AStreamJid, const Jid &AContactJid,
					  const QString& AMessage, const QString &AHtml)
{
	if (!AMessage.isEmpty())
	{
		Message message;
		message.setType(Message::Chat).setBody(AMessage).setTo(AContactJid.full());
		message.stanza().setAttribute(SKIP_OTR_FLAG, "true");

		if (!AHtml.isEmpty())
		{
			QDomDocument doc;
			doc.setContent(QString("<html xmlns=\'" NS_XHTML_IM "\'>"
								   "<body xmlns=\'" NS_XHTML "\'>"
								   "%1</body></html>").arg(AHtml));
			message.stanza().element().appendChild(message.stanza().document()
												   .importNode(doc.documentElement(),
															   true));
		}
		FStanzaProcessor->sendStanzaOut(AStreamJid, message.stanza());
	}
}

//-----------------------------------------------------------------------------

bool Otr::isLoggedIn(const Jid &AStreamJid, const Jid &AContactJid) const
{
	if (FOnlineUsers.contains(AStreamJid) &&
		FOnlineUsers.value(AStreamJid).contains(AContactJid))
		return FOnlineUsers.value(AStreamJid).value(AContactJid)->isLoggedIn();
	return false;
}

//-----------------------------------------------------------------------------

bool Otr::displayOtrMessage(const Jid &AStreamJid, const Jid &AContactJid,
							const QString& AMessage)
{
	notifyInChatWindow(AStreamJid, AContactJid, AMessage);
	LOG_STRM_INFO(AStreamJid, QString("OTR displayOtrMessage, contact=%1")
								.arg(AContactJid.full()));
	return true;
}

//-----------------------------------------------------------------------------

void Otr::stateChange(const Jid &AStreamJid, const Jid &AContactJid, StateChange AChange)
{
	LOG_STRM_INFO(AStreamJid, QString("OTR stateChange, contact=%1").arg(AContactJid.full()));

	if (!FOnlineUsers.value(AStreamJid).contains(AContactJid))
		FOnlineUsers[AStreamJid][AContactJid] = new OtrClosure(AStreamJid, AContactJid, this);

	bool verified  = isVerified(AStreamJid, AContactJid);
	bool encrypted = FOnlineUsers[AStreamJid][AContactJid]->encrypted();
	QString message, tooltip, icon;

	switch (AChange)
	{
		case StateChangeGoingSecure:
			message = encrypted?
					  tr("Attempting to refresh the private conversation")
					: tr("Attempting to start a private conversation");
			break;

		case StateChangeGoneSecure:
			message  = verified ? tr("Private conversation started")
								: tr("Unverified conversation started");

			tooltip  = verified ? tr("Private conversation with %1 started")
								: tr("Unverified conversation with %1 started");
			icon = verified ? MNI_OTR_ENCRYPTED : MNI_OTR_UNVERFIFIED;
			eventNotify(NNT_OTR_ESTABLISHED, message, tooltip, icon, AStreamJid, AContactJid);
			break;

		case StateChangeGoneInsecure:
			message  = tr("Private conversation lost");
			tooltip  = tr("Private conversation with %1 lost");
			eventNotify(NNT_OTR_TERMINATED, message, tooltip, MNI_OTR_NO, AStreamJid, AContactJid);
			break;

		case StateChangeClose:
			message  = tr("Private conversation closed");
			tooltip  = tr("Private conversation with %1 closed");
			eventNotify(NNT_OTR_TERMINATED, message, tooltip, MNI_OTR_NO, AStreamJid, AContactJid);
			break;

		case StateChangeRemoteClose:
			message  = tr("Has ended the private conversation with you");
			tooltip  = tr("%1 has ended the private conversation with you");
			eventNotify(NNT_OTR_TERMINATED, message, tooltip, MNI_OTR_NO, AStreamJid, AContactJid);
			break;

		case StateChangeStillSecure:
			message  = verified ? tr("Private conversation refreshed")
								: tr("Unverified conversation refreshed");
			tooltip  = verified ? tr("Private conversation with %1 refreshed")
								: tr("Unverified conversation with %1 refreshed");
			icon = verified ? MNI_OTR_ENCRYPTED : MNI_OTR_UNVERFIFIED;
			eventNotify(NNT_OTR_ESTABLISHED, message, tooltip, icon, AStreamJid, AContactJid);
			break;

		case StateChangeTrust:
			message  = verified ? tr("Contact authenticated")
								: tr("Contact not authenticated");
			tooltip  = verified ? tr("%1 authenticated")
								: tr("%1 not authenticated");
			icon = verified ? MNI_OTR_ENCRYPTED : MNI_OTR_UNVERFIFIED;
			eventNotify(NNT_OTR_VERIFY, message, tooltip, icon, AStreamJid, AContactJid);
			break;
	}

	notifyInChatWindow(AStreamJid, AContactJid, message);
	emit otrStateChanged(AStreamJid, AContactJid);
}

void Otr::receivedSMP(const Jid &AStreamJid, const Jid &AContactJid,
					  const QString& AQuestion)
{
	LOG_STRM_INFO(AStreamJid, QString("OTR receivedSMP, contact=%1")
				  .arg(AContactJid.full()));

	if (FOnlineUsers.contains(AStreamJid) &&
		FOnlineUsers.value(AStreamJid).contains(AContactJid))
	{
		FOnlineUsers[AStreamJid][AContactJid]->receivedSmp(AQuestion);

		IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
		if (window && window->isActiveTabPage())
//TODO: Check currently selected address
			FOnlineUsers[AStreamJid][AContactJid]->showSmpDialog();
		else
			eventNotify(NNT_OTR_VERIFY,
						tr("Received fingerprint verification"),
						tr("Fingerprint verification from %1"),
						MNI_OTR_UNVERFIFIED, AStreamJid, AContactJid);
	}
}

void Otr::updateSMP(const Jid &AStreamJid, const Jid &AContactJid, int AProgress)
{
	LOG_STRM_INFO(AStreamJid, QString("OTR updateSMP, contact=%1")
								.arg(AContactJid.full()));

	if (FOnlineUsers.contains(AStreamJid) &&
		FOnlineUsers.value(AStreamJid).contains(AContactJid))
		FOnlineUsers[AStreamJid][AContactJid]->updateSmpDialog(AProgress);
}

QString Otr::humanAccount(const Jid &AStreamJid)
{
	IAccount *account = FAccountManager->findAccountByStream(AStreamJid);
	return account ? account->name() : AStreamJid.uFull();
}

QString Otr::humanAccountPublic(const Jid &AStreamJid)
{
	return AStreamJid.bare();
}

QString Otr::humanContact(const Jid &AStreamJid, const Jid &AContactJid)
{
	Q_UNUSED(AStreamJid)
//FIXME: Return correct contact name
	return AContactJid.uFull();
}

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

INotification Otr::eventNotify(const QString &ATypeId, const QString &AMessagePopup,
							   const QString &AMessageTooltip, const QString &AIcon,
							   const Jid &AStreamJid, const Jid &AContactJid)
{
	INotification notify;

	notify.kinds = FNotifications->enabledTypeNotificationKinds(ATypeId);

	IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
	if (window && window->isActiveTabPage())	// The window is existing and is an active tab page!
		notify.kinds = 0;                       // So, do not notify!

	if (notify.kinds)
	{
		notify.typeId = ATypeId;
		notify.data.insert(NDR_STREAM_JID, AStreamJid.full());
		notify.data.insert(NDR_CONTACT_JID, AContactJid.full());
		notify.data.insert(NDR_ICON, FMenuIcons->getIcon(AIcon));
		notify.data.insert(NDR_POPUP_HTML, AMessagePopup);
		notify.data.insert(NDR_POPUP_TITLE, FNotifications->contactName(AStreamJid, AContactJid));
		notify.data.insert(NDR_POPUP_IMAGE, FNotifications->contactAvatar(AContactJid));
		notify.data.insert(NDR_POPUP_CAPTION, tr("Off-the-Record messaging"));

		notify.data.insert(NDR_TOOLTIP, AMessageTooltip.arg(FNotifications->contactName(AStreamJid, AContactJid)));
		notify.data.insert(NDR_ROSTER_ORDER, RNO_OTR);
		notify.data.insert(NDR_ROSTER_FLAGS,IRostersNotify::Blink|IRostersNotify::AllwaysVisible|IRostersNotify::HookClicks);
		notify.data.insert(NDR_ROSTER_CREATE_INDEX, true);
		notify.data.insert(NDR_SOUND_FILE, SDF_OTR_EVENT);

		if (FNotifies.contains(AStreamJid) && FNotifies[AStreamJid].contains(AContactJid))
			FNotifications->removeNotification(FNotifies[AStreamJid].value(AContactJid));
		FNotifies[AStreamJid].insert(AContactJid, FNotifications->appendNotification(notify));
	}

	return notify;
}

void Otr::removeNotifications(IMessageChatWindow *AWindow)
{
	if (FNotifies.contains(AWindow->streamJid()) &&
		FNotifies[AWindow->streamJid()].contains(AWindow->contactJid()))
	{
		FNotifications->removeNotification(FNotifies[AWindow->streamJid()][AWindow->contactJid()]);
		FNotifies[AWindow->streamJid()].remove(AWindow->contactJid());
		if (FNotifies[AWindow->streamJid()].isEmpty())
			FNotifies.remove(AWindow->streamJid());
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
			Jid contactJid(AStanza.from());

			if (AStanza.type() == PRESENCE_TYPE_AVAILABLE)
			{
				if (!FOnlineUsers.value(AStreamJid).contains(contactJid))
					FOnlineUsers[AStreamJid][contactJid] = new OtrClosure(AStreamJid, contactJid, this);
				FOnlineUsers[AStreamJid][contactJid]->setIsLoggedIn(true);
			}
			else if (AStanza.type() == PRESENCE_TYPE_UNAVAILABLE)
			{
				if (FOnlineUsers.contains(AStreamJid) &&
					FOnlineUsers.value(AStreamJid).contains(contactJid))
				{
					if (Options::node(OPV_OTR_ENDWHENOFFLINE).value().toBool())
						FOtrPrivate->expireSession(AStreamJid, contactJid);
					FOnlineUsers[AStreamJid][contactJid]->setIsLoggedIn(false);
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
				Jid contactJid(message.to());
				QString encrypted = FOtrPrivate->encryptMessage(AStreamJid, contactJid,
																message.body());
				//if there has been an error, drop the message
				if (encrypted.isEmpty())
				{
					LOG_WARNING("Failed to encrypt");
					return true;
				}

				message.setBody(encrypted);
				AStanza = message.stanza();

				if (getMessageState(AStreamJid, contactJid) == IOtr::MsgStateEncrypted)
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

				QString decrypted;
				IOtr::MessageType messageType = FOtrPrivate->decryptMessage(AStreamJid, message.from(),
																			message.body(), decrypted);
				switch (messageType)
				{
					case IOtr::MsgTypeNone:
						break;
					case IOtr::MsgTypeIgnore:
						ignore = true;
						break;
					case IOtr::MsgTypeOtr:
						message.setBody(decrypted);
						AStanza = message.stanza();
						break;
				}
				return ignore;
			}
		}
		else
			message.stanza().element().removeAttribute("skip_otr_processing");
	}

	return false;
}

QList<OtrFingerprint> Otr::getFingerprints()
{
	return FOtrPrivate->getFingerprints();
}

void Otr::deleteFingerprint(const OtrFingerprint &AFingerprint)
{
	return FOtrPrivate->deleteFingerprint(AFingerprint);
}

void Otr::verifyFingerprint(const OtrFingerprint& AFingerprint, bool AVerified)
{
	FOtrPrivate->verifyFingerprint(AFingerprint, AVerified);
}

QHash<Jid, QString> Otr::getPrivateKeys()
{
	return FOtrPrivate->getPrivateKeys();
}

void Otr::deleteKey(const Jid &AStreamJid)
{
	FOtrPrivate->deleteKey(AStreamJid);
}

void Otr::startSMP(const Jid &AStreamJid, const Jid &AContactJid,
				   const QString& AQuestion, const QString& ASecret)
{
	FOtrPrivate->startSMP(AStreamJid, AContactJid, AQuestion, ASecret);
}

void Otr::continueSMP(const Jid &AStreamJid, const Jid &AContactJid,
					  const QString& ASecret)
{
	FOtrPrivate->continueSMP(AStreamJid, AContactJid, ASecret);
}

void Otr::abortSMP(const Jid& AStreamJid, const Jid& AContactJid)
{
	FOtrPrivate->abortSMP(AStreamJid, AContactJid);
}

IOtr::MessageState Otr::getMessageState(const Jid& AStreamJid, const Jid& AContactJid)
{
	return FOtrPrivate->getMessageState(AStreamJid, AContactJid);
}

QString Otr::getMessageStateString(const Jid &AStreamJid, const Jid &AContactJid)
{
	return FOtrPrivate->getMessageStateString(AStreamJid, AContactJid);
}

OtrFingerprint Otr::getActiveFingerprint(const Jid &AStreamJid, const Jid &AContactJid)
{
	return FOtrPrivate->getActiveFingerprint(AStreamJid, AContactJid);
}

bool Otr::isVerified(const Jid &AStreamJid, const Jid &AContactJid)
{
	return FOtrPrivate->isVerified(AStreamJid, AContactJid);
}

bool Otr::smpSucceeded(const Jid &AStreamJid, const Jid &AContactJid)
{
	return FOtrPrivate->smpSucceeded(AStreamJid, AContactJid);
}

void Otr::generateKey(const Jid& AStreamJid)
{
	FOtrPrivate->generateKey(AStreamJid);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_otr, Otr)
#endif
