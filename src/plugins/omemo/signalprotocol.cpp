#include "signalprotocol.h"
#include "omemostore.h"

#include <QMutex>
#include <QDateTime>

extern "C" {
#include <gcrypt.h>
}
#include <key_helper.h>
#include <session_builder.h>
#include <session_cipher.h>
#include <session_state.h>
#include <protocol.h>

using namespace OmemoStore;

SignalProtocol* SignalProtocol::FInstance(nullptr);

SignalProtocol* SignalProtocol::instance(const QString &AFileName)
{
	return FInstance?FInstance:FInstance=new SignalProtocol(AFileName);
}

void SignalProtocol::init()
{
	gcry_check_version(nullptr);
	gcry_control(GCRYCTL_SUSPEND_SECMEM_WARN);
	gcry_control(GCRYCTL_INIT_SECMEM, 16384, 0);
	gcry_control(GCRYCTL_RESUME_SECMEM_WARN);
	gcry_control(GCRYCTL_USE_SECURE_RNDPOOL);
	gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
}

QString SignalProtocol::dbFileName() const
{
	return FFileName;
}

signal_context *SignalProtocol::globalContext()
{
	return FGlobalContext;
}

int SignalProtocol::error() const
{
	return FError;
}

int SignalProtocol::install(quint32 ASignedPreKeyId, uint APreKeyStartId, uint APreKeyAmount)
{
	char * err_msg = "";
	int ret_val = 0;
	bool db_needs_init = false;
	bool db_needs_reset = false;

	ratchet_identity_key_pair * identity_key_pair_p = nullptr;
	signal_protocol_key_helper_pre_key_list_node * pre_keys_head_p = nullptr;
	session_signed_pre_key * signed_pre_key_p = nullptr;
	signal_buffer * signed_pre_key_data_p = nullptr;
	uint32_t registration_id;
	int init_status = AXC_DB_NOT_INITIALIZED;

	qInfo("%s: calling install-time functions", __func__);

	ret_val = axc_db_create();
	if (ret_val){
		err_msg = "failed to create db";
		goto cleanup;
	}

	qDebug("%s: created db if it did not exist already", __func__);

	ret_val = axc_db_init_status_get(&init_status);
	switch (ret_val) {
		case -1:
		default:
			err_msg = "failed to read init status";
			goto cleanup;
		case 0:
			// there is a value
			switch (init_status) {
				case AXC_DB_NOT_INITIALIZED:
					// init needed
					db_needs_init = true;
					break;
				case AXC_DB_NEEDS_ROLLBACK:
					// reset and init needed
					db_needs_reset = true;
					db_needs_init = true;
					break;
				case AXC_DB_INITIALIZED:
				default:
					// the db is already initialised
					break;
			}
			break;
		case 1:
			// no value = not initialised -> init needed
			db_needs_init = true;
			break;
	}

	if (db_needs_reset) {
		qDebug("%s: db needs reset", __func__ );
		ret_val = axc_db_destroy();
		if (ret_val) {
			err_msg = "failed to reset db";
			goto cleanup;
		}

		ret_val = axc_db_create();
		if (ret_val) {
			err_msg = "failed to create db after reset";
			goto cleanup;
		}
	} else {
		qDebug("%s: db does not need reset", __func__ );
	}

	if (db_needs_init) {
		qDebug("%s: db needs init", __func__ );
		qDebug("%s: setting init status to AXC_DB_NEEDS_ROLLBACK (%i)", __func__, AXC_DB_NEEDS_ROLLBACK );

		ret_val = axc_db_init_status_set(AXC_DB_NEEDS_ROLLBACK);
		if (ret_val) {
			err_msg = "failed to set init status to AXC_DB_NEEDS_ROLLBACK";
			goto cleanup;
		}

		ret_val = signal_protocol_key_helper_generate_identity_key_pair(&identity_key_pair_p, FGlobalContext);
		if (ret_val) {
			err_msg = "failed to generate the identity key pair";
			goto cleanup;
		}
		qDebug("%s: generated identity key pair", __func__ );

		ret_val = signal_protocol_key_helper_generate_registration_id(&registration_id, 1, FGlobalContext);
		if (ret_val) {
			err_msg = "failed to generate registration id";
			goto cleanup;
		}
		qDebug("%s: generated registration id: %i", __func__, registration_id);

		ret_val = signal_protocol_key_helper_generate_pre_keys(&pre_keys_head_p,
															   APreKeyStartId,
															   APreKeyAmount,
															   FGlobalContext);
		if(ret_val) {
			err_msg = "failed to generate pre keys";
			goto cleanup;
		}
		qDebug("%s: generated pre keys", __func__ );

		ret_val = signal_protocol_key_helper_generate_signed_pre_key(
					&signed_pre_key_p, identity_key_pair_p, ASignedPreKeyId,
					quint64(QDateTime::currentMSecsSinceEpoch()),
					FGlobalContext);
		if (ret_val) {
			err_msg = "failed to generate signed pre key";
			goto cleanup;
		}
		qDebug("%s: generated signed pre key", __func__ );

		ret_val = axc_db_identity_set_key_pair(identity_key_pair_p);
		if (ret_val) {
			err_msg = "failed to set identity key pair";
			goto cleanup;
		}
		qDebug("%s: saved identity key pair", __func__ );

		ret_val = axc_db_identity_set_local_registration_id(registration_id);
		if (ret_val) {
			err_msg = "failed to set registration id";
			goto cleanup;
		}
		qDebug("%s: saved registration id", __func__ );

		ret_val = axc_db_pre_key_store_list(pre_keys_head_p);
		if (ret_val) {
			err_msg = "failed to save pre key list";
			goto cleanup;
		}
		qDebug("%s: saved pre keys", __func__ );

		ret_val = session_signed_pre_key_serialize(&signed_pre_key_data_p, signed_pre_key_p);
		if (ret_val) {
			err_msg = "failed to serialize signed pre key";
			goto cleanup;
		}

		ret_val = axc_db_signed_pre_key_store(session_signed_pre_key_get_id(signed_pre_key_p),
											  signal_buffer_data(signed_pre_key_data_p),
											  signal_buffer_len(signed_pre_key_data_p),
											  this);
		if (ret_val) {
			err_msg = "failed to save signed pre key";
			goto cleanup;
		}
		qDebug("%s: saved signed pre key", __func__ );

		ret_val = axc_db_init_status_set(AXC_DB_INITIALIZED);
		if (ret_val) {
			err_msg = "failed to set init status to AXC_DB_INITIALIZED";
			goto cleanup;
		}
		qDebug("%s: initialised DB", __func__ );

	} else {
		qDebug("%s: db already initialized", __func__ );
	}

cleanup:
	if (ret_val < 0) {
		qCritical("%s: %s", __func__, err_msg);
	}

	if (db_needs_init) {
		SIGNAL_UNREF(identity_key_pair_p);
		signal_protocol_key_helper_key_list_free(pre_keys_head_p);
		SIGNAL_UNREF(signed_pre_key_p);
		signal_buffer_bzero_free(signed_pre_key_data_p);
	}

	return ret_val;
}

