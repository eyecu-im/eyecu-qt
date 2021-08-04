#include "signalprotocol.h"

#include <utils/logger.h>
#include <QMutex>
#include <QDateTime>
#include <QDebug>

extern "C" {
#include <gcrypt.h>
}
#include <key_helper.h>
#include <session_builder.h>
#include <session_cipher.h>
#include <session_state.h>
#include <protocol.h>

using namespace OmemoStore;

void SignalProtocol::init()
{
	gcry_check_version(nullptr);
	gcry_control(GCRYCTL_SUSPEND_SECMEM_WARN);
	gcry_control(GCRYCTL_INIT_SECMEM, 16384, 0);
	gcry_control(GCRYCTL_RESUME_SECMEM_WARN);
	gcry_control(GCRYCTL_USE_SECURE_RNDPOOL);
	gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
}

QString SignalProtocol::calcFingerprint(const QByteArray &APublicKey)
{
	QString result;
	if (APublicKey.size() == 33 && APublicKey[0] == 5)
	{
		for (int i=0; i<8; ++i)
		{
			if (i)
				result.append(' ');
			result.append(QString::fromLatin1(APublicKey.mid(1+i*4, 4).toHex()));
		}
	}
	else
		qCritical("Invalid public key data");

	return result;
}

QString SignalProtocol::connectionName() const
{
	return FConnectionName;
}

bool SignalProtocol::onNewKeyReceived(const QString &AName, const QByteArray &AKeyData)
{
	return FIdentityKeyListener?FIdentityKeyListener->onNewKeyReceived(AName, AKeyData)
							   :true;
}

signal_context *SignalProtocol::globalContext() const
{
	return FGlobalContext;
}

signal_protocol_store_context *SignalProtocol::storeContext() const
{
	return FStoreContext;
}

int SignalProtocol::error() const
{
	return FError;
}

int SignalProtocol::install(quint32 ASignedPreKeyId, uint APreKeyStartId, uint APreKeyAmount)
{
	char * errMsg = nullptr;
	bool dbNeedsInit = false;
	bool dbNeedsReset = false;

	ratchet_identity_key_pair * identityKeyPair = nullptr;
	signal_protocol_key_helper_pre_key_list_node * preKeysHead = nullptr;
	session_signed_pre_key * signedPreKey = nullptr;
	signal_buffer * signedPreKeyData = nullptr;
	uint32_t registrationId;
	int initStatus = DbNotInitialized;

	qDebug("%s: calling install-time functions", __func__);

	int rc = create(this);
	if (rc){
		errMsg = "failed to create db";
		goto cleanup;
	}

	qDebug("%s: created db if it did not exist already", __func__);

	rc = initStatusGet(initStatus, this);
	switch (rc) {
		case -1:
		default:
			errMsg = "failed to read init status";
			goto cleanup;
		case 0:
			// there is a value
			qDebug() << "Init status=" << initStatus;
			switch (initStatus) {
				case DbNotInitialized:
					// init needed
					dbNeedsInit = true;
					break;
				case DbNeedsRollback:
					// reset and init needed
					dbNeedsReset = true;
					dbNeedsInit = true;
					break;
				case DbInitialized:
				default:
					// the db is already initialised
					break;
			}
			break;
		case 1:
			// no value = not initialised -> init needed
			dbNeedsInit = true;
			break;
	}

	if (dbNeedsReset) {
		qDebug("%s: db needs reset", __func__ );
		rc = destroy(this);
		if (rc) {
			errMsg = "failed to reset db";
			goto cleanup;
		}

		rc = create(this);
		if (rc) {
			errMsg = "failed to create db after reset";
			goto cleanup;
		}
	} else {
		qDebug("%s: db does not need reset", __func__ );
	}

	if (dbNeedsInit) {
		qDebug("%s: db needs init", __func__ );
		rc = initStatusSet(DbNeedsRollback, this);
		if (rc) {
			errMsg = "failed to set init status to DbNeedsRollback";
			goto cleanup;
		}

		rc = signal_protocol_key_helper_generate_identity_key_pair(&identityKeyPair, FGlobalContext);
		if (rc) {
			errMsg = "failed to generate the identity key pair";
			goto cleanup;
		}

		rc = signal_protocol_key_helper_generate_registration_id(&registrationId, 1, FGlobalContext);
		if (rc) {
			errMsg = "failed to generate registration id";
			goto cleanup;
		}

		rc = signal_protocol_key_helper_generate_pre_keys(&preKeysHead, APreKeyStartId,
														  APreKeyAmount, FGlobalContext);
		if(rc) {
			errMsg = "failed to generate pre keys";
			goto cleanup;
		}

		rc = signal_protocol_key_helper_generate_signed_pre_key(
					&signedPreKey, identityKeyPair, ASignedPreKeyId,
					quint64(QDateTime::currentMSecsSinceEpoch()),
					FGlobalContext);
		if (rc) {
			errMsg = "failed to generate signed pre key";
			goto cleanup;
		}

		rc = identitySetLocalRegistrationId(this, registrationId);
		if (rc) {
			errMsg = "failed to set registration id";
			goto cleanup;
		}

		rc = identitySetKeyPair(identityKeyPair, this);
		if (rc) {
			errMsg = "failed to set identity key pair";
			goto cleanup;
		}

		rc = preKeyStoreList(preKeysHead, this);
		if (rc) {
			errMsg = "failed to save pre key list";
			goto cleanup;
		}

		rc = session_signed_pre_key_serialize(&signedPreKeyData, signedPreKey);
		if (rc) {
			errMsg = "failed to serialize signed pre key";
			goto cleanup;
		}

		rc = signedPreKeyStore(session_signed_pre_key_get_id(signedPreKey),
											  signal_buffer_data(signedPreKeyData),
											  signal_buffer_len(signedPreKeyData),
											  this);
		if (rc) {
			errMsg = "failed to save signed pre key";
			goto cleanup;
		}

		rc = initStatusSet(DbInitialized, this);
		if (rc) {
			errMsg = "failed to set init status to DbNeedsRollback";
			goto cleanup;
		}
	}

cleanup:
	if (rc < 0)
		qCritical("%s: %s", __func__, errMsg);

	if (dbNeedsInit) {
		SIGNAL_UNREF(identityKeyPair);
		signal_protocol_key_helper_key_list_free(preKeysHead);
		SIGNAL_UNREF(signedPreKey);
		signal_buffer_bzero_free(signedPreKeyData);
	}

	return rc;
}