int SignalProtocol::getDeviceId(quint32 &AId)
{
	return signal_protocol_identity_get_local_registration_id(FStoreContext, &AId);
}

int SignalProtocol::isSessionExistsAndInitiated(const QString &ABareJid, qint32 ADeviceId)
{
	signal_protocol_address address = {ABareJid.toUtf8(),
									   size_t(ABareJid.size()),
									   ADeviceId};
	int ret_val = 0;
	char * err_msg = nullptr;

	session_record * sessionRecord = nullptr;
	session_state * sessionState = nullptr;

	//TODO: if there was no response yet, even though it is an established session it keeps sending prekeymsgs
	//      maybe that is "uninitiated" too?
	if(!signal_protocol_session_contains_session(FStoreContext, &address)) {
		return 0;
	}

	ret_val = signal_protocol_session_load_session(FStoreContext, &sessionRecord, &address);
	if (ret_val){
		err_msg = "database error when trying to retrieve session";
		goto cleanup;
	} else {
		sessionState = session_record_get_state(sessionRecord);
		if (session_state_has_pending_key_exchange(sessionState)) {
			err_msg = "session exists but has pending synchronous key exchange";
			ret_val = 0;
			goto cleanup;
		}
		ret_val = 1;
	}

cleanup:
	if (ret_val < 1)
	  qCritical("%s: %s", __func__, err_msg);

	SIGNAL_UNREF(sessionRecord);
	return ret_val;
}

session_cipher *SignalProtocol::sessionCipherCreate(const QString &ABareJid, int ADeviceId)
{
	signal_protocol_address AAddress = {ABareJid.toUtf8(),
										size_t(ABareJid.size()),
										ADeviceId};
	// Create the session cipher
	session_cipher	*cipher(nullptr);
	int rc = session_cipher_create(&cipher, FStoreContext, &AAddress, FGlobalContext);
	if (rc != SG_SUCCESS)
		qCritical("session_builder_process_pre_key_bundle() failed! rc=%d", rc);
	return cipher;
}

QByteArray SignalProtocol::encrypt(session_cipher *ACipher, const QByteArray &AUnencrypted)
{
	QByteArray result;
	ciphertext_message *message;
	int rc = session_cipher_encrypt(ACipher,
									reinterpret_cast<const quint8*>(
										AUnencrypted.data()),
									size_t(AUnencrypted.size()),
									&message);
	if (rc == SG_SUCCESS)
	{
		// Get the serialized content
		signal_buffer *serialized = ciphertext_message_get_serialized(message);
		result = SignalProtocol::signalBufferToByteArray(serialized);
		signal_buffer_free(serialized);
	}

	SIGNAL_UNREF(message);

	return result;
}

QByteArray SignalProtocol::decrypt(session_cipher *ACipher, const QByteArray &AEncrypted)
{
	signal_message *ciphertext;
	QByteArray result;

	int rc = signal_message_deserialize(&ciphertext,
										reinterpret_cast<const quint8*>(
											AEncrypted.data()),
										size_t(AEncrypted.size()),
										FGlobalContext);
	if (rc == SG_SUCCESS)
	{
		signal_buffer *buffer(nullptr);

		rc = session_cipher_decrypt_signal_message(ACipher, ciphertext,
												   nullptr, &buffer);
		if (rc == SG_SUCCESS)
			result = SignalProtocol::signalBufferToByteArray(buffer);
		else
			qCritical("session_cipher_decrypt_signal_message() failed! rc=%d", rc);

		signal_buffer_bzero_free(buffer);
	}
	else
		qCritical("signal_message_deserialize() failed! rc=%d", rc);

	return result;
}

QByteArray SignalProtocol::decryptPre(session_cipher *ACipher, const QByteArray &AEncrypted)
{
	pre_key_signal_message *ciphertext(nullptr);
	QByteArray result;
	int rc = pre_key_signal_message_deserialize(&ciphertext,
												reinterpret_cast<const quint8*>(
													AEncrypted.data()),
												size_t(AEncrypted.size()),
												FGlobalContext);
	if (rc == SG_SUCCESS)
	{
		signal_buffer *plaintext(nullptr);

		rc = session_cipher_decrypt_pre_key_signal_message(ACipher, ciphertext,
														   nullptr, &plaintext);
		if (rc == SG_SUCCESS)
		{
			result = SignalProtocol::signalBufferToByteArray(plaintext);
		}
		else
			qCritical("session_cipher_decrypt_signal_message() failed! rc=%d", rc);

		signal_buffer_bzero_free(plaintext);
	}
	else
		qCritical("pre_key_signal_message_deserialize() failed! rc=%d", rc);

	return result;
}