quint32 SignalProtocol::getDeviceId()
{
	quint32 id;
	int rc = signal_protocol_identity_get_local_registration_id(FStoreContext, &id);
	if (rc == SG_SUCCESS)
		return id;
	qCritical("%s: signal_protocol_identity_get_local_registration_id() failed! rc=%d",
			  __func__, rc);
	return 0;
}

int SignalProtocol::sessionInitStatus(const QString &ABareJid, qint32 ADeviceId)
{
	QByteArray bareJid = ABareJid.toUtf8();
	signal_protocol_address address = {bareJid.data(),
									   size_t(bareJid.size()),
									   ADeviceId};

	int rc = 0;
	char * errMsg = nullptr;

	session_record * sessionRecord = nullptr;
	session_state * sessionState = nullptr;

	if(!signal_protocol_session_contains_session(FStoreContext, &address))
		return NoSession;

	rc = signal_protocol_session_load_session(FStoreContext, &sessionRecord, &address);
	if (rc){
		errMsg = "Database error when trying to retrieve session";
		goto cleanup;
	} else {
		sessionState = session_record_get_state(sessionRecord);
		if (session_state_has_pending_key_exchange(sessionState)) {
			errMsg = "Session exists but has pending synchronous key exchange";
			rc = NoSession;
			goto cleanup;
		}
		if (session_state_has_unacknowledged_pre_key_message(sessionState))
		{
			errMsg = "Has unacknowledged PreKey message";
			rc = SessionInitiated;
		}
		else
		{
			errMsg = "Have no unacknowledged PreKey message";
			rc = SessionAcknowledged;
		}
	}

cleanup:
	if (rc < 1)
		qCritical("%s: %s", __func__, errMsg);
	else
		qDebug("%s: %s", __func__, errMsg);

	SIGNAL_UNREF(sessionRecord);
	return rc;
}

SignalProtocol::Cipher SignalProtocol::sessionCipherCreate(const QString &ABareJid, int ADeviceId)
{
	return Cipher(this, ABareJid, ADeviceId);
}

QByteArray SignalProtocol::signalBufferToByteArray(signal_buffer *ABuffer)
{
	size_t len = signal_buffer_len(ABuffer);
	if (len>0)
	{
		const uint8_t *data = signal_buffer_const_data(ABuffer);
		if (data)
			return BYTE_ARRAY(data, len);
		else
			qCritical("signal_buffer_const_data() returned NULL!");
	}
	else
		qCritical("signal_buffer_len() returned: %d", len);
	return QByteArray();
}

QByteArray SignalProtocol::getIdentityKeyPublic() const
{
	ratchet_identity_key_pair *keyPair;
	QByteArray retVal;
	int rc = signal_protocol_identity_get_key_pair(FStoreContext, &keyPair);
	if (rc == SG_SUCCESS)
	{
		ec_public_key *key = ratchet_identity_key_pair_get_public(keyPair);
		if (key)
		{
			signal_buffer *buffer;
			rc = ec_public_key_serialize(&buffer, key);
			if (rc ==SG_SUCCESS)
			{
				retVal = signalBufferToByteArray(buffer);
				signal_buffer_free(buffer);
			}
			else
				qCritical("ec_public_key_serialize() failed! rc=%d", rc);
		}
		else
			qCritical("ratchet_identity_key_pair_get_public() returned NULL!");
	}
	else
		qCritical("signal_protocol_identity_get_key_pair() failed! rc=%d", rc);

	return retVal;
}

QByteArray SignalProtocol::getIdentityKeyPrivate() const
{
	ratchet_identity_key_pair *keyPair = nullptr;
	QByteArray retVal;
	int rc = signal_protocol_identity_get_key_pair(FStoreContext, &keyPair);
	if (rc == SG_SUCCESS)
	{
		ec_private_key *key = ratchet_identity_key_pair_get_private(keyPair);
		if (key)
		{
			signal_buffer *buffer;
			rc = ec_private_key_serialize(&buffer, key);
			if (rc ==SG_SUCCESS)
			{
				retVal = signalBufferToByteArray(buffer);
				signal_buffer_bzero_free(buffer);
			}
			else
				qCritical("ec_public_key_serialize() failed! rc=%d", rc);
		}
		else
			qCritical("ratchet_identity_key_pair_get_public() returned NULL!");
	}
	else
		qCritical("signal_protocol_identity_get_key_pair() failed! rc=%d", rc);

	return retVal;
}

QByteArray SignalProtocol::getSignedPreKeyPublic(quint32 AKeyId) const
{
	QByteArray retVal;
	session_signed_pre_key *preKey;
	int rc = signal_protocol_signed_pre_key_load_key(FStoreContext, &preKey, AKeyId);
	if (rc == SG_SUCCESS)
	{
		ec_key_pair *keyPair = session_signed_pre_key_get_key_pair(preKey);
		if (keyPair)
		{
			ec_public_key *key = ec_key_pair_get_public(keyPair);
			if (key)
			{
				signal_buffer *buffer;
				rc = ec_public_key_serialize(&buffer, key);
				if (rc == SG_SUCCESS)
				{
					retVal = signalBufferToByteArray(buffer);
					signal_buffer_free(buffer);
				}
				else
					qCritical("ec_public_key_serialize() failed! rc=%d", rc);
			}
			else
				qCritical("ec_key_pair_get_public() returned NULL!");
		}
		else
			qCritical("session_signed_pre_key_get_key_pair() returned NULL!");
	}
	else
		qCritical("signal_protocol_signed_pre_key_load_key() failed! rc=%d", rc);

	return retVal;
}

QByteArray SignalProtocol::getSignedPreKeySignature(quint32 AKeyId) const
{
	QByteArray retVal;
	session_signed_pre_key *preKey;
	int rc = signal_protocol_signed_pre_key_load_key(FStoreContext, &preKey, AKeyId);
	if (rc == SG_SUCCESS)
	{
		const size_t sigLen = session_signed_pre_key_get_signature_len(preKey);
		if (sigLen>0)
		{
			const quint8 *signature = session_signed_pre_key_get_signature(preKey);
			if (signature)
			{
				retVal = BYTE_ARRAY(signature, sigLen);
			}
			else
				qCritical("session_signed_pre_key_get_signature() returned NULL!");
		}
		else
			qCritical("session_signed_pre_key_get_signature_len() returned: %d!", sigLen);
	}
	else
		qCritical("signal_protocol_signed_pre_key_load_key() failed! rc=%d", rc);

	return retVal;
}