QByteArray SignalProtocol::signalBufferToByteArray(signal_buffer *ABuffer)
{
	size_t len = signal_buffer_len(ABuffer);
	if (len>0)
	{
		const uint8_t *data = signal_buffer_const_data(ABuffer);
		if (data)
			return QByteArray(reinterpret_cast<const char *>(data), int(len));
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
				retVal = QByteArray(reinterpret_cast<const char *>(signature), int(sigLen));
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

session_pre_key_bundle *SignalProtocol::createPreKeyBundle(uint32_t ARegistrationId,
														   int ADeviceId, uint32_t APreKeyId,
														   const QByteArray &APreKeyPublic,
														   uint32_t ASignedPreKeyId,
														   const QByteArray &ASignedPreKeyPublic,
														   const QByteArray &ASignedPreKeySignature,
														   const QByteArray &AIdentityKey) const
{
	char *err_msg(nullptr);
	session_pre_key_bundle *bundle(nullptr);
	ec_public_key *preKeyPublic(nullptr),
				  *signedPreKeyPublic(nullptr),
				  *identityKey(nullptr);

	int rc = curve_decode_point(&preKeyPublic,
								reinterpret_cast<const quint8*>(
								APreKeyPublic.data()),
								size_t(APreKeyPublic.size()),
								FGlobalContext);
	if (rc != SG_SUCCESS) {
		err_msg = "curve_decode_point() failed!";
		goto cleanup;
	}

	rc = curve_decode_point(&signedPreKeyPublic,
							reinterpret_cast<const quint8*>(
							ASignedPreKeyPublic.data()),
							size_t(ASignedPreKeyPublic.size()),
							FGlobalContext);
	if (rc != SG_SUCCESS) {
		err_msg = "curve_decode_point() failed!";
		goto cleanup;
	}

	rc = curve_decode_point(&identityKey,
							reinterpret_cast<const quint8*>(
							AIdentityKey.data()),
							size_t(AIdentityKey.size()),
							FGlobalContext);
	if (rc != SG_SUCCESS) {
		err_msg = "curve_decode_point() failed!";
		goto cleanup;
	}

	rc = session_pre_key_bundle_create(&bundle, ARegistrationId, ADeviceId,
									   APreKeyId, preKeyPublic,
									   ASignedPreKeyId,
									   signedPreKeyPublic,
									   reinterpret_cast<const quint8*>(
										   ASignedPreKeySignature.data()),
									   size_t(ASignedPreKeySignature.size()),
									   identityKey);
	if (rc != SG_SUCCESS) {
		err_msg = "session_pre_key_bundle_create() failed!";
		goto cleanup;
	}

cleanup:
	if (err_msg)
		qCritical("%s: rc=%d", err_msg, rc);

	SIGNAL_UNREF(identityKey);
	SIGNAL_UNREF(signedPreKeyPublic);
	SIGNAL_UNREF(preKeyPublic);

	return bundle;
}

QByteArray SignalProtocol::encryptMessage(const QString &AMessageText,
										  const QByteArray &AKey,
										  const QByteArray &AIv,
										  QByteArray &AAuthTag)
{
	int rc = SG_SUCCESS;
	char * err_msg = nullptr;

	int algo = GCRY_CIPHER_AES128;
	int mode = GCRY_CIPHER_MODE_GCM;
	gcry_cipher_hd_t cipher_hd = {nullptr};
	QByteArray outBuf;
	QByteArray inBuf(AMessageText.toUtf8());

	if(AIv.size() != 16) {
		err_msg = "invalid AES IV size (must be not less than 8)";
		rc = SG_ERR_UNKNOWN;
		goto cleanup;
	}

	rc = int(gcry_cipher_open(&cipher_hd, algo, mode, 0));
	if (rc) {
		err_msg = "failed to init cipher";
		goto cleanup;
	}

	rc = int(gcry_cipher_setkey(cipher_hd, AKey.data(), size_t(AKey.size())));
	if (rc) {
		err_msg = "failed to set key";
		goto cleanup;
	}

	outBuf.resize(inBuf.size());
	rc = int(gcry_cipher_encrypt(cipher_hd, outBuf.data(), size_t(outBuf.size()),
											inBuf.data(), size_t(inBuf.size())));
	if (rc) {
		err_msg = "failed to encrypt";
		goto cleanup;
	}

	AAuthTag.resize(8);
	rc = int(gcry_cipher_gettag (cipher_hd, AAuthTag.data(), size_t(AAuthTag.size())));
	if (rc) {
		err_msg = "failed get authentication tag";
	} else {
		qDebug("Authentication tag: %s", AAuthTag.toHex().data());
	}

cleanup:
	if (rc) {
		if (rc > 0) {
			qCritical("%s: %s (%s: %s)\n", __func__, err_msg, gcry_strsource(rc), gcry_strerror(rc));
			rc = SG_ERR_UNKNOWN;
		} else {
			qCritical("%s: %s\n", __func__, err_msg);
		}
	}

	gcry_cipher_close(cipher_hd);

	return outBuf;
}

QString SignalProtocol::decryptMessage(const QByteArray &AEncryptedText,
									   const QByteArray &AKey,
									   const QByteArray &AIv,
									   const QByteArray &AAuthTag)
{
	int ret_val = SG_SUCCESS;
	char * err_msg = nullptr;

	int algo = GCRY_CIPHER_AES128;
	int mode = GCRY_CIPHER_MODE_GCM;

	gcry_cipher_hd_t cipher_hd = {nullptr};
	QByteArray outBuf;

	if(AIv.size() != 16) {
	  err_msg = "invalid AES IV size (must be not less than 8)";
	  ret_val = SG_ERR_UNKNOWN;
	  goto cleanup;
	}

	ret_val = int(gcry_cipher_open(&cipher_hd, algo, mode, 0));
	if (ret_val) {
	  err_msg = "failed to init cipher";
	  goto cleanup;
	}

	ret_val = int(gcry_cipher_setkey(cipher_hd, AKey.data(), size_t(AKey.size())));
	if (ret_val) {
	  err_msg = "failed to set key";
	  goto cleanup;
	}

	outBuf.resize(AEncryptedText.size());
	ret_val = int(gcry_cipher_decrypt(cipher_hd, outBuf.data(), size_t(outBuf.size()),
									  AEncryptedText.data(), size_t(AEncryptedText.size())));
	if (ret_val) {
		err_msg = "failed to decrypt";
		goto cleanup;
	}

	ret_val = int(gcry_cipher_checktag (cipher_hd, AAuthTag.data(),
										size_t(AAuthTag.size())));
	if (ret_val) {
		err_msg = "failed check authentication tag";
	} else {
		qDebug("Authentication tag checked successfuly!");
	}

  cleanup:
	if (ret_val) {
		if (ret_val > 0) {
			qCritical("%s: %s (%s: %s)\n", __func__, err_msg, gcry_strsource(ret_val), gcry_strerror(ret_val));
			ret_val = SG_ERR_UNKNOWN;
		} else {
			qCritical("%s: %s\n", __func__, err_msg);
		}
	}

	gcry_cipher_close(cipher_hd);

	return QString::fromUtf8(outBuf);
}

void SignalProtocol::getKeyPair(QByteArray &AKey, QByteArray &AIv)
{
	AKey.resize(16);
	gcry_randomize(AKey.data(), size_t(AKey.size()), GCRY_STRONG_RANDOM);
	AIv.resize(16);
	gcry_randomize(AIv.data(), size_t(AIv.size()), GCRY_STRONG_RANDOM);
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

static int choose_aes(int cipher, size_t key_len, int * algo_p, int * mode_p) {
  int algo = 0;
  int mode = 0;

  switch(key_len) {
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

  switch (cipher) {
	case SG_CIPHER_AES_CBC_PKCS5:
	  mode = GCRY_CIPHER_MODE_CBC;
	  break;
	case SG_CIPHER_AES_CTR_NOPADDING:
	  mode = GCRY_CIPHER_MODE_CTR;
	  break;
	default:
	  return SG_ERR_UNKNOWN;
  }

  *algo_p = algo;
  *mode_p = mode;

  return 0;
}

int SignalProtocol::randomFunc(uint8_t *AData, size_t ALen, void *AUserData)
{
	Q_UNUSED(AUserData);

//	SignalProtocol * axc_ctx_p = reinterpret_cast<SignalProtocol*>(AUserData);
	gcry_randomize(AData, ALen, GCRY_STRONG_RANDOM);

	return SG_SUCCESS;
}

int SignalProtocol::hmacSha256InitFunc(void **AHmacContext, const uint8_t *AKey, size_t AKeyLen, void *AUserData)
{
	Q_UNUSED(AUserData);

//	SignalProtocol * axc_ctx_p = reinterpret_cast<SignalProtocol *>(AUserData);
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

int SignalProtocol::hmacSha256UpdateFunc(void *AHmacContext, const uint8_t *AData, size_t ADataLen, void *AUserData)
{
	Q_UNUSED(AUserData);

	SignalProtocol * axc_ctx_p = reinterpret_cast<SignalProtocol *>(AUserData);
	(void) axc_ctx_p;

	gcry_mac_write(*reinterpret_cast<gcry_mac_hd_t *>(AHmacContext), AData, ADataLen);

	return SG_SUCCESS;
}

int SignalProtocol::hmacSha256FinalFunc(void *AHmacContext, signal_buffer **AOutput, void *AUserData)
{
	Q_UNUSED(AUserData);

//	SignalProtocol * axc_ctx_p = reinterpret_cast<SignalProtocol *>(AUserData);
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

	gcry_mac_hd_t * mac_hd_p = reinterpret_cast<gcry_mac_hd_t *>(AHmacContext);
	gcry_mac_close(*mac_hd_p);
	free(mac_hd_p);
}

int SignalProtocol::sha512DigestInitFunc(void **ADigestContext, void *AUserData)
{
	Q_UNUSED(AUserData);

//	SignalProtocol * axc_ctx_p = (SignalProtocol *) AUserData;
	int ret_val = 0;
	char * err_msg = nullptr;

	gcry_md_hd_t * hash_hd_p = nullptr;
	hash_hd_p = reinterpret_cast<gcry_md_hd_t*>(malloc(sizeof(gcry_mac_hd_t)));
	if (!hash_hd_p) {
	  err_msg = "could not malloc sha512 ctx";
	  ret_val = SG_ERR_NOMEM;
	  goto cleanup;
	}

	ret_val = int(gcry_md_open(hash_hd_p, GCRY_MD_SHA512, 0));
	if (ret_val) {
	  err_msg = "could not create sha512 ctx";
	  goto cleanup;
	}

	*ADigestContext = hash_hd_p;

  cleanup:
	if (ret_val) {
	  if (ret_val > 0) {
		qCritical("%s: %s (%s: %s)\n", __func__, err_msg, gcry_strsource(ret_val), gcry_strerror(ret_val));
		ret_val = SG_ERR_UNKNOWN;
	  } else {
		qCritical("%s: %s\n", __func__, err_msg);
	  }

	  if (hash_hd_p) {
		gcry_md_close(*hash_hd_p);
		free(hash_hd_p);
	  }
	}

	return ret_val;
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

//	SignalProtocol *signalProtocol = (SignalProtocol *)AUserData;
	gcry_md_hd_t * hash_hd_p = (gcry_md_hd_t *) ADigestContext;
	int ret_val = 0;
	char * err_msg = nullptr;

	int algo = GCRY_MD_SHA512;
	size_t hash_len = 0;
	unsigned char * hash_data_p = nullptr;
	signal_buffer * out_buf_p = nullptr;

	hash_len = gcry_md_get_algo_dlen(algo);

	hash_data_p = gcry_md_read(*hash_hd_p, algo);
	if (!hash_data_p) {
	  ret_val = SG_ERR_UNKNOWN;
	  err_msg = "failed to read hash";
	  goto cleanup;
	}

	out_buf_p = signal_buffer_create((uint8_t *) hash_data_p, hash_len);
	if (!out_buf_p) {
	  ret_val = SG_ERR_NOMEM;
	  err_msg = "failed to create hash output buf";
	  goto cleanup;
	}

	gcry_md_reset(*hash_hd_p);

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

	return ret_val;
}

void SignalProtocol::sha512DigestCleanupFunc(void *ADigestContext, void *AUserData)
{
	Q_UNUSED(AUserData);

	gcry_md_hd_t * hash_hd_p = reinterpret_cast<gcry_md_hd_t *>(ADigestContext);

	gcry_md_close(*hash_hd_p);
	free(hash_hd_p);
}

int SignalProtocol::encryptFunc(signal_buffer **AOutput, int ACipher, const uint8_t *AKey,
								size_t AKeyLen, const uint8_t *AIv, size_t AIvLen,
								const uint8_t *APlaintext, size_t APlaintextLen, void *AUserData)
{
	Q_UNUSED(AUserData);

	int rc = SG_SUCCESS;
	char * err_msg = nullptr;
//	SignalProtocol * axc_ctx_p = (SignalProtocol *)AUserData;

	int algo = 0;
	int mode = 0;
	size_t pad_len = 0;
	size_t ct_len = 0;
	gcry_cipher_hd_t cipher_hd = {nullptr};
	uchar * pt_p = nullptr;
	uchar * out_p = nullptr;
	signal_buffer * out_buf_p = nullptr;

	if(AIvLen != 16) {
	  err_msg = "invalid AES IV size (must be 16)";
	  rc = SG_ERR_UNKNOWN;
	  goto cleanup;
	}

	rc = choose_aes(ACipher, AKeyLen, &algo, &mode);
	if (rc) {
	  err_msg = "failed to choose cipher";
	  rc = SG_ERR_UNKNOWN;
	  goto cleanup;
	}

	rc = gcry_cipher_open(&cipher_hd, algo, mode, 0);
	if (rc) {
	  err_msg = "failed to init cipher";
	  goto cleanup;
	}

	rc = gcry_cipher_setkey(cipher_hd, AKey, AKeyLen);
	if (rc) {
	  err_msg = "failed to set key";
	  goto cleanup;
	}

	switch (ACipher) {
	  case SG_CIPHER_AES_CBC_PKCS5:
		pad_len = 16 - (APlaintextLen % 16);
		if (pad_len == 0) {
		  pad_len = 16;
		}
		ct_len = APlaintextLen + pad_len;
		rc = int(gcry_cipher_setiv(cipher_hd, AIv, AIvLen));
		if (rc) {
		  err_msg = "failed to set iv";
		  goto cleanup;
		}
		break;
	  case SG_CIPHER_AES_CTR_NOPADDING:
		ct_len = APlaintextLen;
		rc = int(gcry_cipher_setctr(cipher_hd, AIv, AIvLen));
		if (rc) {
		  err_msg = "failed to set iv";
		  goto cleanup;
		}
		break;
	  default:
		rc = SG_ERR_UNKNOWN;
		err_msg = "unknown cipher";
		goto cleanup;
	}

	pt_p = (uint8_t*)malloc(sizeof(uint8_t) * ct_len);
	if (!pt_p) {
	  err_msg = "failed to malloc pt buf";
	  rc = SG_ERR_NOMEM;
	  goto cleanup;
	}
	memset(pt_p, pad_len, ct_len);
	memcpy(pt_p, APlaintext, APlaintextLen);

	out_p = (uchar*)malloc(sizeof(uint8_t) * ct_len);
	if (!out_p) {
	  err_msg = "failed to malloc ct buf";
	  rc = SG_ERR_NOMEM;
	  goto cleanup;
	}

	rc = gcry_cipher_encrypt(cipher_hd, out_p, ct_len, pt_p, ct_len);
	if (rc) {
	  err_msg = "failed to encrypt";
	  goto cleanup;
	}

	out_buf_p = signal_buffer_create(out_p, ct_len);
	*AOutput = out_buf_p;

cleanup:
	if (rc) {
	  if (rc > 0) {
		qCritical("%s: %s (%s: %s)\n", __func__, err_msg, gcry_strsource(rc), gcry_strerror(rc));
		rc = SG_ERR_UNKNOWN;
	  } else {
		qCritical("%s: %s\n", __func__, err_msg);
	  }
	}

	free(out_p);
	gcry_cipher_close(cipher_hd);

	return rc;
}

int SignalProtocol::decryptFunc(signal_buffer **AOutput, int ACipher, const uint8_t *AKey, size_t AKeyLen, const uint8_t *AIv, size_t AIvLen, const uint8_t *ACiphertext, size_t ACiphertextLen, void *AUserData)
{
	Q_UNUSED(AUserData);

	int ret_val = SG_SUCCESS;
	char * err_msg = nullptr;
//	SignalProtocol *axc_ctx_p = reinterpret_cast<SignalProtocol *>(AUserData);

	int algo = 0;
	int mode = 0;
	gcry_cipher_hd_t cipher_hd = {nullptr};
	uchar * out_p = nullptr;
	size_t pad_len = 0;
	signal_buffer * out_buf_p = nullptr;

	if(AIvLen != 16) {
	  err_msg = "invalid AES IV size (must be 16)";
	  ret_val = SG_ERR_UNKNOWN;
	  goto cleanup;
	}

	ret_val = choose_aes(ACipher, AKeyLen, &algo, &mode);
	if (ret_val) {
	  err_msg = "failed to choose cipher";
	  ret_val = SG_ERR_UNKNOWN;
	  goto cleanup;
	}

	ret_val = gcry_cipher_open(&cipher_hd, algo, mode, 0);
	if (ret_val) {
	  err_msg = "failed to init cipher";
	  goto cleanup;
	}

	ret_val = gcry_cipher_setkey(cipher_hd, AKey, AKeyLen);
	if (ret_val) {
	  err_msg = "failed to set key";
	  goto cleanup;
	}

	switch (ACipher) {
	  case SG_CIPHER_AES_CBC_PKCS5:
		pad_len = 1;
		ret_val = gcry_cipher_setiv(cipher_hd, AIv, AIvLen);
		if (ret_val) {
		  err_msg = "failed to set iv";
		  goto cleanup;
		}
		break;
	  case SG_CIPHER_AES_CTR_NOPADDING:
		ret_val = gcry_cipher_setctr(cipher_hd, AIv, AIvLen);
		if (ret_val) {
		  err_msg = "failed to set iv";
		  goto cleanup;
		}
		break;
	  default:
		ret_val = SG_ERR_UNKNOWN;
		err_msg = "unknown cipher";
		goto cleanup;
	}

	out_p = (uchar*)malloc(sizeof(uint8_t) * ACiphertextLen);
	if (!out_p) {
	  err_msg = "failed to malloc pt buf";
	  ret_val = SG_ERR_NOMEM;
	  goto cleanup;
	}

	ret_val = gcry_cipher_decrypt(cipher_hd, out_p, ACiphertextLen, ACiphertext, ACiphertextLen);
	if (ret_val) {
	  err_msg = "failed to decrypt";
	  goto cleanup;
	}

	if (pad_len) {
	  pad_len = out_p[ACiphertextLen - 1];
	}

	out_buf_p = signal_buffer_create(out_p, ACiphertextLen - pad_len);
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

	free(out_p);
	gcry_cipher_close(cipher_hd);

	return ret_val;
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

SignalProtocol::SignalProtocol(const QString &AFileName):
	FGlobalContext(nullptr),
	FIdentityKeyPair(nullptr),
	FRegistrationId(0),
	FPreKeysHead(nullptr),
	FSignedPreKey(nullptr),
	FStoreContext(nullptr),
	FFileName(AFileName),
	FMutex(new QMutex(QMutex::Recursive))
{
	char *err_msg = nullptr;
	signal_protocol_store_context * store_context_p = nullptr;

	// 1. create global context
	if (signal_context_create(&FGlobalContext, this)) {
	  err_msg = "failed to create global signal protocol context";
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
	  err_msg = "failed to set signal protocol crypto provider";
	  FError = -1;
	  goto cleanup;
	}
	qDebug("%s: set signal protocol crypto provider", __func__);

	// 3. set locking functions
	if (signal_context_set_locking_functions(FGlobalContext, recursiveMutexLock, recursiveMutexUnlock)) {
	  err_msg = "failed to set locking functions";
	  FError = -1;
	  goto cleanup;
	}
	qDebug("%s: set locking functions", __func__);

	// Init store context
	if (signal_protocol_store_context_create(&store_context_p, FGlobalContext)) {
		err_msg = "failed to create store context";
		FError = -1;
		goto cleanup;
	}
	qDebug("%s: created store context", __func__);

	signal_protocol_session_store session_store;
	session_store.load_session_func = &axc_db_session_load;
	session_store.get_sub_device_sessions_func = &axc_db_session_get_sub_device_sessions;
	session_store.store_session_func = &axc_db_session_store;
	session_store.contains_session_func = &axc_db_session_contains;
	session_store.delete_session_func = &axc_db_session_delete;
	session_store.delete_all_sessions_func = &axc_db_session_delete_all;
	session_store.destroy_func = &axc_db_session_destroy_store_ctx;
	session_store.user_data = this;

	if (signal_protocol_store_context_set_session_store(store_context_p, &session_store)) {
		err_msg = "failed to create session store";
		FError = -1;
		goto cleanup;
	}

	signal_protocol_pre_key_store pre_key_store;
	pre_key_store.load_pre_key = &axc_db_pre_key_load;
	pre_key_store.store_pre_key = &axc_db_pre_key_store;
	pre_key_store.contains_pre_key = &axc_db_pre_key_contains;
	pre_key_store.remove_pre_key = &axc_db_pre_key_remove;
	pre_key_store.destroy_func = &axc_db_pre_key_destroy_ctx;
	pre_key_store.user_data = this;

	if (signal_protocol_store_context_set_pre_key_store(store_context_p, &pre_key_store)) {
		err_msg = "failed to set pre key store";
		FError = -1;
		goto cleanup;
	}

	signal_protocol_signed_pre_key_store signed_pre_key_store;
	signed_pre_key_store.load_signed_pre_key = &axc_db_signed_pre_key_load;
	signed_pre_key_store.store_signed_pre_key = &axc_db_signed_pre_key_store;
	signed_pre_key_store.contains_signed_pre_key = &axc_db_signed_pre_key_contains;
	signed_pre_key_store.remove_signed_pre_key = &axc_db_signed_pre_key_remove;
	signed_pre_key_store.destroy_func = &axc_db_signed_pre_key_destroy_ctx;
	signed_pre_key_store.user_data = this;

	if (signal_protocol_store_context_set_signed_pre_key_store(store_context_p, &signed_pre_key_store)) {
		err_msg = "failed to set signed pre key store";
		FError = -1;
		goto cleanup;
	}

	signal_protocol_identity_key_store identity_key_store;
	identity_key_store.get_identity_key_pair = &axc_db_identity_get_key_pair;
	identity_key_store.get_local_registration_id = &axc_db_identity_get_local_registration_id;
	identity_key_store.save_identity = &axc_db_identity_save;
	identity_key_store.is_trusted_identity = &axc_db_identity_always_trusted;
	identity_key_store.destroy_func = &axc_db_identity_destroy_ctx;
	identity_key_store.user_data = this;

	if (signal_protocol_store_context_set_identity_key_store(store_context_p, &identity_key_store)) {
		err_msg = "failed to set identity key store";
		FError = -1;
		goto cleanup;
	}

	FStoreContext = store_context_p;
	qDebug("%s: set store context", __func__);

cleanup:
	if (FError < 0) {
		qCritical("%s: %s", __func__, err_msg);
	} else {
		OmemoStore::init(FFileName);
		qInfo("%s: done initializing SignalProtocol", __func__);
	}
}

SignalProtocol::~SignalProtocol()
{
	uninit();
	// Uninit store context
	signal_protocol_store_context_destroy(FStoreContext);
}

axc_buf_list_item::axc_buf_list_item(uint32_t AId, signal_buffer *ABuf_p):
	id(AId), buf_p(ABuf_p)
{}

signal_context *				SessionBuilder::FGlobalContext(nullptr);
signal_protocol_store_context *	SessionBuilder::FStoreContext(nullptr);

SessionBuilder::SessionBuilder(const QString &ABareJid, int ADeviceId):
	ABuilder(nullptr)
{
	signal_protocol_address AAddress = {ABareJid.toUtf8(),
										size_t(ABareJid.size()),
										ADeviceId};
	// Instantiate a session_builder for a recipient address.
	int rc = session_builder_create(&ABuilder, FStoreContext, &AAddress, FGlobalContext);
	if (rc != SG_SUCCESS)
		qCritical("session_builder_create() failed! rc=%d", rc);
}

SessionBuilder::~SessionBuilder()
{
	if (ABuilder)
		session_builder_free(ABuilder);
}

bool SessionBuilder::isOk() const
{
	return ABuilder != nullptr;
}

void SessionBuilder::init(signal_context *AGlobalContext, signal_protocol_store_context *AStoreContext)
{
	FGlobalContext = AGlobalContext;
	FStoreContext = AStoreContext;
}