QByteArray SignalProtocol::getPreKeyPublic(quint32 AKeyId) const
{
	QByteArray retVal;
	session_pre_key *preKey;
	int rc = signal_protocol_pre_key_load_key(FStoreContext, &preKey, AKeyId);
	if (rc == SG_SUCCESS)
	{
		ec_key_pair *keyPair = session_pre_key_get_key_pair(preKey);
		if (keyPair)
		{
			ec_public_key *key = ec_key_pair_get_public(keyPair);
			if (key)
			{
				signal_buffer *buffer;
				rc = ec_public_key_serialize(&buffer, key);
				if (rc == SG_SUCCESS)
				{
					retVal = signalBufferToByteArray(buffer);
					signal_buffer_free(buffer);
				}
				else
					qCritical("ec_public_key_serialize() failed! rc=%d", rc);
			}
			else
				qCritical("ec_key_pair_get_public() returned NULL!");
		}
		else
			qCritical("session_pre_key_get_key_pair() returned NULL!");
	}
	else
		qCritical("signal_protocol_pre_key_load_key() failed! rc=%d", rc);

	return retVal;
}

QByteArray SignalProtocol::getPreKeyPrivate(quint32 AKeyId) const
{
	QByteArray retVal;
	session_pre_key *preKey;
	int rc = signal_protocol_pre_key_load_key(FStoreContext, &preKey, AKeyId);
	if (rc == SG_SUCCESS)
	{
		ec_key_pair *keyPair = session_pre_key_get_key_pair(preKey);
		if (keyPair)
		{
			ec_private_key *key = ec_key_pair_get_private(keyPair);
			if (key)
			{
				signal_buffer *buffer;
				rc = ec_private_key_serialize(&buffer, key);
				if (rc == SG_SUCCESS)
				{
					retVal = signalBufferToByteArray(buffer);
					signal_buffer_bzero_free(buffer);
				}
				else
					qCritical("ec_private_key_serialize() failed! rc=%d", rc);
			}
			else
				qCritical("ec_key_pair_get_private() returned NULL!");
		}
		else
			qCritical("session_pre_key_get_key_pair() returned NULL!");
	}
	else
		qCritical("signal_protocol_pre_key_load_key() failed! rc=%d", rc);

	return retVal;
}

QMap<quint32, QByteArray> SignalProtocol::getPreKeys() const
{
	QMap<quint32, QByteArray> preKeys;
	preKeyGetList(PRE_KEYS_AMOUNT, preKeys, this);
	return preKeys;
}

QList<IdentityKey> SignalProtocol::getIdentityKeys() const
{
	QList<IdentityKey> identityKeys;
	identityKeyGetList(0, identityKeys, this);
	return identityKeys;
}

session_pre_key_bundle *SignalProtocol::createPreKeyBundle(uint32_t ARegistrationId,
														   int ADeviceId, uint32_t APreKeyId,
														   const QByteArray &APreKeyPublic,
														   uint32_t ASignedPreKeyId,
														   const QByteArray &ASignedPreKeyPublic,
														   const QByteArray &ASignedPreKeySignature,
														   const QByteArray &AIdentityKey) const
{
	char *errMsg(nullptr);
	session_pre_key_bundle *bundle(nullptr);
	ec_public_key *preKeyPublic(nullptr),
				  *signedPreKeyPublic(nullptr),
				  *identityKey(nullptr);

	int rc = curve_decode_point(&preKeyPublic, DATA_SIZE(APreKeyPublic), FGlobalContext);
	if (rc != SG_SUCCESS) {
		errMsg = "curve_decode_point() failed! (public pre key)";
		goto cleanup;
	}

	rc = curve_decode_point(&signedPreKeyPublic, DATA_SIZE(ASignedPreKeyPublic), FGlobalContext);
	if (rc != SG_SUCCESS) {
		errMsg = "curve_decode_point() failed! (public signed pre key)";
		goto cleanup;
	}

	rc = curve_decode_point(&identityKey, DATA_SIZE(AIdentityKey), FGlobalContext);
	if (rc != SG_SUCCESS) {
		errMsg = "curve_decode_point() failed! (identity key)";
		goto cleanup;
	}

	rc = session_pre_key_bundle_create(&bundle, ARegistrationId, ADeviceId,
									   APreKeyId, preKeyPublic,
									   ASignedPreKeyId,
									   signedPreKeyPublic,
									   DATA_SIZE(ASignedPreKeySignature),
									   identityKey);
	if (rc != SG_SUCCESS) {
		errMsg = "session_pre_key_bundle_create() failed!";
		goto cleanup;
	}

cleanup:
	if (errMsg)
		qCritical("%s: rc=%d", errMsg, rc);

	SIGNAL_UNREF(identityKey);
	SIGNAL_UNREF(signedPreKeyPublic);
	SIGNAL_UNREF(preKeyPublic);

	return bundle;
}

SignalProtocol::SignalMessage SignalProtocol::getSignalMessage(const QByteArray &AEncrypted)
{
	return SignalMessage(FGlobalContext, AEncrypted);
}

SignalProtocol::PreKeySignalMessage SignalProtocol::getPreKeySignalMessage(const QByteArray &AEncrypted)
{
	return PreKeySignalMessage(FGlobalContext, AEncrypted);
}

SignalProtocol::SessionBuilder SignalProtocol::getSessionBuilder(const QString &ABareJid, quint32 ADeviceId)
{
	return SessionBuilder(ABareJid, ADeviceId, FGlobalContext, FStoreContext);
}

int SignalProtocol::generateIdentityKeyPair(ratchet_identity_key_pair **AIdentityKeyPair)
{
	return signal_protocol_key_helper_generate_identity_key_pair(AIdentityKeyPair, FGlobalContext);
}

int SignalProtocol::generateRegistrationId(quint32 *ARegistrationId, int AExtendedRange)
{
	return signal_protocol_key_helper_generate_registration_id(ARegistrationId, AExtendedRange, FGlobalContext);
}

int SignalProtocol::generatePreKeys(signal_protocol_key_helper_pre_key_list_node **APreKeyList, uint AStart, uint ACount)
{
	return signal_protocol_key_helper_generate_pre_keys(APreKeyList, AStart, ACount, FGlobalContext);
}

int SignalProtocol::generateSignedPreKey(session_signed_pre_key **ASignedPreKey,
										 const ratchet_identity_key_pair *AIdentityKeyPair,
										 uint32_t ASignedPreKeyId,
										 uint64_t ATimestamp)
{
	return signal_protocol_key_helper_generate_signed_pre_key(ASignedPreKey, AIdentityKeyPair, ASignedPreKeyId, ATimestamp, FGlobalContext);
}

static int chooseAes(int ACipher, size_t AKeyLen, int &AAlgo, int &AMode) {
  int algo = 0;
  int mode = 0;

  switch(AKeyLen) {
	case 16:
	  algo = GCRY_CIPHER_AES128;
	  break;
	case 24:
	  algo = GCRY_CIPHER_AES192;
	  break;
	case 32:
	  algo = GCRY_CIPHER_AES256;
	  break;
	default:
	  return SG_ERR_UNKNOWN;
  }

  switch (ACipher) {
	case SG_CIPHER_AES_CBC_PKCS5:
	  mode = GCRY_CIPHER_MODE_CBC;
	  break;
	case SG_CIPHER_AES_CTR_NOPADDING:
	  mode = GCRY_CIPHER_MODE_CTR;
	  break;
	default:
	  return SG_ERR_UNKNOWN;
  }

  AAlgo = algo;
  AMode = mode;

  return 0;
}

int SignalProtocol::randomFunc(uint8_t *AData, size_t ALen, void *AUserData)
{
	Q_UNUSED(AUserData);

	gcry_randomize(AData, ALen, GCRY_STRONG_RANDOM);

	return SG_SUCCESS;
}

int SignalProtocol::hmacSha256InitFunc(void **AHmacContext, const uint8_t *AKey, size_t AKeyLen, void *AUserData)
{
	Q_UNUSED(AUserData);

	int ret_val = 0;
	char * err_msg = nullptr;

	gcry_mac_hd_t * hmac_hd_p = nullptr;

	hmac_hd_p = reinterpret_cast<gcry_mac_hd_t *>(malloc(sizeof(gcry_mac_hd_t)));
	if (!hmac_hd_p) {
		err_msg = "could not malloc hmac-sha256 ctx";
		ret_val = SG_ERR_NOMEM;
		goto cleanup;
	}

	ret_val = int(gcry_mac_open(hmac_hd_p, GCRY_MAC_HMAC_SHA256, 0, nullptr));
	if (ret_val) {
		err_msg = "could not create hmac-sha256 ctx";
		goto cleanup;
	}

	ret_val = int(gcry_mac_setkey(*hmac_hd_p, AKey, AKeyLen));
	if (ret_val) {
		err_msg = "could not set key for hmac";
		goto cleanup;
	}

	*AHmacContext = hmac_hd_p;

cleanup:
	if (ret_val) {
		if (ret_val > 0) {
			qCritical("%s: %s (%s: %s)\n", __func__, err_msg, gcry_strsource(ret_val), gcry_strerror(ret_val));
			ret_val = SG_ERR_UNKNOWN;
		} else {
			qCritical("%s: %s\n", __func__, err_msg);
		}

		if (hmac_hd_p) {
			gcry_mac_close(*hmac_hd_p);
			free(hmac_hd_p);
		}
	}

	return ret_val;
}

int SignalProtocol::hmacSha256UpdateFunc(void *AHmacContext, const uint8_t *AData,
										 size_t ADataLen, void *AUserData)
{
	Q_UNUSED(AUserData);

	gcry_mac_write(*reinterpret_cast<gcry_mac_hd_t *>(AHmacContext), AData, ADataLen);

	return SG_SUCCESS;
}

int SignalProtocol::hmacSha256FinalFunc(void *AHmacContext, signal_buffer **AOutput, void *AUserData)
{
	Q_UNUSED(AUserData);

	int ret_val = 0;
	char * err_msg = nullptr;

	int algo = GCRY_MAC_HMAC_SHA256;
	size_t mac_len = 0;
	uchar * mac_data_p = nullptr;
	signal_buffer * out_buf_p = nullptr;

	mac_len = gcry_mac_get_algo_maclen(algo);

	mac_data_p = reinterpret_cast<uchar *>(malloc(sizeof(uint8_t) * mac_len));
	if (!mac_data_p) {
		ret_val = SG_ERR_NOMEM;
		err_msg = "failed to malloc mac buf";
		goto cleanup;
	}

	ret_val = gcry_mac_read(*reinterpret_cast<gcry_mac_hd_t *>(AHmacContext), mac_data_p, &mac_len);
	if (ret_val) {
		err_msg = "failed to read mac";
		goto cleanup;
	}

	out_buf_p = signal_buffer_create(mac_data_p, mac_len);
	if (!out_buf_p) {
		ret_val = SG_ERR_NOMEM;
		err_msg = "failed to create mac output buf";
		goto cleanup;
	}

	*AOutput = out_buf_p;

	cleanup:
	if (ret_val) {
		if (ret_val > 0) {
			qCritical("%s: %s (%s: %s)\n", __func__, err_msg, gcry_strsource(ret_val), gcry_strerror(ret_val));
			ret_val = SG_ERR_UNKNOWN;
		} else {
			qCritical("%s: %s\n", __func__, err_msg);
		}
	}
	free(mac_data_p);

	return ret_val;
}

void SignalProtocol::hmacSha256CleanupFunc(void *AHmacContext, void *AUserData)
{
	Q_UNUSED(AUserData);

	gcry_mac_hd_t * macHd = reinterpret_cast<gcry_mac_hd_t *>(AHmacContext);
	gcry_mac_close(*macHd);
	free(macHd);
}

int SignalProtocol::sha512DigestInitFunc(void **ADigestContext, void *AUserData)
{
	Q_UNUSED(AUserData);

	int rc = 0;
	char * errMsg = nullptr;

	gcry_md_hd_t * hashHd = reinterpret_cast<gcry_md_hd_t*>(malloc(sizeof(gcry_mac_hd_t)));
	if (!hashHd) {
	  errMsg = "could not malloc sha512 ctx";
	  rc = SG_ERR_NOMEM;
	  goto cleanup;
	}

	rc = int(gcry_md_open(hashHd, GCRY_MD_SHA512, 0));
	if (rc) {
	  errMsg = "could not create sha512 ctx";
	  goto cleanup;
	}

	*ADigestContext = hashHd;

cleanup:
	if (rc) {
		if (rc > 0) {
			qCritical("%s: %s (%s: %s)\n", __func__, errMsg,
					  gcry_strsource(gcry_error_t(rc)), gcry_strerror(gcry_error_t(rc)));
			rc = SG_ERR_UNKNOWN;
		} else {
			qCritical("%s: %s\n", __func__, errMsg);
		}

		if (hashHd) {
			gcry_md_close(*hashHd);
			free(hashHd);
		}
	}

	return rc;
}

int SignalProtocol::sha512DigestUpdateFunc(void *ADigestContext, const uint8_t *AData,
										   size_t ADataLen, void *AUserData)
{
	Q_UNUSED(AUserData);

	gcry_md_write(*reinterpret_cast<gcry_md_hd_t *>(ADigestContext), AData, ADataLen);

	return SG_SUCCESS;
}

int SignalProtocol::sha512DigestFinalFunc(void *ADigestContext, signal_buffer **AOutput,
										  void *AUserData)
{
	Q_UNUSED(AUserData);

	gcry_md_hd_t * hashHd = reinterpret_cast<gcry_md_hd_t *>(ADigestContext);
	int rc = 0;
	char * errMsg = nullptr;

	const int algo = GCRY_MD_SHA512;
	signal_buffer * outBuf = nullptr;

	size_t hashLen = gcry_md_get_algo_dlen(algo);

	unsigned char * hashData = gcry_md_read(*hashHd, algo);
	if (!hashData) {
		rc = SG_ERR_UNKNOWN;
		errMsg = "failed to read hash";
		goto cleanup;
	}

	outBuf = signal_buffer_create(hashData, hashLen);
	if (!outBuf) {
		rc = SG_ERR_NOMEM;
		errMsg = "failed to create hash output buf";
		goto cleanup;
	}

	gcry_md_reset(*hashHd);

	*AOutput = outBuf;

cleanup:
	if (rc) {
		if (rc > 0) {
			qCritical("%s: %s (%s: %s)\n", __func__, errMsg,
					  gcry_strsource(gcry_error_t(rc)), gcry_strerror(gcry_error_t(rc)));
			rc = SG_ERR_UNKNOWN;
		} else {
			qCritical("%s: %s\n", __func__, errMsg);
		}
	}

	return rc;
}

void SignalProtocol::sha512DigestCleanupFunc(void *ADigestContext, void *AUserData)
{
	Q_UNUSED(AUserData);

	gcry_md_hd_t * hashHd = reinterpret_cast<gcry_md_hd_t *>(ADigestContext);
	gcry_md_close(*hashHd);
	free(hashHd);
}

int SignalProtocol::encryptFunc(signal_buffer **AOutput, int ACipher, const uint8_t *AKey,
								size_t AKeyLen, const uint8_t *AIv, size_t AIvLen,
								const uint8_t *APlaintext, size_t APlaintextLen, void *AUserData)
{
	Q_UNUSED(AUserData);

	int rc = SG_SUCCESS;
	char * errMsg = nullptr;

	int algo = 0;
	int mode = 0;
	size_t padLen = 0;
	size_t ctLen = 0;
	gcry_cipher_hd_t cipherHd = nullptr;
	uchar * pt = nullptr;
	uchar * out = nullptr;
	signal_buffer * outBuf = nullptr;

	if(AIvLen != 16) {
		errMsg = "invalid AES IV size (must be 16)";
		rc = SG_ERR_UNKNOWN;
		goto cleanup;
	}

	rc = chooseAes(ACipher, AKeyLen, algo, mode);
	if (rc) {
		errMsg = "failed to choose cipher";
		rc = SG_ERR_UNKNOWN;
		goto cleanup;
	}

	rc = gcry_cipher_open(&cipherHd, algo, mode, 0);
	if (rc) {
		errMsg = "failed to init cipher";
		goto cleanup;
	}

	rc = gcry_cipher_setkey(cipherHd, AKey, AKeyLen);
	if (rc) {
		errMsg = "failed to set key";
		goto cleanup;
	}

	switch (ACipher) {
		case SG_CIPHER_AES_CBC_PKCS5:
			padLen = 16 - (APlaintextLen % 16);
			if (padLen == 0) {
			  padLen = 16;
			}
			ctLen = APlaintextLen + padLen;
			rc = int(gcry_cipher_setiv(cipherHd, AIv, AIvLen));
			if (rc) {
			  errMsg = "failed to set iv";
			  goto cleanup;
			}
			break;
		case SG_CIPHER_AES_CTR_NOPADDING:
			ctLen = APlaintextLen;
			rc = int(gcry_cipher_setctr(cipherHd, AIv, AIvLen));
			if (rc) {
				errMsg = "failed to set iv";
				goto cleanup;
			}
			break;
		default:
			rc = SG_ERR_UNKNOWN;
			errMsg = "unknown cipher";
			goto cleanup;
	}

	pt = reinterpret_cast<uint8_t*>(malloc(sizeof(uint8_t) * ctLen));
	if (!pt) {
		errMsg = "failed to malloc pt buf";
		rc = SG_ERR_NOMEM;
		goto cleanup;
	}
	memset(pt, int(padLen), ctLen);
	memcpy(pt, APlaintext, APlaintextLen);

	out = reinterpret_cast<uchar*>(malloc(sizeof(uint8_t) * ctLen));
	if (!out) {
		errMsg = "failed to malloc ct buf";
		rc = SG_ERR_NOMEM;
		goto cleanup;
	}

	rc = gcry_cipher_encrypt(cipherHd, out, ctLen, pt, ctLen);
	if (rc) {
		errMsg = "failed to encrypt";
		goto cleanup;
	}

	outBuf = signal_buffer_create(out, ctLen);
	*AOutput = outBuf;

cleanup:
	if (rc) {
		if (rc > 0) {
			qCritical("%s: %s (%s: %s)\n", __func__, errMsg, gcry_strsource(rc), gcry_strerror(rc));
			rc = SG_ERR_UNKNOWN;
		} else {
			qCritical("%s: %s\n", __func__, errMsg);
		}
	}

	free(out);
	gcry_cipher_close(cipherHd);

	return rc;
}

int SignalProtocol::decryptFunc(signal_buffer **AOutput, int ACipher, const uint8_t *AKey, size_t AKeyLen, const uint8_t *AIv, size_t AIvLen, const uint8_t *ACiphertext, size_t ACiphertextLen, void *AUserData)
{
	Q_UNUSED(AUserData);

	int rc = SG_SUCCESS;
	char * errMsg = nullptr;

	int algo = 0;
	int mode = 0;
	gcry_cipher_hd_t cipherHd = nullptr;
	uchar * out = nullptr;
	size_t padLen = 0;
	signal_buffer * outBuf = nullptr;

	if(AIvLen != 16) {
		errMsg = "invalid AES IV size (must be 16)";
		rc = SG_ERR_UNKNOWN;
		goto cleanup;
	}

	rc = chooseAes(ACipher, AKeyLen, algo, mode);
	if (rc) {
		errMsg = "failed to choose cipher";
		rc = SG_ERR_UNKNOWN;
		goto cleanup;
	}

	rc = int(gcry_cipher_open(&cipherHd, algo, mode, 0));
	if (rc) {
		errMsg = "failed to init cipher";
		goto cleanup;
	}

	rc = int(gcry_cipher_setkey(cipherHd, AKey, AKeyLen));
	if (rc) {
	  errMsg = "failed to set key";
	  goto cleanup;
	}

	switch (ACipher) {
		case SG_CIPHER_AES_CBC_PKCS5:
			padLen = 1;
			rc = int(gcry_cipher_setiv(cipherHd, AIv, AIvLen));
			if (rc) {
				errMsg = "failed to set iv";
				goto cleanup;
			}
			break;
		case SG_CIPHER_AES_CTR_NOPADDING:
			rc = int(gcry_cipher_setctr(cipherHd, AIv, AIvLen));
			if (rc) {
				errMsg = "failed to set iv";
				goto cleanup;
			}
			break;
		default:
			rc = SG_ERR_UNKNOWN;
			errMsg = "unknown cipher";
			goto cleanup;
	}

	out = reinterpret_cast<uchar*>(malloc(sizeof(uchar) * ACiphertextLen));
	if (!out) {
		errMsg = "failed to malloc pt buf";
		rc = SG_ERR_NOMEM;
		goto cleanup;
	}

	rc = int(gcry_cipher_decrypt(cipherHd, out, ACiphertextLen, ACiphertext, ACiphertextLen));
	if (rc) {
		errMsg = "failed to decrypt";
		goto cleanup;
	}

	if (padLen) {
		padLen = out[ACiphertextLen - 1];
	}

	outBuf = signal_buffer_create(out, ACiphertextLen - padLen);
	*AOutput = outBuf;

cleanup:
	if (rc) {
		if (rc > 0) {
			qCritical("%s: %s (%s: %s)\n", __func__, errMsg, gcry_strsource(rc), gcry_strerror(rc));
			rc = SG_ERR_UNKNOWN;
		} else {
			qCritical("%s: %s\n", __func__, errMsg);
		}
	}

	free(out);
	gcry_cipher_close(cipherHd);

	return rc;
}

void SignalProtocol::logFunc(int ALevel, const char *AMessage, size_t ALen, void *AUserData)
{
	switch (ALevel)
	{
		case SG_LOG_ERROR:
			qCritical("%s", AMessage);
			break;
		case SG_LOG_WARNING:
			qWarning("%s", AMessage);
			break;
		case SG_LOG_NOTICE:
		case SG_LOG_INFO:
			qInfo("%s", AMessage);
			break;
		case SG_LOG_DEBUG:
			qDebug("%s", AMessage);
			break;
	}
}

void SignalProtocol::recursiveMutexLock(void *AUserData)
{
	reinterpret_cast<SignalProtocol *>(AUserData)->recursiveMutexLock();
}

void SignalProtocol::recursiveMutexUnlock(void *AUserData)
{
	reinterpret_cast<SignalProtocol *>(AUserData)->recursiveMutexUnlock();
}

void SignalProtocol::recursiveMutexLock()
{
	FMutex->lock();
}

void SignalProtocol::recursiveMutexUnlock()
{
	FMutex->unlock();
}

SignalProtocol::SignalProtocol(const QString &AFileName, const QString &AConnectionName, IIdentityKeyListener *AIdentityKeyListener):
	FGlobalContext(nullptr),
	FStoreContext(nullptr),
	FIdentityKeyListener(AIdentityKeyListener),
	FConnectionName(AConnectionName),
	FMutex(new QMutex(QMutex::Recursive)),
	FError(0)
{
	qDebug("SignalProtocol::SignalProtocol(\"%s\")", AFileName.toUtf8().data());

	char *errMsg = nullptr;

	// 1. create global context
	if (signal_context_create(&FGlobalContext, this)) {
		errMsg = "failed to create global signal protocol context";
		FError = -1;
		goto cleanup;
	}
	qDebug("%s: created and set signal protocol context", __func__);

	// 2. init and set crypto provider
	signal_crypto_provider provider;
	provider.random_func = randomFunc;
	provider.hmac_sha256_init_func = hmacSha256InitFunc;
	provider.hmac_sha256_update_func = hmacSha256UpdateFunc;
	provider.hmac_sha256_final_func = hmacSha256FinalFunc;
	provider.hmac_sha256_cleanup_func = hmacSha256CleanupFunc;
	provider.sha512_digest_init_func = sha512DigestInitFunc;
	provider.sha512_digest_update_func = sha512DigestUpdateFunc;
	provider.sha512_digest_final_func = sha512DigestFinalFunc;
	provider.sha512_digest_cleanup_func = sha512DigestCleanupFunc;
	provider.encrypt_func = encryptFunc;
	provider.decrypt_func = decryptFunc;
	provider.user_data = this;

	if (signal_context_set_crypto_provider(FGlobalContext, &provider)) {
		errMsg = "failed to set signal protocol crypto provider";
		FError = -1;
		goto cleanup;
	}

	// 3. set locking functions
	if (signal_context_set_locking_functions(FGlobalContext, recursiveMutexLock, recursiveMutexUnlock)) {
		errMsg = "failed to set locking functions";
		FError = -1;
		goto cleanup;
	}

	// 3. set logging function
	if (signal_context_set_log_function(FGlobalContext,	logFunc)) {
		errMsg = "failed to set logging function";
		FError = -1;
		goto cleanup;
	}

	// Init store context
	if (signal_protocol_store_context_create(&FStoreContext, FGlobalContext)) {
		errMsg = "failed to create store context";
		FError = -1;
		goto cleanup;
	}

	signal_protocol_session_store sessionStore;
	sessionStore.load_session_func = &sessionLoad;
	sessionStore.get_sub_device_sessions_func = &sessionGetSubDeviceSessions;
	sessionStore.store_session_func = &OmemoStore::sessionStore;
	sessionStore.contains_session_func = &sessionContains;
	sessionStore.delete_session_func = &sessionDelete;
	sessionStore.delete_all_sessions_func = &sessionDeleteAll;
	sessionStore.destroy_func = &sessionDestroyStoreCtx;
	sessionStore.user_data = this;

	if (signal_protocol_store_context_set_session_store(FStoreContext, &sessionStore)) {
		errMsg = "failed to create session store";
		FError = -1;
		goto cleanup;
	}

	signal_protocol_pre_key_store preKeyStore;
	preKeyStore.load_pre_key = &preKeyLoad;
	preKeyStore.store_pre_key = &OmemoStore::preKeyStore;
	preKeyStore.contains_pre_key = &preKeyContains;
	preKeyStore.remove_pre_key = &preKeyRemove;
	preKeyStore.destroy_func = &preKeyDestroyCtx;
	preKeyStore.user_data = this;

	if (signal_protocol_store_context_set_pre_key_store(FStoreContext, &preKeyStore)) {
		errMsg = "failed to set pre key store";
		FError = -1;
		goto cleanup;
	}

	signal_protocol_signed_pre_key_store signedPreKeyStore;
	signedPreKeyStore.load_signed_pre_key = &signedPreKeyLoad;
	signedPreKeyStore.store_signed_pre_key = &OmemoStore::signedPreKeyStore;
	signedPreKeyStore.contains_signed_pre_key = &signedPreKeyContains;
	signedPreKeyStore.remove_signed_pre_key = &signedPreKeyRemove;
	signedPreKeyStore.destroy_func = &signedPreKeyDestroyCtx;
	signedPreKeyStore.user_data = this;

	if (signal_protocol_store_context_set_signed_pre_key_store(FStoreContext, &signedPreKeyStore)) {
		errMsg = "failed to set signed pre key store";
		FError = -1;
		goto cleanup;
	}

	signal_protocol_identity_key_store identityKeyStore;
	identityKeyStore.get_identity_key_pair = &identityGetKeyPair;
	identityKeyStore.get_local_registration_id = &identityGetLocalRegistrationId;
	identityKeyStore.save_identity = &identitySave;
	identityKeyStore.is_trusted_identity = &identityIsTrusted;
	identityKeyStore.destroy_func = &identityDestroyCtx;
	identityKeyStore.user_data = this;

	if (signal_protocol_store_context_set_identity_key_store(FStoreContext, &identityKeyStore)) {
		errMsg = "failed to set identity key store";
		FError = -1;
		goto cleanup;
	}

cleanup:
	if (FError < 0) {
		qCritical("%s: %s", __func__, errMsg);
	} else {
		addDatabase(AFileName, FConnectionName);
		qDebug("%s: done initializing SignalProtocol", __func__);
	}
}

SignalProtocol::~SignalProtocol()
{
	removeDatabase(FConnectionName);
	// Uninit store context
	signal_protocol_store_context_destroy(FStoreContext);
}

//
// *** SessionBuilder ***
//
// SessionBuilderData
//
SignalProtocol::SessionBuilderData::SessionBuilderData(const QString &ABareJid, quint32 ADeviceId):
	FBareJid(ABareJid.toUtf8()),
	FAddress({FBareJid.data(),
			 size_t(ABareJid.size()),
			 int(ADeviceId)}),
	FBuilder(nullptr)
{}

SignalProtocol::SessionBuilderData::SessionBuilderData(const SignalProtocol::SessionBuilderData &AOther):
	FBareJid(AOther.FBareJid),
	FAddress(AOther.FAddress),
	FBuilder(AOther.FBuilder)
{}

SignalProtocol::SessionBuilderData::~SessionBuilderData()
{
	session_builder_free(FBuilder);
}
//
// SessionBuilder
//
SignalProtocol::SessionBuilder::SessionBuilder(const QString &ABareJid, quint32 ADeviceId,
											   signal_context *AGlobalContext,
											   signal_protocol_store_context *AStoreContext):
	d(new SessionBuilderData(ABareJid, ADeviceId))
{
	// Instantiate a session_builder for a recipient address.
	int rc = session_builder_create(&d->FBuilder, AStoreContext, &d->FAddress, AGlobalContext);
	if (rc != SG_SUCCESS)
	{
		d = nullptr;
		qCritical("session_builder_create() failed! rc=%d", rc);
	}
}

SignalProtocol::SessionBuilder::SessionBuilder(const SignalProtocol::SessionBuilder &AOther):
	d(AOther.d)
{}

SignalProtocol::SessionBuilder::~SessionBuilder()
{}

bool SignalProtocol::SessionBuilder::processPreKeyBundle(session_pre_key_bundle *APreKey)
{
	if (d)
	{
		int rc = session_builder_process_pre_key_bundle(d->FBuilder, APreKey);
		if (rc == SG_SUCCESS)
			return true;
		else
			qCritical("session_builder_process_pre_key_bundle() failed! rc=%d", rc);
	}
	else
		qCritical("SessionBuilder is NULL!");
	return false;
}

bool SignalProtocol::SessionBuilder::isNull() const
{
	return d == nullptr;
}

//
// *** Cipher ***
//
SignalProtocol::Cipher::Cipher(SignalProtocol *ASignalProtocol,
							   const QString &ABareJid, int ADeviceId):
	FSignalProtocol(ASignalProtocol),
	FCipher(nullptr),
	FBareJid(ABareJid.toUtf8()),
	FAddress({FBareJid.data(), size_t(FBareJid.size()), ADeviceId})
{
	// Create the session cipher
	int rc = session_cipher_create(&FCipher, ASignalProtocol->storeContext(), &FAddress, ASignalProtocol->globalContext());
	if (rc != SG_SUCCESS)
	{
		FCipher = nullptr;
		qCritical("session_cipher_create() failed! rc=%d", rc);
	}
}

SignalProtocol::Cipher::Cipher(const SignalProtocol::Cipher &AOther):
	FCipher(AOther.FCipher),
	FBareJid(AOther.FBareJid)
{
	SIGNAL_REF(FCipher);
}

SignalProtocol::Cipher::~Cipher()
{
	SIGNAL_UNREF(FCipher);
}

bool SignalProtocol::Cipher::isNull() const
{
	return FCipher == nullptr;
}

QByteArray SignalProtocol::Cipher::encrypt(const QByteArray &AUnencrypted)
{
	QByteArray result;
	ciphertext_message *message;
	int rc = session_cipher_encrypt(FCipher,
									DATA_SIZE(AUnencrypted),
									&message);
	if (rc == SG_SUCCESS)
	{
		// Get the serialized content
		signal_buffer *serialized = ciphertext_message_get_serialized(message);
		result = SignalProtocol::signalBufferToByteArray(serialized);
		signal_buffer_free(serialized);
		SIGNAL_UNREF(message);
	}
	else
		qCritical("session_cipher_encrypt() failed! rc=%d", rc);

	return result;
}

QByteArray SignalProtocol::Cipher::decrypt(const SignalMessage &AMessage)
{
	QByteArray result;

	if (AMessage.isNull())
		qCritical("SignalMessage is NULL!");
	else
	{
		signal_buffer *buffer(nullptr);

		int rc = session_cipher_decrypt_signal_message(FCipher, AMessage, nullptr, &buffer);
		if (rc == SG_SUCCESS)
			result = SignalProtocol::signalBufferToByteArray(buffer);
		else
			qCritical("session_cipher_decrypt_signal_message() failed! rc=%d", rc);

		signal_buffer_bzero_free(buffer);
	}

	return result;
}

QByteArray SignalProtocol::Cipher::decrypt(const PreKeySignalMessage &AMessage, bool &APreKeysUpdated)
{
	QByteArray result;

	if (AMessage.isNull())
		qCritical("Message is NULL!");
	else
	{
		signal_buffer *plaintext(nullptr);

		char * errorMsg(nullptr);
		int cnt(0);
		int rc = session_cipher_decrypt_pre_key_signal_message(FCipher, AMessage,
															   nullptr, &plaintext);
		if (rc != SG_SUCCESS) {
			errorMsg = "PreKey Signal Message decryption failed";
			goto cleanup;
		}

		cnt = preKeyGetCount(FSignalProtocol);
		if (cnt >= 0) {
			quint32 preKeyId = AMessage.preKeyId();
			quint32 id(0);
			rc = preKeyGetMaxId(id, FSignalProtocol);
			if (rc) {
				errorMsg = "Failed to retrieve max pre key id";
				goto cleanup;
			}

			for (int i = cnt; i<100; ++i) {
				for (++id;
					 id == preKeyId ||
					 signal_protocol_pre_key_contains_key(FSignalProtocol->storeContext(), id);
					 ++id);

				signal_protocol_key_helper_pre_key_list_node * keyList = nullptr;
				rc = signal_protocol_key_helper_generate_pre_keys(&keyList, id, 1,
																  FSignalProtocol->globalContext());
				if (rc) {
					errorMsg = "failed to generate a new key";
					goto cleanup;
				}

				qDebug("New Pre Key generated! ID=%d", id);

				rc = signal_protocol_pre_key_store_key(FSignalProtocol->storeContext(),
													   signal_protocol_key_helper_key_list_element(keyList));
				signal_protocol_key_helper_key_list_free(keyList);
				if (rc) {
					errorMsg = "Failed to store new key";
					goto cleanup;
				}

				APreKeysUpdated = true;
			}
		}

		result = SignalProtocol::signalBufferToByteArray(plaintext);

cleanup:
		if (rc != SG_SUCCESS)
			qCritical("%s: %d", errorMsg, rc);

		signal_buffer_bzero_free(plaintext);		
	}

	return result;
}

SignalProtocol::Cipher SignalProtocol::Cipher::operator =(const SignalProtocol::Cipher &AOther)
{
	FCipher = AOther.FCipher;
	FBareJid = AOther.FBareJid;
	SIGNAL_REF(FCipher);
}

bool SignalProtocol::Cipher::operator ==(const SignalProtocol::Cipher &AOther) const
{
	return FCipher == AOther.FCipher;
}

bool SignalProtocol::Cipher::operator !=(const SignalProtocol::Cipher &AOther) const
{
	return !operator==(AOther);
}

//
// *** SignalMessage ***
//
SignalProtocol::SignalMessage::SignalMessage(const SignalProtocol::SignalMessage &AOther):
	FMessage(AOther.FMessage)
{
	SIGNAL_REF(FMessage);
}

SignalProtocol::SignalMessage::~SignalMessage()
{
	SIGNAL_UNREF(FMessage);
}

bool SignalProtocol::SignalMessage::isNull() const
{
	return FMessage == nullptr;
}

SignalProtocol::SignalMessage SignalProtocol::SignalMessage::operator =(const SignalProtocol::SignalMessage &AOther)
{
	FMessage = AOther.FMessage;
	SIGNAL_REF(FMessage);
}

bool SignalProtocol::SignalMessage::operator == (const SignalProtocol::SignalMessage &AOther) const
{
	return FMessage == AOther.FMessage;
}

bool SignalProtocol::SignalMessage::operator !=(const SignalProtocol::SignalMessage &AOther) const
{
	return !operator==(AOther);
}

SignalProtocol::SignalMessage::operator signal_message *() const
{
	return FMessage;
}

SignalProtocol::SignalMessage::SignalMessage(signal_context *AGlobalContext, const QByteArray &AEncrypted)
{
	int rc = signal_message_deserialize(&FMessage,
										DATA_SIZE(AEncrypted),
										AGlobalContext);
	if (rc != SG_SUCCESS)
	{
		qCritical("signal_message_deserialize() failed! rc=%d", rc);
		FMessage = nullptr;
	}

}

//
// *** PreKeySignalMessage ***
//
SignalProtocol::PreKeySignalMessage::PreKeySignalMessage(const SignalProtocol::PreKeySignalMessage &AOther):
	FMessage(AOther.FMessage)
{
	SIGNAL_REF(FMessage);
}

SignalProtocol::PreKeySignalMessage::~PreKeySignalMessage()
{
	SIGNAL_UNREF(FMessage);
}

bool SignalProtocol::PreKeySignalMessage::isNull() const
{
	return FMessage == nullptr;
}

SignalProtocol::PreKeySignalMessage SignalProtocol::PreKeySignalMessage::operator =(const SignalProtocol::PreKeySignalMessage &AOther)
{
	FMessage = AOther.FMessage;
	SIGNAL_REF(FMessage);
}

bool SignalProtocol::PreKeySignalMessage::operator ==(const SignalProtocol::PreKeySignalMessage &AOther) const
{
	return FMessage == AOther.FMessage;
}

bool SignalProtocol::PreKeySignalMessage::operator !=(const SignalProtocol::PreKeySignalMessage &AOther) const
{
	return !operator==(AOther);
}

bool SignalProtocol::PreKeySignalMessage::hasPreKeyId() const
{
	return pre_key_signal_message_has_pre_key_id(FMessage);
}

quint32 SignalProtocol::PreKeySignalMessage::preKeyId() const
{
	return pre_key_signal_message_get_pre_key_id(FMessage);
}

SignalProtocol::PreKeySignalMessage::PreKeySignalMessage(signal_context *AGlobalContext, const QByteArray &AEncrypted)
{
	char *errorMsg;
	int rc = pre_key_signal_message_deserialize(&FMessage,
												DATA_SIZE(AEncrypted),
												AGlobalContext);
	if (rc == SG_SUCCESS)
		return;

	FMessage = nullptr;

	if (rc == SG_ERR_INVALID_PROTO_BUF) {
		errorMsg = "Not a pre key msg";
	} else if (rc == SG_ERR_INVALID_KEY_ID) {
		errorMsg = "Invalid Key ID";
	} else if (rc != SG_SUCCESS) {
		errorMsg = "Failed to deserialize pre key message";
	}

	qCritical("PreKeySignalMessage constructor failed: %s: %d", errorMsg, rc);
}

SignalProtocol::PreKeySignalMessage::operator pre_key_signal_message *() const
{
	return FMessage;
}
